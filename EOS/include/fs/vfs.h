#ifndef __VFS_H__
#define __VFS_H__

#include <types.h>
#include <device.h>
#include <list.h>
#include <fs/vfs_tool.h>
#include <string.h>

#define RD_FS 0x01
#define WR_FS 0x02
#define RD_WR_FS 0x03

struct device;
// @ == parameter
struct file_system
{
	char* name;
	u8 flag;
	/* probe
	params: @1
	return: explore device file_system
	*/
	int (*probe)(struct device*);	
	/* create
	params: @1 is file name
	@2 is permission
	@3 is the device
	@4 is device file_system private data
	return: create an ordinary file with default mode(READ,WRITE)
	and permission(USER)
	*/
	int (*create)(char*, u8, struct device*, void*);
	/* write
	params: @1 file descriptor
	@2 is the buffer
	@3 is the buffer size in bytes
	@4 is device
	@5 is device file_system private data
	return: write inside file the buffer
	*/
	int (*write)(int, void*, u32, struct device*, void*);
	/* read
	params: @1 file descriptor
	@2 is the buffer
	@3 is the buffer size in bytes
	@4 is device
	@5 is device file_system private data
	return: read from file inside the buffer
	*/
	int (*read)(int, void*, u32, struct device*, void*);
	/* exist
	params: @1 is file name
	@2 is device
	@3 is device file_system private data
	return: find the name
	*/
	int (*exist)(char*, struct device*, void*);
	/* mount
	params: @1 is device
	@2 is mounting point 
	@ is device file_system private data
	return: mount device on mounting point
	*/
	int (*mount)(struct device*, char*, void*);
	/* umount
	params: @1 is device
	@2 is mounting point 
	@ is device file_system private data
	return: mount device on mounting point
	*/
	int (*umount)(struct device*, char*, void*);	
	/* mkdir
	params: @1 is directory name
	@2 is directory mode
	@3 is device
	@4 device file_system private data
	*/
	int (*mkdir)(char*, u8, struct device*, void*);
	/* rndir
	params: @1 file name
	@2 is the new name (basename)
	@3 is device
	@4 is device file_system private data
	return: rename directory
	*/
	int (*rndir)(char*, char*, struct device*, void*);	
	/* rmdir
	params: @1 file name
	@2 is device
	@3 is device file_system private data
	return: remove directory
	*/
	int (*rmdir)(char*, struct device*, void*);	
	/* chdir
	params: @1 file name
	@2 is device
	@3 is device file_system private data
	return: change current directory
	*/
	int (*chdir)(char*, struct device*, void);	
	/* chmod
	params: @1 file name
	@2 mode (READ-WRITE-EXEC)
	@3 is device
	@4 is device file_system private data
	return: change file mode
	*/
	int (*chmod)(char*, u8, struct device*, void*);	
	/* chown
	params: @1 file name
	@2 owner bytes
	@3 is device
	@4 is device file_system private data
	return: change file owner
	*/
	int (*chown)(char*, u8, struct device*, void*);
	/* link
	params: @1 --> old_path
	@2 --> new_path
	@3 --> device
	@4 --> file system private data
	return: link @1 at @2
	*/
	int (*link)(char*, char*, struct device*, void*);
	/* unlink
	params: @1 --> link file path
	@2 --> device
	@3 --> file system private data
	return: unlink @1
	*/
	int (*unlink)(char*, struct device*, void*);
	/* open
	params: @1 file decriptor
	@2 is mode
	@3 is device
	@4 is device file_system private data
	return: open an existing file
	*/
	int (*open)(int, u8, struct device*, void*);	
	/* close
	params: @1 file descriptor
	@2 is device
	@3 is device file_system private data
	return: close an existing file
	*/
	int (*close)(int, struct device*, void*);
	/* eof
	params:
	@1 file desciptor
	@2 device
	@ file system private data
	return:
	return 1 if EOF 0 otherwise
	*/
	int (*eof)(int, struct device*, void*);
};

struct mount_info
{
	char location[PATH_MAX_LEN]; // (absolute path location of device mounting point)
	int len;
	struct device* dev;
	struct list_head head;
};

struct descriptor
{
	int fd;
	struct device* dev;
	struct list_head head;
};

struct vfs_data
{
	struct list mount_list;
	struct list descriptor_list;
	struct descriptor* des;
};

struct vfs_operation
{
	//Basic VFS functions
	void (*add_mount_point)(struct mount_info* _minfo);
	void (*rm_mount_point)(struct mount_info* _minfo);
	struct mount_info* (*dev_mount_point)(struct device*);
	struct mount_info* (*get_path_file_system)(char*);
	struct descriptor* (*get_descriptor_file_system)(int);
	//VFS interface

	int (*vfs_create)(char*, u8);
	int (*vfs_write)(int, void*, u32);
	int (*vfs_read)(int, void*, u32);
	int (*vfs_exist)(char*);
	int (*vfs_mount)(char*, char*, u8);
	int (*vfs_umount)(char*);
	int (*vfs_mkdir)(char*, u8);
	int (*vfs_rndir)(char*, char*);
	int (*vfs_rmdir)(char*);
	int (*vfs_chdir)(char*);
	int (*vfs_chmod)(char*, u8);
	int (*vfs_chown)(char*, u8);
	int (*vfs_link)(char*, char*);
	int (*vfs_unlink)(char*);
	int (*vfs_open)(int, u8);
	int (*vfs_close)(int);
	int (*vfs_eof)(int);
	int (*vfs_probe)(struct device*);
};

struct vfs
{
	struct vfs_data __data__;
	struct vfs_operation __operation__;
};

void add_mount_point(struct mount_info*);
void rm_mount_point(struct mount_info*);
struct mount_info* dev_mount_point(struct device*);
struct mount_info* get_path_file_system(char*);

int vfs_create(char*, u8);
int vfs_write(int, void*, u32);
int vfs_read(int, void*, u32);
int vfs_exist(char*);
int vfs_mount(char*, char*, u8);
int vfs_umount(char*);
int vfs_mkdir(char*, u8);
int vfs_rndir(char*, char*);
int vfs_rmdir(char*);
int vfs_chdir(char*);
int vfs_chmod(char*, u8);
int vfs_chown(char*, u8);
int vfs_open(int, u8);
int vfs_close(int);
int vfs_eof(int);
int vfs_link(char*, char*);
int vfs_unlink(char*);
int vfs_probe(struct device*);

void init_vfs();

#endif //__VFS_H__
k