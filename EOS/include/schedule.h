#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__

#include <process.h>
#include <memory_map.h>
#include <paging.h>
#include <queue.h>
#include <list.h>
#include <gdt.h>

#define  s_quantum 120 // ms
#define  l_quantum 480 //ms
#define  quantum   40  //ms


struct schd_data
{
	struct queue_head queue_0; // single fifo schd algorithme
	struct queue_head queue_1;
	struct queue_head queue_2;
	struct queue_head queue_3;
	struct queue_head queue_4; // level fifo schd algorithm

	int count; 
	struct process* current;
	struct list_head ready_list;
	struct list_head sleep_list;
	struct process* shell;
	struct process* gui;

};

struct schd_operation
{
	void (*schedule)();
	void (*sys_kill)(struct process*);
	void (*sys_wait)(int);
	void (*sys_sleep)(struct process*,int);
	int (*sys_getpid)(struct process*);
	int (*sys_getppid)(struct process*);
	struct process* (*sys_fork)(struct process*);
	void (*sys_exec)(const char*, int, char**);	
};

struct schd
{
	struct schd_data __data__;
	struct schd_operation __operation__;
};

struct process* create_process();
void create_mm_space(struct process*,int); // code size
void user_mapping(struct process*);
void run(struct process*);
void fifo_0();
void fifo_4();
void init_schd(struct schd*);

void sys_kill(struct process*);
void sys_wait(int);
void sys_sleep(struct process*,int);
int sys_getpid(struct process*);
int sys_getppid(struct process*);
struct process* sys_fork(struct process*);
void sys_exec(const char*, int, char**, char**);	

#endif
