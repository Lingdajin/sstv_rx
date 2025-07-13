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
#include <os/lsyncutil.h>
#include <os/hostmemmon.h>
#include "debug.h"
#include "globals.h"
#include "threadutil.h"
#include "rf_drv.h"
#include "psd_a2d.h"
#include "function.h"

ST_RF_DRV stRfDrv;
STATE state = NOT_SYNC;

extern void rx_thread(void *arg);
extern void sstv_handle_thread(void *arg);
extern SSTV_State sstv_state;

const char *sstv_State_Names[] = { "SSTV_STATE_IDLE", "SSTV_STATE_VIS_SYNC_SEARCH", "SSTV_STATE_VIS_SYNC_FOUND", "SSTV_STATE_LINE_SYNC_SEARCH", "SSTV_STATE_LINE_SYNC_FOUND", "SSTV_STATE_SCAN_LINE", "SSTV_SATE_SCAN_LINE_DONE", "SSTV_STATE_PARITY_SYNC_SEARCH", "SSTV_STATE_EVEN_SCAN_LINE", "SSTV_STATE_ODD_SCAN_LINE", "SSTV_STATE_DONE" };


int main(int argc, char** argv) {

	osHostMemMonSetAddress(0x80000FE0);
	osHostMemMonFormatBegin(0);

	osHostMemMonFormat("sstv_state: %s\r\n", sstv_State_Names[sstv_state]);

	osHostMemMonFormatEnd();

	init_rf();
	sbutil_create_pinned_thread(0, 1, 0, 0, rx_thread, NULL);
	sbutil_create_pinned_thread(0, 2, 0, 0, sstv_handle_thread, NULL);

	return EXIT_SUCCESS;
}
