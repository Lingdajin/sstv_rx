#ifndef FUNCTION_H
#define FUNCTION_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sbdsp/sbatan.h"

// 定点数定义
#define SHIFT_BITS 12 // 定义定点数的小数位数
#define ONE_SHIFTED (1 << SHIFT_BITS) // 定点数的1

// sb_fxatan常量定义
#define SB_FXATAN_MAX 32767   // sb_fxatan输出对应π的最大值
#define SB_FXATAN_PI_SCALE 32767  // sb_fxatan中π对应的比例因子

#define SSTV_TARGET_IQ_SAMPLE_RATE 48000 //目标I/Q对的采样率
#define PSDI_TOTAL_SAMPLE_COUNT (SSTV_TARGET_IQ_SAMPLE_RATE * 0.5)


// SSTV频率范围定义
#define SYNC_FREQ 1200
#define BLACK_FREQ 1500
#define WHITE_FREQ 2300
#define FREQ_RANGE (WHITE_FREQ - BLACK_FREQ)

// 图像定义
#define IMAGE_WIDTH 320   // 图像宽度(像素)
#define IMAGE_HEIGHT 240  // 图像高度(行数)
#define TOTAL_SAMPLES 1771680 // 总采样点数

// BMP文件相关定义
#define BYTES_PER_PIXEL 3 // RGB每像素3字节
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define PADDING_SIZE(width) ((4 - ((width) * BYTES_PER_PIXEL) % 4) % 4)

#define sb_osFastPause(DELAY) {int i; for(i=0; i<DELAY; i++) __sb_barrier();}

#define FREQ_1900 1900          // 第一个目标频率(Hz)
#define FREQ_1200 1200          // 第二个目标频率(Hz)
#define FREQ_1500 1500          // 第三个目标频率(Hz)
#define FREQ_2300 2300          // 第四个目标频率(Hz)
#define FREQ_TOLERANCE 150      // 频率容差(Hz)
#define WINDOW_10MS SSTV_TARGET_IQ_SAMPLE_RATE * 0.01         // 10ms窗口大小 (48000 * 0.01)
#define WINDOW_1MS SSTV_TARGET_IQ_SAMPLE_RATE * 0.001           // 1ms窗口大小 (48000 * 0.001)
#define WINDOW_250US SSTV_TARGET_IQ_SAMPLE_RATE * 0.00025 // 250us窗口大小 (48000 * 0.00025)
#define STEP_10MS SSTV_TARGET_IQ_SAMPLE_RATE * 0.01           // 10ms步长
#define STEP_1MS SSTV_TARGET_IQ_SAMPLE_RATE * 0.001             // 1ms步长
#define SSTV_SCAN_LINE_LENGTH SSTV_TARGET_IQ_SAMPLE_RATE * 0.088 // 每行的采样点数 (48000 * 0.088)
#define SSTV_SCAN_LINE_RY_BY_LENGTH SSTV_TARGET_IQ_SAMPLE_RATE * 0.044
#define VIS_LEADER1_MIN_CONSECUTIVE_WINDOWS 295 // VIS同步头前1900hz最小连续窗口数
#define VIS_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS 8 // VIS同步脉冲1200hz最小连续窗口数
#define VIS_LEADER2_MIN_CONSECUTIVE_WINDOWS 295 // VIS同步头后1900hz最小连续窗口数
#define LINE_SYNC_LEADER_MIN_CONSECUTIVE_WINDOWS 8 // 行同步头1200Hz最小连续窗口数
#define LINE_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS 2 // 行同步头1500Hz最小连续窗口数
#define PARITY_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS 15 // 奇偶行同步脉冲最小连续窗口数
#define PARITY_SYNC_PORCH_MIN_CONSECUTIVE_WINDOWS 4 // 奇偶行同步门廊最小连续窗口数


short frequency_buf_10ms[WINDOW_10MS]; // 用于存储10ms的频率数据
short frequency_buf_1ms[WINDOW_1MS]; // 用于存储10ms的频率数据
short frequency_buf_88ms[SSTV_SCAN_LINE_LENGTH]; // 用于存储88ms的频率数据
short frequency_buf_250us[WINDOW_250US]; // 用于存储250us的频率数据
short frequency_buf_44ms[SSTV_SCAN_LINE_RY_BY_LENGTH]; // 用于存储44ms的频率数据

short Y_88ms[SSTV_SCAN_LINE_LENGTH]; // 用于存储Y信号的88ms数据
short RBY_44ms[SSTV_SCAN_LINE_RY_BY_LENGTH]; // 用于存储RBY信号的44ms数据

// 全局数据数组
short fx_y_lines[IMAGE_HEIGHT][IMAGE_WIDTH];      // 存储所有行的Y值
short fx_ry_lines[IMAGE_HEIGHT/2][IMAGE_WIDTH];   // 存储RY值
short fx_by_lines[IMAGE_HEIGHT/2][IMAGE_WIDTH];   // 存储BY值

unsigned char image_data[IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL]; // 存储最终图像数据

//图像保存路径
const char* bmp_file_path = "sstv_image.bmp";


// 状态机状态
typedef enum {
    VIS_STATE_IDLE,                // 初始状态，搜索第一个1900Hz
    VIS_STATE_FOUND_1900_FIRST,    // 寻找第一个1900Hz，准备跳跃
    VIS_STATE_FOUND_1200,          // 寻找1200Hz，
    VIS_STATE_FOUND_1900_SECOND,    // 寻找第二个1900Hz
    VIS_STATE_COMPLETE             // 完整检测到序列
} VIS_State;

typedef enum {
    STATE_IDLE,                // 初始状态，搜索第一个1200Hz
    STATE_FOUND_1200_FIRST,    // 找到第一个1200Hz，准备跳跃
    STATE_FOUND_1500,      // 搜索1500Hz信号
    STATE_COMPLETE             // 完整检测到序列
} Line_State;

typedef enum {
    SSTV_STATE_IDLE,                // 初始状态，等待数据
    SSTV_STATE_VIS_SYNC_SEARCH,         // 搜索同步信号
    SSTV_STATE_VIS_SYNC_FOUND,          // 找到同步信号
    SSTV_STATE_LINE_SYNC_SEARCH,         // 搜索行同步信号
    SSTV_STATE_LINE_SYNC_FOUND,          // 找到行同步信号
    SSTV_STATE_SCAN_LINE,          // 扫描行数据
    SSTV_SATE_SCAN_LINE_DONE,  // 扫描行数据完成
    SSTV_STATE_PARITY_SYNC_SEARCH,         // 搜索奇偶行同步信号
    SSTV_STATE_EVEN_SCAN_LINE,          // 扫描偶数行数据
    SSTV_STATE_ODD_SCAN_LINE,           // 扫描奇数行数据
    SSTV_STATE_DONE                   // 处理完成
} SSTV_State;


static short calculate_average_short(short* data, int length);
static int is_vis_freq_match(short freq, short target_freq);

// 函数声明
void fm_demodulate(short* I, short* Q, int length, short* freq_fx, int sample_rate);
void freq_to_yuv(short* freq_fx, int length, short* Y_fx);
void map_to_pixels_fx(short* data_fx, int data_length, short* pixels_fx, int pixel_count);
void yuv_to_rgb(short Y, short RY, short BY, short* R, short* G, short* B);
void save_bmp(const char* filename, unsigned char* image_data, int width, int height);

void get_frequency_buffer(short* target_frequency_buffer, int requested_length);
int vis_sync_search();
int line_sync_search();
void line_scan(int line_number);
int parity_sync_search();
void even_scan(int line_number);
void odd_scan(int line_number);
void save_image();


#endif // FUNCTION_H
