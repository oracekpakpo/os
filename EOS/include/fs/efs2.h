#ifndef __EFS2_H__
#define __EFS2_H__ 

#include <fs/dcache.h>
#include <fs/fs.h>
#include <device.h>
#include <time.h>
#include <mutex.h>
#include <list.h>
#include <ata.h>
#include <fifo.h>

#define INODE_HASH_MAX 50
#define DENTRY_HASH_MAX 50


typedef struct
{
	time_t s_ctime;
	time_t s_atime;
	u32_t s_blksize;
	u32_t s_sec_per_blk;
	u32_t s_isize;
	u32_t s_inum_off;
	u32_t s_inum_blk;
	u32_t s_inum_cache;
	u32_t s_i_off;
	u32_t s_i_blk;
	u32_t s_dnum_off;
	u32_t s_dnum_blk;
	u32_t s_dnum_cache;
	u32_t s_d_off;
	u32_t s_d_blk;
	u32_t s_i_free;
	u32_t s_d_free;
	u32_t s_version;
	u32_t s_stat;
	u32_t s_signature;
	struct dentry s_root;
	struct list s_i_cache;
	struct list s_d_cache;
}__attribute__((packed)) efs2_superblock_t;

typedef struct
{
	u32_t c_index;
	u8_t* c_btmp;
	struct list_head c_head;
} efs2_cache_t;


typedef struct
{
	time_t i_ctime;
	time_t i_atime;
	u8_t i_name[DNAME_MAX_LEN];
	u8_t i_mode;
	u32_t i_link;
	u32_t i_num;
	u32_t i_uid;
	u32_t i_gid;
	/*
	* 0 -- 3 --> direct block
	* 4 --> double indirect block
	*/
	int i_block[5];
	u32_t i_dev;
	u32_t i_size;
	struct list_head i_head;
} efs2_inode_t;

typedef struct
{
	struct list i_list[INODE_HASH_MAX];
	void (*insert)(efs2_inode_t*, inode_hashtable_t*);
	void (*remove)(efs2_inode_t*, inode_hashtable_t*);
	efs2_inode_t* (*find)(u32_t, inode_hashtable_t*);
} inode_hashtable_t;

void inode_hashtable_t_insert(efs2_inode_t* __inode, inode_hashtable_t* __hashtable);
void inode_hashtable_t_remove(efs2_inode_t* __inode, inode_hashtable_t* __hashtable);
efs2_inode_t* inode_hashtable_t_find(u32_t __key, inode_hashtable_t* __hashtable);

static inline void init_inode_hashtable_t(inode_hashtable_t* __hashtable)
{
	for(int i = 0; i < INODE_HASH_MAX; i++)
		init_list(&__hashtable->i_list[i]);
	__hashtable->insert = inode_hashtable_t_insert;
	__hashtable->remove = inode_hashtable_t_remove;
	__hashtable->find = inode_hashtable_t_find;
}

typedef struct
{
	struct list d_list[DENTRY_HASH_MAX];
	void (*insert)(struct dentry*, dentry_hashtable_t*);
	void (*remove)(struct dentry*, dentry_hashtable_t*);
	struct dentry* (*find)(struct dentry*, u8_t*, dentry_hashtable_t*);
	u32_t (*hash)(u8_t*);
} dentry_hashtable_t;

void dentry_hashtable_t_insert(struct dentry* __dentry, dentry_hashtable_t* __hashtable);
void dentry_hashtable_t_remove(struct dentry* __dentry, dentry_hashtable_t* __hashtable);
struct dentry* dentry_hashtable_t_find(struct dentry* __parent, u8_t* __name, dentry_hashtable_t* __hashtable);

static inline u32_t dentry_hashtable_t_hash(u8_t* __name)
{
	int __key = 0;

	for(int i = 0; __name[i] != '\0'; i++)
		__key += __name[i];
	
	return __key;
}

static inline void init_dentry_hashtable_t(dentry_hashtable_t* __hashtable)
{
	for(int i = 0; i < DENTRY_HASH_MAX; i++)
		init_list(&__hashtable->d_list[i]);
	__hashtable->insert = dentry_hashtable_t_insert;
	__hashtable->remove = dentry_hashtable_t_remove;
	__hashtable->find = dentry_hashtable_t_find;
	__hashtable->hash = dentry_hashtable_t_hash;
}

typedef struct
{
	inode_hashtable_t i_hash;
	dentry_hashtable_t d_hash;
	struct dcache* dcache;
	struct dcache_manager* dcache_manager;
	dev_t* dev;
	struct dentry* root;
	efs2_superblock_t superblock;
	struct mutex mutex;	
	struct list open_files;
} efs2_data_t;

typedef struct
{
	struct dentry* f_dentry;
	u32_t fd;
	u32_t opened;
	u32_t f_size;
	struct list_head f_head;
} efs2_file_t;

int efs2_alloc_inum(efs2_data_t* volatile __data);
void efs2_free_inum(efs2_data_t* volatile __data, u32_t __inum);
int efs2_alloc_dnum(efs2_data_t* volatile __data);
void efs2_free_dnum(efs2_data_t* volatile __data, u32_t __dnum);
void efs2_write_inode(efs2_data_t* volatile __data, efs2_inode_t* __inode);
efs2_inode_t* efs2_read_inode(efs2_data_t* volatile __data, u32_t __inum);
void efs2_write_block(efs2_data_t* volatile __data, u32_t __block, void* __buf);
void efs2_read_block(efs2_data_t* volatile __data, u32_t __block, void* __buf);
int efs2_write(efs2_data_t* volatile __data, u32_t __fd, void* __buf, u32_t __bytes);
int efs2_read(efs2_data_t* volatile __data, u32_t __fd, void* __buf, u32_t __bytes);
int efs2_close(efs2_data_t* volatile __data, u32_t __fd);
int efs2_open(efs2_data_t* volatile __data, u32_t __fd, u8_t __mode);
int efs2_stat(efs2_data_t* volatile __data, u32_t __fd, struct stat* __stat);
int efs2_eof(efs2_data_t* volatile __data, u32_t __fd);
int efs2_lseek(efs2_data_t* volatile __data, u32_t __fd, u32_t __off, u32_t __pos);
int efs2_flush(efs2_data_t* volatile __data, u32_t __fd);

static inline u32_t lba_to_block(efs2_data_t* volatile __data, u32_t __lba)
{
	u32_t __bnum = __lba / __data->superblock.s_sec_per_blk;
	if(!(__lba % __data->superblock.s_sec_per_blk))
		__bnum--;

	return __bnum;
}

static inline u32_t block_to_lba(efs2_data_t* volatile __data, u32_t __block)
{
	return (1 + __block * __data->superblock.s_sec_per_blk);
}

#endif //__EFS2_H__