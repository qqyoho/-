/*
 * final_protect.h
 *
 *  Created on: 2019年7月3日
 *      Author: daihuifa
 */

#ifndef FINAL_PROTECT_H_
#define FINAL_PROTECT_H_

#include <stdint.h>

typedef struct _SYSTEM_PROTECT_DATA_STRUCT_
{
	uint16_t vol_high_protect;
	uint16_t vol_high_recover;
	uint16_t vol_low_protect;
	uint16_t vol_low_recover; 
    unsigned char  vol_high_status;
    unsigned char  vol_low_status;
}t_system_protect_data;

extern t_system_protect_data g_system_protect;

/*=============================================================
* 函数名称：final_protect_mem_init
* 参数功能：系统终极保护判决
* 参数个数：0
* 函数参数：
*
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                             修改人             修改内容
* 2019-07-03          戴辉发             创建
==============================================================*/
void final_protect_mem_init(void);

/*=============================================================
* 函数名称：final_protect_timer
* 参数功能：终极保护定时器
* 参数个数：0
* 函数参数：
*
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                             修改人             修改内容
* 2019-07-03          戴辉发             创建
==============================================================*/
void final_protect_timer(void);

/*=============================================================
* 函数名称：system_final_protect
* 参数功能：系统终极保护判决
* 参数个数：0
* 函数参数：
*
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                             修改人             修改内容
* 2019-01-17          戴辉发             创建
==============================================================*/
void system_final_protect(void);

#endif /* FINAL_PROTECT_H_ */
