#include <string.h>
#include "vol_manage.h"
#include "system_adjust.h"
#include "balance.h"
#include "adc_sampling.h"
#include "parameter.h"
#include "system_control.h"
#include "current_manage.h"
#include "soc.h"
#include "soc_update.h"
#include "peak_record.h"
#include "mode_manage.h"
#include "vol_curr_addi_deal.h"
#include "hardware.h"
#include "protect_record.h"
#include "run_record.h"
#include "ch_detect.h"
#include "afe_app.h"
#include "temp_manage.h"

#define GET_VOLTAGE_SPIN_TIME      2
#define SAMPLE_STATUS_FINISHED     4
#define MAX_VOL_CHANAGE_TIME       (60 * 2)
#define MAX_VOL_WAIT_TIME          (60 * 5)

#define VOL_DIFF_DELAY            (5)
#define BAT_TOP_OFFSET             0
#define MAX_CELL_AD_COUNT          3
#define MAX_VOLTAGE_FINISHED       10
#define MAX_VOLTAGE_MIDDLE         6

/* 电压状态 */
typedef enum _E_VOLTAGE_INNER_STATUS_
{
    E_VOLTAGE_LOW_STATUS, 
    E_VOLTAGE_LOW_DELAY_STATUS, 
    E_VOLTAGE_LOW_ALARM_STATUS, 
    E_VOLTAGE_LOW_ALARM_DELAY_STATUS, 
    E_VOLTAGE_IDLE_STATUS, 
    E_VOLTAGE_HIGH_ALARM_STATUS, 
    E_VOLTAGE_HIGH_ALARM_DELAY_STATUS, 
    E_VOLTAGE_HIGH_DELAY_STATUS, 
    E_VOLTAGE_HIGH_STATUS
}e_voltage_inner_status;

static volatile uint8_t g_total_vol_time_delay;
static volatile uint8_t g_bat_vol_time_delay[BAT_NUM];
static volatile uint16_t g_vol_change_time;
static volatile uint16_t g_vol_change_recover;
static volatile uint8_t vol_afe_process_cr;
static e_voltage_inner_status g_bat_vol_status[BAT_NUM];
static e_voltage_inner_status g_total_vol_status;
static e_voltage_status g_cell_vol_status; /* 单体电压状态, 滤波后的状态 */
static e_voltage_status g_vol_status; /* 总压电压状态 */
static e_voltage_status g_voltage_flag;
static uint8_t g_vol_finished;

static uint8_t g_vol_afe_status;
static int8_t g_vol_two_comp;

static uint8_t g_vol_delay;

static uint8_t afe_fault_delay;

static void total_vol_status_process(void);
static void bat_vol_status_process(void);
uint16_t g_report_Voltage; //上报总电压 0.1V/bit
/*=============================================================
 * 函数名称：VoltageProtectTimerDelay
 * 函数功能：电压保护处理100ms延时
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-05-09        李勇     	创建
==============================================================*/
void VoltageProtectTimerDelay(void)
{
	uint8_t i;

	if (g_total_vol_time_delay) g_total_vol_time_delay --;
	for (i = 0; i < BAT_NUM; i ++)
    {
		if (g_bat_vol_time_delay[i])
        {
			g_bat_vol_time_delay[i] --;
		}
	}
}
/*=============================================================
 * 函数名称：v_timer_s_run
 * 函数功能：电压逻辑模块1s定时器
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void v_timer_s_run(void)
{
    if (g_vol_change_time) g_vol_change_time --;
    if( g_vol_delay ) g_vol_delay--;
    if( afe_fault_delay ) afe_fault_delay--;
    if( g_vol_change_recover ) g_vol_change_recover--;
    if( vol_afe_process_cr ) vol_afe_process_cr--;
}

/*=============================================================
 * 函数名称：vol_manage_init
 * 函数功能：电压逻辑模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	 创建
==============================================================*/
void vol_manage_init(void)
{
	uint8_t i;
	for(i = 0; i < BAT_NUM; i++)
    {
		g_bat_vol_status[i] = E_VOLTAGE_IDLE_STATUS;
		g_bat_vol_time_delay[i] = 0;
	}
	g_voltage_flag = E_VOL_IDLE;/* 初始化为正常状态 */
	g_total_vol_status = E_VOLTAGE_IDLE_STATUS;
	g_total_vol_time_delay = 0;
    g_vol_change_time = MAX_VOL_CHANAGE_TIME;
    g_vol_afe_status = 0;
    g_vol_delay = VOL_DIFF_DELAY; 	
    g_vol_finished = 0;
    afe_fault_delay = 0;
    g_vol_two_comp = 0;
    set_vol_buf((uint32_t )cell_get_voltage_process);
}

/*=============================================================
 * 函数名称：vol_wakeup_init
 * 函数功能：电压逻辑模块唤醒内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2019-08-09        戴辉发     	创建
==============================================================*/
void vol_wakeup_init(void)
{
    uint8_t i;
    for(i = 0; i < BAT_NUM; i++)
    {
        g_bat_vol_status[i] = E_VOLTAGE_IDLE_STATUS;
        g_bat_vol_time_delay[i] = 0;
    }
    g_total_vol_status = E_VOLTAGE_IDLE_STATUS;
    g_total_vol_time_delay = 0;
    g_cell_vol_status = E_VOL_IDLE;
    g_vol_status = E_VOL_IDLE;
    g_voltage_flag = E_VOL_IDLE;
    protect_code[0] &= ~0x0F;
    g_vol_finished = 0;
}

/*=============================================================
 * 函数名称：judge_vol_sample_finished
 * 函数功能：获取电压采样完整性标志
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           电压采样完整标志
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-09-07        戴辉发     	创建
==============================================================*/
uint8_t judge_vol_sample_finished(void)
{
    if (g_vol_finished >= SAMPLE_STATUS_FINISHED)
    {
        g_vol_finished = SAMPLE_STATUS_FINISHED;
        return 1;
    }
	return 0;
}


/*=============================================================
 * 函数名称：cell_get_voltage_process
 * 函数功能：获取流程
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void cell_get_voltage_process(uint8_t index, float ad_value)
{
	uint16_t new_voltage;
	
    /* 当前电压值 */
    new_voltage = calaclute_voltage(index, ad_value);

    g_run_sys_data.cell_vol[index] = new_voltage;

    /* 设置均衡标志 */
    if(judge_balance_status())
    {
        if(index == g_balance_index)
        {
            g_run_sys_data.cell_vol[index] |= 0x8000;
        }
        else
        {
            g_run_sys_data.cell_vol[index] &= ~0x8000;
        }
    }
    else
    {
        g_run_sys_data.cell_vol[index] &= ~0x8000;
    }
    if (index >= (BAT_NUM - 1))
    /* 本次采样完成 */
    {
        uint32_t temp; 
        temp = get_total_vol();

        if (g_vol_finished < SAMPLE_STATUS_FINISHED)
        {
            g_vol_finished += 1;
        }
        else
        {
            set_cell_vol(temp);
        }
    } 
    
    mode_sample_status_indication();
}
/*============================================================= w

 * 函数名称：vol_status_process
 * 函数功能：电压告警处理流程
 * 参数个数：4
 * 参数描述：
 *          [IN]     cell_flag   总压和电体电压标识，0：总压，1：单体电压
 *          [IN]     vol         电压值，单位mV
 *          [IN/OUT] staus       当前电压状态
 *          [IN/OUT] delay       延时
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
static void vol_status_process(uint8_t cell_flag, uint16_t vol, e_voltage_inner_status *staus, uint16_t *delay)
{
	uint16_t high_pt, high_pt_recover;
	uint16_t high_am, high_am_recover;
	uint16_t low_pt, low_pt_recover;
	uint16_t low_am, low_am_recover;
    uint16_t protect_delay;
    uint16_t high_pt_delay, high_pt_recover_delay;
    uint16_t high_am_delay, high_am_recover_delay;
    uint16_t low_pt_delay, low_pt_recover_delay;
    uint16_t low_am_delay, low_am_recover_delay;
    
#if defined(TIANFENG) && defined(BAT_8S)
    int16_t aveTemp = get_average_cell_temp();
#endif

	if (cell_flag)
	{
        
		high_pt = get_cell_vol_high_protect();
		high_pt_recover = get_cell_vol_high_recover();
		high_am = get_cell_vol_high_alarm_level();
		high_am_recover = get_cell_vol_high_alarm_recover();

        
		low_pt = get_cell_vol_low_protect();
        low_pt_recover = get_cell_vol_low_recover();
		low_am = get_cell_vol_low_alarm_level();
		low_am_recover = get_cell_vol_low_alarm_recover();
        
	}
	else
	{
		high_pt = get_vol_high_protect();
		high_pt_recover = get_vol_high_recover();
		high_am = get_vol_high_alarm_level();
		high_am_recover = get_vol_high_alarm_recover();

		low_pt = get_vol_low_protect();
        low_pt_recover = get_vol_low_recover();
		low_am = get_vol_low_alarm_level();
		low_am_recover = get_vol_low_alarm_recover();
	}
    protect_delay = get_vol_protect_delay();
    if (protect_delay >= 50)
    {
        protect_delay = 50;
    }
    high_pt_delay = protect_delay;
    high_pt_recover_delay = protect_delay;
    high_am_delay = protect_delay;
    high_am_recover_delay = protect_delay;
    low_pt_delay = protect_delay;
    low_pt_recover_delay = protect_delay;
    low_am_delay = protect_delay;
    low_am_recover_delay = protect_delay;

#if defined(TIANFENG) && defined(BAT_8S)
    if(E_CHARGE_STATUS != get_system_status())
    {
        if(cell_flag)
        {
            high_am = 3800;
            high_am_recover = 3330;
            high_pt = 3800;
            high_pt_recover = 3700;
            if(aveTemp < 0)
            {
                low_am = 2400;
                low_am_recover = 3000;
                low_pt = 2000;
                low_pt_recover = 2800;
            }
        }
        else
        {
            high_am = 380 * BAT_NUM;
            high_am_recover = 333 * BAT_NUM;
            high_pt = 380 * BAT_NUM;
            high_pt_recover = 333 * BAT_NUM;
            if(aveTemp < 0)
            {
                low_am = 1920;
                low_am_recover = 2400;
                low_pt = 1600;
                low_pt_recover = 2240;
            }
        }
        high_pt_delay = 30;
        high_pt_recover_delay = 30;
    }
    else
    {
        if(cell_flag)
        {
            low_am = 2000;
            low_am_recover = 2700;
            low_pt = 1800;
            low_pt_recover = 2200;
        }
        else
        {
            low_am = 200 * BAT_NUM;
            low_am_recover = 270 * BAT_NUM;
            low_pt = 180 * BAT_NUM;
            low_pt_recover = 230 * BAT_NUM;
            high_am_delay = 30;
            high_am_recover_delay = 30;
        }
        high_pt_delay = 30;
        high_pt_recover_delay = 10;
        low_pt_delay = 30;
        low_pt_recover_delay = 30;
    }
#endif

	switch(*staus)
	{
	case E_VOLTAGE_IDLE_STATUS:
        if (vol <= low_am)
        {
            *staus = E_VOLTAGE_LOW_ALARM_DELAY_STATUS;
            *delay = low_am_delay;
        }
		else if (vol >= high_am)
		{
            *staus = E_VOLTAGE_HIGH_ALARM_DELAY_STATUS;
            *delay = high_am_delay;
		}
        else
        {
            *delay = 0;
        }
		break;
    case E_VOLTAGE_HIGH_ALARM_DELAY_STATUS:
        if (vol >= high_pt)
        
        { 
            *staus = E_VOLTAGE_HIGH_DELAY_STATUS;
            *delay = high_pt_delay;
        }
		else if (vol >= high_am)
        
		{
            if (0 == *delay)
            {
                *staus = E_VOLTAGE_HIGH_ALARM_STATUS;
                *delay = high_am_recover_delay;
            }
		}
        else
        
        {
            *staus = E_VOLTAGE_IDLE_STATUS;
            *delay = 0;
        }
        break;
	case E_VOLTAGE_HIGH_ALARM_STATUS:
        if (vol >= high_pt)
        
        {
            *staus = E_VOLTAGE_HIGH_DELAY_STATUS;
            *delay = high_pt_delay;
        }
		else if (vol < high_am_recover)
		{
            if (0 == *delay)
            {
			    *staus = E_VOLTAGE_IDLE_STATUS;
            }
		}
        else
        {
            *delay = high_am_recover_delay;
        }
		break;
	case E_VOLTAGE_HIGH_DELAY_STATUS:
		if (vol < high_pt)
		{
            *staus = E_VOLTAGE_HIGH_ALARM_STATUS;
			*delay = high_am_recover_delay;
		}
		else
		{
            if (0 == *delay)
            {
                *staus = E_VOLTAGE_HIGH_STATUS;
                *delay = high_pt_recover_delay;
            }
		}
		break;
	case E_VOLTAGE_HIGH_STATUS:
#if defined(TIANFENG) && defined(BAT_8S)
        *delay = high_pt_recover_delay;
#else
        if (vol < high_pt_recover)
        {
            if (0 == *delay)
            {
                *staus = E_VOLTAGE_HIGH_ALARM_STATUS;
                *delay = high_am_recover_delay;
            }
        }
        else
        {
            *delay = high_pt_recover_delay;
        }
#endif
		break;
	case E_VOLTAGE_LOW_ALARM_DELAY_STATUS:
		if (vol <= low_pt)
		{
            *staus = E_VOLTAGE_LOW_DELAY_STATUS;
            *delay = low_pt_delay;
		}
        else if (vol <= low_am)
        {
            if (0 == *delay)
            {
                *staus = E_VOLTAGE_LOW_ALARM_STATUS;
                *delay = low_am_recover_delay;
            }
        }
		else
		{
			*staus = E_VOLTAGE_IDLE_STATUS;
            *delay = 0;
		}
		break;
	case E_VOLTAGE_LOW_ALARM_STATUS:
		if (vol <= low_pt)
		{
            *staus = E_VOLTAGE_LOW_DELAY_STATUS;
            *delay = low_pt_delay;
		}
        else if (vol > low_am_recover)
        {
            if (0 == *delay)
            {
                *staus = E_VOLTAGE_IDLE_STATUS;
            }
        }
        else
        {
            *delay = low_am_recover_delay;
        }
		break;
	case E_VOLTAGE_LOW_DELAY_STATUS:
		if (vol <= low_pt)
		{
			if (0 == *delay)
			{
				*staus = E_VOLTAGE_LOW_STATUS;
                *delay = low_pt_recover_delay;
			}
		}
		else
		{
			*staus = E_VOLTAGE_LOW_ALARM_STATUS;
            *delay = low_am_recover_delay;
		}
		break;
	case E_VOLTAGE_LOW_STATUS:
#if defined(TIANFENG) && defined(BAT_8S)
        *delay = low_pt_recover_delay;
#else
		if (vol > low_pt_recover)
		{
			if ( 0 == *delay )
			{
				*staus = E_VOLTAGE_LOW_ALARM_STATUS;
                *delay = low_am_recover_delay;
			}
		}
        else
        {
            *delay = low_pt_recover_delay;
        }
#endif
		break;
	default:
		*staus = E_VOLTAGE_IDLE_STATUS;
		break;
	}
}

/*=============================================================
 * 函数名称：bat_vol_status_process
 * 函数功能：单体电芯电压处理流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
static void bat_vol_status_process(void)
{
	uint16_t temp;
	uint8_t i;
	uint16_t cell_timer;

	for (i = 0; i < BAT_NUM; i ++)
    {
		temp = g_run_sys_data.cell_vol[i]&0x7fff;
        cell_timer = g_bat_vol_time_delay[i];
        vol_status_process(1, temp, &g_bat_vol_status[i], &cell_timer);
        g_bat_vol_time_delay[i] = cell_timer;
	}
}

/*=============================================================
 * 函数名称：total_vol_status_process
 * 函数功能：总压处理流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void total_vol_status_process(void)
{
	uint16_t total_timer;
    uint32_t  total_vol = get_total_vol();    
    //获取上报总电压 0.1V/bit
    g_report_Voltage = total_vol/100;
    g_run_sys_data.total_vol =  total_vol/10;   
	total_timer = g_total_vol_time_delay;
    vol_status_process(0, total_vol/10, &g_total_vol_status, &total_timer);
    set_max_volt(total_vol);
    set_min_volt(total_vol);
	g_total_vol_time_delay = total_timer;
}
/*=============================================================
 * 函数名称：vol_manage_process
 * 函数功能：电压管理流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void vol_manage_process(void)
{
	if ( judge_vol_sample_finished() )
    /* 单次电压采集完成 */
    {
		uint8_t i;
		uint8_t over_alarm_flag = 0;
		uint8_t over_flag = 0;
		uint8_t under_alarm_flag = 0;
		uint8_t under_flag = 0;
		e_voltage_inner_status cell_status;
        

        set_min_cell(get_min_cell_vol());
        set_max_cell(get_max_cell_vol());

		/* 单体电压处理流程 */
		bat_vol_status_process();
		/* 总压处理流程 */
		total_vol_status_process();

        
        
		for (i = 0; i < BAT_NUM; i ++)
		{
			cell_status = g_bat_vol_status[i];
			if(g_bat_vol_status[i] == E_VOLTAGE_HIGH_STATUS)
			/* 单体过压保护 */
			{
				over_flag = 1; 
			}
			else if(cell_status == E_VOLTAGE_HIGH_ALARM_STATUS || cell_status == E_VOLTAGE_HIGH_DELAY_STATUS)
			{
				over_alarm_flag = 1;
			}
			else if(cell_status == E_VOLTAGE_LOW_ALARM_STATUS || cell_status == E_VOLTAGE_LOW_DELAY_STATUS)
			{
				under_alarm_flag = 1;
			}
			else if(cell_status == E_VOLTAGE_LOW_STATUS)
			/* 单体欠压保护 */
			{
				under_flag = 1;
			}
		}
       

				
        /* 单体电芯电压状态 */
        if (over_flag == 1)
        {
            g_cell_vol_status = E_VOL_OVER;
        }
        else if (under_flag == 1)
        {  
            g_cell_vol_status = E_VOL_UNDER;
        }
        else if (1 == over_alarm_flag)
        {
            g_cell_vol_status = E_VOL_OVER_ALARM;
        }
        else if (1 == under_alarm_flag)
        {
            g_cell_vol_status = E_VOL_UNDER_ALARM;
        }			
        else
        {
            g_cell_vol_status = E_VOL_IDLE;
        }
                
        /* 总电压状态 */
        if (E_VOLTAGE_HIGH_STATUS == g_total_vol_status)
        {             
            g_vol_status = E_VOL_OVER;
        }
        else if (E_VOLTAGE_LOW_STATUS == g_total_vol_status)
        {    
            g_vol_status = E_VOL_UNDER;
        }
        else if ((E_VOLTAGE_HIGH_ALARM_STATUS == g_total_vol_status) || 
            (E_VOLTAGE_HIGH_DELAY_STATUS == g_total_vol_status))
        {
            g_vol_status = E_VOL_OVER_ALARM;
        }
        else if ((E_VOLTAGE_LOW_ALARM_STATUS == g_total_vol_status) || 
            (E_VOLTAGE_LOW_DELAY_STATUS == g_total_vol_status))
        {
            g_vol_status = E_VOL_UNDER_ALARM;
        }
        else
        {
            g_vol_status = E_VOL_IDLE;
        }
				
                
        /* 判别整个电压状态 */
        if((1 == over_flag) || (E_VOLTAGE_HIGH_STATUS == g_total_vol_status))
        {
            if( g_voltage_flag != E_VOL_OVER )
            {
              g_voltage_flag = E_VOL_OVER;
              if( E_CHARGE_STATUS == get_system_status() ) 
              set_cell_ov_count();
              record_protect = 1; 
            }				
        }
        else if((1 == under_flag) || (E_VOLTAGE_LOW_STATUS == g_total_vol_status))
        {
            if( g_voltage_flag != E_VOL_UNDER )
            {
              g_voltage_flag = E_VOL_UNDER;
              if( E_CHARGE_STATUS != get_system_status() ) 
              set_cell_uv_count();    
              record_protect = 1; 
            }
        }
        else if((1 == over_alarm_flag) || 
            (E_VOLTAGE_HIGH_ALARM_STATUS == g_total_vol_status) || 
            (E_VOLTAGE_HIGH_DELAY_STATUS == g_total_vol_status))
        {
            g_voltage_flag = E_VOL_OVER_ALARM;
        }
        else if((1 == under_alarm_flag) || 
            (E_VOLTAGE_LOW_ALARM_STATUS == g_total_vol_status) || 
            (E_VOLTAGE_LOW_DELAY_STATUS == g_total_vol_status))
        {
            g_voltage_flag = E_VOL_UNDER_ALARM;
        }
        else
        {
            g_voltage_flag = E_VOL_IDLE;
        }
                
			
        
        
        if( g_cell_vol_status == E_VOL_UNDER )
          protect_code[0] |= 0x01;
        else
          protect_code[0] &= ~0x01;
        
        if( g_cell_vol_status == E_VOL_OVER )
          protect_code[0] |= 0x02;
        else
          protect_code[0] &= ~0x02;
        
        if( g_vol_status == E_VOL_UNDER )
          protect_code[0] |= 0x04;
        else
          protect_code[0] &= ~0x04;
        
        if( g_vol_status == E_VOL_OVER )
          protect_code[0] |= 0x08;
        else
          protect_code[0] &= ~0x08;
                
		control_voltage_indication();
	}
}

/*=============================================================
 * 函数名称：get_vol_status
 * 函数功能：获取电压状态
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           电压状态
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-25        戴辉发     	创建
==============================================================*/
e_voltage_status get_vol_status(void)
{
	return g_voltage_flag;
}

/*=============================================================
 * 函数名称：get_cell_vol_status
 * 函数功能：获取单体电压状态
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           单体电压状态
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-25        戴辉发     	创建
==============================================================*/
e_voltage_status get_cell_vol_status(void)
{
	return g_cell_vol_status;
}

/*=============================================================
 * 函数名称：get_total_vol_status
 * 函数功能：获取单体电压状态
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           总压状态
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-25        戴辉发     	创建
==============================================================*/
e_voltage_status get_total_vol_status(void)
{
	return g_vol_status;
}

/*=============================================================
 * 函数名称：get_max_cell_vol
 * 函数功能：获取最高电压
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
uint16_t get_max_cell_vol(void)
{
	uint8_t i = 0;
	uint16_t ret,temp;

	ret = g_run_sys_data.cell_vol[0]&0x7fff;
	for(i = 1; i < BAT_NUM; i ++)
	{
       temp = g_run_sys_data.cell_vol[i]&0x7fff;
		if( ret < temp )
		{
			ret = temp;
		}
	}
	return ret;
}

/*=============================================================
 * 函数名称：get_min_cell_vol
 * 函数功能：获取最低电压
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
uint16_t get_min_cell_vol(void)
{
	uint8_t i = 0;
	uint16_t ret,temp;

	ret = g_run_sys_data.cell_vol[0]&0x7fff;

	for(i = 1; i < BAT_NUM; i ++)
	{
       temp = g_run_sys_data.cell_vol[i]&0x7fff;
		if(ret > temp)
		{
			ret = temp;
		}
	}
	return ret;
}

/*=============================================================
 * 函数名称：get_average_vol
 * 函数功能：获取平均电压
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
uint16_t get_average_vol(void)
{
	uint8_t i = 0;
	long tmp = 0;

	for(i = 0; i < BAT_NUM; i ++)
	{
		tmp += g_run_sys_data.cell_vol[i] & 0x7fff;
	}
	tmp = tmp / BAT_NUM;

	return (uint16_t)tmp;
}



/*=============================================================
 * 函数名称：get_max_cell_index
 * 函数功能：找到最高电压电芯位置
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最高电压下标
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2019-08-17         戴辉发      创建
==============================================================*/
uint8_t get_max_cell_index(void)
{
	uint8_t tmp_max = 0;
	uint8_t i;
    uint16_t max,temp;

    max = g_run_sys_data.cell_vol[0]&0x7fff;
	for( i = 1; i < BAT_NUM; i ++ )
	{
        temp = g_run_sys_data.cell_vol[i]&0x7fff;
		if( temp  >  max )
		{
            max = temp;
			tmp_max = i;
		}
	}
	return tmp_max;
}

/*=============================================================
 * 函数名称：get_min_cell_index
 * 函数功能：找到最低电压电芯位置
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最低电压下标
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2019-08-17         戴辉发      创建
==============================================================*/
uint8_t get_min_cell_index(void)
{
	uint8_t tmp_min = 0;
	uint8_t i;
    uint16_t min,temp;

    min = g_run_sys_data.cell_vol[0]&0x7fff;
	for( i = 1; i < BAT_NUM; i ++ )
	{
        temp = g_run_sys_data.cell_vol[i]&0x7fff;
		if( temp  <  min )
		{
            min = temp;
			tmp_min = i;
		}
	}
	return tmp_min;
}

/*=============================================================
 * 函数名称：get_total_vol
 * 函数功能：找到高电压位置
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           总压，单位10毫伏
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2018-07-07         戴辉发      创建
==============================================================*/
uint32_t get_total_vol(void)
{
	uint32_t t_vol = 0,temp;
	uint8_t i;

	for (i = 0; i < BAT_NUM; i ++)
	{
        temp = g_run_sys_data.cell_vol[i]&0x7fff;
		t_vol += temp;
	}

	return t_vol;
}



/*=============================================================
 * 函数名称：judge_low_voltage
 * 函数功能：判断欠压保护状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0         电压正常
 *          1         欠压保护状态
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2018-07-07         戴辉发      创建
==============================================================*/
uint16_t judge_low_voltage(void)
{
	if (g_voltage_flag == E_VOL_UNDER)
		return 1;
	return 0;
}

/*=============================================================
 * 函数名称：get_vol_afe_status
 * 函数功能：获取判决依据为电压的AFE状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0         AFE正常
 *          1         AFE异常
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2020-02-01         戴辉发      创建
==============================================================*/
uint8_t get_vol_afe_status(void)
{
    return g_vol_afe_status;
}

/*=============================================================
 * 函数名称：get_vol_two_comp
 * 函数功能：获取AFE和MCU检查的总压比较结果
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0         正常
 *          1         异常
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2020-06-01         liy      创建
==============================================================*/
uint8_t get_vol_two_comp(void)
{
    return g_vol_two_comp;
}

                  
/*=============================================================
 * 函数名称：vol_afe_fault_process
 * 函数功能：电压判决AFE故障处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2020-02-21         戴辉发      创建
==============================================================*/
void vol_afe_fault_process(void)
{
    static uint16_t cell_vol[BAT_NUM];/* 电芯电压 */
    uint8_t i,flag=0;
    uint16_t calc_total;
    uint16_t moni_total;
    moni_total = GetTotalVoltage();
    calc_total = get_total_vol()/10;

    if( vol_afe_process_cr ) return;
       vol_afe_process_cr  = 1;
        
    for ( i = 0; i < BAT_NUM; i ++ )
    {
      
        if ( cell_vol[i] != g_run_sys_data.cell_vol[i] )
        {
           g_vol_change_time = MAX_VOL_CHANAGE_TIME;        
           flag=1;
        }
        
        cell_vol[i] = g_run_sys_data.cell_vol[i];
    }

    switch( g_vol_afe_status )
    {
    case 0:
      
        if ( 0 == g_vol_change_time )
        {
            g_vol_afe_status = 1;
            g_vol_change_time = MAX_VOL_WAIT_TIME;
        }
        
        break;     
    case 1: 
        if( flag != 0 )
        {
            g_vol_afe_status = 0;   
        }
        else if ( 0 == g_vol_change_time )
        {
            g_vol_afe_status = 2;
            g_vol_change_recover = 20;
            set_afe_volt_stable();
            record_protect = 1;
        }
        
        break;      
    default:
        if( flag != 0 )
        {
            if( g_vol_change_recover == 0 ) 
            {
               g_vol_afe_status = 0;
            }
        }      
        
        break;
    }
    
    
    /* 电压处理流程 */
    if ((moni_total > calc_total + 200) || ((moni_total + 200) < calc_total )||( g_run_sys_data.total_vol+200 < moni_total )||( g_run_sys_data.total_vol > moni_total + 200 ))
    {//误差超过1v
         if(( g_vol_delay == 0 )||( g_vol_two_comp == -1 ))
         {
            if( g_vol_two_comp != 1 )
            {
              g_vol_two_comp = 1;
              set_afe_volt_differe();
              record_protect = 1;
            }
            
         }
    }
    else
    {
        if(  g_vol_two_comp == 1 )
        {//设置恢复启动 
            g_vol_two_comp = -1;
            g_vol_delay = 2*VOL_DIFF_DELAY;
        }
        else if(  g_vol_two_comp == -1 )
        {//等待恢复完成延时
           if( g_vol_delay == 0 )
           {
              g_vol_two_comp = 0;
              g_vol_delay = VOL_DIFF_DELAY;
           }
        }
        else
        {//无对比故障，反复设置延时计数器
           g_vol_delay = VOL_DIFF_DELAY;
        }
    }
    

    control_AFE_indication();
    
    if( 1 == get_afe_status() )
    {
       protect_code[5] |= 0x01;
    }
    else
    {
       protect_code[5] &= ~0x01;
    }
    
    if( 0 != g_vol_two_comp )
    {
       protect_code[5] |= 0x02;
    }
    else
    {
       protect_code[5] &= ~0x02;
    }
    
    if( 2 == g_vol_afe_status )
    {
       protect_code[5] |= 0x04;
    }
    else
    {
       protect_code[5] &= ~0x04;
    }
    
    if( 0 != get_curr_afe_flag())
    {
       protect_code[5] |= 0x08;
    }
    else
    {
       protect_code[5] &= ~0x08;
    }
}
/*=============================================================
 * 函数名称：set_max_cell_over_voltage
 * 函数功能：设置 最高电压电芯 为过压状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          
 * 修改记录：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2021-05-10          liyong      创建
==============================================================*/
void set_max_cell_over_voltage(void)
{
    uint8_t i = get_max_cell_index();

    g_bat_vol_status[i] = E_VOLTAGE_HIGH_STATUS;
    g_bat_vol_time_delay[i] =  get_vol_protect_delay();
}

/*=============================================================
 * 函数名称：GetReportVol
 * 函数功能：获取当前问题最严重的电压
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *==============================================================
 * 日    期           修改人      修改类型
 * 2024-07-15         Fanyl      创建
==============================================================*/
uint16_t GetReportVol(void)
{
    uint16_t ret = 0;
#if defined(BATTARY_LFP)
    uint16_t temp = 3200;
#endif
    uint16_t maxVol = get_max_cell_vol();
    uint16_t minVol = get_min_cell_vol();
    
    if(minVol > temp)
    {
        ret = maxVol;
    }
    else if(maxVol <= temp)
    {
        ret = minVol;
    }
    else
    {
        ret = maxVol;
        if((minVol <= (get_cell_vol_low_protect() + 300)) && (maxVol <(get_cell_vol_high_protect() - 200)))
        {
            ret = minVol;
        }
    }
    
    return ret;
}
