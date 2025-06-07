/**
 * Pinned thread creation API.
 *
 * The API provides a simple interface for creating thread pinned
 * to a specific SandBlaster DSP core and TPU.
 *
 * \file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#ifndef THREADUTIL_H_
#define THREADUTIL_H_

#include <pthread.h>

/**
 * Create a thread pinned to the specified SandBlaster DSP core and TPU.
 *
 * This function creates a POSIX thread a "pinned" to the specified
 * SandBlaster DSP core and TPU. A "pinned" thread runs on the specified
 * TPU without any interruptions until it finishes. After that, the TPU can
 * be reused for other threads.
 *
 * \param[in]  core_id	    the core number to which the thread should be pinned;
 *			    valid core numbers are: 0,1,2
 * \param[in]  tpu_id	    the TPU number to which the thread should be pinned;
 *			    valid TPU numbers are: 0,1,2,3
 * \param[in]  stack_size   thread stack size in bytes or 0 to use the default size
 * \param[in]  stack_addr   thread stack address or NULL to use the default stack location
 * \param[in]  f	    a pointer to a thread function (see pthread_create)
 * \param[in]  arg	    an argument to be passed to f upon the thread creation
 * \return     POSIX thread
 */
pthread_t* sbutil_create_pinned_thread(unsigned core_id, unsigned tpu_id,
    size_t stack_size, void* stack_addr, void* (*f) (void*), void* arg);

#endif /* THREADUTIL_H_ */

/* vi: set ts=4 sw=4 et: */
