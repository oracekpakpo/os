#include <paging.h>

struct allocator __frame_allocator;
struct page_table __page_table __attribute__((aligned(0x400*0x1000)));
struct page_directory __page_directory __attribute__((aligned(0x1000)));

void enable_paging()
{
    __asm__ __volatile__("movl %1, %%cr3;"
                        "push %%eax \n"
                        "movl %%cr0, %%eax \n"
                        "orl %0,%%eax \n"
                        "movl %%eax, %%cr0 \n"
                        "pop %%eax"
                        ::"i"(PAGING_FLAG),
                        "r"(&__page_directory)
                        );
}

void disable_paging()
{
    __asm__ __volatile__("push %%eax;"
                        "movl %%cr0,%%eax;"
                        "andl %0,%%eax;"
                        "movl %%eax, %%cr0;"
                        "pop %%eax;"
                        ::"i"(~PAGING_FLAG));

}

int _kernel_mapping(u32 v_base, u32 v_limit)
{

    if(v_base%PAGE_SIZE || v_limit%PAGE_SIZE)
    {
        return 0;
    }

    for(int i = v_base >> 22; i < v_limit >> 22; i++)
    {
        __page_directory.pde[i] = (u32)(&__page_table.table[i])|PAGE_PRESENT|PAGE_READ_WRITE;
        
        for(int j = 0; j < 0x400; j++)
        {
            __page_table.table[i].page[j] = (i*0x400000 + j*PAGE_SIZE)|PAGE_PRESENT|PAGE_READ_WRITE;
        }
    }

    return 1;
}

int _heap_mapping(u32 v_base, u32 v_limit)
{
    if(v_base%PAGE_SIZE || v_limit%PAGE_SIZE)
    {
        return 0;
    }

    for(int i = v_base >> 22; i < v_limit >> 22; i++)
    {
        __page_directory.pde[i] = (u32)(&__page_table.table[i])|PAGE_PRESENT|PAGE_READ_WRITE|
        PAGE_USER_SUPERUSER;
        
        for(int j = 0; j < 0x400; j++)
        {
            __page_table.table[i].page[j] = (i*0x400000 + j*PAGE_SIZE)|PAGE_PRESENT|PAGE_READ_WRITE|
            PAGE_USER_SUPERUSER;
        }
    }

    return 1;    
}



int init_frame_allocator(char* p_base, char* p_limit)
{
    if(p_base%PAGE_SIZE || p_limit%PAGE_SIZE)
    {
        return 0;
    }

    kprintf("Installing EOS user page allocator inside %x --> %x",p_base,p_limit);
    
    __frame_allocator.p_base = p_base;
    __frame_allocator.p_limit = p_limit;
    INIT_LIST_HEAD(&__frame_allocator.frame_list);

    kprintf(" [ok]\n");
    return 1;
}

struct frame* sys_alloc_frame(int nb_page)
{
    struct list_head* it = NULL;
    list_for_each(it,&__frame_allocator.frame_list)
    {
        struct frame* frame = list_entry(it,struct frame, head);
        if(!(frame->nb_page & 0x01) && (frame->nb_page >> 1) >= nb_page)
        {
            int r_page = (frame->nb_page >> 1) - nb_page;
            if(r_page > 0)
            {
                struct frame* _new = (struct frame*)sys_alloc(sizeof(struct frame));
                _new->p_addr = frame->p_addr + nb_page*PAGE_SIZE;
                _new->nb_page = r_page;
                _new->nb_page <<= 1;
                _new->v_addr = NULL;

                _new->head.next = frame->head.next;
                frame->head.next = &_new->head;
                _new->head.prev = &frame->head;
            }

            frame->nb_page = nb_page;
            frame->nb_page <<= 1;
            frame->nb_page |= 1;
            return frame;
        }
    }// case frame is found inside free frames

    if(IS_EMPTY(&__frame_allocator.frame_list))
    {
        struct frame* frame = (struct frame*)sys_alloc(sizeof(struct frame));
        frame->p_addr = __frame_allocator.p_base;
        frame->nb_page = nb_page;
        frame->nb_page <<= 1;
        frame->nb_page |= 1;
        frame->v_addr = NULL;
        list_auto_add(&_frame->head,&__frame_allocator.frame_list);
        return frame;
    }// case frame is empty

    struct frame* _new = (struct frame*)sys_alloc(sizeof(struct frame));
    list_last_head(it,&__frame_allocator.frame_list);

    struct frame* _old = list_entry(it,struct frame, head);

    if(_old->p_addr + (_old->nb_page >> 1)*PAGE_SIZE + nb_page*PAGE_SIZE > __frame_allocator.p_limit)
        return NULL;

    _new->nb_page = nb_page;
    _new->nb_page <<= 1;
    _new->nb_page |= 1;
    _new->p_addr = _old->p_addr + (_old->nb_page >> 1) * PAGE_SIZE;
    _new->v_addr = NULL;
    list_add(&_new->head,&_old->head);
    return _new;
    // case frame isn't found inside free frames

}

struct frame* search()
{
    struct list_head* it = NULL, *it_next = NULL;

    list_for_each(it,&__frame_allocator.frame_list)
    {
        struct frame* frame = list_entry(it,struct frame,head);
        it_next = it->next;

        if(it_next)
        {
            struct frame* next = list_entry(it_next,struct frame,head);
            if(!(frame->nb_page & 0x01) && !(next->nb_page & 0x01))
            {
                return frame;
            }
        }
    }

    return NULL;
}

void sys_free_frame(struct frame* frame)
{
    frame->nb_page &= 0xfffffffe;
    loop:;
    
    struct frame* start = search();
    if(start)
    {
        struct frame* next = list_entry(start->head.next, struct frame, head);
        while(!(next->nb_page & 0x01))
        {
            start->nb_page += next->nb_page;
            list_del(&next->head);
            sys_free(next);
            next = list_entry(start->head.next, struct frame, head);
        }
        goto loop;
    }
}

