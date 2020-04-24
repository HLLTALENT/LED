#ifndef SPI_MASTER_USR_H
#define SPI_MASTER_USR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

extern spi_device_handle_t spi_w25q128;

extern void spi_readwrite_byte(spi_device_handle_t spi,uint8_t *write_byte,uint16_t write_len,uint8_t *read_byte,uint16_t read_len);
extern void spi_readwrite_bytes(spi_device_handle_t spi,uint8_t *write_bytes,uint16_t write_len,uint8_t *read_bytes,uint16_t read_len);
extern void spi_read_bytes(spi_device_handle_t spi,uint8_t *read_bytes,uint16_t read_len);
extern void spi_write_bytes(spi_device_handle_t spi,uint8_t *write_bytes,uint16_t write_len);

extern void spi_init(spi_device_handle_t *spi,int xMhz);
extern void spi_speed_set(spi_device_handle_t *spi,int xMhz);

extern uint8_t Spi_ReadByte(uint8_t read_byte);
extern void Spi_WriteByte(uint8_t write_byte);


#endif

