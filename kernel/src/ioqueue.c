#include "ioqueue.h"
#include "memory.h"
#include "stddef.h"
#include "printk.h"

//=========================
// debugging
//=========================
//#define DEBUG

#ifdef DEBUG
    #define pr_debug(fmt, ...) printk(fmt, ##__VA_ARGS__)
#else
    #define pr_debug(fmt, ...) do { } while (0)
#endif

//=========================
// internal struct
//=========================

//=========================
// global variable
//=========================

//=========================
// internal functions
//=========================

//=========================
// external functions
//=========================
uint32_t ioq_len(struct ioqueue* ioq)
{
    uint32_t len = 0;
    if (ioq->head >= ioq->tail) {
        len = ioq->head - ioq->tail;
    } else {
        len = ioq->size - (ioq->tail - ioq->head);
    }
    return len;
}

bool ioq_empty(struct ioqueue* ioq)
{
    return ((ioq->head == ioq->tail)?true:false);
}

bool ioq_full(struct ioqueue* ioq)
{
    uint32_t next = (ioq->head + 1) % ioq->size;
    return ((next == ioq->tail)?true:false);
}

void ioq_put(struct ioqueue* ioq, uint8_t data)
{
    // block if there is no slot.
    sema_down(&ioq->slot);
    mutex_lock(&ioq->mlock);

    ioq->buf[ioq->head] = data;
    ioq->head = (ioq->head + 1) % ioq->size;
    pr_debug("%s:head=%d, data=0x%x\n", __func__, ioq->head, data);

    mutex_unlock(&ioq->mlock);
    sema_up(&ioq->item);
}

uint8_t ioq_get(struct ioqueue* ioq)
{
    uint8_t data;

    // block if there is no item.
    sema_down(&ioq->item);
    mutex_lock(&ioq->mlock);

    data = ioq->buf[ioq->tail];
    ioq->tail = (ioq->tail + 1) % ioq->size;
    pr_debug("%s:tail=%d, data=0x%x\n", __func__, ioq->tail, data);

    mutex_unlock(&ioq->mlock);
    sema_up(&ioq->slot);

    return data;
}

struct ioqueue* ioq_init(uint32_t size)
{
    pr_debug("%s +++\n", __func__);

    if (0 == size) {
        return NULL;
    }

    struct ioqueue* ioq = (struct ioqueue*)kmalloc(sizeof(struct ioqueue));
    ioq->buf = kmalloc(size);
    ioq->size = size;
    ioq->head = 0;
    ioq->tail = 0;
    sema_init(&ioq->item, 0);
    sema_init(&ioq->slot, size - 1);
    mutex_init(&ioq->mlock);

    pr_debug("%s ---\n", __func__);
    return ioq;
}

void ioq_deinit(struct ioqueue* ioq)
{
    kfree(ioq->buf);
    kfree(ioq);
}
