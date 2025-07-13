#include <stdio.h>
#include <stdlib.h>
#include <machine/machdefs.h>

#include "psd_defines.h"
#include "function.h"
#define CORE_ADDR_BASE(core_id)	(0x80000000 | (core_id) << 28)

void
start_a2d(
		unsigned	core_id,		/* core id */
		unsigned    channel,    /* 0 = A, 1 = B      */
		unsigned	dec,
		unsigned    nbufs,      /* number of buffers */
		unsigned    nbytes,     /* bytes/per buffer  */
		short      *ibuf,       /* data              */
		short      *qbuf
  )
{
	unsigned core = CORE_ADDR_BASE(core_id);
	unsigned mmio = core | MMIO_SB3500_BASE_ADDR;
	unsigned control;
	volatile unsigned *a2d;
	volatile unsigned *en;

	if( channel == 0 ) {
	a2d = (volatile unsigned *)(mmio|MMIO_SB3500A2D_A_OFF);
	en  = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_EN);
	}
	else {
	a2d = (volatile unsigned *)(mmio|MMIO_SB3500A2D_B_OFF);
	en  = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_EN);
	}

	control = (0x3U<<30)            /* enable both I,Q */
			| ((dec&0x3)<<28)            /* Decimation control */
			| (((nbufs-1)&0xff)<<20)     /* buffers         */
			| ((0&0x3)<<18)              /* no PM timer        */
			| ((0&0x7)<<15)              /* no NMPT timer        */
			| ((nbytes*2-8)&0x7ff8);       /* bytes truncated to multiple of 8 */

	a2d[2] = (unsigned)ibuf;       /* I start register */
	a2d[4] = (unsigned)qbuf;       /* Q start register */
	a2d[6] = control;              /* Control register */
	en[0]  = 0x1;                  /* enable A2D Interrupt */
}

void
stop_a2d(
		unsigned	core_id,
		unsigned    channel    /* 0 = A, 1 = B      */
//  short **	  p_last_i,
//  short **	  p_last_q
  )
{
	unsigned core = CORE_ADDR_BASE(core_id);
	unsigned mmio = core | MMIO_SB3500_BASE_ADDR;
	volatile unsigned *a2d;
	volatile unsigned *en;
	volatile unsigned *status;

	if( channel == 0 ) {
		a2d    = (volatile unsigned *)(mmio|MMIO_SB3500A2D_A_OFF);
		en     = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_EN);
		status = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_STATUS);
	}
	else {
		a2d    = (volatile unsigned *)(mmio|MMIO_SB3500A2D_B_OFF);
		en     = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_EN);
		status = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_STATUS);
	}

	a2d[6]    = 0;              /* Clear Control register */
	en[0]     = 0;              /* disable A2D Interrupt */
	status[0] = 1;              /* clear A2D Interrupt flag */

//  *p_last_i = (short *)a2d[2];
//  *p_last_q = (short *)a2d[4];
}

int
is_interrupt_a2d(
		unsigned    channel
  )
{
	unsigned gifr = __sb_cfsr(MACH_GIFR);
	unsigned bit  = (channel==0)?
		   gifr>>MMIO_SB3500PSDA_INT:
		   gifr>>MMIO_SB3500PSDB_INT;

	return bit&0x1;
}

void
reset_interrupt_a2d(
		unsigned	core_id,
		unsigned    channel    /* 0 = A, 1 = B */
  )
{
	unsigned core = CORE_ADDR_BASE(core_id);
	unsigned mmio = core | MMIO_SB3500_BASE_ADDR;
	unsigned gifr_mask;
	volatile unsigned *status;

	if( channel == 0 ) {
	gifr_mask = 1<<MMIO_SB3500PSDA_INT;
	status    = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_STATUS);
	}
	else {
	gifr_mask = 1<<MMIO_SB3500PSDB_INT;
	status    = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_STATUS);
	}

	__sb_ctsr(gifr_mask, MACH_GIFR0); //暂存疑，有MACH_GIFR0和MACH_GIFR1，根据内核中断表似乎就是区分中断类型
	status[0] = 0x1;                /* clear A2D Interrupt flag */
}

/* vi: set ts=2 sw=4 et: */
