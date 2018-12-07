#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <fs/fs.h>
#include <types.h>
#include <list.h>

#define FREE_DEVICE   0x00
#define BUSY_DEVICE   0x01
#define DIRTY_DEVICE  0x02
#define UNKNOW_DEVICE 0x00
#define CHAR_DEVICE   0x01
#define BLOCK_DEVICE  0x02

#define IS_FREE_DEVICE(dev)  (dev)->dev_state & FREE_DEVICE
#define IS_BUSY_DEVICE(dev)  (dev)->dev_state & BUSY_DEVICE
#define IS_DIRTY_DEVICE(dev) (dev)->dev_state & DIRTY_DEVICE
#define IS_CHAR_DEVICE(dev)  (dev)->dev_type & CHAR_DEVICE
#define IS_BLOCK_DEVICE(dev) (dev)->dev_type & BLOCK_DEVICE
#define IS_UNKNOW_DEVICE(dev) (dev)->dev_type & UNKNOW_DEVICE

struct file_system_type
{
	u8_t* fs_name;  // file system name
	u8_t  fs_mode;  // file system mode
	u8_t  fs_mount; // file system mount 
					// status
};

struct file_system_operation
{

	int (*create)(void*, u8_t*, u8_t);        // @1 --> file path
											   // @2 --> file permission
	
	int (*write)(void*, u32_t, void*, u32_t); // @1 --> file descriptor
											   // @2 --> the out 
											   // @3 --> size
	
	int (*read)(void*, u32_t, void*, u32_t);  // @1 --> file descriptor
											   // @2 --> the out 
											   // @3 --> size
	
	int (*exist)(void*, u8_t*);               // @1 --> file path
	
	int (*mount)(void*, u8_t* , u8_t*);       // @1 --> device path
								    		   // @2 --> mounting point	

	int (*umount)(void*, u8_t*, u8_t*);       // @1 --> device path
								    		   // @2 --> mounting point  	

	int (*mkdir)(void*, u8_t*, u8_t);         // @1 --> dentry path
									           // @2 --> dentry mode
	
	int (*rndir)(void*, u8_t*, u8_t*);        // @1 --> file path
										       // @2 --> file base name	
	
	int (*rmdir)(void*, u8_t*);               // @1 --> dentry path

	int (*chdir)(void*, u8_t*);               // @1 --> dentry path

	int (*chmod)(void*, u8_t*, u8_t);         // @1 --> file path
									           // @2 --> file mode

	int (*chown)(void*, u8_t*, u8_t);         // @1 --> file path
									           // @2 --> file permission

	int (*link)(void*, u8_t*, u8_t*, u8_t);   // @1 --> file path
											   // @2 --> link path
											   // @3 --> link option 
	
	int (*unlink)(void*, u8_t*);              // @1 --> link path

	int (*open)(void*, u32_t, u8_t);          // @1 --> file descriptor
									           // @2 --> file mode


	int (*close)(void*, u32_t);               // @1 --> file descriptor

	int (*eof)(void*, u32_t);                 // @1 --> file descriptor

	int (*stat)(void*, u32_t, struct stat*);  // @1 --> file descriptor
											   // @2 --> file stat
	
	int (*seek)(void*, u32_t, u32_t, u32_t); // @1 --> file descriptor
											 // @2 --> offset
											 // @3 --> reference position	
	
	int (*flush)(void*, u32_t); 			 // @1 --> file descriptor

};

typedef struct
{
	struct file_system_type fs_type;
	struct file_system_operation fs_ops;
	struct list_head fs_head;
}fs_t;


typedef struct
{
	// read - write drivers
	u32_t (*read)(u32_t __pos, u32_t __off, void* __buf, u32_t __bytes);
	u32_t (*write)(u32_t __pos, u32_t __off, void* __buf, u32_t __bytes);
	u32_t dev_flba; // device first lba offset
	u32_t dev_llba; // device last lba offset
	u32_t dev_id; // device id
	u8_t dev_name[DNAME_MAX_LEN]; // device name
	u32_t dev_state; // device state
	u32_t dev_type; // device type
	struct dentry* dev_dentry; // device mount dentry
	fs_t dev_fs;
	struct list_head dev_head;
}dev_t;

#define DEV_HASH_MAX 5

typedef struct
{
	struct list dev_list[DEV_HASH_MAX];
	u32_t devices;
	void (*insert)(dev_t* __dev, dev_manager_t* __dev_manager);
	void (*remove)(u8_t* __dev_name, dev_manager_t* __dev_manager);
	dev_t* (*find)(u8_t* __dev_name, dev_manager_t* __dev_manager);
	void (*rename)(dev_t* __dev, u8_t* __name, dev_manager_t __dev_manager);
	u32_t (*hash)(u8_t* __dev_name);
	u32_t (*hash_index)(u32_t __key);
}dev_manager_t;

void dev_insert(dev_t*, dev_manager_t*);
void dev_remove(u8_t*, dev_manager_t*);
dev_t* dev_find(u8_t*, dev_manager_t*);
u32_t dev_hash(u8_t*);
void dev_rename(dev_t*, u8_t*, dev_manager_t*);


static inline u32_t dev_hash_index(u32_t __key) // generate a index from a key for dev_list
{
	return __key % DEV_HASH_MAX;
}

static inline void init_dev_manager(dev_manager_t* __dev_manager)
{
	for(int i = 0; i < DEV_HASH_MAX; i++)
		init_list(&__dev_manager->dev_list[i]);

	__dev_manager->devices = 0;
	__dev_manager->insert = dev_insert;
	__dev_manager->remove = dev_remove;
	__dev_manager->find = dev_find;
	__dev_manager->hash = dev_hash;
	__dev_manager->hash_index = dev_hash_index;
	__dev_manager->rename = dev_rename;
}


#endif //__DEVICE_H__