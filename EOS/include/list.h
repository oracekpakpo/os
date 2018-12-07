#ifndef _LIST_H__
#define _LIST_H__
#include <lib.h>

#define IS_EMPTY(x) (x)->next == NULL
#define IS_HEAD(x) (x)->prev == (x)

struct list_head
{
    struct list_head* prev;
    struct list_head* next;
};

#define LIST_HEAD_INIT(name) \
{&(name),&(name)} /* Macro defined to init list head */

#define LIST_HEAD(name) \
struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head* head)
{
    head->next = NULL;
    head->prev = head;
}

static inline void list_add(struct list_head* new, struct list_head* head)
{
    head->next = new;
    new->prev = head;
    new->next = NULL;
}

static inline void list_del(struct list_head* p)
{
    if(p->prev)
        p->prev->next = p->next;
    if(p->next) 
        p->next->prev = p->prev;

    p->next = NULL;
    p->prev = NULL;
}

static inline int list_empty(struct list_head* head)
{
    return IS_EMPTY(head);
}

#define list_entry(ptr,type,field) \
(type*)((char*)ptr - (char*)&((type*)0)->field)

#define list_first_entry(head,type,field) \
list_entry((head)->next,type,field)

#define list_for_each(p,head) \
for(p=(head)->next; p!=NULL; p=p->next)

#define list_last_head(p,head) \
for(p=(head)->next; p!=NULL && p->next!=NULL; p=p->next)

#define list_from(p,list) \
for(p=list; p!=NULL; p=p->next)

static inline void list_auto_add(struct list_head* new, struct list_head* head)
{
    for(struct list_head* p = head; p != NULL; p = p->next)
    {
        if(p->next == NULL)
        {
            list_add(new,p);
        }
    }
}

static inline struct list_head* list_get(int index, struct list_head* head)
{
    int local_index = 0;
    struct list_head*p = NULL;

    list_for_each(p,head)
    {
        local_index++;
        if(local_index == index)
        {
            return p;
        }
    }
    return head;
}

struct list
{
    struct list_head head;
    struct list_head* last_head;
};

static inline void init_list(struct list* list)
{
    INIT_LIST_HEAD(&list->head);
    list->last_head = &list->head;
}

static inline void list_remove(struct list* list, struct list_head* head)
{
    if(list->last_head == head)
        list->last_head = head->prev;

    list_del(head);
}

static inline void list_insert(struct list* list, struct list_head* head)
{
    list_add(head,list->last_head);
    if(head)
        list->last_head = head;
}

#endif
