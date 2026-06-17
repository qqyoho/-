/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：peak_record.c
 * 文件功能描述：实现系统复位次数
 *
 * 修改记录：
 * 2019-11-18 戴辉发   创建
*----------------------------------------------------------*/
#include <string.h>

#include "parameter.h"
#include "protect_record.h"
#include "storage_manage.h"
#include "rtc.h"
#include "fm33lg0xx_fl.h"

#define PEAK_RECORD_TIME           (180)
 
static uint8_t g_protect1_main_flag;
static uint8_t g_protect2_main_flag;
static uint8_t g_fault1_main_flag;
static uint8_t g_fault2_main_flag;
uint32_t mcu_reset;

/*=============================================================
 * 函数名称：protect1_record_default
 * 函数功能：保护1数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void protect1_record_default(void)
{
    protect1_numb.ch_ov_curr_num.conut = 0;
    protect1_numb.dch_over_curr_num.conut = 0;
    protect1_numb.disch_temp_num.conut = 0;
    protect1_numb.disdch_temp_num.conut = 0;
    protect1_numb.low_vol_num.conut = 0;
    protect1_numb.over_vol_num.conut = 0;
    protect1_numb.number = MAX_STORAGE_NUM;
}
/*=============================================================
 * 函数名称：peak_record_default
 * 函数功能：保护2数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void protect2_record_default(void)
{
     protect2_numb.afe_ocd_num.conut = 0;
     protect2_numb.afe_ov_num.conut = 0;
     protect2_numb.afe_uv_num.conut = 0;
     protect2_numb.mcu_ov_num.conut = 0;
     protect2_numb.mcu_uv_num.conut = 0;
     protect2_numb.short_num.conut = 0;
     protect2_numb.number = MAX_STORAGE_NUM;
}

/*=============================================================
 * 函数名称：protect_record_mem_init
 * 函数功能：保护计数模块内存初始化
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                      修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void protect_record_mem_init(void)
{
	protect1_record_default();
    protect2_record_default();
    g_protect1_main_flag = 0;
    g_protect2_main_flag = 0;
}

/*=============================================================
 * 函数名称：fault1_record_default
 * 函数功能：异常1数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void fault1_record_default(void)
{
    fault1_numb_record.afe_error_num.conut = 0;
    fault1_numb_record.afe_vol_c_num.conut = 0;
    fault1_numb_record.afe_vol_d_num.conut = 0;
    fault1_numb_record.afe_vol_s_num.conut = 0;
    fault1_numb_record.poweroff_fault_num.conut = 0;
    fault1_numb_record.poweron_fault_num.conut = 0;
    fault1_numb_record.number = MAX_STORAGE_NUM;
}

/*=============================================================
 * 函数名称：fault2_record_default
 * 函数功能：异常2数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void fault2_record_default(void)
{
    fault2_numb_record.can_fault_num.conut = 0;
    fault2_numb_record.chmos_fault_time.conut = 0;
    fault2_numb_record.dismos_fault_time.conut = 0;
    fault2_numb_record.eeprom_fault_num.conut = 0;
    fault2_numb_record.mcu_reset_num.conut = 0;
    fault2_numb_record.system_run_time.conut = 0;
    fault2_numb_record.number = MAX_STORAGE_NUM;
}

/*=============================================================
 * 函数名称：peak_record_default
 * 函数功能：保护2数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void fault_record_mem_init(void)
{
    fault1_record_default();
    fault2_record_default();
    g_fault1_main_flag = 0;
    g_fault2_main_flag = 0;
}

/*=============================================================
 * 函数名称：protect_record_hard_init
 * 函数功能：保护数字模块数据读取
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void protect_record_hard_init(void)
{
    int16_t flag = 0;
    t_protect1_numb protect1_record;
    t_protect2_numb protect2_record;
    //获取极限值1
    flag = read_data_from_storage(PROTECT1_SECTOR_MAIN, (uint8_t *)&protect1_numb, sizeof(t_protect1_numb));
    if( flag == sizeof(t_protect1_numb) )
    {
        flag = read_data_from_storage(PROTECT1_SECTOR_BACK, (uint8_t *)&protect1_record, sizeof(t_protect1_numb));
        if( flag == sizeof(t_protect1_numb) )
        {
            flag = judge_new_sector(protect1_numb.number, protect1_record.number);
        }
    }

    if (flag == -1)
    {
        g_protect1_main_flag = 1;
        memcpy(&protect1_numb, &protect1_record, sizeof(protect1_record));
    }
    else if (flag == 0)
    {
        g_protect1_main_flag = 1;
        protect1_record_default();

    }
    else
    {
        g_protect1_main_flag = 0;
    }

    /* 获取极限值2 */
    flag = read_data_from_storage(PROTECT2_SECTOR_MAIN, (uint8_t *)&protect2_numb, sizeof(t_protect2_numb));
    if (flag == sizeof(t_protect2_numb))
    {
        flag = read_data_from_storage(PROTECT2_SECTOR_BACK, (uint8_t *)&protect2_record, sizeof(t_protect2_numb));
        if (flag == sizeof(t_protect2_numb))
        {
            flag = judge_new_sector(protect2_numb.number, protect2_record.number);
        }
    }

    if (flag == -1)
    {
        g_protect2_main_flag = 1;
        memcpy(&protect2_numb, &protect2_record, sizeof(t_protect2_numb));
    }
    else if (flag == 0)
    {
        g_protect2_main_flag = 1;
        protect2_record_default();
    }
    else
    {
        g_protect2_main_flag = 0;
    }
}

/*=============================================================
 * 函数名称：fault_record_hard_init
 * 函数功能：异常数字模块数据读取
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void fault_record_hard_init(void)
{
    int16_t flag = 0;
    t_fault1_numb_record fault1_record;
    t_fault2_numb_record fault2_record;

    //获取异常1
    flag = read_data_from_storage(FAULT1_SECTOR_MAIN, (uint8_t *)&fault1_numb_record, sizeof(t_fault1_numb_record));
    if (flag == sizeof(t_fault1_numb_record))
    {
        flag = read_data_from_storage(FAULT1_SECTOR_BACK, (uint8_t *)&fault1_record, sizeof(t_fault1_numb_record));
        if (flag == sizeof(t_fault1_numb_record))
        {
            flag = judge_new_sector(fault1_numb_record.number, fault1_record.number);
        }
    }

    if (flag == -1)
    {
        g_fault1_main_flag = 1;
        memcpy(&fault1_numb_record, &fault1_record, sizeof(fault1_record));
    }
    else if (flag == 0)
    {
        g_fault1_main_flag = 1;
        fault1_record_default();
    }
    else
    {
        g_fault1_main_flag = 0;
    }
    //获取异常2
    flag = read_data_from_storage(FAULT2_SECTOR_MAIN, (uint8_t *)&fault2_numb_record, sizeof(t_fault2_numb_record));
    if (flag == sizeof(t_fault2_numb_record))
    {
        flag = read_data_from_storage(FAULT2_SECTOR_BACK, (uint8_t *)&fault2_record, sizeof(t_fault2_numb_record));
        if (flag == sizeof(t_fault2_numb_record))
        {
            flag = judge_new_sector(fault2_numb_record.number, fault2_record.number);
        }
    }

	if (flag == -1)
	{
		g_fault2_main_flag = 1;
		memcpy(&fault2_numb_record, &fault2_record, sizeof(fault2_record));
	}
	else if (flag == 0)
	{
		g_fault2_main_flag = 1;
		fault2_record_default();
		
	}
	else
	{
		g_fault2_main_flag = 0;
	} 
}

/*=============================================================
 * 函数名称：recordprotect1_data
 * 函数功能：保存当前保护1记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void recordprotect1_data(void)
{
    uint16_t address;
    t_write_data_st data;

    protect1_numb.number = sector_num_next(protect1_numb.number);
    if (g_protect1_main_flag)
    {
        g_protect1_main_flag = 0;
        address = PROTECT1_SECTOR_MAIN;
    }
    else
    {
        g_protect1_main_flag = 1;
        address = PROTECT1_SECTOR_BACK;
    }

    data.msg_type = E_PROTECT1_RECORD_MSG;
    data.write_len = sizeof(t_protect1_numb);
    data.write_memery_address = (uint8_t *)&protect1_numb;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：recordprotect2_data
 * 函数功能：保存当前保护2记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void recordprotect2_data(void)
{
    uint16_t address;
    t_write_data_st data;

    protect2_numb.number = sector_num_next(protect2_numb.number);
    if (g_protect2_main_flag)
    {
        g_protect2_main_flag = 0;
        address = PROTECT2_SECTOR_MAIN;
    }
    else
    {
        g_protect2_main_flag = 1;
        address = PROTECT2_SECTOR_BACK;
    }

    data.msg_type = E_PROTECT2_RECORD_MSG;
    data.write_len = sizeof(t_protect2_numb);
    data.write_memery_address = (uint8_t *)&protect2_numb;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：set_cell_ov_count
 * 函数功能：设置过压次数
 * 参数个数：无
 * 参数描述：
 *          无
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_cell_ov_count(void)
{
    if (protect1_numb.over_vol_num.conut < 65530)
        protect1_numb.over_vol_num.conut ++;
    get_timdedata(protect1_numb.over_vol_num.time);
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_cell_uv_count
 * 函数功能：设置欠压次数
 * 参数个数：
 * 参数描述：
 *         
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_cell_uv_count(void)
{
    if( protect1_numb.low_vol_num.conut < 65530 )
        protect1_numb.low_vol_num.conut ++;
    get_timdedata( protect1_numb.low_vol_num.time );
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}


/*=============================================================
 * 函数名称：set_discurrent_over_count
 * 函数功能：放电过流次数
 * 参数个数：
 * 参数描述：
 *          
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_discurrent_over_count(void)
{
    if( protect1_numb.dch_over_curr_num.conut < 65530 )
        protect1_numb.dch_over_curr_num.conut ++;
    get_timdedata( protect1_numb.dch_over_curr_num.time );
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_chcurrent_over_count
 * 函数功能：充电过流次数
 * 参数个数：1
 * 参数描述：
 *         
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_chcurrent_over_count(void)
{
    if( protect1_numb.ch_ov_curr_num.conut < 65530 )
        protect1_numb.ch_ov_curr_num.conut ++;
    get_timdedata( protect1_numb.ch_ov_curr_num.time );
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_temp_nocharge
 * 函数功能：温度禁止充电次数
 * 参数个数：1
 * 参数描述：
 *          
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_temp_nocharge(void)
{
    if(protect1_numb.disch_temp_num.conut < 65530)
        protect1_numb.disch_temp_num.conut ++;
    get_timdedata( protect1_numb.disch_temp_num.time );
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_temp_nodischarge
 * 函数功能：温度禁止放电次数
 * 参数个数：
 * 参数描述：
 *         
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_temp_nodischarge(void)
{
    if( protect1_numb.disdch_temp_num.conut < 65530 )
        protect1_numb.disdch_temp_num.conut ++;
    get_timdedata( protect1_numb.disdch_temp_num.time );
    needstoreflag |= E_PROTECT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_short
 * 函数功能：短路次数累计
 * 参数个数：1
 * 参数描述：
 *          
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_short(void)
{
    if( protect2_numb.short_num.conut < 65530 )
        protect2_numb.short_num.conut ++;
    get_timdedata( protect2_numb.short_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_final_ov
 * 函数功能：终极过压累计
 * 参数个数：
 * 参数描述：
 *          
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_final_ov(void)
{
    if( protect2_numb.mcu_ov_num.conut < 65530 )
        protect2_numb.mcu_ov_num.conut ++;
    get_timdedata( protect2_numb.mcu_ov_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_final_uv
 * 函数功能：终极欠压累计
 * 参数个数：
 * 参数描述：
 *          
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_final_uv(void)
{
    if( protect2_numb.mcu_uv_num.conut < 65530 )
        protect2_numb.mcu_uv_num.conut ++;
    get_timdedata( protect2_numb.mcu_uv_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_ov
 * 函数功能：afe 过压次数统计
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_ov(void)
{
    if( protect2_numb.afe_ov_num.conut < 65530 )
        protect2_numb.afe_ov_num.conut ++;
    get_timdedata( protect2_numb.afe_ov_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}
/*=============================================================
 * 函数名称：set_afe_ov
 * 函数功能：afe 欠压次数统计
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_uv(void)
{
    if (protect2_numb.afe_uv_num.conut < 65530)
    {
        protect2_numb.afe_uv_num.conut ++;
    }
    get_timdedata( protect2_numb.afe_uv_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_ov
 * 函数功能：afe 过流次数统计
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_ocd(void)
{
    if (protect2_numb.afe_ocd_num.conut < 65530)
    {
        protect2_numb.afe_ocd_num.conut ++;
    }
    get_timdedata( protect2_numb.afe_ocd_num.time );
    needstoreflag |= E_PROTECT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：recordfault1_data
 * 函数功能：保存当前异常1记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void recordfault1_data(void)
{
    uint16_t address;
    t_write_data_st data;

    fault1_numb_record.number = sector_num_next(fault1_numb_record.number);
    if (g_fault1_main_flag)
    {
        g_fault1_main_flag = 0;
        address = FAULT1_SECTOR_MAIN;
    }
    else
    {
        g_fault1_main_flag = 1;
        address = FAULT1_SECTOR_BACK;
    }

    data.msg_type = E_FAULT1_RECORD_MSG;
    data.write_len = sizeof(t_protect1_numb);
    data.write_memery_address = (uint8_t *)&fault1_numb_record;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：recordfault2_data
 * 函数功能：保存当前异常2记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void recordfault2_data(void)
{
    uint16_t address;
    t_write_data_st data;

    fault2_numb_record.number = sector_num_next(fault2_numb_record.number);
    if (g_fault2_main_flag)
    {
        g_fault2_main_flag = 0;
        address = FAULT2_SECTOR_MAIN;
    }
    else
    {
        g_fault2_main_flag = 1;
        address = FAULT2_SECTOR_BACK;
    }

    data.msg_type = E_FAULT2_RECORD_MSG;
    data.write_len = sizeof(t_protect2_numb);
    data.write_memery_address = (uint8_t *)&fault2_numb_record;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：set_afe_commu
 * 函数功能：AFE通信故障次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_commu(void)
{
    if (fault1_numb_record.afe_error_num.conut < 65530)
        fault1_numb_record.afe_error_num.conut++;
    get_timdedata(fault1_numb_record.afe_error_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_volt_stable
 * 函数功能：AFE采集电压不变化次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_volt_stable(void)
{
  if( fault1_numb_record.afe_vol_s_num.conut < 65530 )
    fault1_numb_record.afe_vol_s_num.conut++;
    get_timdedata(fault1_numb_record.afe_vol_s_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_volt_differe
 * 函数功能：AFE电压与总压不一致异常采集次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_volt_differe(void)
{
   if( fault1_numb_record.afe_vol_d_num.conut < 65530 )
    fault1_numb_record.afe_vol_d_num.conut++;
    get_timdedata(fault1_numb_record.afe_vol_d_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_volt_current
 * 函数功能：AFE电压电流异常次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_afe_volt_current(void)
{
  if( fault1_numb_record.afe_vol_c_num.conut < 65530 )
    fault1_numb_record.afe_vol_c_num.conut++;
    get_timdedata(fault1_numb_record.afe_vol_c_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_volt_current
 * 函数功能：电源模块上电失败次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_power_on_fault(void)
{
  if( fault1_numb_record.poweron_fault_num.conut < 65530 )
    fault1_numb_record.poweron_fault_num.conut++;
    get_timdedata(fault1_numb_record.poweron_fault_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_afe_volt_current
 * 函数功能：电源模块下电失败次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_power_off_fault(void)
{
  if( fault1_numb_record.poweroff_fault_num.conut < 65530 )
    fault1_numb_record.poweroff_fault_num.conut++;
    get_timdedata(fault1_numb_record.poweroff_fault_num.time);
    needstoreflag |= E_FAULT1_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_eeprom_fault
 * 函数功能：存储器存储出错次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_eeprom_fault(void)
{
  if( fault2_numb_record.eeprom_fault_num.conut < 65530 )
    fault2_numb_record.eeprom_fault_num.conut++;
    get_timdedata(fault2_numb_record.eeprom_fault_num.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_can_fault
 * 函数功能：CAN通讯失败次数
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_can_fault(void)
{
  if( fault2_numb_record.can_fault_num.conut < 65530 )
    fault2_numb_record.can_fault_num.conut++;
    get_timdedata(fault2_numb_record.can_fault_num.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_chmos_fault
 * 函数功能：失效时电流+充电MOS失效时间
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_chmos_fault(void)
{
  if( fault2_numb_record.chmos_fault_time.conut < 65530 )
    fault2_numb_record.chmos_fault_time.conut = g_run_sys_data.current;
    get_timdedata(fault2_numb_record.chmos_fault_time.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_chmos_fault
 * 函数功能：失效时电流+放电MOS失效时间
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_dismos_fault(void)
{
  if( fault2_numb_record.dismos_fault_time.conut < 65530 )
    fault2_numb_record.dismos_fault_time.conut = g_run_sys_data.current;
    get_timdedata(fault2_numb_record.dismos_fault_time.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_mcu_reset_fault
 * 函数功能：历史累计MCU复位次数+时间
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_mcu_reset_fault(void)
{
    mcu_reset = RMU->RSTFR;
    RMU->RSTFR = mcu_reset;
   get_rtc_time();
   if( fault2_numb_record.mcu_reset_num.conut < 65530 )
    fault2_numb_record.mcu_reset_num.conut++;
    get_timdedata(fault2_numb_record.mcu_reset_num.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}

/*=============================================================
 * 函数名称：set_bms_run_time
 * 函数功能：BMS累计运行时间
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void set_bms_run_time(void)
{
  if( fault2_numb_record.system_run_time.conut < 65530 )
    fault2_numb_record.system_run_time.conut++;
    get_timdedata(fault2_numb_record.system_run_time.time);
    needstoreflag |= E_FAULT2_RECORD_MSG;
}


/*=============================================================
 * 函数名称：protect_data_clear
 * 函数功能：保护次数数据统一清0
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-10-13                  liyong       创建
==============================================================*/
void protect_data_clear(void)
{
    protect1_record_default();
    protect2_record_default();
    recordprotect1_data();
    recordprotect2_data();
}

/*=============================================================
 * 函数名称：fault_data_clear
 * 函数功能：异常次数数据统一清0
 * 参数个数：
 * 参数描述：
 *        
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-10-13                  liyong       创建
==============================================================*/
void fault_data_clear(void)
{
    fault1_record_default();
    fault2_record_default();
    recordfault1_data();
    recordfault2_data(); 
}