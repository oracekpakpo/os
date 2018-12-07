#include <fs/dcache.h>

struct dcache* dcache_manager_alloc_dcache(struct dcache_manager* __dcache_manager)
{
	if(!list_empty(&__dcache_manager->dcache_manager_fdcache.head))
	{
		struct dcache* __dcache = list_first_entry(&__dcache_manager->dcache_manager_fdcache.head,struct dache,dcache_head);
		list_remove(&__dcache_manager->dcache_manager_fdcache,&__dcache->dcache_head);
		return __dcache;
	}

	if(__dcache_manager->dcache_manager_dcache < (u32_t)(__dcache_manager->dcache_manager_end - __dcache_manager->dcache_manager_beg) / __dcache_manager->dcache_size)
	{
		struct dcache* __dcache = sys_alloc(sizeof(struct dcache));
		__dcache->dcache_beg = __dcache_manager->dcache_manager_dcache * __dcache_manager->dcache_size + __dcache_manager->dcache_manager_beg;
		__dcache->dcache_end = __dcache->dcache_beg + __dcache_manager->dcache_size;
		init_list(&__dcache->dcache_fdentry);
		__dcache->dcache_dentry = 0;
		__dcache->alloc_dentry = dcache_alloc_dentry;
		__dcache->free_dentry = dcache_free_dentry;
		__dcache_manager->dcache_manager_dcache++;	
		return __dcache;
	}
	
	return NULL;
}


void dcache_manager_free_dcache(struct dcache* __dcache, struct dcache_manager* __dcache_manager)
{
	list_insert(&__dcache_manager->dcache_manager_fdcache,&__dcache->dcache_head);
	__dcache = NULL;
}

struct dentry* dcache_alloc_dentry(struct dcache* __dcache)
{
	if(!list_empty(&__dcache->dcache_fdentry.head))
	{
		struct dentry* __dentry = list_first_entry(&__dcache->dcache_fdentry.head,struct dentry,d_chead);
		list_remove(&__dcache->dcache_fdentry,&__dentry->d_chead);
		return __dentry;
	}

	if(__dcache->dcache_dentry < (u32_t)(__dcache->dcache_end - __dcache->dcache_beg) / sizeof(struct dentry))
	{
		struct dentry* __dentry = (struct dentry*)(__dcache->dcache_beg);
		__dentry = &__dentry[__dcache->dcache_dentry];
		__dcache->dcache_dentry++;
		return __dentry;
	}

	return NULL;
}

void dcache_free_dentry(struct dentry* __dentry, struct dcache* __dcache)
{
	list_insert(&__dcache->dcache_fdentry,&__dentry->d_chead);
	__dentry = NULL;
}

