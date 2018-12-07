#ifndef _PROCESS_H__
#define _PROCESS_H__

#include <paging.h>
#include <types.h>
#include <memory.h>
#include <queue.h>
#define  k_stack   0x4000
#define  u_stack   0x10000 // 64ko

extern int max_file;

struct mm_space
{
    u32 kstack[k_stack]; // (kernel stack --> esp0 = kstack + k_stack)
    struct frame* code;
    struct frame* stack;
};

struct registers
{
    u32 eax, ebx, ecx, edx, esi,
    edi, eflags, esp, eip;
    u16 cs, ds, es, fs, gs;

}__attribute__((packed));

typedef struct process
{
    char* name;
    int id;
    int state;    
    int sleep_time;
    int quantum;
    int priority;
    int realtime;
    struct queue_head schd_queue; 
    struct list_head schd_list// (schedule data)

    struct registers regs;
    struct process* parent;
    int fd[max_file];
    uchar cwd[256];
    struct mm_space mm_struct; // (memory context)

    struct list_head head;
    struct list_head childs; // (within link between process)

}process;

enum 
{
    sleep,
    ready,
    run     
};

void sys_save_regs(struct registers*);
void sys_load_regs(struct registers*);

#endif
