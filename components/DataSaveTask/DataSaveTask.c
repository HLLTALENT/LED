/*******************************************************************************
  * @file       Data Save Application Task   
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
#include <stdlib.h>
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"
#include "cJSON.h"
#include "w25q128.h"
#include "PCF8563.h"
#include "Json_parse.h"
#include "DataDeleteTask.h"
#include "spi_master_usr.h"
#include "MSS.h"
#include "E2prom.h"
#include "ht9b95a.h"

//extern volatile uint8_t         SAVE_ADDR_SPACE;
#define FN_USAGE_DISPLAY 500
#define POST_DATA_NUMBER 2000

char delete_addr[100];
char write_addr[100];
char post_addr[100];
char postnum_addr[100];
char *VSDatabuffer;
uint8_t vsdatasize;
extern void osi_Erase_Memory(void);

/*******************************************************************************
//save sensor data
*******************************************************************************/
void DataSave(char *VSDatabuffer, uint8_t vsdatasize)
{
    //unsigned long   WRITE_ADDR = 0x01;
    uint8_t zerobuf[256] = "\0";
    //uint8_t use_age;

    osi_w25q_Write_Addr_Check(WRITE_ADDR); //w25q128 check write address with locked

    w25q_WriteData(WRITE_ADDR, VSDatabuffer, sizeof(VSDatabuffer)); //w25q_WriteData(WRITE_ADDR,buffer,size); //write data in nor-flash
    //w25q_ReadData( WRITE_ADDR, VSDatabuffer,sizeof(VSDatabuffer));
    //printf("time1:%s,breathrate1:%1.0f,heartrate1:%1.0f",VS_Data.buffer1,VS_Data.BreathingRate,VS_Data.HeartRate);
    //printf("\r\n");

    POST_NUM += 1;
    WRITE_ADDR += 1 + vsdatasize; //End with a '!'

    if (SAVE_ADDR_SPACE == 0)
    {
        memcpy(write_addr, &WRITE_ADDR, 4);
        E2prom_Write(0x20, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x20, (uint8_t *)write_addr, 256);

        memcpy(postnum_addr, &POST_NUM, 4);
        E2prom_Write(0x30, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x30, (uint8_t *)postnum_addr, 256);
        //E2prom_Write(DATA_WRITE_ADDR1,(uint8_t*)&WRITE_ADDR,8);//osi_at24c08_write(DATA_WRITE_ADDR1,WRITE_ADDR);  //save the write address

        //E2prom_Write(DATA_AMOUNT_ADDR1,(uint8_t*)&POST_NUM,8);//osi_at24c08_write(DATA_AMOUNT_ADDR1,POST_NUM);  //save the post data amount
    }
    else
    {
        memcpy(write_addr, &WRITE_ADDR, 4);
        E2prom_Write(0x60, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x60, (uint8_t *)write_addr, 256);

        memcpy(postnum_addr, &POST_NUM, 4);
        E2prom_Write(0x70, (uint8_t *)zerobuf, 256);
        E2prom_Write(0x70, (uint8_t *)postnum_addr, 256);

        //E2prom_Write(DATA_WRITE_ADDR2,(uint8_t*)&WRITE_ADDR,8);//osi_at24c08_write(DATA_WRITE_ADDR2,WRITE_ADDR);  //save the write address

        //E2prom_Write(DATA_AMOUNT_ADDR2,(uint8_t*)&POST_NUM,8);//osi_at24c08_write(DATA_AMOUNT_ADDR2,POST_NUM);  //save the post data amount
    }

    //if(POST_NUM%FN_USAGE_DISPLAY==0)
    //{
    //use_age = Get_Usage_Val();

    //osi_TaskDisable();  //disable the tasks

    //HT9B95A_Display_Usage_Val(use_age);

    //osi_TaskEnable(0);  //enable all tasks
    //}

    //memcpy(write_addr,&WRITE_ADDR,100);
    //E2prom_Write(0x20, (uint8_t *)zerobuf, 256);
    //E2prom_Write(0x20,(uint8_t*)write_addr,256);
    //printf("WRITE_ADDR=%s\n",write_addr);
    //printf("(uint8_t*)WRITE_ADDR=%p\n",(uint8_t*)write_addr);
}

void DataSaveTask(void)
{
    unsigned long d_addr, w_addr, p_addr;
    //uint16_t det_data_sum;
    //uint16_t det_data_num;

    for (;;)
    {
        if ((POST_NUM >= Memory_Max_Addr) || (WRITE_ADDR > Memory_Max_Addr) || (POST_ADDR > Memory_Max_Addr) || (DELETE_ADDR > Memory_Max_Addr))
        {
            for (;;)
            {
                osi_Erase_Memory(); //Erase memory
            }                       //DataSaveTask(void *pvParameters);//osi_SyncObjSignalFromISR(&Dlete_Binary);  //start delete task
        }
        else
        {
            d_addr = DELETE_ADDR; //delete pointer variable value

            w_addr = WRITE_ADDR; //write pointer variable value

            p_addr = POST_ADDR; //post pointer variable value

            if ((w_addr >= p_addr) && (w_addr >= d_addr) && (p_addr >= d_addr)) //w_addr>d_addr
            {
                if ((w_addr + SAVE_DATA_SIZE) <= Memory_Max_Addr) //Memory Chip max address,end with a '!'
                {
                    DataSave((char *)&VS_Data, sizeof(VS_Data)); //save data
                }
                else
                {
                    WRITE_ADDR = 0; //first Sector

                    /*if(d_addr==0)
          { 
            det_data_sum=POST_DATA_NUMBER;
              
            while(det_data_sum)
            {
              Read_PostDataLen(POST_ADDR,&det_data_end_addr,det_data_sum,&det_data_num,NULL);
              
              PostAddrChage(det_data_num,det_data_end_addr);  //change the point
              
              if(det_data_sum>det_data_num)
              {
                det_data_sum -= det_data_num;
              }
              else
              {
                break;
              }
            }
            osi_Erase_Memory();  //Erase memory
          }*/
                    DataSave((char *)&VS_Data, sizeof(VS_Data)); //save data
                }
            }
            else if (((p_addr >= d_addr) && (p_addr > w_addr) && (d_addr > w_addr)) || ((d_addr > w_addr) && (d_addr > p_addr) && (w_addr >= p_addr))) //w_addr<d_addr
            {
                //if((w_addr+SAVE_DATA_SIZE)>=d_addr)  //WRITE_ADDR pointer variable can not be equal to DELETED pointer variable//
                //{
                //det_data_sum=POST_DATA_NUMBER;

                //while(det_data_sum)
                //{
                //Read_PostDataLen(POST_ADDR,&det_data_end_addr,det_data_sum,&det_data_num,NULL);

                //PostAddrChage(det_data_num,det_data_end_addr);  //change the point

                //if(det_data_sum>det_data_num)
                //{
                //det_data_sum-=det_data_num;
                //}
                //else
                //{
                //break;
                //}
                //}
                //osi_Erase_Memory();  //Erase memory
                //}
                DataSave((char *)&VS_Data, sizeof(VS_Data)); //save data
            }
            else //pointer variable wrong,need delete all sensor data
            {
                for (;;)
                {
                    osi_Erase_Memory(); //Erase memory
                }
            }
        }
    }
}

/*******************************************************************************
                                      END         
*******************************************************************************/
