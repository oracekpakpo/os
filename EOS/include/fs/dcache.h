#ifndef __DCACHE_H__
#define __DCACHE_H__

#include <fs/fs.h>
#include <memory.h>
#include <types.h>
#include <list.h>

struct dcache
{
	u8_t* dcache_beg;
	u8_t* dcache_end;
	struct list dcache_fdentry;
	struct list_head dcache_head;
	u32_t dcache_dentry;
	struct dentry* (*alloc_dentry)(struct dcache*);
	void (*free_dentry)(struct dentry*, struct dcache*);
};

struct dentry* dcache_alloc_dentry(struct dcache* __dcache);
void dcache_free_dentry(struct dentry* __dentry, struct dcache* __dcache);


struct dcache_manager
{
	u8_t* dcache_manager_beg;
	u8_t* dcache_manager_end;
	u32_t dcache_size;
	struct list dcache_manager_fdcache;
	u32_t dcache_manager_dcache;
	struct dcache* (*alloc_dcache)(struct dcache_manager*);
	void (*free_dcache)(struct dcache*, struct dcache_manager*);
};

struct dcache* dcache_manager_alloc_dcache(struct dcache_manager* __dcache_manager);
void dcache_manager_free_dcache(struct dcache* __dcache, struct dcache_manager* __dcache_manager);

static inline void init_dcache_manager(u8_t* __mbeg, u8_t* __mend, u32_t __dcache_size, struct dcache_manager* __dcache_manager)
{
	__dcache_manager->dcache_manager_beg = __mbeg;
	__dcache_manager->dcache_manager_end = __mend;
	__dcache_manager->dcache_size = __dcache_size;
	init_list(&__dcache_manager->dcache_manager_fdcache);
	__dcache_manager->dcache_manager_dcache = 0;
	__dcache_manager->alloc_dcache = dcache_manager_alloc_dcache;
	__dcache_manager->free_dcache = dcache_manager_free_dcache;
}


#endif //__DCACHE_H__