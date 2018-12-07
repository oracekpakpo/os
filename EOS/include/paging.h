#ifndef _PAGING_H__
#define _PAGING_H__
#include <types.h>
#include <memory.h>
#include <list.h>

#define PAGE_SIZE 0x1000 // 4 Ko
#define PAGING_FLAG 0x80000000
#define PAGE_PRESENT 0x001
#define PAGE_READ_WRITE 0x002
#define PAGE_USER_SUPERUSER 0x004
#define PAGE_WRITE 0x008
#define PAGE_DISABLE 0x010
#define PAGE_ACCESSED 0x020
#define PAGE_SIZE_4KO 0x000
#define PAGE_SIZE_4MO 0x080

struct frame
{
	char* p_addr;
	char* v_addr;
	int nb_page;
	struct list_head head;
};

struct allocator
{
	struct list_head frame_list;
	char* p_base;
	char* p_limit;
};

struct table
{
	u32 page[0x400];
};

struct page_table
{
	struct table table[0x400]:
};

struct page_directory
{
	u32 pde[0x400];
};


static inline void init_sys_page_allocator(sys_page_allocator* alloc, u32 mem_base, u32 mem_limit)
{
	alloc->mem_base = mem_base;
	alloc->mem_limit = mem_limit;

	INIT_LIST_HEAD(&alloc->b_page);
	INIT_LIST_HEAD(&alloc->f_page);
	INIT_LIST_HEAD(&alloc->d_page);
}

struct frame* sys_alloc_frame(int);
void sys_free_frame(struct frame*);
struct frame* search();
int init_frame_allocator(char*, char*);

void enable_paging();
void disable_paging();
int _kernel_mapping(u32 v_base, u32 v_limit);
int _heap_mapping(u32 v_base, u32 v_limit);

#endif
