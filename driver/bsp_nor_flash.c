/*******************************************************************************
  
  Copyright (C) 2005-2013	Neolink Software Co., Ltd. All rights reserved.
  ------------------------------------------------------------------------------
  FileName: 		bsp_nor_flash.c
  Author:   		mfchen
  Description:      stm32 nor flash
  Version:   		1.00.0
  Changes:  		
  				    1.00.0  - Initial release. 				2013-07-11
*******************************************************************************/
#include "fm33lg0xx_fl.h"
#include "bsp_nor_flash.h"
#include "idog.h"


//扇区擦函数(每个扇区512字节) 
//适用于bootloader，禁止擦除boot区
//输入参数：SectorNum   需要擦除扇区的扇区号
uint8_t Flash_Erase_Sector( uint16_t SectorNum )
{
	uint16_t i;
	uint8_t Result = 0;
	uint32_t *PFlash;
	
	if(SectorNum < (NOR_FLASH_SOFT_START/SECTOR_SIZE)) return 2;//禁止擦除bootloader区
    
	PFlash = (uint32_t *)(uint32_t)(SectorNum * SECTOR_SIZE);
	FL_FLASH_PageErase(FLASH,SectorNum * SECTOR_SIZE);
	for (i = 0; i < 128; i++ )
	{
		if (PFlash[i] != 0xFFFFFFFF ) 
		{
			Result = 1;
			break;
		}
	}
	return Result;
}
/*********************************************************************************
 * 函数名:   nor_flash_earse() 
 * 函数描述: 擦除NOR Flash
 * 参数:     start_addr : 擦除的起始地址
 *           lenght :     擦除的长度
 * 返回值:   无 
 * 修改记录：
 *		  2013-7-11 mfchen创建
*********************************************************************************/
int32_t nor_flash_earse(uint32_t start_addr, uint32_t lenght)
{
    uint32_t end_addr;
    uint8_t err = 0;
	end_addr = start_addr + lenght;

	if ((end_addr > NOR_FLASH_END) || (start_addr < NOR_FLASH_SOFT_START) || ((start_addr % SECTOR_SIZE) != 0))
	/* 地址小于正式软件起始地址 */
	/* 地址大于FLASH最大地址 */
	/* 地址不为也起始地址 */
	{
		err = 1;
	}
    else
    {
         __disable_irq();          
        while (start_addr < end_addr)
        {
            /* clear all pending flags */
            feed_hard_owdg();     
            feed_iwdg();   
            //SECTOR_SIZE
            if ( Flash_Erase_Sector(start_addr/SECTOR_SIZE) != 0 )
            {
                err = 2;
                break;
            }    
            start_addr += SECTOR_SIZE;
        }
        __enable_irq();
    }
   
	return err;
}
/*
Flash写操作

*/
uint8_t FL_FLASH_Program_String(uint32_t Address, uint32_t *data,uint32_t Length)
{
  uint32_t i;
  uint8_t ret;
	for(i=0; i<Length; i++)
	{
		ret = FL_FLASH_Program_Word(FLASH, Address + i * 4, data[i]);
		if( ret == 0 )
			return 1;
	}
  return 0;
}

/*********************************************************************************
 * 函数名:   nor_flash_earse() 
 * 函数描述: 编写NOR Flash，注意，本函数使用范围为小端模式
 * 参数:     addr :     编程的起始地址
 *           *data :    编程的数据
 *           lenght :   编程的长度
 * 返回值:   编程字节数，正常返回length 
 * 修改记录：
 *		  2013-7-11 mfchen创建
*********************************************************************************/
int32_t nor_flash_program (uint32_t addr, uint8_t *data, uint32_t length)
{
	uint32_t *pdata = (uint32_t *)data;
	uint32_t num = (length >> 2);
    uint32_t temp;
    uint8_t err = 0;
    __disable_irq();
	while ((num --) && (addr < NOR_FLASH_END))
	{
		feed_hard_owdg();     
        feed_iwdg();  
        if ( FL_FLASH_Program_String(addr,pdata,1) != 0 )
		{
			err = 1;
            break;
		}
        pdata++;
		addr += 4;
	}
    temp = 0;
    if ( 1 == length%4 )
	{
        temp = 0xffffff;
		temp = (temp << 8) + data[length - 1];
	}
    else if ( 2 == length%4 )
	{
		temp = 0xffff;
		temp = (temp << 8) + data[length - 2];
        temp = (temp << 8) + data[length - 1];
	}
    else if ( 3 == length%4 )
	{
		temp = 0xff;
		temp = (temp << 8) + data[length - 3];
        temp = (temp << 8) + data[length - 2];
        temp = (temp << 8) + data[length - 1];
	}    
    
	if (( 0 == err )&&( 0 != temp ))
	{
		 if ( FL_FLASH_Program_String(addr,pdata,1) != 0  )
		{
			err = 2;
		}  
	}
    
     __enable_irq();

    if( 0 != err ) length = 0;       
	return length;
}

/*********************************************************************************
 * 函数名:   nor_flash_earse() 
 * 函数描述: 擦除NOR Flash
 * 参数:     addr :     读取的起始地址
 *           *data :    读取的数据
 *           lenght :   读取的长度
 * 返回值:   无 
 * 修改记录：
 *		  2013-7-11 mfchen创建
*********************************************************************************/
int32_t nor_flash_read(uint32_t star_addr, uint8_t *data, uint32_t lenght)
{
	uint32_t i;

	for (i = 0; ((i < lenght) && ((star_addr + i) < NOR_FLASH_END)); i++)
	{
		*(data + i) = *(__IO uint8_t *)(star_addr + i);
	}

	return i;
}

/*=============================================================
* 函数名称：app_jump_success
* 函数功能：升级跳转成功写flash
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2020-09-7          li            创建
==============================================================*/
void  app_jump_success( void )
{
    uint32_t end;
    
    nor_flash_read(NOR_VERSION_START, (uint8_t *)&end, sizeof(end));
    
    if (end != PROGRAM_SOFT_RUN_VERSION)
    {
        if (end == 0xffffffff)
        {
            end = PROGRAM_SOFT_RUN_VERSION;
            nor_flash_program(NOR_VERSION_START, (uint8_t *)&end, sizeof(end));
        }
        else
        {   
            nor_flash_earse(NOR_VERSION_START, SECTOR_SIZE);   
            end = PROGRAM_SOFT_RUN_VERSION;
            nor_flash_program(NOR_VERSION_START, (uint8_t *)&end, sizeof(end));
        }
    }
}





