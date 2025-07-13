#include "function.h"
#include "os/osapi.h"

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

int sstv_handle_done_flag = 0; // SSTV处理完成标志
extern int sstv_iq_data_ready_flag; // SSTV数据准备就绪标志
extern short frequency_buf[PSDI_TOTAL_SAMPLE_COUNT]; // 用于存储解调后的频率数据
int frequency_buf_position = PSDI_BUF_SIZE; //读取频率buffer指针

int power_flag = 0;

// 辅助函数：计算short类型数组的平均值
short calculate_average_short(short* data, int length) {
	long sum;
	int i;
    if (length <= 0) return 0;

    sum = 0;

    for (i = 0; i < length; i++) {
        sum += data[i];
    }
    return (short)(sum / length);
}

// 辅助函数：检查频率是否匹配目标频率（在容差范围内）
int is_vis_freq_match(short freq, short target_freq) {
    return abs(freq - target_freq) <= FREQ_TOLERANCE;
}

// 定点数版的YUV到RGB转换
void yuv_to_rgb(short Y, short RY, short BY, short* R, short* G, short* B) {
    // 标准YUV到RGB转换
	double r = 0.003906*((298.082*(Y - 16)) + (408.583*(RY - 128))); // YUV到RGB转换
	double g = 0.003906 * ((298.082 * (Y - 16.0)) + (-100.291 * (BY - 128.0)) + (-208.12 * (RY - 128.0)));
	double b = 0.003906 * ((298.082 * (Y - 16.0)) + (516.411 * (BY - 128.0)));

    // 限制在0-255范围内
	*R = (r < 0) ? 0 : ((r > 255) ? 255 : (short)r);
	*G = (g < 0) ? 0 : ((g > 255) ? 255 : (short)g);
	*B = (b < 0) ? 0 : ((b > 255) ? 255 : (short)b);

}

// 保存BMP文件
void save_bmp(const char* filename, unsigned char* image_data, int width, int height) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("无法创建BMP文件\n");
        return;
    }

    int padding_size = PADDING_SIZE(width);
    int stride = width * BYTES_PER_PIXEL + padding_size;
    int file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    // BMP文件头
    unsigned char file_header[FILE_HEADER_SIZE] = {
        'B', 'M',                   // 文件类型标识
        file_size & 0xFF,           // 文件大小
        (file_size >> 8) & 0xFF,
        (file_size >> 16) & 0xFF,
        (file_size >> 24) & 0xFF,
        0, 0, 0, 0,                 // 保留
        FILE_HEADER_SIZE + INFO_HEADER_SIZE & 0xFF,  // 像素数据偏移
        ((FILE_HEADER_SIZE + INFO_HEADER_SIZE) >> 8) & 0xFF,
        ((FILE_HEADER_SIZE + INFO_HEADER_SIZE) >> 16) & 0xFF,
        ((FILE_HEADER_SIZE + INFO_HEADER_SIZE) >> 24) & 0xFF
    };

    // BMP信息头
    unsigned char info_header[INFO_HEADER_SIZE] = {
        INFO_HEADER_SIZE & 0xFF,             // 信息头大小
        (INFO_HEADER_SIZE >> 8) & 0xFF,
        (INFO_HEADER_SIZE >> 16) & 0xFF,
        (INFO_HEADER_SIZE >> 24) & 0xFF,
        width & 0xFF,                        // 宽度
        (width >> 8) & 0xFF,
        (width >> 16) & 0xFF,
        (width >> 24) & 0xFF,
        height & 0xFF,                       // 高度
        (height >> 8) & 0xFF,
        (height >> 16) & 0xFF,
        (height >> 24) & 0xFF,
        1, 0,                                // 颜色平面数
        BYTES_PER_PIXEL * 8, 0,              // 每像素位数
        0, 0, 0, 0,                          // 压缩方式(无压缩)
        0, 0, 0, 0,                          // 图像大小
        0, 0, 0, 0,                          // 水平分辨率
        0, 0, 0, 0,                          // 垂直分辨率
        0, 0, 0, 0,                          // 调色板颜色数
        0, 0, 0, 0                           // 重要颜色数
    };

    // 写入文件头和信息头
    fwrite(file_header, 1, FILE_HEADER_SIZE, file);
    fwrite(info_header, 1, INFO_HEADER_SIZE, file);

    // 准备行填充
    unsigned char padding[3] = {0, 0, 0};

    // BMP文件以从下到上的顺序存储行
    for (int y = height - 1; y >= 0; y--) {
        // 写入一行像素数据
        fwrite(&image_data[y * width * BYTES_PER_PIXEL], BYTES_PER_PIXEL, width, file);

        // 添加行填充(如果需要)
        if (padding_size > 0) {
            fwrite(padding, 1, padding_size, file);
        }
    }

    fclose(file);
}


// 定点数版的FM解调函数
void fm_demodulate(short* I, short* Q, int length, short* freq_fx, int sample_rate) {
    short fx_phase[length];       // 使用静态数组避免栈溢出
    short fx_unwrapped[length];


    // 计算每个采样点的相位 - 使用SB3500平台提供的sb_fxatan函数
    for(int i = 0; i < length; i++) {
        // sb_fxatan输出范围是-32768到32767，对应-π到π
        fx_phase[i] = sb_fxatan(I[i], Q[i]);
    }

    // 相位解包装 - 确保相位连续
    fx_unwrapped[0] = fx_phase[0];
    for(int i = 1; i < length; i++) {
        int diff = fx_phase[i] - fx_phase[i-1];

        // 处理相位跳变 (sb_fxatan输出中π对应32767)
        if(diff > SB_FXATAN_MAX) diff -= 2 * SB_FXATAN_MAX;
        else if(diff < -SB_FXATAN_MAX) diff += 2 * SB_FXATAN_MAX;

        fx_unwrapped[i] = fx_unwrapped[i-1] + diff;
    }

    // 计算频率 - 相位差分
    // freq = (phase_diff * sample_rate) / (2*π)
    // 由于sb_fxatan输出已经归一化为-π到π的范围，我们使用其比例因子
    for(int i = 0; i < length - 1; i++) {
        // 相位差
        int phase_diff = fx_unwrapped[i+1] - fx_unwrapped[i];

        // 频率计算: f = (Δφ/2π) * sample_rate
        // 这里我们需要将相位差除以2π对应的sb_fxatan值(2*32767)，然后乘以采样率
        freq_fx[i] = (phase_diff * sample_rate) / (2 * SB_FXATAN_PI_SCALE);
        //printf("freq_fx[%d]: %d\n", i, freq_fx[i]);
    }
}

// 定点数版的频率转YUV函数
void freq_to_yuv(short* freq_fx, int length, short* Y_fx) {


    for(int i = 0; i < length; i++) {
        // 根据SSTV标准, 将频率转换为亮度值
        if(freq_fx[i] <= BLACK_FREQ) {
            Y_fx[i] = 0;
        } else if(freq_fx[i] >= WHITE_FREQ) {
            Y_fx[i] = 255;
        } else {
            // Y = ((freq - BLACK_FREQ) / FREQ_RANGE) * 255.0
            Y_fx[i] = ((freq_fx[i] - BLACK_FREQ) * 255 / FREQ_RANGE);
        }

        // 计算RY和BY
        //R_Y_fx[i] = ((freq_fx[i] - BLACK_FREQ) / FREQ_RANGE) * 255 - 128;
        //B_Y_fx[i] = ((freq_fx[i] - BLACK_FREQ) / FREQ_RANGE) * 255 - 128;
    }
}

// 定点数版的数据映射到像素
void map_to_pixels_fx(short* data_fx, int data_length, short* pixels_fx, int pixel_count) {
    // 使用定点数计算

    int samples_per_pixel_fixed; // 定点数表示的每个像素对应的采样点数

    // 根据 data_length 设置特定的定点数比例因子
    // 这些值是预先计算的 round(float_value * ONE_SHIFTED)
    if (data_length == 4224 - 1) { // 对应原始 data_length 4223
        // 原始浮点数: 13.196875
        // 定点数: round(13.196875 * 4096) = round(54054.4) = 54054
        samples_per_pixel_fixed = 54054;
    } else if (data_length == 2112 - 1) { // 对应原始 data_length 2111
        // 原始浮点数: 6.596875
        // 定点数: round(6.596875 * 4096) = round(27020.8) = 27021
        samples_per_pixel_fixed = 27021;
    } else {
        // 通用情况：如果 data_length 不是这两个特定值
        // 则使用 (data_length / pixel_count) 作为比例，并转换为定点数
        // 这对应于原始代码中注释掉的行: //double samples_per_pixel_fx = (double)data_length / pixel_count;
        if (pixel_count <= 0) { // 防止除以零或无效的 pixel_count
            samples_per_pixel_fixed = 0; // 或者进行其他错误处理/设置默认值
        } else {
            // 计算 (data_length * ONE_SHIFTED) / pixel_count，并进行四舍五入
            samples_per_pixel_fixed = (int)(((long long)data_length * ONE_SHIFTED + pixel_count / 2) / pixel_count);
        }
    }

    for (int pixel = 0; pixel < pixel_count; pixel++) {
        // 计算像素对应的采样点范围
        // 原始逻辑: int start_idx = pixel * samples_per_pixel_fx; (浮点乘法后截断)
        // 定点数逻辑: start_idx = floor(pixel * samples_per_pixel_float)
        // (long long) 类型转换防止 pixel * samples_per_pixel_fixed 溢出
        int start_idx = (int)(((long long)pixel * samples_per_pixel_fixed) >> SHIFT_BITS);

        // 原始逻辑: int end_idx = start_idx + samples_per_pixel_fx - 1;
        // 这有效地使用了 samples_per_pixel_fx 的整数部分进行计算：
        // end_idx = start_idx + floor(samples_per_pixel_float) - 1;
        int samples_per_pixel_int_part = samples_per_pixel_fixed >> SHIFT_BITS; // 获取定点数的整数部分
        int end_idx = start_idx + samples_per_pixel_int_part - 1;

        // 边界检查
        if (end_idx >= data_length) {
            end_idx = data_length - 1;
        }
        if (start_idx < 0) { // 尽管在正常情况下 start_idx >= 0
            start_idx = 0;
        }
        // 确保 start_idx 不会大于 data_length -1 (如果 data_length 为0或负，则此检查可能不够)
        if (start_idx >= data_length && data_length > 0) {
            start_idx = data_length -1;
        }


        // 累加像素范围内的数据值
        int sum = 0;
        int count = 0;

        // 确保循环是有效的，并且索引在范围内
        if (start_idx <= end_idx && start_idx < data_length) {
            for (int i = start_idx; i <= end_idx; i++) {
                sum += data_fx[i];
                count++;
            }
        }

        // 计算平均值
        pixels_fx[pixel] = (count > 0) ? (sum / count) : 0;
    }
}


void get_frequency_buffer(short* target_frequency_buffer, int requested_length) {
    int total_samples_read = 0;

    // 当此函数开始执行时，它正在处理数据（或准备处理），
    // 因此 sstv_handle_done_flag 应为0，除非它明确请求数据。
    // 循环内的逻辑会适时设置 sstv_handle_done_flag。

    while (total_samples_read < requested_length) {
        // 检查当前 frequency_buf 中的数据是否已全部读取完毕
        // PSDI_BUF_SIZE 是每次 fm_demodulate 填充到 frequency_buf 中的样本数
        if (frequency_buf_position >= PSDI_BUF_SIZE) {
            // 当前缓冲区数据已耗尽，需要从接收线程请求新的数据
            sstv_handle_done_flag = 1; // 通知接收线程：我已处理完上一批数据，请补充

            // 等待接收线程准备好数据
            while (sstv_iq_data_ready_flag != 1) {
                sb_osFastPause(10); // 短暂暂停，避免忙等待，让出CPU
            }
            // 接收线程已填充 frequency_buf 并设置 sstv_iq_data_ready_flag = 1

            sstv_iq_data_ready_flag = 0;  // 消耗数据就绪标志：我已收到新数据通知
            frequency_buf_position = 0;   // 重置读取位置到新填充缓冲区的开头
            sstv_handle_done_flag = 0;    // 通知接收线程：我正在处理新的数据
        }

        // 计算本次迭代可以从 frequency_buf 中读取多少样本
        int samples_available_in_current_buf = PSDI_BUF_SIZE - frequency_buf_position;
        int samples_to_copy_this_iteration = requested_length - total_samples_read;

        if (samples_to_copy_this_iteration > samples_available_in_current_buf) {
            samples_to_copy_this_iteration = samples_available_in_current_buf;
        }

        // 从全局 frequency_buf 复制数据到目标缓冲区
        for (int i = 0; i < samples_to_copy_this_iteration; i++) {
            target_frequency_buffer[total_samples_read + i] = frequency_buf[frequency_buf_position + i];
        }

        // 更新全局 frequency_buf 的读取位置和已读取的总样本数
        frequency_buf_position += samples_to_copy_this_iteration;
        total_samples_read += samples_to_copy_this_iteration;
    }
    // 当循环结束时，表示已成功读取 requested_length 个样本。
    // 此时 sstv_handle_done_flag 为 0，表示当前没有主动请求数据。
}

int vis_sync_search() {
    VIS_State current_state = VIS_STATE_IDLE;
    int consecutive_matches = 0; // 用于记录连续匹配的1ms窗口数量

    while (1) {
        // 从 get_frequency_buffer 获取1ms的频率数据
        get_frequency_buffer(frequency_buf_1ms, WINDOW_1MS);

        // 计算这1ms窗口内的平均频率
        short avg_freq_1ms = calculate_average_short(frequency_buf_1ms, WINDOW_1MS);

        switch (current_state) {
            case VIS_STATE_IDLE:
                // 寻找第一个1900Hz导音的开始
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                    consecutive_matches = 1;
                    current_state = VIS_STATE_FOUND_1900_FIRST;
                } else {
                    consecutive_matches = 0; // 未匹配，重置计数
                }
                break;

            case VIS_STATE_FOUND_1900_FIRST:
                // 持续检测第一个1900Hz导音
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                    consecutive_matches++;
                    if (consecutive_matches >= VIS_LEADER1_MIN_CONSECUTIVE_WINDOWS) {
                        // 第一个导音持续时间足够长，转换到寻找1200Hz同步脉冲状态
                        current_state = VIS_STATE_FOUND_1200;
                        consecutive_matches = 0; // 为下一个状态重置计数
                    }
                } else {
                    // 第一个导音中断，重置状态机到初始状态
                    // 并检查当前1ms窗口是否是新的导音开始
                    current_state = VIS_STATE_IDLE;
                    if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                         consecutive_matches = 1;
                         current_state = VIS_STATE_FOUND_1900_FIRST; // 立即开始新的Leader1检测
                    } else {
                        consecutive_matches = 0;
                    }
                }
                break;

            case VIS_STATE_FOUND_1200:
                // 寻找1200Hz同步脉冲
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1200)) {
                    consecutive_matches++;
                    if (consecutive_matches >= VIS_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS) {
                        // 同步脉冲持续时间足够长，转换到寻找第二个1900Hz导音状态
                        current_state = VIS_STATE_FOUND_1900_SECOND;
                        consecutive_matches = 0; // 为下一个状态重置计数
                    }
                } else {
                    // 同步脉冲中断或未找到，重置状态机
                    // 并检查当前1ms窗口是否是新的导音开始
                    current_state = VIS_STATE_IDLE;
                     if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                         consecutive_matches = 1;
                         current_state = VIS_STATE_FOUND_1900_FIRST;
                    } else {
                        consecutive_matches = 0;
                    }
                }
                break;

            case VIS_STATE_FOUND_1900_SECOND:
                // 寻找第二个1900Hz导音
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                    consecutive_matches++;
                    if (consecutive_matches >= VIS_LEADER2_MIN_CONSECUTIVE_WINDOWS) {
                        // 第二个导音持续时间足够长，表示完整的VIS同步头已找到
                        // printf("VIS 同步头找到!\n");
                        return 1; // 成功找到，返回1
                    }
                } else {
                    // 第二个导音中断，重置状态机
                    // 并检查当前1ms窗口是否是新的导音开始
                    current_state = VIS_STATE_IDLE;
                    if (is_vis_freq_match(avg_freq_1ms, FREQ_1900)) {
                         consecutive_matches = 1;
                         current_state = VIS_STATE_FOUND_1900_FIRST;
                    } else {
                        consecutive_matches = 0;
                    }
                }
                break;
        }
    }
    // 此函数要么在找到同步后返回1，要么无限循环。
    // 因此，理论上不会执行到这里。
    // return 0; // 可以作为备用返回，但逻辑上不应到达。
}

int line_sync_search()
{
    Line_State current_state = STATE_IDLE;
    int consecutive_matches = 0;

    while (1) { // 无限循环直到找到行同步信号
        // 从 get_frequency_buffer 获取1ms的频率数据
        get_frequency_buffer(frequency_buf_1ms, WINDOW_1MS);

        // 计算这1ms窗口内的平均频率
        short avg_freq_1ms = calculate_average_short(frequency_buf_1ms, WINDOW_1MS);

        switch (current_state) {
            case STATE_IDLE:
                // 寻找1200Hz行同步导音的开始
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1200)) { // 复用 is_vis_freq_match，因为它做的就是频率比较
                    consecutive_matches = 1;
                    current_state = STATE_FOUND_1200_FIRST;
                } else {
                    consecutive_matches = 0;
                }
                break;

            case STATE_FOUND_1200_FIRST:
                // 持续检测1200Hz导音
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1200)) {
                    consecutive_matches++;
                    if (consecutive_matches >= LINE_SYNC_LEADER_MIN_CONSECUTIVE_WINDOWS) {
                        // 1200Hz导音持续时间足够，转换到寻找1500Hz脉冲状态
                        current_state = STATE_FOUND_1500;
                        consecutive_matches = 0; // 为下一个状态重置计数
                    }
                } else {
                    // 1200Hz导音中断，重置状态机
                    // 并检查当前窗口是否是新的1200Hz导音开始
                    current_state = STATE_IDLE;
                    if (is_vis_freq_match(avg_freq_1ms, FREQ_1200)) {
                        consecutive_matches = 1;
                        current_state = STATE_FOUND_1200_FIRST;
                    } else {
                        consecutive_matches = 0;
                    }
                }
                break;

            case STATE_FOUND_1500:
                // 寻找1500Hz同步脉冲
                if (is_vis_freq_match(avg_freq_1ms, FREQ_1500)) {
                    consecutive_matches++;
                    if (consecutive_matches >= LINE_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS) {
                        // 1500Hz脉冲持续时间足够，表示行同步信号找到
                        // printf("行同步信号找到!\n");
                        return 1; // 成功找到，返回1
                    }
                } else {
                    // 未找到1500Hz脉冲，或者脉冲中断
                    // 行同步要求1200Hz紧接着是1500Hz，如果1500Hz未立即出现或中断，则认为同步失败，需要重新寻找1200Hz
                    current_state = STATE_IDLE;
                    // 检查当前窗口是否是新的1200Hz导音开始
                    if (is_vis_freq_match(avg_freq_1ms, FREQ_1200)) {
                        consecutive_matches = 1;
                        current_state = STATE_FOUND_1200_FIRST;
                    } else {
                        consecutive_matches = 0;
                    }
                }
                break;
        }
    }
    // 理论上不会执行到这里
    // return 0;
}

void line_scan(int line_number)
{
    get_frequency_buffer(frequency_buf_88ms, SSTV_SCAN_LINE_LENGTH);
    freq_to_yuv(frequency_buf_88ms, SSTV_SCAN_LINE_LENGTH, Y_88ms);
    map_to_pixels_fx(Y_88ms, SSTV_SCAN_LINE_LENGTH, fx_y_lines[line_number], IMAGE_WIDTH);
}

// 返回值：0=未找到, 1=找到偶数行同步, 2=找到奇数行同步
int parity_sync_search() {
    Parity_State current_state = PARITY_SEARCH_STATE_IDLE;
    int consecutive_matches = 0;

    int detected_parity_type = 0; // 0: 未确定, 1: 偶数, 2: 奇数

    while (1) { // 无限循环直到找到同步
        // 获取 0.25ms 的频率数据
        get_frequency_buffer(frequency_buf_250us, WINDOW_250US);

        // 计算这 0.25ms 窗口内的平均频率
        short avg_freq_0_25ms = calculate_average_short(frequency_buf_250us, WINDOW_250US);

        switch (current_state) {
            case PARITY_SEARCH_STATE_IDLE:
                if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1500)) {
                    consecutive_matches = 1;
                    current_state = PARITY_SEARCH_STATE_FOUND_EVEN_PULSE;
                    detected_parity_type = 1; // 假设为偶数行
                } else if (is_vis_freq_match(avg_freq_0_25ms, FREQ_2300)) {
                    consecutive_matches = 1;
                    current_state = PARITY_SEARCH_STATE_FOUND_ODD_PULSE;
                    detected_parity_type = 2; // 假设为奇数行
                } else {
                    consecutive_matches = 0;
                    detected_parity_type = 0;
                }
                break;

            case PARITY_SEARCH_STATE_FOUND_EVEN_PULSE:
                if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1500)) {
                    consecutive_matches++;
                    if (consecutive_matches >= PARITY_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS) {
                        current_state = PARITY_SEARCH_STATE_SEARCHING_PORCH;
                        consecutive_matches = 0; // 为下一状态重置
                    }
                } else {
                    // 1500Hz脉冲中断，重置
                    current_state = PARITY_SEARCH_STATE_IDLE;
                    consecutive_matches = 0;
                    detected_parity_type = 0;
                    // 重新检查当前窗口是否是新的脉冲开始
                    if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1500)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_EVEN_PULSE;
                        detected_parity_type = 1;
                    } else if (is_vis_freq_match(avg_freq_0_25ms, FREQ_2300)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_ODD_PULSE;
                        detected_parity_type = 2;
                    }
                }
                break;

            case PARITY_SEARCH_STATE_FOUND_ODD_PULSE:
                if (is_vis_freq_match(avg_freq_0_25ms, FREQ_2300)) {
                    consecutive_matches++;
                    if (consecutive_matches >= PARITY_SYNC_PULSE_MIN_CONSECUTIVE_WINDOWS) {
                        current_state = PARITY_SEARCH_STATE_SEARCHING_PORCH;
                        consecutive_matches = 0; // 为下一状态重置
                    }
                } else {
                    // 2300Hz脉冲中断，重置
                    current_state = PARITY_SEARCH_STATE_IDLE;
                    consecutive_matches = 0;
                    detected_parity_type = 0;
                    // 重新检查当前窗口是否是新的脉冲开始
                     if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1500)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_EVEN_PULSE;
                        detected_parity_type = 1;
                    } else if (is_vis_freq_match(avg_freq_0_25ms, FREQ_2300)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_ODD_PULSE;
                        detected_parity_type = 2;
                    }
                }
                break;

            case PARITY_SEARCH_STATE_SEARCHING_PORCH:
                if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1900)) {
                    consecutive_matches++;
                    if (consecutive_matches >= PARITY_SYNC_PORCH_MIN_CONSECUTIVE_WINDOWS) {
                        // 找到完整的奇偶同步信号
                        // printf("奇偶同步信号找到! 类型: %s\n", detected_parity_type == 1 ? "偶数" : "奇数");
                        return detected_parity_type; // 返回1代表偶数, 2代表奇数
                    }
                } else {
                    // 1900Hz门廊信号未找到或中断，重置整个状态机
                    current_state = PARITY_SEARCH_STATE_IDLE;
                    consecutive_matches = 0;
                    detected_parity_type = 0;
                    // 重新检查当前窗口是否是新的脉冲开始
                    if (is_vis_freq_match(avg_freq_0_25ms, FREQ_1500)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_EVEN_PULSE;
                        detected_parity_type = 1;
                    } else if (is_vis_freq_match(avg_freq_0_25ms, FREQ_2300)) {
                        consecutive_matches = 1;
                        current_state = PARITY_SEARCH_STATE_FOUND_ODD_PULSE;
                        detected_parity_type = 2;
                    }
                }
                break;
        }
    }
    // 理论上不会执行到这里
    // return 0;
}

void even_scan(int line_number)
{
    get_frequency_buffer(frequency_buf_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH);
    freq_to_yuv(frequency_buf_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH, RBY_44ms);
    map_to_pixels_fx(RBY_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH, fx_ry_lines[line_number / 2], IMAGE_WIDTH);
}

void odd_scan(int line_number)
{
    get_frequency_buffer(frequency_buf_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH);
    freq_to_yuv(frequency_buf_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH, RBY_44ms);
    map_to_pixels_fx(RBY_44ms, SSTV_SCAN_LINE_RY_BY_LENGTH, fx_y_lines[(line_number - 1) / 2], IMAGE_WIDTH);
}

void save_image()
{
    // 将YUV转换为RGB
    printf("\n将YUV转换为RGB...\n");
    for(int line = 0; line < IMAGE_HEIGHT; line++) {
        int group = line / 2;

        for(int pixel = 0; pixel < IMAGE_WIDTH; pixel++) {
            short R, G, B;

            // YUV到RGB转换
            yuv_to_rgb(
                fx_y_lines[line][pixel],
                fx_ry_lines[group][pixel],
                fx_by_lines[group][pixel],
                &R, &G, &B
            );

            // 在BMP图像中，像素按BGR顺序存储
            int idx = (line * IMAGE_WIDTH + pixel) * BYTES_PER_PIXEL;
            image_data[idx] = B;     // B
            image_data[idx + 1] = G; // G
            image_data[idx + 2] = R; // R
        }
    }

    // 保存为BMP文件
    save_bmp(bmp_file_path, image_data, IMAGE_WIDTH, IMAGE_HEIGHT);
}

void get_power(short* I, short* Q, int length, int* power_buf)
{
	for(int i = 0; i < length; i++){
		power_buf[i] = ((I[i] * I[i]) + (Q[i] * Q[i])) / 256;
	}
	power_flag = 1;
}
