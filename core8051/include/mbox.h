/*
 * Copyright (C) 2016 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    core_mbox Mailboxes
 * @ingroup     core
 * @brief       Mailbox implementation
 *
 * @{
 *
 * @file
 * @brief       Mailbox API
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef MBOX_H
#define MBOX_H

#include "list.h"
#include "cib.h"
#include "msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Static initializer for mbox objects */
//#define MBOX_INIT(queue, queue_size) {{0}, {0}, CIB_INIT(queue_size), queue}
//#define MBOX_INIT(queue, queue_size) (0, 0, CIB_INIT(queue_size), queue)

/**
 * @brief Mailbox struct definition
 */
typedef struct {
    //8051 implementation list_node changed to list_node_t
    list_node_t readers;    /**< list of threads waiting for message    */
    list_node_t writers;    /**< list of threads waiting to send        */
    cib_t cib;              /**< cib for msg array                      */
    msg_t *msg_array;       /**< ptr to array of msg queue              */
} mbox_t;

//enum {
//    NON_BLOCKING = 0,       /**< non-blocking mode */
//    BLOCKING,               /**< blocking mode */
//};
enum {
      NON_BLOCKING,
      BLOCKING,
};
/**
 * @brief Initialize mbox object
 *
 * @note The message queue size must be a power of two!
 *
 * @param[in]   mbox        ptr to mailbox to initialize
 * @param[in]   queue       array of msg_t used as queue
 * @param[in]   queue_size  number of msg_t objects in queue
 */
/* 8051 implementation */
void mbox_init(mbox_t* XDATA mbox, msg_t* XDATA queue, unsigned int XDATA queue_size);
/*static inline void mbox_init(mbox_t *mbox, msg_t *queue, unsigned int queue_size)
{
    mbox->msg_array->sender_pid = queue->sender_pid;
    mbox->msg_array->type = queue->type;
    mbox->msg_array->content.ptr = queue->content.ptr;
    mbox->msg_array->content.value = queue->content.value;
}*/

/**
 * @brief Add message to mailbox
 *
 * If the mailbox is full, this fuction will return right away.
 *
 * @internal
 *
 * @param[in] mbox      ptr to mailbox to operate on
 * @param[in] msg       ptr to message that will be copied into mailbox
 * @param[in] blocking  block if 1, don't block if 0
 *
 * @return  1   if msg could be delivered
 * @return  0   otherwise
 */
int _mbox_put(mbox_t* XDATA mbox, msg_t* XDATA msg, int XDATA blocking);

/**
 * @brief Get message from mailbox
 *
 * If the mailbox is empty, this fuction will return right away.
 *
 * @internal
 *
 * @param[in] mbox  ptr to mailbox to operate on
 * @param[in] msg   ptr to storage for retrieved message
 * @param[in] blocking  block if 1, don't block if 0
 *
 * @return  1   if msg could be retrieved
 * @return  0   otherwise
 */
int _mbox_get(mbox_t* XDATA mbox, msg_t* XDATA msg, int XDATA blocking);

/**
 * @brief Add message to mailbox
 *
 * If the mailbox is full, this fuction will block until space becomes
 * available.
 *
 * @param[in] mbox  ptr to mailbox to operate on
 * @param[in] msg   ptr to message that will be copied into mailbox
 */
/* 8051 implementation */
void mbox_put(mbox_t* XDATA mbox, msg_t* XDATA msg);
/*static inline void mbox_put(mbox_t *mbox, msg_t *msg)
{
    _mbox_put(mbox, msg, BLOCKING);
}*/

/**
 * @brief Add message to mailbox
 *
 * If the mailbox is full, this fuction will return right away.
 *
 * @param[in] mbox  ptr to mailbox to operate on
 * @param[in] msg   ptr to message that will be copied into mailbox
 *
 * @return  1   if msg could be delivered
 * @return  0   otherwise
 */
/* 8051 implementation */
int mbox_try_put(mbox_t* XDATA mbox, msg_t* XDATA msg);
/*static inline int mbox_try_put(mbox_t *mbox, msg_t *msg)
{
    return _mbox_put(mbox, msg, NON_BLOCKING);
}*/

/**
 * @brief Get message from mailbox
 *
 * If the mailbox is empty, this fuction will block until a message becomes
 * available.
 *
 * @param[in] mbox  ptr to mailbox to operate on
 * @param[in] msg   ptr to storage for retrieved message
 */
/* 8051 implementation */
void mbox_get(mbox_t* XDATA mbox, msg_t* XDATA msg);
/*static inline void mbox_get(mbox_t *mbox, msg_t *msg)
{
    _mbox_get(mbox, msg, BLOCKING);
}*/

/**
 * @brief Get message from mailbox
 *
 * If the mailbox is empty, this fuction will return right away.
 *
 * @param[in] mbox  ptr to mailbox to operate on
 * @param[in] msg   ptr to storage for retrieved message
 *
 * @return  1   if msg could be retrieved
 * @return  0   otherwise
 */
/* 8051 implementation */
int mbox_try_get(mbox_t* XDATA mbox, msg_t* XDATA msg);
/*static inline int mbox_try_get(mbox_t *mbox, msg_t *msg)
{
    return _mbox_get(mbox, msg, NON_BLOCKING);
}*/

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* MBOX_H */
