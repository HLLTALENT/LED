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
#include "stdint.h"
#include "stdbool.h"

/*******************************************************************************
//HT9B95A sensor register address
*******************************************************************************/
#define HT9B95A_ADDR		0X3E
#define HT9B95A_MAX_RAM_ADDR    0X15
#define DISPLAY_NULL            0xff
#define ERROR_CODE              0xffff



/*******************************************************************************
//FUNCTION PROTOTYPES
*******************************************************************************/
extern void HT9B95A_Init(bool lcd_reset);  //HT9B95A Init

extern void HT9B95A_OFF(void);

extern void HT9B95A_DisplayAll(void);

extern void HT9B95A_clear_up_area(void);

extern void HT9B95A_clear_down_area(void);

extern void HT9B95A_Display_Temp_Val(float temp_val,bool temp_flag);  //Display the temprature value

extern void HT9B95A_Display_Light_Val(float light_val);

extern void HT9B95A_Display_Humi_Val(uint8_t humi_val);  //Display the Humility value

extern void HT9B95A_Display_Wind_Speed(float wind_speed);

extern void HT9B95A_Display_CO2_val(float co2_val);

//extern void HT9B95A_Display_Light_Val(float light_val);  //Display the light value
//
//extern void HT9B95A_Display_Ext1Temp_Val(float temp_val,bool temp_flag);  //Display the Ext1 Temp value
//
//extern void HT9B95A_Display_Ext2Temp_Val(float temp_val,bool temp_flag);  //Display the Ext2 Temp value
//
//extern void HT9B95A_Display_Ext2_Val(float Ext2_val);

extern void HT9B95A_Display_Usage_Val(uint8_t usage_val);  //Display the Usage value

extern void HT9B95A_Display_Power_val(float power_val);  //Display the Power value

extern void HT9B95A_Display_Rssi_val(int Rssi_val);  //Display the Rssi value

extern void HT9B95A_Display_WiFi_Status(bool connect);  //Display the WiFi status

extern void Display_Internet_Connect(bool net_status,bool net_all);  //Display the Internet Status

extern void Display_Data_Post(bool post_status);  //Display the Data Post Status

extern void Display_AP_Mode(bool ap_mode);  //Display the AP Mode Status

//extern void Display_Factory_Reset(bool f_reset);  //Display the Factory Reset Status
//
//extern void Display_Water_Check(bool check_result);  //Display the Water Check result

extern void Display_SimCard_Status(bool sim_status);  //Display the SIM Card status

extern void Display_usb_Status(bool usb_status);  //Display the USB status

extern void Display_Ethernet_Status(bool lan_status);  //Display the Ethernet conneted status

extern void Display_Gprs_Rssi(uint8_t gprs_rssi);  //Display the Gprs Rssi

extern void Display_ErrCode(uint8_t err_code);  //Display the error code

extern void HT9B95A_Display_HeartRate_Val(uint8_t HeartRate_val);

extern void HT9B95A_Display_BreathingRate_Val(uint8_t BreathingRate_Val);

//extern void Display_Ext_TH(bool ext_valid);  //Display the ext temp/humi flag
//
//extern void Display_Ext_LT(bool ext_valid);  //Display the ext light flag

extern void clear_sensor_Status(void);

extern void Display_TH_Status(bool sensor_status);

extern void Display_TH_dataStatus(bool data_status);

extern void Display_E_TH_Status(bool sensor_status);

extern void Display_E_TH_dataStatus(bool data_status);

extern void Display_S_TH_Status(bool sensor_status);

extern void Display_S_TH_dataStatus(bool data_status);

extern void Display_E_T_Status(bool sensor_status);

extern void Display_E_T_dataStatus(bool data_status);

extern void Display_LIGHT_Status(bool sensor_status);

extern void Display_LIGHT_dataStatus(bool data_status);

extern void Display_WS_Status(bool sensor_status);

extern void Display_WS_dataStatus(bool data_status);

extern void Display_CO2_Status(bool sensor_status);

extern void Display_CO2_dataStatus(bool data_status);

extern void Display_PH_Status(bool sensor_status);

extern void Display_PH_dataStatus(bool data_status);

/*******************************************************************************
                                      END         
*******************************************************************************/



















