#ifndef __IIC_H
#define __IIC_H			 

#ifdef __cplusplus
 extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Uncomment the following line to use the default sEE_TIMEOUT_UserCallback() 
   function implemented in stm32072b_eval_i2c_ee.c file.
   sEE_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occurs during communication (waiting on an event that doesn't occur, bus 
   errors, busy devices ...). */   
#define USE_DEFAULT_TIMEOUT_CALLBACK 
     
#define sEE_HW_ADDRESS         0xA0   /* E0 = E1 = E2 = 0 */ 

#define sEE_I2C_TIMING         0x30113B3B//0x20101e1e//0x00100444
#define AFE_I2C_TIMING         0x40B22536
#define sEE_PAGESIZE           64
   
#define  countof(a)            (sizeof(a) / sizeof(*(a)))
#define  BufferSize            (countof(Write_Buffer)-1)
   

/* Maximum number of trials for sEE_WaitEepromStandbyState() function */
#define sEE_MAX_TRIALS_NUMBER     300

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void I2C_Device_Init();
/*
参数说明：
uint8_t slave_adr ：从机地址
uint8_t len_adr ：数据地址长度 
uint16_t data_adr：数据地址
uint8_t *data：数据存储地址
uint8_t len：读取数据长度
*/
uint8_t i2c_send_data_bytes( uint8_t slave_adr,uint8_t len_adr,uint16_t data_adr, uint8_t *data ,uint8_t len );
/*
参数说明：
uint8_t slave_adr ：从机地址
uint8_t len_adr ：数据地址长度 
uint16_t data_adr：数据地址
uint8_t *data：数据存储地址
uint8_t len：读取数据长度
*/
uint8_t i2c_recived_data_bytes(  uint8_t slave_adr,uint8_t len_adr,uint16_t data_adr, uint8_t *data ,uint8_t len );
/* USER Callbacks: These are functions for which prototypes only are declared in
   EEPROM driver and that should be implemented into user application. */  
/* sEE_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occurs during communication (waiting on an event that doesn't occur, bus 
   errors, busy devices ...).
   You can use the default timeout callback implementation by uncommenting the 
   define USE_DEFAULT_TIMEOUT_CALLBACK in stm32072b_eval_i2c_ee.h file.
   Typically the user implementation of this callback should reset I2C peripheral
   and re-initialize communication or in worst case reset all the application. */

/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  pBuffer: pointer to the buffer  containing the data to be written 
  *         to the EEPROM.
  * @param  WriteAddr: EEPROM's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the EEPROM.
  * @retval None
  */
void sEE_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
/**
  * @brief  Reads a block of data from the EEPROM.
  * @param  pBuffer: pointer to the buffer that receives the data read from 
  *         the EEPROM.
  * @param  ReadAddr: EEPROM's internal address to start reading from.
  * @param  NumByteToRead: pointer to the variable holding number of bytes to 
  *         be read from the EEPROM.
  *
  * @retval sEE_OK (0) if operation is correctly performed, else return value 
  *         different from sEE_OK (0) or the timeout user callback.
  */
uint32_t sEE_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead);
uint8_t get_eeprom_status(void);
#ifdef __cplusplus
}
#endif

#endif /* __ISO_I2C_EE_H */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */

/**********************************END OF FILE********************************/
