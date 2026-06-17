/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：soc_update.c
* 文件功能描述：实现SOC容量更新
*
* 修改记录：
* 2018-10-15 戴辉发 创建
*----------------------------------------------------------*/
#include "soc_update.h"
#include "string.h"
#include "storage_manage.h"
#include "vol_manage.h"
#include "current_manage.h"
#include "parameter.h"
#include "soc.h"
#include "temp_manage.h"

static uint32_t g_dch_capcity; /* 10mAS */
static uint8_t g_real_flag; /* 开始记录放电容量标识 */
static uint8_t g_real_main_flag; /* 存储实时扇区标志 */

/*=============================================================
 * 函数名称：soc_update_mem_init
 * 函数功能：容量更新模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-10-15       	戴辉发     	创建
==============================================================*/
void soc_update_mem_init(void)
{
    g_dch_capcity = 0;
}

/*=============================================================
 * 函数名称：soc_update_mem_init
 * 函数功能：容量更新模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *          
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-02-24        戴辉发      创建
==============================================================*/
uint32_t get_real_capcity(void)
{
    return g_dch_circle.total;
}

/*=============================================================
 * 函数名称：soc_update_dch_capcity
 * 函数功能：SOC更新模块电流积分
 * 参数个数：1
 * 函数参数：
 *           [IN]      current            电流，只在放电时有效
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-02-23          戴辉发             创建
==============================================================*/
void soc_update_dch_capcity(int16_t current)
{
    if ( 1 == g_real_flag )
    {
        g_dch_capcity += current;
    }
}

/*=============================================================
 * 函数名称：soc_update_dch_capcity_statrt
 * 函数功能：启动SOC更新模块电流积分
 * 参数个数：1
 * 函数参数：
 * 返回值：  
 *          无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-02-23          戴辉发             创建
==============================================================*/
void soc_update_dch_capcity_statrt(void)
{
    if (0 == g_real_flag)
    {
        g_real_flag = 1;
        g_dch_capcity = 0;
    }
}
/*=============================================================
 * 函数名称：capacity_data_default
 * 函数功能：容量累计次数数据更新
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期           修改人      修改类型
 * 2020-10-13         liyong      创建
==============================================================*/
void capacity_data_default(void)
{
     uint32_t rated_capcity;
      /* 电池额定电量, 单位10毫安秒 */
     rated_capcity = get_rated_capcity();
     rated_capcity *= AH_VALUE; /* 换算为10毫安S */
     g_dch_circle.total = rated_capcity;
     g_dch_circle.charge_num.conut = 0;
     g_dch_circle.circle_num.conut = 0;
     g_dch_circle.number = MAX_STORAGE_NUM;
     g_real_main_flag = 1;
}
/*=============================================================
 * 函数名称：soc_update_hard_init
 * 函数功能：SOC更新硬件初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期           修改人      修改类型
 * 2018-10-16         戴辉发      创建
==============================================================*/
void soc_update_hard_init(void)
{
	short flag = 0;
	t_dch_circle_st real_capcity;
    uint32_t rated_capcity;

    /* 电池额定电量, 单位10毫安秒 */
    rated_capcity = get_rated_capcity();
    rated_capcity *= AH_VALUE; /* 换算为10毫安S */

	flag = read_data_from_storage(SOC_CAPCITY_MAIN_ADDRESS, (uint8_t *)&g_dch_circle, sizeof(t_dch_circle_st));
    if( flag == sizeof(t_dch_circle_st) )
    {     
        flag = read_data_from_storage(SOC_CAPCITY_BACK_ADDRESS, (uint8_t *)&real_capcity, sizeof(t_dch_circle_st));
        if( flag == sizeof(t_dch_circle_st) )
        {
          flag = judge_new_sector(g_dch_circle.number, real_capcity.number);
        }
    }

	if (flag == -1)
    /* 该参数写过 */
	{
		g_real_main_flag = 1;
		memcpy(&g_dch_circle, &real_capcity, sizeof(real_capcity));
	}
	else if (flag == 0)
    /* 从未写过 */
	{
       capacity_data_default();
	}
	else
    /* 该参数写过 */
	{
		g_real_main_flag = 0;
	}
    
   
    float fullCapacity = rated_capcity;

    if( g_dch_circle.circle_num.conut >= MAXIMUM_CYCLES )
    {
         fullCapacity = fullCapacity*0.8;
    }
    else
    {
         fullCapacity -= 0.2*g_dch_circle.circle_num.conut/MAXIMUM_CYCLES*fullCapacity;
    }
    
    g_dch_circle.total = (uint32_t)(fullCapacity+0.5);

    if( g_dch_circle.total > rated_capcity )
    {
        g_dch_circle.total = rated_capcity;
    }
    else if( g_dch_circle.total < ( 0.8 * rated_capcity ) )
    {
        g_dch_circle.total = (uint32_t)(0.8 * rated_capcity);
    }
}

/*=============================================================
 * 函数名称：write_capcity_para
 * 函数功能：写充放电循环等参数
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期           修改人       修改类型
 * 2018-07-06         戴辉发       创建
==============================================================*/
 void write_capcity_para(void)
{
	uint16_t address;
	t_write_data_st data;

	g_dch_circle.number = sector_num_next(g_dch_circle.number);
	if (g_real_main_flag)
	{
		g_real_main_flag = 0;
		address = SOC_CAPCITY_MAIN_ADDRESS;
	}
	else
	{
		g_real_main_flag = 1;
		address = SOC_CAPCITY_BACK_ADDRESS;
	}

	data.msg_type = E_SOC_CIRCLE_PARA_MSG;
	data.write_len = sizeof(g_dch_circle);
	data.write_memery_address = (uint8_t *)&g_dch_circle;
	data.write_storge_address = address;
	set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：capacity_data_clear
 * 函数功能：容量累计次数复位
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期           修改人      修改类型
 * 2020-10-13         liyong      创建
==============================================================*/
void capacity_data_clear(void)
{
    capacity_data_default();
    write_capcity_para();
}
/*=============================================================
 * 函数名称：soc_update_dch_capcity_end
 * 函数功能：完成SOC更新模块电流积分
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-02-23          戴辉发             创建
==============================================================*/
void soc_update_dch_capcity_end(void)
{
    if (1 == g_real_flag)
    {
        g_real_flag = 0;

#if !defined (BATTARY_LFP)
        if (get_min_cell_temp() >= 150)
#else
        if (get_min_cell_temp() >= 200)
#endif
        {
            uint32_t total_capcity = get_total_capcity();

            if ((( g_dch_capcity  < ( 0.95 * total_capcity ) ) && (g_dch_capcity > (total_capcity >> 1))) || 
              (( g_dch_capcity  > ( 1.05 * total_capcity ) ) && (g_dch_capcity < (total_capcity * 1.5))))
            /*if((g_dch_capcity > (0.8 * total_capcity)) && (g_dch_capcity < (1.5 * total_capcity))*/
            /* 需要更新当前总容量 */
            {
                uint32_t temp;

                temp = (g_dch_capcity / AH_VALUE) * AH_VALUE;
                if ((g_dch_capcity - temp) >= (AH_VALUE / 2))
                {
                    g_dch_capcity = temp + AH_VALUE;
                }
                else
                {
                    g_dch_capcity = temp;
                }
                g_dch_circle.total = g_dch_capcity;         
                set_total_capcity(g_dch_capcity);               
                needstoreflag |= E_SOC_CIRCLE_PARA_MSG;
            }
        }
        g_dch_capcity = 0;
    }
}

/*=============================================================
 * 函数名称：soc_update_dch_capcity_stop
 * 函数功能：停止SOC更新模块电流积分
 * 参数个数：1
 * 函数参数：
 * 返回值：  
 *          无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-02-23          戴辉发             创建
==============================================================*/
void soc_update_dch_capcity_stop(void)
{
    g_real_flag = 0;
    g_dch_capcity = 0;
}

/*=============================================================
 * 函数名称：soc_update_capcity_config
 * 函数功能：参数配置的容量更新
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *          无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-02-24          戴辉发             创建
==============================================================*/
void soc_update_capcity_config(void)
{
    uint32_t rated_capcity;

    /* 电池额定电量, 单位10毫安秒 */
    rated_capcity = get_rated_capcity();
    rated_capcity *= AH_VALUE; /* 换算为10毫安S */
    g_real_flag = 0;
    g_dch_capcity = 0;
}
