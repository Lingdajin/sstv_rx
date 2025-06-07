#include <math.h>
#include <device/sbdspsoc_sb3500.h>
#include <device/mmioutil.h>
#include "threadutil.h"
#include "psd_a2d.h"
#include "function.h"

#define SSTV_DECIMATION_FACTOR 159       // 2000000/12000 - 1 approx 165.

// 定义一个数据块的大小，用于传递给SSTV解调器

#define PSDI_BUF_COUNT 2 // buffer数量
#define PSDI_BUF_SIZE (SSTV_TARGET_IQ_SAMPLE_RATE * 0.5)

int sstv_iq_data_ready_flag = 0;    //0为刚开始时数据未准备好，1为缓冲区1准备好，2为缓冲区2准备好

short RX1_psdi_ibuf[PSDI_TOTAL_SAMPLE_COUNT]; // 硬件DMA缓冲区 I
short RX1_psdi_qbuf[PSDI_TOTAL_SAMPLE_COUNT]; // 硬件DMA缓冲区 Q

short RX2_psdi_ibuf[PSDI_TOTAL_SAMPLE_COUNT]; // 双缓冲机制，定义两个buffer轮流填充
short RX2_psdi_qbuf[PSDI_TOTAL_SAMPLE_COUNT]; 

short frequency_buf[PSDI_TOTAL_SAMPLE_COUNT]; // 用于存储解调后的频率数据

extern int sstv_handle_done_flag; // SSTV处理完成标志


void *rx_thread(void *arg) {
    unsigned dec = SSTV_DECIMATION_FACTOR;
    unsigned core = 3;
    unsigned ch = PSD_A;
    unsigned i;
    unsigned current_dma_buffer_idx = 0; // 用于跟踪当前硬件填充的DMA buffer段
    sstv_iq_data_ready_flag = 0;

    printf("SSTV Receiver Thread Started. Target I/Q Sample Rate: ~%u Hz\n", SSTV_TARGET_IQ_SAMPLE_RATE);

    while(1) {
        // 1. 配置并启动ADC
        // PSDI_BUF_COUNT 和 PSDI_BUF_SIZE (即 PSDI_BUF_SAMPLE_COUNT) 定义了硬件DMA缓冲区的管理方式
        reset_interrupt_a2d(core, ch);
        start_a2d(core, ch, dec,
                    PSDI_BUF_COUNT,      // DMA管理的小缓冲区数量
                    PSDI_BUF_SIZE,       // 每个小DMA缓冲区的大小 (I/Q对的数量)
                    RX1_psdi_ibuf, RX1_psdi_qbuf);

        fm_demodulate(RX2_psdi_ibuf, RX2_psdi_qbuf, PSDI_BUF_SIZE, frequency_buf, SSTV_TARGET_IQ_SAMPLE_RATE);

        sstv_iq_data_ready_flag = 1; // 设置标志，表示数据块准备就绪

        printf("SSTV Data Block 2 Ready. Processing...\n");


        // 2. 收集足够一个SSTV处理块的数据，双缓冲机制实现
        // 等待ADC硬件填充完一个DMA缓冲区段
        while(is_interrupt_a2d(ch) != 1) {
            sb_osFastPause(10); // 短暂暂停，让出CPU
        }

        while(!sstv_handle_done_flag){  //等待处理完前一个buffer
		    sb_osFastPause(10);
        }

        reset_interrupt_a2d(core, ch); // 清除中断标志
        

        start_a2d(core, ch, dec,
                    PSDI_BUF_COUNT,      // DMA管理的小缓冲区数量
                    PSDI_BUF_SIZE,       // 每个小DMA缓冲区的大小 (I/Q对的数量)
                    RX2_psdi_ibuf, RX2_psdi_qbuf);

        fm_demodulate(RX1_psdi_ibuf, RX1_psdi_qbuf, PSDI_BUF_SIZE, frequency_buf, SSTV_TARGET_IQ_SAMPLE_RATE);
        
        sstv_iq_data_ready_flag = 1; // 设置标志，表示数据块准备就绪

        printf("SSTV Data Block 1 Ready. Processing...\n");

        while(is_interrupt_a2d(ch) != 1) {
            sb_osFastPause(10); // 短暂暂停，让出CPU
        }

        while(!sstv_handle_done_flag){
		    sb_osFastPause(10);
        }

    }
}
