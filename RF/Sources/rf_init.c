#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <device/sbdspsoc_sb3500.h>
#include <device/mmioutil.h>

#include "rf_drv.h"
#include "debug.h"
#include "globals.h"
ST_RF_DRV stRfDrv;
FREQ_GAIN gsFreqGain;

void init_rf(void)
{
	int16_t dB;
	double txf, rxf;

	iris404(&stRfDrv);
	stRfDrv.Initialization();
	stRfDrv.InitChip();

	printf("ACP_IRIS\n");
	cmd_standby_idle(0);
	osFastPause(10000);
	cmd_idle_fdidle(0);
	osFastPause(10000);
	cmd_start_fdrx(0);
	osFastPause(10000);
//	cmd_start_fdtx(0);
//	osFastPause(10000);

	reg_write(0x0,0x1F30662); // ACP chip TXLOW2 port
	reg_write(0x8,0x1FDA200); // ACP chip RXLOW2 port
//	reg_write(0x4,0x08A0000); // ACP Band8 Tx freq 897MHz
//	reg_write(0x5,0x0000000);
//	reg_write(0x0F,0x1D89D89); // ACP Band8 Rx freq 942MHz
//	reg_write(0x0E,0x0900000);
	stRfDrv.SetMode(0);	//0:TDD mode
	stRfDrv.SetFreq(837000000, 945000000);
//	stRfDrv.SetFreq(2404970000, 2404970000);

	reg_write(0x0,0x1F34E62 ); // bit14 RAMPOUT LDO on
	reg_write(0x1,0x0060C0B  ); // +9dBm

    //Rx
//	stRfDrv.TxCtrl(0);
    stRfDrv.RxCtrl(1);
    PRINT_ON(printf("rx switch on\n"));
    osFastPause(1000);
    gsFreqGain.rxgain = 50;
	stRfDrv.SetRxGain(0, gsFreqGain.rxgain);


    //Tx
//    stRfDrv.TxCtrl(1);
//    osFastPause(1000);
//	stRfDrv.TxPACtrl(1);
//	osFastPause(2000);
//	PRINT_ON(printf("set up Tx\n"));
//	gsFreqGain.txgain = -10;//HALF-FDD: -50 //max 21 -> 23dBm
//	stRfDrv.SetTxAtten(0, gsFreqGain.txgain);
	gsFreqGain.gainstep = 12;

    //Freq
//	gsFreqGain.localfreq = ACP_FREQ;
//	gsFreqGain.rxfreq = gsFreqGain.localfreq;
//	gsFreqGain.txfreq = gsFreqGain.rxfreq-45000000;
//	stRfDrv.SetFreq(gsFreqGain.rxfreq, gsFreqGain.txfreq);

	//GetRxGain
	stRfDrv.GetRxGain(0, &dB);
	printf("get rxgain %d\n",dB);
	//GetTxAtten
//	stRfDrv.GetTxAtten(0, &dB);
//	printf("get TxAtten %d\n",dB);
	PRINT_ON(printf("set RxInd %d\n",dB));
	iris404_getfreq(&txf,&rxf);
	printf("the txf is [%.9f]MHz, rxf is [%.9f]MHz.\n",txf,rxf);
	printf("the addr10 is:%X", reg_read(0x0A));
	osFastPause(1000);
	//Tx
}
