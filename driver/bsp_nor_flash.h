/*******************************************************************************
  
  Copyright (C) 2005-2013	Neolink Software Co., Ltd. All rights reserved.
  ------------------------------------------------------------------------------
  FileName: 		bsp_nor_flash.h
  Author:   		mfchen
  Description:   	stm32 nor flash
  Version:   		1.00.0
  Changes:  		
  					1.00.0  - Initial release. 				2013-07-11
              		
*******************************************************************************/
#ifndef __BSP_NOR_FLASH_H_
#define __BSP_NOR_FLASH_H_



#define PROGRAM_SOFT_RUN_VERSION          0xA55A5AA5

/* FLASH扇区定义 */
/* 片内FLASH扇区大小，512Byte */
#define SECTOR_SIZE            0x00000200

#define NOR_BOOT_LOAD_START    0x00000000

/* 正式软件扇区起始地址 */
#define NOR_FLASH_SOFT_START   0x00005000
#define NOR_FLASH_SOFT_END     0x0001FBFF
/* 正式软件结束地址，流下1K地址写软件版本以及更新信息 */

/* 软件版本信息地址 */
#define NOR_VERSION_START      0x0001FC00
/* 字节0和字节1: 写入开始编程信号，0x5A5A */
/* 字节2和字节3: 写入编程完成信号，0x5AA5 */
/* 字节4和字节5: 运行软件正常信号，0xA55A */

/* FLASH最大地址 */
#define NOR_FLASH_END          0x00020000

/*********************************************************************************
* 函数名:   nor_flash_earse() 
* 函数描述: 擦除NOR Flash
* 参数:     start_addr : 擦除的起始地址
*           lenght :     擦除的长度
* 返回值:   无 
* 修改记录：
*		  2013-7-11 mfchen创建
*********************************************************************************/
int32_t nor_flash_earse (uint32_t start_addr, uint32_t lenght);

/*********************************************************************************
* 函数名:   nor_flash_earse() 
* 函数描述: 编写NOR Flash
* 参数:     addr :     编程的起始地址
*           *data :    编程的数据
*           lenght :   编程的长度
* 返回值:   无 
* 修改记录：
*		  2013-7-11 mfchen创建
*********************************************************************************/
int32_t nor_flash_program (uint32_t addr, uint8_t *data, uint32_t lenght);

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
int32_t nor_flash_read (uint32_t star_addr, uint8_t *data, uint32_t lenght);
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
void  app_jump_success( void );
#endif

