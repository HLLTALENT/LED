/*


CLI_Init(void);
初始化函数，主要为UART初始化
本例使用UART2，115200，8N1

*/
#ifndef _CLI_H_
#define _CLI_H_

extern void CLI_Init(void);
extern void CLI_echo(void);
extern void GIPO_INIT(void);
extern void on_pushButton_start_clicked();

#endif

