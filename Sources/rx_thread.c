#include <math.h>
#include <device/sbdspsoc_sb3500.h>
#include <device/mmioutil.h>
#include "threadutil.h"
#include "psd_a2d.h"
#include "function.h"
#include "globals.h"

#define SSTV_DECIMATION_FACTOR 3       // 2000000/12000 - 1 approx 165.

// 定义一个数据块的大小，用于传递给SSTV解调器

#define PSDI_BUF_COUNT 2 // buffer数量


int sstv_iq_data_ready_flag = 0;    //0为刚开始时数据未准备好，1为缓冲区1准备好，2为缓冲区2准备好

short RX1_psdi_ibuf[PSDI_TOTAL_SAMPLE_COUNT]; // 硬件DMA缓冲区 I
short RX1_psdi_qbuf[PSDI_TOTAL_SAMPLE_COUNT]; // 硬件DMA缓冲区 Q

short RX1_capture_ibuf[PSDI_TOTAL_SAMPLE_COUNT / 5];
short RX1_capture_qbuf[PSDI_TOTAL_SAMPLE_COUNT / 5];

short frequency_buf[PSDI_TOTAL_SAMPLE_COUNT]; // 用于存储解调后的频率数据
// int power_buffer[PSDI_TOTAL_SAMPLE_COUNT];

extern int sstv_handle_done_flag; // SSTV处理完成标志
extern int power_flag;


void rx_thread(void *arg) {
    unsigned dec = SSTV_DECIMATION_FACTOR;
    unsigned core = 1;
    unsigned ch = PSD_A;
    sstv_iq_data_ready_flag = 0;

    printf("SSTV Receiver Thread Started. Target I/Q Sample Rate: ~%u Hz\n", SSTV_TARGET_IQ_SAMPLE_RATE);
    for(int i = 0; i < 16; i++){
    	RX1_psdi_ibuf[i] = 1;
    	RX1_psdi_qbuf[i] = 1;
    }

    while(1) {
        // 1. 配置并启动ADC
        // PSDI_BUF_COUNT 和 PSDI_BUF_SIZE (即 PSDI_BUF_SAMPLE_COUNT) 定义了硬件DMA缓冲区的管理方式
//        stop_a2d(core, ch);
        reset_interrupt_a2d(core, ch);
        start_a2d(core, ch, dec,
                    PSDI_BUF_COUNT,      // DMA管理的小缓冲区数量
                    PSDI_TOTAL_SAMPLE_COUNT / 2,       // 每个小DMA缓冲区的大小 (I/Q对的数量)
                    RX1_psdi_ibuf, RX1_psdi_qbuf);
        while(is_interrupt_a2d(ch) != 1){
        	printf(".");
        	sb_osFastPause(10);
        }
        reset_interrupt_a2d(core, ch);
        
        //5倍抽取
        for(int i=0; i<PSDI_TOTAL_SAMPLE_COUNT / 5 / 2; i++){
        	RX1_capture_ibuf[i] = RX1_psdi_ibuf[5*i];
        	RX1_capture_qbuf[i] = RX1_psdi_qbuf[5*i];
        }

        printf("reset1\r\n");
        while(is_interrupt_a2d(ch) != 1){
        	printf(".");
        	sb_osFastPause(10);
        }
        for(int i=PSDI_TOTAL_SAMPLE_COUNT / 5 / 2; i<PSDI_TOTAL_SAMPLE_COUNT / 5; i++){
        	RX1_capture_ibuf[i] = RX1_psdi_ibuf[5*i];
        	RX1_capture_qbuf[i] = RX1_psdi_qbuf[5*i];
        }
        // printf("i: \r\n");
        // for(int i = 0; i< PSDI_TOTAL_SAMPLE_COUNT / 5; i++){
        // 	printf("%d\r\n",RX1_capture_ibuf[i]);
        // }
        printf("reset2\r\n");
        stop_a2d(core, ch);
        osFastPause(1000);
        // break; // 仅测试一次，实际应用中可以去掉这个break
//        fm_demodulate(RX2_psdi_ibuf, RX2_psdi_qbuf, PSDI_BUF_SIZE, frequency_buf, SSTV_TARGET_IQ_SAMPLE_RATE);
//
//        sstv_iq_data_ready_flag = 1; // 设置标志，表示数据块准备就绪
//
//        printf("SSTV Data Block 2 Ready. Processing...\n");
//
//
//        // 2. 收集足够一个SSTV处理块的数据，双缓冲机制实现
//        // 等待ADC硬件填充完一个DMA缓冲区段
//        while(is_interrupt_a2d(ch) != 1) {
//        	printf(".");
//            sb_osFastPause(10); // 短暂暂停，让出CPU
//        }
//
//        while(!sstv_handle_done_flag){  //等待处理完前一个buffer
//		    sb_osFastPause(10);
//        }
//        stop_a2d(core, ch);
//        reset_interrupt_a2d(core, ch); // 清除中断标志
//
//        start_a2d(core, ch, dec,
//                    PSDI_BUF_COUNT,      // DMA管理的小缓冲区数量
//                    PSDI_BUF_SIZE,       // 每个小DMA缓冲区的大小 (I/Q对的数量)
//                    RX2_psdi_ibuf, RX2_psdi_qbuf);
//
//        fm_demodulate(RX1_psdi_ibuf, RX1_psdi_qbuf, PSDI_BUF_SIZE, frequency_buf, SSTV_TARGET_IQ_SAMPLE_RATE);
//
//        sstv_iq_data_ready_flag = 1; // 设置标志，表示数据块准备就绪
//
//        printf("SSTV Data Block 1 Ready. Processing...\n");
//
//        while(is_interrupt_a2d(ch) != 1) {
//            sb_osFastPause(10); // 短暂暂停，让出CPU
//        }
//
//        while(!sstv_handle_done_flag){
//		    sb_osFastPause(10);//        }
//        reset_interrupt_a2d(core, ch);
//        stop_a2d(core, PSD_A);
//        for(int i = 0; i < PSDI_BUF_SIZE; i++){
//        	printf("I%d: %d, Q%d: %d\r\n", i, RX1_psdi_ibuf[i], i, RX1_psdi_qbuf[i]);
//        }
//        printf("i: \r\n");
//        for(int i = 0; i < PSDI_BUF_SIZE; i+=5){
//                	printf("%d\r\n",RX1_psdi_ibuf[i]);
//                }
//        get_power(RX1_psdi_ibuf, RX1_psdi_qbuf, PSDI_BUF_SIZE, power_buffer);
//        while(!power_flag){
//        	sb_osFastPause(10);
//        }
//        for(int i = 0; i < PSDI_BUF_SIZE; i++){
//        	printf("power: %d\r\n", power_buffer[i]);
//        }
//        return EXIT_SUCCESS;
    }
}
