/**
 * Constant definition and utility functions
 *
 * This file contains
 * 1. constant definition;
 * 2. maximum utility function;
 * 3. saturation utility function.
 *
 * file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#ifndef CONST_H_
#define CONST_H_

#define MAX_SYMBOLS_PER_FRAME	2
#define SUBCARRIER_OFFSET	64
#define SUBCARRIER_SIZE		12
#define MAX_MOD_BITS		2
#define MAX_FRAME_SIZE		(MAX_MOD_BITS*SUBCARRIER_SIZE*MAX_SYMBOLS_PER_FRAME)
#define NINPUTS 		MAX_FRAME_SIZE
#define NOUTPUTS 		((MAX_FRAME_SIZE/2)-6)
#define SUBFRAME_FFT_OUT_SIZE  SUBCARRIER_SIZE*14

#define TYPE_i short

#define MAX(v1,v2) ((v1>=v2)?v1:v2)
#define SAT16(_x) (_x > 0x7FFF ? 0x7FFF : (_x < -0x8000 ? -0x8000 : _x))

#endif /* CONST_H_ */

/* vi: set ts=4 sw=4 et: */
