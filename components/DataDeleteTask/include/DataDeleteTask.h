/*******************************************************************************
  * @file       Nor Flash Data Deleted Application Task   
  * @author 
  * @version
  * @date 
  * @brief
  ******************************************************************************
  * @attention
  *
  *
*******************************************************************************/



#define DATA_SAVE_FLAG_ADDR     244     //unsigned char + 1(crc)

#define SUCCESS

extern  unsigned long   DELETE_ADDR;
extern  unsigned long   WRITE_ADDR;
extern  unsigned long   POST_NUM ;
extern  unsigned long   POST_ADDR ;
extern  unsigned long   SAVE_ADDR_SPACE ;
/*-------------------------------- Includes ----------------------------------*/


/*******************************************************************************
 FUNCTION PROTOTYPES
*******************************************************************************/

extern void osi_Erase_Memory(void);
extern void N25q_EraseMemory(unsigned long addr);
extern void Save_Data_Reset(void);
extern uint8_t Get_Usage_Val(void);
extern void at24c08_read_addr(void);
extern void osi_at24c08_read_addr(void);
extern void at24c08_save_addr(void);
extern void osi_at24c08_save_addr(void);


/*******************************************************************************
                                      END         
*******************************************************************************/





