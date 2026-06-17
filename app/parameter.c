/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：parameter.c
 * 文件功能描述：实现系统参数管理
 *
 * 修改记录：
 * 2018-06-22 戴辉发   创建
*----------------------------------------------------------*/

#include "parameter.h"
#include "storage_manage.h"
#include <string.h>
#include "vol_manage.h"
#include "soc_update.h"
#include "soc.h"
#include "afe_app.h"

#define RVSD_BUFFER_LENGTH            (4)

t_vol_para_st g_vol_para; /* 电压参数 */
static uint8_t g_vol_main_flag;
static uint8_t g_vol_end_id;

t_curr_para_st g_curr_para;
static uint8_t g_curr_main_flag;
static uint8_t g_curr_end_id;

t_temp_para_st g_temp_para;
static uint8_t g_temp_main_flag;
static uint8_t g_temp_end_id;

t_hot_balance_para_st g_hot_balance_para;
static uint8_t g_hot_balance_main_flag;
static uint8_t g_hot_balance_end_id;

t_capcity_para_st g_capcity_para;
static uint8_t g_capcity_main_flag;
static uint8_t g_capcity_end_id;

t_other_para_st g_other_para;
static uint8_t g_other_main_flag;
static uint8_t g_other_end_id;
t_system_data g_run_sys_data;

uint8_t protect_code[8];

/*=============================================================
* 函数名称：system_parameter_mem_init
* 函数功能：系统参数内存初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void system_parameter_mem_init(void)
{
  uint8_t i;
  for( i=0;i<8;i++)
   protect_code[i] = 0;
}

/*==============================================================
* 函数名称：vol_para_data_default
* 函数功能：电压参数恢复默认值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17          戴辉发             创建
==============================================================*/
static void vol_para_data_default(void)
{
#if defined (FUDEER) && defined (BAT_8S)
    /* 福得尔8串 */
    /* 电芯电压告警参数, 0.001V为单位 */
    g_vol_para.vol_para.cell_vol.high.alarm = 3600;
    g_vol_para.vol_para.cell_vol.high.alarm_recover = 3500;
    g_vol_para.vol_para.cell_vol.high.protect = 3650;
    g_vol_para.vol_para.cell_vol.high.recover = 3350;
    g_vol_para.vol_para.cell_vol.low.alarm = 2600;
    g_vol_para.vol_para.cell_vol.low.alarm_recover = 2800;
    g_vol_para.vol_para.cell_vol.low.protect = 2500;
    g_vol_para.vol_para.cell_vol.low.recover = 3000;

    /* 总压告警参数, 0.1V为单位 */
    g_vol_para.vol_para.voltage.high.alarm = 2880;
    g_vol_para.vol_para.voltage.high.alarm_recover = 2720;
    g_vol_para.vol_para.voltage.high.protect = 2920;
    g_vol_para.vol_para.voltage.high.recover = 2680;
    g_vol_para.vol_para.voltage.low.alarm = 2080;
    g_vol_para.vol_para.voltage.low.alarm_recover = 2250;
    g_vol_para.vol_para.voltage.low.protect = 1980;
    g_vol_para.vol_para.voltage.low.recover = 2450;

    /* 电压保护延时 */
    g_vol_para.pretect_delay = 3;
    /* 单体低压禁充门限 */
    g_vol_para.dis_charge_level = 2000;
    /* 电压功能开关 */
    g_vol_para.voltage_flag = 0xFF;
    
#elif defined (YUHENG) && defined (BAT_8S)
    /* 电芯电压告警参数, 0.001V为单位 */
    g_vol_para.vol_para.cell_vol.high.alarm = 3600;
    g_vol_para.vol_para.cell_vol.high.alarm_recover = 3400;
    g_vol_para.vol_para.cell_vol.high.protect = 3650; 
    g_vol_para.vol_para.cell_vol.high.recover = 3400; /* 修改参数 */
    g_vol_para.vol_para.cell_vol.low.alarm = 2600;
    g_vol_para.vol_para.cell_vol.low.alarm_recover = 2900;
    g_vol_para.vol_para.cell_vol.low.protect = 2400;    /* 修改参数 */
    g_vol_para.vol_para.cell_vol.low.recover = 2900;    /* 修改参数 */

    /* 总压告警参数, 0.1V为单位 */
    g_vol_para.vol_para.voltage.high.alarm = 2880;
    g_vol_para.vol_para.voltage.high.alarm_recover = 2720;
    g_vol_para.vol_para.voltage.high.protect = 2920;
    g_vol_para.vol_para.voltage.high.recover = 2720;
    g_vol_para.vol_para.voltage.low.alarm = 2080;
    g_vol_para.vol_para.voltage.low.alarm_recover = 2320;
    g_vol_para.vol_para.voltage.low.protect = 1920;
    g_vol_para.vol_para.voltage.low.recover = 2320;

    /* 电压保护延时 */
    g_vol_para.pretect_delay = 5;       /* 修改参数 */
    /* 单体低压禁充门限 */
    g_vol_para.dis_charge_level = 1500; /* 修改参数 */
    /* 电压功能开关 */
    g_vol_para.voltage_flag = 0xFF;
       
#elif defined(TIANFENG) && defined(BAT_8S)
    /* 电芯电压告警参数, 0.001V为单位 */
    g_vol_para.vol_para.cell_vol.high.alarm = 3500;
    g_vol_para.vol_para.cell_vol.high.alarm_recover = 3400;
    g_vol_para.vol_para.cell_vol.high.protect = 3650; 
    g_vol_para.vol_para.cell_vol.high.recover = 3300; /* 修改参数 */
    g_vol_para.vol_para.cell_vol.low.alarm = 2600;
    g_vol_para.vol_para.cell_vol.low.alarm_recover = 3000;
    g_vol_para.vol_para.cell_vol.low.protect = 2400;    /* 修改参数 */
    g_vol_para.vol_para.cell_vol.low.recover = 2900;    /* 修改参数 */

    /* 总压告警参数, 0.1V为单位 */
    g_vol_para.vol_para.voltage.high.alarm = 2920;
    g_vol_para.vol_para.voltage.high.alarm_recover = 2920;
    g_vol_para.vol_para.voltage.high.protect = 2920;
    g_vol_para.vol_para.voltage.high.recover = 2640;
    g_vol_para.vol_para.voltage.low.alarm = 2080;
    g_vol_para.vol_para.voltage.low.alarm_recover = 2400;
    g_vol_para.vol_para.voltage.low.protect = 1920;
    g_vol_para.vol_para.voltage.low.recover = 2320;

    /* 电压保护延时 */
    g_vol_para.pretect_delay = 5;       /* 修改参数 */
    /* 单体低压禁充门限 */
    g_vol_para.dis_charge_level = 1500; /* 修改参数 */
    /* 电压功能开关 */
    g_vol_para.voltage_flag = 0xFF;
    
#elif defined(TIANHONG) && defined(BAT_8S)
    /* 天宏加力8串 */
    /* 电芯电压告警参数, 0.001V为单位 */
    g_vol_para.vol_para.cell_vol.high.alarm = 3600;
    g_vol_para.vol_para.cell_vol.high.alarm_recover = 3500;
    g_vol_para.vol_para.cell_vol.high.protect = 3750;
    g_vol_para.vol_para.cell_vol.high.recover = 3350;
    g_vol_para.vol_para.cell_vol.low.alarm = 2600;
    g_vol_para.vol_para.cell_vol.low.alarm_recover = 2800;
    g_vol_para.vol_para.cell_vol.low.protect = 2300;
    g_vol_para.vol_para.cell_vol.low.recover = 3000;

    /* 总压告警参数, 0.1V为单位 */
    g_vol_para.vol_para.voltage.high.alarm = 2880;
    g_vol_para.vol_para.voltage.high.alarm_recover = 2720;
    g_vol_para.vol_para.voltage.high.protect = 2920;
    g_vol_para.vol_para.voltage.high.recover = 2680;
    g_vol_para.vol_para.voltage.low.alarm = 2080;
    g_vol_para.vol_para.voltage.low.alarm_recover = 2250;
    g_vol_para.vol_para.voltage.low.protect = 1980;
    g_vol_para.vol_para.voltage.low.recover = 2450;

    /* 电压保护延时 */
    g_vol_para.pretect_delay = 3;
    /* 单体低压禁充门限 */
    g_vol_para.dis_charge_level = 2000;
    /* 电压功能开关 */
    g_vol_para.voltage_flag = 0xFF;
#endif
}

/*==============================================================
* 函数名称：vol_para_data_hard_init
* 函数功能：电压参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void vol_para_data_hard_init(void)
{
	short flag = 0;
	t_vol_para_st vol_data;

	flag = read_data_from_storage(VOL_PARA_SECTOR_MAIN, (uint8_t *)&g_vol_para, sizeof(g_vol_para));
    if( flag ==  sizeof(g_vol_para) )
    {
       flag = read_data_from_storage(VOL_PARA_SECTOR_BACK, (uint8_t *)&vol_data, sizeof(vol_data));
       if( flag ==  sizeof(g_vol_para) )
       {
          flag = judge_new_sector(g_vol_para.end_id, vol_data.end_id);
       }
    }
   
	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_vol_main_flag = 1;
		g_vol_end_id = vol_data.end_id;
		memcpy(&g_vol_para, &vol_data, sizeof(vol_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
        g_vol_main_flag = 1;
        g_vol_end_id = MAX_STORAGE_NUM;

        vol_para_data_default();
	}
	else
	/* 主扇区数据最新 */
	{
		g_vol_main_flag = 0;
		g_vol_end_id = g_vol_para.end_id;
	}

}

/*==============================================================
* 函数名称：save_vol_para_parameter
* 函数功能：保存电压参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_vol_para_parameter(void)
{
	t_write_data_st data;

	g_vol_end_id = sector_num_next(g_vol_end_id);

	g_vol_para.end_id = g_vol_end_id;
	
	if (g_vol_main_flag)
    {
		g_vol_main_flag = 0;
		data.write_storge_address = VOL_PARA_SECTOR_MAIN;
	}
	else
    {
		g_vol_main_flag = 1;
		data.write_storge_address = VOL_PARA_SECTOR_BACK;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_vol_para);
	data.write_memery_address = (uint8_t *)&g_vol_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
* 函数名称：curr_para_data_default
* 函数功能：电流参数恢复默认值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17          戴辉发             创建
==============================================================*/
static void curr_para_data_default(void)
{
#if defined(FUDEER) && defined(BAT_8S)
 /* 充电电流参数, A */
    g_curr_para.ch_current.in_level = 6;
    g_curr_para.ch_current.alarm = 550;
    g_curr_para.ch_current.protect = 600;
    g_curr_para.ch_current.limit = 700;
    g_curr_para.ch_current.protect_delay = 5;

    /* 放电电流参数, A */
    g_curr_para.dch_current.in_level = -5;
    g_curr_para.dch_current.alarm = -1800;
    g_curr_para.dch_current.protect = -1900;
    g_curr_para.dch_current.protect_delay = 5;
    g_curr_para.dch_current.second_protect = -2200;
    g_curr_para.dch_current.second_delay = 500;
    g_curr_para.dch_current.locked_num = 5;
    g_curr_para.dch_current.short_protect = -8000;
    g_curr_para.dch_current.short_delay = 200;
    g_curr_para.dch_current.short_recover_delay = 1;
    g_curr_para.dch_current.protect_recover_delay = 20;
    /* 电流功能开关 */
    g_curr_para.current_flag = 0xFF;
    
#elif defined(YUHENG) && defined(BAT_8S)
    /* 充电电流参数, 0.1A */
    g_curr_para.ch_current.in_level = 5;
    g_curr_para.ch_current.alarm = 550;
    g_curr_para.ch_current.protect = 600;           /* 修改参数 */
    g_curr_para.ch_current.limit = 700;
    g_curr_para.ch_current.protect_delay = 3;       /* 修改参数 */

    /* 放电电流参数, 0.1A */
    g_curr_para.dch_current.in_level = -5;
    g_curr_para.dch_current.alarm = -1800;
    g_curr_para.dch_current.protect = -2000;        /* 修改参数 */
    g_curr_para.dch_current.protect_delay = 2;      /* 修改参数 */
    g_curr_para.dch_current.second_protect = -2500; /* 修改参数 */
    g_curr_para.dch_current.second_delay = 40;      /* 修改参数 */
    g_curr_para.dch_current.locked_num = 5;
    g_curr_para.dch_current.short_protect = -8000;
    g_curr_para.dch_current.short_delay = 200;
    g_curr_para.dch_current.short_recover_delay = 1;
    g_curr_para.dch_current.protect_recover_delay = 20;
    /* 电流功能开关 */
    g_curr_para.current_flag = 0xFF;
#elif defined(TIANFENG) && defined(BAT_8S)
    /* 充电电流参数, 0.1A */
    g_curr_para.ch_current.in_level = 5;
    g_curr_para.ch_current.alarm = 350;
    g_curr_para.ch_current.protect = 600;           /* 修改参数 */
    g_curr_para.ch_current.limit = 700;
    g_curr_para.ch_current.protect_delay = 3;       /* 修改参数 */

    /* 放电电流参数, 0.1A */
    g_curr_para.dch_current.in_level = -5;
    g_curr_para.dch_current.alarm = -2300;
    g_curr_para.dch_current.protect = -2500;        /* 修改参数 */
    g_curr_para.dch_current.protect_delay = 10;      /* 修改参数 */
    g_curr_para.dch_current.second_protect = -7000; /* 修改参数 */
    g_curr_para.dch_current.second_delay = 1000;      /* 修改参数 */
    g_curr_para.dch_current.locked_num = 5;
    g_curr_para.dch_current.short_protect = -8000;
    g_curr_para.dch_current.short_delay = 200;
    g_curr_para.dch_current.short_recover_delay = 1;
    g_curr_para.dch_current.protect_recover_delay = 20;
    /* 电流功能开关 */
    g_curr_para.current_flag = 0xFF;
    
#elif defined(TIANHONG) && defined(BAT_8S)
     /* 充电电流参数, A */
    g_curr_para.ch_current.in_level = 6;
    g_curr_para.ch_current.alarm = 550;
    g_curr_para.ch_current.protect = 600;
    g_curr_para.ch_current.limit = 700;
    g_curr_para.ch_current.protect_delay = 5;

    /* 放电电流参数, A */
    g_curr_para.dch_current.in_level = -5;
    g_curr_para.dch_current.alarm = -1800;
    g_curr_para.dch_current.protect = -1900;
    g_curr_para.dch_current.protect_delay = 5;
    g_curr_para.dch_current.second_protect = -2200;
    g_curr_para.dch_current.second_delay = 500;
    g_curr_para.dch_current.locked_num = 5;
    g_curr_para.dch_current.short_protect = -10400;
    g_curr_para.dch_current.short_delay = 100;
    g_curr_para.dch_current.short_recover_delay = 1;
    g_curr_para.dch_current.protect_recover_delay = 20;
    /* 电流功能开关 */
    g_curr_para.current_flag = 0xFF;
#endif
}

/*==============================================================
* 函数名称：curr_para_data_hard_init
* 函数功能：电流参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void curr_para_data_hard_init(void)
{
	short flag = 0;
	t_curr_para_st curr_data;

	flag = read_data_from_storage(CURR_PARA_SECTOR_MAIN, (uint8_t *)&g_curr_para, sizeof(g_curr_para));
    if( flag == sizeof(g_curr_para) )
    {
       flag = read_data_from_storage(CURR_PARA_SECTOR_BACK, (uint8_t *)&curr_data, sizeof(curr_data));
       if( flag == sizeof(g_curr_para) )
         flag = judge_new_sector(g_curr_para.end_id, curr_data.end_id);
    }
	
	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_curr_main_flag = 1;
		g_curr_end_id = curr_data.end_id;
		memcpy(&g_curr_para, &curr_data, sizeof(curr_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
		g_curr_main_flag = 1;
		g_curr_end_id = MAX_STORAGE_NUM;

        curr_para_data_default();
	}
	else
	/* 主扇区数据最新 */
	{
		g_curr_main_flag = 0;
		g_curr_end_id = g_curr_para.end_id;
	}
}

/*==============================================================
* 函数名称：save_curr_para_parameter
* 函数功能：保存电流参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_curr_para_parameter(void)
{
	t_write_data_st data;

	g_curr_end_id = sector_num_next(g_curr_end_id);

	g_curr_para.end_id = g_curr_end_id;

	if (g_curr_main_flag)
    {
		g_curr_main_flag = 0;
		data.write_storge_address = CURR_PARA_SECTOR_MAIN;
	}
	else
    {
		g_curr_main_flag = 1;
		data.write_storge_address = CURR_PARA_SECTOR_BACK;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_curr_para);
	data.write_memery_address = (uint8_t *)&g_curr_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
* 函数名称：temp_para_data_default
* 函数功能：温度参数恢复默认值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17          戴辉发             创建
==============================================================*/
static void temp_para_data_default(void)
{
#if defined(FUDEER) && defined(BAT_8S)
    /* 电芯温度告警参数, 0.1℃为单位 */
    g_temp_para.cell_temp.ch_temp.high.alarm = 550;
    g_temp_para.cell_temp.ch_temp.high.protect = 600;
    g_temp_para.cell_temp.ch_temp.high.recover = 550;
    g_temp_para.cell_temp.ch_temp.low.alarm = 100;
    g_temp_para.cell_temp.ch_temp.low.protect = 15;
    g_temp_para.cell_temp.ch_temp.low.recover = 25;

    g_temp_para.cell_temp.dch_temp.high.alarm = 550;
    g_temp_para.cell_temp.dch_temp.high.protect = 650;
    g_temp_para.cell_temp.dch_temp.high.recover = 550;
    g_temp_para.cell_temp.dch_temp.low.alarm = -150;
    g_temp_para.cell_temp.dch_temp.low.protect = -200;
    g_temp_para.cell_temp.dch_temp.low.recover = -150;

    g_temp_para.cell_temp.protect_delay = 3;

    /* 功率温度告警参数 */
    g_temp_para.power_temp.temp.high.alarm = 950;
    g_temp_para.power_temp.temp.high.protect = 1050;
    g_temp_para.power_temp.temp.high.recover = 850;
    g_temp_para.power_temp.temp.low.alarm = -300;
    g_temp_para.power_temp.temp.low.protect = -380;
    g_temp_para.power_temp.temp.low.recover = -300;

    g_temp_para.power_temp.protect_delay = 3;

    /* 环境温度告警参数 */
    g_temp_para.enviror_temp.temp.high.alarm = 600;
    g_temp_para.enviror_temp.temp.high.protect = 750;
    g_temp_para.enviror_temp.temp.high.recover = 650;
    g_temp_para.enviror_temp.temp.low.alarm = -300;
    g_temp_para.enviror_temp.temp.low.protect = -380;
    g_temp_para.enviror_temp.temp.low.recover = -300;

    g_temp_para.enviror_temp.protect_delay = 3;

    /* 温度功能开关 */
    g_temp_para.temp_flag = 0xFB;
    
#elif defined(YUHENG) && defined(BAT_8S)
    /* 电芯温度告警参数, 0.1℃为单位 */
    g_temp_para.cell_temp.ch_temp.high.alarm = 450;    /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.high.protect = 550;  /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.high.recover = 450;   /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.low.alarm = 50;
    g_temp_para.cell_temp.ch_temp.low.protect = 0;      /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.low.recover = 20;     /* 修改参数 */

    g_temp_para.cell_temp.dch_temp.high.alarm = 550;
    g_temp_para.cell_temp.dch_temp.high.protect = 600;
    g_temp_para.cell_temp.dch_temp.high.recover = 550;
    g_temp_para.cell_temp.dch_temp.low.alarm = -100;    /* 修改参数 */
    g_temp_para.cell_temp.dch_temp.low.protect = -200;  /* 修改参数 */
    g_temp_para.cell_temp.dch_temp.low.recover = -180;  /* 修改参数 */

    g_temp_para.cell_temp.protect_delay = 3;

    /* 功率温度告警参数 */
    g_temp_para.power_temp.temp.high.alarm = 750;       /* 修改参数 */
    g_temp_para.power_temp.temp.high.protect = 900;     /* 修改参数 */
    g_temp_para.power_temp.temp.high.recover = 650;     /* 修改参数 */
    g_temp_para.power_temp.temp.low.alarm = -300;
    g_temp_para.power_temp.temp.low.protect = -380;
    g_temp_para.power_temp.temp.low.recover = -300;

    g_temp_para.power_temp.protect_delay = 3;

    /* 环境温度告警参数 */
    g_temp_para.enviror_temp.temp.high.alarm = 600;
    g_temp_para.enviror_temp.temp.high.protect = 750;
    g_temp_para.enviror_temp.temp.high.recover = 650;
    g_temp_para.enviror_temp.temp.low.alarm = -300;
    g_temp_para.enviror_temp.temp.low.protect = -380;
    g_temp_para.enviror_temp.temp.low.recover = -300;

    g_temp_para.enviror_temp.protect_delay = 3;

    /* 温度功能开关 */
    g_temp_para.temp_flag = 0xFB;
    
#elif defined(TIANFENG) && defined(BAT_8S)
    /* 电芯温度告警参数, 0.1℃为单位 */
    g_temp_para.cell_temp.ch_temp.high.alarm = 450;    /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.high.protect = 550;  /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.high.recover = 450;   /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.low.alarm = 50;
    g_temp_para.cell_temp.ch_temp.low.protect = 0;      /* 修改参数 */
    g_temp_para.cell_temp.ch_temp.low.recover = 50;     /* 修改参数 */

    g_temp_para.cell_temp.dch_temp.high.alarm = 550;
    g_temp_para.cell_temp.dch_temp.high.protect = 600;
    g_temp_para.cell_temp.dch_temp.high.recover = 550;
    g_temp_para.cell_temp.dch_temp.low.alarm = -150;    /* 修改参数 */
    g_temp_para.cell_temp.dch_temp.low.protect = -200;  /* 修改参数 */
    g_temp_para.cell_temp.dch_temp.low.recover = -150;  /* 修改参数 */

    g_temp_para.cell_temp.protect_delay = 3;

    /* 功率温度告警参数 */
    g_temp_para.power_temp.temp.high.alarm = 850;       /* 修改参数 */
    g_temp_para.power_temp.temp.high.protect = 1000;     /* 修改参数 */
    g_temp_para.power_temp.temp.high.recover = 750;     /* 修改参数 */
    g_temp_para.power_temp.temp.low.alarm = -400;
    g_temp_para.power_temp.temp.low.protect = -600;
    g_temp_para.power_temp.temp.low.recover = -550;

    g_temp_para.power_temp.protect_delay = 3;

    /* 环境温度告警参数 */
    g_temp_para.enviror_temp.temp.high.alarm = 850;
    g_temp_para.enviror_temp.temp.high.protect = 1000;
    g_temp_para.enviror_temp.temp.high.recover = 750;
    g_temp_para.enviror_temp.temp.low.alarm = -400;
    g_temp_para.enviror_temp.temp.low.protect = -600;
    g_temp_para.enviror_temp.temp.low.recover = -550;

    g_temp_para.enviror_temp.protect_delay = 3;

    /* 温度功能开关 */
    g_temp_para.temp_flag = 0xFB;
    
#elif defined(TIANHONG) && defined(BAT_8S)
    /* 电芯温度告警参数, 0.1℃为单位 */
    g_temp_para.cell_temp.ch_temp.high.alarm = 550;
    g_temp_para.cell_temp.ch_temp.high.protect = 600;
    g_temp_para.cell_temp.ch_temp.high.recover = 550;
    g_temp_para.cell_temp.ch_temp.low.alarm = 100;
    g_temp_para.cell_temp.ch_temp.low.protect = -50;
    g_temp_para.cell_temp.ch_temp.low.recover = 25;

    g_temp_para.cell_temp.dch_temp.high.alarm = 550;
    g_temp_para.cell_temp.dch_temp.high.protect = 650;
    g_temp_para.cell_temp.dch_temp.high.recover = 550;
    g_temp_para.cell_temp.dch_temp.low.alarm = -150;
    g_temp_para.cell_temp.dch_temp.low.protect = -200;
    g_temp_para.cell_temp.dch_temp.low.recover = -150;

    g_temp_para.cell_temp.protect_delay = 3;

    /* 功率温度告警参数 */
    g_temp_para.power_temp.temp.high.alarm = 950;
    g_temp_para.power_temp.temp.high.protect = 1050;
    g_temp_para.power_temp.temp.high.recover = 850;
    g_temp_para.power_temp.temp.low.alarm = -300;
    g_temp_para.power_temp.temp.low.protect = -380;
    g_temp_para.power_temp.temp.low.recover = -300;

    g_temp_para.power_temp.protect_delay = 3;

    /* 环境温度告警参数 */
    g_temp_para.enviror_temp.temp.high.alarm = 600;
    g_temp_para.enviror_temp.temp.high.protect = 750;
    g_temp_para.enviror_temp.temp.high.recover = 650;
    g_temp_para.enviror_temp.temp.low.alarm = -300;
    g_temp_para.enviror_temp.temp.low.protect = -380;
    g_temp_para.enviror_temp.temp.low.recover = -300;

    g_temp_para.enviror_temp.protect_delay = 3;

    /* 温度功能开关 */
    g_temp_para.temp_flag = 0xFB;
#endif
}

/*==============================================================
* 函数名称：temp_para_data_hard_init
* 函数功能：温度参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void temp_para_data_hard_init(void)
{
	short flag = 0;
	t_temp_para_st temp_data;

	flag = read_data_from_storage(TEMP_PARA_SECTOR_MAIN, (uint8_t *)&g_temp_para, sizeof(g_temp_para));
    if( flag == sizeof(g_temp_para) )
    {
       flag = read_data_from_storage(TEMP_PARA_SECTOR_BACK, (uint8_t *)&temp_data, sizeof(temp_data));
       if( flag == sizeof(g_temp_para) )
       {
          flag = judge_new_sector(g_temp_para.end_id, temp_data.end_id);
       }
    }
	
	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_temp_main_flag = 1;
		g_temp_end_id = temp_data.end_id;
		memcpy(&g_temp_para, &temp_data, sizeof(temp_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
		g_temp_main_flag = 1;
		g_temp_end_id = MAX_STORAGE_NUM;

        temp_para_data_default();
	}
	else
	/* 主扇区数据最新 */
	{
		g_temp_main_flag = 0;
		g_temp_end_id = g_temp_para.end_id;
	}
}

/*==============================================================
* 函数名称：save_temp_para_parameter
* 函数功能：保存温度参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_temp_para_parameter(void)
{
	t_write_data_st data;

	g_temp_end_id = sector_num_next(g_temp_end_id);

	g_temp_para.end_id = g_temp_end_id;

	if (g_temp_main_flag)
    {
		g_temp_main_flag = 0;
		data.write_storge_address = TEMP_PARA_SECTOR_MAIN;
	}
	else
    {
		g_temp_main_flag = 1;
		data.write_storge_address = TEMP_PARA_SECTOR_BACK;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_temp_para);
	data.write_memery_address = (uint8_t *)&g_temp_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
 * 函数名称：balance_para_data_default
 * 函数功能：均衡加热参数恢复默认值
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-11-17          戴辉发             创建
==============================================================*/
static void balance_para_data_default(void)
{
#if defined(FUDEER)
   /* 加热温度参数 */
    g_hot_balance_para.hot.start = 50;
    g_hot_balance_para.hot.end = 150;

    /* 均衡模块参数 */
    g_hot_balance_para.balance.start_temp = 0;
    g_hot_balance_para.balance.end_temp = 550;
    
    g_hot_balance_para.balance.diff_start = 50;
    g_hot_balance_para.balance.diff_end = 30;
    g_hot_balance_para.balance.start = 3500;
    g_hot_balance_para.balance.delay = 1;
    g_hot_balance_para.balance.fail_level = 800;
    g_hot_balance_para.balance.recover_level = 500;
    /* 均衡功能开关 */
    g_hot_balance_para.balance_flag = 0x11;
#elif defined(YUHENG)
    /* 加热温度参数 */
    g_hot_balance_para.hot.start = 50;
    g_hot_balance_para.hot.end = 150;

    /* 均衡模块参数 */
    g_hot_balance_para.balance.start_temp = 0;
    g_hot_balance_para.balance.end_temp = 500;
    g_hot_balance_para.balance.diff_start = 50;
    g_hot_balance_para.balance.diff_end = 30;
    g_hot_balance_para.balance.start = 3450;
    g_hot_balance_para.balance.delay = 1;
    g_hot_balance_para.balance.fail_level = 800;
    g_hot_balance_para.balance.recover_level = 500;
    /* 均衡功能开关 */
    g_hot_balance_para.balance_flag = 0x11;
    
#elif defined(TIANFENG)
    /* 加热温度参数 */
    g_hot_balance_para.hot.start = 0;
    g_hot_balance_para.hot.end = 50;

    /* 均衡模块参数 */
    g_hot_balance_para.balance.start_temp = 0;
    g_hot_balance_para.balance.end_temp = 500;
    g_hot_balance_para.balance.diff_start = 50;
    g_hot_balance_para.balance.diff_end = 30;
    g_hot_balance_para.balance.start = 3450;
    g_hot_balance_para.balance.delay = 1;
    g_hot_balance_para.balance.fail_level = 800;
    g_hot_balance_para.balance.recover_level = 500;
    /* 均衡功能开关 */
    g_hot_balance_para.balance_flag = 0x11;
    
#elif defined(TIANHONG)
    /* 加热温度参数 */
    g_hot_balance_para.hot.start = 50;
    g_hot_balance_para.hot.end = 150;

    /* 均衡模块参数 */
    g_hot_balance_para.balance.start_temp = 0;
    g_hot_balance_para.balance.end_temp = 550;
    
    g_hot_balance_para.balance.diff_start = 50;
    g_hot_balance_para.balance.diff_end = 30;
    g_hot_balance_para.balance.start = 3400;
    g_hot_balance_para.balance.delay = 1;
    g_hot_balance_para.balance.fail_level = 800;
    g_hot_balance_para.balance.recover_level = 500;
    /* 均衡功能开关 */
    g_hot_balance_para.balance_flag = 0x11;
    
#elif
    /* 加热温度参数 */
    g_hot_balance_para.hot.start = 50;
    g_hot_balance_para.hot.end = 150;

    /* 均衡模块参数 */
    g_hot_balance_para.balance.start_temp = 0;
    g_hot_balance_para.balance.end_temp = 500;
    g_hot_balance_para.balance.diff_start = 50;
    g_hot_balance_para.balance.diff_end = 30;
    g_hot_balance_para.balance.start = 3450;
    g_hot_balance_para.balance.delay = 1;
    g_hot_balance_para.balance.fail_level = 800;
    g_hot_balance_para.balance.recover_level = 500;
    /* 均衡功能开关 */
    g_hot_balance_para.balance_flag = 0x11;
#endif
}

/*==============================================================
* 函数名称：balance_para_data_hard_init
* 函数功能：加热与均衡参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void balance_para_data_hard_init(void)
{
	short flag = 0;
	t_hot_balance_para_st balance_data;

	flag =  read_data_from_storage(BALANCE_PARA_SECTOR_MAIN, (uint8_t *)&g_hot_balance_para, sizeof(g_hot_balance_para));
    if(flag == sizeof(g_hot_balance_para))
    {
       flag = read_data_from_storage(BALANCE_PARA_SECTOR_BACK, (uint8_t *)&balance_data, sizeof(balance_data));
       if(flag == sizeof(g_hot_balance_para))
       {
          flag = judge_new_sector(g_hot_balance_para.end_id, balance_data.end_id);
       }
    }
	
	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_hot_balance_main_flag = 1;
		g_hot_balance_end_id = balance_data.end_id;
		memcpy(&g_hot_balance_para, &balance_data, sizeof(balance_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
		g_hot_balance_main_flag = 1;
		g_hot_balance_end_id = MAX_STORAGE_NUM;

        balance_para_data_default();
	}
	else
	/* 主扇区数据最新 */
	{
		g_hot_balance_main_flag = 0;
		g_hot_balance_end_id = g_hot_balance_para.end_id;
	}
}

/*==============================================================
* 函数名称：save_balance_para_parameter
* 函数功能：保存均衡参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_balance_para_parameter(void)
{
	t_write_data_st data;

	g_hot_balance_end_id = sector_num_next(g_hot_balance_end_id);

	g_hot_balance_para.end_id = g_hot_balance_end_id;
	
	if (g_hot_balance_main_flag){
		g_hot_balance_main_flag = 0;
		data.write_storge_address = BALANCE_PARA_SECTOR_MAIN;
	}
	else{
		g_hot_balance_main_flag = 1;
		data.write_storge_address = BALANCE_PARA_SECTOR_BACK;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_hot_balance_para);
	data.write_memery_address = (uint8_t *)&g_hot_balance_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
* 函数名称：capcity_para_data_default
* 函数功能：容量参数恢复默认值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17          戴辉发             创建
==============================================================*/
static void capcity_para_data_default(void)
{

  /* 智能补电参数 */
    g_capcity_para.smart.delay = 24;
    g_capcity_para.smart.soc_level = 90;

    g_capcity_para.capcity.low = 10;
    g_capcity_para.capcity.protect = 5;
    g_capcity_para.capcity.high = 90;

    g_capcity_para.rated_capcity = RATE_CAPCITY;
    g_capcity_para.low_power_delay = 30;
    g_capcity_para.deactive_delay = DETIVE_TIME;
    g_capcity_para.low_deactive_delay = 2;

    /* 容量功能开关 */
    g_capcity_para.capcity_flag = 0x03;

}

/*==============================================================
* 函数名称：capcity_para_data_hard_init
* 函数功能：容量参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void capcity_para_data_hard_init(void)
{
	short flag = 0;
	t_capcity_para_st capcity_data;

	flag = read_data_from_storage(CAPCITY_PARA_SECTOR_MAIN, (uint8_t *)&g_capcity_para, sizeof(g_capcity_para));
    if( flag == sizeof(g_capcity_para) )
    {
        flag = read_data_from_storage(CAPCITY_PARA_SECTOR_BACK, (uint8_t *)&capcity_data, sizeof(capcity_data));
        if( flag == sizeof(g_capcity_para) )
        {
           flag = judge_new_sector(g_capcity_para.end_id, capcity_data.end_id);
        }
    }
	

	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_capcity_main_flag = 1;
		g_capcity_end_id = capcity_data.end_id;
		memcpy(&g_capcity_para, &capcity_data, sizeof(capcity_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
		g_capcity_main_flag = 1;
		g_capcity_end_id = MAX_STORAGE_NUM;

        capcity_para_data_default();
	}
	else
	/* 主扇区数据最新 */
	{
		g_capcity_main_flag = 0;
		g_capcity_end_id = g_capcity_para.end_id;
	}
}

/*==============================================================
* 函数名称：save_capcity_para_parameter
* 函数功能：保存容量参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_capcity_para_parameter(void)
{
	t_write_data_st data;

	g_capcity_end_id = sector_num_next(g_capcity_end_id);

	g_capcity_para.end_id = g_capcity_end_id;
	
	if (g_capcity_main_flag){
		g_capcity_main_flag = 0;
		data.write_storge_address = CAPCITY_PARA_SECTOR_MAIN;
	}
	else{
		g_capcity_main_flag = 1;
		data.write_storge_address = CAPCITY_PARA_SECTOR_BACK;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_capcity_para);
	data.write_memery_address = (uint8_t *)&g_capcity_para;

	set_write_data_to_queue(&data);

	/*额定容量*/
    g_dch_circle.total = g_capcity_para.rated_capcity;
    g_dch_circle.total *= AH_VALUE; 
    write_capcity_para();

	/* 最小值 */
	update_cur_soc();
}

/*==============================================================
* 函数名称：other_para_data_default
* 函数功能：容量参数恢复默认值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17          戴辉发             创建
==============================================================*/
static void other_para_data_default(void)
{
    g_other_para.cell_num = BAT_NUM;
    /* UI功能开关 */
    g_other_para.show_flag = 0x0C;
    /* 外部开关功能 */
    g_other_para.outside_flag = 0x00;
}

/*==============================================================
* 函数名称：other_para_data_hard_init
* 函数功能：其他参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
static void other_para_data_hard_init(void)
{
	short flag = 0;
	t_other_para_st other_data;

	flag = read_data_from_storage(OTHER_PARA_SECTOR_MAIN, (uint8_t *)&g_other_para, sizeof(g_other_para));
    if( flag == sizeof(g_other_para) )
    {
        flag = read_data_from_storage(OTHER_PARA_SECTOR_BACK, (uint8_t *)&other_data, sizeof(other_data));
        if( flag == sizeof(g_other_para) )
        {
	       flag = judge_new_sector(g_other_para.end_id, other_data.end_id);
        }
    }
	


	if (flag == -1)
	/* 备份扇区数据最新 */
	{
		g_other_main_flag = 1;
		g_other_end_id = other_data.end_id;
		memcpy(&g_other_para, &other_data, sizeof(other_data));
	}
	else if (flag == 0)
	/* 主备均没有写入过数据 */
	{
		g_other_main_flag = 1;
		g_other_end_id = MAX_STORAGE_NUM;

        other_para_data_default(); 
	}
	else
	/* 主扇区数据最新 */
	{
		g_other_main_flag = 0;
		g_other_end_id = g_other_para.end_id;
	}
}

/*==============================================================
* 函数名称：save_other_para_parameter
* 函数功能：保存容量参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_other_para_parameter(void)
{
	t_write_data_st data;

	g_other_end_id = sector_num_next(g_other_end_id);

	g_other_para.end_id = g_other_end_id;

	if (g_other_main_flag)
    {
		g_other_main_flag = 0;
		data.write_storge_address = OTHER_PARA_SECTOR_MAIN;
	}
	else
    {
		g_other_main_flag = 1;
		data.write_storge_address = OTHER_PARA_SECTOR_MAIN;
	}
	data.msg_type = E_RUN_PARAMETER_PARA_MSG;
	data.write_len = sizeof(g_other_para);
	data.write_memery_address = (uint8_t *)&g_other_para;

	set_write_data_to_queue(&data);
}

/*=============================================================
* 函数名称：system_para_default
* 函数功能：设定系统参数为缺省值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17         戴辉发             创建
==============================================================*/
void system_para_default(void)
{
    vol_para_data_default();
    save_vol_para_parameter();
    curr_para_data_default();
    save_curr_para_parameter();
    temp_para_data_default();
    save_temp_para_parameter();
    balance_para_data_default();
    save_balance_para_parameter();
    capcity_para_data_default();
    save_capcity_para_parameter();
    other_para_data_default();
    save_other_para_parameter();
    
    default_capacity_adjust();
}

/*=============================================================
* 函数名称：system_parameter_hard_init
* 函数功能：系统参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void system_parameter_hard_init(void)
{
	/* 电压参数内存初始化 */
	vol_para_data_hard_init();
	/* 电流参数内存初始化 */
	curr_para_data_hard_init();
	/* 温度参数内存初始化 */
	temp_para_data_hard_init();
	/* 均衡参数内存初始化 */
	balance_para_data_hard_init();
	/* 容量参数内存初始化 */
	capcity_para_data_hard_init();
	/* 其他参数内存初始化 */
	other_para_data_hard_init();  
}

/*=============================================================
* 函数名称：set_cell_vol_high_alarm_level
* 函数功能：设置单体过压告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_alarm_level(uint16_t alarm_level)
{
	g_vol_para.vol_para.cell_vol.high.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_cell_vol_high_alarm_level
* 函数功能：获取单体过压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_alarm_level(void)
{
	return g_vol_para.vol_para.cell_vol.high.alarm;
}

/*=============================================================
* 函数名称：set_cell_vol_high_alarm_recover
* 函数功能：设置单体过压告警恢复门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_alarm_recover(uint16_t alarm_level)
{
	g_vol_para.vol_para.cell_vol.high.alarm_recover = alarm_level;
}

/*=============================================================
* 函数名称：get_cell_vol_high_alarm_recover
* 函数功能：获取单体过压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压告警恢复门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_alarm_recover(void)
{
	return g_vol_para.vol_para.cell_vol.high.alarm_recover;
}

/*=============================================================
* 函数名称：set_cell_vol_high_protect
* 函数功能：设置单体过压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_protect(uint16_t protect)
{
	g_vol_para.vol_para.cell_vol.high.protect = protect;
}

/*=============================================================
* 函数名称：get_cell_vol_high_protect
* 函数功能：获取单体过压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_protect(void)
{
	return g_vol_para.vol_para.cell_vol.high.protect;
}

/*=============================================================
* 函数名称：set_cell_vol_high_recover
* 函数功能：设置单体过压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_recover(uint16_t level)
{
	g_vol_para.vol_para.cell_vol.high.recover = level;
}

/*=============================================================
* 函数名称：get_cell_vol_high_recover
* 函数功能：获取单体过压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_recover(void)
{
	return g_vol_para.vol_para.cell_vol.high.recover;
}

/*=============================================================
* 函数名称：set_cell_vol_low_alarm_level
* 函数功能：设置单体欠压告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_alarm_level(uint16_t alarm_level)
{
	g_vol_para.vol_para.cell_vol.low.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_cell_vol_low_alarm_level
* 函数功能：获取单体欠压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_alarm_level(void)
{
	return g_vol_para.vol_para.cell_vol.low.alarm;
}

/*=============================================================
* 函数名称：set_cell_vol_low_alarm_recover
* 函数功能：设置单体欠压告警恢复门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_alarm_recover(uint16_t alarm_level)
{
	g_vol_para.vol_para.cell_vol.low.alarm_recover = alarm_level;
}

/*=============================================================
* 函数名称：get_cell_vol_low_alarm_level
* 函数功能：获取单体欠压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压告警恢复门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_alarm_recover(void)
{
	return g_vol_para.vol_para.cell_vol.low.alarm_recover;
}

/*=============================================================
* 函数名称：set_cell_vol_low_protect
* 函数功能：设置单体欠压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_protect(uint16_t protect)
{
	g_vol_para.vol_para.cell_vol.low.protect = protect;
}

/*=============================================================
* 函数名称：get_cell_vol_low_protect
* 函数功能：获取单体欠压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_protect(void)
{
	return g_vol_para.vol_para.cell_vol.low.protect;
}

/*=============================================================
* 函数名称：set_cell_vol_low_recover
* 函数功能：设置单体欠压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_recover(uint16_t level)
{
	g_vol_para.vol_para.cell_vol.low.recover = level;
}

/*=============================================================
* 函数名称：get_cell_vol_low_recover
* 函数功能：获取单体欠压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_recover(void)
{
	return g_vol_para.vol_para.cell_vol.low.recover;
}

/*=============================================================
* 函数名称：set_vol_high_alarm_level
* 函数功能：设置总压过压告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_alarm_level(uint16_t alarm_level)
{
	g_vol_para.vol_para.voltage.high.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_vol_high_alarm_level
* 函数功能：获取总压过压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_alarm_level(void)
{
	return g_vol_para.vol_para.voltage.high.alarm;
}

/*=============================================================
* 函数名称：set_vol_high_alarm_recover
* 函数功能：设置总压过压告警恢复门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_alarm_recover(uint16_t alarm_level)
{
	g_vol_para.vol_para.voltage.high.alarm_recover = alarm_level;
}

/*=============================================================
* 函数名称：get_vol_high_alarm_recover
* 函数功能：获取总压过压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压告警恢复门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_alarm_recover(void)
{
	return g_vol_para.vol_para.voltage.high.alarm_recover;
}

/*=============================================================
* 函数名称：set_vol_high_protect
* 函数功能：设置总压过压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_protect(uint16_t protect)
{
	g_vol_para.vol_para.voltage.high.protect = protect;
}

/*=============================================================
* 函数名称：get_vol_high_protect
* 函数功能：获取总压过压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*          返回总压过压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_protect(void)
{
	return g_vol_para.vol_para.voltage.high.protect;
}

/*=============================================================
* 函数名称：set_vol_high_recover
* 函数功能：设置总压过压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_recover(uint16_t level)
{
	g_vol_para.vol_para.voltage.high.recover = level;
}

/*=============================================================
* 函数名称：get_vol_high_recover
* 函数功能：获取总压过压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*          返回总压过压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_recover(void)
{
	return g_vol_para.vol_para.voltage.high.recover;
}

/*=============================================================
* 函数名称：set_vol_low_alarm_level
* 函数功能：设置总压欠压告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_alarm_level(uint16_t alarm_level)
{
	g_vol_para.vol_para.voltage.low.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_vol_low_alarm_level
* 函数功能：获取总压欠压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*          返回总压欠压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_alarm_level(void)
{
	return g_vol_para.vol_para.voltage.low.alarm;
}

/*=============================================================
* 函数名称：set_vol_low_alarm_recover
* 函数功能：设置总压欠压告警恢复门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_alarm_recover(uint16_t alarm_level)
{
	g_vol_para.vol_para.voltage.low.alarm_recover = alarm_level;
}

/*=============================================================
* 函数名称：get_vol_low_alarm_recover
* 函数功能：获取总压欠压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压告警恢复门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_alarm_recover(void)
{
	return g_vol_para.vol_para.voltage.low.alarm_recover;
}

/*=============================================================
* 函数名称：set_vol_low_protect
* 函数功能：设置总压欠压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_protect(uint16_t protect)
{
	g_vol_para.vol_para.voltage.low.protect = protect;
}

/*=============================================================
* 函数名称：get_vol_low_protect
* 函数功能：获取总压欠压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_protect(void)
{
	return g_vol_para.vol_para.voltage.low.protect;
}

/*=============================================================
* 函数名称：set_vol_low_recover
* 函数功能：设置总压欠压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_recover(uint16_t level)
{
	g_vol_para.vol_para.voltage.low.recover = level;
}

/*=============================================================
* 函数名称：get_vol_low_protect_recover
* 函数功能：获取总压欠压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_recover(void)
{
	return g_vol_para.vol_para.voltage.low.recover;
}

/*=============================================================
* 函数名称：set_vol_protect_dealy
* 函数功能：设置电压保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_protect_delay(unsigned int delay)
{
	g_vol_para.pretect_delay = delay;
}

/*=============================================================
* 函数名称：get_vol_protect_delay
* 函数功能：获取电压保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电压保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_protect_delay(void)
{
    if (g_vol_para.pretect_delay <= 50)
    {
        return 10*g_vol_para.pretect_delay;
    }
    return 50;
}

/*=============================================================
* 函数名称：set_vol_dis_charge_level
* 函数功能：设置单体电压低压禁充门限
* 参数个数：1
* 函数参数：
*           [IN]      level              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_dis_charge_level(uint16_t level)
{
	g_vol_para.dis_charge_level = level;
}

/*=============================================================
* 函数名称：get_vol_dis_charge_level
* 函数功能：获取单体电压低压禁充门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           单体电压低压禁充门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-12          戴辉发             创建
==============================================================*/
uint16_t get_vol_dis_charge_level(void)
{
	return g_vol_para.dis_charge_level;
}

/*=============================================================
* 函数名称：set_ch_current_alarm_level
* 函数功能：设置充电过流告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_alarm_level(short alarm_level)
{
	g_curr_para.ch_current.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_ch_current_alarm_level
* 函数功能：获取充电过流告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_alarm_level(void)
{
	return g_curr_para.ch_current.alarm;
}

/*=============================================================
* 函数名称：set_ch_current_in_level
* 函数功能：设置充电电流进入门限
* 参数个数：1
*           [IN]      level              门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_in_level(short level)
{
	g_curr_para.ch_current.in_level = level;
}

/*=============================================================
* 函数名称：get_ch_current_in_level
* 函数功能：获取充电电流进入门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电电流进入门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_in_level(void)
{
	return g_curr_para.ch_current.in_level;
}

/*=============================================================
* 函数名称：set_ch_current_protect
* 函数功能：设置充电过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_protect(short level)
{
	g_curr_para.ch_current.protect = level;
}

/*=============================================================
* 函数名称：get_ch_current_high_protect
* 函数功能：获取充电过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_protect(void)
{
	return g_curr_para.ch_current.protect;
}

/*=============================================================
* 函数名称：set_ch_current_protect_delay
* 函数功能：设置充电过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      level              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_protect_delay(uint16_t level)
{
	g_curr_para.ch_current.protect_delay = level;
}

/*=============================================================
* 函数名称：get_ch_current_protect_delay
* 函数功能：获取充电过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_ch_current_protect_delay(void)
{
	return g_curr_para.ch_current.protect_delay;
}

/*=============================================================
* 函数名称：set_ch_current_limit
* 函数功能：设置充电限流值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_limit(short level)
{
	g_curr_para.ch_current.limit = level;
}

/*=============================================================
* 函数名称：get_ch_current_limit
* 函数功能：获取充电限流值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电限流值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_limit(void)
{
	return g_curr_para.ch_current.limit;
}

/*=============================================================
* 函数名称：set_dch_current_in_level
* 函数功能：设置放电电流进入门限
* 参数个数：1
*           [IN]      in_level           门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_in_level(short level)
{
	g_curr_para.dch_current.in_level = level;
}

/*=============================================================
* 函数名称：get_dch_current_in_level
* 函数功能：获取放电电流进入门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电电流进入门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_in_level(void)
{
	return g_curr_para.dch_current.in_level;
}

/*==============================================================
* 函数名称：set_dch_current_locked_num
* 函数功能：设置放电过流锁定次数
* 参数个数：1
* 函数参数：
*           [IN]      value              放电过流锁定次数
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_locked_num(uint16_t value)
{
	g_curr_para.dch_current.locked_num = value;
}

/*==============================================================
* 函数名称：get_dch_current_locked_num
* 函数功能：获取放电过流锁定次数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流锁定次数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_locked_num(void)
{
	return g_curr_para.dch_current.locked_num;
}

/*=============================================================
* 函数名称：set_dch_current_alarm_level
* 函数功能：设置放电过流告警门限
* 参数个数：1
*           [IN]      level              门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_high_level(short level)
{
	g_curr_para.dch_current.alarm = level;
}

/*=============================================================
* 函数名称：get_dch_current_alarm_level
* 函数功能：获取放电过流告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_alarm_level(void)
{
	return g_curr_para.dch_current.alarm;
}

/*==============================================================
* 函数名称：set_dch_current_protect
* 函数功能：设置放电过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect(short level)
{
	g_curr_para.dch_current.protect = level;
}

/*==============================================================
* 函数名称：get_dch_current_protect
* 函数功能：获取放电过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_protect(void)
{
	return g_curr_para.dch_current.protect;
}

/*==============================================================
* 函数名称：set_dch_current_protect_delay
* 函数功能：设置放电过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              过流保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect_delay(uint16_t value)
{
	g_curr_para.dch_current.protect_delay = value;
}

/*==============================================================
* 函数名称：get_dch_current_protect_delay
* 函数功能：获取放电过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流保护延时
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_protect_delay(void)
{
	return g_curr_para.dch_current.protect_delay;
}

/*==============================================================
* 函数名称：set_dch_current_second_protect
* 函数功能：设置放电二次过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_second_protect(short protect)
{
	g_curr_para.dch_current.second_protect = protect;
}

/*==============================================================
* 函数名称：get_dch_current_second_protect
* 函数功能：获取放电二次过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电二次过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_second_protect(void)
{
	return g_curr_para.dch_current.second_protect;
}

/*==============================================================
* 函数名称：set_dch_current_second_delay
* 函数功能：设置放电二次过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              放电过流保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_second_delay(uint16_t value)
{
	g_curr_para.dch_current.second_delay = value;
}

/*==============================================================
* 函数名称：get_dch_current_second_delay
* 函数功能：获取放电二次过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电二次过流保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_second_delay(void)
{
	return g_curr_para.dch_current.second_delay;
}

/*==============================================================
* 函数名称：set_dch_current_short_protect
* 函数功能：设置短路保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_protect(short protect)
{
	g_curr_para.dch_current.short_protect = protect;
}

/*==============================================================
* 函数名称：get_dch_current_short_protect
* 函数功能：获取短路保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_short_protect(void)
{
	return g_curr_para.dch_current.short_protect;
}

/*==============================================================
* 函数名称：set_dch_current_short_delay
* 函数功能：设置短路保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_delay(uint16_t value)
{
	g_curr_para.dch_current.short_delay = value;
}

/*==============================================================
* 函数名称：get_dch_current_short_delay
* 函数功能：获取短路保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_short_delay(void)
{
	return g_curr_para.dch_current.short_delay;
}

/*==============================================================
* 函数名称：set_dch_current_short_recover_delay
* 函数功能：设置短路保护恢复延时
* 参数个数：1
* 函数参数：
*           [IN]      value              恢复延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_recover_delay(uint16_t value)
{
	g_curr_para.dch_current.short_recover_delay = value;
}

/*==============================================================
* 函数名称：get_dch_current_short_recover_delay
* 函数功能：获取短路保护恢复延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护恢复延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_short_recover_delay(void)
{
	return g_curr_para.dch_current.short_recover_delay;
}

/*==============================================================
* 函数名称：set_dch_current_protect_recover_delay
* 函数功能：设置过流保护恢复延时
* 参数个数：1
* 函数参数：
*           [IN]      value              恢复延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect_recover_delay(uint16_t value)
{
	g_curr_para.dch_current.protect_recover_delay = value;
}

/*==============================================================
* 函数名称：get_dch_current_protect_recover_delay
* 函数功能：获取过流保护恢复延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回过流保护恢复延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_protect_recover_delay(void)
{
	return g_curr_para.dch_current.protect_recover_delay;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_high_alarm_level
* 函数功能：设置充电高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_alarm_level(short alarm_level)
{
	g_temp_para.cell_temp.ch_temp.high.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_high_alarm_level
* 函数功能：获取充电高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_high_alarm_level(void)
{
	return g_temp_para.cell_temp.ch_temp.high.alarm;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_high_protect
* 函数功能：设置高温禁充门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_protect(short protect)
{
	g_temp_para.cell_temp.ch_temp.high.protect = protect;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_high_protect
* 函数功能：获取高温禁充门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温禁充门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_high_protect(void)
{
	return g_temp_para.cell_temp.ch_temp.high.protect;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_high_recover
* 函数功能：设置高温充释门限值
* 参数个数：1
* 函数参数：
*           [IN]      levle              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_recover(short levle)
{
	g_temp_para.cell_temp.ch_temp.high.recover = levle;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_high_recover
* 函数功能：获取高温充释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温充释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_ch_cell_temp_high_recover(void)
{
	return g_temp_para.cell_temp.ch_temp.high.recover;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_low_alarm_level
* 函数功能：设置充电低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_alarm_level(short alarm_level)
{
	g_temp_para.cell_temp.ch_temp.low.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_low_alarm_level
* 函数功能：获取充电低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_alarm_level(void)
{
	return g_temp_para.cell_temp.ch_temp.low.alarm;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_low_protect
* 函数功能：设置低温禁充门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_protect(short protect)
{
	g_temp_para.cell_temp.ch_temp.low.protect = protect;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_low_protect
* 函数功能：获取低温禁充门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温禁充门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_protect(void)
{
	return g_temp_para.cell_temp.ch_temp.low.protect;
}

/*=============================================================
* 函数名称：set_ch_cell_temp_low_recover
* 函数功能：设置低温充释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_recover(short level)
{
	g_temp_para.cell_temp.ch_temp.low.recover = level;
}

/*=============================================================
* 函数名称：get_ch_cell_temp_low_recover
* 函数功能：获取低温充释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温充释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_recover(void)
{
	return g_temp_para.cell_temp.ch_temp.low.recover;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_high_alarm_level
* 函数功能：设置放电高温告警门限
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_alarm_level(short level)
{
	g_temp_para.cell_temp.dch_temp.high.alarm = level;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_high_alarm_level
* 函数功能：获取放电高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_alarm_level(void)
{
	return g_temp_para.cell_temp.dch_temp.high.alarm;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_high_protect
* 函数功能：设置高温禁放门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_protect(short protect)
{
	g_temp_para.cell_temp.dch_temp.high.protect = protect;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_high_protect
* 函数功能：获取高温禁放门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温禁放门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_protect(void)
{
	return g_temp_para.cell_temp.dch_temp.high.protect;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_high_recover
* 函数功能：设置高温放释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_recover(short level)
{
	g_temp_para.cell_temp.dch_temp.high.recover = level;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_high_recover
* 函数功能：获取高温放释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温放释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_recover(void)
{
	return g_temp_para.cell_temp.dch_temp.high.recover;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_low_alarm_level
* 函数功能：设置放电低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      level              门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_alarm_level(short level)
{
	g_temp_para.cell_temp.dch_temp.low.alarm = level;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_low_alarm_level
* 函数功能：获取放电低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_alarm_level(void)
{
	return g_temp_para.cell_temp.dch_temp.low.alarm;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_low_protect
* 函数功能：设置低温禁放门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_protect(short level)
{
	g_temp_para.cell_temp.dch_temp.low.protect = level;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_low_protect
* 函数功能：获取低温禁放门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温禁放门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_protect(void)
{
	return g_temp_para.cell_temp.dch_temp.low.protect;
}

/*=============================================================
* 函数名称：set_dch_cell_temp_low_recover
* 函数功能：设置低温放释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_recover(short level)
{
	g_temp_para.cell_temp.dch_temp.low.recover = level;
}

/*=============================================================
* 函数名称：get_dch_cell_temp_low_recover
* 函数功能：获取低温放释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温放释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_recover(void)
{
	return g_temp_para.cell_temp.dch_temp.low.recover;
}

/*=============================================================
* 函数名称：set_cell_tempprotect_delay
* 函数功能：设置电芯温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_tempprotect_delay(uint16_t delay)
{
	g_temp_para.cell_temp.protect_delay = delay;
}

/*=============================================================
* 函数名称：get_cell_tempprotect_delay
* 函数功能：获取电芯温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_tempprotect_delay(void)
{
	return g_temp_para.cell_temp.protect_delay;
}

/*=============================================================
* 函数名称：set_power_temp_high_alarm_level
* 函数功能：设置功率高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_alarm_level(short alarm_level)
{
	g_temp_para.power_temp.temp.high.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_power_temp_high_alarm_level
* 函数功能：获取功率高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_alarm_level(void)
{
	return g_temp_para.power_temp.temp.high.alarm;
}

/*=============================================================
* 函数名称：set_power_temp_high_protect
* 函数功能：设置功率高温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_protect(short protect)
{
	g_temp_para.power_temp.temp.high.protect = protect;
}

/*=============================================================
* 函数名称：get_power_temp_high_protect
* 函数功能：获取功率高温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_protect(void)
{
	return g_temp_para.power_temp.temp.high.protect;
}

/*=============================================================
* 函数名称：set_power_temp_high_recover
* 函数功能：设置功率高温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_recover(short level)
{
	g_temp_para.power_temp.temp.high.recover = level;
}

/*=============================================================
* 函数名称：get_power_temp_high_recover
* 函数功能：获取功率高温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_recover(void)
{
	return g_temp_para.power_temp.temp.high.recover;
}

/*=============================================================
* 函数名称：set_power_temp_low_alarm_level
* 函数功能：设置功率低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_alarm_level(short alarm_level)
{
	g_temp_para.power_temp.temp.low.alarm = alarm_level;
}

/*=============================================================
* 函数名称：get_power_temp_low_alarm_level
* 函数功能：获取功率低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_alarm_level(void)
{
	return g_temp_para.power_temp.temp.low.alarm;
}

/*==============================================================
* 函数名称：set_power_temp_low_protect
* 函数功能：设置功率低温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_protect(short protect)
{
	g_temp_para.power_temp.temp.low.protect = protect;
}

/*==============================================================
* 函数名称：get_power_temp_low_protect
* 函数功能：获取功率低温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_protect(void)
{
	return g_temp_para.power_temp.temp.low.protect;
}

/*==============================================================
* 函数名称：set_power_temp_low_recover
* 函数功能：设置功率低温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_recover(short level)
{
	g_temp_para.power_temp.temp.low.recover = level;
}

/*==============================================================
* 函数名称：get_power_temp_low_recover
* 函数功能：获取功率低温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_recover(void)
{
	return g_temp_para.power_temp.temp.low.recover;
}

/*==============================================================
* 函数名称：set_power_temp_protect_delay
* 函数功能：设置功率温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_protect_delay(uint16_t delay)
{
	g_temp_para.power_temp.protect_delay = delay;
}

/*==============================================================
* 函数名称：get_power_temp_protect_delay
* 函数功能：获取功率温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_power_temp_protect_delay(void)
{
	return g_temp_para.power_temp.protect_delay;
}

/*==============================================================
* 函数名称：set_enviror_temperature_high_alarm_level
* 函数功能：设置环境高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_alarm_level(short alarm_level)
{
	g_temp_para.enviror_temp.temp.high.alarm = alarm_level;
}

/*==============================================================
* 函数名称：get_enviror_temp_high_alarm_level
* 函数功能：获取环境高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_alarm_level(void)
{
	return g_temp_para.enviror_temp.temp.high.alarm;
}

/*==============================================================
* 函数名称：set_enviror_temp_high_protect
* 函数功能：设置环境高温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_protect(short protect)
{
	g_temp_para.enviror_temp.temp.high.protect = protect;
}

/*==============================================================
* 函数名称：get_enviror_temp_high_protect
* 函数功能：获取环境高温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_protect(void)
{
	return g_temp_para.enviror_temp.temp.high.protect;
}

/*==============================================================
* 函数名称：set_enviror_temp_high_recover
* 函数功能：设置环境高温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_recover(short level)
{
	g_temp_para.enviror_temp.temp.high.recover = level;
}

/*==============================================================
* 函数名称：get_enviror_temp_high_recover
* 函数功能：获取环境高温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_recover(void)
{
	return g_temp_para.enviror_temp.temp.high.recover;
}

/*==============================================================
* 函数名称：set_enviror_temp_low_alarm_level
* 函数功能：设置环境低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_alarm_level(short alarm_level)
{
	g_temp_para.enviror_temp.temp.low.alarm = alarm_level;
}

/*==============================================================
* 函数名称：get_enviror_temp_low_alarm_level
* 函数功能：获取环境低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_alarm_level(void)
{
	return g_temp_para.enviror_temp.temp.low.alarm;
}

/*==============================================================
* 函数名称：set_enviror_temp_low_protect
* 函数功能：设置环境低温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_protect(short level)
{
	g_temp_para.enviror_temp.temp.low.protect = level;
}

/*==============================================================
* 函数名称：get_enviror_temp_low_protect
* 函数功能：获取环境低温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_protect(void)
{
	return g_temp_para.enviror_temp.temp.low.protect;
}

/*==============================================================
* 函数名称：set_enviror_temp_low_recover
* 函数功能：设置环境低温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_recover(short level)
{
	g_temp_para.enviror_temp.temp.low.recover = level;
}

/*==============================================================
* 函数名称：get_enviror_temp_low_recover
* 函数功能：获取环境低温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_recover(void)
{
	return g_temp_para.enviror_temp.temp.low.recover;
}

/*==============================================================
* 函数名称：set_enviror_temp_protect_delay
* 函数功能：设置环境温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_protect_delay(uint16_t delay)
{
	g_temp_para.enviror_temp.protect_delay = delay;
}

/*==============================================================
* 函数名称：get_enviror_temp_protect_delay
* 函数功能：获取环境温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_enviror_temp_protect_delay(void)
{
	return g_temp_para.enviror_temp.protect_delay;
}

/*==============================================================
* 函数名称：set_hot_start_temp
* 函数功能：设置加热起始温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              加热起始温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_hot_start_temp(short value)
{
	g_hot_balance_para.hot.start = value;
}

/*==============================================================
* 函数名称：get_hot_start_temp
* 函数功能：获取加热起始温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回加热起始温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_hot_start_temp(void)
{
	return g_hot_balance_para.hot.start;
}

/*==============================================================
* 函数名称：set_hot_end_temp
* 函数功能：设置加热结束温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              加热结束温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_hot_end_temp(short value)
{
	g_hot_balance_para.hot.end = value;
}

/*==============================================================
* 函数名称：get_hot_end_temp
* 函数功能：获取加热结束温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回加热结束温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_hot_end_temp(void)
{
	return g_hot_balance_para.hot.end;
}

/*==============================================================
* 函数名称：set_balance_start_temp
* 函数功能：设置均衡工作最低温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡工作最低温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_temp(short value)
{
	g_hot_balance_para.balance.start_temp = value;
}

/*==============================================================
* 函数名称：get_balance_start_temp
* 函数功能：获取均衡工作最低温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡工作最低温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_balance_start_temp(void)
{
	return g_hot_balance_para.balance.start_temp;
}

/*==============================================================
* 函数名称：set_balance_end_temp
* 函数功能：设置均衡工作最高温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡工作最高温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_end_temp(short value)
{
	g_hot_balance_para.balance.end_temp = value;
}

/*==============================================================
* 函数名称：get_balance_end_temp
* 函数功能：获取均衡工作最高温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡工作最高温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_balance_end_temp(void)
{
	return g_hot_balance_para.balance.end_temp;
}

/*==============================================================
* 函数名称：set_balance_start_difference
* 函数功能：设置均衡起始压差
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡起始压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_difference(uint16_t value)
{
	g_hot_balance_para.balance.diff_start = value;
}

/*==============================================================
* 函数名称：get_balance_start_difference
* 函数功能：获取均衡起始压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡起始压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_start_difference(void)
{
	return g_hot_balance_para.balance.diff_start;
}

/*==============================================================
* 函数名称：set_balance_end_difference
* 函数功能：设置均衡结束压差
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡结束压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_end_difference(uint16_t value)
{
	g_hot_balance_para.balance.diff_end = value;
}

/*==============================================================
* 函数名称：get_balance_end_difference
* 函数功能：获取均衡结束压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡结束压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_end_difference(void)
{
	return g_hot_balance_para.balance.diff_end;
}

/*==============================================================
* 函数名称：set_balance_start_voltage
* 函数功能：设置均衡起始电压
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡起始电压
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_voltage(uint16_t value)
{
	g_hot_balance_para.balance.start = value;
}

/*==============================================================
* 函数名称：get_balance_start_voltage
* 函数功能：获取均衡起始电压
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡起始电压
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_start_voltage(void)
{
	return g_hot_balance_para.balance.start;
}

/*==============================================================
* 函数名称：set_cell_fail
* 函数功能：设置电芯失效门限值
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_fail(uint16_t value)
{
	g_hot_balance_para.balance.fail_level = value;
}

/*==============================================================
* 函数名称：get_cell_fail
* 函数功能：获取电芯失效门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯失效门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_fail(void)
{
	return g_hot_balance_para.balance.fail_level;
}

/*==============================================================
* 函数名称：set_cell_recover
* 函数功能：设置电芯失效恢复压差
* 参数个数：1
* 函数参数：
*           [IN]      value              电芯失效恢复压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_recover(uint16_t value)
{
	g_hot_balance_para.balance.recover_level = value;
}

/*==============================================================
* 函数名称：get_cell_recover
* 函数功能：获取电芯失效恢复压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯失效恢复压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_recover(void)
{
	return g_hot_balance_para.balance.recover_level;
}

/*==============================================================
* 函数名称：set_balance_delay
* 函数功能：设置均衡时间限时
* 参数个数：1
* 函数参数：
*           [IN]      delay              均衡时间限时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_delay(uint16_t delay)
{
	g_hot_balance_para.balance.delay = delay;
}

/*==============================================================
* 函数名称：get_balance_delay
* 函数功能：获取均衡时间限时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡时间限时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
unsigned int get_balance_delay(void)
{
	return g_hot_balance_para.balance.delay;
}

/*==============================================================
* 函数名称：set_smart_soc_level
* 函数功能：设置智能补电门限
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_smart_soc_level(uint8_t level)
{
	g_capcity_para.smart.soc_level = level;
}

/*==============================================================
* 函数名称：get_smart_soc_level
* 函数功能：获取智能补电门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回智能补电门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_smart_soc_level(void)
{
	return g_capcity_para.smart.soc_level;
}

/*==============================================================
* 函数名称：set_smart_delay
* 函数功能：设置智能补电周期
* 参数个数：1
* 函数参数：
*           [IN]      value              智能补电周期
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_smart_delay(uint8_t value)
{
	g_capcity_para.smart.delay = value;
}

/*==============================================================
* 函数名称：get_smart_delay
* 函数功能：获取智能补电周期
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回智能补电周期
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_smart_delay(void)
{
	return g_capcity_para.smart.delay;
}

/*==============================================================
* 函数名称：set_low_capcity
* 函数功能：设置一级容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_low_capcity(uint8_t value)
{
	g_capcity_para.capcity.low = value;
}

/*==============================================================
* 函数名称：get_low_capcity
* 函数功能：获取一级容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           一级容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_low_capcity(void)
{
	return g_capcity_para.capcity.low;
}

/*==============================================================
* 函数名称：set_protect_capcity
* 函数功能：设置保护容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_protect_capcity(uint8_t value)
{
	g_capcity_para.capcity.protect = value;
}

/*==============================================================
* 函数名称：get_protect_capcity
* 函数功能：获取保护容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           保护容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_protect_capcity(void)
{
	return g_capcity_para.capcity.protect;
}

/*==============================================================
* 函数名称：set_high_capcity
* 函数功能：设置高容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_high_capcity(uint8_t value)
{
	g_capcity_para.capcity.high = value;
}

/*==============================================================
* 函数名称：get_high_capcity
* 函数功能：获取高容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           高容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_high_capcity(void)
{
	return g_capcity_para.capcity.high;
}

/*==============================================================
* 函数名称：set_rated_capcity
* 函数功能：设置额定容量
* 参数个数：1
* 函数参数：
*           [IN]      value              额定容量
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_rated_capcity(uint16_t value)
{
	g_capcity_para.rated_capcity = value;
}

/*==============================================================
* 函数名称：get_rated_capcity
* 函数功能：获取额定容量
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回额定容量
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_rated_capcity(void)
{
	return g_capcity_para.rated_capcity;
}

/*==============================================================
* 函数名称：set_low_power_delay
* 函数功能：设置低功耗延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
void set_low_power_delay(uint16_t delay)
{
	g_capcity_para.low_power_delay = delay;
}

/*==============================================================
* 函数名称：get_low_power_delay
* 函数功能：获取低功耗延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低功耗延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
uint16_t get_low_power_delay(void)
{
	return g_capcity_para.low_power_delay;
}
                                   
/*==============================================================
* 函数名称：set_deactive_delay
* 函数功能：设置不活动延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
void set_deactive_delay(uint16_t delay)
{
	g_capcity_para.deactive_delay = delay;
}

/*==============================================================
* 函数名称：get_deactive_delay
* 函数功能：获取不活动延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回不活动延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
uint16_t get_deactive_delay(void)
{
	return g_capcity_para.deactive_delay;
}

/*==============================================================
* 函数名称：set_low_deactive_delay
* 函数功能：设置低功耗不活动延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void set_low_deactive_delay(uint16_t delay)
{
	g_capcity_para.low_deactive_delay = delay;
}

/*==============================================================
* 函数名称：get_low_deactive_delay
* 函数功能：获取低功耗不活动延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低功耗不活动延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
uint16_t get_low_deactive_delay(void)
{
	return g_capcity_para.low_deactive_delay;
}

/*=============================================================
* 函数名称：set_voltage_flag
* 函数功能：设定电压功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_voltage_flag(uint8_t flag)
{
	g_vol_para.voltage_flag = flag;
}

/*=============================================================
* 函数名称：get_voltage_flag
* 函数功能：获取电压功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电压功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_voltage_flag(void)
{
	return g_vol_para.voltage_flag;
}

/*=============================================================
* 函数名称：set_current_flag
* 函数功能：设定电流功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_current_flag(uint8_t flag)
{
	g_curr_para.current_flag = flag;
}

/*=============================================================
* 函数名称：get_current_flag
* 函数功能：获取电流功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电流功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_current_flag(void)
{
	return g_curr_para.current_flag;
}

/*=============================================================
* 函数名称：set_temperature_flag
* 函数功能：设定温度功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_temperature_flag(uint8_t flag)
{
	g_temp_para.temp_flag = flag;
}

/*=============================================================
* 函数名称：get_temperature_flag
* 函数功能：获取温度功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           温度功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_temperature_flag(void)
{
	return g_temp_para.temp_flag;
}

/*=============================================================
* 函数名称：set_balance_flag
* 函数功能：设置均衡开关参数
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_flag(uint8_t flag)
{
	g_hot_balance_para.balance_flag = flag;
}

/*=============================================================
* 函数名称：get_balance_flag
* 函数功能：获取均衡开关参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡开关参数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_balance_flag(void)
{
	return g_hot_balance_para.balance_flag;
}

/*=============================================================
* 函数名称：set_capcity_flag
* 函数功能：设定电池容量告警开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               电池容量告警开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_capcity_flag(uint8_t flag)
{
	g_capcity_para.capcity_flag = flag;
}

/*=============================================================
* 函数名称：get_capcity_flag
* 函数功能：获取电池容量告警开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电池容量告警开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_capcity_flag(void)
{
	return g_capcity_para.capcity_flag;
}

/*=============================================================
* 函数名称：set_show_switch_flag
* 函数功能：设置显示开关参数
* 参数个数：1
* 函数参数：
*           [IN]      flag               显示开关参数
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_show_flag(uint8_t flag)
{
	g_other_para.show_flag = flag;
}

/*=============================================================
* 函数名称：get_show_flag
* 函数功能：获取显示开关参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回显示开关参数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_show_flag(void)
{
	return g_other_para.show_flag;
}

/*=============================================================
* 函数名称：set_outside_flag
* 函数功能：设定外部开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               外部开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_outside_flag(uint8_t flag)
{
	g_other_para.outside_flag = flag;
}

/*=============================================================
* 函数名称：get_outside_flag
* 函数功能：获取外部开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           外部开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_outside_flag(void)
{
	return g_other_para.outside_flag;
}

