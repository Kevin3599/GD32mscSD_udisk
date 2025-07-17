#include "usbd_sdcard_access.h"
#include "../../inc/sdcard.h"
#include "gd32f303e_eval.h"  // 添加LED控制头文件

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
        gd_eval_led_toggle(LED5);
        for(volatile int j = 0; j < 500000; j++);
    } else if (capacity_blocks < 524288) { // 64-256MB
        for(int i = 0; i < 2; i++) {
            gd_eval_led_toggle(LED5);
            for(volatile int j = 0; j < 300000; j++);
        }
    } else {                               // > 256MB
        for(int i = 0; i < 3; i++) {
            gd_eval_led_toggle(LED5);
            for(volatile int j = 0; j < 300000; j++);
        }
    }
    
    return capacity_blocks;
}
