#ifndef _EFS_H__
#define _EFS_H__

#include <types.h>
#include <time.h>
#include <memory.h>
#include <lib.h>
#include <list.h>
#include <string.h>
#include <io.h>
#include <ata.h>
#include <fs/vfs.h>
#include <mutex.h>

#define blk_size 0x1000
#define INODE_BITMAP_BLOCK_COUNT 5
#define DI_BLOCK 4 // 4 indirect block

/* Mode constant */
#define EFS_READ_FLAG 0x01
#define EFS_WRITE_FLAG 0x02
#define EFS_EXEC_FLAG 0x04
#define EFS_APPEND_FLAG 0x07

/* types constant */
#define EFS_FILE_FLAG 0x1f
#define EFS_DIRECTORY_FLAG 0x3f
#define EFS_LINK_FLAG 0x5f
#define EFS_CHARACTER_DEV_FLAG 0x7f
#define EFS_BLOCK_DEV_FLAG 0x9f
#define EFS_SIGNATURE 0xEF50

/* Seek constant */
#define FILE_BEG 0
#define FILE_CUR 1
#define FILE_END 2

/* Dentry state */
#define EFS_VALID_DENTRY 0x20
#define EFS_INVALID_DENTRY 0x30

typedef struct
{
    u32 iblock[DI_BLOCK]; // Curosr hardwaried to DI_BLOCK double indirect block
}f_cursor;

typedef struct mode
{
    unsigned  r:1;
    unsigned  w:1;
    unsigned  x:1;
    unsigned  :2;
    unsigned  type:3;
}__attribute__((packed)) mode;


typedef struct
{
    /* Data */
    u32 data_block_count;
    u32 free_data_block_count;
    u32 block_size;
    u32 first_data_block;

    /* Inode */
    u32 inode_block_count;
    u32 inode_size;
    u32 inode_count;
    u32 inode_per_block:
    u32 first_inode_block;

    /* Control */
    u32 data_bitmap_block_count;
    u32 first_data_bitmap_block;
    u32 current_data_bitmap_block_end;
    u32 inode_bitmap_block_count;
    u32 first_inode_bitmap_block;
    u32 current_inode_bitmap_block_end;
    /* Disk info */
    sys_time first_mounting;
    sys_time last_mounting;
    u32 version;
    u32 signature;
    u32 error;
    u32 state;
    uchar path_mounted[360];
}__attribute__((packed)) efs_super_block; // super block size = 512

typedef f_cursor f_size;

typedef struct
{
    u32 user_id;
    u32 group_id;
    f_size size; // DI_BLOCK*sizeof(u32) bytes
    u32 parent;
    int block_num[DI_BLOCK]; // DI_BLOCK indirect block
    mode _mode; // 1 byte
    u32 link_count;
    uchar path[64];
    sys_time created_time;
    sys_time last_access_time; //
}__attribute__((packed)) efs_inode; // inode size=256


typedef struct cache
{
    u8 fully;
    u8 bitmap[blk_size];
    struct list_head head;
}__attribute__((packed)) cache;


typedef struct
{
    efs_super_block* sb;
    struct list_head inode_bitmap_cache;
    struct list_head data_bitmap_cache;
    u32 disk_size;
    u32 partition;
    ata* ata_device;
}__attribute__((packed)) disk;

typedef struct efs_dentry
{
    efs_inode* inode;
    uchar name[64];
    u8 state;
    struct efs_dentry* parent;
    struct list_head head;
    u32 inode_number;
    u32 parent_number;
    mode flags;
    struct list childs;
} efs_dentry;

typedef struct efs_file
{
    int fd; // file descriptor
    int n_proc; // how many process use file
    f_cursor cursor;
    efs_dentry* dentry;
    struct list_head head; // warning:used to find appropriate file with fd from vfs
}efs_file;


typedef struct efs_priv_data
{
    struct list __files;
    struct efs_dentry root;
    efs_super_block __sb;
    disk __disk;
    struct mutex mutex;
    int block_size;
    int blocks;
    int lba; // starting lba number
}efs_priv_data;

/* efs low level functions */
u32 translate_block_to_lba(u32 block_num);
u32 translate_lba_to_block(u32 lba);
u32 efs_read_disk_size(disk* hd);
void efs_set_block_used(disk* hd, u32 blknum);
void efs_release_block(disk* hd, u32 blknum);
int efs_get_block(disk* hd);
void efs_set_inode_used(disk* hd, u32 inode_number);
void efs_release_inode(disk* hd, u32 inode_number);
int efs_get_inode_number(disk* hd);
efs_inode* efs_read_inode(disk* hd, efs_inode* inode, u32 inode_number);
void efs_write_inode(disk* hd, efs_inode* inode, u32 inode_number);
u32 efs_write_block(u32 block, u32 offset, void* buffer);
u32 efs_read_block(u32 block, u32 offset, void* buffer);
void init_efs_priv_data(efs_priv_data*, u32, u32);
int load_efs(efs_dentry*, disk*);
int save_efs(efs_dentry*, disk*);

/* efs_dentry managment low level functions */
//Warning: these functions take a clean path 
void efs_create_path(efs_priv_data*, char*, u8);
efs_dentry* efs_lookup_path(efs_priv_data*, char*);
efs_entry* efs_create_dentry_inside(efs_priv_data*, efs_dentry*, uchar*, u8, u8);
efs_dentry* efs_lookup_dentry(efs_dentry* , uchar*, u8);
efs_dentry* efs_lookup(efs_priv_data*, char*);
void efs_init_root_dentry(efs_dentry*, efs_priv_data*);

void efs_set_dentry_mode(efs_dentry*, u8);
u8 efs_get_dentry_mode(efs_dentry*);
efs_inode* efs_get_dentry_inode(efs_dentry*);
void efs_set_dentry_inode(efs_dentry*, efs_inode*);
void efs_init_dentry(efs_dentry*);
void efs_set_dentry_parent(efs_dentry*, efs_dentry*);
efs_dentry* efs_get_dentry_parent(efs_dentry*);
void efs_add_dentry_to(efs_dentry*, efs_dentry*);
void efs_set_dentry_state(efs_dentry* , u8);
u8 efs_get_dentry_state(efs_dentry*);
void efs_set_dentry_name(efs_dentry* , uchar*);
uchar* efs_get_dentry_name(efs_dentry*);
void efs_check_dentry(efs_dentry*);
void efs_sync_dentry(efs_dentry*);
int efs_scan_disk(disk*, efs_dentry*);

/* efs_file managment functions*/
int efs_probe(struct device*);
int efs_create(char*, u8, struct device*, void*);
int efs_open(int, u8, struct device*, void*);
int efs_close(int, struct device*, void*);
int efs_write(int, void*, u32, struct device*, void*);
int efs_read(int, void*, u32, struct device*,void*);
int efs_lseek(int, u32, u8, struct device*, void*);
int efs_eof(int, struct device*, void*);
void efs_flush(int, struct device*, void*);
int efs_link(char*, char*, struct device*, void*);
int efs_unlink(char*, struct device*, void*);
int efs_exist(char*, struct device*, void*);

/* efs_dentry managment high level functions */
int efs_mkdir(char*, u8, struct device*, void*);
int efs_rmdir(char*, struct device*,void*);
int efs_rndir(char*, char* ,struct device*, void*);
int efs_chdir(char*, struct device*, void*);
int efs_chmod(char*, u8, struct device*, void*);
int efs_chown(char*, u8, struct device*, void*);
int efs_mount(struct device*, char*, void*);
int efs_umount(struct device*, char*, void*);

int make_efs(struct device*, efs_priv_data*);

static inline int get_current_cursor(f_cursor* cursor)
{
    int i = 0;
    for(i = 0; i < DI_BLOCK && cursor->iblock[i] == MAX_LONG; i++);
    return i;
}

static inline void clean_cursor(f_cursor* cursor)
{
    memset(cursor,0,sizeof(f_cursor));
}

static inline void move_file_cursor(f_cursor* cursor, u32 bytes)
{
    int i = get_current_cursor(cursor);
    u32 j = MAX_LONG - cursor->iblock[i];
    
    if(j >= bytes)
    {
        cursor->iblock[i] += bytes;
    }
    else
    {
        cursor->iblock[i] += j;
        if((i + 1) < DI_BLOCK)
        {
            cursor->iblock[i+1] += bytes-j;
        }
    }
}


#endif // _EFS_H__
