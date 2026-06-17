/*
 * final_protect.c
 *
 *  Created on: 2019年7月3日
 *      Author: daihuifa
 */
#include "final_protect.h"
#include "vol_manage.h"
#include "parameter.h"
#include "switch_status.h"
#include "soc.h"
#include "afe_app.h"
#include "protect_record.h"
#include "run_record.h"
#include "soc_update.h"
#include "system_control.h"

#define TIMER_1S              100

t_system_protect_data g_system_protect;

static volatile uint16_t final_high_timer;
static volatile uint16_t final_low_timer;

/*=============================================================
* 函数名称：final_protect_mem_init
* 参数功能：系统终极保护判决模块
* 参数个数：0
* 函数参数：
*
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2019-07-03          戴辉发             创建
==============================================================*/
void final_protect_mem_init(void)
{
	final_high_timer = 0;
	final_low_timer = 0;
    g_system_protect.vol_high_status = 0;
    g_system_protect.vol_low_status = 0;
#if defined ( BATTARY_LFP )
    g_system_protect.vol_high_protect = 3900;
    g_system_protect.vol_high_recover = 3400;
#else
    g_system_protect.vol_high_protect = 4280;
    g_system_protect.vol_high_recover = 4200;
#endif
    g_system_protect.vol_low_protect = get_vol_dis_charge_level();
    g_system_protect.vol_low_recover = get_vol_dis_charge_level()+200;
   
}
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
 * 日期                修改人             修改内容
 * 2019-07-03          戴辉发             创建
==============================================================*/
void final_protect_timer(void)
{
	if (final_high_timer) final_high_timer --;
	if (final_low_timer) final_low_timer --;
}

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
 * 日期                修改人             修改内容
 * 2019-01-17          戴辉发             创建
==============================================================*/
void system_final_protect(void)
{
    afe_final_protect();
	if (judge_vol_sample_finished())
	{
		uint16_t max_vol = get_max_cell_vol();
		uint16_t min_vol = get_min_cell_vol();
        uint16_t flag;

        g_system_protect.vol_low_protect = get_vol_dis_charge_level();
        g_system_protect.vol_low_recover = get_vol_dis_charge_level() + 200;
		switch (g_system_protect.vol_high_status)
		{
		case 0:
			if (max_vol >= g_system_protect.vol_high_protect)
			{
				g_system_protect.vol_high_status = 1;
				final_high_timer = (TIMER_1S * 3);
			}
			break;
		case 1:
			if (max_vol >= g_system_protect.vol_high_protect)
			{
                if (0 == final_high_timer)
                {
                    g_system_protect.vol_high_status = 2;
                    set_final_ov();
                    record_protect = 1;
                }
			}
            else
            {
                g_system_protect.vol_high_status = 0;
            }
			break;
		default:
			if (max_vol < g_system_protect.vol_high_recover)
			{
				g_system_protect.vol_high_status = 0;
			}
			break;
		}

		switch (g_system_protect.vol_low_status)
		{
		case 0:
			if (min_vol <= g_system_protect.vol_low_protect)
			{
				g_system_protect.vol_low_status = 1;
				final_low_timer = (TIMER_1S * 3);
			}
			break;
		case 1:
			if (min_vol <= g_system_protect.vol_low_protect)
			{
                if (0 == final_low_timer)
                {
                    g_system_protect.vol_low_status = 2;
                    set_final_uv();
                    record_protect = 1;
                }
			}
            else
            {
                g_system_protect.vol_low_status = 0;
            }
			break;
		default:
			if (min_vol > g_system_protect.vol_low_recover)
			{
				g_system_protect.vol_low_status = 0;
			}
			break;
		}

        flag = get_final_set_switch();
        if (2 == g_system_protect.vol_high_status)
        /* 单体过压 */
        {
            flag |= 0x01;
            //soc_update_dch_capcity_statrt();
            protect_code[0] |= 0x20;
        }
        else
        {
            flag &= (~0x01);
            protect_code[0] &= ~0x20;
        }

		if (2 == g_system_protect.vol_low_status)
        /* 单体欠压 */
        {
            flag |= 0x02;
            /* 容量更新结束标识 */
            //soc_update_dch_capcity_end();
            protect_code[0] |= 0x10;
        }
        else
        {
            flag &= (~0x02);
            protect_code[0] &= ~0x10;
        }
        /*0x01  软件终极过压  0x02 软件终极欠压 */
		final_set_switch(flag);  
	}
    
    control_final_indication();
}
