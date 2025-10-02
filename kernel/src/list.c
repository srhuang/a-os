#include "list.h"
#include "stddef.h"

// The list_init() function initializes a linked list.
void list_init (struct list* plist)
{
    plist->head.prev = NULL;
    plist->head.next = &plist->tail;
    plist->tail.prev = &plist->head;
    plist->tail.next = NULL;
}

// The list_insert() function inserts an element before the specified element.
void list_insert(struct list_elem* prev_elem, struct list_elem* elem)
{
    prev_elem->prev->next = elem;
    elem->prev = prev_elem->prev;
    elem->next = prev_elem;
    prev_elem->prev = elem;
}

// The list_remove() function removes the element from the list.
void list_remove(struct list_elem* elem)
{
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
}

// The list_append() function appends the element to the tail of the list.
void list_append(struct list* plist, struct list_elem* elem)
{
    list_insert(&plist->tail, elem);
}

// The list_push() function pushes the element to the head of the list.
void list_push(struct list* plist, struct list_elem* elem)
{
    list_insert(plist->head.next, elem);
}

// The list_pop() function pops the element from the head of the list.
struct list_elem* list_pop(struct list* plist)
{
    //check if the list is empty
    if (plist->head.next == &plist->tail)
    {
        return NULL;
    }

    // pop the element
    struct list_elem* elem = plist->head.next;
    list_remove(elem);
    return elem;
}

// The list_empty() function returns true if the list is empty.
bool list_empty(struct list* plist)
{
    return (plist->head.next == &plist->tail ? true : false);
}

// The list_len() function returns the length of the list.
uint32_t list_len(struct list* plist)
{
    struct list_elem* elem = plist->head.next;
    uint32_t length = 0;
    while (elem != &plist->tail)
    {
        elem = elem->next;
        length++;
    }
    return length;
}

// The list_find() function returns true
// if the specified element is in the list.
bool list_find(struct list* plist, struct list_elem* elem)
{
    struct list_elem* cur_elem = plist->head.next;
    while (cur_elem != &plist->tail)
    {
        if (cur_elem == elem)
        {
            return true;
        }
        cur_elem = cur_elem->next;
    }
    return false;
}

// The list_traversal() function traverses the list
// and returns the first element matching the check function.
struct list_elem* list_traversal(struct list* plist, function func, int arg)
{
    struct list_elem* elem = plist->head.next;

    // check empty
    if (list_empty(plist))
    {
        return NULL;
    }

    while (elem != &plist->tail)
    {
        if (func(elem, arg))
        {
            return elem;
        }
        elem = elem->next;
    }

    return NULL;
}

