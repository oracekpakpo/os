#include <fs/vfs.h>

// VFS contexte
struct vfs _vfs;

void init_vfs()
{
	_vfs.__operation__.vfs_create = vfs_create;
	_vfs.__operation__.vfs_write = vfs_write;
	_vfs.__operation__.vfs_read = vfs_read;
	_vfs.__operation__.vfs_exist = vfs_exist;
	_vfs.__operation__.vfs_mount = vfs_mount;
	_vfs.__operation__.vfs_umount = vfs_umount;
	_vfs.__operation__.vfs_mkdir = vfs_mkdir;
	_vfs.__operation__.vfs_rndir = vfs_rndir;
	_vfs.__operation__.vfs_rmdir = vfs_rmdir;
	_vfs.__operation__.vfs_chdir = vfs_chdir;
	_vfs.__operation__.vfs_chmod = vfs_chmod;
	_vfs.__operation__.vfs_chown = vfs_chown;
	_vfs.__operation__.vfs_link = vfs_link;
	_vfs.__operation__.vfs_unlink = vfs_unlink;
	_vfs.__operation__.vfs_open = vfs_open;
	_vfs.__operation__.vfs_close = vfs_close;
	_vfs.__operation__.vfs_eof = vfs_eof;
	_vfs.__operation__.vfs_probe = vfs_probe;

	_vfs.__operation__.add_mount_point = add_mount_point;
	_vfs.__operation__.rm_mount_point = rm_mount_point;
	_vfs.__operation__.dev_mount_point = dev_mount_point;
	_vfs.__operation__.get_path_file_system = get_path_file_system;
	_vfs.__operation__.get_descriptor_file_system = get_descriptor_file_system;

	init_list(&_vfs.__data__.mount_list);
	init_list(&_vfs.__data__.descriptor_list);
}

void add_mount_point(struct mount_info* minfo)
{
	if(minfo)
	{
		list_insert(&_vfs.__data__.mount_list,&minfo->head);
	}
}

void rm_mount_point(struct mount_info* minfo)
{
	if(minfo)
	{
		struct list_head* it = NULL;
		list_for_each(it,&_vfs.__data__.mount_list.head)
		{
			if(minfo == list_entry(it,struct mount_info, head))
			{
				list_del(&minfo->head);
				sys_free(minfo);
				return ;
			}
		}
	}
}

struct mount_info* dev_mount_point(struct device* dev)
{
	struct list_head* it = NULL;
	list_for_each(it,&_vfs.__data__.mount_list.head)
	{
		struct mount_info* minfo = list_entry(it,struct mount_info,head);
		if(minfo->__dev == dev)
			return minfo;	
	}

	return NULL;
}

struct mount_info* get_path_file_system(char* path)
{
	if(!path_clean(path))
	{
		kprintf("Bad path\n");
		return NULL;
	}

	if(list_empty(&_vfs.__data__.mount_list.head))
	{
		kprintf("No mount device\n");
		return NULL;
	}

	struct mount_info* minfo = list_first_entry(&_vfs.__data__.mount_list.head,struct mount_info,head);
	int max = minfo->len;

	struct list_head* it = NULL;
	list_for_each(it,&_vfs.__data__.mount_list.head)
	{
		struct mount_info* _minfo = list_entry(it,struct mount_info,head);
		if(strncmp(_minfo->location,path,_minfo->len) == 0 && _minfo->len > max)
		{
			max = _minfo->len;
			minfo = _minfo;
		}
	}
	path = path + minfo->len;
	if(path[0] == '/')
		path += 1;
	
	return minfo;
}


struct descriptor* get_descriptor_file_system(int fd)
{
	struct list_head* it = NULL;
	list_for_each(it,&_vfs.__data__.descriptor_list.head)
	{
		struct descriptor* des = list_entry(it,struct descriptor,head);
		if(des->fd == fd)
			return des;
	}
	return NULL;
}

int vfs_probe(struct device* dev)
{
	return dev->fs->probe(dev);
}

int vfs_create(char* _path, u8 flag)
{

	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);

	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_create failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_create failed!(read only file system)\n");
		return -1
	}

	
	int fd = minfo->dev->fs->create(path,flag,minfo->dev,
		minfo->dev->private_data);

	struct descriptor* des = sys_alloc(sizeof(struct descriptor));
	des->fd = fd;
	des->dev = minfo->dev;
	list_insert(&_vfs.__data__.descriptor_list,&des->head);
	_vfs.__data__.des = des;

	return fd;
}


int vfs_write(int fd, void* buffer, u32 bytes)
{
	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_write failed!(read only file system)\n");
		return -1
	}

	if(_vfs.__data__.des->fd != fd)
	{
		_vfs.__data__.des = get_descriptor_file_system(fd);
		
		if(!_vfs.__data__.des)
		{
			kprintf("Bad file descriptor\n");
			return -1;
		}
		
	}

	return _vfs.__data__.des->dev->fs->write(fd,buffer,bytes,_vfs.__data__.des->dev,
		_vfs.__data__.des->dev->private_data);

}

int vfs_read(int fd, void* buffer, u32 bytes)
{
	if(!(_vfs.__data__.__fs->flag & RD_FS))
	{
		kprintf("vfs_read failed!(write only file system)\n");
		return -1;
	}

	if(_vfs.__data__.des->fd != fd)
	{
		_vfs.__data__.des = get_descriptor_file_system(fd);
		
		if(!_vfs.__data__.des)
		{
			kprintf("Bad file descriptor\n");
			return -1;
		}
		
	}

	return _vfs.__data__.des->dev->fs->read(fd,buffer,bytes,_vfs.__data__.des->dev,
		_vfs.__data__.des->dev->private_data);

}

int vfs_exist(char* _path)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);

	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_exist failed!(file system not found)\n");
		return -1;
	}

	return minfo->dev->fs->exist(path,minfo->dev,
		minfo->dev->private_data);
}

int vfs_mount(char* dev, char* _path, u8 flag)
{
	
}

int vfs_umount(char* _path)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);

	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_umount failed!(file system not found)\n");
		return -1;
	}

}

int vfs_mkdir(char* _path, u8 flag)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);

	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_mkdir failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_mkdir failed!(read only file system)\n");
		return -1
	}
	return minfo->dev->fs->mkdir(path,flag,minfo->dev,
		minfo->dev->private_data);
}

int vfs_rndir(char* _path, char* name)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);

	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_rndir failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_rndir failed!(read only file system)\n");
		return -1
	}

	return minfo->dev->fs->rndir(path,name,minfo->dev,
		minfo->dev->private_data);
}

int vfs_rmdir(char* _path)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);
	
	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_rmdir failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_rmdir failed!(read only file system)\n");
		return -1
	}

	return minfo->dev->fs->rmdir(path,minfo->dev,
		minfo->dev->private_data);
}

int vfs_chdir(char* _path)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);
	
	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_chdir failed!(file system not found)\n");
		return -1;
	}

	return minfo->dev->fs->chdir(path,minfo->dev,
		minfo->dev->private_data);
}

int vfs_chmod(char* _path, u8 flag)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);
	
	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_chmod failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_chmod failed!(read only file system)\n");
		return -1
	}

	return minfo->dev->fs->chmod(path,flag,minfo->dev,
		minfo->dev->private_data);
}

int vfs_chown(char* _path, u8 flag)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);
	
	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_chown failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_chown failed!(read only file system)\n");
		return -1
	}

	return minfo->dev->fs->chown(path,flag,minfo->dev,
		minfo->dev->private_data);
}

int vfs_link(char* _old , char* _new)
{
	struct mount_info* old_minfo = NULL;
	struct mount_info* new_minfo = NULL;

	char old[PATH_MAX_LEN]; strcpy(old,_old);
	char new[PATH_MAX_LEN]; strcpy(new,_new);

	if((old_minfo = _vfs.__operation__.get_path_file_system(old)) == NULL)
	{
		kprintf("vfs_link failed!(file system not found)\n");
		return -1;
	}	

	if((new_minfo = _vfs.__operation__.get_path_file_system(new)) == NULL)
	{
		kprintf("vfs_link failed!(file system not found)\n");
		return -1;
	}

	if(new_minfo != old_minfo)
	{
		kprintf("vfs_link failed!(mismatched file system)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_link failed!(read only file system)\n");
		return -1
	}

	return old_minfo->dev->fs->link(old,new,old_minfo->dev,
		old_minfo->dev->private_data);

}

int vfs_unlink(char* _path)
{
	struct mount_info* minfo = NULL;
	char path[PATH_MAX_LEN]; strcpy(path,_path);
	
	if((minfo = _vfs.__operation__.get_path_file_system(path)) == NULL)
	{
		kprintf("vfs_unlink failed!(file system not found)\n");
		return -1;
	}

	if(!(_vfs.__data__.__fs->flag & WR_FS))
	{
		kprintf("vfs_unlink failed!(read only file system)\n");
		return -1
	}

	return minfo->dev->fs->unlink(path,minfo->dev,
		minfo->dev->private_data);
}

int vfs_open(int fd, u8 flag)
{
	if(_vfs.__data__.des->fd != fd)
	{
		_vfs.__data__.des = get_descriptor_file_system(fd);
		
		if(!_vfs.__data__.des)
		{
			kprintf("Bad file descriptor\n");
			return -1;
		}
		
	}

	return _vfs.__data__.des->dev->fs->open(fd,flag,_vfs.__data__.des->dev,
		_vfs.__data__.des->dev->private_data);
}

int vfs_close(int fd)
{
	if(_vfs.__data__.des->fd != fd)
	{
		_vfs.__data__.des = get_descriptor_file_system(fd);
		
		if(!_vfs.__data__.des)
		{
			kprintf("Bad file descriptor\n");
			return -1;
		}
		
	}

	return _vfs.__data__.des->dev->fs->close(fd,_vfs.__data__.des->dev,
		_vfs.__data__.des->dev->private_data);

}

int vfs_eof(int fd)
{
	if(_vfs.__data__.des->fd != fd)
	{
		_vfs.__data__.des = get_descriptor_file_system(fd);
		
		if(!_vfs.__data__.des)
		{
			kprintf("Bad file descriptor\n");
			return -1;
		}
		
	}

	return _vfs.__data__.des->dev->fs->eof(fd,_vfs.__data__.des->dev,
		_vfs.__data__.des->dev->private_data);

}
