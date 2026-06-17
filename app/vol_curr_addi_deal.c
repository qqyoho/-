/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：vol_curr_addi_deal.c
 * 文件功能描述：实现电流电压相互之间的辅助判断
 * 
 * 修改记录：
 * 2020-02-27 戴辉发   创建
*----------------------------------------------------------*/
#include "vol_curr_addi_deal.h"
#include "vol_manage.h"
#include "parameter.h"
#include "soc.h"
#include "system_adjust.h"
#include "system_control.h"
#include "protect_record.h"
#include "run_record.h"
#define MAX_SOC_VOL_TIME            10
#define MAX_SOC_WAIT_TIME           (2 * 60 * 10) /* 100mS级别 */

#define MAX_CELL_VOL                5000

static int32_t g_change_soc;
static uint32_t g_max_vol;
static uint32_t g_min_vol;

static uint8_t g_soc_afe_flag;

#if !defined (BATTARY_LFP)
    #define  SOC_CHANGE_SIZE        (40)
    #define  VOLTAGE_CHANGE_SIZE    (100 * BAT_NUM)//0.1V
#else
    #define  SOC_CHANGE_SIZE        (90)
    #define  VOLTAGE_CHANGE_SIZE    (100 * BAT_NUM)
#endif
/*=============================================================
 * 函数名称：vol_curr_addi_deal_mem_init
 * 函数功能：电压电流辅助判决
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
void vol_curr_addi_deal_mem_init(void)
{
    g_change_soc = 0;

    /* 重新初始化 */
    g_max_vol = 0;
    g_min_vol = (BAT_NUM * MAX_CELL_VOL);
    g_soc_afe_flag = 0;
}


/*=============================================================
 * 函数名称：vol_curr_addi_wakeup_mem_init
 * 函数功能：辅助判决模块唤醒初始化，每次开机参数重新初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-29        戴辉发     	创建
==============================================================*/
void vol_curr_addi_wakeup_mem_init(void)
{
    set_init_soc();
    g_max_vol = 0;
    g_min_vol = (BAT_NUM * MAX_CELL_VOL);
    if (0 != g_soc_afe_flag)
    {
        g_soc_afe_flag = 3;
    }
}

/*=============================================================
 * 函数名称：set_init_soc
 * 函数功能：设定初始SOC
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
void set_init_soc(void)
{
    g_change_soc = 0;
}

/*=============================================================
 * 函数名称：设定电压
 * 函数功能：设定初始总压
 * 参数个数：1
 * 参数描述：
 *          [IN]     total_vol  电芯电压缓冲
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
void set_cell_vol(uint32_t total_vol)
{
    if (g_max_vol < total_vol)
    {
        g_max_vol = total_vol;
    }
    if (g_min_vol > total_vol)
    {
        g_min_vol = total_vol;
    }
}

/*=============================================================
 * 函数名称：vol_curr_soc_update
 * 函数功能：辅助模块，SOC更新
 * 参数个数：curr
 * 参数描述：
 *          [IN]     ccurr       电流
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
void vol_curr_soc_update(int16_t curr)
{
    g_change_soc += curr;
}

/*=============================================================
 * 函数名称：get_curr_afe_flag
 * 函数功能：获取电流判决结果
 * 参数个数：0
 * 参数描述：
 *          
 * 返 回 值：
 *          0        无异常
 *          1        电压异常
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
uint8_t get_curr_afe_flag(void)
{
    return g_soc_afe_flag;
}

/*=============================================================
 * 函数名称：vol_curr_deal_process
 * 函数功能：电压电流互为判决处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-27        戴辉发     	创建
==============================================================*/
void vol_curr_deal_process(void)
{
#if 0
	if (judge_vol_sample_finished())
    /* 单次电压采集完成 */
    {
        int32_t change_soc;
        uint32_t rated_capcity;

        /* 电池额定电量, 单位10毫安秒 */
        rated_capcity = get_rated_capcity();
        rated_capcity *= AH_VALUE; /* 换算为10毫安S */
        
        if (g_change_soc < 0)
        {
            change_soc = 0 - g_change_soc;
        }
        else
        {
            change_soc = g_change_soc;
        }
        change_soc = (int32_t)( 100.0 * change_soc / rated_capcity );

        switch(g_soc_afe_flag)
        {
        case 0:
            /* 判决电流有效变化，而电压无效变化状态 */
            if (change_soc >= SOC_CHANGE_SIZE)
            /* 变化容量超过40% */
            {
                if ( g_max_vol < ( g_min_vol + VOLTAGE_CHANGE_SIZE ) )
                /* 最大最小总压却只差300mV以内，说明电压采样出现问题 */
                {
                   if( g_soc_afe_flag != 1 )
                   {
                     g_soc_afe_flag = 1;  
                     set_afe_volt_current();  
                     record_protect = 1;
                   }
                     
                     
                }
            }
            break;
        default:
            break;
        }
    }
#endif
}
