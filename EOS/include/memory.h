#ifndef _MEMORY_H__
#define _MEMORY_H__

#include <memory_map.h>
#include <types.h>
#include <lib.h>
#include <list.h>

typedef struct
{
	struct list_head head;
	void* ptr;
	u32 bytes;
} mm_chunk; // the structure of a memory chunk

typedef struct
{
	struct list_head b_mm_chunk; // busy memory chunk list
	struct list_head f_mm_chunk; // free memory chunk list
	struct list_head d_mm_chunk; // dirty memory chunk list
	memory heap_base;
	memory heap_limit;
} sys_memory_allocator; // system's dynamic allocator structure

static inline void init_sys_memory_allocator(sys_memory_allocator* alloc, u32 heap_base, u32 heap_limit)
{
	INIT_LIST_HEAD(&alloc->b_mm_chunk);
	INIT_LIST_HEAD(&alloc->f_mm_chunk);
	INIT_LIST_HEAD(&alloc->d_mm_chunk);
	alloc->heap_base = (memory)heap_base;
	alloc->heap_limit = (memory)heap_limit;
}// initialize system's dynamic allocator

mm_chunk* sys_check_f_mm_chunk(u32 bytes); // check inside the f_mm_chunk list a chunk
void* sys_alloc(u32 bytes);
void sys_free(void* ptr);
void init_heap_allocator(u32 heap_base, u32 heap_limit);

#endif // _MEMORY_H__
