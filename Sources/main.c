/*********************************************************************
 * Name: main.c                
 * Author:
 * Copyright: Your Copyright notice 
 * *******************************************************************
 */


#include <stdint.h>
#include <math.h>

#include <device/sbdspsoc_sb3500.h>
#include <device/mmioutil.h>
#include <sbdsp/sbfft.h>

#include "debug.h"
#include "globals.h"
#include "threadutil.h"
#include "rf_drv.h"
#include "psd_a2d.h"
#include "function.h"

ST_RF_DRV stRfDrv;
STATE state = NOT_SYNC;

extern void *rx_thread(void *arg);
extern void sstv_handle_thread(void *arg);


int main(int argc, char** argv) {

	init_rf();
	sbutil_create_pinned_thread(2, 1, 0, 0, rx_thread, NULL);
	sbutil_create_pinned_thread(2, 2, 0, 0, sstv_handle_thread, NULL);

	return EXIT_SUCCESS;
}
