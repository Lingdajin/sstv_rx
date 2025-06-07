/**
 * PSD constant definition
 *
 * This file contains the constant definition for PSD.
 *
 * file
 */

/* Copyright(c) 2011 Optimum Semiconductor Technologies, Inc.
 * All rights reserved.
 */

#ifndef _PSD_DEFINES_H_
#define _PSD_DEFINES_H_

#define RX_PSD_CHANNEL	    0
#define PSD_NBUFS	    2
#define PSD_BUF_SIZE	    128
#define PSD_TOT_SIZE	    (PSD_NBUFS*PSD_BUF_SIZE)
#define IQ_INPUT_SIZE	    (2*PSD_BUF_SIZE)

#define TRC()  {printf(__FILE__":%d\n",__LINE__);}

#define PSDI_BUF_COUNT              2
#define PSDI_BUF_SAMPLE_COUNT       128
#define PSDI_BUF_SIZE               (PSDI_BUF_SAMPLE_COUNT*2)
#define PDSI_INPUT_SIZE				(PSDI_BUF_COUNT * PSDI_BUF_SAMPLE_COUNT) //PSD收的一个符号的长度(其实和buffer长度相同)
#define PSDI_TOTAL_SAMPLE_COUNT     (PSDI_BUF_COUNT*PSDI_BUF_SAMPLE_COUNT)
/* samples capture, each for I & Q */
#define CAPTURE_SAMPLE_COUNT        (PSDI_TOTAL_SAMPLE_COUNT*10)//(2112<<5)

#define PSD_A 0
#define PSD_B 1

#ifndef MMIO_SB3500_BASE_ADDR
#define MMIO_SB3500_BASE_ADDR   0x0fc00000
#endif

#ifndef MMIO_SB3500A2D_A_OFF
#define MMIO_SB3500A2D_A_OFF	0x0000
#endif

#ifndef MMIO_SB3500D2A_A_OFF
#define MMIO_SB3500D2A_A_OFF    0x0040
#endif
#ifndef MMIO_SB3500PSDA_EN
#define MMIO_SB3500PSDA_EN      0x2480
#endif
#ifndef MMIO_SB3500PSDA_STATUS
#define MMIO_SB3500PSDA_STATUS  0x2400
#endif
#ifndef MMIO_SB3500PSDA_INT
#define MMIO_SB3500PSDA_INT     28
#endif


#ifndef MMIO_SB3500A2D_B_OFF
#define MMIO_SB3500A2D_B_OFF	0x1000
#endif

#ifndef MMIO_SB3500D2A_B_OFF
#define MMIO_SB3500D2A_B_OFF    0x1040
#endif
#ifndef MMIO_SB3500PSDB_EN
#define MMIO_SB3500PSDB_EN      0x2488
#endif
#ifndef MMIO_SB3500PSDB_STATUS
#define MMIO_SB3500PSDB_STATUS  0x2408
#endif
#ifndef MMIO_SB3500PSDB_INT
#define MMIO_SB3500PSDB_INT     29
#endif



#endif /* PSD_DEFINES_H_ */

/* vi: set ts=4 sw=4 et: */
