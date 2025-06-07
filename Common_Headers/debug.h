/**
 * Constant definition for debugging
 *
 * This defines constant for output debugging utilities.
 *
 * file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//#define MCAPPEX_TEST_VIT_OUT_ADDR   0x40800000
//#define MCAPPEX_TEST_CYC00_ADDR     0x40800008
//#define MCAPPEX_TEST_CYC01_ADDR     0x4080000C
//#define MCAPPEX_TEST_CYC10_ADDR     0x40800010
//#define MCAPPEX_TEST_CYC11_ADDR     0x40800014

#define CRC_ADDR              0x40800000
#define TPU0_CORE0_ADDR       0x40800008
#define TPU0_CORE0_FINSH_ADDR 0x40800010
#define TPU1_CORE0_ADDR       0x4080000C
#define MCAPPEX_RX_ADDR       0x40600008
#define MCAPPEX_TX_ADDR       0x4060000C
#define MCAPPEX_TX1_ADDR      0x40600000

#define TICS_NOW __sb_cfsr(MACH_CYC);

#ifndef RUN_ON_BOARD
#define PRINT_ON(x)  (x)
#else
#define PRINT_ON(x)  ((void)0)
#endif

#define DEBUG_LOG 1
#define PRINTF	printf

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

/* vi: set ts=4 sw=4 et: */

#endif /* DEBUG_H_ */

