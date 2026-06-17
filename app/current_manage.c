/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：current_manage.c
* 文件功能描述：实现电流管理
*
* 修改记录：
* 2018-06-27 戴辉发 创建
*----------------------------------------------------------*/

#include "current_manage.h"
#include "adc_sampling.h"
#include "hardware.h"
#include "system_adjust.h"
#include "parameter.h"
#include "ch_detect.h"
#include "soc.h"
#include "system_control.h"
#include "switch_status.h"
#include "peak_record.h"
#include "protect_record.h"
#include "run_record.h"
#include "ch_addition.h"
#include "afe_app.h"
#include "temp_manage.h"

#define MAX_AVL_COUNT                (5)
#define CURRENT_POWERON              (6)

#define TIMER_TIMES                  (5)
#define TIMER_INVALID                (0xffff)

#define CURRENT_TIMES                (10)

#define TIMER_1S_TIMES               (1000L / TIMER_TIMES / CURRENT_TIMES)
#define TIMER_250MS_TIMES            (250L / TIMER_TIMES / CURRENT_TIMES)

#define CURRENT_FAULT_TIMER          2
#define CH_CURRENT_WAIT_TIMER        0
#define CH_LOWTEMP_CURR_DISCH_TIME   100 /* 5S */

#define CURRENT_STABLE_TIMER         (2000 / TIMER_TIMES)

#define CURRENT_POWER_DELAY          (600)
#define SECOND_TO_MSECOND            (200)

#define MIN_CURRENT_VALUE            (-32500)
#define MAX_CURRENT_VALUE            (32500)

#if defined(TIANFENG) && defined(BAT_8S)
#define DCH_SMALL_CURRENT_LEVEL      (-10)
#else
#define DCH_SMALL_CURRENT_LEVEL      (-50)
#endif

#define CH_SMALL_CURRENT_LEVEL       (10)
#define DCH_LOW_CURRENT_LEVEL        (-100)
#define DCH_LOWPROTECT_CURRENT_LEVEL (-500)

/* 电流状态 */
typedef enum _E_CURR_ITEM_
{
    E_FLT_IDLE, /* 检测失效状态 */
	E_SECOND,   /* 放电二次过流状态 */
	E_DISCH_OVER, /* 放电过流状态 */
	E_DISCH_ALARM, /* 放电过流告警状态 */
	E_DISCH, /* 放电电流状态 */
	E_IDLE, /* 待机电流状态 */
	E_CH, /* 充电过流状态 */
    E_CH_ALARM, /* 充电过流告警 */
    E_CH_OVER, /* 充电电流状态 */
}e_curr_item;


static volatile uint8_t c_process_delay;
static int16_t g_current; /* 实时电流 */
extern int16_t g_test_current;
extern uint8_t g_test_current_valid;

static e_curr_item g_status; /* 电流父状态 */
uint8_t g_current_power_flag; /* 电流标志*/

static e_current_status g_c_state;
uint16_t g_vref_cpu_value;
float g_verf_ad_value;

static uint16_t c_fault_delay;
static uint16_t c_disch_protect2_delay; 
static uint16_t c_disch_pro2_delay;
static uint16_t c_disch_protect1_delay;
static uint16_t c_ch_protect_delay;
static uint16_t c_disch_alarm_delay;
static uint16_t c_ch_alarm_delay;
static uint16_t c_ch_delay;
static uint16_t c_disch_delay;

static uint8_t c_fault_status;
static uint8_t c_disch_protect2_status;
static uint8_t c_disch_protect1_status;
static uint8_t c_ch_protect_status;
static uint8_t c_disch_alarm_status;
static uint8_t c_ch_alarm_status;
static uint8_t c_ch_status;
static uint8_t c_disch_status;
//static uint8_t c_lowpower_dch_status;
int32_t g_current_offset = 0; 
uint8_t g_current_poweron; 

#if defined(TIANFENG) && defined(BAT_8S)/* 原为低温小电流禁止充放电，但设置了低温禁充温度，所以该功能未用到 */
static uint16_t tfLowTempCurrDisCHTime;	
static uint8_t tfLowTempCurrDisCHFlag;	
#endif

/*=============================================================
 * 函数名称：c_timer_ms_run
 * 函数功能：电流管理模块毫秒级定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void c_timer_ms_run(void)
{
    if ( c_process_delay ) c_process_delay --;
    if (c_disch_pro2_delay) c_disch_pro2_delay--;
}
/*=============================================================
 * 函数名称：get_current_offset_poweron
 * 函数功能：开机进行0点校准
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2021-06-08          liyong             创建
==============================================================*/
uint8_t get_current_offset_poweron(void)    
{
   if( g_current_poweron < CURRENT_POWERON )
     return 0;
   return 1;
}
/*=============================================================
 * 函数名称：get_cur_current
 * 函数功能：获取当前工作电流
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           
 * 修改记录：
   当前工作电流，单位为0.1A
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-11          戴辉发             创建
==============================================================*/
void adc_to_current_vaule( float curr)    
{
  float cur = curr;
  if( g_current_poweron < CURRENT_POWERON )
  {  
       g_current_poweron++;
       if( g_current_poweron >= 3 )
       {
          g_current_offset += cur;
          if( g_current_poweron == CURRENT_POWERON )
          {
             g_current_offset /= (CURRENT_POWERON-2);
             g_adjust_curr_para.b_value = g_current_offset;
             g_current_poweron = CURRENT_POWERON+10;
          }
       }      
  }
  else
  {
      cur -= g_adjust_curr_para.b_value;
      if ( 0 == g_adjust_curr_para.k_value )
      {
          cur = cur/ CURRENT_RVALUE ;  
      }
      else
      {
          cur = cur/ g_adjust_curr_para.k_value ;
      }
      
      if( cur > 0 )
          cur += 0.5;
      else
          cur -= 0.5;
      cur /= 10;
      
      if( cur > 0 )
          cur += 0.5;
      else
          cur -= 0.5;
      
      current_get_process((int16_t)(cur));  
  }
    
}

/*=============================================================
 * 函数名称：current_mem_init
 * 函数功能：电流管理模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void current_mem_init(void)
{
	/* 电流标识初始化 */
	g_status = E_IDLE;
    /* 对外电流状态 */
	g_c_state = E_C_IDLE;
    
	g_current_power_flag = 0;
    
    g_current = 0;
    
    c_fault_delay = 0;
    c_disch_protect2_delay = 0;
    c_disch_pro2_delay = 0; /* 单位为5ms，天丰二次保护计时用 */
    c_disch_protect1_delay = 0;
    c_ch_protect_delay = 0;
    c_disch_alarm_delay = 0;
    c_ch_alarm_delay = 0;
    c_ch_delay = 0;
    c_disch_delay = 0;
    c_fault_status = 0;
    c_disch_protect2_status = 0;
    c_disch_protect1_status = 0;
    c_ch_protect_status = 0;
    c_disch_alarm_status = 0;
    c_ch_alarm_status = 0;
    c_ch_status = 0;
    c_disch_status = 0;
    g_current_poweron = 0;
    g_current_offset = 0;
    set_curr_deal_process((uint32_t)adc_to_current_vaule);
    
#if defined(TIANFENG) && defined(BAT_8S)
    tfLowTempCurrDisCHTime = CH_LOWTEMP_CURR_DISCH_TIME;
    tfLowTempCurrDisCHFlag = 0;
#endif
}


/*=============================================================
 * 函数名称：get_cur_current
 * 函数功能：获取当前工作电流
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           返回当前工作电流，单位为0.01A
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-11          戴辉发             创建
==============================================================*/
int16_t get_cur_current(void)
{
	return g_run_sys_data.current;
}
/*=============================================================
 * 函数名称：get_current_power_flag
 * 函数功能：获取当前电流功率状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           获取当前电流功率状态
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-05          戴辉发             创建
==============================================================*/
uint8_t get_current_power_flag(void)
{
	return g_current_power_flag;
}
#define  RELEASE   20
/*=============================================================
 * 函数名称：get_status_data_process_section
 * 函数功能：分析数据状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-24          liyong             创建
==============================================================*/
void get_status_data_process_section(int16_t data, int16_t low, int16_t high, uint8_t *status, uint16_t *delay, uint16_t delaytime, uint16_t recoverDelay)
{
    switch(*status)
    {
	case 0:
        if ((data > high) || (data < low))
        {
			*status = 1;
			*delay = delaytime;
        }
        break;
	case 1:
        if ((data > high) || (data < low))
        {
			if (0 == (*delay))
            {
				*status = 2;
				*delay = recoverDelay;
			}
        }
		else
		{
			*status = 0;
		}
		break;
	default:
        if ((data < high) && (data > low))
        {
            if (0 == (*delay))
            {
                 *status = 0;
            }
        }
		else
		{
			*delay = recoverDelay;
		}
		break;
    }
}

/*=============================================================
 * 函数名称：get_status_data_process_greater
 * 函数功能：分析数据状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-24          liyong             创建
==============================================================*/
void get_status_data_process_greater(int16_t data, int16_t threshold, int16_t release, uint8_t *status, uint16_t *delay, uint16_t delaytime, uint16_t recoverDelay)
{
    switch(*status)
    {
	case 0:
        if (data > threshold)
        {
			*status = 1;  
			*delay = delaytime;
        }
        break;
	case 1:
        if (data > threshold)
        {
			if (0 == (*delay))
			{
				*status = 2;
				*delay = recoverDelay;
			}
        }
		else
		{
			*status = 0;
		}
		break;
	default:
        if (data < release)
        {
            if (0 == (*delay))
            {
                *status = 0;
            }
        }
		else
		{
			*delay = recoverDelay;
		}
		break;
    }
}

/*=============================================================
 * 函数名称：get_status_data_process_less
 * 函数功能：分析数据状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-24          liyong             创建
==============================================================*/
void get_status_data_process_less(int16_t data, int16_t threshold, int16_t release, uint8_t *status, uint16_t *delay, uint16_t delaytime, uint16_t recoverDelay)
{
    switch (*status)
    {
	case 0:
        if (data < threshold)
        {
			*status = 1;
            *delay = delaytime;
        }
        break;
	case 1:
        if (data < threshold)
        {
            if (0 == *delay)
			{
				*status = 2;
				*delay = recoverDelay;
			}
        }
		else
		{
			*status = 0;
		}
		break;
	default:
        if (data > release)
        {
            if (0 == *delay)
            {
                *status = 0;
            }
        }
		else
		{
            *delay = recoverDelay;
		}
		break;
    }
}

/*=============================================================
 * 函数名称：get_current_status
 * 函数功能：获取当前电流模块状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           当前电流模块状态
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-28          戴辉发             创建
==============================================================*/
e_current_status get_current_status(void)
{
	return g_c_state;
}

/*=============================================================
 * 函数名称：get_work_current
 * 函数功能：获取电流
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           电流值
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-28          戴辉发             创建
==============================================================*/
int16_t get_work_current(int16_t c_ad)
{
    /* 做电流温度补偿 */
	if(( c_ad < get_ch_current_in_level() ) && ( c_ad > get_dch_current_in_level() ))
	{
		c_ad = 0;
	}

	return (int16_t)c_ad;
}

/*=============================================================
 * 函数名称：current_get_process
 * 函数功能：电流获取过程
 * 参数个数：1
 *          [IN]       ad_value          AD值
 * 函数参数：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
uint8_t current_get_process(int16_t ad_value)
{
    uint8_t ret = 0;

    /* 设定SOC实时AD值 */
    set_soc_current_ad_value(ad_value);

    /* 计算实时电流 */
    g_current = get_work_current(ad_value);
    if (g_test_current_valid)
    {
        g_current = g_test_current;
    }
    
    return ret;
}

/***********************************************************
⒈电流指示
  ⑴电流 <= 放电电流门限
    ①g_status = E_DISCH
    ②清除过流恢复定时器
    ③清除过流等待稳定定时器
 ***********************************************************/
/*=============================================================
 * 函数名称：current_ch_over_idle_process
 * 函数功能：充电过流处理流程
 * 参数个数：1
 * 函数参数：
 *           [IN]      current            电流值
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void current_ch_over_idle_process(short current)
{
    /* ⒈电流指示 */
    if (current < get_dch_current_in_level())
    /*   ⑵电流 <= 放电电流门限 */
    {
    /*     ①g_status = E_DISCH */
        g_status = E_IDLE;
    }
}

/*=============================================================
 * 函数名称：init_low_power_status
 * 函数功能：初始化低功耗管理模块状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2020-06-29        liyong     	创建
==============================================================*/
uint8_t get_small_current_in_sheep()
{
    uint8_t flag = 0;

    if (E_CHARGE_STATUS == get_system_status())
    {
        if (get_real_current() < 20)
        {
            flag = 1;
        }
    }
    else
    {
        if (get_real_current() < 5)
        {
            flag = 1;
        }
    }
    return flag;
}

/*=============================================================
 * 函数名称：current_power_manage
 * 函数功能：电流低功耗处理流程
 * 参数个数：0
 * 函数参数：
 *          
 * 返 回 值：
 *          无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-02-26          戴辉发             创    建
==============================================================*/
static void current_power_manage(void)
{
    int16_t work_current = g_run_sys_data.current;
    int16_t temp = (get_rated_capcity() / 2); /* 电流单位为0.1A */
    if ( E_CHARGE_STATUS == get_system_status())
    /* 充电状态下 */
    {
	    if( temp > CH_SMALL_CURRENT_LEVEL ) temp = CH_SMALL_CURRENT_LEVEL;
        if ( work_current < temp )
        /* 电流在低功耗关断门限电流下，属于小电流标识2 */
        {
            //g_current_power_flag = 3;
        }
        else
        {
            g_current_power_flag = 0;
        }
#if defined(TIANFENG) && defined(BAT_8S)
        int16_t minTemp = get_min_cell_temp();
        if(tfLowTempCurrDisCHFlag != 1)
        {
            if(minTemp < 0)
            {
                if(work_current > 5)
                {
                    if(tfLowTempCurrDisCHTime <= 0)
                    {
                        tfLowTempCurrDisCHFlag = 1;
                    }
                }
                else
                {
                    tfLowTempCurrDisCHTime = CH_LOWTEMP_CURR_DISCH_TIME;
                    tfLowTempCurrDisCHFlag = 0;
                }
            }
            else
            {
                tfLowTempCurrDisCHTime = CH_LOWTEMP_CURR_DISCH_TIME;
                tfLowTempCurrDisCHFlag = 0;
            }
        }
#endif
    }
    else
    {
#if defined(TIANFENG) && defined(BAT_8S)
        tfLowTempCurrDisCHFlag = 0;
        tfLowTempCurrDisCHTime = CH_LOWTEMP_CURR_DISCH_TIME;
#endif
        e_soc_flag_type soc_status = get_soc_status();

        switch(soc_status)
        {
        case E_PROTECT_SOC:
#if 1 //defined (TIANHONG) || defined(FUDEER) || defined(TIANFENG)/* 天宏8串磷酸铁锂 */
            
#else
            if (work_current <= DCH_LOW_CURRENT_LEVEL)
            /* 严重保护状态下，电流大于关断门限 */
            {
                g_current_power_flag = 2;
//                if( c_lowpower_dch_status == 0 )
//                {
//                   c_lowpower_dch_status = 1;
//                   t_power_delay = CURRENT_POWER_DELAY;
//                } 
            }
            else
#endif
            {
                if ((work_current > DCH_SMALL_CURRENT_LEVEL) && (work_current < CH_SMALL_CURRENT_LEVEL))
                /* 小电流范围 */
                {
                    g_current_power_flag = 1;
                }
                else
                /* 正常工作电流 */
                {
                    g_current_power_flag = 0;
                }
            }
            
//            if( c_lowpower_dch_status == 1 )
//            {
//               if( t_power_delay == 0 )
//               {
//                   c_lowpower_dch_status = 2;
//                   g_current_power_flag = 2;
//               }   
//            } 
            
            break;
        case E_LOW_SOC:
        /* 低电量 */
#if 1//defined (TIANHONG)|| defined(FUDEER) || defined(TIANFENG)/* 天宏8串磷酸铁锂 */
            
#else
            if (work_current <= DCH_LOWPROTECT_CURRENT_LEVEL)
            /* 初级保护状态下，电流大于关断门限 */
            {
                 g_current_power_flag = 2;
//                if( c_lowpower_dch_status == 0 )
//                {
//                   c_lowpower_dch_status = 1;
//                   t_power_delay = CURRENT_POWER_DELAY;
//                } 
            }
            else
#endif
            {
                if ((work_current > DCH_SMALL_CURRENT_LEVEL) && (work_current < CH_SMALL_CURRENT_LEVEL))
                /* 小电流范围 */
                {
                    g_current_power_flag = 1;
                }
                else
                /* 正常工作电流 */
                {
                    g_current_power_flag = 0;
                }
            }
            
//            if( c_lowpower_dch_status == 1 )
//            {
//               if( t_power_delay == 0 )
//               {
//                   c_lowpower_dch_status = 2;
//                   g_current_power_flag = 2;
//               }   
//            }   
            break;
        default:
        /* 正常电量 */
            if ((work_current > DCH_SMALL_CURRENT_LEVEL) && ( work_current < CH_SMALL_CURRENT_LEVEL ))
            /* 小电流范围 */
            {
                g_current_power_flag = 1;
            }
            else
            /* 正常工作电流 */
            {
                g_current_power_flag = 0;
            }
            break;
        }
    }
}

static void current_inner_timer_process(void)
{
    if (c_process_delay) return;
    c_process_delay = CURRENT_TIMES;

	if (c_fault_delay > 0) c_fault_delay --;
	if (c_disch_protect2_delay > 0) c_disch_protect2_delay --; 
	if (c_disch_protect1_delay > 0) c_disch_protect1_delay --;
	if (c_ch_protect_delay > 0) c_ch_protect_delay --;
	if (c_disch_alarm_delay > 0) c_disch_alarm_delay --;
	if (c_ch_alarm_delay > 0) c_ch_alarm_delay --;
	if (c_ch_delay > 0) c_ch_delay --;
	if (c_disch_delay > 0) c_disch_delay --;
#if defined(TIANFENG) && defined(BAT_8S)
    if(tfLowTempCurrDisCHTime > 0) tfLowTempCurrDisCHTime--;
#endif
}
//考虑电流激变
//如果突然电流由0 变成 10 A 
//延时时间不能太长
//现在是在放电，突然充电过流
//现在在充电，突然放电过流
//转换状态需要时间，退出不需要
void current_process( int16_t work_current )
{
	current_inner_timer_process();

    /* 失效判决 */
    get_status_data_process_section(work_current, MIN_CURRENT_VALUE, MAX_CURRENT_VALUE, &c_fault_status, &c_fault_delay, TIMER_1S_TIMES * 5,
     RELEASE);

#if defined(TIANFENG) && defined(BAT_8S)
    /* 放电二次过流判决 */
     if( get_dch_current_second_delay() <= (TIMER_TIMES))
        get_status_data_process_less( work_current,
		get_dch_current_second_protect(),
		500+get_dch_current_second_protect(),
		&c_disch_protect2_status,
		&c_disch_pro2_delay,
		1,
     300/(TIMER_TIMES));
     else
        get_status_data_process_less( work_current,
		get_dch_current_second_protect(),
		500+get_dch_current_second_protect(),
		&c_disch_protect2_status,
		&c_disch_pro2_delay,
		get_dch_current_second_delay()/(TIMER_TIMES),
     300/(TIMER_TIMES));
#else
    /* 放电二次过流判决 */
     if( get_dch_current_second_delay() <= (TIMER_TIMES * CURRENT_TIMES) )
        get_status_data_process_less( work_current,
		get_dch_current_second_protect(),
		100+get_dch_current_second_protect(),
		&c_disch_protect2_status,
		&c_disch_protect2_delay,
		1,
     RELEASE);
     else
        get_status_data_process_less( work_current,
		get_dch_current_second_protect(),
		100+get_dch_current_second_protect(),
		&c_disch_protect2_status,
		&c_disch_protect2_delay,
		get_dch_current_second_delay()/(TIMER_TIMES * CURRENT_TIMES),
     RELEASE);
#endif
        
    /* 放电过流判决 */
#if defined(TIANFENG) && defined(BAT_8S)
     if( g_status != E_DISCH_OVER )
     {
         get_status_data_process_less( work_current,
         get_dch_current_protect(),
         500+get_dch_current_protect(),
         &c_disch_protect1_status,
         &c_disch_protect1_delay,
         TIMER_1S_TIMES * get_dch_current_protect_delay(),
         RELEASE);
     }
#else
     get_status_data_process_less( work_current,
	 get_dch_current_protect(),
	 50+get_dch_current_protect(),
	 &c_disch_protect1_status,
	 &c_disch_protect1_delay,
     TIMER_1S_TIMES * get_dch_current_protect_delay(),
     RELEASE);
#endif
	 
    /* 充电过流判决 */
     get_status_data_process_greater( work_current,
	 get_ch_current_protect(),
	 -50+get_ch_current_protect(),
	 &c_ch_protect_status,
	 &c_ch_protect_delay,
	 TIMER_1S_TIMES*get_ch_current_protect_delay(),
     TIMER_1S_TIMES * 3);
    
    /* 放电过流告警 */
#if defined(TIANFENG) && defined(BAT_8S)
     get_status_data_process_less( work_current,
	 get_dch_current_alarm_level(),
	 500+get_dch_current_alarm_level(),
	 &c_disch_alarm_status,
	 &c_disch_alarm_delay,
	 TIMER_1S_TIMES*get_dch_current_protect_delay(),
     TIMER_1S_TIMES * 5);
#else
     get_status_data_process_less( work_current,
	 get_dch_current_alarm_level(),
	 get_dch_current_alarm_level(),
	 &c_disch_alarm_status,
	 &c_disch_alarm_delay,
	 TIMER_1S_TIMES*get_dch_current_protect_delay(),
     RELEASE);
#endif
	 
    /* 充电过流告警 */
#if defined(TIANFENG) && defined(BAT_8S)
     get_status_data_process_greater( work_current,
	 get_ch_current_alarm_level(),
	 -50+get_ch_current_alarm_level(),
	 &c_ch_alarm_status,
	 &c_ch_alarm_delay,
	 TIMER_1S_TIMES*5,
     TIMER_1S_TIMES * 5);
#else
     get_status_data_process_greater( work_current,
	 get_ch_current_alarm_level(),
	 get_ch_current_alarm_level(),
	 &c_ch_alarm_status,
	 &c_ch_alarm_delay,
	 TIMER_1S_TIMES*get_ch_current_protect_delay(),
     RELEASE);
#endif

    /* 充电电流判决,需要考虑当前充电机状态 */
    if (ChargerIsConnect() == 1)
    {
        get_status_data_process_greater(work_current, 
            get_ch_current_in_level(), 
            2, 
            &c_ch_status, 
            &c_ch_delay, 
            TIMER_1S_TIMES*1,
     RELEASE);
    }
    else
    /* 在非充电状态下，进入充电电流状态需要延时1分钟 */
    {
        get_status_data_process_greater(work_current, 
            get_ch_current_in_level(), 
            2, 
            &c_ch_status, 
            &c_ch_delay, 
            TIMER_1S_TIMES * 20,
     RELEASE);
    }
	 
    /* 放电电流判决 */
     get_status_data_process_less( work_current,
	 get_dch_current_in_level(),
	 -2,
	 &c_disch_status,
	 &c_disch_delay,
	 TIMER_250MS_TIMES,
     RELEASE);
     
     if( 2 == c_fault_status )
     {
        g_status = E_FLT_IDLE;
     }
     else if( g_status == E_FLT_IDLE )
     {//错误恢复
         g_status = E_IDLE;
     } 
     else if( 2 == c_disch_protect2_status )
     {
         g_status = E_SECOND;
     }
     else if( g_status == E_SECOND )
     {//放电二次恢复
      /* ⒈关机后恢复 (初始化状态标志) ⒉充电恢复  ⒊负载移除恢复 */  
        if ( 2 == c_ch_status )
        {
            g_status = E_CH; 
        }
        else if ( 1 == get_load_status() )
        {
            g_status = E_IDLE; 
        }
     }
     else if( 2 == c_disch_protect1_status )
     {
        g_status = E_DISCH_OVER;
     }
     else if( g_status == E_DISCH_OVER )
     {//放电过流恢复
#if !(defined(TIANFENG) && defined(BAT_8S))
        if ( 2 == c_ch_status )
        {
            g_status = E_CH; 
        }
         else if ( 1 == get_load_status() )
        {
            g_status = E_IDLE; 
        } 
#endif
     }
     else if( 2 == c_ch_protect_status )
     {
        g_status = E_CH_OVER;      
     }
     else if( g_status == E_CH_OVER )
     {//充电过流恢复
         if ( 2 == c_disch_status )
        {
            g_status = E_DISCH;
        }
//        else if ( E_CHARGE_STATUS != get_system_status() )
//        {
//            g_status = E_IDLE;
//        } 
     }
     else if( 2 == c_disch_alarm_status )
     {
        g_status = E_DISCH_ALARM;     
     }
     else if( 2 == c_ch_alarm_status )
     {
        g_status = E_CH_ALARM;  
     }
     else if( 2 == c_disch_status )
     {
        g_status = E_DISCH;     
     }
     else if( 2 == c_ch_status )
     {
        g_status = E_CH;     
     }
     else
     {
        g_status = E_IDLE;
     }
    
}

/*=============================================================
 * 函数名称：current_deal_process
 * 函数功能：电流处理流程
 * 参数个数：0
 * 函数参数：
 *          
 * 返 回 值：
 *          无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-02-26          戴辉发             创    建
==============================================================*/
static void current_deal_process(void)
{
    current_process(g_current);
    set_min_current(g_current);
    set_max_current(g_current);
    /* 电流判决 */
    switch( g_status )
    {        
    case E_FLT_IDLE:
        if( g_c_state != E_C_FAULT )
        {
            g_c_state = E_C_FAULT;
            record_protect = 1; 
        }
        break;
    case E_SECOND: 
        if( g_c_state != E_C_SECOND_PROTECT )
        {
            g_c_state = E_C_SECOND_PROTECT;  
            set_discurrent_over_count();
            record_protect = 1;
        }    
        break;       
    case E_DISCH_OVER:
        if( g_c_state != E_C_DCH_PROTECT )
        {
            g_c_state = E_C_DCH_PROTECT;
            set_discurrent_over_count();
            record_protect = 1;
        }
        break;
    case E_DISCH_ALARM: 
        g_c_state = E_C_DCH_ALARM;
        break;
    case E_DISCH: 
        g_c_state = E_C_DCH;
        break;
    case E_IDLE: 
        g_c_state = E_C_IDLE;
        break;
    case E_CH:
        g_c_state = E_C_CH;
        break;
    case E_CH_ALARM:  
        g_c_state = E_C_CH_ALARM;
        break;
    case E_CH_OVER:
        if( g_c_state != E_C_CH_PROTECT )
        {
            g_c_state = E_C_CH_PROTECT;
            set_chcurrent_over_count();
            record_protect = 1;
        }      
        break;
    default:
        g_c_state = E_C_IDLE;
        break;
    }


    if( g_c_state == E_C_CH_PROTECT )
    {
        protect_code[1] |= 0x01;
    }
    else
    {
        protect_code[1] &= ~0x01;
    }

    if(( E_C_DCH_PROTECT == g_c_state )||( E_C_SECOND_PROTECT == g_c_state ))
    {
        protect_code[1] |= 0x02;
    }
    else
    {
        protect_code[1] &= ~0x02;
    }

    if( g_c_state == E_C_FAULT )
    {
        protect_code[1] |= 0x10;
    }
    else
    {
        protect_code[1] &= ~0x10;
    }

    control_current_indication();
}

/*=============================================================
 * 函数名称：current_managment_process
 * 函数功能：电流处理流程
 * 参数个数：0
 * 函数参数：
 *          
 * 返 回 值：
 *          无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创    建
==============================================================*/
void current_managment_process(void)
{
    current_deal_process();
    current_power_manage();
}

/*=============================================================
 * 函数名称：get_real_current
 * 函数功能：获取当前实时电流
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          实时电流
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2019-08-22          戴辉发             创建
==============================================================*/
int16_t get_real_current(void)
{
    return g_current;
}

#if defined(TIANFENG) && defined(BAT_8S)
/*=============================================================
 * 函数名称：getTfLowTempCurrDisCHFlag
 * 函数功能：获取小于0℃，充电电流大于0.5A禁充标志
 * 参数描述：无
 * 返 回 值：1禁充
            0 正常
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2024-03-13           Fanyl             创建
==============================================================*/
uint8_t getTfLowTempCurrDisCHFlag(void)
{
    return tfLowTempCurrDisCHFlag;
}
#endif
