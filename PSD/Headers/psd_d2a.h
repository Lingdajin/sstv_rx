#ifndef _PSD_D2A_H_
#define _PSD_D2A_H_

void
start_d2a(
    unsigned    channel,    /* 0 = A, 1 = B      */
    unsigned    nbufs,      /* number of buffers */
    unsigned    nbytes,     /* bytes/per buffer  */
    short*      ibuf,       /* data              */
    short*      qbuf
);

void
stop_d2a(
    unsigned    channel    /* 0 = A, 1 = B      */
);

int
is_interrupt_d2a(
    unsigned    channel
);

void
reset_interrupt_d2a(
    unsigned    channel    /* 0 = A, 1 = B */
);

#endif
