/*******************************************************************************
  * @file       PCF8563 Driver  
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
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "PCF8563.h"
#include "driver/i2c.h"
#include "string.h"
#include "stdlib.h"
//#include "E2prom.h"

#define I2C_MASTER_SCL_IO 14        /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 27        /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM 0            //I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000   /*!< I2C master clock frequency */

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

#define ADDR_PAGE0 0xA8
#define ADDR_PAGE1 0xAA
#define ADDR_PAGE2 0xAC
#define ADDR_PAGE3 0xAE

//static const char *TAG = "PCF8563";

int time_delay = 0;
char buffer[100];
int buf_size = 100;

void timer_once_cb(void *arg);            //函数声明
esp_timer_handle_t timer_once_handle = 0; //定义单次定时器句柄

//定义一个单次运行的定时器结构体
esp_timer_create_args_t timer_once_arg = {
    .callback = &timer_once_cb, //设置回调函数
    .arg = NULL,                //不携带参数
    .name = "OnceTimer"         //定时器名字
};

void timer_once_cb(void *arg)
{
    time_delay = 1;
    //int64_t tick = esp_timer_get_time();
    //printf("方法回调名字: %s , 距离定时器开启时间间隔 = %lld \r\n", __func__, tick);
}

void delay_us(uint32_t nus)
{
    time_delay = 0;
    esp_timer_start_once(timer_once_handle, nus); //启动单次定时器
    while (time_delay == 0)
        ;
    return;
}

static esp_err_t PCF_Page_Write(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, int length)
{
    int ret;
    int i;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    for (i = 0; i < length; i++)
    {
        i2c_master_write_byte(cmd, *dat, ACK_CHECK_EN);
        dat++;
    }
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t PCF_Page_Read(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, int length)
{
    int ret;
    int i;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2 * sla_addr + 1, ACK_CHECK_EN);

    for (i = 0; i < length; i++)
    {
        if (i != length - 1)
        {
            i2c_master_read_byte(cmd, dat, ACK_VAL);
            dat++;
        }
        else
        {
            i2c_master_read_byte(cmd, dat, NACK_VAL);
        }
    }
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void PCF8563_Init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

void MulTry_I2C_RD_mulReg(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, uint8_t length)
{
    uint8_t n_try;

    for (n_try = 0; n_try < RETRY_TIME_OUT; n_try++)
    {
        if (PCF_Page_Read(sla_addr, reg_addr, dat, length) == 1) //if(I2C_RD_mulReg(sla_addr,reg_addr,buf,len)==1)
        {
            break;
        }
        vTaskDelay(3 / portTICK_RATE_MS);
        //MAP_UtilsDelay(40000);  //delay about 3ms
    }
}

void MulTry_I2C_WR_mulReg(uint8_t sla_addr, uint8_t reg_addr, uint8_t *dat, uint8_t length)
{
    uint8_t n_try;

    for (n_try = 0; n_try < RETRY_TIME_OUT; n_try++)
    {
        if (PCF_Page_Write(sla_addr, reg_addr, dat, length) == 1) //if(I2C_WR_mulReg(sla_addr,reg_addr,buf,len)==1)
        {
            break;
        }
        vTaskDelay(6 / portTICK_RATE_MS);
        //MAP_UtilsDelay(80000);  //delay about 6ms
    }
}

/*******************************************************************************
//PCF_Format_BCD to PCF_Format_BIN
*******************************************************************************/
static uint8_t Bin_To_Bcd(uint8_t bin_val)
{
    return 16 * (bin_val / 10) + bin_val % 10;
}

/*******************************************************************************
//PCF_Format_BIN to PCF_Format_BCD
*******************************************************************************/
static uint8_t Bcd_To_Bin(uint8_t bcd_val)
{
    return 10 * (bcd_val / 16) + bcd_val % 16;
}

///*******************************************************************************
////PCF8563 update time
//*******************************************************************************/
//void Update_UTCtime(char *time)
//{
//  uint16_t year;
//  char *InpString;
//  uint8_t write_time[7];
//  uint8_t mon,day,hour,min,sec;
//
//  InpString = strtok(time, "-");
//  year = (uint16_t)strtoul(InpString, 0, 10)-2000;  //year
//
//  InpString = strtok(NULL, "-");
//  mon=(uint8_t)strtoul(InpString, 0, 10);  //mon
//
//  InpString = strtok(NULL, "T");
//  day=(uint8_t)strtoul(InpString, 0, 10);  //day
//
//  InpString = strtok(NULL, ":");
//  hour=(uint8_t)strtoul(InpString, 0, 10);  //hour
//
//  InpString = strtok(NULL, ":");
//  min=(uint8_t)strtoul(InpString, 0, 10);  //min
//
//  InpString = strtok(NULL, "Z");
//  sec=(uint8_t)strtoul(InpString, 0, 10);  //sec
//
//
//  write_time[0]=Bin_To_Bcd(sec);  //second
//
//  write_time[1]=Bin_To_Bcd(min);  //minute
//
//  write_time[2]=Bin_To_Bcd(hour);  //hour
//
//  write_time[3]=Bin_To_Bcd(day);  //date
//
//  write_time[5]=Bin_To_Bcd(mon);  //month
//
//  write_time[6]=Bin_To_Bcd(year);  //year
//
//  MulTry_I2C_WR_mulReg(PCF8563_ADDR,VL_seconds,write_time,sizeof(write_time));  //Write Timer Register
//}

/*******************************************************************************
//PCF8563 read UTC time
*******************************************************************************/
//void Read_UTCtime(char *buffer,uint8_t buf_size)
void Read_UTCtime(void)
{
    struct tm ts;
    uint8_t read_time[7];

    //char data_read[100];
    //int read_size=100;

    MulTry_I2C_RD_mulReg(PCF8563_ADDR, VL_seconds, read_time, sizeof(read_time)); //Read Timer Register

    read_time[0] &= 0x7f;
    ts.tm_sec = Bcd_To_Bin(read_time[0]); //second value

    read_time[1] &= 0x7f;
    ts.tm_min = Bcd_To_Bin(read_time[1]); //minite value

    read_time[2] &= 0x3f;
    ts.tm_hour = Bcd_To_Bin(read_time[2]); //hour value

    read_time[3] &= 0x3f;
    ts.tm_mday = Bcd_To_Bin(read_time[3]); //day value

    read_time[5] &= 0x1f;
    ts.tm_mon = Bcd_To_Bin(read_time[5]) - 1; //month value

    ts.tm_year = Bcd_To_Bin(read_time[6]) + 100; //year value

    ts.tm_isdst = 0; //do not use Daylight Saving Time

    strftime(buffer, buf_size, "%Y-%m-%dT%H:%M:%SZ", &ts); //UTC time
    //printf("time2=%s\r\n", buffer);

    //vTaskDelay(20 / portTICK_RATE_MS);

    //E2prom_Write(0x00,(uint8_t*)buffer,buf_size);
    //E2prom_Read(0x00,(uint8_t*)data_read,read_size);
    //printf("data_read=%s\n",data_read);
}

/*******************************************************************************
//PCF8563 read Unix time
*******************************************************************************/
unsigned long Read_UnixTime(void)
{
    struct tm ts;
    uint8_t read_time[7];

    MulTry_I2C_RD_mulReg(PCF8563_ADDR, VL_seconds, read_time, sizeof(read_time)); //Read Timer Register

    read_time[0] &= 0x7f;
    ts.tm_sec = Bcd_To_Bin(read_time[0]); //second

    read_time[1] &= 0x7f;
    ts.tm_min = Bcd_To_Bin(read_time[1]); //minite

    read_time[2] &= 0x3f;
    ts.tm_hour = Bcd_To_Bin(read_time[2]); //hour

    read_time[3] &= 0x3f;
    ts.tm_mday = Bcd_To_Bin(read_time[3]); //date

    read_time[5] &= 0x1f;
    ts.tm_mon = Bcd_To_Bin(read_time[5]) - 1; //month

    ts.tm_year = Bcd_To_Bin(read_time[6]) + 100; //2000-1900 year

    ts.tm_isdst = 0; //do not use Daylight Saving Time

    return mktime(&ts); //unix time
}

/*******************************************************************************
//PCF8563 init
*******************************************************************************/
void Timer_IC_Init(void)
{
    uint8_t status_val[2] = {0x00, 0x00};

    uint8_t alarm_setval[4] = {0x80, 0x80, 0x80, 0x80}; //MIN/HOUR/DAY/WEEDDAY ALARM DISABLED

    uint8_t timer_set[3] = {0x00, 0x00, 0x00}; //CLKOUT INHIBITED/TIMER DISABLED/TIMER VALUE

    MulTry_I2C_WR_mulReg(PCF8563_ADDR, Control_status_1, status_val, sizeof(status_val)); //Write Timer Register
    printf("\n");

    MulTry_I2C_WR_mulReg(PCF8563_ADDR, Minute_alarm, alarm_setval, sizeof(alarm_setval)); //Write Timer Register

    MulTry_I2C_WR_mulReg(PCF8563_ADDR, CLKOUT_control, timer_set, sizeof(timer_set)); //Write Timer Register
}

/*******************************************************************************
//PCF8563 Reset Time
*******************************************************************************/
void Timer_IC_Reset_Time(void)
{
    uint8_t time_val[7] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x18};

    MulTry_I2C_WR_mulReg(PCF8563_ADDR, VL_seconds, time_val, sizeof(time_val)); //Write Timer Register
}

/*******************************************************************************
//Return Unix time
*******************************************************************************/
unsigned long Return_UnixTime(uint8_t *t_buf)
{
    struct tm ts;

    ts.tm_sec = t_buf[0]; //second

    ts.tm_min = t_buf[1]; //minite

    ts.tm_hour = t_buf[2]; //hour

    ts.tm_mday = t_buf[3]; //date

    ts.tm_mon = t_buf[5] - 1; //month

    ts.tm_year = t_buf[6] + 100; //2000-1900 year

    ts.tm_isdst = 0; //do not use Daylight Saving Time

    return mktime(&ts); //unix time
}

/*******************************************************************************
//PCF8563 update time
*******************************************************************************/
/*void Check_Update_UTCtime(char *time)
{
  char *InpString;
  uint8_t i;
  uint8_t reg_buf[7] = {0};
  uint8_t time_buf[7] = {0};
  unsigned long net_time = 0;
  unsigned long sys_time = 0;

  InpString = strtok(time, "-");
  time_buf[6] = (uint8_t)((uint16_t)strtoul(InpString, 0, 10)-2000);  //year

  InpString = strtok(NULL, "-");
  time_buf[5]=(uint8_t)strtoul(InpString, 0, 10);  //month
  
  InpString = strtok(NULL, "T");
  time_buf[3]=(uint8_t)strtoul(InpString, 0, 10);  //day
  
  InpString = strtok(NULL, ":");
  time_buf[2]=(uint8_t)strtoul(InpString, 0, 10);  //hour

  InpString = strtok(NULL, ":");
  time_buf[1]=(uint8_t)strtoul(InpString, 0, 10);  //min

  InpString = strtok(NULL, "Z");
  time_buf[0]=(uint8_t)strtoul(InpString, 0, 10);  //sec
  
  net_time = Return_UnixTime(time_buf);
    
  sys_time = Read_UnixTime();
  
  if((net_time >= (sys_time+UPDATE_TIME_SIZE))||(sys_time >= (net_time+UPDATE_TIME_SIZE)))
  {
    reg_buf[0]=Bin_To_Bcd(time_buf[0])&0x7f;  //second
      
    reg_buf[1]=Bin_To_Bcd(time_buf[1])&0x7f;  //minute
    
    reg_buf[2]=Bin_To_Bcd(time_buf[2])&0x3f;  //hour
    
    reg_buf[3]=Bin_To_Bcd(time_buf[3])&0x3f;  //date
    
    reg_buf[5]=Bin_To_Bcd(time_buf[5])&0x1f;  //month
    
    reg_buf[6]=Bin_To_Bcd(time_buf[6]);  //year
      
    for(i=0;i<RETRY_TIME_OUT;i++)
    {
      MulTry_I2C_WR_mulReg(PCF8563_ADDR,VL_seconds,reg_buf,sizeof(reg_buf));  //Write Timer Register
      
      sys_time = Read_UnixTime();
      
      if((net_time <= (sys_time+UPDATE_TIME_SIZE))&&(sys_time <= (net_time+UPDATE_TIME_SIZE)))
      {
        break;
      }
      else
      {
        vTaskDelay(15 / portTICK_RATE_MS);
        //MAP_UtilsDelay(200000);  //delay about 15ms
      }
    }
  }
}*/
/*******************************************************************************
                                      END         
*******************************************************************************/
