#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "CLI.h"

#define  GPIO_FRONT      34     
#define  GPIO_BACK       35


#define UART2_TXD  17//(GPIO_NUM_17)
#define UART2_RXD  16//(GPIO_NUM_16)
#define UART2_RTS  (UART_PIN_NO_CHANGE)
#define UART2_CTS  (UART_PIN_NO_CHANGE)


#define BUF_SIZE   1024

//#define ERROR_CODE

//float temp=0.0;
//float humi=0.0;
//float *temp_val=&temp;
//float *humi_val=&humi;
extern float outSumEnergyHeartWfm_thresh;
extern float outSumEnergyBreathWfm_thresh;


/*void on_pushButton_start_clicked()
{
    if (gpio_get_level(GPIO_BACK)==0x00)
    {
      outSumEnergyBreathWfm_thresh = 0.001;
      outSumEnergyHeartWfm_thresh  = 0.001;
      printf("BACK param:%f ",outSumEnergyHeartWfm_thresh);
      printf("BACK param:%f ",outSumEnergyBreathWfm_thresh);

    }
    if (gpio_get_level(GPIO_FRONT)==0x00)
    {
      
      outSumEnergyBreathWfm_thresh = 10;
      outSumEnergyHeartWfm_thresh = 0.1;
      printf("FRONT param:%f ",outSumEnergyHeartWfm_thresh);
      printf("FRONT param:%f ",outSumEnergyBreathWfm_thresh);
      
    }
}
*/

void CLI_echo(void)
{
    if(gpio_get_level(GPIO_FRONT)==0x00)
    {//static char data_u2[BUF_SIZE];
        vTaskDelay(6/ portTICK_RATE_MS);
        static char CLI_cmd_0[]="sensorStop\r\n";
        static char CLI_cmd_1[]="flushCfg\r\n";
        static char CLI_cmd_2[]="dfeDataOutputMode 1\r\n";
        static char CLI_cmd_3[]="channelCfg 15 3 0\r\n";
        static char CLI_cmd_4[]="adcCfg 2 1\r\n";
        static char CLI_cmd_5[]="adcbufCfg -1 0 0 1 0\r\n";
        static char CLI_cmd_6[]="profileCfg 0 77 7 6 57 0 0 70 1 200 4000 0 0 48\r\n";
        static char CLI_cmd_7[]="chirpCfg 0 0 0 0 0 0 0 1\r\n";
        static char CLI_cmd_8[]="frameCfg 0 0 2 0 50 1 0\r\n";
        static char CLI_cmd_9[]="lowPower 0 1\r\n";
        //static char CLI_cmd_10[] ="calibDcRangeSig -1 0 0 0 0\r\n";
        static char CLI_cmd_11[]="guiMonitor 0 0 0 0 1\r\n";
        static char CLI_cmd_12[]="vitalSignsCfg 0.3 0.9 256 512 4 0.1 0.05 100000 300000\r\n";
        static char CLI_cmd_13[]="motionDetection 1 20 2.0 0\r\n";
        static char CLI_cmd_14[]="sensorStart\r\n";
        
        outSumEnergyBreathWfm_thresh = 10;
        outSumEnergyHeartWfm_thresh  = 0.1;
        printf("FRONT param:%f\r\n",outSumEnergyHeartWfm_thresh);
        printf("FRONT param:%f\r\n",outSumEnergyBreathWfm_thresh);

        uart_write_bytes(UART_NUM_2, CLI_cmd_0,12);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_1,10);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_2,21);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_3,19);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_4,12);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_5,22);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_6,49);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_7,26);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_8,25);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_9,14);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        //uart_write_bytes(UART_NUM_2, CLI_cmd_10,28);
        //vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_11,22);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_12,56);
        vTaskDelay(1000/ portTICK_RATE_MS);
                
        uart_write_bytes(UART_NUM_2, CLI_cmd_13,28);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_14,13);
        vTaskDelay(1000/ portTICK_RATE_MS);
    }
    
    if(gpio_get_level(GPIO_BACK)==0x00)
    {
         vTaskDelay(6/ portTICK_RATE_MS);
        static char CLI_cmd_0[]="sensorStop\r\n";
        static char CLI_cmd_1[]="flushCfg\r\n";
        static char CLI_cmd_2[]="dfeDataOutputMode 1\r\n";
        static char CLI_cmd_3[]="channelCfg 15 3 0\r\n";
        static char CLI_cmd_4[]="adcCfg 2 1\r\n";
        static char CLI_cmd_5[]="adcbufCfg -1 0 0 1 0\r\n";
        static char CLI_cmd_6[]="profileCfg 0 77 7 6 57 0 0 70 1 100 2000 0 0 40\r\n";
        static char CLI_cmd_7[]="chirpCfg 0 0 0 0 0 0 0 1\r\n";
        static char CLI_cmd_8[]="frameCfg 0 0 2 0 50 1 0\r\n";
        static char CLI_cmd_9[]="lowPower 0 1\r\n";
        //static char CLI_cmd_10[] ="calibDcRangeSig -1 0 0 0 0\r\n";
        static char CLI_cmd_11[]="guiMonitor 0 0 0 0 1\r\n";
        static char CLI_cmd_12[]="vitalSignsCfg 0.1 0.7 256 320 1 0.1 0.05 300000 300000\r\n";
        static char CLI_cmd_13[]="motionDetection 1 20 0.04 0\r\n";
        static char CLI_cmd_14[]="sensorStart\r\n";

        outSumEnergyBreathWfm_thresh = 0.001;
        outSumEnergyHeartWfm_thresh  = 0.001;
        printf("BACK param:%f\r\n",outSumEnergyHeartWfm_thresh);
        printf("BACK param:%f\r\n",outSumEnergyBreathWfm_thresh);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_0,12);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_1,10);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_2,21);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_3,19);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_4,12);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_5,22);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_6,49);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_7,26);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_8,25);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_9,14);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        //uart_write_bytes(UART_NUM_2, CLI_cmd_10,28);
        //vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_11,22);
        vTaskDelay(1000/ portTICK_RATE_MS);
    
        uart_write_bytes(UART_NUM_2, CLI_cmd_12,56);
        vTaskDelay(1000/ portTICK_RATE_MS);
                
        uart_write_bytes(UART_NUM_2, CLI_cmd_13,29);
        vTaskDelay(1000/ portTICK_RATE_MS);
        
        uart_write_bytes(UART_NUM_2, CLI_cmd_14,13);
        vTaskDelay(1000/ portTICK_RATE_MS);
    }

    
   
}

void GIPO_INIT(void)
{

    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16
    io_conf.pin_bit_mask = (1ULL<<GPIO_FRONT);
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);   

    //io_conf.pin_bit_mask = (1<<SCLK);
    //gpio_config(&io_conf);   

    
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_BACK);
    gpio_config(&io_conf);

} 
 
    
        
    




void CLI_Init(void)
{
    
    /**********************uart init**********************************************/
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };


    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, UART2_TXD, UART2_RXD, UART2_RTS, UART2_CTS);
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);
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