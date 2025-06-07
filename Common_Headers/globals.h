/**
 * Global variables definition
 *
 * The functions provides definition for global variables.
 *
 * file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "const.h"
#include "psd_defines.h"
#include "rf_drv.h"
#include "common_intf_phy.h"

extern ST_RF_DRV stRfDrv;
extern void init_rf(void);
#define ACP_FREQ 879600000
#ifdef UE_BAND_5
#define ACP_BAND_FREQ_START 869000000
#define ACP_BAND_FREQ_NUM 250
//#define ACP_BAND_FREQ_START 879600000
//#define ACP_BAND_FREQ_NUM 1
#else ///UE_BAND_8
#define ACP_BAND_FREQ_START 925000000
#define ACP_BAND_FREQ_NUM 350
#endif

typedef struct {
	int	localfreq;
	int	txfreq;
	int	rxfreq;
	int txgain;
	int	rxgain;
	int	gainstep;
	uint32_t volt;
}FREQ_GAIN;
extern FREQ_GAIN gsFreqGain;
#define MAX_GAIN 12

typedef enum {
  NOT_SYNC,
  RESYNC,
  PSS_SYNCING_STEP1,
  PSS_SYNCING_STEP2,
  SSS_SYNC,
  DONE,
  IDLE = 18,
}STATE;
extern STATE state;


////////////////////////////此处开始是从 RF-TEST 工程中的global.h摘的

#define A_ELEMS(x) (sizeof(x) / sizeof(x[0]))

#define SIN_SAMPLES                 (48)
#define SIN_MAX                     (0x800)
#define SIN_TOTAL_SAMPLE_COUNT      (PSDO_TOTAL_SAMPLE_COUNT)
#define PI                      3.1415962535

#define FFT_SIZE                (256)
#define SYMBOL_SIZE             (2048)

#define NW_SUB_FRAME_SAMPLE_COUNT	1920

#endif /* GLOBALS_H_ */

/* vi: set ts=4 sw=4 et: */
