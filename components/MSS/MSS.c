#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "MSS.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "CLI.h"

#define UART1_TXD (GPIO_NUM_4)  //4//(GPIO_NUM_17)
#define UART1_RXD (GPIO_NUM_13) //26//(GPIO_NUM_16)
#define UART1_RTS (UART_PIN_NO_CHANGE)
#define UART1_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE 1024
#define BUF2_SIZE 288

#define LENGTH_HEADER_BYTES 40    // Header + Magic Word
#define LENGTH_MAGIC_WORD_BYTES 8 // Length of Magic Word appended to the UART packet from the EVM
#define LENGTH_TLV_MESSAGE_HEADER_BYTES 8

#define LENGTH_OFFSET_BYTES LENGTH_HEADER_BYTES - LENGTH_MAGIC_WORD_BYTES + LENGTH_TLV_MESSAGE_HEADER_BYTES
#define LENGTH_OFFSET_NIBBLES 2 * LENGTH_OFFSET_BYTES
#define INDEX_BREATHING_RATE_FFT 12
#define INDEX_BREATHING_RATE_PEAK 14
#define INDEX_HEART_RATE_EST_FFT 8
#define INDEX_HEART_RATE_EST_PEAK 11
#define INDEX_CONFIDENCE_METRIC_BREATH 15
#define INDEX_CONFIDENCE_METRIC_BREATH_xCorr 16
#define INDEX_CONFIDENCE_METRIC_HEART 17
#define INDEX_ENERGYWFM_BREATH 20
#define INDEX_ENERGYWFM_HEART 21
#define INDEX_RANGE_PROFILE_START 35
#define INDEX_HEART_RATE_EST_FFT_xCorr 10

#define INDEX_IN_DATA_BREATHING_RATE_FFT (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_BREATHING_RATE_FFT * 8) / 2
#define INDEX_IN_DATA_BREATHING_RATE_PEAK (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_BREATHING_RATE_PEAK * 8) / 2
#define INDEX_IN_DATA_HEART_RATE_EST_FFT (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_HEART_RATE_EST_FFT * 8) / 2
#define INDEX_IN_DATA_HEART_RATE_EST_PEAK (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_HEART_RATE_EST_PEAK * 8) / 2
#define INDEX_IN_DATA_CONFIDENCE_METRIC_BREATH (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_CONFIDENCE_METRIC_BREATH * 8) / 2
#define INDEX_IN_DATA_CONFIDENCE_METRIC_HEART (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_CONFIDENCE_METRIC_HEART * 8) / 2
#define INDEX_IN_DATA_ENERGYWFM_BREATH (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_ENERGYWFM_BREATH * 8) / 2
#define INDEX_IN_DATA_CONFIDENCE_METRIC_BREATH_xCorr (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_CONFIDENCE_METRIC_BREATH_xCorr * 8) / 2
#define INDEX_IN_DATA_HEART_RATE_EST_FFT_xCorr (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_HEART_RATE_EST_FFT_xCorr * 8) / 2
#define INDEX_IN_DATA_ENERGYWFM_HEART (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_ENERGYWFM_HEART * 8) / 2
#define INDEX_IN_DATA_RANGE_PROFILE_START (LENGTH_MAGIC_WORD_BYTES + LENGTH_OFFSET_NIBBLES + INDEX_RANGE_PROFILE_START * 8) / 2

#define THRESH_BREATH_CM 1.0
#define ALPHA_HEARTRATE_CM 0.2
#define THRESH_HEART_CM 0.25
#define THRESH_DIFF_EST 20
#define ALPHA_RCS 0.2
#define RCS_thresh 500
#define THRESH_BACK 30
////#define outSumEnergyHeartWfm_thresh       0.1
////#define outSumEnergyBreathWfm_thresh      10

float outSumEnergyHeartWfm_thresh;
float outSumEnergyBreathWfm_thresh;

float outSumEnergyHeartWfm;
float diffEst_heartRate;
//float heartRateEstDisplay;
char data_1[BUF_SIZE];
char data_3[BUF_SIZE];

char data_2[BUF2_SIZE];
char parseData[4];
char parseData_1[2];
float parseValueOut;
float BreathingRate_Out;
float HeartRate_Out;
int16_t value;
double maxRCS;
int i;

uint16_t parseValueInt16(char *data_2, int valuePos, int valueSize)
{
    memcpy(parseData_1, (data_2 + valuePos), valueSize);
    /*printf("parseData_1 recv:");
       for(int i=0;i<2;i++)
        {
           printf("%02X ",parseData_1[i]);
        }
       printf("\r\n");*/
    *((char *)(&value)) = parseData[0];
    *((char *)(&value) + 1) = parseData[1];

    //value = (int)((unsigned char)parseData_1[0] | (unsigned char)parseData_1[1]<<8);//
    // printf("%d ",value);
    return value;
}

float parseValueFloat(char *data_2, int valuePos, int valueSize)
{
    memcpy(parseData, (data_2 + valuePos), valueSize);
    /*printf("parseData recv:");
       for(int i=0;i<4;i++)
        {
           printf("%02X ",parseData[i]);
        }
       printf("\r\n");*/
    *((char *)(&parseValueOut)) = parseData[0];
    *((char *)(&parseValueOut) + 1) = parseData[1];
    *((char *)(&parseValueOut) + 2) = parseData[2];
    *((char *)(&parseValueOut) + 3) = parseData[3];
    //parseValueOut = *((float *)parseData);
    return parseValueOut;
}

void Parse_Data()
{
    static float outHeartNew_CM;
    static float maxRCS_updated;
    int j;
    unsigned int numRangeBinProcessed = 19;
    double yRangePlot[numRangeBinProcessed];
    double RangeProfile[2 * numRangeBinProcessed];
    unsigned int indexRangeBin;
    vTaskDelay(1000 / portTICK_RATE_MS);
    for (i = 0; i < 1024; i++)
    {
        if ((data_3[i] == 0X02) && (data_3[i + 1] == 0X01) && (data_3[i + 2] == 0X04) && (data_3[i + 3] == 0X03) && (data_3[i + 4] == 0X06) && (data_3[i + 5] == 0X05) && (data_3[i + 6] == 0X08) && (data_3[i + 7] == 0X07))
        {

            break;
            //printf("%d ",i);
        }
    }

    memcpy(data_2, (data_3 + i), 288);
    /*printf("data_2: ");
    for (int z = 0; z < 288; z++)

    {
        printf("%02X ", data_2[i]);
    }
    printf("\r\n");*/

    float BreathingRate_FFT = parseValueFloat(data_2, INDEX_IN_DATA_BREATHING_RATE_FFT, 4); // Breathing Rate
    float BreathingRatePK_Out = parseValueFloat(data_2, INDEX_IN_DATA_BREATHING_RATE_PEAK, 4);
    float heartRate_FFT = parseValueFloat(data_2, INDEX_IN_DATA_HEART_RATE_EST_FFT, 4); // Heart Rate
    float heartRate_Pk = parseValueFloat(data_2, INDEX_IN_DATA_HEART_RATE_EST_PEAK, 4);
    float breathRate_CM = parseValueFloat(data_2, INDEX_IN_DATA_CONFIDENCE_METRIC_BREATH, 4); // Confidence Metric
    float heartRate_CM = parseValueFloat(data_2, INDEX_IN_DATA_CONFIDENCE_METRIC_HEART, 4);
    float heartRate_xCorr = parseValueFloat(data_2, INDEX_IN_DATA_HEART_RATE_EST_FFT_xCorr, 4); // Heart Rate - AutoCorrelation
    //printf("heartRate_xCorr:%1.0f\r\n",heartRate_xCorr );
    float outSumEnergyBreathWfm = parseValueFloat(data_2, INDEX_IN_DATA_ENERGYWFM_BREATH, 4); // Waveforms Energy
    float BreathingRate_xCorr_CM = parseValueFloat(data_2, INDEX_IN_DATA_CONFIDENCE_METRIC_BREATH_xCorr, 4);
    outSumEnergyHeartWfm = parseValueFloat(data_2, INDEX_IN_DATA_ENERGYWFM_HEART, 4); // Waveforms Energy
                                                                                      //printf("outSumEnergyHeartWfm:%1.0f\r\n",outSumEnergyHeartWfm);

    for (unsigned int index = 0; index < (2 * numRangeBinProcessed); index++)
    {
        int indexRange = 168;
        uint16_t tempRange_int = parseValueInt16(data_2, indexRange, 2);
        //printf("tempRange_int:%d ",tempRange_int);
        //printf("\r\n");
        RangeProfile[index] = tempRange_int;
        //printf("RangeProfile[index]:%f ",RangeProfile[index]);
        indexRange = indexRange + 4;
        break;
    }

    for (indexRangeBin = 0; indexRangeBin < numRangeBinProcessed; indexRangeBin++)
    {
        yRangePlot[indexRangeBin] = sqrt(RangeProfile[2 * indexRangeBin] * RangeProfile[2 * indexRangeBin] + RangeProfile[2 * indexRangeBin + 1] * RangeProfile[2 * indexRangeBin + 1]);
        break;
    }
    maxRCS = yRangePlot[0];
    for (j = 1; j < numRangeBinProcessed; j++)
    {
        if (yRangePlot[j] > maxRCS)
            maxRCS = yRangePlot[j];
        //printf("maxRCS:%f ",maxRCS);
        //printf("\r\n");
        maxRCS_updated = ALPHA_RCS * (maxRCS) + (1 - ALPHA_RCS) * maxRCS_updated;
        break;
    }
    //maxRCS_updated = ALPHA_RCS*(maxRCS) + (1-ALPHA_RCS)*maxRCS_updated;
    // Heart-Rate Display Decision
    if (outSumEnergyBreathWfm_thresh != outSumEnergyHeartWfm_thresh) //(outSumEnergyBreathWfm_thresh == 10) && (outSumEnergyHeartWfm_thresh  == 0.1))
    {
        float outHeartPrev_CM = outHeartNew_CM;
        outHeartNew_CM = ALPHA_HEARTRATE_CM * (heartRate_CM) + (1 - ALPHA_HEARTRATE_CM) * outHeartPrev_CM;

        diffEst_heartRate = abs(heartRate_FFT - heartRate_Pk);
        if (outSumEnergyHeartWfm < outSumEnergyHeartWfm_thresh || maxRCS_updated < RCS_thresh)
        {
            HeartRate_Out = 0;
            printf("heartRate:%1.0f\r\n", HeartRate_Out);
            //vTaskDelay(1000 / portTICK_RATE_MS);
            //printf("\r\n");
        }
        else
        {
            if ((outHeartNew_CM > THRESH_HEART_CM) || (diffEst_heartRate < THRESH_DIFF_EST))
            {
                HeartRate_Out = heartRate_FFT;
                printf("heartRate_FFT:%1.1f\r\n", HeartRate_Out);
                // vTaskDelay(1000 / portTICK_RATE_MS);
            }
            else
            {
                HeartRate_Out = heartRate_Pk;
                printf("heartRate_Pk:%1.1f\r\n", HeartRate_Out);
                //vTaskDelay(1000 / portTICK_RATE_MS);
            }
        }
    }
    else if (outSumEnergyBreathWfm_thresh == outSumEnergyHeartWfm_thresh) //((outSumEnergyBreathWfm_thresh == 0.001) && (outSumEnergyHeartWfm_thresh  == 0.001))
    {
        if ((outSumEnergyHeartWfm < outSumEnergyHeartWfm_thresh) || (maxRCS_updated < RCS_thresh))
        {
            HeartRate_Out = 0;
            printf("heartRate:%1.0f\r\n", HeartRate_Out);
            //vTaskDelay(1000 / portTICK_RATE_MS);
            //printf("\r\n");
        }
        else
        {
            if (abs(heartRate_xCorr - heartRate_FFT) < THRESH_BACK)
            {
                HeartRate_Out = heartRate_FFT;
                printf("heartRate_FFT:%1.0f\r\n", HeartRate_Out);
                // vTaskDelay(1000 / portTICK_RATE_MS);
            }
            else
            {
                HeartRate_Out = heartRate_xCorr;
                printf("heartRate_xCorr:%1.0f\r\n", HeartRate_Out);
                //vTaskDelay(1000 / portTICK_RATE_MS);
            }
        }
    }

    // Breathing-Rate decision
    if ((outSumEnergyBreathWfm < outSumEnergyBreathWfm_thresh) || (maxRCS_updated < RCS_thresh) || (BreathingRate_xCorr_CM <= 0.002))
    {

        BreathingRate_Out = 0;
        printf("BreathingRate:%1.3f\r\n", BreathingRate_Out);
        vTaskDelay(1000 / portTICK_RATE_MS);
        //printf("\r\n");
    }
    else
    {
        if (breathRate_CM > THRESH_BREATH_CM)
        {
            BreathingRate_Out = BreathingRate_FFT;
            printf("BreathingRate:%1.3f\r\n", BreathingRate_Out);
            vTaskDelay(1000 / portTICK_RATE_MS);
            //printf("\r\n");
        }
        else
        {
            BreathingRate_Out = BreathingRatePK_Out;
            printf("BreathingRate:%1.3f\r\n", BreathingRate_Out);
            vTaskDelay(1000 / portTICK_RATE_MS);
            //printf("\r\n");
        }
    }
}

void MSS_Read_echo(void)
{

    //unsigned int index;
    uart_read_bytes(UART_NUM_1, (uint8_t *)data_1, BUF_SIZE, 100 / portTICK_RATE_MS); //int len1 = uart_read_bytes(UART_NUM_1, (uint8_t*)data_1, BUF_SIZE,100/ portTICK_RATE_MS);
    //vTaskDelay(1000 / portTICK_RATE_MS);
    /*if (len1 != 0)
    {
        printf("UART1 recv:");
        for (int i = 0; i < len1; i++)
        {
            printf("%02X ", data_1[i]);
        }
        printf("\r\n");
    }*/
}

void MSS_Init(void)
{

    /**********************uart init**********************************************/
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART1_TXD, UART1_RXD, UART1_RTS, UART1_CTS);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
    //uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);

    /******************************gpio init*******************************************/
    //gpio_config_t io_conf;
    //disable interrupt
    //io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    //io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16
    //io_conf.pin_bit_mask = (1<<RS485RD);

    //io_conf.pin_bit_mask = (1<<P_KEY);
    //disable pull-down mode
    //io_conf.pull_down_en = 0;
    //disable pull-up mode
    //io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    //gpio_config(&io_conf);
}