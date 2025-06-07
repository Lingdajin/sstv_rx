#include <stdio.h>
#include <stdlib.h>
#include <machine/machdefs.h>

#include "psd_defines.h"

void
start_d2a(
    unsigned    channel,    /* 0 = A, 1 = B      */
    unsigned    nbufs,      /* number of buffers */
    unsigned    nbytes,     /* bytes/per buffer  */
    short*      ibuf,       /* data              */
    short*      qbuf
    )
{
  unsigned core = (__sb_cfsr(MACH_THID)>>4)&0xf;
  unsigned mmio = (core<<28) | MMIO_SB3500_BASE_ADDR;
  unsigned control;
  volatile unsigned * d2a;
  volatile unsigned * en;

  if( channel == 0 ) {
    d2a = (volatile unsigned *)(mmio|MMIO_SB3500D2A_A_OFF);
    en  = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_EN);
  }
  else {
    d2a = (volatile unsigned *)(mmio|MMIO_SB3500D2A_B_OFF);
    en  = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_EN);
  }

  control = (3<<30)              /* enable both I,Q */
    | (1<<28)                    /* direction */
    | (((nbufs-1)&0xff)<<20)     /* buffers         */
    | ((0&0x3)<<18)              /* no PM timer        */
    | ((0&0x7)<<15)              /* no NMPT timer        */
    | ((nbytes-8)&0x7ff8);       /* bytes truncated to multiple of 8 */

  d2a[2] = (unsigned)ibuf;
  d2a[4] = (unsigned)qbuf;

  d2a[6] = control;
  en[0]  = 0x2;                   /* enable D2A */
}

void
stop_d2a(
    unsigned    channel    /* 0 = A, 1 = B      */
    )
{
  unsigned core = (__sb_cfsr(MACH_THID)>>4)&0xf;
  unsigned mmio = (core<<28) | MMIO_SB3500_BASE_ADDR;
  volatile unsigned *d2a;
  volatile unsigned *en;
  volatile unsigned *status;

  if( channel == 0 ) {
    d2a    = (volatile unsigned *)(mmio|MMIO_SB3500D2A_A_OFF);
    en     = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_EN);
    status = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_STATUS);
  }
  else {
    d2a    = (volatile unsigned *)(mmio|MMIO_SB3500D2A_B_OFF);
    en     = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_EN);
    status = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_STATUS);
  }

  d2a[6]     = 0;         /* Clear Control register */
  en[0]      = 0;         /* Disable D2A interrupt */
  status[0]  = 1;         /* Disable D2A interrupt */
}

int
is_interrupt_d2a(
    unsigned    channel
    )
{

  unsigned gifr =  __sb_cfsr(MACH_GIFR);
  unsigned bit  = (channel==0)?
           gifr>>MMIO_SB3500PSDA_INT:
           gifr>>MMIO_SB3500PSDB_INT;
 // printf("the gifr data is [%x] \n",gifr);
  //printf("the interrupt data is [%x] \n",(bit&0x1));
  return bit&0x1;
}

void
reset_interrupt_d2a(
    unsigned    channel    /* 0 = A, 1 = B */
    )
{
  unsigned core = (__sb_cfsr(MACH_THID)>>4)&0xf;
  unsigned mmio = (core<<28)|MMIO_SB3500_BASE_ADDR;
  unsigned gifr_mask;
  volatile unsigned* status;

  if( channel == 0 ) {
    gifr_mask = 1<<MMIO_SB3500PSDA_INT;
    status    = (volatile unsigned *)(mmio|MMIO_SB3500PSDA_STATUS);
  }
  else {
    gifr_mask = 1<<MMIO_SB3500PSDB_INT;
    status    = (volatile unsigned *)(mmio|MMIO_SB3500PSDB_STATUS);
  }

  __sb_ctsr(gifr_mask, MACH_GIFR0);
  status[0] = 0x2;                /* enable D2A */
}

/* vi: set ts=2 sw=4 et: */
