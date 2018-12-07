#include <schedule.h>

struct schd __sys_schd;

struct process* current = NULL;
int count = 0;
extern struct page_table __page_table;
extern struct page_directory __page_directory;
extern struct sys_tss_t tss;
extern int max_file;

struct process* create_process()
{
	cli();
	struct process* __new = (struct process*)sys_alloc(sizeof(struct process));
	if(!__new)
		return NULL;

	struct list_head* it = NULL;

	if(IS_EMPTY(&__sys_schd.__data__.ready_list))
	{
		__new->id = 1000;
	}	
	else
	{
		list_last_head(it,&__sys_schd.__data__.ready_list);
		struct process* __proc = list_entry(it,struct process,head);		
		__new->id = __proc->id + 1;
	}
	// field --> id

	__new->fd[0] = 0; // stdin file descriptor
	__new->fd[1] = 1; // stdout file descriptor
	__new->fd[2] = 2; // stderr file descriptor
	for(int i = 3; i < max_file; i++) __new->fd[i] = -1;
	// field --> fd

	__new->parent = __sys_schd.__data__.shell;
	__new->sleep_time = 0;
	strncpy(__new->cwd,__new->parent->cwd,256);
	// field --> cwd

	if(__sys_schd.__operation__.schedule == fifo_0)
	{
		__new->quantum = s_quantum;
		__new->priority = 0;
		queue_auto_add(&__new->schd_queue,&__sys_schd.__data__.queue_0);
	}
	else
	{
		__new->quantum = l_quantum;
		__new->priority = 10;
		queue_auto_add(&__new->schd_queue,&__sys_schd.__data__.queue_2);
	}
	list_auto_add(&__new->schd_list,&__sys_schd.__data__.ready_list);
	// fileds --> quantum & priority

	INIT_LIST_HEAD(&__new->childs);
	// field --> childs

	__new->regs.eax = 0x0;
	__new->regs.ebx = 0x0;
	__new->regs.ecx = 0x0;
	__new->regs.edx = 0x0;
	__new->regs.esi = 0x0;
	__new->regs.edi = 0x0;
	__new->regs.eflags = 0x0;
	__new->regs.esp = user_stack + u_stack; 
	__new->regs.eip = user_base;
	__new->regs.cs = USER_CS;
	__new->regs.ds = USER_DS;
	__new->regs.es = USER_ES;
	__new->regs.fs = USER_FS;
	__new->regs.gs = USER_GS;
	// field --> regs

	__asm__ __volatile__("pushfl;"
						"popl %0;"
						:"=m"(__new->regs.eflags)
						:
						);// Update eflags register

	__new->state = ready;
	// field --> state

	count++;
	__sys_schd.__data__.count = count;
	
	sti();
	return __new;
}

void create_mm_space(struct process* __act, int size) // code size
{
	cli();
	int nb_page = size / PAGE_SIZE;
	if(size % PAGE_SIZE) nb_page++;

	__act->mm_struct.code = sys_alloc_frame(nb_page);
	__act->mm_struct.stack = sys_alloc_frame(u_stack / PAGE_SIZE);
	tss.esp0 = __act->mm_struct.kstack + k_stack;
	tss.ss0 = KERNEL_SS;
	
	sti();
} 

void user_mapping(struct process* __act)
{
	cli();
	for(int i = user_base >> 22; i < 0x400; i++)
	{
		__page_directory.pde[i] = 0;
	}

	int c_table = __act->mm_struct.code->nb_page / 0x400;
	int s_table = __act->mm_struct.stack->nb_page / 0x400;

	u32 c_addr = (u32)__act->mm_struct.code->p_addr;
	u32 s_addr = (u32)__act->mm_struct.stack->p_addr;

	int c_page = __act->mm_struct.code->nb_page % 0x400;
	int s_page = __act->mm_struct.stack->nb_page % 0x400;

	if(c_table)
	{
		for(int i = user_base >> 22; i < (user_base >> 22) + c_table; i++)
		{
			__page_directory.pde[i] = (u32)(&__page_table.table[i])|PAGE_PRESENT|
			PAGE_READ_WRITE|PAGE_USER_SUPERUSER;

			for(int j = 0; j < 0x400; j++)
			{
				__page_table.table[i].page[j] = (u32)(c_addr)|PAGE_PRESENT|
				PAGE_READ_WRITE|PAGE_USER_SUPERUSER;

				c_addr += PAGE_SIZE;
			}
		}
	}

	if(c_page)
	{
		__page_directory.pde[(user_base >> 22) + c_table] = (u32)(&__page_table.table[(user_base >> 22) + c_table])|
	PAGE_PRESENT|PAGE_USER_SUPERUSER|PAGE_READ_WRITE;

		for(int i = 0; i < c_page; i++)
		{
			__page_table.table[(user_base >> 22) + c_table].page[i] = (u32)(c_addr)|PAGE_PRESENT|PAGE_READ_WRITE|
			PAGE_USER_SUPERUSER;

			c_addr += PAGE_SIZE;
		}
	}
	
	if(s_table)
	{
		for(int i = user_stack >> 22; i < (user_stack >> 22) + s_table; i++)
		{
			__page_directory.pde[i] = (u32)(&__page_table.table[i])|PAGE_PRESENT|
			PAGE_READ_WRITE|PAGE_USER_SUPERUSER;

			for(int j = 0; j < 0x400; j++)
			{
				__page_table.table[i].page[j] = (u32)(s_addr)|PAGE_PRESENT|
				PAGE_READ_WRITE|PAGE_USER_SUPERUSER;

				s_addr += PAGE_SIZE;
			}
		}
	}

	if(s_page)
	{
		__page_directory.pde[(user_stack >> 22) + s_table] = (u32)(&__page_table.table[(user_stack >> 22) + s_table])|
	PAGE_PRESENT|PAGE_USER_SUPERUSER|PAGE_READ_WRITE;

		for(int i = 0; i < s_page; i++)
		{
			__page_table.table[(user_stack >> 22) + s_table].page[i] = (u32)(s_addr)|PAGE_PRESENT|PAGE_READ_WRITE|
			PAGE_USER_SUPERUSER;

			s_addr += PAGE_SIZE;
		}
	}

	sti();
}

void run(struct process* __act)
{
	cli();
	current = __act;
	__sys_schd.__data__.current = current;

	sys_load_regs(&__act->regs);
	__asm__ __volatile__("push %0;"
						"push %1;"
						"push %2;"
						"push %3;"
						"push %4;"
						"pushl %%eax;"
						"movw $0x20, %%ax;"
						"outw %%ax, $0x20;"
						"popl eax;"
						"iretl;"
						::"m"(__act->regs.ss),"m"(__act->regs.esp),
						"m"(__act->regs.eflags),"m"(__act->regs.cs),
						"m"(__act->regs.eip)
						);
}

void fifo_0()
{
	current->quantum -= quantum;
	current->realtime += quantum;

	if(count <= 2)
		return;

	if(current->quantum)
		return;
	else
		current->quantum = s_quantum;
	sys_save_regs(&current->regs);

	if(current->regs.cs == KERNEL_CS)
		current->regs.ds = KERNEL_DS;
	else
		current->regs.ds = USER_DS;

	/* Get (eip,cs,eflags,esp,ss) data from stack */
	__asm__ __volatile__("movl 20(%%ebp),%%eax;"
						"movl %%eax, %0;"
						"movl 24(%%ebp),%%eax;"
						"movw %%ax, %1;"
						"movl 28(%%ebp),%%eax;"
						"movl %%eax, %2;"
						"movl 32(%%ebp),%%eax;"
						"movl %%eax, %3;"
						"movl 38(%%ebp),%%eax;"
						"movw %%ax, %4;"
						::"m"(current->regs.eip),"m"(current->regs.cs),
						"m"(current->regs.eflags),"m"(current->regs.esp),
						"m"(current->regs.ss)
						);

	current->regs.eflags |= (1 << 9); // Enable interrupt flag
	curent->regs.eflags &= 0xffffcfff; // IO-DPL :0x0	

	queue_del(&current->schd_queue);
	queue_auto_add(&current->schd_queue, &__sys_schd.__data__.queue_0);

	struct list_head* it = NULL;
	list_for_each(it,&__sys_schd.__data__.sleep_list)
	{
		struct process* __act = list_entry(it,struct process, schd_list);
		__act->sleep_time -= quantum;
		if(__act->sleep_time <= 0)
		{
			list_del(&__act->schd_list); // remove from sleep list
			list_auto_add(&__act->schd_list, &__sys_schd.__data__.ready_list); // Add on ready list
			
			/* Critical code section */
			__act->schd_queue.next = __sys_schd.__data__.queue_0.next;
			__sys_schd.__data__.queue_0.next = &__act->schd_queue;
			__act->schd_queue.prev = &__sys_schd.__data__.queue_0;
		}
	}// Update sleep list 

	struct process* __next = queue_first_entry(&__sys_schd.__data__.queue_0,struct process, schd_queue);
	// Select next process

	if(__next->regs.cs == USER_CS)
		tss.esp0 = __next->mm_struct.kstack + k_stack;
	// Update process kernel stack when were going to user mode

	user_mapping(__next);
	run(__next);	
}

void fifo_4()
{

}

void init_schd()
{
	kprintf("Installing EOS process manager ");

	INIT_QUEUE_HEAD(&__sys_schd.__data__.queue_0);
	INIT_QUEUE_HEAD(&__sys_schd.__data__.queue_1);
	INIT_QUEUE_HEAD(&__sys_schd.__data__.queue_2);
	INIT_QUEUE_HEAD(&__sys_schd.__data__.queue_3);
	INIT_QUEUE_HEAD(&__sys_schd.__data__.queue_4);

	INIT_LIST_HEAD(&__sys_schd.__data__.ready_list);
	INIT_LIST_HEAD(&__sys_schd.__data__.sleep_list);

	__sys_schd.__data__.count = 0;
	__sys_schd.__data__.current = NULL;
	__sys_schd.__data__.shell = NULL;
	__sys_schd.__data__.gui = NULL;

	__sys_schd.__operation__.schedule = fifo_0;
	__sys_schd.__operation__.sys_kill = sys_kill;
	__sys_schd.__operation__.sys_wait = sys_wait;
	__sys_schd.__operation__.sys_sleep = sys_sleep;
	__sys_schd.__operation__.sys_getid = sys_getid;
	__sys_schd.__operation__.sys_getpid = sys_getpid;
	__sys_schd.__operation__.sys_fork = sys_fork;
	__sys_schd.__operation__.sys_exec = sys_exec;

	kprintf("[ok]\n";)
}

void sys_kill(struct process* __act)
{
	cli(); // clear interrupt
	count--;
	__sys_schd.__data__.count = count;
	list_del(&__act->schd_list); // remove from ready_list
	queue_del(&__act->schd_queue); // remove from schd queue
	sys_free_frame(__act->mm_struct.code); // restore code frame
	sys_free_frame(__act->mm_struct.stack);	// restore stack frame

	sys_free(__act->name);
	struct list_head* it = NULL;

	list_for_each(it,&__act->childs)
	{
		struct process* child = list_entry(it,struct process, head);
		child->parent = NULL;		
	}

	sys_free(__act);

	user_mapping(__sys_schd.__data__.shell);
	run(__sys_schd.__data__.shell);
}

void sys_wait(int id)
{
	struct list_head* it = NULL;
	loop:;

	list_for_each(it,&__sys_schd.ready_list)
	{
		struct process* __new = list_entry(it,struct process,schd_list);
		if(__new->id == id)
			goto loop;
	}

	list_for_each(it,&__sys_schd.sleep_list)
	{
		struct process* __new = list_entry(it,struct process,schd_list);
		if(__new->id == id)
			goto loop;
	}
}

void sys_sleep(struct process* __act, int ms)
{
	__act->sleep_time = ms;
	list_del(&__act->schd_list);
	list_auto_add(&__act->schd_list, &__sys_schd.__data__.sleep_list);

	__act->state = sleep;
	queue_del(&__act->schd_queue);

	while(__act->sleep_time);
}

int sys_getid(struct process* __act)
{
	return __act->id;
}

int sys_getpid(struct process* __act)
{
	if(!__act->parent)
		return -1;

	return __act->parent->id;
}

struct process* sys_fork(struct process* __act)
{
	 cli();
	 sys_save_regs(&__act->regs);
	 struct process* child = create_process();
	 // Création de l'process fils

	if(!child)
		return NULL;

	create_mm_space(child,__act->mm_struct.code->nb_page * PAGE_SIZE);
	
	char* code = (char*)sys_alloc(child->mm_struct.code->nb_page*PAGE_SIZE);
	char* stack = (char*)sys_alloc(u_stack);
	if(!code || !stack)
	{
		sys_free(code);
		sys_free(stack);
		return NULL;
	}

	memcpy(code,user_base,child->mm_struct.code->nb_page*PAGE_SIZE);
	memcpy(stack,user_stack,u_stack);			
	// Copie du code dans un tampon

	list_auto_add(&child->schd_list,&__sys_schd.__data__.ready_list);
	child->parent = __act;
	list_auto_add(&child->head,&__act->childs);
	// Mise en place des liens

	/* Critical code section */
	child->schd_queue.next = __sys_schd.__data__.queue_0.next;
	__sys_schd.__data__.queue_0.next = &child->schd_queue;
	child->schd_queue.prev = &__sys_schd.__data__.queue_0;

	strncpy(child->cwd,__act->cwd,256);
	// current working directory
	memcpy(child->fd,__act->fd,(sizeof(int))*max_file);
	// fichiers ouverts

	child->regs = __act->regs;
	child->regs.eax = 0;
	__act->reg.eax = __act->id;

	user_mapping(child);
	memcpy(user_base,code,child->mm_struct.code->nb_page*PAGE_SIZE);
	memcpy(user_stack,stack,u_stack);
	
	sys_free(code);
	sys_free(stack);

	run(child);

	return child; // to avoid GCC warnning	

}

void sys_exec(const char*, int, char**)
{

}	
