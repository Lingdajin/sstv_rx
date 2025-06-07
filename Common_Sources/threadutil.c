/**
 * Pinned thread creation API implementation.
 *
 * This file implements a simple API for creating threads pinned
 * to the user specified SB3500 DSP core and TPU.
 * The implementation is based on POSIX standard with some extensions
 * specific to SandBlaster DSP OS.
 *
 * \file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <machine/machdefs.h>

#include "threadutil.h"

/* global variables*/
pthread_t core0_tid[MACH_THREADS];
pthread_t core1_tid[MACH_THREADS];
pthread_t core2_tid[MACH_THREADS];

volatile short global = 0;
volatile short flag = -1;

static struct sched_param sched_cache[OS_MAX_CORES*MACH_THREADS];
static pthread_attr_t attrs_cache[OS_MAX_CORES*MACH_THREADS];
static pthread_t thread_cache[OS_MAX_CORES*MACH_THREADS];

pthread_t* sbutil_create_pinned_thread(unsigned core_id, unsigned tpu_id,
        size_t stack_size, void* stack_addr, void* (*f)(void*), void* arg) {

    int rc;
    struct sched_param* sched;
    pthread_attr_t* attr;
    pthread_t* thread;

    assert(core_id < OS_MAX_CORES);
    assert(tpu_id < MACH_THREADS);

    attr = attrs_cache + (core_id*MACH_THREADS + tpu_id);
    thread = thread_cache + (core_id*MACH_THREADS + tpu_id);
    sched = sched_cache + (core_id*MACH_THREADS + tpu_id);

    rc = pthread_attr_init(attr);
    assert(0 == rc);
    rc = pthread_attr_setschedpolicy(attr, SCHED_PIN);
    assert(0 == rc);
    sched->sched_priority = 0;
    sched->sched_cpuset = 0x01 << tpu_id;
    rc = pthread_attr_setschedparam(attr, sched);
    assert(0 == rc);
    rc = pthread_attr_setcore_sb(attr, core_id);
    assert(0 == rc);
    if (stack_size != 0) {
        rc = pthread_attr_setstacksize(attr, stack_size);
        assert(0 == rc);
    }

    if(stack_addr != 0) {
        rc = pthread_attr_setstackaddr(attr, stack_addr);
        assert (0 == rc);
    }

    rc = pthread_create(thread, attr, f, arg);
    assert(0 == rc);

    return thread;
}

/* vi: set ts=4 sw=4 et: */
