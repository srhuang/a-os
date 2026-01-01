#ifndef __KERNEL_INC_IOQUEUE_H
#define __KERNEL_INC_IOQUEUE_H
#include "lock.h"

//=========================
// define
//=========================

//=========================
// struct
//=========================
struct ioqueue {
    uint8_t* buf;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    struct semaphore item;
    struct semaphore slot;
    struct mutex mlock;
};

//=========================
// external variable
//=========================

//=========================
// function
//=========================
uint32_t            ioq_len(struct ioqueue* ioq);
bool                ioq_empty(struct ioqueue* ioq);
bool                ioq_full(struct ioqueue* ioq);
void                ioq_put(struct ioqueue* ioq, uint8_t data);
uint8_t             ioq_get(struct ioqueue* ioq);
struct ioqueue*     ioq_init(uint32_t size);
void                ioq_deinit(struct ioqueue* ioq);

#endif
