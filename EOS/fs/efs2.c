#include <fs/efs2.h>

int max_file = MAX_FILE;
extern ata hd;
extern struct process* current;
/* static standard buffer */
struct fifo* stdin;
struct fifo* stderr;
struct fifo* stdout;

void inode_hashtable_t_insert(efs2_inode_t* __inode, inode_hashtable_t* __hashtable)
{
	list_insert(&__hashtable->i_list[__inode->i_num % INODE_HASH_MAX],&__inode->i_head);
}

void inode_hashtable_t_remove(efs2_inode_t* __inode, inode_hashtable_t* __hashtable)
{
	list_remove(&__hashtable->i_list[__inode->i_num % INODE_HASH_MAX], &__inode->i_head);
}

efs2_inode_t* inode_hashtable_t_find(u32_t __key, inode_hashtable_t* __hashtable)
{
	struct list_head* it = NULL;
	list_for_each(it,&__hashtable->i_list[__key % INODE_HASH_MAX].head)
	{
		efs2_inode_t* __inode = list_entry(it,efs2_inode_t,i_head);
		if(__inode->i_num == __key)
		{
			return __inode;
		}
	}	

	return NULL;
}

void dentry_hashtable_t_insert(struct dentry* __dentry, dentry_hashtable_t* __hashtable)
{
	int __key = dentry_hashtable_t_hash(__dentry->d_name) % DENTRY_HASH_MAX;
	list_insert(&__hashtable->d_list[__key],&__dentry->d_hhead);
}

void dentry_hashtable_t_remove(struct dentry* __dentry, dentry_hashtable_t* __hashtable)
{
	int __key = dentry_hashtable_t_hash(__dentry->d_name) % DENTRY_HASH_MAX;
	list_remove(&__hashtable->d_list[__key],&__dentry->d_hhead);
}

struct dentry* dentry_hashtable_t_find(struct dentry* __parent, u8_t* __name, dentry_hashtable_t* __hashtable)
{
	int __key = dentry_hashtable_t_hash(__name) % DENTRY_HASH_MAX;
	struct list_head* it = NULL;
	list_for_each(it,&__hashtable->d_list[__key].head)
	{
		struct dentry* __dentry = list_entry(it,struct dentry,d_hhead);
		if(!strcmp(__dentry->d_name,__name) && __dentry->parent == __parent)
			return __dentry;
	}

	return NULL;
}

int efs2_alloc_inum(efs2_data_t* volatile __data)
{
	struct list_head* it = NULL;
	list_for_each(it,&__data->superblock.s_i_cache.head)
	{
		struct efs_cache_t* __cache = list_entry(it,efs_cache_t,c_head);
		if(!(__cache->c_btmp[__data->superblock.s_blksize - 1] & 0x01))
		{
			for(int i = 0; i < __data->superblock.s_blksize; i++)
			{
				if(__cache->c_btmp[i] != 0xff)
				{
					for(int j = 0; j < 8; j++)
					{
						if(!(__cache->c_btmp[i] & (1 << (7-j))))
						{
							__cache->c_btmp[i] |= (1 << (7-j));

							int __inode = (i * 8 + j) + (__cache->c_index * 8 * __data->superblock.s_blksize); 
							if((i * 8 + j) == (8 * __data->superblock.s_blksize - 2))
								__cache->c_btmp[i] |= 0x01;
							__data->superblock.s_i_free--;
							return __inode;							
						}
					}
				}
			}
		}
	}

	if(__data->superblock.s_inum_cache < __data->superblock.s_inum_blk)
	{
		efs_cache_t* __cache = sys_alloc(sizeof(efs_cache_t));
		__cache->c_index = __data->superblock.s_inum_cache + 1;
		__cache->c_btmp = sys_alloc(__data->superblock.s_blksize);
		list_insert(&__data->superblock.s_i_cache,&__cache->c_head);
		__data->superblock.s_inum_cache++;
		__data->superblock.s_i_free--;
		__cache->c_btmp[0] |= (1 << 7);
		return (__cache->c_index * 8 * __data->superblock.s_blksize);
	}

	return -1;
}

void efs2_free_inum(efs2_data_t* volatile __data, u32_t __inum)
{
	u32_t __icache = __inum / (8 * __data->superblock.s_blksize);
	struct list_head* it = NULL;
	list_for_each(it,__data->superblock.s_i_cache.head)
	{
		efs_cache_t* __cache = list_entry(it,efs_cache_t,c_head);
		if(__cache->c_index == __icache)
		{
			u32_t __icache_off = __icache % (8 * __data->superblock.s_blksize);
			__cache->c_btmp[__icache_off / 8] &= ~(1 << (7 - (__icache_off % 8)));
			__data->superblock.s_i_free++;
		}
	}
}

int efs2_alloc_dnum(efs2_data_t* volatile __data)
{
	struct list_head* it = NULL;
	list_for_each(it,&__data->superblock.s_d_cache.head)
	{
		struct efs_cache_t* __cache = list_entry(it,efs_cache_t,c_head);
		if(!(__cache->c_btmp[__data->superblock.s_blksize - 1] & 0x01))
		{
			for(int i = 0; i < __data->superblock.s_blksize; i++)
			{
				if(__cache->c_btmp[i] != 0xff)
				{
					for(int j = 0; j < 8; j++)
					{
						if(!(__cache->c_btmp[i] & (1 << (7-j))))
						{
							__cache->c_btmp[i] |= (1 << (7-j));

							int __block = (i * 8 + j) + (__cache->c_index * 8 * __data->superblock.s_blksize); 
							if((i * 8 + j) == (8 * __data->superblock.s_blksize - 2))
								__cache->c_btmp[i] |= 0x01;
							__data->superblock.s_d_free--;
							return __block + 
							(__data->superblock.s_d_off + __data->superblock.s_d_blk) / __data->superblock.s_sec_per_blk;							
						}
					}
				}
			}
		}
	}

	if(__data->superblock.s_dnum_cache < __data->superblock.s_dnum_blk)
	{
		efs_cache_t* __cache = sys_alloc(sizeof(efs_cache_t));
		__cache->c_index = __data->superblock.s_dnum_cache + 1;
		__cache->c_btmp = sys_alloc(__data->superblock.s_blksize);
		list_insert(&__data->superblock.s_d_cache,&__cache->c_head);
		__data->superblock.s_dnum_cache++;
		__data->superblock.s_d_free--;
		__cache->c_btmp[0] |= (1 << 7);
		return (__cache->c_index * 8 * __data->superblock.s_blksize) + 
		(__data->superblock.s_d_off + __data->superblock.s_d_blk) / __data->superblock.s_sec_per_blk;
	}

	return -1;	
}

void efs2_free_dnum(efs2_data_t* volatile __data, u32_t __dnum)
{
	__dnum -= (__data->superblock.s_d_off + __data->superblock.s_d_blk) / __data->superblock.s_sec_per_blk;
	u32_t __dcache = __dnum / (8 * __data->superblock.s_blksize);
	struct list_head* it = NULL;
	list_for_each(it,__data->superblock.s_d_cache.head)
	{
		efs_cache_t* __cache = list_entry(it,efs_cache_t,c_head);
		if(__cache->c_index == __dcache)
		{
			u32_t __dcache_off = __dcache % (8 * __data->superblock.s_blksize);
			__cache->c_btmp[__dcache_off / 8] &= ~(1 << (7 - (__dcache_off % 8)));
			__data->superblock.s_d_free++;
		}
	}
}

void efs2_write_inode(efs2_data_t* volatile __data, efs2_inode_t* __inode)
{
	__data->dev->write(__data->superblock.s_i_off,__inode->i_num * __data->superblock.s_isize,__inode,__data->superblock.s_isize);
}

efs2_inode_t* efs2_read_inode(efs2_data_t* volatile __data, u32_t __inum)
{
	efs2_inode_t* __inode = sys_alloc(__data->superblock.s_isize);
	__data->dev->read(__data->superblock.s_i_off,__inum * __data->superblock.s_isize,__inode,__data->superblock.s_isize);
	return __inode;
}

void efs2_write_block(efs2_data_t* volatile __data, u32_t __block, void* __buf)
{
	__data->dev->write(block_to_lba(__data,__block),0,__buf,__data->superblock.s_blksize);
}

void efs2_read_block(efs2_data_t* volatile __data, u32_t __block, void* __buf)
{
	__data->dev->read(block_to_lba(__data,__block),0,__buf,__data->superblock.s_blksize);
}

int efs2_write(efs2_data_t* volatile __data, u32_t __fd, void* __buf, u32_t __bytes)
{
	
}

int efs2_read(efs2_data_t* volatile __data, u32_t __fd, void* __buf, u32_t __bytes)
{

}

int efs2_close(efs2_data_t* volatile __data, u32_t __fd)
{
	if(__fd == 0 || __fd == 1 || __fd == 2)
        return 1;

    int __exist = 0;

    for(int i = 3; i < max_file; i++)
    {
        if(current->fd[i] == __fd)
        {
            current->fd[i] = -1;
            __exist = 1;
            break;
        }
    }

    if(!__exist)
    	return -1;

	struct list_head* it = NULL;

	list_for_each(it,&__data->open_files.head)
	{
		efs2_file_t* __file = list_entry(it,efs2_file_t,f_head);
		if(__file->fd == __fd)
		{
			__file->opened = 0;
			list_remove(&__data->open_files,&__file->f_head);
			efs2_inode_t* __inode = __data->i_hash.find(__file->f_dentry->d_inode,&__data->i_hash);
			__inode->i_mode = __file->f_dentry->d_mode;
			efs2_write_inode(__data,__inode);
			sys_free(__file);

			return 1;
		}
	}

	return -1;
}

int efs2_open(efs2_data_t* volatile __data, u32_t __fd, u8_t __mode)
{
	struct list_head* it = NULL;
	list_for_each(it,__data->open_files.head)
	{
		efs2_file_t* __file = list_entry(it,efs2_file_t,f_head);
		if(__file->fd == __fd)
		{
			__file->f_dentry->d_mode |= (__mode & 0x0f);
			__file->opened = 1;
			__file->f_size = 0;

			if(IS_APPEND_O(__mode) && IS_WR_O(__mode))
			{
				efs2_inode_t* __inode = __data->i_hash.find(__file->f_dentry->d_inode,&__data->i_hash);
				if(!__inode)
					return -1;

				__file->f_size = __inode->i_size;
			}

			return 1;
		}
	}

	return -1;
}

int efs2_stat(efs2_data_t* volatile __data, u32_t __fd, struct stat* __stat)
{
	struct list_head* it = NULL;
	list_for_each(it,&__data->open_files.head)
	{
		efs2_file_t* __file = list_entry(it,efs2_file_t,f_head);
		if(__file->fd == __fd)
		{
			efs2_inode_t* __inode = __data->i_hash.find(__file->f_dentry->d_inode,&__data->i_hash);
			if(!__inode)
				return -1;

			__stat->st_dev = __inode->i_dev;
			__stat->st_inode = __inode->i_num;
			__stat->st_mode = __inode->i_mode;
			__stat->st_link = __inode->i_link;
			__stat->st_uid = __inode->i_uid;
			__stat->st_gid = __inode->i_gid;
			__stat->st_size = __inode->i_size;
			__stat->st_blksize = __data->superblock.s_blksize;
			__stat->st_ctime = __inode->i_ctime;
			__stat->st_atime = __inode->i_atime;
			
			return 1;
		}
	}

	return -1;
}

int efs2_eof(efs2_data_t* volatile __data, u32_t __fd)
{
	struct list_head* it = NULL;
	list_for_each(it,&__data->open_files.head)
	{
		efs2_file_t* __file = list_entry(it,efs2_file_t,f_head);
		if(__file->fd == __fd)
		{
			efs2_inode_t* __inode = __data->i_hash.find(__file->f_dentry->d_inode,&__data->i_hash);
			if(!__inode)
				return -1;

			return __file->f_size >= __inode->i_size;
		}
	}

	return -1;
}

int efs2_lseek(efs2_data_t* volatile __data, u32_t __fd, u32_t __off, u32_t __pos)
{
	struct list_head* it = NULL;
	list_for_each(it,&__data->open_files.head)
	{
		efs2_file_t* __file = list_entry(it,efs2_file_t,f_head);
		if(__file->fd == __fd)
		{
			switch(__pos)
			{
				case CURSOR_BEG_OFF:
				__file->f_size = __off;
				break;

				case CURSOR_CUR_OFF:
				__file->f_size += __off;
				break;

				case CURSOR_END_OFF:
				__file->f_size -= __off;
				break;

				default:
				break;
			}

			return 1;
		}
	}

	return -1;
}

int efs2_flush(efs2_data_t* volatile __data, u32_t __fd)
{
	if(__fd == 0)
		fifo_clean(stdin);
	else if(__fd == 1)
		fifo_clean(stdout);
	else if(__fd == 2)
		fifo_clean(stderr);
	else
		return -1;

	return 1;
}
