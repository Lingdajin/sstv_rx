#include "function.h"

short frequency_buf_300ms[SSTV_TARGET_IQ_SAMPLE_RATE * 0.3]; // 300ms窗口的频率数据

SSTV_State sstv_state = SSTV_STATE_IDLE; // 初始状态

int line_number = 0; // 当前行号

void sstv_handle_thread(void *arg)
{
    while(1){
        switch (sstv_state)
        {
        case SSTV_STATE_IDLE:
            
            sstv_state = SSTV_STATE_VIS_SYNC_SEARCH;
            break;
        case SSTV_STATE_VIS_SYNC_SEARCH:
            // 搜索视觉同步信号
            if(vis_sync_search()) {
                // 找到视觉同步信号，切换到视觉同步找到状态
                sstv_state = SSTV_STATE_VIS_SYNC_FOUND;
            }
            break;
        case SSTV_STATE_VIS_SYNC_FOUND:
            // 视觉同步信号已找到，跳过300ms数据
            get_frequency_buffer(frequency_buf_300ms, SSTV_TARGET_IQ_SAMPLE_RATE * 0.3);
            break;
        case SSTV_STATE_LINE_SYNC_SEARCH:
            // 搜索行同步信号
            if(line_sync_search()) {
                // 找到行同步信号，切换到行同步找到状态
                sstv_state = SSTV_STATE_LINE_SYNC_FOUND;
            }
            break;
        case SSTV_STATE_LINE_SYNC_FOUND:
            //跳过1ms数据
            get_frequency_buffer(frequency_buf_300ms, SSTV_TARGET_IQ_SAMPLE_RATE * 0.001);
            sstv_state = SSTV_STATE_SCAN_LINE;
            break;
        case SSTV_STATE_SCAN_LINE:
            line_scan(line_number); // 扫描行数据
            // line_number++;
            // if(line_number >= IMAGE_HEIGHT) {
            //     // 行扫描完成，切换到处理完成状态
            //     sstv_state = SSTV_SATE_DONE;
            // }
            break;
        case SSTV_SATE_SCAN_LINE_DONE:
            // 扫描行数据完成，准备处理奇偶行同步
            sstv_state = SSTV_STATE_PARITY_SYNC_SEARCH; // 重新搜索行同步信号
            break;
        case SSTV_STATE_PARITY_SYNC_SEARCH:
            int parity_result = parity_sync_search(); // 搜索奇偶行同步信号
            if(parity_result == 1){  // 找到偶数行同步信号
                sstv_state = SSTV_STATE_EVEN_SCAN_LINE; // 切换到奇偶行同步找到状态
            } else if(parity_result == 2) { // 找到奇数行同步信号
                sstv_state = SSTV_STATE_ODD_SCAN_LINE; // 切换到奇偶行同步找到状态
            }
            break;
        case SSTV_STATE_EVEN_SCAN_LINE:
            even_scan(line_number); // 扫描偶数行数据
            line_number++;
            if(line_number >= IMAGE_HEIGHT) {
                // 行扫描完成，切换到处理完成状态
                sstv_state = SSTV_SATE_DONE;
            }else {
                sstv_state = SSTV_STATE_LINE_SYNC_SEARCH; // 切换到行同步扫描状态
            }
            break;
        case SSTV_STATE_ODD_SCAN_LINE:
            odd_scan(line_number); // 扫描奇数行数据
            line_number++;
            if(line_number >= IMAGE_HEIGHT) {
                // 行扫描完成，切换到处理完成状态
                sstv_state = SSTV_SATE_DONE;
            }else {
                sstv_state = SSTV_STATE_LINE_SYNC_SEARCH; // 切换到扫描行同步扫描状态
            }
            break;
        case SSTV_STATE_DONE:
            // 处理完成，保存图像
            save_image();
            printf("SSTV processing completed.\r\n");
            printf("image saved at: %s\r\n", bmp_file_path);
            // 重置状态机
            sstv_state = SSTV_STATE_IDLE; // 重置状态机
            line_number = 0; // 重置行号
            break;
        default:
            break;
        }
    }
}