#ifndef _QUEUE_H__
#define _QUEUE_H__
#include <lib.h>

struct queue_head
{
    struct queue_head* prev;
    struct queue_head* next;
};

static inline void INIT_QUEUE_HEAD(struct queue_head* head)
{
    head->prev=head;
    head->next=NULL;
}

static inline u32 queue_empty(struct queue_head* head)
{
    return head->next==NULL;
}

#define queue_for_each(p,head) \
for(p=(head)->next; p!=NULL; p=p->next)

#define queue_first_entry(head,type,field)\
(type*)((char*)(head)->next - (char*)&((type*)0)->field)

#define queue_entry(ptr,type,field)\
(type*)((char*)ptr-(char*)&((type*)0)->field)

static inline void queue_del(struct queue_head* p)
{
    p->prev->next=p->next;
    if(p->next) p->next->prev=p->prev;

    p->next=NULL;
    p->prev=NULL;

}

static inline u32 queue_size(struct queue_head* head)
{
    struct queue_head* it=NULL;
    u32 size=0;
    queue_for_each(it,head)
    {
        size++;
    }

    return size;
}

static inline void queue_add(struct queue_head* new, struct queue_head* head)
{
    head->next = new;
    new->prev = head;
    new->next = NULL;
}

static inline void queue_auto_add(struct queue_head* new, struct queue_head* head)
{
    struct queue_head* it = NULL;
    for(it=head; it!=NULL; it=it->next)
    {
        if(it->next == NULL)
        {
            queue_add(new,head);
        }
    }
}

struct queue
{
    struct queue_head head;
    struct queue_head* last_head;
};

static inline void init_queue(struct queue* queue)
{
    INIT_QUEUE_HEAD(&queue->head);
    queue->last_head = &queue->head;
}

static inline struct queue_head* queue_first_head(struct queue* queue)
{
    if(queue->head.next != NULL)
    {
        queue_del(queue->head.next);
        return queue->head.next;
    }
    return NULL;
}

static inline void queue_insert(struct queue* queue, struct queue_head* head)
{
    queue_add(head,queue->last_head);
    if(head)
        queue->last_head = head;
}

#endif