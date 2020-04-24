/*******************************************************************************
  * @file       W25Q128 NOR FLASH CHIP DRIVER APPLICATION      
  * @author 
  * @version
  * @date 
  * @brief
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/

/*-------------------------------- Includes ----------------------------------*/
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "stdlib.h"
#include "w25q128.h"
#include "spi_master_usr.h"
#include "PCF8563.h"
#include "MSS.h"

//extern OsiSyncObj_t  Spi_Mutex;  //Used for SPI Lock
//extern OsiSyncObj_t  xMutex5;  //Used for Post_Data_Buffer Lock

//extern char Post_Data_Buffer[4096];

#define PIN_NUM_CS 5

#define SET_SPI1_CS_ON() gpio_set_level(PIN_NUM_CS, 1)  //CS HIGH
#define SET_SPI1_CS_OFF() gpio_set_level(PIN_NUM_CS, 0) //CS LOW

#define SUCCESS
#define FAILURE

//static SemaphoreHandle_t w25q128_irq_xSemaphore = NULL;//用于中断同步
SemaphoreHandle_t w25q128_spi_xSemaphore = NULL;
SemaphoreHandle_t w25q128_irq_lock = NULL;

/*******************************************************************************
// Configures SPI Rate
// Return: None
*******************************************************************************/
/*void DECA_SPI_Config_Rate(int scalingfactor)
{
  xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );
  spi_speed_set(&spi_w25q128,scalingfactor);
  xSemaphoreGive( w25q128_spi_xSemaphore );
}*/

/*******************************************************************************
  SPI_CS init
*******************************************************************************/

void SPI_CS_init(void)
{
    //CS
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16
    io_conf.pin_bit_mask = (1 << PIN_NUM_CS);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

/*******************************************************************************
  nor flash spi start
*******************************************************************************/
static void Flash_Spi_Start(void)
{
    xSemaphoreTake(w25q128_spi_xSemaphore, portMAX_DELAY);
    SET_SPI1_CS_OFF(); //w25q128 spi cs enable
                       //MAP_SPIEnable(GSPI_BASE);  //enable the spi channel
}

/*******************************************************************************
  nor flash spi stop
*******************************************************************************/
static void Flash_Spi_Stop(void)
{
    //MAP_SPIDisable(GSPI_BASE);  //disable the spi channel
    SET_SPI1_CS_ON(); //w25q128 spi cs disable
    xSemaphoreGive(w25q128_spi_xSemaphore);
}

/*******************************************************************************
  w25q128 read register
*******************************************************************************/
uint8_t w25q_ReadReg(uint8_t com_val)
{
    uint8_t value;

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(com_val); //SPI_SendReciveByte(com_val);  //send the read command//发送写

    value = Spi_ReadByte(0x00); //value=SPI_SendReciveByte(0x00);  //clk signal,get register value//接收读

    Flash_Spi_Stop(); //nor flash spi stop

    return value;
}

/*******************************************************************************
  w25q128 write register
*******************************************************************************/
void w25q_WriteCommand(uint8_t com_val)
{
    Flash_Spi_Start(); //nor flash spi start/

    Spi_WriteByte(com_val); //SPI_SendReciveByte(com_val);  //send the write command//发送

    Flash_Spi_Stop(); //nor flash spi stop
}

/*******************************************************************************
  waite write completed
*******************************************************************************/
static void w25q_WaitCompleted(void)
{
    uint16_t retry = 0;

    while (w25q_ReadReg(READ_STATUS_REGISTER) & 0x01) //read status register,bit0=0:Ready,bit0=1:Busy
    {
        if (retry++ > 60000) //time out 1.5min
        {
            break;
        }
        vTaskDelay(1.5 / portTICK_RATE_MS); //delay_us(1500);//MAP_UtilsDelay(20000);  //delay about 1.5ms
                                            //delay_us(1500);//MAP_UtilsDelay(20000);  //delay about 1.5ms
    }
}

/*******************************************************************************
  w25q128 write register
*******************************************************************************/
void w25q_WriteReg(uint8_t com_val, uint8_t value)
{
    w25q_WriteCommand(WRITE_ENABLE); //write enable

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(com_val); //SPI_SendReciveByte(com_val);  //send the write register command//发送

    Spi_WriteByte(value); //SPI_SendReciveByte(value);     //write register value//

    Flash_Spi_Stop(); //nor flash spi stop

    w25q_WaitCompleted(); //waite command completed
}

/*******************************************************************************
// w25q128 read id
*******************************************************************************/
uint16_t w25q_ReadId(void)
{
    uint16_t w25qid;

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(W25Q_DEVICE_ID); //SPI_SendReciveByte(W25Q_DEVICE_ID);  //send the read manufacturer id

    Spi_WriteByte(0x00); //SPI_SendReciveByte(0x00);  //recive data

    Spi_WriteByte(0x00); //SPI_SendReciveByte(0x00);  //recive data

    Spi_WriteByte(0x00); //SPI_SendReciveByte(0x00);  //recive data

    w25qid = Spi_ReadByte(0x00); //SPI_SendReciveByte(0x00);  //recive data

    w25qid = w25qid << 8;

    w25qid += Spi_ReadByte(0x00); //SPI_SendReciveByte(0x00);  //recive data

    Flash_Spi_Stop(); //nor flash spi stop

    return w25qid;
}

/*******************************************************************************
// init memory ic
*******************************************************************************/
int w25q_Init(void)
{
    if (w25q_ReadId() == W25Q128)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
  w25q128 read data
*******************************************************************************/
void w25q_ReadData(uint32_t addr, char *buffer, uint8_t size)
{
    uint8_t i;

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(READ_DATA); //SPI_SendReciveByte(READ_DATA);  //read operations code

    Spi_WriteByte((uint8_t)((addr) >> 16)); //SPI_SendReciveByte((uint8_t)((addr)>>16));  //24bit address first 8 bit address

    Spi_WriteByte((uint8_t)((addr) >> 8)); //SPI_SendReciveByte((uint8_t)((addr)>>8));

    Spi_WriteByte((uint8_t)(addr)); //SPI_SendReciveByte((uint8_t)(addr));  //24bit address last 8 bit address

    for (i = 0; i < size; i++)
    {
        buffer[i] = Spi_ReadByte(0x00); //SPI_SendReciveByte(0x00);  //read one byte

        //if(buffer[i]=='!')  //end with '!'
        //{
        //buffer[i]='\0';  //Read completed

        //*read_size=i+1;

        // Flash_Spi_Stop();  //nor flash spi stop

        //if(buffer[0]=='{')
        //{
        //return SUCCESS;
        //}
        //else
        //{
        //return FAILURE;
        //}
        //}
    }

    Flash_Spi_Stop(); //nor flash spi stop
}
//*read_size=size;

//return FAILURE;

/*******************************************************************************
  w25q128 write data
*******************************************************************************/
static void w25q_WritePage(uint32_t addr, char *buffer, uint8_t Size)
{
    uint8_t i;

    w25q_WriteCommand(WRITE_ENABLE); //write enable

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(PAGE_PROGRAM); //SPI_SendReciveByte(PAGE_PROGRAM);  //page program code

    Spi_WriteByte((uint8_t)(addr >> 16)); //SPI_SendReciveByte((uint8_t)(addr>>16));   //24bit address first 8 bit address

    Spi_WriteByte((uint8_t)(addr >> 8)); //SPI_SendReciveByte((uint8_t)(addr>>8));

    Spi_WriteByte((uint8_t)addr); //SPI_SendReciveByte((uint8_t)addr);	//24bit address last 8 bit address

    for (i = 0; i < Size; i++)
    {
        Spi_WriteByte(buffer[i]); //SPI_SendReciveByte(buffer[i]);  //write data
    }

    //if(end_flag)
    //{
    //Spi_WriteByte('!');//SPI_SendReciveByte('!');  //write End Flag
    //}

    Flash_Spi_Stop(); //nor flash spi stop

    w25q_WaitCompleted(); //w25q128 wait command completed
}

/*******************************************************************************
  w25q128 write data
*******************************************************************************/
void w25q_WriteData(uint32_t addr, char *buffer, uint8_t Size)
{
    uint8_t remain;

    remain = 256 - addr % 256; //page remain byte number

    if ((Size + 1) <= remain) //can write completed in current page
    {
        w25q_WritePage(addr, buffer, Size); //write completed in current page
    }
    else
    {
        w25q_WritePage(addr, buffer, remain); //can not write completed in current page

        w25q_WritePage(addr + remain, buffer + remain, Size - remain); //write completed in current page
    }
}

/*******************************************************************************
  w25q128 erase subsector-4k
*******************************************************************************/
void w25q_EraseSubsector(uint32_t addr)
{
    w25q_WriteCommand(WRITE_ENABLE); //write enable

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(SECTOR_ERASE); //SPI_SendReciveByte(SECTOR_ERASE);  //subsector eaase code

    Spi_WriteByte((uint8_t)((addr) >> 16)); //SPI_SendReciveByte((uint8_t)((addr)>>16));  //24bit address first 8 bit address

    Spi_WriteByte((uint8_t)((addr) >> 8)); //SPI_SendReciveByte((uint8_t)((addr)>>8));

    Spi_WriteByte((uint8_t)(addr)); //SPI_SendReciveByte((uint8_t)(addr));  //24bit address last 8 bit address

    Flash_Spi_Stop(); //nor flash spi stop

    w25q_WaitCompleted(); //wait command completed
}

/*******************************************************************************
  w25q128 erase chip
*******************************************************************************/
void w25q_EraseChip(void)
{
    w25q_WriteCommand(WRITE_ENABLE); //write enable

    w25q_WriteCommand(CHIP_ERASE); //chip eaase code

    w25q_WaitCompleted(); //wait command completed
}

/*******************************************************************************
  w25q128 power down mode
*******************************************************************************/
void w25q_PowerDown(void)
{
    w25q_WriteCommand(POWER_DOWN); //power down code

    delay_us(30); //MAP_UtilsDelay(400);  //delay about 30us
}

/*******************************************************************************
  w25q128 wake up
*******************************************************************************/
void w25q_WakeUp(void)
{
    w25q_WriteCommand(RELEASE_POWER_DOWN); //release power down code

    delay_us(30); //MAP_UtilsDelay(400);  //delay about 30us
}

/*******************************************************************************
  w25q128 read data
*******************************************************************************/
void w25q_Read_Data(uint32_t addr, char *buffer, uint16_t size)
{
    uint16_t i;

    Flash_Spi_Start(); //nor flash spi start

    Spi_WriteByte(READ_DATA); //SPI_SendReciveByte(READ_DATA);  //read operations code

    Spi_WriteByte((uint8_t)((addr) >> 16)); //SPI_SendReciveByte((uint8_t)((addr)>>16));   //24bit address first 8 bit address

    Spi_WriteByte((uint8_t)((addr) >> 8)); //SPI_SendReciveByte((uint8_t)((addr)>>8));

    Spi_WriteByte((uint8_t)(addr)); //SPI_SendReciveByte((uint8_t)(addr));  //24bit address last 8 bit address

    for (i = 0; i < size; i++)
    {
        buffer[i] = Spi_ReadByte(0x00); //SPI_SendReciveByte(0x00);	//read one byte
    }

    Flash_Spi_Stop(); //nor flash spi stop
}

/*******************************************************************************
  w25q128 write data no check
*******************************************************************************/
void w25q_Write_Data(uint32_t addr, char *buffer, uint16_t Size)
{
    uint8_t i, n_i;
    uint16_t remain;

    n_i = Size / 256 + 2;

    remain = 256 - addr % 256; //page remain byte number

    remain = remain > Size ? Size : remain;

    for (i = 0; i <= n_i; i++)
    {
        if (remain > 0)
        {
            w25q_WritePage(addr, buffer, remain); //no end flag
        }

        if (remain == Size)
        {
            break;
        }

        buffer += remain;

        addr += remain;

        Size -= remain;

        remain = Size > 256 ? 256 : Size;
    }
}

void w25q128xSemaphore_init(void)
{
    w25q128_spi_xSemaphore = xSemaphoreCreateMutex();
    w25q128_irq_lock = xSemaphoreCreateMutex();
}
/*******************************************************************************
//w25q128 check write address with locked
*******************************************************************************/
void osi_w25q_Write_Addr_Check(unsigned long w_Addr)
{
    uint8_t i;
    uint16_t sec_data;
    uint16_t sec_remain;
    unsigned long sec_addr;
    char read_buf[SAVE_DATA_SIZE];

    sec_addr = (w_Addr / 4096) * 4096;

    sec_data = w_Addr % 4096;

    sec_remain = 4096 - sec_data;

    //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );
    //osi_SyncObjWait(&Spi_Mutex,OSI_WAIT_FOREVER);  //SPI Semaphore Take

    w25q_Read_Data(w_Addr, read_buf, SAVE_DATA_SIZE); //read the data write area

    //osi_SyncObjSignal(&Spi_Mutex);  //SPI Semaphore Give
    //xSemaphoreGive( w25q128_spi_xSemaphore );

    for (i = 0; i < SAVE_DATA_SIZE; i++)
    {
        if (read_buf[i] != 0xff) //need to delete
        {
            if (sec_data > 0)
            {
                //osi_SyncObjWait(&xMutex5,OSI_WAIT_FOREVER);  //Post_Data_Buffer Semaphore Take
                //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );

                //memset(Post_Data_Buffer,'\0',sizeof(Post_Data_Buffer));  //clear the data buffer

                //osi_SyncObjWait(&Spi_Mutex,OSI_WAIT_FOREVER);  //SPI Semaphore Take
                //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );

                //w25q_Read_Data(sec_addr,Post_Data_Buffer,sec_data);

                //w25q_EraseSubsector(sec_addr);  //erase the subsector

                //w25q_Write_Data(sec_addr,Post_Data_Buffer,sec_data);

                if (sec_remain < SAVE_DATA_SIZE)
                {
                    w25q_EraseSubsector(sec_addr + 4096); //erase the subsector
                }

                //osi_SyncObjSignal(&Spi_Mutex);  //SPI Semaphore Give
                //xSemaphoreGive( w25q128_spi_xSemaphore );
                //xSemaphoreGive( w25q128_spi_xSemaphore );
                //osi_SyncObjSignal(&xMutex5);  //Post_Data_Buffer Semaphore Give
            }
            else
            {
                //osi_SyncObjWait(&Spi_Mutex,OSI_WAIT_FOREVER);  //SPI Semaphore Take
                //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );

                w25q_EraseSubsector(sec_addr); //erase the subsector

                //osi_SyncObjSignal(&Spi_Mutex);  //SPI Semaphore Give
                //xSemaphoreGive( w25q128_spi_xSemaphore );
            }

            break;
        }
    }
}

/*******************************************************************************
                                      END         
*******************************************************************************/
