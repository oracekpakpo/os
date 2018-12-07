#ifndef __FIFO_H__
#define __FIFO_H__
#include <types.h>
#define buffer_size 256

struct fifo
{
    u8 buffer[buffer_size];
    u16 read_offset;
    u16 write_offset;
}__attribute__((packed));

static inline void fifo_write(struct fifo* buffer, u8 byte)
{
    if(buffer)
    {
        buffer->buffer[buffer->write_offset]=byte;
        if(buffer->write_offset==buffer_size-1)
        {
            buffer->write_offset=0;
        }
        else
        {
            buffer->write_offset++;
        }
    }
    return;
}

static inline u8 fifo_read(struct fifo* buffer)
{
    u8 byte=0;
    if(buffer)
    {
        byte= buffer->buffer[buffer->read_offset];
        buffer->buffer[buffer->read_offset]=0;
        if(buffer->read_offset==buffer_size-1)
        {
            buffer->read_offset=0;
        }
        else
        {
            buffer->read_offset++;
        }
    }
    return byte;
}


static inline void fifo_clean(struct fifo* buffer)
{
    if(buffer)
    {
        memset(buffer->buffer,0,buffer_size);
        buffer->read_offset=0;
        buffer->write_offset=0;
    }
}


#endif //__FIFO_H__