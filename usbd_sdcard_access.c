#include "usbd_sdcard_access.h"
#include "../../inc/sdcard.h"
#include "gd32f303e_eval.h"  // 使用正确的GD32F303E评估板头文件
#include "../../inc/internal_flash_if.h"  // 引入内部Flash接口
#include <stdio.h>
#include <string.h>

/*!
    \brief      read data from multiple blocks of SD card
    \param[in]  pbuf: pointer to user buffer
    \param[in]  read_addr: address to be read
    \param[in]  block_size: size of block
    \param[in]  block_num: number of block
    \param[out] none
    \retval     operation status
*/
uint32_t sdcard_multi_blocks_read(uint8_t *pbuf,
                                  uint32_t read_addr,
                                  uint16_t block_size,
                                  uint32_t block_num)
{
    /* 红色LED闪烁指示读操作 */
    gd_eval_led_on(LED2);  // 红色LED点亮，指示开始读取
    
    /* 检查地址和缓冲区对齐 */
    if ((read_addr % 512) != 0 || ((uint32_t)pbuf % 4) != 0) {
        /* 地址或缓冲区未对齐，读取失败 */
        gd_eval_led_off(LED2);
        return 1;
    }
    
    /* read_addr 已经是字节地址，SD 函数需要字节地址 */
    if (block_num == 1) {
        /* sd_block_read(buffer, addr_in_bytes, block_size_512) */
        if (SD_OK == sd_block_read((uint32_t *)pbuf, read_addr, 512)) {
            gd_eval_led_off(LED2);  // 读取成功，关闭LED
            return 0;
        }
    } else {
        /* sd_multiblocks_read(buffer, addr_in_bytes, block_size_512, num_blocks) */
        if (SD_OK == sd_multiblocks_read((uint32_t *)pbuf, read_addr, 512, block_num)) {
            gd_eval_led_off(LED2);  // 读取成功，关闭LED
            return 0;
        }
    }
    
    /* 读取失败，LED快速闪烁3次 */
    
    for(int i = 0; i < 3; i++) {
        gd_eval_led_off(LED2);
        for(volatile int j = 0; j < 100000; j++);  // 简单延时
        gd_eval_led_on(LED2);
        for(volatile int j = 0; j < 100000; j++);
    }
    gd_eval_led_off(LED2);
    
    return 1;
}

/*!
    \brief      write data from multiple blocks of SD card
    \param[in]  pbuf: pointer to user buffer
    \param[in]  write_addr: address to be write
    \param[in]  block_size: size of block
    \param[in]  block_num: number of block
    \param[out] none
    \retval     operation status
*/
uint32_t sdcard_multi_blocks_write(uint8_t *pbuf,
                                   uint32_t write_addr,
                                   uint16_t block_size,
                                   uint32_t block_num)
{
    /* 红色LED长亮指示写操作（写操作比读操作LED亮时间更长） */
    gd_eval_led_on(LED2);
    
    /* 检查地址和缓冲区对齐 */
    if ((write_addr % 512) != 0 || ((uint32_t)pbuf % 4) != 0) {
        /* 地址或缓冲区未对齐，写入失败 */
        gd_eval_led_off(LED2);
        return 1;
    }
    
    /* write_addr 已经是字节地址，SD 函数需要字节地址 */
    if (block_num == 1) {
        /* sd_block_write(buffer, addr_in_bytes, block_size_512) */
        if (SD_OK == sd_block_write((uint32_t *)pbuf, write_addr, 512)) {
            /* 写入成功，延时后关闭LED */
            for(volatile int j = 0; j < 200000; j++);  // 写操作延时更长
            gd_eval_led_off(LED2);
            return 0;
        }
    } else {
        /* sd_multiblocks_write(buffer, addr_in_bytes, block_size_512, num_blocks) */
        if (SD_OK == sd_multiblocks_write((uint32_t *)pbuf, write_addr, 512, block_num)) {
            /* 写入成功，延时后关闭LED */
            for(volatile int j = 0; j < 200000; j++);  // 写操作延时更长
            gd_eval_led_off(LED2);
            return 0;
        }
    }
    
    /* 写入失败，LED慢速闪烁5次 */
    for(int i = 0; i < 5; i++) {
        gd_eval_led_off(LED2);
        for(volatile int j = 0; j < 200000; j++);  // 失败时延时更长
        gd_eval_led_on(LED2);
        for(volatile int j = 0; j < 200000; j++);
    }
    gd_eval_led_off(LED2);
    
    return 1;
}

/*!
    \brief      get SD card capacity
    \param[in]  none
    \param[out] none
    \retval     SD card capacity in blocks
*/
uint32_t sdcard_get_capacity(void)
{
    /* sd_card_capacity_get() 返回的是 KB，128MB = 131072 KB */
    uint32_t capacity_kb = sd_card_capacity_get();
    uint32_t capacity_blocks = (capacity_kb * 1024) / 512;  // KB转字节再除以512得到块数
    
    /* 128MB SD卡应该返回约 262144 块 (128*1024*1024/512) */
    /* 添加LED指示容量范围：
     * LED闪烁1次: < 64MB
     * LED闪烁2次: 64-256MB  <- 128MB应该在这里
     * LED闪烁3次: > 256MB
     */
    if (capacity_blocks < 131072) {        // < 64MB
        gd_eval_led_toggle(LED3);  // 使用LED3代替LED1
        for(volatile int j = 0; j < 500000; j++);
    } else if (capacity_blocks < 524288) { // 64-256MB
        for(int i = 0; i < 2; i++) {
            gd_eval_led_toggle(LED3);  // 使用LED3代替LED1
            for(volatile int j = 0; j < 300000; j++);
        }
    } else {                               // > 256MB
        for(int i = 0; i < 3; i++) {
            gd_eval_led_toggle(LED3);  // 使用LED3代替LED1
            for(volatile int j = 0; j < 300000; j++);
        }
    }
    
    return capacity_blocks;
}

/*!
    \brief      执行SDIO写入测试并记录日志
    \param[in]  none
    \param[out] none
    \retval     测试结果: 0=成功, 1=失败
*/
uint32_t sdio_write_test_and_log(void)
{
    uint8_t test_data[512];
    uint8_t read_back_data[512];
    uint32_t test_address = 0x1000;  // 测试地址(第8个扇区)
    char log_buffer[512];
    char timestamp[32];
    uint32_t result = 0;
    
    /* 格式化时间戳 */
    format_timestamp(timestamp, sizeof(timestamp));
    
    /* 准备测试数据 */
    for(int i = 0; i < 512; i++) {
        test_data[i] = (uint8_t)(i & 0xFF);
    }
    
    /* 开始测试，LED2亮起表示正在测试 */
    gd_eval_led_on(LED2);
    
    /* 步骤1: 写入测试数据 */
    if(sdcard_multi_blocks_write(test_data, test_address, 512, 1) != 0) {
        /* 写入失败 */
        result = 1;
        strcpy(log_buffer, "[");
        strcat(log_buffer, timestamp);
        strcat(log_buffer, "] SDIO Write Test FAILED - Write operation failed\r\n");
        
        /* LED2快速闪烁表示写入失败 */
        for(int i = 0; i < 5; i++) {
            gd_eval_led_off(LED2);
            for(volatile int j = 0; j < 100000; j++);
            gd_eval_led_on(LED2);
            for(volatile int j = 0; j < 100000; j++);
        }
        gd_eval_led_off(LED2);
        
        /* 记录错误日志 */
        write_log_to_udisk(log_buffer);
        return result;
    }
    
    /* 步骤2: 读取并验证数据 */
    if(sdcard_multi_blocks_read(read_back_data, test_address, 512, 1) != 0) {
        /* 读取失败 */
        result = 1;
        strcpy(log_buffer, "[");
        strcat(log_buffer, timestamp);
        strcat(log_buffer, "] SDIO Write Test FAILED - Read operation failed\r\n");
        
        /* LED2快速闪烁表示读取失败 */
        for(int i = 0; i < 5; i++) {
            gd_eval_led_off(LED2);
            for(volatile int j = 0; j < 100000; j++);
            gd_eval_led_on(LED2);
            for(volatile int j = 0; j < 100000; j++);
        }
        gd_eval_led_off(LED2);
        
        /* 记录错误日志 */
        write_log_to_udisk(log_buffer);
        return result;
    }
    
    /* 步骤3: 验证数据完整性 */
    for(int i = 0; i < 512; i++) {
        if(test_data[i] != read_back_data[i]) {
            /* 数据验证失败 */
            result = 1;
            strcpy(log_buffer, "[");
            strcat(log_buffer, timestamp);
            strcat(log_buffer, "] SDIO Write Test FAILED - Data verification failed\r\n");
            
            /* LED2慢速闪烁表示数据验证失败 */
            for(int j = 0; j < 3; j++) {
                gd_eval_led_off(LED2);
                for(volatile int k = 0; k < 200000; k++);
                gd_eval_led_on(LED2);
                for(volatile int k = 0; k < 200000; k++);
            }
            gd_eval_led_off(LED2);
            
            /* 记录错误日志 */
            write_log_to_udisk(log_buffer);
            return result;
        }
    }
    
    /* 测试成功 */
    gd_eval_led_off(LED2);
    strcpy(log_buffer, "[");
    strcat(log_buffer, timestamp);
    strcat(log_buffer, "] SDIO Write Test PASSED - Successfully completed\r\n");
    
    /* 记录成功日志 */
    write_log_to_udisk(log_buffer);
    
    return result;
}

/*!
    \brief      将日志写入到内部Flash(U盘)
    \param[in]  log_message: 要写入的日志消息
    \param[out] none
    \retval     写入结果: 0=成功, 1=失败
*/
uint32_t write_log_to_udisk(const char* log_message)
{
    /* 定义日志文件在内部Flash中的位置和大小 */
    #define LOG_FILE_START_BLOCK    100     // 从第100个512字节块开始
    #define LOG_FILE_MAX_BLOCKS     20      // 最多占用20个块(10KB)
    #define LOG_BLOCK_SIZE          512     // 每个块的大小
    
    static uint32_t current_block = LOG_FILE_START_BLOCK;
    static uint32_t current_offset = 0;
    
    uint32_t message_len = strlen(log_message);
    uint8_t temp_buffer[LOG_BLOCK_SIZE];
    uint32_t write_result = 0;
    
    /* LED4亮起表示开始写入日志 */
    gd_eval_led_on(LED4);
    
    /* 检查是否需要换到新的块 */
    if(current_offset + message_len >= LOG_BLOCK_SIZE) {
        /* 当前块空间不够，移到下一个块 */
        current_block++;
        current_offset = 0;
        
        /* 检查是否超出日志区域 */
        if(current_block >= LOG_FILE_START_BLOCK + LOG_FILE_MAX_BLOCKS) {
            /* 日志区域已满，从头开始 */
            current_block = LOG_FILE_START_BLOCK;
            current_offset = 0;
        }
    }
    
    /* 读取当前块的内容 */
    memset(temp_buffer, 0, LOG_BLOCK_SIZE);
    uint32_t block_address = current_block * LOG_BLOCK_SIZE;
    
    /* 尝试读取现有内容 */
    if(flash_if_read(temp_buffer, block_address, LOG_BLOCK_SIZE) != 0) {
        /* 读取失败，清空缓冲区 */
        memset(temp_buffer, 0, LOG_BLOCK_SIZE);
    }
    
    /* 将新日志消息添加到缓冲区 */
    if(current_offset + message_len < LOG_BLOCK_SIZE) {
        memcpy(&temp_buffer[current_offset], log_message, message_len);
        current_offset += message_len;
        
        /* 写入到Flash */
        if(flash_if_write(temp_buffer, block_address, LOG_BLOCK_SIZE) == 0) {
            write_result = 0;  // 成功
        } else {
            write_result = 1;  // 失败
        }
    } else {
        write_result = 1;  // 缓冲区溢出
    }
    
    /* 根据写入结果控制LED */
    if(write_result == 0) {
        /* 写入成功 - LED4短暂闪烁 */
        for(volatile int i = 0; i < 30000; i++);
        gd_eval_led_off(LED4);
    } else {
        /* 写入失败 - LED4快速闪烁3次 */
        for(int i = 0; i < 3; i++) {
            gd_eval_led_off(LED4);
            for(volatile int j = 0; j < 20000; j++);
            gd_eval_led_on(LED4);
            for(volatile int j = 0; j < 20000; j++);
        }
        gd_eval_led_off(LED4);
    }
    
    return write_result;
}

/*!
    \brief      格式化时间戳
    \param[in]  buffer: 输出缓冲区
    \param[in]  size: 缓冲区大小
    \param[out] none
    \retval     none
*/
void format_timestamp(char* buffer, uint32_t size)
{
    /* 简单的时间戳格式，基于静态计数器
     * 在实际应用中，您可以使用RTC模块获取真实时间
     */
    static uint32_t timestamp_counter = 0;
    timestamp_counter++;
    
    /* 简单的字符串格式化，避免使用snprintf */
    const char* prefix = "T";
    uint32_t i = 0;
    
    /* 复制前缀 */
    buffer[i++] = prefix[0];
    
    /* 添加计数器数字 */
    uint32_t temp = timestamp_counter;
    uint32_t divisor = 10000;
    uint8_t started = 0;
    
    while(divisor > 0 && i < size - 1) {
        uint32_t digit = temp / divisor;
        if(digit > 0 || started || divisor == 1) {
            buffer[i++] = '0' + digit;
            started = 1;
        }
        temp %= divisor;
        divisor /= 10;
    }
    
    buffer[i] = '\0';
}

/*!
    \brief      初始化SDIO测试功能
    \param[in]  none
    \param[out] none
    \retval     none
*/
void sdio_test_init(void)
{
    /* 初始化LED指示灯 */
    gd_eval_led_init(LED3);  // 容量指示LED
    gd_eval_led_init(LED2);  // 读写操作指示LED
    gd_eval_led_init(LED4);  // 日志写入指示LED
    gd_eval_led_init(LED5);  // 状态指示LED
    
    /* 初始化完成，LED5闪烁一次 */
    gd_eval_led_on(LED5);
    for(volatile int i = 0; i < 200000; i++);
    gd_eval_led_off(LED5);
    
    /* 创建日志文件 */
    if(create_log_file() == 0) {
        /* 写入初始化日志 */
        write_log_to_udisk("[INIT] SDIO Test Module Initialized Successfully\r\n");
    } else {
        /* 创建日志文件失败，LED2长亮表示错误 */
        gd_eval_led_on(LED2);
    }
}

/*!
    \brief      运行定期SDIO测试
    \param[in]  none
    \param[out] none
    \retval     none
*/
void sdio_test_run_periodic(void)
{
    static uint32_t test_counter = 0;
    test_counter++;
    
    /* LED5亮起表示开始测试周期 */
    gd_eval_led_on(LED5);
    
    /* 执行SDIO写入测试 */
    uint32_t test_result = sdio_write_test_and_log();
    
    /* 根据测试结果控制LED */
    if(test_result == 0) {
        /* 测试成功 - LED5慢速闪烁2次 */
        for(int i = 0; i < 2; i++) {
            gd_eval_led_off(LED5);
            for(volatile int j = 0; j < 150000; j++);
            gd_eval_led_on(LED5);
            for(volatile int j = 0; j < 150000; j++);
        }
        gd_eval_led_off(LED5);
    } else {
        /* 测试失败 - LED2长亮表示错误状态 */
        gd_eval_led_on(LED2);
        /* LED5快速闪烁5次表示测试失败 */
        for(int i = 0; i < 5; i++) {
            gd_eval_led_off(LED5);
            for(volatile int j = 0; j < 50000; j++);
            gd_eval_led_on(LED5);
            for(volatile int j = 0; j < 50000; j++);
        }
        gd_eval_led_off(LED5);
    }
    
    /* 每10次测试显示一次SD卡容量信息 */
    if(test_counter % 10 == 0) {
        sdcard_get_capacity();
    }
}

/*!
    \brief      创建日志文件
    \param[in]  none
    \param[out] none
    \retval     创建结果: 0=成功, 1=失败
*/
uint32_t create_log_file(void)
{
    /* 简单的FAT文件系统结构创建 */
    uint8_t file_buffer[512];
    char file_header[] = "SDIO Test Log File\r\n"
                         "Created by GD32F303E SDIO Test Module\r\n"
                         "Log Format: [Timestamp] Message\r\n"
                         "========================================\r\n";
    
    /* 清空缓冲区 */
    memset(file_buffer, 0, 512);
    
    /* 复制文件头 */
    uint32_t header_len = strlen(file_header);
    if(header_len < 512) {
        memcpy(file_buffer, file_header, header_len);
    }
    
    /* LED5亮起表示正在创建日志文件 */
    gd_eval_led_on(LED5);
    
    /* 将文件头写入到日志区域的第一个块 */
    #define LOG_FILE_START_BLOCK    100
    uint32_t block_address = LOG_FILE_START_BLOCK * 512;
    
    uint32_t result = 0;
    if(flash_if_write(file_buffer, block_address, 512) != 0) {
        result = 1;  // 创建失败
        
        /* LED5快速闪烁表示创建失败 */
        for(int i = 0; i < 5; i++) {
            gd_eval_led_off(LED5);
            for(volatile int j = 0; j < 30000; j++);
            gd_eval_led_on(LED5);
            for(volatile int j = 0; j < 30000; j++);
        }
    } else {
        /* LED5慢速闪烁表示创建成功 */
        for(int i = 0; i < 2; i++) {
            gd_eval_led_off(LED5);
            for(volatile int j = 0; j < 100000; j++);
            gd_eval_led_on(LED5);
            for(volatile int j = 0; j < 100000; j++);
        }
    }
    
    gd_eval_led_off(LED5);
    return result;
}
