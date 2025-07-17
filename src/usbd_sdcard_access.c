#include "../inc/usbd_sdcard_access.h"
#include "sdcard.h"

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
    if (block_num == 1) {
        if (SD_OK == sd_block_read((uint32_t *)pbuf, read_addr, block_size)) {
            return 0;
        }
    } else {
        if (SD_OK == sd_multiblocks_read((uint32_t *)pbuf, read_addr, block_size, block_num)) {
            return 0;
        }
    }
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
    if (block_num == 1) {
        if (SD_OK == sd_block_write((uint32_t *)pbuf, write_addr, block_size)) {
            return 0;
        }
    } else {
        if (SD_OK == sd_multiblocks_write((uint32_t *)pbuf, write_addr, block_size, block_num)) {
            return 0;
        }
    }
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
    return sd_card_capacity_get();
}
