#ifndef __FS_H__
#define __FS_H__
#include <list.h>
#include <time.h>

#define DNAME_MAX_LEN  32
#define READ_O         0x01
#define WRITE_O        0x02
#define READ_WRITE     0x03
#define APPEND_0	   0x04
#define ORDINARY_FILE  0x10
#define DIRECTORY_FILE 0x20
#define PIPE_FILE      0x30
#define SOCKET_FILE	   0x40 
#define DEVICE_FILE    0x50
#define LINK_FILE	   0x60

#define IS_WR_O(mode) mode & WRITE_O
#define IS_RD_O(mode) mode & READ_O
#define IS_RD_WR(mode) mode & READ_WRITE
#define IS_APPEND_O(mode) mode & APPEND_0
#define IS_ORDINARY_FILE(mode) mode & ORDINARY_FILE
#define IS_DIRECTORY_FILE(mode) mode & DIRECTORY_FILE
#define IS_PIPE_FILE(mode) mode & PIPE_FILE
#define IS_SOCKET_FILE(mode) mode & SOCKET_FILE
#define IS_DEVICE_FILE(mode) mode & DEVICE_FILE
#define IS_LINK_FILE(mode) mode & LINK_FILE

#define CURSOR_BEG_OFF 0x0
#define CURSOR_CUR_OFF 0x1
#define CURSOR_END_OFF 0x2

struct partition {
	u8_t bootable;		/* 0 = no, 0x80 = bootable */
	u8_t s_head;		/* Starting head */
	u16_t s_sector:6;		/* Starting sector */
	u16_t s_cyl:10;		/* Starting cylinder */
	u8_t id;			/* System ID */
	u8_t e_head;		/* Ending Head */
	u16_t e_sector:6;		/* Ending Sector */
	u16_t e_cyl:10;		/* Ending Cylinder */
	u32_t s_lba;		/* Starting LBA value */
	u32_t size;		/* Total Sectors in partition */
} __attribute__ ((packed));

struct stat
{
	u32_t st_dev;
	u32_t st_inode;
	u8_t st_mode;
	u32_t st_link;
	u32_t st_uid;
	u32 st_gid;
	u32_t st_size;
	u32_t st_blksize;
	time_t st_ctime; // created time
	time_t st_atime; // last acess time
};

struct dentry
{
	u8_t d_mode; // right + type
	u8_t d_name[DNAME_MAX_LEN];
	u32_t d_inode;
	struct dentry* parent;
	struct dentry* itself;
	struct list d_childs;
	struct list_head d_head;
	struct list_head d_chead; // dcache head
	struct list_head d_hhead; // hash table head
	struct list_head d_fhead; // open file head
};

#endif //__FS_H__