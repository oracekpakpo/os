#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <types.h>
#include <fs/vfs.h>
#include <memory.h>
#include <io.h>

#define DEV_BUSY 0x01
#define DEV_FREE 0x00
#define DEV_BROKE 0xff
#define DEV_MAX_NAME 64

struct file_system;

typedef enum device_type
{
	UNKNOWN_DEVICE = 0,
	CHAR_DEVICE = 1,
	BLOCK_DEVICE = 2,
};

struct device
{
	char name[DEV_MAX_NAME];
	int id;
	int state;
	device_type dev_type;
	struct file_system* fs;
	void* private_data;
	void* dev_file; // EFS dev file	

	/*read device driver
	params: lba is data location on device
	offset is the offset from lba
	buffer is data buffer
	bytes is the buffer size in bytes
	return: return the byte count readed
	*/
	u32 (*read)(u32 lba, u32 offset, void* buffer, u32 bytes);

	/*write device driver
	params: lba is data location on device
	offset is the offset from lba
	buffer is data buffer
	bytes is the buffer size in bytes
	return: return the byte count written
	*/	 
	u32 (*write)(u32 lba, u32 offset, void* buffer, u32 bytes);

};

/* Devices manager structure */
extern struct sys_devices
{
	/* __devices__ is a pointer on system devices
	*/
	struct device* __devices__[30];

	/* Devices manager operations
	*/

	void (*device_add)(struct device*); // Registering a device
	struct device* (*device_get)(int); // find a device by id
	void (*device_remove)(int);	// find a device by id and remove it from list
	void (*device_list)(void); // print all devices 
}__sys_devices;

static inline void device_add(struct device* __dev)
{
	for(int i = 0; i < 30; i++)
	{
		if(!__sys_devices.__devices__[i])
		{
			__sys_devices.__devices__[i] = __dev;
			__sys_devices.__devices__[i]->id = i;
			break;
		}
	}
}

static inline void device_remove(int id)
{
	for(int i = 0; i < 30; i++)
	{
		if(__sys_devices.__devices__[i]->id == id)
		{
			sys_free(__sys_devices.__devices__[i]);
			break;
		}
		
	}
}

static inline struct device* device_get(int id)
{
	for(int i = 0; i < 30; i++)
	{
		if(__sys_devices.__devices__[i]->id == id)
			return __sys_devices.__devices__[i];
	}

	return NULL;
}

static inline void device_list(void)
{
	for(int i = 0; i < 30; i++)
	{
		if(__sys_devices.__devices__[i])
		{
			kprintf("%s %d %s %s\n",__sys_devices.__devices__[i]->name,
				__sys_devices.__devices__[i]->id, __sys_devices.__devices__[i]->dev_type == CHAR_DEVICE ? "char device":"block device",
				__sys_devices.__devices__[i]->fs->name);
		}
	}
}

static inline void init_sys_devices(void)
{
	kprintf("Installing EOS devices manager ");
	
	for(int i = 0; i < 30; i++)
		__sys_devices.__devices__[i] = NULL;

	__sys_devices.device_add = device_add;
	__sys_devices.device_remove = device_remove;
	__sys_devices.device_get = device_get;
	__sys_devices.device_list = device_list;

	kprintf("[ok]\n");
}


#endif //__DEVICE_H__