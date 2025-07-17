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

#endif /* USBD_SDCARD_ACCESS_H */
