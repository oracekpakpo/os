#include <memory.h>

sys_memory_allocator dynamic_alloc;

mm_chunk* sys_check_f_mm_chunk(u32 bytes) // check inside the f_mm_chunk list a chunk
{
	struct list_head* it = NULL;
	list_for_each(it,&dynamic_alloc.f_mm_chunk)
	{
		mm_chunk* chunk = list_entry(it,mm_chunk,head);
		u32 chunk_bytes = chunk->bytes;
		if(chunk_bytes >= bytes)
		{
			list_del(&chunk->head); // remove from f_mm_chunk list
			chunk->bytes = bytes;

			u32 rbytes = chunk_bytes - bytes;

			if(rbytes > sizeof(mm_chunk))
			{
				mm_chunk* new_chunk  = (mm_chunk*)chunk->ptr + chunk->bytes;
				new_chunk->bytes = rbytes - sizeof(mm_chunk);
				new_chunk->ptr = new_chunk + sizeof(mm_chunk);
				list_auto_add(&new_chunk->head,&dynamic_alloc.f_mm_chunk); // add new chunk to f_mm_chunk list
			}

			return chunk;
		}
	}

	return NULL;
}


void* sys_alloc(u32 bytes)
{
	mm_chunk* chunk = sys_check_f_mm_chunk(bytes);
	if(chunk)
	{
		list_auto_add(&chunk->head,&dynamic_alloc.d_mm_chunk); // add chunk found to d_mm_chunk list
		return chunk->ptr;
	}

	if(IS_EMPTY(&dynamic_alloc.b_mm_chunk))
	{
		mm_chunk* new_chunk = (mm_chunk*)dynamic_alloc.heap_base;
		new_chunk->bytes = bytes;
		new_chunk->ptr = (void*)new_chunk + new_chunk->bytes;
		list_auto_add(&new_chunk->head,&dynamic_alloc.b_mm_chunk);

		return new_chunk->ptr;
	}

	struct list_head* it = NULL;
	list_last_head(it,&dynamic_alloc.b_mm_chunk); // get last head of b_mm_chunk

	mm_chunk* last_chunk = list_entry(it,mm_chunk,head); // last head --> mm_chunk
	mm_chunk* new_chunk = (mm_chunk*)last_chunk->ptr + last_chunk->bytes;
	new_chunk->ptr = (void*)new_chunk + sizeof(mm_chunk);
	new_chunk->bytes = bytes;

	if((new_chunk->ptr + new_chunk->bytes) >= HEAP_LIMIT)
		return NULL;

	list_auto_add(&new_chunk->head,&dynamic_alloc.b_mm_chunk);
	return new_chunk->ptr;
}

void sys_free(void* ptr)
{
	struct list_head* it = NULL;
	list_for_each(it,&dynamic_alloc.b_mm_chunk)
	{
		mm_chunk* chunk = list_entry(it,mm_chunk,head);
		if(chunk->ptr == ptr)
		{
			list_del(&chunk->head);
			list_auto_add(&chunk->head,&dynamic_alloc.f_mm_chunk);
			memset(chunk->ptr,0,chunk->bytes);
			return;
		}
	}

	list_for_each(it,&dynamic_alloc.d_mm_chunk)
	{
		mm_chunk* chunk = list_entry(it,mm_chunk,head);
		if(chunk->ptr == ptr)
		{
			list_del(&chunk->head);
			list_auto_add(&chunk->head,&dynamic_alloc.f_mm_chunk);
			memset(chunk->ptr,0,chunk->bytes);
			return;
		}
	}

}


void init_heap_allocator(u32 heap_base, u32 heap_limit)
{
	kprintf("Initialize EOS heap inside %x --> %x",heap_base,heap_limit);
	init_sys_memory_allocator(&dynamic_alloc,heap_base,heap_limit);
	kprintf(" [ok]\n");
}