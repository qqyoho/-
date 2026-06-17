/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：iap_app.c
 * 文件功能描述：实现IAP跳转
 *
 * 修改记录：
 * 2018-08-24 戴辉发 创建
*----------------------------------------------------------*/
#include "parameter.h"
#include "string.h"
#include "iap_app.h"
#include "bsp_nor_flash.h"
#include "idog.h"
#include "hardware.h"
#include "power.h"
#include "storage_manage.h"
#include "power.h"
#include "fm33lg0xx_fl.h"
#include "fm33lg0xx.h"
#include "afe_app.h"
#include "switch_status.h"

extern const char soft_version;
typedef  void (*p_function)(void);
p_function jump_to_iap;
static uint8_t g_update_main_flag;
/*=============================================================
 * 函数名称：update_record_default
 * 函数功能：升级记录模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void update_record_default(void)
{
   uint8_t i = 0;
   for( i =0;i<24;i++) 
    last_two_update.updatetime[0].softversion[i] = 0;
   for( i =0;i<24;i++) 
    last_two_update.updatetime[1].softversion[i] = 0;
}
/*=============================================================
 * 函数名称：update_record_mem_init
 * 函数功能：均衡模块计数值初始化
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2020-6-22        liyong    创建
==============================================================*/
void update_record_mem_init(void)
{
	update_record_default();
    g_update_main_flag = 0;
}

/*=============================================================
 * 函数名称：balance_record_hard_init
 * 函数功能：均衡模块计数值初始化
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void update_record_hard_init(void)
{
	int16_t flag = 0;
	t_update_record update_record; 
    
    flag = read_data_from_storage(UPDATE_SECTOR_MAIN, (uint8_t *)&last_two_update, sizeof(t_update_record));
    if( flag == sizeof(t_update_record) )
    {
        flag = read_data_from_storage(UPDATE_SECTOR_BACK, (uint8_t *)&update_record, sizeof(t_update_record));
        if( flag == sizeof(t_update_record) )
        {
           flag = judge_new_sector(last_two_update.number, update_record.number);
        }
    }

	if (flag == -1)
	{
		g_update_main_flag = 1;
		memcpy(&last_two_update, &update_record, sizeof(t_update_record));
	}
	else if (flag == 0)
	{
		g_update_main_flag = 1;
		update_record_default();
		last_two_update.number = MAX_STORAGE_NUM;
	}
	else
	{
		g_update_main_flag = 0;
	} 
}

/*=============================================================
 * 函数名称：f_EnableReadProtection
 * 函数功能：设置读取保护
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-07-13                  liyong         创建
==============================================================*/
void f_EnableReadProtection(void)  
{  
    uint32_t Option_byte_adr = 0x1FFFFC00; 
    if( 0x0002 != FL_FLASH_GetSWDReadProtectionState(FLASH) )
    {
        uint32_t Option_byte_data;
        Option_byte_data = *(uint32_t *)Option_byte_adr;
        Option_byte_data &= 0xff00ff00;
        Option_byte_data |= 0x00aa0055;
        //*(uint32_t *)Option_byte_adr = Option_byte_data;
        //FL_FLASH_Program_String( Option_byte_adr, &Option_byte_data,1 );
    }
}
/*=============================================================
 * 函数名称：f_DisenableReadProtection
 * 函数功能：设置读取不保护
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-07-13                  liyong         创建
==============================================================*/
void f_DisenableReadProtection(void)  
{  
   //通过jflash 全擦除芯片
   //通过 SWD 对 flash 进行全空间擦写（mat erase 或者伪全擦），全擦完成后，
   //SWD 可以任意改写 OPTBYTES 禁止 DBRDP，然后复位芯片；复位完成后，芯片将处于无 debug
   //保护状态。
}

/*=============================================================
 * 函数名称：Set_readProtect
 * 函数功能：设置读取保护
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-07-13                  liyong         创建
==============================================================*/
void Set_readProtect(void)
{
#ifndef  NEGT_DEBUG
	//f_EnableReadProtection();
#endif
}

/*=============================================================
 * 函数名称：update_record_time
 * 函数功能：升级记录 最后两次
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03                  liyong         创建
==============================================================*/
void update_record_time(void)
{
	uint16_t address;
	t_write_data_st data;

	memcpy(&(last_two_update.updatetime[1].softversion), &(last_two_update.updatetime[0].softversion), 24);
	memcpy(&(last_two_update.updatetime[1].time), &(last_two_update.updatetime[0].time), 6);       
	memcpy(&(last_two_update.updatetime[0].softversion), &(soft_version), 24);
	get_timdedata( last_two_update.updatetime[0].time );

	last_two_update.number = sector_num_next(last_two_update.number);

	if (g_update_main_flag)
	{
		g_update_main_flag = 0;
		address = UPDATE_SECTOR_MAIN;
	}
	else
	{
		g_update_main_flag = 1;
		address = UPDATE_SECTOR_BACK;
	}

	data.msg_type = E_UPDATE_RECORD_MSG;
	data.write_len = sizeof(last_two_update);
	data.write_memery_address = (uint8_t *)&last_two_update;
	data.write_storge_address = address;
	set_write_data_to_queue(&data);
	storage_manage_process();
}

/*=============================================================
 * 函数名称：start_iap_process
 * 函数功能：跳转到IAP模块
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-08-24       	戴辉发     	创建
==============================================================*/
void start_iap_process(void)
{
    feed_iwdg();
    update_record_time();
    nor_flash_earse(NOR_VERSION_START, SECTOR_SIZE);
    feed_hard_owdg();
	cutoff_all_switch();
	delay_ms(50);
	bms_enter_sheep();  
	feed_iwdg();
    feed_hard_owdg();
    NVIC_SystemReset();
}