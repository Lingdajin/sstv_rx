#ifndef _PSD_A2D_H_
#define _PSD_A2D_H_

void
start_a2d(
	unsigned	core_id,	/* core id */
    unsigned    channel,    /* 0 = A, 1 = B      */
    unsigned	dec,
    unsigned    nbufs,      /* number of buffers */
    unsigned    nbytes,     /* bytes/per buffer  */
    short		*ibuf,       /* data              */
    short		*qbuf
);

void
stop_a2d(
	unsigned	core_id,
    unsigned    channel    /* 0 = A, 1 = B      */
//    short **	  p_last_i,
//    short **	  p_last_q
);

int
is_interrupt_a2d(
    unsigned    channel
);

void
reset_interrupt_a2d(
	unsigned	core_id,
    unsigned    channel    /* 0 = A, 1 = B */
);

#endif
