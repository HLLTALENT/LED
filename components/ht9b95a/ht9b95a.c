/*******************************************************************************
  * @file       HT9B95A LCD DRIVER APPLICATION       
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
#include "ht9b95a.h"
#include <math.h>
#include "E2prom.h"


#define I2C_MASTER_SCL_IO               14               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO               27               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM                  0//I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE       0                /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE       0                /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ              100000           /*!< I2C master clock frequency */


#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */


/*******************************************************************************
  write a byte to slave register
*******************************************************************************/
static int IIC_WR_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t val)
{
    int ret;
  
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2*sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
  //IIC_Start();  //IIC start 	
    i2c_master_write_byte(cmd, val, ACK_CHECK_EN);
  //IIC_Send_Byte(2*sla_addr);  //send write command
    //for(i=0;i<length;i++)
    //{
        //i2c_master_write_byte(cmd, *dat, ACK_CHECK_EN);
        //dat++;	
    //}
    i2c_master_stop(cmd);
    
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{   
    //return 0;
  //}
  
  //IIC_Send_Byte(reg_addr);  //send register address
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //return 0;
  //}
  
  //IIC_Send_Byte(val);  //send data value
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{ 
    //return 0;
  //}
   
  //IIC_Stop();   //IIC stop	
  
  //return 1;
}

void MulTry_IIC_WR_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t val)
{
  uint8_t n_try;
  
  for(n_try=0;n_try<RETRY_TIME_OUT;n_try++)
  {
    if(IIC_WR_Reg(sla_addr,reg_addr,val)==ESP_OK)
    {
      break;
    }
    vTaskDelay(6 / portTICK_RATE_MS);
    //MAP_UtilsDelay(80000);  //delay about 6ms
  }
}
/*******************************************************************************
  Read a byte from slave register
*******************************************************************************/
static int IIC_RD_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t *val)		
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2*sla_addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);

    //i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2*sla_addr+1, ACK_CHECK_EN);
   
    i2c_master_read_byte(cmd, val, NACK_VAL); 

    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
  //IIC_Start();  //IIC start  	
  
  //IIC_Send_Byte(2*sla_addr);  //send write command
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //return 0;
  //}
  
  //IIC_Send_Byte(reg_addr);  //send register address
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //return 0;
  //}
  
  //IIC_Start();  //IIC start
  
  //IIC_Send_Byte(2*sla_addr+1);  //send read command
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //return 0;
  //}
  
  //*val=IIC_Read_Byte(0);  //read a byte
  
  //IIC_Stop();   //IIC stop
  
  //return 0;
}
/*******************************************************************************
//Read a byte from slave register whit multiple try
*******************************************************************************/
void MulTry_IIC_RD_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t *val)
{
  uint8_t n_try;
  
  for(n_try=0;n_try<RETRY_TIME_OUT;n_try++)
  {
    if(IIC_RD_Reg(sla_addr,reg_addr,val)==ESP_OK)
    {
      break;
    }
    vTaskDelay(3 / portTICK_RATE_MS);
    //MAP_UtilsDelay(40000);  //delay about 3ms
  }
}
/*******************************************************************************
//write a byte to slave register
*******************************************************************************/
static int HT9B95A_Wr_Cmd(uint8_t sla_addr,uint8_t cmd1)
{
    int ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  //IIC_Start();  //IIC start 	
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 2*sla_addr, ACK_CHECK_EN);
  //IIC_Send_Byte(2*slaaddr);  //send write command
    i2c_master_write_byte(cmd, cmd1, ACK_CHECK_EN);
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //IIC_Stop();  //IIC stop
    i2c_master_stop(cmd);
    //return FAILURE;
  //}
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
  //IIC_Send_Byte(cmd);  //send data value
  
  //if(IIC_Wait_Ack()<0)  //wait device ack
  //{
    //IIC_Stop();  //IIC stop
    
    //return FAILURE;
  //}
   
  //IIC_Stop();  //IIC stop	
  
  //return SUCCESS;
}

static short Multi_HT9B95A_Wr_Cmd(uint8_t sla_addr,uint8_t wr_cmd)
{
  uint8_t i;
  
  for(i=0;i<RETRY_TIME_OUT;i++)
  {
    if(HT9B95A_Wr_Cmd(sla_addr,wr_cmd)==1)
    {
      return 1;
    }
    vTaskDelay(15 / portTICK_RATE_MS);//MAP_UtilsDelay(200000);  //Delay About 15ms
  }
  
  return 0;
}

/*******************************************************************************
//LCD IIC read data
*******************************************************************************/
static void LCD_MulTry_IIC_RD_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t *val)
{
  uint8_t i,reg_val1,reg_val2;
  
  for(i=0;i<RETRY_TIME_OUT;i++)
  {
    //delay_us(2);//MAP_UtilsDelay(20);  //Delay About 1.5us
    
    MulTry_IIC_RD_Reg(sla_addr,reg_addr,&reg_val1);
    
    //delay_us(2);//MAP_UtilsDelay(20);  //Delay About 1.5us
    
    MulTry_IIC_RD_Reg(sla_addr,reg_addr,&reg_val2);
    
    if(reg_val1 == reg_val2)
    {
      *val = reg_val1;
      
      break;
    }
  }
}

/*******************************************************************************
//LCD IIC write data
*******************************************************************************/
static void LCD_MulTry_IIC_WR_Reg(uint8_t sla_addr,uint8_t reg_addr,uint8_t val)
{
  uint8_t i,reg_val;
  
  for(i=0;i<RETRY_TIME_OUT;i++)
  {
    MulTry_IIC_WR_Reg(sla_addr,reg_addr,val);
    
    LCD_MulTry_IIC_RD_Reg(sla_addr,reg_addr,&reg_val);
    
    if(reg_val == val)
    {
      break;
    }
  }
}

/*******************************************************************************
//LCD Clear
*******************************************************************************/
static void HT9B95A_Clear(void)
{
  uint8_t ram_addr;
  
  for(ram_addr=0;ram_addr<=HT9B95A_MAX_RAM_ADDR;ram_addr++)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr,0x00);
    
    vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
  }
}

/*******************************************************************************
//HT9B95A Init
*******************************************************************************/
void HT9B95A_Init(bool lcd_reset)
{
  uint8_t i;
  short set_val;
  
  //delay_us(80);//iic_delay_nus = 80;

  for(i=0;i<RETRY_TIME_OUT;i++)
  {
    vTaskDelay(15 / portTICK_RATE_MS);//MAP_UtilsDelay(200000);                //Delay About 15ms
    
    if(lcd_reset)
    {
      set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xf6);    //Software Reset Setting,LCD Output Waveform Setting
      if(set_val<0)
      {
        continue;
      }
      
      vTaskDelay(150 / portTICK_RATE_MS);//MAP_UtilsDelay(2000000);                //Delay About 150ms
    }

    set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xfd);    //1/4Duty and 1/3 bias Setting
    if(set_val<0)
    {
      continue;
    }
    
    vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
    
    set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xc0);    //LCD driving voltage adjustment
    if(set_val<0)
    {
      continue;
    }
    
    vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
    
    set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xe2);    //Lcd Current Mode Setting,LCD Frame Frequency Setting
    if(set_val<0)
    {
      continue;
    }
    
    vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
    
    set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xf8);    //nomal display   
    if(set_val<0)
    {
      continue;
    }
    
    vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
    
    set_val = Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xf5);    //Software Reset Setting,LCD Output Waveform Setting
    if(set_val<0)
    {
      continue;
    }
    else
    {
      break;
    }
  } 
  vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
    
  HT9B95A_Clear();

  vTaskDelay(3 / portTICK_RATE_MS);//MAP_UtilsDelay(40000);  //Delay About 3ms
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,0x08);  //Based display L1

  //delay_us(40);//iic_delay_nus = 40;
}

/*******************************************************************************
//HT9B95A OFF
*******************************************************************************/
/*void HT9B95A_OFF(void)
{
  Multi_HT9B95A_Wr_Cmd(HT9B95A_ADDR,0xf6);  //Software Reset Setting,LCD Output Waveform Setting
  
  vTaskDelay(1.5 / portTICK_RATE_MS);//MAP_UtilsDelay(20000);  //Delay About 1.5ms
}
*/
/*******************************************************************************
//HT9B95A DISPLAY ALL
*******************************************************************************/
/*void HT9B95A_DisplayAll(void)
{
  uint8_t i;
  
  for(i=0;i<=HT9B95A_MAX_RAM_ADDR;i++)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,i,0xff);
  }
}
*/
/*******************************************************************************
//Display the number 7/8/9(endflag F G E A B C D)
*******************************************************************************/
static void Display_Num_7_8_9(uint8_t ram_addr,uint8_t data_val,bool end_flag)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,ram_addr-1,&reg_val1); 
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,ram_addr,&reg_val2); 
  
  reg_val1 = reg_val1&0xf8;
  
  reg_val2 = reg_val2&0x0f;
  
  if(end_flag)
  {
    reg_val1 = reg_val1|0x08;
  }
  else
  {
    reg_val1 = reg_val1&0xf7;
  }
  
  switch(data_val)
  {
    case 0:  //A B C D E F
    {
      reg_val1 = reg_val1|0x05;  //E F
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 1:  //B C
    {
      reg_val2 = reg_val2|0x60;  //B C
    }
    break;
    
    case 2:  //A B D E G
    {
      reg_val1 = reg_val1|0x03;  //E G
  
      reg_val2 = reg_val2|0xd0;  //A B D
    }
    break;
    
    case 3:  //A B C D G
    {
      reg_val1 = reg_val1|0x02;  //G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 4:  //B C F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0x60;  //B C
    }
    break;
    
    case 5:  //A C D F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0xb0;  //A C D
    }
    break;
    
    case 6:  //A C D E F G
    {
      reg_val1 = reg_val1|0x07;  //E F G
  
      reg_val2 = reg_val2|0xb0;  //A C D
    }
    break;
    
    case 7:  //A B C
    {
      reg_val2 = reg_val2|0xe0;  //A B C
    }
    break;
    
    case 8:  //A B C D E F G
    {
      reg_val1 = reg_val1|0x07;  //E F G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 9:  //A B C D F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    default:
    break;
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr-1,reg_val1); 
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr,reg_val2); 
}

/*******************************************************************************
//Display the number 10(endflag F G E A B C D)
*******************************************************************************/
static void Display_Num_10(uint8_t ram_addr,uint8_t data_val,bool end_flag)
{
  uint8_t reg_val =0;
  
  if(end_flag)
  {
    reg_val = reg_val|0x80;
  }
  
  switch(data_val)
  {
    case 0:  //A B C D E F
    {
      reg_val = reg_val|0x5f;
    }
    break;
    
    case 1:  //B C
    {
      reg_val = reg_val|0x06;
    }
    break;
    
    case 2:  //A B D E G
    {
      reg_val = reg_val|0x3d;
    }
    break;
    
    case 3:  //A B C D G
    {
      reg_val = reg_val|0x2f;
    }
    break;
    
    case 4:  //B C F G
    {
      reg_val = reg_val|0x66;
    }
    break;
    
    case 5:  //A C D F G
    {
      reg_val = reg_val|0x6b;
    }
    break;
    
    case 6:  //A C D E F G
    {
      reg_val = reg_val|0x7b;
    }
    break;
    
    case 7:  //A B C
    {
      reg_val = reg_val|0x0e;
    }
    break;
    
    case 8:  //A B C D E F G
    {
      reg_val = reg_val|0x7f;
    }
    break;
    
    case 9:  //A B C D F G
    {
      reg_val = reg_val|0x6f;
    }
    break;
    
    default:
    break;
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr,reg_val); 
}

/*******************************************************************************
//Display the number 1/2/3/4/5(E G F endflag D C B A)
*******************************************************************************/
static void Display_Num_1_2_3_4_5(uint8_t ram_addr,uint8_t data_val,bool end_flag)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,ram_addr-1,&reg_val1); 
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,ram_addr,&reg_val2); 
  
  reg_val1 = reg_val1&0xf1;
  
  reg_val2 = reg_val2&0x0f;
  
  if(end_flag)
  {
    reg_val1 = reg_val1|0x01;
  }
  else
  {
    reg_val1 = reg_val1&0xfe;
  }
  
  switch(data_val)
  {
    case 0:  //A B C D E F
    {
      reg_val1 = reg_val1|0x0a;  //E F
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 1:  //B C
    {
      reg_val2 = reg_val2|0x60;  //B C
    }
    break;
    
    case 2:  //A B D E G
    {
      reg_val1 = reg_val1|0x0c;  //E G
  
      reg_val2 = reg_val2|0xb0;  //A B D
    }
    break;
    
    case 3:  //A B C D G
    {
      reg_val1 = reg_val1|0x04;  //G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 4:  //B C F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0x60;  //B C 
    }
    break;
    
    case 5:  //A C D F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0xd0;  //A C D
    }
    break;
    
    case 6:  //A C D E F G
    {
      reg_val1 = reg_val1|0x0e;  //E F G
  
      reg_val2 = reg_val2|0xd0;  //A C D
    }
    break;
    
    case 7:  //A B C
    {
      reg_val2 = reg_val2|0x70;  //A B C
    }
    break;
    
    case 8:  //A B C D E F G
    {
      reg_val1 = reg_val1|0x0e;  //E F G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    case 9:  //A B C D F G
    {
      reg_val1 = reg_val1|0x06;  //F G
  
      reg_val2 = reg_val2|0xf0;  //A B C D
    }
    break;
    
    default:
    break;
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr-1,reg_val1); 
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr,reg_val2); 
}

/*******************************************************************************
//Display the number 6(E G F endflag D C B A)
*******************************************************************************/
static void Display_Num_6(uint8_t ram_addr,uint8_t data_val,bool end_flag)
{
  uint8_t reg_val =0;
  
  if(end_flag)
  {
    reg_val = reg_val|0x10;
  }
  
  switch(data_val)
  {
    case 0:  //A B C D E F
    {
      reg_val = reg_val|0xaf;
    }
    break;
    
    case 1:  //B C
    {
      reg_val = reg_val|0x06;
    }
    break;
    
    case 2:  //A B D E G
    {
      reg_val = reg_val|0xcb;
    }
    break;
    
    case 3:  //A B C D G
    {
      reg_val = reg_val|0x4f;
    }
    break;
    
    case 4:  //B C F G
    {
      reg_val = reg_val|0x66;
    }
    break;
    
    case 5:  //A C D F G
    {
      reg_val = reg_val|0x6d;
    }
    break;
    
    case 6:  //A C D E F G
    {
      reg_val = reg_val|0xed;
    }
    break;
    
    case 7:  //A B C
    {
      reg_val = reg_val|0x07;
    }
    break;
    
    case 8:  //A B C D E F G
    {
      reg_val = reg_val|0xef;
    }
    break;
    
    case 9:  //A B C D F G
    {
      reg_val = reg_val|0x6f;
    }
    break;
    
    default:
    break;
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,ram_addr,reg_val); 
}

/*******************************************************************************
//clear up area
*******************************************************************************/
void HT9B95A_clear_up_area(void)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
   
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0x0f));  //PH S21 S20 S19

  Display_Num_1_2_3_4_5(0x0e,DISPLAY_NULL,0);  //Number 3/S24
  
  Display_Num_1_2_3_4_5(0x0f,DISPLAY_NULL,0);  //Number 4/S23
  
  Display_Num_1_2_3_4_5(0x10,DISPLAY_NULL,0);  //Number 5/S22
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x11,&reg_val);  
    
  Display_Num_6(0x11,DISPLAY_NULL,(bool)((reg_val>>4)&0x01));  //Number 6/S1
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x10,&reg_val);
   
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x10,(uint8_t)(reg_val&0xf0));  //P1 S26 S27 S28
}
/*******************************************************************************
//clear down area
*******************************************************************************/
void HT9B95A_clear_down_area(void)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x00,&reg_val);  
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,reg_val&0x0f);  //S28 S29 S30 S31
  
  Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1
  
  Display_Num_7_8_9(0x02,DISPLAY_NULL,0);  //Number 8/S32
  
  Display_Num_7_8_9(0x03,DISPLAY_NULL,0);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_10(0x04,DISPLAY_NULL,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);  
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val&0xf0);  //S34 S35 S36 P2
}


/*******************************************************************************
//Display the temprature value
*******************************************************************************/
void HT9B95A_Display_BreathingRate_Val(uint8_t BreathingRate_Val)
{
  //bool val_flag=0;  //"0:+, 1:-"
  //uint16_t data_val;
  uint8_t f_val,s_val,t_val,l_val,reg_val;
  
  HT9B95A_clear_up_area();  //clear area
  
  //if(temp_flag)  //1:Fahrenheit Temperatures,0://Celsius Temperatures
  //{
    //temp_val = 1.8*temp_val + 32;  //Change to Fahrenheit Temperatures
  //}
  
  //if(temp_val<0)
  //{
    //val_flag=1;  //"0:+, 1:-"
    
    //temp_val=fabs(temp_val);  //Get value
  //}
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
  
  reg_val &= 0x0f;
  
  //if(val_flag)
  //{
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x50));  //S19 S21
  //}
  //else
  //{
    //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x10));  //S19
  //}
  
  //data_val=(uint16_t)(10*temp_val);
    
  f_val= BreathingRate_Val/100;
  
  BreathingRate_Val = BreathingRate_Val%100;
    
  s_val= BreathingRate_Val/10;

   BreathingRate_Val= BreathingRate_Val%10;
  
  t_val= BreathingRate_Val/1;

  l_val=BreathingRate_Val%1;

  if(f_val == 1)
  {
    Display_Num_1_2_3_4_5(0x0e,f_val,1);  //Number 3/S24//Display_Num_7_8_9(0x01,f_val,1);  //Number 7/L1
  }
  else
  {
    Display_Num_1_2_3_4_5(0x0e,DISPLAY_NULL,1);  //Number 3/S24//Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1
  }
  
  if((s_val > 0)||(f_val == 1))
  {
    Display_Num_1_2_3_4_5(0x0f,s_val,0);  //Number 4/S23//Display_Num_7_8_9(0x02,s_val,1);  //Number 8/S32
  }
  else
  {
    Display_Num_1_2_3_4_5(0x0f,DISPLAY_NULL,0);  //Number 4/S23//Display_Num_7_8_9(0x02,DISPLAY_NULL,1);  //Number 8/S32
  }
  
  Display_Num_1_2_3_4_5(0x10,t_val,1);  //Number 5/S22 //Display_Num_7_8_9(0x03,t_val,0);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x11,&reg_val); //LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_6(0x11,l_val,(bool)((reg_val>>4)&0x01));  //Number 6/S1 //Display_Num_10(0x04,0x00,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x10,&reg_val);  //LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x10,reg_val|0x08); //P1//LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val|0x01); //P2 
  //f_val=data_val/1000;
  
  //data_val=data_val%1000;
  
  //s_val=data_val/100;
  
  //data_val=data_val%100;

  
  //t_val=data_val/10;
  
  //l_val=data_val%10;
  
  //if(f_val>0)
  //{
    //if(temp_flag == 0)  //Celsius Temperatures
    //{
     // Display_Num_1_2_3_4_5(0x0e,f_val,1);  //Number 3/S24
    //}
    //else  //1:Fahrenheit Temperatures
    //{
     // Display_Num_1_2_3_4_5(0x0e,f_val,0);  //Number 3/S24
    //}
  //}
  //else
  //{
    //if(temp_flag == 0)  //Celsius Temperatures
    //{
      //Display_Num_1_2_3_4_5(0x0e,DISPLAY_NULL,1);  //Number 3/S24
    //}
    //else  //1:Fahrenheit Temperatures
    //{
      //Display_Num_1_2_3_4_5(0x0e,DISPLAY_NULL,0);  //Number 3/S24
    //}
  //}
  
  //if(s_val>0)
  //{
    //if(temp_flag == 0)  //Celsius Temperatures
   // {
      //Display_Num_1_2_3_4_5(0x0f,s_val,0);  //Number 4/S23
   // }
    //else  //1:Fahrenheit Temperatures
    //{
      //Display_Num_1_2_3_4_5(0x0f,s_val,1);  //Number 4/S23
    //}
  //}
  //else
  //{
   // if(temp_flag == 0)  //Celsius Temperatures
    //{
     // Display_Num_1_2_3_4_5(0x0f,DISPLAY_NULL,0);  //Number 4/S23
   // }
   // else  //1:Fahrenheit Temperatures
   // {
    //  Display_Num_1_2_3_4_5(0x0f,DISPLAY_NULL,1);  //Number 4/S23
   // }
  //}
  
  //Display_Num_1_2_3_4_5(0x10,t_val,1);  //Number 5/S22
  
  //LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x11,&reg_val);  
    
  //Display_Num_6(0x11,l_val,(bool)((reg_val>>4)&0x01));  //Number 6/S1
  
  //LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x10,&reg_val);  
  
  //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x10,reg_val|0x08); //P1 
}

/*******************************************************************************
//Display the light value
*******************************************************************************/
/*void HT9B95A_Display_Light_Val(float light_val)
{
  bool max_flag=0;
  bool mid_flag=0;
  uint8_t reg_val;
  unsigned long data_val;
  uint8_t f_val,s_val,t_val,p_val;
  
  HT9B95A_clear_up_area();  //clear area
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
  
  reg_val &= 0x0f;
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x20));  //S20

  data_val=(unsigned long)(10*light_val);
  
  data_val=data_val%1000000;
  
  if(data_val>=100000)
  {
    max_flag=1;
    
    f_val=data_val/100000;
    
    data_val=data_val%100000;
    
    s_val=data_val/10000;
    
    data_val=data_val%10000;
    
    t_val=data_val/1000;
    
    data_val=data_val%1000;
    
    p_val=data_val/100;
  }
  else if(data_val>=10000)
  {
    mid_flag=1;
    
    f_val=data_val/10000;
    
    data_val=data_val%10000;
    
    s_val=data_val/1000;
    
    data_val=data_val%1000;
    
    t_val=data_val/100;
    
    data_val=data_val%100;
    
    p_val=data_val/10;
  }
  else
  {
    f_val=data_val/1000;
    
    data_val=data_val%1000;
    
    s_val=data_val/100;
    
    data_val=data_val%100;
    
    t_val=data_val/10;
    
    p_val=data_val%10;
  }
 
  
  if(f_val)
  {
    Display_Num_1_2_3_4_5(0x0e,f_val,0);  //Number 3/S24
  }
  else
  {
    Display_Num_1_2_3_4_5(0x0e,DISPLAY_NULL,0);  //Number 3/S24
  }
  
  if(f_val|s_val)
  {
    Display_Num_1_2_3_4_5(0x0f,s_val,0);  //Number 4/S23
  }
  else
  {
    Display_Num_1_2_3_4_5(0x0f,DISPLAY_NULL,0);  //Number 4/S23
  }
  
  Display_Num_1_2_3_4_5(0x10,t_val,0);  //Number 5/S22
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x11,&reg_val);  
    
  Display_Num_6(0x11,p_val,(bool)((reg_val>>4)&0x01));  //Number 6/S1
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x10,&reg_val);  
  
  if(max_flag)
  {
    reg_val = reg_val|0x06;  //S26 S27
  }
  if(mid_flag)
  {
    reg_val = reg_val|0x04;  //S26
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x10,reg_val|0x08|0x01); //P1/S25
}
*/

/*******************************************************************************
//Display the HeartRate value
*******************************************************************************/
void HT9B95A_Display_HeartRate_Val(uint8_t HeartRate_val)
{
  //bool val_flag=0;  //"0:+, 1:-"
  //uint16_t data_val;
  uint8_t f_val,s_val,t_val,reg_val;
  
  HT9B95A_clear_down_area();  //clear area
  
  //if(temp_flag)  //1:Fahrenheit Temperatures,0://Celsius Temperatures
  //{
    //temp_val = 1.8*temp_val + 32;  //Change to Fahrenheit Temperatures
  //}
  
  //if(temp_val<0)
  //{
    //val_flag=1;  //"0:+, 1:-"
    
    //temp_val=fabs(temp_val);  //Get value
  //}
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x00,&reg_val);
    
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,reg_val|0x80); //S28 
  //LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
  
  //reg_val &= 0x0f;
  
  //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x50));
  //if(val_flag)
  //{
    //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x50));  //S19 S21
  //}
  //else
  //{
    //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x10));  //S19
  //}
  
  f_val=HeartRate_val/100;
  
  HeartRate_val = HeartRate_val%100;
    
  s_val=HeartRate_val/10;
  
  t_val=HeartRate_val%10;


  if(f_val == 1)
  {
    Display_Num_7_8_9(0x01,f_val,1);  //Number 7/L1
  }
  else
  {
    Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1
  }
  
  if((s_val > 0)||(f_val == 1))
  {
    Display_Num_7_8_9(0x02,s_val,1);  //Number 8/S32
  }
  else
  {
    Display_Num_7_8_9(0x02,DISPLAY_NULL,1);  //Number 8/S32
  }
  
  Display_Num_7_8_9(0x03,t_val,0);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_10(0x04,0x00,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val|0x01); //P2 
}

/*******************************************************************************
//Display the Humility value
*******************************************************************************/
/*void HT9B95A_Display_Humi_Val(uint8_t humi_val)
{
  uint8_t f_val,s_val,t_val,reg_val;
  
  HT9B95A_clear_down_area();  //clear area
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x00,&reg_val);
    
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,reg_val|0x80); //S28 
  
  f_val=humi_val/100;
  
  humi_val = humi_val%100;
    
  s_val=humi_val/10;
  
  t_val=humi_val%10;


  if(f_val == 1)
  {
    Display_Num_7_8_9(0x01,f_val,1);  //Number 7/L1
  }
  else
  {
    Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1
  }
  
  if((s_val > 0)||(f_val == 1))
  {
    Display_Num_7_8_9(0x02,s_val,1);  //Number 8/S32
  }
  else
  {
    Display_Num_7_8_9(0x02,DISPLAY_NULL,1);  //Number 8/S32
  }
  
  Display_Num_7_8_9(0x03,t_val,0);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_10(0x04,0x00,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val|0x01); //P2 
}
*/
/*******************************************************************************
//Display the wind speed
*******************************************************************************/
/*void HT9B95A_Display_Wind_Speed(float wind_speed)
{
  uint16_t data_val;
  uint8_t f_val,s_val,t_val,reg_val;
  
  HT9B95A_clear_down_area();  //clear area
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x00,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,reg_val|0x40); //S29 
  
  data_val = (uint16_t)(10*wind_speed);
    
  f_val=data_val/100;
  
  data_val = data_val%100;
    
  s_val=data_val/10;
  
  t_val=data_val%10;

  Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1

  
  if(f_val > 0)
  {
    Display_Num_7_8_9(0x02,f_val,0);  //Number 8/S32
  }
  else
  {
    Display_Num_7_8_9(0x02,DISPLAY_NULL,0);  //Number 8/S32
  }
  
  Display_Num_7_8_9(0x03,s_val,1);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_10(0x04,t_val,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val|0x01); //P2 
}

*******************************************************************************
//Display the CO2 value
*******************************************************************************/
/*void HT9B95A_Display_CO2_val(float co2_val)
{
  bool max_flag=0;
  bool mid_flag=0;
  uint8_t reg_val;
  unsigned long data_val;
  uint8_t f_val,s_val,t_val,p_val;
  
  data_val=(unsigned long)(10*co2_val);
  
  data_val=data_val%1000000;
  
  if(data_val>=100000)
  {
    max_flag=1;
    
    f_val=data_val/100000;
    
    data_val=data_val%100000;
    
    s_val=data_val/10000;
    
    data_val=data_val%10000;
    
    t_val=data_val/1000;
    
    data_val=data_val%1000;
    
    p_val=data_val/100;
  }
  else if(data_val>=10000)
  {
    mid_flag=1;
    
    f_val=data_val/10000;
    
    data_val=data_val%10000;
    
    s_val=data_val/1000;
    
    data_val=data_val%1000;
    
    t_val=data_val/100;
    
    data_val=data_val%100;
    
    p_val=data_val/10;
  }
  else
  {
    f_val=data_val/1000;
    
    data_val=data_val%1000;
    
    s_val=data_val/100;
    
    data_val=data_val%100;
    
    t_val=data_val/10;
    
    p_val=data_val%10;
  }
  
  HT9B95A_clear_down_area();  //clear area

  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x00,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x00,reg_val|0x10); //S31
  
  if(f_val)
  {
    Display_Num_7_8_9(0x01,f_val,1);  //Number 7/L1
  }
  else
  {
    Display_Num_7_8_9(0x01,DISPLAY_NULL,1);  //Number 7/L1
  }
  
  if(f_val|s_val)
  {
    Display_Num_7_8_9(0x02,s_val,0);  //Number 8/S32  
  }
  else
  {
    Display_Num_7_8_9(0x02,DISPLAY_NULL,0);  //Number 8/S32
  }
  
  Display_Num_7_8_9(0x03,t_val,0);  //Number 9/S33
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val);  
  
  Display_Num_10(0x04,p_val,(bool)(reg_val>>7));  //Number 10/B4
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x03,&reg_val);
  
  if(max_flag)
  {
    reg_val = reg_val|0x06;  //S35 S36
  }
  if(mid_flag)
  {
    reg_val = reg_val|0x02;  //S35
  }
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x03,reg_val|0x08|0x01); //P2/S34
}

*/

///*******************************************************************************
////Display the light value
//*******************************************************************************/
//void HT9B95A_Display_Light_Val(float light_val)
//{
//  bool end_flag;
//  bool max_flag=0;
//  bool mid_flag=0;
//  uint8_t reg_val;
//  unsigned long data_val;
//  uint8_t f_val,s_val,t_val,p_val;
//  
//  data_val=(unsigned long)(10*light_val);
//  
//  data_val=data_val%1000000;
//  
//  if(data_val>=100000)
//  {
//    mid_flag=1;
//    
//    max_flag=1;
//    
//    f_val=data_val/100000;
//    
//    data_val=data_val%100000;
//    
//    s_val=data_val/10000;
//    
//    data_val=data_val%10000;
//    
//    t_val=data_val/1000;
//    
//    data_val=data_val%1000;
//    
//    p_val=data_val/100;
//  }
//  else if(data_val>=10000)
//  {
//    mid_flag=1;
//    
//    f_val=data_val/10000;
//    
//    data_val=data_val%10000;
//    
//    s_val=data_val/1000;
//    
//    data_val=data_val%1000;
//    
//    t_val=data_val/100;
//    
//    data_val=data_val%100;
//    
//    p_val=data_val/10;
//  }
//  else
//  {
//    f_val=data_val/1000;
//    
//    data_val=data_val%1000;
//    
//    s_val=data_val/100;
//    
//    data_val=data_val%100;
//    
//    t_val=data_val/10;
//    
//    p_val=data_val%10;
//  }
//  
//  MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x10,&reg_val);
//    
//  if(reg_val&0x01)
//  {
//    end_flag=1;
//  }
//  else
//  {
//    end_flag=0;
//  }
//  
//  if(f_val)
//  {
//    Display_Data_Num(0x10,f_val,end_flag);
//  }
//  else
//  {
//    Display_Data_Num(0x10,DISPLAY_NULL,end_flag);
//  }
//  
//  MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0f,&reg_val);
//    
//  if(reg_val&0x01)
//  {
//    end_flag=1;
//  }
//  else
//  {
//    end_flag=0;
//  }
//  
//  if(s_val|f_val)
//  {
//    Display_Data_Num(0x0f,s_val,end_flag);
//  }
//  else
//  {
//    Display_Data_Num(0x0f,DISPLAY_NULL,end_flag);
//  }
//
//  Display_Data_Num(0x0e,t_val,mid_flag);
//  
//  Display_Data_Num(0x0d,p_val,max_flag);
//}

///*******************************************************************************
////Display the Ext1 Temp value
//*******************************************************************************/
//void HT9B95A_Display_Ext1Temp_Val(float temp_val,bool temp_flag)
//{
//  bool val_flag=0;  //"0:+, 1:-"
//  bool c_f_flag =0;
//  uint16_t data_val;
//  uint8_t f_val,s_val,t_val,reg_val;
//  
//  if(temp_val==ERROR_CODE)
//  {
//    Display_Nomal_Num(0x02,DISPLAY_NULL,0);  //Number 10
//    
//    Display_Nomal_Num(0x03,DISPLAY_NULL,0);  //Number 11
//    
//    Display_Nomal_Num(0x04,DISPLAY_NULL,0);  //Number 12
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0a,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0a,(uint8_t)(reg_val&0xfe));  //T13
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0b,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0b,(uint8_t)(reg_val&0xfe));  //T12
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0xe1));  //X1 X2 X3 T25
//  }
//  else
//  {
//    if(temp_flag == 0)  //Celsius Temperatures
//    {
//      if(temp_val<0)
//      {
//        val_flag=1;  //"0:+, 1:-"
//        
//        temp_val=fabs(temp_val);
//      }
//      
//      data_val=(uint16_t)(10*temp_val);
//      
//      if(data_val>=1000)
//      {
//        c_f_flag =1;
//        
//        data_val=data_val%1000;
//      }
//
//      f_val=data_val/100;
//      
//      data_val=data_val%100;
//      
//      s_val=data_val/10;
//      
//      t_val=data_val%10;
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0b,&reg_val);
//    
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0b,(uint8_t)(reg_val|0x01));  //T12
//      
//      if((f_val>0)||(c_f_flag == 1))
//      {
//        Display_Nomal_Num(0x02,f_val,c_f_flag);  //Number 10
//      }
//      else
//      {
//        Display_Nomal_Num(0x02,DISPLAY_NULL,0);  //Number 10
//      }
//      
//      Display_Nomal_Num(0x03,s_val,val_flag);  //Number 11  
//      
//      Display_Nomal_Num(0x04,t_val,1);  ////Number 12
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//    
//      reg_val &= 0xf1;
//        
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x0a));  //X2 X3
//    }
//    else  //Fahrenheit Temperatures
//    {
//      temp_val = 1.8*temp_val + 32;  //Change to Fahrenheit Temperatures
//      
//      if(temp_val<0)
//      {
//        val_flag=1;  //"0:+, 1:-"
//        
//        temp_val=fabs(temp_val);
//      }
//      
//      data_val=(uint16_t)(10*temp_val);
//      
//      if(data_val>=1000)
//      {
//        c_f_flag =1;
//        
//        data_val=data_val%1000;
//      }
//
//      f_val=data_val/100;
//      
//      data_val=data_val%100;
//      
//      s_val=data_val/10;
//      
//      t_val=data_val%10;
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0b,&reg_val);
//    
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0b,(uint8_t)(reg_val|0x01));  //T12
//      
//      if((f_val>0)||(c_f_flag == 1))
//      {
//        Display_Nomal_Num(0x02,f_val,c_f_flag);  //Number 10
//      }
//      else
//      {
//        Display_Nomal_Num(0x02,DISPLAY_NULL,0);  //Number 10
//      }
//      
//      Display_Nomal_Num(0x03,s_val,val_flag);  //Number 11  
//      
//      Display_Nomal_Num(0x04,t_val,1);  ////Number 12
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//    
//      reg_val &= 0xf1;
//        
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x06));  //X1 X3
//    }
//  }
//}
//
///*******************************************************************************
////Display the Ext2 Temp value
//*******************************************************************************/
//void HT9B95A_Display_Ext2Temp_Val(float temp_val,bool temp_flag)
//{
//  bool val_flag=0;  //"0:+, 1:-"
//  bool c_f_flag =0;
//  uint16_t data_val;
//  uint8_t f_val,s_val,t_val,reg_val;
//
//  if(temp_val==ERROR_CODE)
//  {
//    Display_Nomal_Num(0x06,DISPLAY_NULL,0);  //Number 13
//    
//    Display_Nomal_Num(0x07,DISPLAY_NULL,0);  //Number 14
//    
//    Display_Nomal_Num(0x08,DISPLAY_NULL,0);  //Number 15
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0x1f));  //T14 T15 T24
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x09,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x09,(uint8_t)(reg_val&0xc0));  //X4 X5 X6
//  }
//  else
//  {
//    if(temp_flag == 0)  //Celsius Temperatures
//    {
//      if(temp_val<0)
//      {
//        val_flag=1;  //"0:+, 1:-"
//        
//        temp_val=fabs(temp_val);
//      }
//      
//      data_val=(uint16_t)(10*temp_val);
//      
//      if(data_val>=1000)
//      {
//        c_f_flag =1;
//        
//        data_val=data_val%1000;
//      }
//        
//      f_val=data_val/100;
//      
//      data_val=data_val%100;
//      
//      s_val=data_val/10;
//      
//      t_val=data_val%10;
//      
//      if((f_val>0)||(c_f_flag == 1))
//      {
//        Display_Nomal_Num(0x06,f_val,c_f_flag);  //Number 13
//      }
//      else
//      {
//        Display_Nomal_Num(0x06,DISPLAY_NULL,0);  //Number 13
//      }
//      
//      Display_Nomal_Num(0x07,s_val,1);  //Number 14 T23
//      
//      Display_Nomal_Num(0x08,t_val,0);  //Number 15 T26
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//      
//      reg_val &= 0x1f;
//    
//      if(val_flag)
//      {
//        MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0xa0));  //T14 T24
//      }
//      else
//      {
//        MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x20));  //T14
//      }
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x09,&reg_val);
//    
//      reg_val &= 0xc0;
//        
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x09,(uint8_t)(reg_val|0x18));  //X5 X6
//    }
//    else  //Fahrenheit Temperatures
//    {
//      temp_val = 1.8*temp_val + 32;  //Change to Fahrenheit Temperatures
//      
//      if(temp_val<0)
//      {
//        val_flag=1;  //"0:+, 1:-"
//        
//        temp_val=fabs(temp_val);
//      }
//      
//      data_val=(uint16_t)(10*temp_val);
//      
//      if(data_val>=1000)
//      {
//        c_f_flag =1;
//        
//        data_val=data_val%1000;
//      }
//        
//      f_val=data_val/100;
//      
//      data_val=data_val%100;
//      
//      s_val=data_val/10;
//      
//      t_val=data_val%10;
//      
//      if((f_val>0)||(c_f_flag == 1))
//      {
//        Display_Nomal_Num(0x06,f_val,c_f_flag);  //Number 13
//      }
//      else
//      {
//        Display_Nomal_Num(0x06,DISPLAY_NULL,0);  //Number 13
//      }
//      
//      Display_Nomal_Num(0x07,s_val,1);  //Number 14 T23
//      
//      Display_Nomal_Num(0x08,t_val,0);  //Number 15 T26
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//      
//      reg_val &= 0x1f;
//    
//      if(val_flag)
//      {
//        MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0xa0));  //T14 T24
//      }
//      else
//      {
//        MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x20));  //T14
//      }
//      
//      MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x09,&reg_val);
//    
//      reg_val &= 0xc0;
//        
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x09,(uint8_t)(reg_val|0x28));  //X4 X6
//    }
//  }
//}
//
///*******************************************************************************
////Display the Ext2 Number value
//*******************************************************************************/
//void HT9B95A_Display_Ext2_Val(float Ext2_val)
//{
//  bool val_flag=0;  //"0:+, 1:-"
//  bool c_f_flag =0;
//  uint16_t data_val;
//  uint8_t f_val,s_val,t_val,reg_val;
//
//  if(Ext2_val==ERROR_CODE)
//  {
//    Display_Nomal_Num(0x06,DISPLAY_NULL,0);  //Number 13
//    
//    Display_Nomal_Num(0x07,DISPLAY_NULL,0);  //Number 14
//    
//    Display_Nomal_Num(0x08,DISPLAY_NULL,0);  //Number 15
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0x1f));  //T14 T15 T24
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x09,&reg_val);
//    
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x09,(uint8_t)(reg_val&0xc0));  //X4 X5 X6
//  }
//  else
//  {
//    if(Ext2_val<0)
//    {
//      val_flag=1;  //"0:+, 1:-"
//      
//      Ext2_val=fabs(Ext2_val);
//    }
//    
//    data_val=(uint16_t)(10*Ext2_val);
//    
//    if(data_val>1999)
//    {
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,0x10);  //Number 13 T19
//      
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,0x11);  //Number 14 T23
//      
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x08,0x10);  //Number 15 T26
//    }
//    else
//    {
//      if(data_val>=1000)
//      {
//        c_f_flag =1;
//        
//        data_val=data_val%1000;
//      }
//
//      f_val=data_val/100;
//      
//      data_val=data_val%100;
//      
//      s_val=data_val/10;
//      
//      t_val=data_val%10;
//      
//      if((f_val>0)||(c_f_flag == 1))
//      {
//        Display_Nomal_Num(0x06,f_val,c_f_flag);  //Number 13
//      }
//      else
//      {
//        Display_Nomal_Num(0x06,DISPLAY_NULL,0);  //Number 13
//      }
//      
//      Display_Nomal_Num(0x07,s_val,1);  //Number 14 T23
//      
//      Display_Nomal_Num(0x08,t_val,0);  //Number 15 T26
//    }
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//      
//    reg_val &= 0x7f;
//      
//    if(val_flag)
//    {
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x80));  //T24
//    }
//    else
//    {
//      MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val));  //T24
//    }
//    
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x09,&reg_val);
//      
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x09,(uint8_t)(reg_val&0xc0)); 
//  }
//}

/*******************************************************************************
//Display the Usage value
*******************************************************************************/
/*void HT9B95A_Display_Usage_Val(uint8_t usage_val)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x12,&reg_val1);
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val2);
  
  reg_val1 = reg_val1&0x00;
  
  reg_val2 = reg_val2&0x0f; 
  
  if(usage_val >= 95) //S37 T10-T1
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0xfe);  //S37 T6 T5 T4 T3 T2 T1
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 85)  //S37 T10-T2
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0xee);  //S37 T6 T5 T4 T3 T2
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 75)  //S37 T10-T3
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0xce);  //S37 T6 T5 T4 T3
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 65)  //S37 T10-T4
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x8e);  //S37 T6 T5 T4
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 55)  //S37 T10-T5
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x0e);  //S37 T6 T5
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 45)  //S37 T10-T6
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x0a);  //S37 T6
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 35)  //S37 T10-T7
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x02);  //S37
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0xf0);  //T10 T9 T8 T7
  }
  else if(usage_val >= 25)  //S37 T10-T8
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x02);  //S37
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0x70);  //T10 T9 T8
  }
  else if(usage_val >= 15)  //S37 T10-T9
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x02);  //S37
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0x30);  //T10 T9
  }
  else if(usage_val >= 5)  //S37 T10
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x02);  //S37
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2|0x10);  //T10
  }
  else  //S37 
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,reg_val1|0x02);  //S37
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val2);  //
  }
}

*******************************************************************************
//Display the Power value
*******************************************************************************/
/*void HT9B95A_Display_Power_val(float power_val)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x04,&reg_val1);
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x07,&reg_val2);
  
  reg_val1 = reg_val1&0x7f;
  
  reg_val2 = reg_val2&0x71;
  
  if(power_val>=4)  //B0 B1 B2 B3 B4 B5
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1|0x80);  //B4
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x8f);  //B0 B1 B2 B3 B5
  }
  else if(power_val>=3.87)  //B0 B2 B3 B4 B5
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1|0x80);  //B4
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x8d);  //B0 B2 B3 B5
  }
  else if(power_val>=3.79)  //B0 B5 B4 B3
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1|0x80);  //B4
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x89);  //B0 B5 B3
  }
  else if(power_val>=3.73)  //B0 B5 B4
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1|0x80);  //B4
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x81);  //B0 B5
  }
  else if(power_val>=3.5)  //B0 B5
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1);  //
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x81);  //B0 B5
  }
  else  //B0
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x04,reg_val1);
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x01);  //B0
  }
}

******************************************************************************
//Display the Rssi value
//Rssi_val:rssi value
******************************************************************************/
/*void HT9B95A_Display_Rssi_val(int Rssi_val)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x06,&reg_val1);
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x07,&reg_val2);
  
  reg_val1 = reg_val1&0xfc;  //S16 S17
  
  reg_val2 = reg_val2&0x8f;  //S14 S15 S18
    
  if(Rssi_val == ERROR_CODE)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1);  //not dispaly S16 S17
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2);  //not dispaly S14 S15
  }
  if((Rssi_val<0)&&(Rssi_val>-120))
  {
    if(Rssi_val<=-85)
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1|0x01);  //dispaly S17
      
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2);  //not dispaly S14 S15
    }
    else if(Rssi_val<=-75)
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1|0x03);  //not dispaly S17 S16
      
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2);  //not dispaly S14 S15
    }
    else if(Rssi_val<=-65)
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1|0x03);  //not dispaly S17 S16
      
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x20);  //dispaly S15
    }
    else
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1|0x03);  //dispaly S17 S16
      
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2|0x60);  //dispaly S15 S14
    }
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val1);  //not dispaly S16 S17
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,reg_val2);  //not dispaly S14 S15
  }
}

******************************************************************************
//Display the WiFi status
//connect:0/1,not connect/connect
******************************************************************************/
/*void HT9B95A_Display_WiFi_Status(bool connect)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x07,&reg_val);
  
  if(connect)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,0xef&reg_val);  //not display S18
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x07,0x10|reg_val);  //display S18
  }
}

*******************************************************************************
//Display the Internet Status
*******************************************************************************/
/*void Display_Internet_Connect(bool net_status,bool net_all)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x06,&reg_val);
    
  reg_val = reg_val&0xf3;  //S12 S13
  
  if(net_all)
  {
    if(net_status)
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,(uint8_t)(reg_val|0x08));  //S12
    }
    else
    {
      LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,(uint8_t)(reg_val|0x0c));  //S12 S13
    }
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val);
  }
}

*******************************************************************************
//Display the Data Post Status
*******************************************************************************/
/*void Display_Data_Post(bool post_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x06,&reg_val);

  if(post_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val|0x80);  //S11
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val&0x7f);  //S11
  }
}

*******************************************************************************
//Display the AP Mode Status
*******************************************************************************/
/*void Display_AP_Mode(bool ap_mode)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);

  if(ap_mode)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,reg_val|0x08);  //S4
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,reg_val&0xf7);  //S4
  }
}

*******************************************************************************
////Display the Factory Reset Status
*******************************************************************************/
//void Display_Factory_Reset(bool f_reset)
//{
//  uint8_t reg_val;
//
//  MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x12,&reg_val);
//  
//  if(f_reset)
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,(uint8_t)(reg_val|0x20));  //M3
//  }
//  else
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,(uint8_t)(reg_val&0xdf));  //M3
//  }
//}

///*******************************************************************************
////Display the Water Check result
//*******************************************************************************/
//void Display_Water_Check(bool check_result)
//{
//  uint8_t i;
//  uint8_t reg_val;
//   
//  MAP_UtilsDelay(200000);  //delay about 15ms
//  
//  for(i=0;i<RETRY_TIME_OUT;i++)
//  {
//    MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x12,&reg_val);
//    
//    if((reg_val&0x01)==1)
//    {
//      break;
//    }
//    else
//    {
//      reg_val=reg_val|0x01;
//    }
//  }
//  
//  if(check_result)
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,(uint8_t)(reg_val|0x40));  //T2
//  }
//  else
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x12,(uint8_t)(reg_val&0xbf));  //T2
//  }
//}

/*******************************************************************************
//Display the SIM Card status
*******************************************************************************/
/*void Display_SimCard_Status(bool sim_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
  
  if(sim_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x02));  //S6
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0xfd));  //S6
  }
}

*******************************************************************************
//Display the USB status
*******************************************************************************/
/*void Display_usb_Status(bool usb_status)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x11,&reg_val);
  
  if(usb_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x11,(uint8_t)(reg_val|0x10));  //S1
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x11,(uint8_t)(reg_val&0xef));  //S1
  }
}

*******************************************************************************
//Display the Ethernet conneted status
*******************************************************************************/
/*void Display_Ethernet_Status(bool lan_status)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0c,&reg_val);
  
  if(lan_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0c,(uint8_t)(reg_val|0x01));  //S2
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x0c,(uint8_t)(reg_val&0xfe));  //S2
  }
}

*******************************************************************************
//Display the Gprs Rssi
//0,1-4,4-9,9-14,14-31,99
*******************************************************************************/
/*void Display_Gprs_Rssi(uint8_t gprs_rssi)
{
  uint8_t reg_val1,reg_val2;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val1);
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x06,&reg_val2);
  
  reg_val1 = reg_val1&0xfe;
  
  reg_val2 = reg_val2&0x8f;
  
  if((gprs_rssi>31)||(gprs_rssi<=0))
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,reg_val1);  
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val2);  
  }
  else if(gprs_rssi <= 4)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val1|0x01));  //S7
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,reg_val2);
  }
  else if(gprs_rssi <= 9)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val1|0x01));  //S7
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,(uint8_t)(reg_val2|0x10));  //S8
  }
  else if(gprs_rssi <= 14)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val1|0x01));  //S7
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,(uint8_t)(reg_val2|0x30));  //S8 S9
  }
  else if(gprs_rssi <= 31)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val1|0x01));  //S7
    
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x06,(uint8_t)(reg_val2|0x70));  //S8 S9 S10
  }
}

*******************************************************************************
//Display the error code
*******************************************************************************/
/*void Display_ErrCode(uint8_t err_code)
{
  uint8_t reg_val;
  uint8_t f_val,s_val;
    
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x0c,&reg_val);
 
  if(err_code)
  {
    f_val=err_code/10;
  
    s_val=err_code%10;
    
    Display_Num_1_2_3_4_5(0x0c,f_val,1);  //Number 1/S3
    
    Display_Num_1_2_3_4_5(0x0d,s_val,reg_val&0x01);  //Number 2/S2
  }
  else
  {
    Display_Num_1_2_3_4_5(0x0c,DISPLAY_NULL,0);  //Number 1/S3
    
    Display_Num_1_2_3_4_5(0x0d,DISPLAY_NULL,reg_val&0x01);  //Number 2/S2
  }
}

*******************************************************************************
////Display the ext temp/humi flag
*******************************************************************************/
//void Display_Ext_TH(bool ext_valid)
//{
//  uint8_t reg_val;
//  
//  MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x05,&reg_val);
//  
//  if(ext_valid)
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val|0x01));  //Q1
//  }
//  else
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x05,(uint8_t)(reg_val&0xfe));  //Q1
//  }
//}
//
///*******************************************************************************
////Display the ext light flag
//*******************************************************************************/
//void Display_Ext_LT(bool ext_valid)
//{
//  uint8_t reg_val;
//  
//  MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);
//  
//  if(ext_valid)
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,(uint8_t)(reg_val|0x01));  //Q2
//  }
//  else
//  {
//    MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,(uint8_t)(reg_val&0xfe));  //Q2
//  }
//}

/*******************************************************************************
//Clear all sensor Status
*******************************************************************************/
/*void clear_sensor_Status(void)
{
  uint8_t reg_val;
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val&0xf0);  //L5 l4 l3 l2
  
//  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,0x00);  //S41 S40 S39 S38 S42 S43 S44 S45
  
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x15,&reg_val);
  
  LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val&0x0f);  //L6 L7 L8 L9
}

*******************************************************************************
//Display the TH sensor Status
*******************************************************************************/
/*void Display_TH_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  //if(sensor_status)
  //{
    //LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x01);  //S45
  //}
  //else
  //{
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xfe);  //S45
   
    delay_us(80);

  //}
}

*******************************************************************************
//Display the TH sensor data Status
*******************************************************************************/
/*void Display_TH_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x15,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val|0x10);  //L9
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val&0xef);  //L9
  }
}

*******************************************************************************
//Display the E_TH sensor Status
*******************************************************************************/
/*void Display_E_TH_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x02);  //S44
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xfd);  //S44
  }
}

*******************************************************************************
//Display the E_TH sensor data Status
*******************************************************************************/
/*void Display_E_TH_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x15,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val|0x20);  //L8
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val&0xdf);  //L8
  }
}

*******************************************************************************
//Display the S_TH sensor Status
*******************************************************************************/
/*void Display_S_TH_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x04);  //S43
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xfb);  //S43
  }
}

*******************************************************************************
//Display the S_TH sensor data Status
*******************************************************************************/
/*void Display_S_TH_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x15,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val|0x40);  //L7
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val&0xbf);  //L7
  }
}

*******************************************************************************
//Display the E_T sensor Status
*******************************************************************************/
/*void Display_E_T_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x08);  //S42
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xf7);  //S42
  }
}

*******************************************************************************
//Display the E_T sensor data Status
*******************************************************************************/
/*void Display_E_T_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x15,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val|0x80);  //L6
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x15,reg_val&0x7f);  //L6
  }
}

*******************************************************************************
//Display the LIGHT sensor Status
*******************************************************************************/
/*void Display_LIGHT_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x80);  //S41
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0x7f);  //S41
  }
}

*******************************************************************************
//Display the LIGHT sensor data Status
*******************************************************************************/
/*void Display_LIGHT_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val|0x08);  //L5
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val&0xf7);  //L5
  }
}

*******************************************************************************
//Display the WS sensor Status
*******************************************************************************/
/*void Display_WS_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x40);  //S40
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xbf);  //S40
  }
}

*******************************************************************************
//Display the WS sensor data Status
*******************************************************************************/
/*void Display_WS_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val|0x04);  //L4
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val&0xfb);  //L4
  }
}

*******************************************************************************
//Display the CO2 sensor Status
*******************************************************************************/
/*void Display_CO2_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x20);  //S39
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xdf);  //S39
  }
}

*******************************************************************************
//Display the CO2 sensor data Status
*******************************************************************************/
/*void Display_CO2_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val|0x02);  //L3
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val&0xfd);  //L3
  }
}

*******************************************************************************
//Display the PH sensor Status
*******************************************************************************/
/*void Display_PH_Status(bool sensor_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x14,&reg_val);

  if(sensor_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val|0x10);  //S38
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x14,reg_val&0xef);  //S38
  }
}

*******************************************************************************
//Display the PH sensor data Status
*******************************************************************************/
/*void Display_PH_dataStatus(bool data_status)
{
  uint8_t reg_val;
   
  LCD_MulTry_IIC_RD_Reg(HT9B95A_ADDR,0x13,&reg_val);

  if(data_status)
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val|0x01);  //L2
  }
  else
  {
    LCD_MulTry_IIC_WR_Reg(HT9B95A_ADDR,0x13,reg_val&0xfe);  //L2
  }
}

*********************************END ***************************************/









