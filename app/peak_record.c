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
#include "peak_record.h"
#include "storage_manage.h"
#include "vol_manage.h"
#include "temp_manage.h"

#define PEAK_RECORD_TIME           (180)

t_peak_record_st1 g_peak_record1;
t_peak_record_st2 g_peak_record2;
static uint8_t g_peak1_main_flag;
static uint8_t g_peak2_main_flag;

/*=============================================================
 * 函数名称：peak_record1_default
 * 函数功能：极限1数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
static void peak_record1_default(void)
{
    g_peak_record1.max_vol.ultimatevaule = 0; /* 最大单体电压 */
	g_peak_record1.min_vol.ultimatevaule = 5000; /* 最小单体电压 */  
    g_peak_record1.max_totalvol.ultimatevaule = 0; /* 最大总电压 */
	g_peak_record1.min_totalvol.ultimatevaule = 500*16; /* 最小总电压 */   
	g_peak_record1.max_current.ultimatevaule= 0; /* 最高充电电流 */
	g_peak_record1.min_current.ultimatevaule = 0; /* 最大放电电流 */
}
/*=============================================================
 * 函数名称：peak_record_default
 * 函数功能：极限2数字模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
static void peak_record2_default(void)
{
	g_peak_record2.max_celltemp.ultimatevaule = 0; /* 最高电芯温度 */
	g_peak_record2.min_celltemp.ultimatevaule = 1250; /* 最低电芯温度 */
	g_peak_record2.max_envtemp.ultimatevaule = 0; /* 最高环境温度 */
	g_peak_record2.min_envtemp.ultimatevaule = 1250; /* 最低环境温度 */
	g_peak_record2.max_powtemp.ultimatevaule = 0; /* 最高功率温度 */
	g_peak_record2.min_powtemp.ultimatevaule = 1250; /* 最低功率温度 */
}

/*=============================================================
 * 函数名称：peak_record_mem_init
 * 函数功能：极限数字模块内存初始化
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                      修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
void peak_record_mem_init(void)
{
	peak_record1_default();
    peak_record2_default();
    g_peak1_main_flag = 0;
    g_peak2_main_flag = 0;
}

/*=============================================================
 * 函数名称：peak_record_hard_init
 * 函数功能：极限数字模块数据读取
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
void peak_record_hard_init(void)
{
	int16_t flag = 0;
	t_peak_record_st1 peak_record;
    t_peak_record_st2 peak_record2;
    //获取极限值1
    flag = read_data_from_storage(PEAK1_SECTOR_MAIN, (uint8_t *)&g_peak_record1, sizeof(t_peak_record_st1));   
    if( flag == sizeof(t_peak_record_st1) )
    {
        flag = read_data_from_storage(PEAK1_SECTOR_BACK, (uint8_t *)&peak_record, sizeof(t_peak_record_st1));
        if( flag == sizeof(t_peak_record_st1) )
        {
           flag = judge_new_sector(g_peak_record1.number, peak_record.number);
        }
    }
	
	if (flag == -1)
	{
		g_peak1_main_flag = 1;
		memcpy(&g_peak_record1, &peak_record, sizeof(peak_record));
	}
	else if (flag == 0)
	{
		g_peak1_main_flag = 1;
		peak_record1_default();
		g_peak_record1.number = MAX_STORAGE_NUM;
	}
	else
	{
		g_peak1_main_flag = 0;
	}
    
    
    //获取极限值2
    flag = read_data_from_storage(PEAK2_SECTOR_MAIN, (uint8_t *)&g_peak_record2, sizeof(t_peak_record_st2));
    if( flag ==  sizeof(t_peak_record_st2) )
    {
       flag = read_data_from_storage(PEAK2_SECTOR_BACK, (uint8_t *)&peak_record2, sizeof(t_peak_record_st2));
       if( flag ==  sizeof(t_peak_record_st2) )
       {
          flag = judge_new_sector(g_peak_record2.number, peak_record2.number);
       }
    }
    

	if (flag == -1)
	{
		g_peak2_main_flag = 1;
		memcpy(&g_peak_record2, &peak_record2, sizeof(t_peak_record_st2));
	}
	else if (flag == 0)
	{
		g_peak2_main_flag = 1;
		peak_record2_default();
		g_peak_record2.number = MAX_STORAGE_NUM;
	}
	else
	{
		g_peak2_main_flag = 0;
	}
    
}

/*=============================================================
 * 函数名称：save_peak1_record
 * 函数功能：保存当前极值记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
void save_peak1_record(void)
{
    uint16_t address;
    t_write_data_st data;

    g_peak_record1.number = sector_num_next(g_peak_record1.number);
    if (g_peak1_main_flag)
    {
        g_peak1_main_flag = 0;
        address = PEAK1_SECTOR_MAIN;
    }
    else
    {
        g_peak1_main_flag = 1;
        address = PEAK1_SECTOR_BACK;
    }

    data.msg_type = E_PEAK1_RECORD_MSG;
    data.write_len = sizeof(g_peak_record1);
    data.write_memery_address = (uint8_t *)&g_peak_record1;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：save_peak2_record
 * 函数功能：保存当前极值记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
void save_peak2_record(void)
{
    uint16_t address;
    t_write_data_st data;

    g_peak_record2.number = sector_num_next(g_peak_record2.number);
    if (g_peak2_main_flag)
    {
        g_peak2_main_flag = 0;
        address = PEAK2_SECTOR_MAIN;
    }
    else
    {
        g_peak2_main_flag = 1;
        address = PEAK2_SECTOR_BACK;
    }

    data.msg_type = E_PEAK2_RECORD_MSG;
    data.write_len = sizeof(g_peak_record2);
    data.write_memery_address = (uint8_t *)&g_peak_record2;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：set_max_cell
 * 函数功能：设置最高电压
 * 参数个数：1
 * 参数描述：
 *          [IN]    max_vol     电芯最高电压
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_max_cell(uint16_t max_vol)
{
    if (g_peak_record1.max_vol.ultimatevaule < max_vol)
    {
        g_peak_record1.max_vol.ultimatevaule = max_vol;
        get_timdedata(g_peak_record1.max_vol.time);
        g_peak_record1.max_vol.location = get_max_cell_index();
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_cell
 * 函数功能：设置最低电压
 * 参数个数：1
 * 参数描述：
 *          [IN]    min_vol     电芯最低电压
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_min_cell(uint16_t min_vol)
{
    if (g_peak_record1.min_vol.ultimatevaule > min_vol)
    {
        g_peak_record1.min_vol.ultimatevaule = min_vol;
        get_timdedata(g_peak_record1.min_vol.time);
        g_peak_record1.min_vol.location = get_min_cell_index();
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_max_volt
 * 函数功能：最高总电压
 * 参数个数：1
 * 参数描述：
 *          [IN]    max_vol     最高总电压
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-6-3                     李勇       创建
==============================================================*/
void set_max_volt(uint16_t max_vol)
{
    if (g_peak_record1.max_totalvol.ultimatevaule < max_vol)
    {
        g_peak_record1.max_totalvol.ultimatevaule = max_vol;
        get_timdedata(g_peak_record1.max_totalvol.time);
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_volt
 * 函数功能：设置最低电压
 * 参数个数：1
 * 参数描述：
 *          [IN]    min_vol     最低总电压
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2020-6-3                     李勇       创建
==============================================================*/
void set_min_volt(uint16_t min_vol)
{
    if (g_peak_record1.min_totalvol.ultimatevaule > min_vol)
    {
        g_peak_record1.min_totalvol.ultimatevaule = min_vol;
        get_timdedata(g_peak_record1.min_totalvol.time);
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_max_current
 * 函数功能：设置最高充电电流
 * 参数个数：1
 * 参数描述：
 *          [IN]    max_current   电流
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_max_current(int16_t max_current)
{
    if (g_peak_record1.max_current.ultimatevaule < max_current)
    {
        g_peak_record1.max_current.ultimatevaule = max_current;
        get_timdedata(g_peak_record1.max_current.time);
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_current
 * 函数功能：设置最大放电电流
 * 参数个数：1
 * 参数描述：
 *          [IN]     min_current  电流
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_min_current(int16_t min_current)
{
    if (g_peak_record1.min_current.ultimatevaule > min_current)
    {
        g_peak_record1.min_current.ultimatevaule= min_current;
        get_timdedata(g_peak_record1.min_current.time);
        needstoreflag |= E_PEAK1_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_max_celltemp
 * 函数功能：设置最高电芯温度
 * 参数个数：1
 * 参数描述：
 *          [IN]      max_celltemp  电芯温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人         修改类型
 * 2019-11-18        戴辉发         创建
==============================================================*/
void set_max_celltemp(int16_t max_celltemp)
{
    if (g_peak_record2.max_celltemp.ultimatevaule < max_celltemp)
    {
        g_peak_record2.max_celltemp.ultimatevaule = max_celltemp;
        get_timdedata(g_peak_record2.max_celltemp.time);
        g_peak_record2.max_celltemp.location = get_max_cell_temp_no();
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_celltemp
 * 函数功能：设置最低电芯温度
 * 参数个数：1
 * 参数描述：
 *          [IN]     min_celltemp 电芯温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_min_celltemp(int16_t min_celltemp)
{
    if (g_peak_record2.min_celltemp.ultimatevaule > min_celltemp)
    {
        g_peak_record2.min_celltemp.ultimatevaule = min_celltemp;
        get_timdedata(g_peak_record2.min_celltemp.time);
        g_peak_record2.min_celltemp.location = get_min_cell_temp_no();
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_max_envtemp
 * 函数功能：设置最高环境温度
 * 参数个数：1
 * 参数描述：
 *          [IN]    max_envtemp   环境温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_max_envtemp(int16_t max_envtemp)
{
    if (g_peak_record2.max_envtemp.ultimatevaule < max_envtemp)
    {
        g_peak_record2.max_envtemp.ultimatevaule = max_envtemp;
        get_timdedata(g_peak_record2.max_envtemp.time);
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_envtemp
 * 函数功能：设置最低环境温度
 * 参数个数：1
 * 参数描述：
 *          [IN]     min_envtemp  环境温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_min_envtemp(int16_t min_envtemp)
{
    if (g_peak_record2.min_envtemp.ultimatevaule > min_envtemp)
    {
        g_peak_record2.min_envtemp.ultimatevaule = min_envtemp;
        get_timdedata(g_peak_record2.min_envtemp.time);
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_max_powtemp
 * 函数功能：设置最高功率温度
 * 参数个数：1
 * 参数描述：
 *          [IN]    max_envtemp   功率温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_max_powtemp(int16_t max_powtemp)
{
    if (g_peak_record2.max_powtemp.ultimatevaule < max_powtemp)
    {
        g_peak_record2.max_powtemp.ultimatevaule = max_powtemp;
        get_timdedata(g_peak_record2.max_powtemp.time);
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_powtemp
 * 函数功能：设置最低功率温度
 * 参数个数：1
 * 参数描述：
 *          [IN]     min_powtemp  功率温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void set_min_powtemp(int16_t min_powtemp)
{
    if (g_peak_record2.min_powtemp.ultimatevaule > min_powtemp)
    {
        g_peak_record2.min_powtemp.ultimatevaule = min_powtemp;
        get_timdedata(g_peak_record2.min_powtemp.time);
        needstoreflag |= E_PEAK2_RECORD_MSG;
    }
}

/*=============================================================
 * 函数名称：set_min_powtemp
 * 函数功能：设置最低功率温度
 * 参数个数：1
 * 参数描述：
 *          [IN]     min_powtemp  功率温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void reset_peak_record()
{
    peak_record1_default();
    peak_record2_default();
    save_peak1_record();
    save_peak2_record();
}