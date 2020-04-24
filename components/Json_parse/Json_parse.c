#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <cJSON.h>
#include "esp_system.h"
#include "Json_parse.h"
#include "MSS.h"
#include "w25q128.h"
#include "Uart0.h"
#include "PCF8563.h"
#include "DataDeleteTask.h"
#include "DataSaveTask.h"

#include "E2prom.h"

struct VitalSigns1
{
    char buffer2[100];
    float BreathingRate1;
    float HeartRate1;
};

struct VitalSigns1 VS_Data1;

unsigned long write_address;
unsigned long postnum_address;
char VSDatabuffer1[256];

esp_err_t parse_Uart0(char *json_data)
{
    cJSON *json_data_parse = NULL;
    cJSON *json_data_parse_VSData = NULL;
    //cJSON *json_data_parse_SeriesNumber = NULL;
    char read_buffer1[100];
    char read_buffer2[100];
    char read_buffer3[100];
    char read_buffer4[100];
    //unsigned long write_address;
    //unsigned long postnum_address;
    //char VSDatabuffer1[256];
    //if(strstr(json_data,"{")==NULL)

    if (json_data[0] != '{')
    {
        printf("uart0 Json Formatting error1\n");
        return 0;
    }

    json_data_parse = cJSON_Parse(json_data);
    if (json_data_parse == NULL) //如果数据包不为JSON则退出
    {
        printf("uart0 Json Formatting error\n");
        cJSON_Delete(json_data_parse);

        return 0;
    }

    else
    {
        json_data_parse_VSData = cJSON_GetObjectItem(json_data_parse, "command");
        if (!(strcmp(json_data_parse_VSData->valuestring, "VSData")))
        {
            printf("VSData=%s\n", json_data_parse_VSData->valuestring);

            sprintf((char *)&VS_Data, "%s%c", json_data_parse_VSData->valuestring, '\0');

            if (SAVE_ADDR_SPACE == 0)
            {
                E2prom_Read(0x20, (uint8_t *)read_buffer1, 256);
                memcpy(&write_address, read_buffer1, 4);

                E2prom_Read(0x30, (uint8_t *)read_buffer2, 256);
                memcpy(&postnum_address, read_buffer2, 4);
            }
            else
            {
                E2prom_Read(0x60, (uint8_t *)read_buffer3, 256);
                memcpy(&write_address, read_buffer3, 4);

                E2prom_Read(0x70, (uint8_t *)read_buffer4, 256);
                memcpy(&postnum_address, read_buffer4, 4);
            }
            while (1)
            {
                //E2prom_Read(0x20,(uint8_t *)read_buffer,100);
                //memcpy(&write_address,read_buffer,100);

                w25q_ReadData(write_address, VSDatabuffer1, 256); // w25q_ReadData( write_address, (char *)&VS_Data,sizeof(VS_Data));
                memcpy(&VS_Data1, VSDatabuffer1, 108);
                printf("write_address=%ld\r\n", write_address);
                printf("(uint8_t*)write_address=%p\r\n", (uint8_t *)write_address);
                printf("time:%s,breathrate:%1.0f,heartrate:%1.0f", VS_Data1.buffer2, VS_Data1.BreathingRate1, VS_Data1.HeartRate1);
                printf("\r\n");

                postnum_address += 1;
                write_address += 1 + sizeof(VS_Data1);
            }
        }

        cJSON_Delete(json_data_parse);

        return 1;
    }
}
