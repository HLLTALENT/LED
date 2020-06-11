#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "PCF8563.h"
#include "driver/i2c.h"

#include "CLI.h"
#include "MSS.h"
#include "w25q128.h"
#include "spi_master_usr.h"
#include "Json_parse.h"
#include "Uart0.h"
#include "DataDeleteTask.h"
#include "DataSaveTask.h"
#include "E2prom.h"
#include "ht9b95a.h"

#define BUF_SIZE 1024

extern char data_1[BUF_SIZE];
extern char data_3[BUF_SIZE];
//extern unsigned long   DELETE_ADDR;
//extern unsigned long   WRITE_ADDR;
//static char FLASH_cmd[]="VS_Data\r\n";
short flash_set_val = -1;

QueueHandle_t xQueue;

int64_t timestamp = 0;

//struct Vitalsigns Vitalsigns1;

void timer_periodic_cb(void *arg);            //定时器函数声明
esp_timer_handle_t timer_periodic_handle = 0; //定义重复定时器句柄

//定义一个重复运行的定时器结构体
esp_timer_create_args_t timer_periodic_arg = {
    .callback =
        &timer_periodic_cb, //设置回调函数
    .arg = NULL,            //不携带参数
    .name = "PeriodicTimer" //定时器名字
};

/*struct Vitalsigns = {
    .time = buffer,
    .BreathingRate = BreathingRate_Out,
    .HeartRate = HeartRate_Out
};*/

void timer_periodic_cb(void *arg) //1ms中断一次
{
    static int64_t timer_count = 0;
    timer_count++;
    if (timer_count >= 1000) //1s
    {
        timer_count = 0;
    }
}

static void Time_Read_Task(void *arg)
{
    while (1)
    {
        timestamp = Read_UnixTime();
        Read_UTCtime();
        vTaskDelay(166 / portTICK_RATE_MS);
    }
}

static void CLI_test_task(void *arg)
{
    while (1)
    {
        CLI_echo();
    }
    vTaskDelete(NULL);
}

static void MSS_test_task(void *arg)
{
    while (1)
    {
        MSS_Read_echo();
        //vTaskDelay(1000 / portTICK_RATE_MS);
        xQueueSend(xQueue, (void *)data_1, 0);
    }
    vTaskDelete(NULL);
}

static void Parse_Data_task(void *arg)
{
    while (1)
    {
        xQueueReceive(xQueue, (void *)data_3, 200 / portTICK_RATE_MS);
        Parse_Data();
        //vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

static void w25q128_task(void *arg)
{
    //struct VitalSigns {
    //char buffer1[100];
    //float BreathingRate;
    //float HeartRate;
    //};
    //struct VitalSigns VS_Data;
    //char buf[100];
    //char write_buf[100]="12345";
    while (1)
    {
        memcpy(VS_Data.buffer1, buffer, 100); //VS_Data.buffer1[100]= buffer[100];
        //printf("time2=%s\n",buffer);
        //printf("time3=%s\n",VS_Data.buffer1);
        VS_Data.BreathingRate = BreathingRate_Out;
        //printf("VS_Data.BreathingRate=%1.0f\n",VS_Data.BreathingRate);
        VS_Data.HeartRate = HeartRate_Out;
        //printf("VS_Data.HeartRate=%1.0f\n",VS_Data.HeartRate);
        //HT9B95A_Init(1);
        //HT9B95A_Display_VSData_Val(VS_Data.HeartRate);

        //w25q_EraseSubsector(0);
        //w25q_WriteData( 0, (char *)&VS_Data,sizeof(VS_Data));
        //w25q_ReadData( 0, (char *)&VS_Data,sizeof(VS_Data));
        //printf("time1:%s,breathrate1:%1.0f,heartrate1:%1.0f",VS_Data.buffer1,VS_Data.BreathingRate,VS_Data.HeartRate);
        //printf("\r\n");
        //w25q_Write_Data(0, (char *)write_buf,6);
        //w25q_Read_Data( 0, (char *)buf,6);
        //printf("buf:%s",buf);
        //vTaskDelay(1/ portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

//void Memory_DeleteTask(void *pvParameters)
//{

//for(;;)
//{
//osi_SyncObjWait(&Dlete_Binary,OSI_WAIT_FOREVER);  //wait task start message

//osi_Erase_Memory();  //Erase memory

//}
//}
static void Datasave_task(void *arg)
{
    while (1)
    {
        DataSaveTask();
    }
    vTaskDelete(NULL);
}

static void Uart0_task(void *arg)
{
    while (1)
    {
        Uart0_read();
    }
    vTaskDelete(NULL);
}

static void LCD_task(void *arg)
{

    HT9B95A_Init(1); //Init LCD driver

    while (1)
    {
        HT9B95A_Display_HeartRate_Val((uint8_t)HeartRate_Out);
        HT9B95A_Display_BreathingRate_Val((uint8_t)BreathingRate_Out); //HT9B95A_Display_Temp_Val(s_val_1,C_F_Temp);  //Display the temprature value
    }
    printf("gjh\r\n");
    vTaskDelete(NULL);
}

void app_main()
{
    Uart0_Init();
    GIPO_INIT();
    SPI_CS_init();
    vTaskDelay(1 / portTICK_RATE_MS);
    PCF8563_Init();
    E2prom_Init();
    Timer_IC_Init();
    CLI_Init();
    vTaskDelay(1 / portTICK_RATE_MS);
    MSS_Init();

    spi_init(&spi_w25q128, 1);
    //DECA_SPI_Config_Rate(4);
    w25q128xSemaphore_init();
    w25q_Init();
    //w25q_EraseChip();
    //HT9B95A_Init(1);
    //Sensors_Init(1);
    /////vTaskDelay(300 / portTICK_RATE_MS);

    timestamp = Read_UnixTime();

    //printf("timestamp=%lld\n",timestamp);
    //Serial.begin(112500);
    xQueue = xQueueCreate(5, sizeof(data_1));
    if (xQueue == NULL)
    {
        printf("Error creating the queue");
    }
    xTaskCreate(&Uart0_task, "Uart0_task", 4096, NULL, 10, NULL);
    xTaskCreate(&CLI_test_task, "CLI_test_task", 2048, NULL, 10, NULL);
    vTaskDelay(1 / portTICK_RATE_MS);
    xTaskCreate(&MSS_test_task, "MSS_test_task", 4096, NULL, 10, NULL);
    xTaskCreate(&Parse_Data_task, "Parse_Data_task", 4096, NULL, 10, NULL);
    xTaskCreate(&Time_Read_Task, "Time_Read_Task", 4096, NULL, 10, NULL);
    xTaskCreate(&w25q128_task, "w25q128_task", 4096, NULL, 10, NULL);
    xTaskCreate(&LCD_task, "LCD_task", 4096, NULL, 10, NULL);
    //xTaskCreate(&Memory_DeleteTask, "Memory_DeleteTask", 4096, NULL, 10, NULL);
    xTaskCreate(&Datasave_task, "Datasave_Task", 4096, NULL, 10, NULL);
    //xTaskCreate(&Uart0_task, "Uart0_task", 4096, NULL, 10, NULL);

    vTaskDelay(5 / portTICK_RATE_MS);

    /*******************************timer 1s init**********************************************/
    esp_err_t err = esp_timer_create(&timer_periodic_arg, &timer_periodic_handle);
    err = esp_timer_start_periodic(timer_periodic_handle, 1000); //创建定时器，单位us 定时1ms
    if (err != ESP_OK)
    {
        printf("timer periodic create err code:%d\n", err);
    }
}
