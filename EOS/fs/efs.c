#include <efs.h>

/* hd: extern ata contexte
*/

extern ata hd;

/* _vfs: externe VFS context
*/

extern struct vfs _vfs;

/* max_file: static variable which 
represent the file count per activity
*/
int max_file = 30;

/* Current activity data is required
*/
 extern struct process* current;

/* static standard buffer */
struct fifo* stdin;
struct fifo* stderr;
struct fifo* stdout;

/* init_efs_priv_data
params: _disk is a pointer on internal hard disk(HDD)
lba is the the first lba number of disk
d_size is the number of block inside de disk
return: create disk context with this parameters
*/
void init_efs_priv_data(efs_priv_data* pdata, u32 lba, u32 d_size)
{
    pdata->__disk.sb = &pdata->__sb;
    pdata->__disk.disk_size = d_size;
    pdata->__disk.partition = lba;
    INIT_LIST_HEAD(&pdata->__disk.data_bitmap_cache);
    INIT_LIST_HEAD(&pdata->__disk.inode_bitmap_cache);

    // Setup efs_core field
    pdata->lba = lba;
    pdata->blocks = d_size;
    pdata->block_size = blk_size;

    init_list(&pdata->__files);
}

#define IS_ROOT(x) ((x) == (x)->parent)

/* translate_block_to_lba
params: block_num is block number
return: lba number according to block number
*/

u32 translate_block_to_lba(u32 block_num)
{
    u32 sector_per_block = blk_size / 512;
    return (1 + block_num*sector_per_block);
}

/* translate_lba_to_block
params: lba is lba number
return: block number according to lba number
*/

u32 translate_lba_to_block(u32 lba)
{
    u8 sector_per_block = blk_size / 512;
    u32 block_num = lba / sector_per_block;
    if(!(lba%8)) 
        block_num--;
    return block_num;
}


/* efs_read_disk_size
params: hd is a pointer of internal HDD
return: HDD size in block
*/

u32 efs_read_disk_size(disk* hd)
{
    if(hd)
    {
        return hd->disk_size;
    }
    return 0;
}

/* efs_set_block_used
params: hd is a pointer of internal HDD
blk_num is the block number we have to make used
return: make blk_num block used
*/

void efs_set_block_used(disk* hd, u32 blk_num)
{
    blk_num -= hd->sb->first_data_block;

    if(hd)
    {
        u32 cache_index = blk_num / (8*hd->sb->block_size);
        struct list_head* it = NULL;
        u32 i = 0;
        list_for_each(it,&hd->data_bitmap_cache)
        {
            if(i == cache_index)
            {
                cache* current_cache = list_entry(it,cache,head);
                u32 cache_byte = blk_num % (8*hd->sb->block_size);
                current_cache->bitmap[cache_byte / 8] |= (1 << (7 - cache_byte%8));
                break;
            }
            i++;
        }
    }
}

/* efs_get_block
params: hd is a pointer of internal HDD
return: return a block number which is free
or -1 if no block is found
*/

int efs_get_block(disk* hd)
{
    if(hd)
    {
        struct list_head * it = NULL;
        u32 cache_index = 0;

        list_for_each(it,&hd->data_bitmap_cache)
        {
            cache* current_cache = list_entry(it,cache,head);
            if(!current_cache->fully)
            {
                for(u32 i = 0; i < hd->sb->block_size; i++)
                {
                    if(current_cache->bitmap[i] != 0xff)
                    {
                        for(u8 bit = 0; bit < 8; bit++)
                        {
                            if(!(current_cache->bitmap[i] & (1 << (7-bit))))
                            {
                                if((i*8 + bit) == (8*hd->sb->block_size - 1))
                                    current_cache->fully = 1;
                                hd->sb->free_data_block_count--;
                                current_cache->bitmap[i] |= (1 << (7-bit));
                                return (hd->sb->first_data_block + i*8 + bit) + (cache_index*(8*hd->sb->block_size));
                            }
                        }
                    }
                }
            }
            else
            {
                cache_index++;
            }
        }

        if(hd->sb->current_data_bitmap_block_end < hd->sb->data_bitmap_block_count)
        {
            cache* new_cache = (cache*)sys_alloc(sizeof(cache));
            list_auto_add(&new_cache->head,&hd->data_bitmap_cache);
            cache_index++;
            hd->sb->current_data_bitmap_block_end++;
            hd->sb->free_data_block_count--;
            new_cache->bitmap[0] |= (1 << (7-bit));
            return (cache_index*hd->sb->block_size*8) + hd->sb->first_data_block + 1;
        }
    }

    return -1;
}

/* efs_release_block
params: hd is a pointer of internal HDD
blk_num is the block number we have to make free
return: make free the blk_num
*/

void efs_release_block(disk* hd, u32 blk_num)
{
    blk_num -= hd->sb->first_data_block;

    if(hd)
    {
        u32 cache_index = blk_num / (8*hd->sb->block_size);
        struct list_head* it = NULL;
        u32 i = 0;
        list_for_each(it,&hd->data_bitmap_cache)
        {
            if(i == cache_index)
            {
                cache* current_cache = list_entry(it,cache,head);
                u32 cache_byte = blk_num % (8*hd->sb->block_size);
                current_cache->bitmap[cache_byte/8] &= ~(1 << (7-cache_byte%8));
                hd->sb->free_data_block_count++;
                break;
            }
            i++;
        }
    }
}

/* efs_get_inode_number
params: hd is a pointer of internal HDD
return: return a free inode number on HDD
or -1 when no inode number is availbable
*/

int efs_get_inode_number(disk* hd)
{
    if(hd)
    {
        struct list_head * it = NULL;
        u32 cache_index = 0;

        list_for_each(it,&hd->inode_bitmap_cache)
        {
            cache* current_cache = list_entry(it,cache,head);
            if(!current_cache->fully)
            {
                for(u32 i = 0; i < hd->sb->block_size; i++)
                {
                    if(current_cache->bitmap[i] != 0xff)
                    {
                        for(u8 bit = 0; bit < 8; bit++)
                        {
                            if(!(current_cache->bitmap[i] & (1 << (7-bit))))
                            {
                                if((i*8 + bit) == (8*hd->sb->block_size-1)) 
                                    current_cache->fully = 1;
                                hd->sb->inode_count++;
                                current_cache->bitmap[i] |= (1 << (7-bit));
                                return (i*8 +bit) + (cache_index*(8*hd->sb->block_size));
                            }
                        }
                    }
                }
            }
            else
            {
                cache_index++;
            }
        }

        if(hd->sb->current_inode_bitmap_block_end < hd->sb->inode_bitmap_block_count)
        {
            cache* new_cache = (cache*)sys_alloc(sizeof(cache));
            new_cache->fully = 0;
            list_auto_add(&new_cache->head,&hd->inode_bitmap_cache);
            cache_index++;
            hd->sb->current_inode_bitmap_block_end++;
            hd->sb->inode_count++;
            new_cache->bitmap[0] |= (1 << 7);
            return (cache_index*hd->sb->block_size*8) + 1;
        }
    }


    return -1;
}

/* efs_set_inode_used
params: hd is a pointer of internal HDD
inode_number is the inode we have to make used
return: make inode_number used on HDD
*/

void efs_set_inode_used(disk* hd, u32 inode_number)
{
    if(hd)
    {
        u32 cache_index = inode_number / (8*hd->sb->block_size);
        struct list_head* it = NULL;
        u32 i = 0;
        list_for_each(it,&hd->inode_bitmap_cache)
        {
            if(i == cache_index)
            {
                cache* current_cache = list_entry(it,cache,head);
                u32 cache_byte = inode_number % (8*hd->sb->block_size);
                current_cache->bitmap[cache_byte/8] |= (1 << (7 - cache_byte%8));
                break;
            }
            i++;
        }

    }
}

/* efs_release_inode
params: hd is a pointer of internal HDD
inode_number is the inode we have to make free
return: make free inode_number of HDD
*/

void efs_release_inode(disk* hd, u32 inode_number)
{
    if(hd)
    {
        u32 cache_index = inode_number / (8*hd->sb->block_size);
        struct list_head* it = NULL;
        u32 i = 0;
        list_for_each(it,&hd->inode_bitmap_cache)
        {
            if(i == cache_index)
            {
                cache* current_cache = list_entry(it,cache,head);
                u32 cache_byte = inode_number % (8*hd->sb->block_size);
                current_cache->bitmap[cache_byte/8] &= ~(1 << (7-cache_byte%8));
                hd->sb->inode_count--;
                break;
            }
            i++;
        }
    }
}

/* efs_read_inode
params: hd is a pointer of internal HDD
inode is a efs_inode buffer
inode_number is the inode number we want read from HDD
return: put the inode of inode_number on HDD inside inode buffer
or return NULL on error case
*/

efs_inode* efs_read_inode(disk* hd, efs_inode* inode, u32 inode_number)
{
    if(hd)
    {
        disk_read(translate_block_to_lba(hd->sb->first_inode_block + inode_number/hd->inode_per_block),
            (inode_number%hd->inode_per_block)*hd->inode_size,inode,sizeof(efs_inode));
        return inode;
    }

    return NULL;
}


/* efs_write_inode
params: hd is a pointer of internal HDD
inode is a efs_inode buffer
inode_number is the inode number we want write on HDD
return: put the inode  on HDD at inode_number
*/

void efs_write_inode(disk* hd, efs_inode* inode, u32 inode_number)
{
    if(hd)
    {
        disk_write(translate_block_to_lba(hd->sb->first_inode_block + inode_number/hd->inode_per_block),
            (inode_number%hd->inode_per_block)*hd->inode_size,inode,sizeof(efs_inode));
    }
    
}

/* efs_write_block
params: block is the block number 
offset is the offset from block
buffer is data we want to write on block at offset
return: return the number of bytes written
*/

u32 efs_write_block(u32 block, u32 offset, void* buffer)
{
    return disk_write(translate_block_to_lba(block),offset,buffer,blk_size);
}

/* efs_read_block
params: block is the block number 
offset is the offset from block
buffer is data we want to read from block at offset
return: return the number of bytes read
*/
u32 efs_read_block(u32 block, u32 offset, void* buffer)
{
    return disk_read(translate_block_to_lba(block),offset,buffer,blk_size);
}


/*
* Now efs_dentry operation
*/

/* efs_set_dentry_mode
params dentry is EFS entry point
mode is right and type of dentry mode(READ,WRITE,EXEC,TYPE)
return: make the dentry at this mode
*/

void efs_set_dentry_mode(efs_dentry* dentry, u8 mode)
{
    dentry->flags.r = flags & 0x01;
    dentry->flags.w = (flags >> 1) & 0x01;
    dentry->flags.x = (flags >> 2) & 0x01;
    dentry->flags.type = (flags >> 5) & 0x07;

    dentry->inode->_mode.r = flags & 0x01;
    dentry->inode->_mode.w = (flags >> 1) & 0x01;
    dentry->inode->_mode.x = (flags >> 2) & 0x01;
    dentry->inode->_mode.type = (flags >> 5) & 0x07;

}

 /* efs_get_dentry_mode
 params: dentry is EFS entry point
 return: return dentry mode
 */

u8 efs_get_dentry_mode(efs_dentry* dentry)
{
    u8 flag0 = dentry->flags.r;
    u8 flag1 = dentry->flags.w; flag1 = flag1 << 1;
    u8 flag2 = dentry->flags.x; flag2 = flag2 << 2;
    u8 flag3 = dentry->flags.type; flag3 = flag3 << 5;

    return flag3|flag2|flag1|flag0;
}

/* efs_lookup_path
params: priv --> EFS privata data
path --> absolute path
return: return the parent dentry of path
*/

efs_dentry* efs_lookup_path(efs_priv_data* priv, char* path)
{
    uchar* name = path;
    uchar* slash = NULL;
    u16 len = 0;

    efs_dentry* dentry = &priv->root;

    if(name[0] == '\0')
        return dentry;

    while(1)
    {
        slash = strchr(name,'/');
        if(!slash) // Ex: /home/user/data
        {
            len = strlen(name);
            return efs_lookup_dentry(dentry,name,len);
        }

        if(slash && !slash[1]) // Ex: /home/user/data/
        {
            len = slash - name;
            return efs_lookup_dentry(dentry,name,len);
        }

        len = slash - name;
        dentry = efs_lookup_dentry(dentry,name,len);
        name = slash + 1; 
    }   
}

/* efs_create_path
params: priv --> EFS privata data
path is a absolute clean path
flags is the right and type of dentry
return: create a dentry with flags at path
*/

void efs_create_path(efs_priv_data* priv, char* path, u8 flags)
{
    uchar* name = path;
    uchar* slash = NULL;
    u16 len = 0;

    efs_dentry* dentry = &priv->root;

    while(1)
    {
        slash = strchr(name,'/');
        if(!slash) // Ex: /home/user/data
        {
            len = strlen(name);
            efs_create_dentry_inside(priv,dentry,name,len,flags)
            return;
        }

        if(slash && !slash[1]) // Ex: /home/user/data/
        {
            len = slash - name;
            efs_create_dentry_inside(priv,dentry,name,len,flags);
            return;
        }

        len = slash - name;
        dentry = efs_create_dentry_inside(priv,dentry,name,len,flags);
        name = slash + 1; 
    }
}

/* efs_create_dentry_inside
params: priv --> EFS privata data
parent is the parent dentry
name is the new dentry name
len is the length of name
flags is the mode of dentry
return: create new dentry inside parent with name and flags specified
*/

efs_entry* efs_create_dentry_inside(efs_priv_data* priv, efs_dentry* parent, uchar* name, u8 len, u8 flags) // flags >> mode
{
    priv->mutex.mutex_lock();
    efs_dentry* entry = efs_lookup_dentry(parent,name,len);

    if(!entry)
    {
        entry = (efs_dentry*)sys_alloc(efs_dentry);
        entry->inode->parent = parent->inode_number;
        strncpy(entry->inode->path,name,len);
        strncpy(entry->name,name,len);
        efs_set_dentry_parent(entry,parent);
        efs_init_dentry(entry);
        efs_add_dentry_to(parent,entry);
        efs_set_dentry_mode(entry,flags);
        efs_set_dentry_state(entry,parent->state);
        entry->inode_number = efs_get_inode_number(&priv->__disk);
        
        if(entry->inode_number == -1)
        {
            sys_free(entry);
            return NULL;
        }

        efs_set_dentry_inode(entry,(efs_inode*)sys_alloc(sizeof(efs_inode)));
        get_local_time(&entry->inode->created_time);
        get_local_time(&entry->inode->last_access_time);        
        efs_write_inode(entry->hd,entry->inode,entry->inode_number);
    }
    priv->mutex.mutex_unlock();
    return entry;
}

/* efs_get_dentry_inode
params: dentry is EFS entry point
return: return the dentry inode
*/

efs_inode* efs_get_dentry_inode(efs_dentry* dentry)
{
    if(dentry)
    {
        if(dentry->inode)
        {
            return dentry->inode;
        }
    }

    return NULL;
}

/* efs_get_dentry_inode
params: dentry is EFS entry point
inode is an external efs_inode buffer
return: put inode buffer inside dentry
*/

void efs_set_dentry_inode(efs_dentry* dentry, efs_inode* inode)
{
    if(dentry && inode)
    {
        dentry->inode = inode;
    }
}

/* efs_init_dentry
params: dentry is EFS entry point
return: initialize list_head
*/

void efs_init_dentry(efs_dentry* dentry)
{
    if(dentry)
    {
        init_list(&dentry->childs);   
        INIT_LIST_HEAD(&dentry->head);
    }
}

/*efs_set_dentry_parent
params: dentry and parent are EFS entry point
return: make dentry's parent parent dentry
*/

void efs_set_dentry_parent(efs_dentry* dentry, efs_dentry* parent)
{
    if(dentry && parent)
    {
        dentry->parent = parent;
    }
}

/* efs_get_dentry_parent
params: dentry is EFS entry point
return: return dentry parent
*/

efs_dentry* efs_get_dentry_parent(efs_dentry* dentry)
{
    if(dentry)
    {
        return dentry->parent;
    }

    return NULL;
}

/* efs_add_dentry_to
params: old and new are EFS dentry point
return: add new inside old
*/

void efs_add_dentry_to(efs_dentry* old, efs_dentry* new)
{
    if(old && new)
    {
        list_insert(&old->childs,&new->head);
    }
}

/* efs_set_dentry_state
params: dentry is EFS entry point
state is the state(VALID,INVALID) of dentry
return: make dentry's state the state
VALID state means that dentry has an inode
INVALID state means that dentry hasn't a inode
*/ 

void efs_set_dentry_state(efs_dentry* dentry, u8 state)
{
    if(dentry)
    {
        dentry->state = state;
    }
}

/* efs_get_dentry_state
params: dentry is EFS entry point
return: return the state of dentry
*/

u8 efs_get_dentry_state(efs_dentry* dentry)
{
    if(dentry)
    {
        return dentry->state;
    }

    return 0;
}

/* efs_set_dentry_name
params: dentry is EFS entry point
name is the dentry's name
return: make dentry's name the name
*/

void efs_set_dentry_name(efs_dentry* dentry, uchar* name)
{
    if(dentry && name)
    {
        strncpy(dentry->name,name,64);
        strncpy(dentry->inode->path,name,64);
    }
}

/* efs_get_dentry_name
params: dentry is EFS entry point
return: return the dentry name
*/

uchar* efs_get_dentry_name(efs_dentry* dentry)
{
    if(dentry)
    {
        return dentry->name;
    }
    return NULL;
}

/* efs_lookup_dentry
params: dentry is EFS entry point
name is the dentry we want to search
len is the length of name
return: search the dentry with name inside *dentry and return it
or NULL when no dentry is found
*/

efs_dentry* efs_lookup_dentry(efs_dentry* dentry, uchar* name, u8 len)
{
    if(dentry && name)
    {
        struct list_head* it = NULL;
        list_for_each(it,&dentry->childs.head)
        {
            efs_dentry* entry = list_entry(it,efs_dentry,head);
            if(!strncmp(entry->name,name,len))
            {
                return entry;
            }
        }

        return NULL;
    }

    return NULL;
}

/* efs_init_root_dentry
params: dentry is EFS entry point
priv --> EFS private data
return: initialize dentry as the root point
*/

void efs_init_root_dentry(efs_dentry* root, efs_priv_data* priv)
{
    root->parent = root;
    efs_inode* inode = (efs_inode*)sys_alloc(sizeof(efs_inode));
    root->inode_number = 0;
    root->inode = inode;
    root->parent_number = 0;
    efs_init_dentry(root);
    efs_set_inode_used(&priv->__disk,root->inode_number);
    efs_set_dentry_name(root,"/");
    root->state = EFS_VALID_DENTRY_FLAG;
    inode->parent = 0;
    set_mode(&inode->_mode,EFS_READ_FLAG|EFS_WRITE_FLAG,EFS_DIRECTORY_FLAG);
    get_local_time(&inode->created_time);
    get_local_time(&inode->last_access_time);

}

/*efs_check_dentry
params: dentry is EFS entry point
return: load dentry inode from HDD
*/

void efs_check_dentry(efs_dentry* dentry)
{
    if(dentry->state == EFS_INVALID_DENTRY)
    {
        dentry->inode = (efs_inode*)sys_alloc(sizeof(efs_inode));
        efs_read_inode(dentry->hd,dentry->inode,dentry->inode_number);       
        dentry->state = EFS_VALID_DENTRY;
    }
}

/*efs_check_dentry
params: dentry is EFS entry point
return: store dentry inode on HDD
*/

void efs_sync_dentry(efs_dentry* dentry)
{
    if(dentry->state == EFS_VALID_DENTRY)
    {
        efs_write_inode(dentry->hd,dentry->inode,dentry->inode_number);
    }
}

/* efs_lookup
params: path is an absolute clean path
return: return the dentry according to path
or NULL when no dentry is associated on path
*/

efs_dentry* efs_lookup(efs_priv_data* priv, char* path) 
{
    priv->mutex.mutex_lock();
    uchar* name = path;
    efs_dentry* dentry = &priv->root;
    u32 len = 0;
    uchar* slash = NULL;

    while((slash = strchr(name,'/'))!=NULL)
    {
        len = slash - name;

        dentry = efs_lookup_dentry(dentry,name,len);

        if(!dentry)
            break;

        name = slash + 1;

    }
    priv->mutex.mutex_unlock();
    return dentry;
}

/*efs_scan_disk
params:
disk* --> internal HDD 
root --> root dentry
*/

int efs_scan_disk(disk* _disk, efs_dentry* root)
{
    efs_inode* inode = (efs_inode*)sys_alloc(sizeof(efs_inode));
    u32 inodes = _disk->sb->inode_count;
    u32 cache_index = 0;
    struct list_head* it = NULL;
    if(!inodes)
        return 1;

    list_for_each(it,&_disk->inode_bitmap_cache)
    {
        cache* i_cache = list_entry(it,cache,head);

        for(int i = 0; i < blk_size; i++)
        {
            if(i_cache->bitmap[i] != 0x0)
            {
                for(u8 bit = 0; bit < 8; bit++)
                {
                    u32 i_num = i*8 + (cache_index*8*blk_size);

                    if(i_num)
                    {

                        efs_read_inode(_disk,inode,i_num);
                        inodes--;
                        efs_dentry* dentry = (efs_dentry*)sys_alloc(sizeof(efs_dentry));
                        strcpy(dentry->name,inode->path);
                        dentry->inode_number = i_num;
                        dentry->flags = inode->_mode;
                        dentry->parent_number = inode->parent;
                        dentry->hd = _disk;
                        dentry->super_block = _disk->sb;
                        dentry->state = EFS_INVALID_DENTRY;
                        dentry->inode = NULL;
                        list_auto_add(&dentry->head,&root->head);
                        if(!inodes)
                            return 1;
                    }
                }
            }
        }

        cache_index++;
    } 

    return 0;
}

/*
 Now efs start operation 
 */

/*load_efs
params:
dentry is efs root dentry "root_dentry"
hd is internal HDD
return: load efs superblock from disk and setup it with cache.
1 if success 0 otherwise and -1 if error
*/

int load_efs(efs_dentry* dentry, disk* hd) // WARNING: function create_disk had been called
{

    kprintf("Installing standard I/O buffer...");
    stdout = (struct fifo*)sys_alloc(sizeof(struct fifo));
    stdin = (struct fifo*)sys_alloc(sizeof(struct fifo));
    stderr = (struct fifo*)sys_alloc(sizeof(struct fifo));
    kprintf("[ok]\n");

    kprintf("Reading EFS superblock at %x ...",hd->partition);
    
    if(!disk_read(hd->partition,0,hd->sb,sizeof(efs_super_block)))
    {
        kprintf("PANIC: couldn't read super block.\n");
        return -1;
    }

    if(hd->sb->signature != EFS_SIGNATURE)
    {
        #ifdef __DEBUG__
        kprintf("Invalid EFS superblock.\n");
        kprintf("Attempting to resolve issue..\n");
        #endif // __DEBUG__

        get_local_time(&hd->sb->first_mounting);
        get_local_time(&hd->sb->last_mounting);

        hd->sb->block_size = blk_size;
        hd->sb->inode_size = sizeof(efs_inode);
        hd->sb->inode_count = 0;
        hd->sb->inode_per_block = hd->sb->block_size / hd->sb->inode_size;

        hd->sb->inode_bitmap_block_count = INODE_BITMAP_BLOCK_COUNT;
        hd->sb->inode_block_count = (8*hd->sb->block_size*INODE_BITMAP_BLOCK_COUNT) / hd->sb->inode_per_block;
        hd->sb->data_bitmap_block_count = (hd->disk_size - (hd->sb->inode_bitmap_block_count + hd->sb->inode_block_count + 1))/(8*hd->sb->block_size);

        hd->sb->first_inode_bitmap_block = translate_lba_to_block(hd->partition) + 1; // 1 block reserved for superblock and mbr
        hd->sb->first_inode_block = hd->sb->first_inode_bitmap_block + hd->sb->inode_bitmap_block_count;

        hd->sb->first_data_bitmap_block = hd->sb->first_inode_block + hd->sb->inode_block_count;
        hd->sb->first_data_block = hd->sb->first_data_bitmap_block + hd->sb->data_bitmap_block_count;

        hd->sb->data_block_count = hd->disk_size - (1 + hd->sb->inode_bitmap_block_count + hd->sb->inode_block_count + hd->sb->data_bitmap_block_count);
        hd->sb->free_data_block_count = hd->sb->data_block_count;

        strcpy(hd->sb->path_mounted,dentry->inode->path);

        hd->sb->signature = EFS_SIGNATURE;
        hd->sb->state = 1;
        hd->sb->version = 1;
        hd->sb->error = 0x0;

        cache* data_btmp=(cache*)sys_alloc(sizeof(cache));
        cache* inode_btmp=(cache*)sys_alloc(sizeof(cache));

        list_auto_add(&data_btmp->head,&hd->data_bitmap_cache);
        list_auto_add(&inode_btmp->head,&hd->inode_bitmap_cache);

        hd->sb->current_inode_bitmap_block_end=1;
        hd->sb->current_data_bitmap_block_end=1;
        
        #ifdef __DEBUG__
        kprintf("Issue resolved.\n");
        #endif //__DEBUG__
        return 1;
       
    }
    else if(hd->sb->signature == EFS_SIGNATURE)
    {
        #ifdef __DEBUG__
        kprintf("Valid EFS superblock.\n");
        #endif //__DEBUG__
        /* Now load bitmap cache to memory */
        for(int i = 0; i < hd->sb->current_inode_bitmap_block_end; i++)
        {
            cache* inode_btmp=(cache*)sys_alloc(sizeof(cache));
            disk_read(translate_block_to_lba(hd->sb->first_inode_bitmap_block),i*hd->sb->block_size,inode_btmp->bitmap,hd->sb->block_size);
            list_auto_add(&inode_btmp->head,&hd->inode_bitmap_cache);
        }

        for(int i = 0; i < hd->sb->current_data_bitmap_block_end; i++)
        {
            cache* data_btmp=(cache*)sys_alloc(sizeof(cache));
            disk_read(translate_block_to_lba(hd->sb->first_data_bitmap_block),i*hd->sb->block_size,data_btmp->bitmap,hd->sb->block_size);
            list_auto_add(&data_btmp->head,&hd->data_bitmap_cache);
        }

        return 1; 
    }

    return 0;
}// Good !

/* save_efs
params:
dentry is efs root dentry "root_dentry"
hd is internal HDD 
return
save superblock and cache on internal HDD
and return 1;
*/

int save_efs(efs_dentry* dentry, disk* hd)
{
    kprintf("Writing EFS superblock from directory %s at partition %x ...",
    dentry->inode->path,hd->partition);

    disk_write(hd->partition,0,hd->sb,sizeof(efs_super_block));
    /* Now save bitmap cache */
    struct list_head* it = NULL;
    int i = 0;
    list_for_each(it,&hd->inode_bitmap_cache)
    {
        cache* current = list_entry(it,cache,head);
        disk_write(translate_block_to_lba(hd->sb->first_inode_bitmap_block),i*hd->sb->block_size,current->bitmap,hd->sb->block_size);
        sys_free(current);
        i++;
    }

    i = 0;

    list_for_each(it,&hd->data_bitmap_cache)
    {
        cache* current = list_entry(it,cache,head);
        disk_write(translate_block_to_lba(hd->sb->first_data_bitmap_block),i*hd->sb->block_size,current->bitmap,hd->sb->block_size);
        sys_free(current);
        i++;
    }

    kprintf("[ok]\n");
    
    return 1;
}

/* efs_create:
params: path is file path
perm is file permission
priv is file system private data
return: return file descriptor and add it on 
current activity
*/
int efs_create(char* path, u8 perm, struct device* /*unused*/, void* priv)
{

    efs_priv_data* pdata = (efs_priv_data*)priv;

    efs_dentry* file_dir = efs_lookup(pdata,path); // Checking directory
    if(!file_dir) return 0;

    uchar* slash = strrchr(path,'/'), *filename = NULL;

    if(slash[1]) filename = slash + 1;

    if(!filename) return -1;

    u8 len = strlen(filename);

    efs_dentry* fentry = efs_lookup_dentry(file_dir,filename,len); // Find filename into directory

    if(fentry)
    {
        efs_file* new_file = (efs_file*) sys_alloc(sizeof(efs_file));
        new_file->dentry = fentry;
        new_file->n_proc = 0;
        
        efs_check_dentry(fentry);

            for(int i = 3; i < max_file; i++)
            {
                if(current->fd[i] = -1)
                {
                    new_file->fd = current->id + i;
                    current->fd[i] = new_file->fd;
                    break;
                }
            }

            list_insert(&pdata->__files,&new_file->head);

        return new_file->fd;
    }
    else
    {
        efs_dentry* new_fentry = (efs_dentry*) sys_alloc(sizeof(efs_dentry));
        efs_add_dentry_to(file_dir,new_fentry);
        efs_init_dentry(new_fentry);
        efs_set_dentry_parent(new_fentry,file_dir);
        new_fentry->inode_number = efs_get_inode_number(&priv->__disk);
        efs_inode* inode = (efs_inode*) sys_alloc(sizeof(efs_inode));
        efs_set_dentry_inode(new_fentry,inode);

        /* Now setup inode  Data*/
        inode->user_id = (perm >> 3) & 0x07; // 077
        inode->group_id = (perm & 0x07); // 077
        clean_cursor(&inode->size); // f_size field
        inode->parent = file_dir->inode_number; // parent fields
        strncpy(inode->path,filename,len); // path field
        get_local_time(&inode->created_time);
        get_local_time(&inode->last_access_time);

        efs_write_inode(&priv->__disk,inode,new_fentry->inode_number);
        new_fentry->state = EFS_VALID_DENTRY;

        efs_file* new_file = (efs_file*) sys_alloc(sizeof(efs_file));
        clean_cursor(&new_file->cursor); // cursor field
        new_file->dentry = new_fentry;
        new_file->n_proc = 0;

        for(int i = 3; i < max_file; i++)
        {
            if(current->fd[i] == -1)
            {
                new_file->fd = current->id + i;
                current->fd[i] = new_file->fd;
                break;
            }
        }

        list_insert(&pdata->__files,&new_file->head);

        return new_file->fd;
    }

    return -1; // Normally cpu don't come here
}


/* efs_open
params: fd is file descriptor
right is file right
priv is fs private data
return: 1 if sucess ;0 otherwise
*/
int efs_open(int fd, u8 flag, struct device* /*unused*/, void* priv)
{
    if(fd == 0 || fd == 1 || fd == 2)
        return 1;

    efs_file* file = NULL;
    struct list_head* it = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;

    list_for_each(it,&pdata->__files.head)
    {
        file = list_entry(it,efs_file,head);
        if(file->fd == fd)
        {
            efs_set_dentry_mode(file->dentry,flag);
            file->n_proc++;

            if(right & EFS_APPEND_FLAG)
            {
                file->cursor = file->dentry->inode->size;
            }

            return 1;
        }
    }

    return 0;
}


/* efs_close
params: fd is file descriptor
priv is fs private data
return: 1 if sucess ;0 otherwise
*/
int efs_close(int fd, struct device* /*unsed*/, void* priv)
{
    
    int fd_exist = 0;

    if(fd == 0 || fd == 1 || fd == 2)
        return 1;

    for(int i = 3; i < max_file; i++)
    {
        if(current->fd[i] == fd)
        {
            current->fd[i] = -1;
            fd_exist = 1;
            break;
        }
    }

    if(!fd_exist)
        return 0;

    struct list_head* it = NULL;
    efs_file* file = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;
    pdata->mutex.mutex_lock();
    list_for_each(it,&pdata->__files.head)
    {
        file = list_entry(it,efs_file,head);
        if(file->fd == fd)
        {
            file->n_proc--;
            if(!file->n_proc)
            {
                efs_sync_dentry(file->dentry);
                sys_free(file->dentry->inode);
                file->dentry->state = EFS_INVALID_DENTRY;
                list_remove(&pdata->__files,&file->head);
                sys_free(file);
                break;
            }
        }
    }
    pdata->mutex.mutex_unlock();
    return 1;
}

/* efs_eof
params: fd is file descriptor
priv is file system private data
return
return 1 if EOF 0 otherwise
*/
int efs_eof(int fd, struct device* /*unused*/, void* priv)
{

    if(fd == 0 || fd == 1 || fd == 2)
        return 1;

    struct list_head* it = NULL;
    efs_file* file = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;

    list_for_each(it,&pdata->__files.head)
    {
        file = list_entry(it,efs_file,head);
        if(file->fd == fd)
        {
            return file->cursor.iblock[get_current_cursor(&file->cursor)] >= 
            file->dentry->inode->size.iblock[get_current_cursor(&file->dentry->inode->size)];
        }
    }   
    return 0;
}


/*efs_write
params:
int --> file descriptor
void* --> buffer
u32 buffer --> length
struct device* --> device
void* --> file system private data
return byte count transfered or -1 if error occur
*/

int efs_write(int fd, void* buffer, u32 bytes, struct device* dev, void* priv)
{

    struct list_head* it = NULL;
    efs_file* file = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;

    list_for_each(it,&pdata->__files.head)
    {
        efs_file* _it = list_entry(it,efs_file,head);
        if(_it->fd == fd)
        {
            file = _it;
            break;
        }
    }    

    if(!file)
    {
        kprintf("No file match to descriptor %d\n",fd);
        return -1;
    }

    if(!file->dentry->parent->inode->_mode.w)
    {
        kprintf("Can not write inside the directory.\n");
        return -1;
    }

    u16 bmr = read_pci_device(pdata->__disk.ata_device->dev,BUS_MASTER_REG_OFF);
    bmr &= ~1:


    if(inb(bmr+ATA_BMS_CR_1) & 0x01)
    {
        kprintf("Device is busy\n");
        return -1;
    }

    if(file->dentry->inode->_mode.type == EFS_CHARACTER_DEV_FLAG || 
        file->dentry->inode->_mode.type == EFS_BLOCK_DEV_FLAG)
    {
        kprintf("Can not write special file\n");
        return -1;
    }

    if(file->dentry->inode->_mode.w && buffer && bytes)
    {
        int wbytes = 0;
        int* p = NULL, *pp = NULL;

        int cursor = get_current_cursor(&file->cursor); // Current DIBLOCK cursor
        int* blk_num = &file->dentry->inode->block_num[cursor];
        int blk = bytes / blk_size;
        if(bytes % blk_size) blk++;

        p = sys_alloc(blk_size);

        if((*blk_num))
            efs_read_block(*blk_num,0,p);
        else
            *blk_num = efs_get_block(&pdata->__disk);
        
        if(*block_num == -1)
        {
            kprintf("No enough memory\n");
            return -1;
        }    

        int p_index = file->cursor.iblock[cursor] >> 22;
        pp = sys_alloc(blk_size);

        if(p[p_index])
            efs_read_block(p[p_index],0,pp);
        else
            p[p_index] = efs_get_block(&pdata->__disk);

        if(p[p_index] == -1)
        {
            kprintf("No enough memory\n");
            return -1;
        }    

        int pp_index = file->cursor.iblock[cursor] & 0x3ff; // get 10 low weight bits

        int blk_off = file->cursor.iblock[cursor] % blk_size; // block offset

        for(int i = 0; i < blk; i++) // Now allocate data block
        {
            if(!pp[pp_index+i])
                pp[pp_index+i] = efs_get_block(&pdata->__disk);    

            if(pp[pp_index+i] == -1)
            {
                kprintf("No enough memory\n");
                return -1;
            }    

        }

        if(!dev->write)
        {
            kprintf("PANIC: device has no function write.\n");
            return -1;
        }

        wbytes = dev->write(translate_block_to_lba(pp[pp_index]),blk_off,buffer,bytes); // Now write data

        move_file_cursor(&file->cursor,wbytes); // Update file cursor
        move_file_cursor(&file->dentry->inode->size,wbytes); // Update file size

        efs_write_block(*blk_num,0,p);
        efs_write_block(p[p_index],0,pp);
        sys_free(p); sys_free(pp);
        get_local_time(&file->dentry->inode->last_access_time);
        
        return wbytes;
    }

    return -1;
}

/*efs_read
params:
int --> file descriptor
void* --> buffer
u32 --> buffer length
struct device* --> device
void* --> file system private data
return: byte count transfered or -1 when error occurs
*/

int efs_read(int fd, void* buffer, u32 bytes, struct device* dev, void* priv)
{

    struct list_head* it = NULL;
    efs_file* file = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;

    list_for_each(it,&pdata->__files.head)
    {
        efs_file* _it = list_entry(it,efs_file,head);
        if(_it->fd == fd)
        {
            file = _it;
            break;
        }
    }    


    if(!file)
    {
        kprintf("No file match to descriptor %d\n",fd);
        return -1;
    }

    if(!file->dentry->parent->inode->_mode.r)
    {
        kprintf("Can not read inside the directory.\n");
        return -1;
    }
    
    u16 bmr = read_pci_device(pdata->__disk.ata_device->dev,BUS_MASTER_REG_OFF);
    bmr &= ~1:


    if(inb(bmr+ATA_BMS_CR_1) & 0x01)
    {
        kprintf("Device is busy\n");
        return -1;
    }

    if(file->dentry->inode->_mode.type == EFS_CHARACTER_DEV_FLAG || 
        file->dentry->inode->_mode.type == EFS_BLOCK_DEV_FLAG)
    {
        kprintf("Can not read special file\n");
        return -1;
    }

    if(file->dentry->inode._mode.r && buffer && bytes)
    {
        int rbytes = 0;
        int* p = NULL, *pp = NULL;
    
        int cursor = get_current_cursor(&file->cursor);
        int* blk_num = &file->dentry->inode->block_num[cursor];

        if(!(*blk_num)) // if block didn't create return with 0
            return -1;

        p = sys_alloc(blk_size);
        int p_index = file->cursor.iblock[cursor] >> 22;

        if(!p[p_index]) // if block didn't create return with 0
            return -1;

        pp = sys_alloc(blk_size);
        int pp_index = file->cursor.iblock[cursor] & 0x3ff; // get 10 low weight bits

        if(!pp[pp_index]) // if block didn't create return with 0
            return -1;

        int blk_off = file->cursor.iblock[cursor] % blk_size; // block offset
        
        if(!dev->read)
        {
            kprintf("PANIC: device has no function read\n");
            return -1;
        }        

        rbytes = dev->read(translate_block_to_lba(pp[pp_index]),blk_off,buffer,bytes); // Now read data

        move_file_cursor(&file->cursor,rbytes); // Update file cursor
        sys_free(p); sys_free(pp);

        return rbytes;
    }

    return -1;
}

/* efs_lseek
params:
int --> file descriptor
u32 --> offset
u8 --> reference (BEGIN,CURRENT,END)
struct device* --> device
void* --> file system private data
return: move file cursor or return -1 if error occurs
*/

int efs_lseek(int fd, u32 offset, u8 ref_pos, struct device* /*unused*/, void* priv)
{
    struct list_head* it = NULL;
    efs_file* file = NULL;
    efs_priv_data* pdata = (efs_priv_data*)priv;

    list_for_each(it,&pdata->__files.head)
    {
        efs_file* _it = list_entry(it,efs_file,head);
        if(_it->fd == fd)
        {
            file = _it;
            break;
        }
    }    

    if(!file)
    {
        kprintf("No file match to descriptor %d\n",fd);
        return -1;
    }

    u32 cursor = 0;
    u32 val = 0;
    switch(ref_pos)
    {
        case FILE_BEG:
            clean_cursor(&file->cursor);
            move_file_cursor(&file->cursor,offset);
            break;
        case FILE_CUR:
            move_file_cursor(&file->cursor,offset);
            break;
        case FILE_END:
            file->cursor = file->dentry->inode->file_size;
            cursor = get_current_cursor(&file->cursor);
            if(offset > file->cursor.iblock[cursor])
            {
                val = file->cursor.iblock[cursor];
                file->cursor.iblock[cursor] = 0;
                if(cursor-1 >=0)
                    file->cursor.iblock[cursor-1] = val-offset; 
            }
            else
            {
                file->cursor.iblock[cursor] -= offset;
            }
            break;
        default:
            return -1;
    }

    return -1;
}

/* efs_link
params:
char* --> old path
char* --> new path
struct device* --> device
void* --> file system private data
return: create a link file for old file or return -1 if error occurs
*/

int efs_link(char* old_path, char* new_path, struct device* /*unused*/, void* priv)
{

    efs_priv_data* pdata = (efs_priv_data*)priv;

    efs_dentry* old_dentry = efs_lookup(pdata,old_path); // Get old file or directory parent
    if(!old_dentry)
        return -1;

    uchar* name = strrchr(old_path,'/'); 
    name += 1; // Get file or directory base name

    if(name[0] != '\0') // Find inside file or directory parent the base name
    {
        old_dentry = efs_lookup_dentry(old_dentry,name,strlen(name));
    }

    efs_check_dentry(old_dentry);
    efs_dentry* new_dentry = efs_lookup(pdata,new_path);  // Get new file or directory parent
    if(!new_dentry)
        return -1;

    uchar* new_name = strrchr(new_path,'/');
    new_name += 1; // Get base name of new file or directory

    if(new_name[0] != '\0')
    {
        efs_dentry* dentry = efs_lookup_dentry(new_dentry,new_name,strlen(new_name)); // If base is founded don't continue
        if(dentry)
        {
            kprintf("Annother entry is found.\n");
            return -1;
        }
        else // else create the link between old and new file or directory
        {
            dentry = (efs_dentry*)sys_alloc(sizeof(efs_dentry));
            efs_set_dentry_parent(dentry,new_dentry);
            efs_set_dentry_name(dentry,new_name);
            efs_set_dentry_inode(dentry,old_dentry->inode);
            efs_set_dentry_state(dentry,old_dentry->state);
            efs_add_dentry_to(new_dentry,dentry);
            efs_init_dentry(dentry);
            dentry->childs = old_dentry->childs;
            dentry->inode_number = old_dentry->inode_number;
            dentry->parent_number = old_dentry->inode->parent;
            dentry->flags.type = EFS_LINK_FILE;
            dentry->inode->link_count++;
            efs_sync_dentry(old_dentry);
            return 1;   
        }
    }
    
    return -1;
}

/* efs_unlink
params:
char* --> file path
struct device* --> device
void* --> file system private data
return: unlink file or return -1 if error occurs
*/

int efs_unlink(char* path, struct device* /*unused*/, void* priv/*unused*/)
{

    efs_priv_data* pdata = (efs_priv_data*)priv;

    efs_dentry* dentry = efs_lookup_path(pdata,path);
    if(!dentry)
        return -1;

        if(dentry->flags.type == EFS_LINK_FILE)
        {
            pdata->mutex.mutex_lock();
            list_remove(&dentry->parent->childs,&dentry->head);
            dentry->inode->link_count--;
            sys_free(dentry);
            pdata->mutex.mutex_unlock();
            return 1;
        }
        else
        {
            kprintf("file isn't a link file.\n");
            return -1;
        }

    return -1;
}

/* efs_flush
params:
int --> file descriptor
struct device* --> device
void* --> file system private data
return: clean input/output system buffer
or -1 if error occurs
*/

int efs_flush(int fd, struct device* /*unused*/, void* priv)
{
    switch(fd)
    {
        case 0:
            fifo_clean(stdin);
            break;
        case 1:
            fifo_clean(stdout);
            break;
        case 2:
            fifo_clean(stderr);
            break;
        default:
            return -1;
    }

    return 1;
}

/* efs_exist
params:
char* --> file path
struct device* --> device
void* --> file system private data
return 1 if file exist, 0 otherwise and -1 if error occurs
*/

int efs_exist(char* path, struct device* /*unused*/, void* priv)
{
    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup_path(pdata,path);
    return (dentry != NULL);
}

/*efs_mkdir
params:
char* --> directory path
u8 --> directory flags(R,W,X)
struct device* --> device
void* --> file system private data
return: create a directory 
1 if sucess, 0 otherwise 
*/

int efs_mkdir(char* path, u8 flags, struct device* /*unused*/, void* priv)
{

    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup(pdata,path);

    if(dentry)
    {
        uchar* name = strrchr(path,'/');
        name += 1;
        if(name[0] != '\0')
        {
            efs_dentry* entry = efs_lookup_dentry(dentry,name,strlen(name));
            if(entry)
            {
                return 1;
            }
            else
            {
                efs_create_dentry_inside(pdata,dentry,name,strlen(name),flags);
                return 1;
            }
        }
    }
    else
    {
        efs_create_path(pdata,path,flags);
        return 1;
    }

    return 0;
}

/*efs_rndir
char* --> directory path
char* --> file name
struct device* --> device
void* --> file system private data
return: change directory name
1 if sucess, 0 otherwise, -1 if error
*/

int efs_rndir(char* path, char* name, struct device* /*unused*/, void* priv)
{

    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_look_path(pdata,path);

    if(!dentry)
        return 0;

    efs_check_dentry(dentry);
    efs_set_dentry_name(dentry,name);
    efs_sync_dentry(dentry);
    
    return 1;
}

/*efs_rmdir
params:
char* --> directory path
struct device* --> device
void* --> file system private data
return: remove directory
1 if sucess, 0 otherwise, -1 if error
*/

int efs_rmdir(char* path, struct device* /*unused*/,void* priv)
{
    efs_priv_data* pdata = (efs_priv_data*)priv;
    pdata->mutex.mutex_lock();

    pdata->mutex.mutex_unlock();
}

/*efs_chdir
params:
char* --> directory path
struct device* --> device
void* --> file system private data
return: change working directory
1 if sucess, 0 otherwise, -1 if error
*/

int efs_chdir(char* path, struct device* /*unused*/, void* priv)
{
    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup_path(pdata,path);
    if(dentry)
    {
        efs_check_dentry(dentry);
        strncpy(current->cwd,path,PATH_MAX_LEN);
        efs_sync_dentry(dentry);
        return 1;
    }    

    kprintf("Invalid directory path.\n");

    return 0;
}

/*efs_chmod
params:
char* --> directory path
u8 --> directory flags(rwx)
struct device* --> device
void* --> file system private data
return: change directory mode
1 if sucess, 0 otherwise, -1 if error
*/

int efs_chmod(char* path, u8 flags, struct device* /*unused*/, void* priv)
{
    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup_path(pdata,path);
    if(dentry)
    {
        efs_check_dentry(dentry);
        efs_set_dentry_mode(dentry,flags);
        efs_sync_dentry(dentry);
        return 1;
    }    

    kprintf("Invalid directory path.\n");

    return 0;   
}

/*efs_chown
params:
char* --> directory path
u8 --> directory flags(user_flags,group_flags)
struct device* --> device
void* --> file system private data
return: change directory owner
1 if sucess, 0 otherwise, -1 if error
*/

int efs_chown(char* path, u8 flags, struct device* /*unused*/, void* priv)
{
    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup_path(pdata,path);
    if(dentry)
    {
        efs_check_dentry(dentry);
        dentry->inode->user_id = (flags >> 3) & 0x07; // 077
        dentry->inode->group_id = (flags & 0x07); // 077
        efs_sync_dentry(dentry);
        return 1;
    }    

    kprintf("Invalid directory path.\n");
    return 0;
}

/*efs_mount
params:
struct device* --> device
char* --> directory path
void* --> file system private data
return: mount device file system at directory path
1 if sucess, 0 otherwise, -1 if error
*/

int efs_mount(struct device* dev, char* path, void* priv)
{
    if(strcmp(dev->fs->name,"EFS")!=0)
    {
        kprintf("Invalid EFS file system.\n")
        return -1;
    }

    if(dev->state == DEV_BUSY)
    {
        kprintf("Device is busy\n");
        return -1;
    }

    if(!dev->read || !dev->write)
    {
        kprintf("Invalid device read/write drivers\n");
        dev->state = DEV_BROKE;
        return -1;
    }

    if(dev->dev_type != BLOCK_DEVICE)
    {
        kprintf("Mount operation requires block device\n");
        return -1;
    }

    efs_priv_data* pdata = (efs_priv_data*)priv;
    efs_dentry* dentry = efs_lookup_path(pdata,path);

    if(!dentry)
    {
        kprintf("Invalid directory path\n");    
        return -1;
    }

    if(dentry->flags.type != EFS_DIRECTORY_FLAG)
    {
        kprintf("Mount operation requires directory entry point\n");
        return -1;
    }

    if(!list_empty(&dentry->childs.head))
    {
        kprintf("Mount operation requires empty directory\n");
        return -1;
    }

    pdata->root = *dentry;
    pdata->mutex.mutex_lock();
    struct mount_info* minfo = (struct mount_info*)sys_alloc(sizeof(struct mount_info));
    strncpy(minfo->location,path,PATH_MAX_LEN);
    minfo->len = strlen(path);
    minfo->dev = dev;
    _vfs.__operation__.add_mount_point(minfo);
    dev->state = DEV_BUSY;
    kprintf("Mounting %s at %s\n",dev->fs->name,path);
    pdata->mutex.mutex_unlock();

    return 1;
}

/*efs_umount
params:
struct device* --> device
char* --> directory path
void* --> file system private data
return: umount device file system from directory path
1 if sucess, 0 otherwise, -1 if error
*/

int efs_umount(struct device* dev, char* _path, void* priv)
{
    _vfs.__operation__.rm_mount_point(_vfs.__operation__.dev_mount_point(dev));

    return 1;
}

int efs_probe(struct device* dev)
{
    
}

int make_efs(struct device* dev, efs_priv_data* data)
{
    if(dev->dev_type != BLOCK_DEVICE)
        return 0;

    dev->state = DEV_FREE;
    dev->private_data = data;
    dev->fs = sys_alloc(sizeof(struct file_system));
    dev->fs->create = efs_create;
    dev->fs->write = efs_write;
    dev->fs->read = efs_read;
    dev->fs->exist = efs_exist;  
    dev->fs->mount = efs_mount;
    dev->fs->umount = efs_umount;
    dev->fs->mkdir = efs_mkdir;
    dev->fs->rndir = efs_rndir;
    dev->fs->rmdir = efs_rmdir;
    dev->fs->chdir = efs_chdir;
    dev->fs->chmod = efs_chmod;
    dev->fs->chown = efs_chown;
    dev->fs->link = efs_link;
    dev->fs->unlink = efs_unlink;
    dev->fs->open = efs_open;
    dev->fs->close = efs_close;
    dev->fs->eof = efs_eof;
    dev->fs->probe = efs_probe;
    dev->fs->name = "EFS";
    dev->fs->flag = RD_WR_FS;
    dev->read = disk_read;
    dev->write = disk_write;
    __sys_devices.device_add(dev);
    INIT_MUTEX(&pdata->mutex);    
}
