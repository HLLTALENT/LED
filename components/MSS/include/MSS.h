/*
MSS  LOG

UART1 

*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef _MSS_H_
#define _MSS_H_

extern float BreathingRate_Out;
extern float HeartRate_Out;

extern void MSS_Init(void);
extern void MSS_Read_echo(void);
extern void Parse_Data(void);
extern float parseValueFloat(char* data_2, int valuePos, int valueSize);
extern uint16_t parseValueInt16(char* data_2, int valuePos, int valueSize);






#endif

