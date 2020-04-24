#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

#include "spi_master_usr.h"

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18

/*
//test evb1000 spi
#define PIN_NUM_MISO 5
#define PIN_NUM_MOSI 18
#define PIN_NUM_CLK  23

#define PIN_NUM_CS   21
*/
//To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
//but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
//#define PARALLEL_LINES 16

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
/*void spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    //gpio_set_level(PIN_NUM_DC, dc);
}*/
spi_device_handle_t spi_w25q128;
//------------------------
void spi_init(spi_device_handle_t *spi,int xMhz)
{
    esp_err_t ret;
    //spi_device_handle_t spi1;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        //.max_transfer_sz=PARALLEL_LINES*320*2+8
        .max_transfer_sz=128
    };
    spi_device_interface_config_t devcfg={
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz=xMhz*1000*1000,           //Clock out at 4 MHz
        .mode=0,                                //SPI mode 0
        .duty_cycle_pos = 128, //50% duty cycle
        //.spics_io_num=PIN_NUM_CS,               //CS pin
        .spics_io_num=-1,               //CS pin
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans=0,
        .queue_size=128                      //We want to be able to queue 7 transactions at a time
        //.pre_cb=spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the DW1000 to the SPI bus
    //ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi1);
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, spi);
    ESP_ERROR_CHECK(ret);
    printf("spi init ok \r\n");
}
//------------------------
void spi_speed_set(spi_device_handle_t *spi,int xMhz)
{
    esp_err_t ret;
    //spi_device_handle_t spi1;
    /*spi_bus_config_t buscfg ={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=PARALLEL_LINES*320*2+8
    };*/
    spi_device_interface_config_t devcfg={
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz=xMhz*1000*1000,           //Clock out at 4 MHz
        .mode=0,                                //SPI mode 0
        .duty_cycle_pos = 128, //50% duty cycle
        //.spics_io_num=PIN_NUM_CS,               //CS pin
        .spics_io_num=-1,               //CS pin
        .cs_ena_pretrans = 1,
        .cs_ena_posttrans=1,
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        //.pre_cb=spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };

    ret=spi_bus_remove_device(*spi);
    ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(VSPI_HOST, &devcfg, spi);
    ESP_ERROR_CHECK(ret);
    printf("spi speed change :%dMhz\r\n",xMhz);


}
void spi_readwrite_byte(spi_device_handle_t spi,uint8_t *write_byte,uint16_t write_len,uint8_t *read_byte,uint16_t read_len)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
        //spi_writebyte(0x00);
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));
        t.length = 8;
        //if(write_len == 0read_byte == NULL)
        if(write_len == 0)
        {
            t.flags = SPI_TRANS_USE_RXDATA;
            //t.rx_buffer = read_byte;
            //t.tx_buffer = write_byte;
            esp_err_t ret = spi_device_transmit(spi, &t);
            *read_byte = t.rx_data[0];
            assert(ret == ESP_OK);
        }
        else
        {
            t.flags = SPI_TRANS_USE_TXDATA;
            t.tx_data[0] = *write_byte;
            esp_err_t ret = spi_device_transmit(spi, &t);
            assert(ret == ESP_OK);
        }
   
}
void spi_readwrite_bytes(spi_device_handle_t spi,uint8_t *write_bytes,uint16_t write_len,uint8_t *read_bytes,uint16_t read_len)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    uint16_t i,len;
    len = (write_len>read_len)?write_len:read_len;
    for(i=0;i<len;i++)
    {
        spi_readwrite_byte(spi,&write_bytes[i],write_len,&read_bytes[i],read_len);
    }
   
}
void spi_write_byte(spi_device_handle_t spi,uint8_t *write_byte)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
        //spi_writebyte(0x00);
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));
        t.length = 8;

        t.flags = SPI_TRANS_USE_TXDATA;
        t.tx_data[0] = *write_byte;
        esp_err_t ret = spi_device_transmit(spi, &t);
        assert(ret == ESP_OK);
}
void spi_write_bytes(spi_device_handle_t spi,uint8_t *write_bytes,uint16_t write_len)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    uint16_t i,len;
    len = write_len;
    for(i=0;i<len;i++)
    {
        spi_write_byte(spi,&write_bytes[i]);
    }
}
void spi_read_byte(spi_device_handle_t spi,uint8_t *read_byte)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    //spi_writebyte(0x00);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.flags = SPI_TRANS_USE_RXDATA;
    //t.rx_buffer = read_byte;
    //t.tx_buffer = write_byte;
    esp_err_t ret = spi_device_transmit(spi, &t);
    *read_byte = t.rx_data[0];
    assert(ret == ESP_OK);
}
void spi_read_bytes(spi_device_handle_t spi,uint8_t *read_bytes,uint16_t read_len)
//void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    uint16_t i,len;
    len = read_len;
    for(i=0;i<len;i++)
    {
        spi_read_byte(spi,&read_bytes[i]);
    }
}

//by Jiahui
uint8_t Spi_ReadByte(uint8_t read_byte)
{
    //spi_writebyte(0x00);
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.flags = SPI_TRANS_USE_RXDATA;
    //t.rx_buffer = read_byte;
    //t.tx_buffer = write_byte;
    esp_err_t ret = spi_device_transmit(spi_w25q128, &t);
    read_byte = t.rx_data[0];
    assert(ret == ESP_OK);
    return read_byte;
}

void Spi_WriteByte(uint8_t write_byte)

{
    //spi_writebyte(0x00);
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));
        t.length = 8;

        t.flags = SPI_TRANS_USE_TXDATA;
        t.tx_data[0] = write_byte;
        esp_err_t ret = spi_device_transmit(spi_w25q128, &t);
        assert(ret == ESP_OK);
}


#if 0
#include "dw1000_driver.h"
void test_spi(void *p)
{
    uint8_t test[5] = {0x55,0x55,0xaa,0xaa,0x55};
    uint8_t test2[5];
    int cnt = 0;
    while(1)
    {
        vTaskDelay(2);
        //spi_writeburst(test,4);
        //spi_write_buf(spi_dw1000,test,4);
        //spi_readwrite_bytes(spi_dw1000,test,4,NULL,0);

         dw1000_init(0,0);

        
    }

}
void spi_dw1000_init(void)
{
    dw1000_io_init();
    //spi_device_handle_t spi
    spi_init(&spi_dw1000,8);
    dw1000_init(0,0);
    //xTaskCreate(test_spi, "test_spi", 4096, NULL, 3, NULL);
}
#endif
