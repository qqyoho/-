/******************** (C) COPYRIGHT  源地工作室 ********************************
 * 文件名  ：i2c.c
 * 描述    ：初始化IIC及一些基本的操作
 * 作者    ：zhuoyingxingyu
 * 淘宝    ：源地工作室http://vcc-gnd.taobao.com/
 * 论坛地址：极客园地-嵌入式开发论坛http://vcc-gnd.com/
 * 版本更新: 2015-12-20
 * 硬件连接  :PB6->SCL and PB7->SDA 
 * 调试方式：J-Link-OB
********************************************************************************/
//头文件
#include<stdint.h>
#include "eeprom.h"
#include "fm33lg0xx_fl.h"
#include "protect_record.h"
#include "parameter.h"
#include "hardware.h"
#define  I2C1_SLAVE_ADDRESS7   (0x08<<1) 
#define  I2C0_SLAVE_ADDRESS7   (0xa0) 

#define       STARTBIT       0
#define       RESTARTBIT     1
#define       STOPBIT        2
#define       RXENABLE       3
#define       RXDISENABLE    4
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t  sEEAddress = sEE_HW_ADDRESS;   

uint16_t  sEEDataNum;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes peripherals used by the I2C EEPROM driver.
  * @param  None
  * @retval None
*/

/**
  * @brief  Initializes peripherals used by the I2C EEPROM driver.
  * @param  None
  * @retval None
*/

static uint8_t eeprom_status=0;

uint8_t get_eeprom_status(void)
{
  //获取eeprom的状态
   return eeprom_status;
}
/*
   
*/
void I2C_Device_Init(void)
{ 
    FL_I2C_MasterMode_InitTypeDef   IICInitStructer;
    FL_I2C_DeInit(I2C);
    IICInitStructer.clockSource = FL_CMU_I2C_CLK_SOURCE_APBCLK;
    IICInitStructer.baudRate = 90000;
    FL_I2C_MasterMode_Init(I2C, &IICInitStructer);
    FL_I2C_Master_Enable(I2C);
}


/*******************************************************************************
**FuncName: ;//
**Function: 发送 起始位 或者 停止位函数 带延时;
**Output  : 1  状态 0 正常 1超时;
**input   : 发送 bit;
**Create date : liyong @2021.6.3
**Modify  : 
*******************************************************************************/
uint8_t I2C_Send_Bit(uint8_t BIT_def)
{
    uint32_t i = 0;
    uint32_t flag = 1;

    switch( BIT_def )
    {
        case STARTBIT:
            FL_I2C_Master_EnableI2CStart(I2C);
            while( i < 10 )
            {
                if( FL_I2C_Master_I2CStart_overStatues(I2C) == 0 )
                {
                    flag = 0;
                    break;
                } 
                FL_DelayUs(1);
                i++;
            }
            break;
        case RESTARTBIT:
            FL_I2C_Master_EnableI2CRestart(I2C);
            while( i < 10 )
            {
                if( FL_I2C_Master_I2CreStart_overStatues(I2C) == 0 )
                {
                    flag = 0;
                    break;
                } 
                FL_DelayUs(1);
                i++;
            }
            break;
        case STOPBIT:
            FL_I2C_Master_EnableI2CStop(I2C);
            while( i < 10 )
            {
                if( FL_I2C_Master_I2CStop_overStatues(I2C) == 0 )
                {
                    flag = 0;
                    break;
                } 
                FL_DelayUs(1);
                i++;
            }
            break;
        case RXENABLE:
            FL_I2C_Master_EnableRX(I2C);
            while( i < 10 )
            {
                if( FL_I2C_Master_IsEnabledRX(I2C) )
                {
                    flag = 0;
                    break;
                } 
                FL_DelayUs(1);
                i++;
            }
            break;
        case RXDISENABLE:
            FL_I2C_Master_DisableRX(I2C);
            while( i < 10 )
            {
                if( FL_I2C_Master_IsEnabledRX(I2C) == 0 )
                {
                    flag = 0;
                    break;
                } 
                FL_DelayUs(1);
                i++;
            }
            break;    
        default:
            break;
    }

    return flag; 

}
/*******************************************************************************
**FuncName: ;//
**Function: 主机发送一个byte数据;
**Output  : 0：成功  1未成功;
**input   : 发送数据;
**Create date : liyong @2021.6.3
**Modify  : 
*******************************************************************************/
uint8_t I2C_Send_Byte(uint8_t x_byte)
{
    uint8_t flag = 1;
    uint16_t delay = 0;
    
    FL_I2C_Master_WriteTXBuff(I2C, x_byte);
    do
    {
        delay++;
        if( delay > 2000 ) return flag; 
    }
    while(!FL_I2C_Master_IsActiveFlag_TXComplete(I2C));
    FL_I2C_Master_ClearFlag_TXComplete(I2C);

    if(!FL_I2C_Master_IsActiveFlag_NACK(I2C))
    {
        flag = 0;
    }
    else
    {
        FL_I2C_Master_ClearFlag_NACK(I2C); 
    }
    return flag;
}
/*******************************************************************************
**FuncName: ;//
**Function: i2c读取数据;
**Output  : 获取数据是否成功;
**input   : 获取数据地址 ;
**Create date : liyong @
**Modify  : 
*******************************************************************************/
uint8_t I2C_Receive_Byte(uint8_t *x_byte)
{
    uint8_t flag = 1;
    uint16_t delay = 0;
    
    /*-------------- enable read -------------*/
    if( I2C_Send_Bit(RXENABLE) )
      return flag;
    
      do
      {
          delay++;
          if( delay > 2000 ) return flag; 
      }while( ! FL_I2C_Master_IsActiveFlag_RXComplete(I2C) );
      
     FL_I2C_Master_ClearFlag_RXComplete(I2C);
     *x_byte = FL_I2C_Master_ReadRXBuff(I2C);
     flag = 0;
     
    return flag;
}
/*
参数说明：
uint8_t slave_adr ：从机地址
uint8_t len_adr ：数据地址长度 
uint16_t data_adr：数据地址
uint8_t *data：数据存储地址
uint8_t len：读取数据长度
*/
uint8_t i2c_send_data_bytes( uint8_t slave_adr,uint8_t len_adr,uint16_t data_adr, uint8_t *data ,uint8_t len )
{
    uint8_t i;

    /* send a start condition to I2C bus */
    if(I2C_Send_Bit(STARTBIT))
        return 0xf1;
    /*-------------- disable read -------------*/
    if(I2C_Send_Bit(RXDISENABLE))
        return 0xf2;

    /* send slave address to I2C bus */
    if(I2C_Send_Byte(slave_adr))
    {
        return 0xf3;
    }

    if(len_adr == 1)
    {
        /* data adr transmission */
        if(I2C_Send_Byte(data_adr))
            return 0xf4;
    }
    else
    {
        /* data adr transmission */
        if(I2C_Send_Byte( data_adr>>8))
            return 0xf5;
        if(I2C_Send_Byte(data_adr))
            return 0xf6;
    }

	for (i = 0; i < len; i ++)
    {
        /* data transmission */
        if(I2C_Send_Byte(data[i]))
          return 0xf7;
	}

	/* send a stop condition to I2C bus */
    if(I2C_Send_Bit(STOPBIT))
        return 0xf8;
    /* wait until stop condition generate */
    
    return 0;
}
/*
参数说明：
uint32_t i2cx ：I2C1或者I2C0
uint8_t slaCOR114
R114en_adr ：数据地址长度 
uint16_t data_adr：数据地址
uint8_t *data：数据存储地址
uint8_t len：读取数据长度
*/
uint8_t i2c_recived_data_bytes(  uint8_t slave_adr,uint8_t len_adr,uint16_t data_adr, uint8_t *data ,uint8_t len )
{    
    /* send a start condition to I2C bus */
    if( I2C_Send_Bit(STARTBIT) )
      return 0xf1;
	/* send slave address to I2C bus */
     /*-------------- disable read -------------*/
    if( I2C_Send_Bit(RXDISENABLE) )
      return 0xf2;
    
    /* send slave address to I2C bus */
    if( I2C_Send_Byte( slave_adr) )
      return 0xf3;

    if( len_adr == 1 )
    {
        /* data adr transmission */
        if( I2C_Send_Byte( data_adr) )
          return 0xf4;
    }
    else
    {
        /* data adr transmission */
        if( I2C_Send_Byte( data_adr>>8) )
          return 0xf4;
        if( I2C_Send_Byte( data_adr) )
          return 0xf5;
	}

	/* send a start condition to I2C bus */
    if( I2C_Send_Bit(RESTARTBIT) )
      return 0xf6;
    
    /* send slave address to I2C bus */
    if( I2C_Send_Byte( slave_adr|0x01) )
      return 0xf7;
    
     
    
    while( len )
    {
       if( len == 1 )
       { 
          FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_NACK); 
       }
       else
       { 
          FL_I2C_Master_SetRespond(I2C, FL_I2C_MASTER_RESPOND_ACK); 
       }
       
       if( I2C_Receive_Byte(data) )
       {
          return 0xf9;
       }
       data++;
       len--;
    }
     
    /* send a stop condition to I2C bus */
    if( I2C_Send_Bit(STOPBIT) )
      return 0xfa;
    
    return 0x0;
}

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
uint32_t sEE_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t NumByteToRead)
{  
   uint8_t flag = 1;
   uint8_t cr = 0;
  do
  {
    flag = i2c_recived_data_bytes( sEEAddress,2,ReadAddr, pBuffer ,NumByteToRead );
    if( flag )
    {
       I2C_Device_Init();
       cr++;
       if( cr > 2 )
       {
          break;
       }
       FL_DelayUs(2000);
    }
  }while( flag );
  
  if( flag )
  {
      eeprom_status = 1;
      set_eeprom_fault();
      protect_code[6] |= 0x01;
  }
  else
  { /* If all operations OK, return sEE_OK (0) */
    eeprom_status = 0;
    protect_code[6] &= ~0x01; 
  }
  
  return flag;
}

/**
  * @brief  Writes more than one byte to the EEPROM with a single WRITE cycle.
  *
  * @note   The number of bytes (combined to write start address) must not 
  *         cross the EEPROM page boundary. This function can only write into
  *         the boundaries of an EEPROM page.
  * @note   This function doesn't check on boundaries condition (in this driver 
  *         the function sEE_WriteBuffer() which calls sEE_WritePage() is 
  *         responsible of checking on Page boundaries).
  * 
  * @param  pBuffer: pointer to the buffer containing the data to be written to 
  *         the EEPROM.
  * @param  WriteAddr: EEPROM's internal address to write to.
  * @param  NumByteToWrite: pointer to the variable holding number of bytes to 
  *         be written into the EEPROM.
  *
  * @retval sEE_OK (0) if operation is correctly performed, else return value 
  *         different from sEE_OK (0) or the timeout user callback.
  */
uint32_t sEE_WritePage( uint8_t* pBuffer, uint16_t WriteAddr, uint8_t NumByteToWrite )
{   
    uint8_t flag = 1;
    uint8_t cr = 0;
    do
    {
        flag = i2c_send_data_bytes( sEEAddress,2,WriteAddr, pBuffer ,NumByteToWrite );

        if( flag )
        {
            I2C_Device_Init();
            cr++;
            if( cr > 2 )
            {
                break;
            }
            FL_DelayUs(2000);
        }
    }while( flag );

    if( flag )
    {
    eeprom_status = 1;
    set_eeprom_fault();
    protect_code[6] |= 0x01;
    }
    else
    { /* If all operations OK, return sEE_OK (0) */
    eeprom_status = 0;
    protect_code[6] &= ~0x01; 
    }

    return flag;
}

/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  pBuffer: pointer to the buffer  containing the data to be written 
  *         to the EEPROM.
  * @param  WriteAddr: EEPROM's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the EEPROM.
  * @retval None
  */
void sEE_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t NumOfPage = 0, NumOfSingle = 0, count = 0;
    uint16_t Addr = 0;

    Addr = WriteAddr % sEE_PAGESIZE;
    count = sEE_PAGESIZE - Addr;
    NumOfPage =  NumByteToWrite / sEE_PAGESIZE;
    NumOfSingle = NumByteToWrite % sEE_PAGESIZE;

    /* If WriteAddr is sEE_PAGESIZE aligned  */
    if(Addr == 0)
    {
        /* If NumByteToWrite < sEE_PAGESIZE */
        if(NumOfPage == 0)
        {
            /* Store the number of data to be written */
            sEEDataNum = NumOfSingle;
            /* Start writing data */
            sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
        }
        /* If NumByteToWrite > sEE_PAGESIZE */
        else
        {
            while(NumOfPage--)
            {
                /* Store the number of data to be written */
                sEEDataNum = sEE_PAGESIZE;
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
                WriteAddr += sEE_PAGESIZE;
                pBuffer += sEE_PAGESIZE;
                delay_ms(5);
            }

            if(NumOfSingle!=0)
            {
                /* Store the number of data to be written */
                sEEDataNum = NumOfSingle;          
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
            }
        }
    }
    /* If WriteAddr is not sEE_PAGESIZE aligned  */
    else
    {
        /* If NumByteToWrite < sEE_PAGESIZE */
        if(NumOfPage== 0)
        {
            /* If the number of data to be written is more than the remaining space 
            in the current page: */
            if (NumByteToWrite > count)
            {
                /* Store the number of data to be written */
                sEEDataNum = count;
                /* Write the data contained in same page */
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
                /* Store the number of data to be written */
                sEEDataNum = (NumByteToWrite - count);
                /* Write the remaining data in the following page */
                sEE_WritePage((uint8_t*)(pBuffer + count), (WriteAddr + count), sEEDataNum);
            }
            else
            {
                /* Store the number of data to be written */
                sEEDataNum = NumOfSingle;
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
            }
        }
        /* If NumByteToWrite > sEE_PAGESIZE */
        else
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / sEE_PAGESIZE;
            NumOfSingle = NumByteToWrite % sEE_PAGESIZE;

            if(count != 0)
            {
                /* Store the number of data to be written */
                sEEDataNum = count;
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
                WriteAddr += count;
                pBuffer += count;
            } 

            while(NumOfPage--)
            {
                /* Store the number of data to be written */
                sEEDataNum = sEE_PAGESIZE;
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum);
                delay_ms(5);
                WriteAddr +=  sEE_PAGESIZE;
                pBuffer += sEE_PAGESIZE;
            }
            if(NumOfSingle != 0)
            {
                /* Store the number of data to be written */
                sEEDataNum = NumOfSingle;
                sEE_WritePage(pBuffer, WriteAddr, sEEDataNum); 
            }
        }
    }  
}

/*-------------------------------------------------------------------------------------*/



/*****************************************END OF FILE******************************/
