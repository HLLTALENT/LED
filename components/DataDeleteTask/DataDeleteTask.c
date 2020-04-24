/*******************************************************************************
  * @file       Nor Flash Data Deleted Application Task   
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdlib.h>
#include "driver/gpio.h"
#include "w25q128.h"
#include "DataDeleteTask.h"
#include "DataSaveTask.h"
#include "spi_master_usr.h"
#include "ht9b95a.h"
#include "E2prom.h"

//extern OsiSyncObj_t     Spi_Mutex;      //Used for SPI Lock
//extern OsiSyncObj_t     Dlete_Binary;   //For Memory Delete Task
unsigned long DELETE_ADDR;
unsigned long WRITE_ADDR;
unsigned long POST_NUM;
unsigned long POST_ADDR;
unsigned long SAVE_ADDR_SPACE;
extern short flash_set_val;
uint8_t zerobuf[256] = "\0";
#define TEST_NUM 0xABCDFFFF

char read_buffer1[100];
char read_buffer2[100];
char read_buffer3[100];
char read_buffer4[100];
unsigned long write_address;
unsigned long delete_address;
unsigned long postnum_address;
unsigned long post_address;
unsigned long save_addr;

/*******************************************************************************
  Read Post Data Amount/Write Data/Post Data/Delete Data Address
*******************************************************************************/
void at24c08_read_addr(void)
{
    if (SAVE_ADDR_SPACE == 0)
    {
        E2prom_Read(0x20, (uint8_t *)read_buffer1, 100);
        memcpy(&write_address, read_buffer1, 4);

        E2prom_Read(0x30, (uint8_t *)read_buffer2, 100);
        memcpy(&delete_address, read_buffer2, 4);

        E2prom_Read(0x40, (uint8_t *)read_buffer3, 100);
        memcpy(&postnum_address, read_buffer3, 4);

        E2prom_Read(0x50, (uint8_t *)read_buffer4, 100);
        memcpy(&post_address, read_buffer4, 4);

        POST_NUM = postnum_address; //POST_NUM = at24c08_read(DATA_AMOUNT_ADDR1);  //read data post amount

        WRITE_ADDR = write_address; //WRITE_ADDR = at24c08_read(DATA_WRITE_ADDR1);  //read data write address

        POST_ADDR = post_address; //POST_ADDR = at24c08_read(DATA_POST_ADDR1);  //read data post address

        DELETE_ADDR = delete_address; //DELETE_ADDR = at24c08_read(DATA_DELETE_ADDR1);  //read data delete address
    }
    else
    {
        E2prom_Read(0x60, (uint8_t *)read_buffer1, 100);
        memcpy(&write_address, read_buffer1, 4);

        E2prom_Read(0x70, (uint8_t *)read_buffer2, 100);
        memcpy(&delete_address, read_buffer2, 4);

        E2prom_Read(0x80, (uint8_t *)read_buffer3, 100);
        memcpy(&postnum_address, read_buffer3, 4);

        E2prom_Read(0x90, (uint8_t *)read_buffer4, 100);
        memcpy(&post_address, read_buffer4, 4);

        POST_NUM = postnum_address; //POST_NUM = at24c08_read(DATA_AMOUNT_ADDR2);  //read data post amount

        WRITE_ADDR = write_address; //WRITE_ADDR = at24c08_read(DATA_WRITE_ADDR2);  //read data write address

        POST_ADDR = post_address; //POST_ADDR = at24c08_read(DATA_POST_ADDR2);  //read data post address

        DELETE_ADDR = delete_address; //DELETE_ADDR = at24c08_read(DATA_DELETE_ADDR2);  //read data delete address
    }
}

/*******************************************************************************
  Read Post Data Amount/Write Data/Post Data/Delete Data Address whit locked
*******************************************************************************/
void osi_at24c08_read_addr(void)
{
    //osi_TaskDisable();  //disable the tasks

    at24c08_read_addr();

    //osi_TaskEnable(0);  //enable all tasks
}

/*******************************************************************************
// save address test
*******************************************************************************/
static void save_addr_test(void)
{
    POST_NUM = TEST_NUM;
    WRITE_ADDR = TEST_NUM;
    POST_ADDR = TEST_NUM;
    DELETE_ADDR = TEST_NUM;

    osi_at24c08_save_addr(); //save

    osi_at24c08_read_addr(); //read

    if ((POST_NUM != TEST_NUM) || (WRITE_ADDR != TEST_NUM) || (POST_ADDR != TEST_NUM) || (DELETE_ADDR != TEST_NUM))
    {
        SAVE_ADDR_SPACE = 0x01;
    }
    else
    {
        SAVE_ADDR_SPACE = 0;
    }

    memcpy(&save_addr, &SAVE_ADDR_SPACE, 4);
    E2prom_Write(0xA0, (uint8_t *)zerobuf, 256);
    E2prom_Write(0xA0, (uint8_t *)save_addr, 256);
    //osi_at24c08_write_byte(DATA_SAVE_FLAG_ADDR,SAVE_ADDR_SPACE);  //save save addr flag
}
/*******************************************************************************
 Save Post Data Amount/Write Data/Post Data/Delete Data Address
*******************************************************************************/
void at24c08_save_addr(void)
{

    if (SAVE_ADDR_SPACE == 0)
    {
        memcpy(postnum_addr, &POST_NUM, 4);
        E2prom_Write(0x40, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x40, (uint8_t *)postnum_addr, 256);

        memcpy(write_addr, &WRITE_ADDR, 4);
        E2prom_Write(0x20, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x20, (uint8_t *)write_addr, 256);

        memcpy(post_addr, &POST_ADDR, 4);
        E2prom_Write(0x50, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x50, (uint8_t *)post_addr, 256);

        memcpy(delete_addr, &DELETE_ADDR, 4);
        E2prom_Write(0x30, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x30, (uint8_t *)delete_addr, 256);
        //E2prom_Write(DATA_AMOUNT_ADDR1,(uint8_t*)&POST_NUM,8);//at24c08_write(DATA_AMOUNT_ADDR1,POST_NUM);  //save post data amount

        //E2prom_Write(DATA_WRITE_ADDR1,(uint8_t*)&WRITE_ADDR,8);//at24c08_write(DATA_WRITE_ADDR1,WRITE_ADDR);  //save data write address

        //E2prom_Write(DATA_POST_ADDR1,(uint8_t*)&POST_ADDR,8);//at24c08_write(DATA_POST_ADDR1,POST_ADDR);  //save data post address

        //E2prom_Write(DATA_DELETE_ADDR1,(uint8_t*)&DELETE_ADDR,8);//at24c08_write(DATA_DELETE_ADDR1,DELETE_ADDR);  //save delete data address
    }
    else
    {
        memcpy(postnum_addr, &POST_NUM, 4);
        E2prom_Write(0x80, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x80, (uint8_t *)postnum_addr, 256);

        memcpy(write_addr, &WRITE_ADDR, 4);
        E2prom_Write(0x60, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x60, (uint8_t *)write_addr, 256);

        memcpy(post_addr, &POST_ADDR, 4);
        E2prom_Write(0x90, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x90, (uint8_t *)post_addr, 256);

        memcpy(delete_addr, &DELETE_ADDR, 4);
        E2prom_Write(0x70, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x70, (uint8_t *)delete_addr, 256);
        //E2prom_Write(DATA_AMOUNT_ADDR2,(uint8_t*)&POST_NUM,8);//at24c08_write(DATA_AMOUNT_ADDR2,POST_NUM);  //save post data amount

        //E2prom_Write(DATA_WRITE_ADDR2,(uint8_t*)&WRITE_ADDR,8);//at24c08_write(DATA_WRITE_ADDR2,WRITE_ADDR);  //save data write address

        //E2prom_Write(DATA_POST_ADDR2,(uint8_t*)&POST_ADDR,8);//at24c08_write(DATA_POST_ADDR2,POST_ADDR);  //save data post address

        //E2prom_Write(DATA_DELETE_ADDR2,(uint8_t*)&DELETE_ADDR,8);//at24c08_write(DATA_DELETE_ADDR2,DELETE_ADDR);  //save delete data address
    }
}
/*******************************************************************************
  Save Post Data Amount/Write Data/Post Data/Delete Data Address with locked
*******************************************************************************/
void osi_at24c08_save_addr(void)
{
    //osi_TaskDisable();  //disable the tasks

    at24c08_save_addr(); //Save Post Data Amount/Write Data/Post Data/Delete Data Address

    //osi_TaskEnable(0);  //enable all tasks
}

uint8_t Get_Usage_Val(void)
{
    uint8_t usg_val;
    unsigned long post_addr_val;
    unsigned long write_addr_val;

    post_addr_val = POST_ADDR;

    write_addr_val = WRITE_ADDR;

    if (write_addr_val >= post_addr_val)
    {
        usg_val = (uint8_t)((100 * (write_addr_val - post_addr_val)) / Memory_Max_Addr);
    }
    else
    {
        usg_val = (uint8_t)((100 * (Memory_Max_Addr - (post_addr_val - write_addr_val))) / Memory_Max_Addr);
    }

    return usg_val;
}

void Save_Data_Reset(void)
{
    POST_NUM = 0; //reset data post amount

    WRITE_ADDR = 0; //reset data write address

    POST_ADDR = 0; //reset data post address

    DELETE_ADDR = 0; //reset data delete address
}

/*******************************************************************************
//SPI Locked Nor Flash Memory Chip Reset
*******************************************************************************/
void osi_Save_Data_Reset(void)
{
    //uint8_t i;
    uint8_t ret_ry = 0;
    uint8_t reg_val = 1;

    save_addr_test();

    Save_Data_Reset();

    osi_at24c08_save_addr(); //save post data amount/write data address/post data address/delete data address

    //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );
    //osi_SyncObjWait(&Spi_Mutex,OSI_WAIT_FOREVER);  //SPI Semaphore Take

    if (flash_set_val != 1)
    {
        flash_set_val = w25q_Init();
    }

    w25q_WriteReg(WRITE_STATUS_REGISTER, 0x00); //write status register,no protected

    w25q_WriteCommand(WRITE_ENABLE); //write enable

    w25q_WriteCommand(CHIP_ERASE); //chip eaase code

    while (reg_val & 0x01)
    {
        if (ret_ry++ > 20) //time out 1min
        {
            break;
        }

        //for(i = 0; i<=10;i++)
        //{
        //    SET_RED_LED_ON();  //Set Red Led ON

        //osi_TaskDisable();  //disable the tasks

        //Display_Factory_Reset(1);  //Display the Factory Reset Status

        //HT9B95A_Display_Usage_Val(10*i);  //Display the Usage value

        //osi_TaskEnable(0);  //enable all tasks

        //vTaskDelay(450/ portTICK_RATE_MS);//MAP_UtilsDelay(6000000);  //delay about 450ms
        //}

        reg_val = w25q_ReadReg(READ_STATUS_REGISTER);

        //sys_run_time = 0;  //clear system timeout
    }

    //xSemaphoreGive( w25q128_spi_xSemaphore );
    //osi_SyncObjSignal(&Spi_Mutex);  //SPI Semaphore Give
}
/*******************************************************************************
//Erase Flash 4k memory
*******************************************************************************/
void N25q_EraseMemory(unsigned long addr)
{

    //xSemaphoreTake( w25q128_spi_xSemaphore, portMAX_DELAY );
    //osi_SyncObjWait(&Spi_Mutex,OSI_WAIT_FOREVER);  //SPI Semaphore Take

    w25q_EraseSubsector(addr); //erase the subsector

    //xSemaphoreGive( w25q128_spi_xSemaphore );
    //osi_SyncObjSignal(&Spi_Mutex);  //SPI Semaphore Give

    portENTER_CRITICAL(0); //enter critical

    DELETE_ADDR += 4096;

    portEXIT_CRITICAL(0); //exit critical

    if (DELETE_ADDR >= Memory_Max_Addr)
    {
        DELETE_ADDR = 0; //subsector 0
    }

    if (SAVE_ADDR_SPACE == 0)
    {
        memcpy(delete_addr, &DELETE_ADDR, 4);
        E2prom_Write(0x40, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x40, (uint8_t *)delete_addr, 256); //E2prom_Write(DATA_DELETE_ADDR1,(uint8_t*)&DELETE_ADDR,8);//osi_at24c08_write(DATA_DELETE_ADDR1,DELETE_ADDR);  //save data delete address
    }
    else
    {
        memcpy(delete_addr, &DELETE_ADDR, 4);
        E2prom_Write(0x80, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x80, (uint8_t *)delete_addr, 256); //E2prom_Write(DATA_DELETE_ADDR2,(uint8_t*)&DELETE_ADDR,8);//osi_at24c08_write(DATA_DELETE_ADDR2,DELETE_ADDR);  //save data delete address
    }
}

/**********************************************************************WRITE_ADDR*********
//Erase memory
*******************************************************************************/
void osi_Erase_Memory(void)
{
    uint16_t i_sub, n_sub;
    unsigned long d_addr, w_addr, p_addr;

    if ((POST_NUM >= Memory_Max_Addr) || (WRITE_ADDR > Memory_Max_Addr) || (POST_ADDR > Memory_Max_Addr) || (DELETE_ADDR > Memory_Max_Addr))
    {
        osi_Save_Data_Reset(); //Nor Flash Memory Chip Reset
    }
    else
    {
        d_addr = DELETE_ADDR; //delete pointer variable value

        p_addr = POST_ADDR; //post pointer variable value

        w_addr = WRITE_ADDR; //write pointer variable value

        if (((w_addr >= p_addr) && (w_addr >= d_addr) && (p_addr >= d_addr)) || ((p_addr >= d_addr) && (p_addr > w_addr) && (d_addr > w_addr))) //p_addr >= d_addr
        {
            n_sub = (p_addr - d_addr) / 4096; //delete 4k/a subsector

            for (i_sub = 0; i_sub < n_sub; i_sub++)
            {
                N25q_EraseMemory(DELETE_ADDR);
            }
        }
        else if ((d_addr > w_addr) && (d_addr > p_addr) && (w_addr >= p_addr)) //d_addr>=p_addr
        {
            n_sub = (Memory_Max_Addr + 1 - d_addr) / 4096; //max 0x00ffffff,4k a subsector

            for (i_sub = 0; i_sub < n_sub; i_sub++)
            {
                N25q_EraseMemory(DELETE_ADDR);
            }

            DELETE_ADDR = 0; //subsector 0

            n_sub = p_addr / 4096; //4k/a subsector

            for (i_sub = 0; i_sub < n_sub; i_sub++)
            {
                N25q_EraseMemory(DELETE_ADDR);
            }
        }
        else //pointer variable wrong,delete all sensor data
        {
            osi_Save_Data_Reset(); //Nor Flash Memory Chip Reset
        }
    }
}

/*******************************************************************************
//nor flash memory chip,data deleted task
*******************************************************************************/
//void Memory_DeleteTask(void *pvParameters)
//{
//uint8_t use_age;

//for(;;)
//{
//osi_SyncObjWait(&Dlete_Binary,OSI_WAIT_FOREVER);  //wait task start message

//osi_Erase_Memory();  //Erase memory

//}
//}

/*******************************************************************************
                                      END         
*******************************************************************************/
