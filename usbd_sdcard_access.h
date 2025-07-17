#ifndef USBD_SDCARD_ACCESS_H
#define USBD_SDCARD_ACCESS_H

#include "gd32f30x.h"

#define SDCARD_BLOCK_SIZE         512            /* 固定每个块大小为512 */
#define SDCARD_BLOCK_NUM          0              /* 动态获取，在初始化时填充 */

/* function declarations */
/* read data from multiple blocks of SD card */
uint32_t sdcard_multi_blocks_read(uint8_t *pbuf,
                                  uint32_t read_addr,
                                  uint16_t block_size,
                                  uint32_t block_num);

/* write data from multiple blocks of SD card */
uint32_t sdcard_multi_blocks_write(uint8_t *pbuf,
                                   uint32_t write_addr,
                                   uint16_t block_size,
                                   uint32_t block_num);

/* get SD card capacity */
uint32_t sdcard_get_capacity(void);

/* SDIO测试和日志记录功能 */
/* 执行SDIO写入测试并记录日志 */
uint32_t sdio_write_test_and_log(void);

/* 将日志写入到内部Flash(U盘) */
uint32_t write_log_to_udisk(const char* log_message);

/* 格式化时间戳 */
void format_timestamp(char* buffer, uint32_t size);

/* 初始化SDIO测试功能 */
void sdio_test_init(void);

/* 运行定期SDIO测试 */
void sdio_test_run_periodic(void);

/* 创建日志文件 */
uint32_t create_log_file(void);

#endif /* USBD_SDCARD_ACCESS_H */
