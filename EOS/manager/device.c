#include <device.h>
#include <string.h>

void dev_insert(dev_t* __dev, dev_manager_t* __dev_manager)
{
	int __key = dev_hash(__dev->dev_name);
	__dev->dev_id = __key;
	list_insert(&__dev_manager->dev_list[dev_hash_index(__key)],&__dev->dev_head);
	__dev_manager->devices++;
}

void dev_remove(u8_t* __dev_name, dev_manager_t* __dev_manager)
{
	int __key = dev_hash(__dev_name);
	if(!list_empty(&__dev_manager->dev_list[dev_hash_index(__key)].head))
	{
		struct list_head* it = NULL;
		list_for_each(it,&__dev_manager->dev_list[dev_hash_index(__key)].head)
		{
			dev_t* __dev = list_entry(it,dev_t,dev_head);
			if(!strcmp(__dev_name,__dev->dev_name))
			{
				list_remove(&__dev_manager->dev_list[dev_hash_index(__key)],&__dev->head);
				__dev_manager->devices--;
				return;
			}
		}
	}
}

dev_t* dev_find(u8_t* __dev_name, dev_manager_t* __dev_manager)
{
	int __key = dev_hash(__dev_name);
	if(!list_empty(&__dev_manager->dev_list[dev_hash_index(__key)].head))
	{
		struct list_head* it = NULL;
		list_for_each(it,&__dev_manager->dev_list[dev_hash_index(__key)].head)
		{
			dev_t* __dev = list_entry(it,dev_t,dev_head);
			if(!strcmp(__dev_name,__dev->dev_name))
			{
				return __dev;
			}
		}
	}	

	return NULL;
}

void dev_rename(dev_t* __dev, u8_t* __name, dev_manager_t* __dev_manager)
{
	dev_remove(__dev->dev_name,__dev_manager);
	strncpy(__dev->dev_name,__name,DNAME_MAX_LEN);
	dev_insert(__dev,__dev_manager);
}

u32_t dev_hash(u8_t* __dev_name)
{
	u32_t __key = 0;
	for(int i = 0; __dev_name[i] != '\0'; i++)
		__key += __dev_name[i];
	return __key;
}
