/*
 * ch_addition.c
 *
 *  Created on: 2018年10月20日
 *      Author: daihuifa
 */

#include "ch_addition.h"
#include "parameter.h"
#include "vol_manage.h"
#include "ch_detect.h"
#include "temp_manage.h"
#include "parameter.h"
#include "mode_manage.h"
#include "soc.h"
#include "can_app.h"
#include "current_manage.h"
#include "soc_update.h"
#include "balance.h"
#include "can.h"
#include "ch_detect.h"
#include "system_control.h"
#include "switch_status.h"
#include "power.h"
#include "fault_manage.h"
#include "vol_curr_addi_deal.h"
#include "short.h"
#include "afe_app.h"
#include "system_adjust.h"
#include "chargerProtocol.h"

#define CH_VOLTAGE_FILTER_TIME  50

#define CH_100MS_TIME           1
#define CH_1000MS_TIME          10
#define CH_500MS_TIME           5
#define CH_200MS_TIME           2
#define CH_3S_TIME              (3 * CH_1000MS_TIME)
#define CH_5S_TIME              (5 * CH_1000MS_TIME)
#define CH_10S_TIME             (10 * CH_1000MS_TIME)
#define CH_60S_TIME             (60 * CH_1000MS_TIME)
#define CH_5MIN_TIME            (300 * CH_1000MS_TIME)
#define CH_1H_TIMER             (get_deactive_delay() * 60 * CH_1000MS_TIME)


#define MAX_SPIN_VOLTAGE        (50)
#define MAX_RESEND_NUM          4
#define MAX_CUR_ADJUST_NUM      4
#define MAX_COLSED_NUM          20

#define CH_MIN_CURRENT_VALUE    (get_rated_capcity() / 2) /* 最小电流0.05C */
#define CH_MAX_CURRENT_VALUE    (5 * get_rated_capcity()) /* 最大电流0.5C */
#define CH_MIN_VOLTAGE_VALUE    (25 * BAT_NUM)            /* 最小电压2.5V * BAT_NUM */

#if defined(HUAFU) && defined(BATTARY_LFP)   
#define CH_MAX_VOLTAGE_VALUE    (355 * BAT_NUM/10)         /* 最高电压3.55V * BAT_NUM */
#define BOTTOM_VOLTAGE          3500
#define PROTECT_VOLTAGE         3550
#define BOTTOM_AVL_VOLTAGE      3400
#elif defined(TIANFENG) && defined(BATTARY_LFP) && defined(BAT_8S)
#define CH_MAX_VOLTAGE_VALUE   (37 * BAT_NUM)               /* 最高电压3.7V * BAT_NUM */
#define BOTTOM_VOLTAGE          3550
#define PROTECT_VOLTAGE         3700
#define BOTTOM_AVL_VOLTAGE      3400
#elif defined(BATTARY_LFP)   
#define CH_MAX_VOLTAGE_VALUE    (36 * BAT_NUM)            /* 最高电压3.6V * BAT_NUM */
#define BOTTOM_VOLTAGE          3500
#define PROTECT_VOLTAGE         3600
#define BOTTOM_AVL_VOLTAGE      3400
#else
#define CH_MAX_VOLTAGE_VALUE    (41 * BAT_NUM)            /* 最高电压4.1V * BAT_NUM */
#define BOTTOM_VOLTAGE          4050
#define PROTECT_VOLTAGE         4150
#define BOTTOM_AVL_VOLTAGE      4000
#endif

#define   HEAT_CURRENT_SET      50    /*加热电流设置为5A*/
/* 电压最高：20--29V，电流0.9--25.5A， */

/* 充电爷爷状态 */
typedef enum _E_CH_GRAND_STATUS_
{
	E_CH_DISABLED_STATUS, /* 放电状态 */
	E_CH_ENABLED_STATUS, /* 充电状态 */
}e_ch_grand_status;

/* 充电父状态 */
typedef enum _E_CH_STATUS_
{
	E_CH_INIT_STATUS, /* 充电初始电流爬升状态 */
	E_CH_IDLE_STATUS, /* 充电末端状态 */
	E_CH_BOTTOM_STATUS, /* 充电末端状态97% */
	E_CH_BALANCE_STATUS, /* 电池充电维护状态 */
	E_CH_FINISHED_STATUS, /* 充电完成状态 */
}e_ch_status;

/* 充电器附加模块消息类型 */
typedef enum _E_CH_MSG_
{
	E_CH_ENABLED_REQUEST, /* 充电附加模块使能请求 */
	E_CH_TIMER_JUDGE_IND, /* 再次判决定时器指示 */
	E_CH_TIMER_FULL_IND, /* 满充判决定时器指示 */
	E_CH_TIMER_CURR_ADJUST_IND, /* 电流调节定时器 */
	E_CH_TIMER_MAINTENANCE_IND, /* 电池充电维护限时定时器 */
	E_CH_CUR_VOL_IND, /* 当前电压指示 */
	E_CH_CURR_IND, /* 电流指示 */
	E_CH_CHARGER_IND, /* 充电器响应 */
	E_CH_AUTH_MAX_MSG_NUM
}e_ch_msg;

static uint16_t g_ch_vol; /* 最终权衡温度后的充电电压 */
static uint16_t g_ch_current; /* 最终权衡温度后的充电电流 */

static uint16_t g_ch_adjust_vol; /* 根据电压状态调整的电压 */
static uint16_t g_ch_adjust_current; /* 根据电压状态调整的电流 */

static e_ch_grand_status g_ch_grand_status; /* 充电器祖父状态 */
static e_ch_status g_ch_status; /* 充电器父状态 */

static uint8_t g_full_timer_flag; /* 充电满充标志 */
static uint8_t g_cur_adjust_num; /* 电流调节次数 */

static volatile uint16_t judge_timer; /* 判决定时器 */
static volatile uint16_t full_timer; /* 满充判决定时器 */
static volatile uint16_t soc_full_timer; /* 满充判决定时器 */
static volatile uint16_t g_ch_timer;
static volatile uint16_t g_ch_link_timer; /* 充电器连接状态定时器 */
/* 充电器连接状态，0：充电器未连接，1：充电器连接状态 */
static uint8_t g_ch_link_status;

/*电压滤波延时*/
static volatile uint16_t voltageFilteredDelay;
static uint8_t rChargerMessFlag;
static volatile uint16_t rxChMessageFlagDelay;
static uint8_t g_soc_update_status;
static uint8_t g_ch_error_status;
static uint8_t chargerOverWakeUpSleepQuick; /*休眠唤醒后，如果过压状态未解除，快速休眠标志*/

uint8_t bChargerNodeID = 0x60;

static uint8_t g_ch_full_flag;

static uint8_t Soc95AdjustDelay;

static void ch_addition_send_command(void);
static void send_nmt_to_charger(void);

typedef void (*ch_status_fun)(void *pmsg);

static void ch_status_msg_process(void);

/* 充电禁止状态下充电指示处理函数 */
static void ch_disabled_ch_indication(void);
/* 充电状态下充电指示处理函数 */
static void ch_enabled_ch_indication(void);

/* 充电状态下初始状态事件处理函数 */
static void ch_init_status_msg_process(void);
/* 充电状态下空闲状态事件处理函数 */
static void ch_idle_status_msg_process(void);
/* 充电状态充电末端状态事件处理函数 */
static void ch_bottom_status_msg_process(void);
/* 充电状态充电均衡维护状态事件处理函数 */
static void ch_balance_status_msg_process(void);

static void send_rxpdo_to_charger(uint16_t voltage, uint16_t current, uint8_t flag);

/*=============================================================
 * 函数名称：ch_addition_mem_init
 * 函数功能：充电辅助模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期             修改人            修改类型
 * 2018-10-20       	戴辉发     	      创建
==============================================================*/
void ch_addition_mem_init(void)
{
	g_ch_grand_status = E_CH_DISABLED_STATUS;
	g_ch_status = E_CH_INIT_STATUS;
	g_cur_adjust_num = 0;
    /* 满充判决标志 */
    g_ch_full_flag = 0;
	judge_timer = 0; /* 判决定时器 */
	full_timer = 0; /* 满充判决定时器 */
	soc_full_timer = 0; /* 满充判决定时器 */
    g_soc_update_status = 0;
    Soc95AdjustDelay = 100;
    voltageFilteredDelay = CH_VOLTAGE_FILTER_TIME;
    g_ch_link_status = 0;
    rChargerMessFlag = 0;
    rxChMessageFlagDelay = 0;
    ChargerProtocolMemoryInit();
}
/*=============================================================
 * 函数名称：get_ch_link_status
 * 函数功能：获取充电器接入状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *=============================================================
 * 日    期          修改人           修改类型
 * 2021-07-18        戴辉发           创建
 ==============================================================*/
uint8_t get_ch_link_status(void)
{
    return g_ch_link_status;
}

void SetRecivedChargerMessFlag(uint8_t flag)
{
    rChargerMessFlag = flag;
    if(flag == 0)
    {
        rxChMessageFlagDelay = 0;
    }
    else
    {
        rxChMessageFlagDelay = CH_3S_TIME;
    }
}

uint8_t GetRecivedChargerMessFlag(void)
{
    if((rChargerMessFlag != 0) && (rxChMessageFlagDelay != 0))
    {
        return 1;
    }
    rChargerMessFlag = 0;
    return 0;
}
/*=============================================================
 * 函数名称：ch_link_heart
 * 函数功能：充电器
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *=============================================================
 * 日    期          修改人           修改类型
 * 2021-07-18        戴辉发           创建
 ==============================================================*/
void ch_link_heart(void)
{
    if (0 == g_ch_link_timer)
    {
        g_ch_link_status = 0;
    }
}

/*=============================================================
 * 函数名称：get_ch_full_flag
 * 函数功能：获取满充标识
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          满充标识
 * 修改记录：
 *===============================================================
 * 日    期          修改人           修改类型
 * 2020-09-16        戴辉发           创建
==============================================================*/
uint8_t getChFullFlag(void)
{
    return g_ch_full_flag;
}

/*==============================================================
 * 函数名称：ch_addition_timer
 * 函数功能：CAN APP定时器，100毫秒定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日    期          修改人           修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
void ch_addition_timer(void)
{
	if (g_ch_timer > 0) g_ch_timer --;
	if (judge_timer > 0) judge_timer --;
	if (full_timer > 0) full_timer --;
	if (soc_full_timer > 0) soc_full_timer --;
    if (Soc95AdjustDelay > 0) Soc95AdjustDelay --;
    if (voltageFilteredDelay > 0) voltageFilteredDelay --;
    if (g_ch_link_timer > 0) g_ch_link_timer --;
    if (rxChMessageFlagDelay > 0) rxChMessageFlagDelay --;
}

/*==============================================================
 * 函数名称：ch_addition_judge_process
 * 函数功能：充电器错误状态判决，主要判决功率线断开环节
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          0          无错误
 *          1          错误状态
 * 修改记录:
 *===============================================================
 * 日    期          修改人           修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
void ch_addition_judge_process(void)
{
	g_ch_error_status = 0;
	if (g_ch_grand_status == E_CH_ENABLED_STATUS)
	{
        if ((g_ch_status == E_CH_INIT_STATUS) && (g_cur_adjust_num > 1))
        /* 初始状态且已经设定过电流 */
        {
            if (0 == get_real_current() && ((get_total_vol() + 20) < (g_ch_vol * 10)))
            {
                g_ch_error_status = 1;
            }
        }
        else if ((g_ch_status == E_CH_IDLE_STATUS) || (g_ch_status == E_CH_BOTTOM_STATUS))
        {
            if (get_total_vol() > (20 + (g_ch_vol * 10)))
            {
                g_ch_error_status = 1;
            }
        }
	}
}

/*==============================================================
 * 函数名称：get_charger_error_status
 * 函数功能：获取充电器状态，主要判决功率线连接状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          0          无错误
 *          1          错误状态
 * 修改记录:
 *===============================================================
 * 日    期          修改人           修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
uint8_t get_charger_error_status(void)
{
	return g_ch_error_status;
}

/*==============================================================
 * 函数名称：ch_addition_process
 * 函数功能：充电附加模块处理流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-05-18          戴辉发             创建
==============================================================*/
void ch_addition_process(void)
{
    /* 与充电器的心跳流程 */
    ch_link_heart();
  if ( get_g_grand_status() == E_ENABLE_STATUS )
  {
      /* 判决充电标识，启动对应的充电附加模块 */
      if ( E_CHARGE_STATUS == get_system_status() )
      /* 充电状态 */
      {
          ch_disabled_ch_indication();
      }
      else
      /* 退出充电状态 */
      {
          ch_enabled_ch_indication();
      }
      
      if (g_ch_grand_status == E_CH_ENABLED_STATUS)
      {
          ch_status_msg_process();
          ch_addition_send_command();
      }
  }
  else
  {
      /* 满充判决标志 */
        g_ch_full_flag = 0;
  }
	
}

/*=============================================================
 * 函数名称：ch_data_indication
 * 函数功能：充电器响应数据
 * 参数个数：1
 * 参数描述：
 *          [IN]     data_buf    当前电流
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2019-05-06        戴辉发      创建
==============================================================*/
void ch_data_indication(uint8_t *data_buf)
{
      /* 检测到充电器接入 */
    g_ch_link_status = 1;
    /* 重置定时器6秒 */
    g_ch_link_timer = CH_3S_TIME * 2;
}

/*=============================================================
 * 函数名称：judge_ch_finished
 * 函数功能：判断充电完成标识
 * 参数个数：0
 * 参数描述：
 * 返 回 值：0       充电未完成
 *           1      充电完成
 * 修改记录：
 *===============================================================
 * 日    期             修改人          修改类型
 * 2019-04-05       	戴辉发     	   创建
==============================================================*/
uint8_t judge_ch_finished(void)
{
	if ((g_ch_status == E_CH_FINISHED_STATUS) && (g_ch_grand_status == E_CH_ENABLED_STATUS))
	{
		return 1;
	}

	return 0;
}

/*=============================================================
 * 函数名称：judge_ch_float
 * 函数功能：判断充电完成标识
 * 参数个数：0
 * 参数描述：
 * 返 回 值： 0       未完成状态
 *          1       完成状态
 * 修改记录：
 *===============================================================
 * 日    期                     修改人            修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
uint8_t judge_ch_float(void)
{
	if ((g_ch_status == E_CH_BALANCE_STATUS) && (g_ch_grand_status == E_CH_ENABLED_STATUS))
	{
		return 1;
	}

	return 0;
}

/*=============================================================
 * 函数名称：ch_addition_send_command
 * 函数功能：充电辅助模块向充电器发送充电命令，需要考虑联网模式下的充电电流问题
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人            修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
static void ch_addition_send_command(void)
{
	int16_t max_temp, min_temp;
	uint16_t min_temp_current = 10 * get_rated_capcity();
	uint16_t max_temp_current = 10 * get_rated_capcity();
	uint8_t ch_flag;

	max_temp = get_max_cell_temp();
	min_temp = get_min_cell_temp();
	if (min_temp <= get_ch_cell_temp_low_protect())
	/* 0.0C */
	{
		min_temp_current = 0;
	}
	else if (min_temp <= get_ch_cell_temp_low_alarm_level())
	/* 0.1C */
	{
		min_temp_current = get_rated_capcity();
	}
	else if (min_temp <= get_ch_cell_temp_high_alarm_level())
	/* 0.5C */
	{
        min_temp_current = get_rated_capcity();
		min_temp_current = (uint16_t)(5 * min_temp_current);
	}

	if (max_temp < get_ch_cell_temp_high_alarm_level())
	/* 0.5C */
	{
		max_temp_current = (uint16_t)(0.5 * max_temp_current);
	}
    else if (max_temp < get_ch_cell_temp_high_protect())
	/* 0.3C */
	{
		max_temp_current = (uint16_t)(0.3 * max_temp_current);
	}
	else if (max_temp >= get_ch_cell_temp_high_protect())
	/* 0.0C */
	{
		max_temp_current = 0;
	}
	/* 两者之间小值放到min_temp_current变量中 */
	if (max_temp_current < min_temp_current)
	{
		min_temp_current = max_temp_current;
	}

	/* 电压调整电流与温度电流，取最小电流 */
	if (min_temp_current > g_ch_adjust_current)
	{
		g_ch_current = g_ch_adjust_current;
	}
	else
	{
		g_ch_current = min_temp_current;
	}
 
	g_ch_vol = g_ch_adjust_vol;
    
	if ((1 == get_ch_switch_status()) && 
		(0 == judge_ch_finished()))
	{
		ch_flag = 0;
        g_ch_current = (uint16_t)(g_ch_current);
	}
    else if (( 1 == judge_heat_on() ) && 
		(0 == judge_ch_finished()))
    {/*进入加热模式*/
        ch_flag = 0;
        g_ch_current = HEAT_CURRENT_SET;
    }
	else
	/* 关闭充电器开关 */
	{
		ch_flag = 1;
        g_ch_current = 0;
	}

    if (0 == g_ch_timer)
    {
        g_ch_timer = CH_100MS_TIME;
        if((get_ch_link_status() == 1) || (GetRecivedChargerMessFlag() == 1))
        {
            send_rystate_to_charger((1 == judge_ch_finished()) ? 0 : g_ch_adjust_vol, g_ch_current, ch_flag);
        }
        //send_rxpdo_to_charger(g_ch_adjust_vol, g_ch_current, ch_flag); /* 去掉充电报文 */
        //send_nmt_to_charger();
    }
}
/*******************************************************************************
 ** FuncName: get_errcode_for_charger
 ** Function: 故障码配置;
 ** Output  : 无;
 ** input   : 无;
 ** Create date : liyong @2021.5.12
 ** Modify  : 
*******************************************************************************/
uint8_t get_errcode_for_charger(void)
{
	uint8_t  errcode = 0;
    uint8_t flag = get_final_set_switch();

    //32	5.5	二次模块粘连故障
    //33	5.6	二次模块无法闭合故障
    //36	5.9	存储器通信故障
    //37	6.1	CAN通信故障
    if (get_main_power_detect_status() == 0)
    {//35	5.8	主电源失效
        errcode = 35;
    } 
    else if (E_BAT_FAIL == get_bat_status())  
    {//31	5.4	电池压差过大保护
        errcode = 31;
    }
    else if (0 == get_current_offset_poweron())
    {//38	6.2	开机自检中
        //30	5.3	AFE电流校准状态
        errcode = 38;
    }
    else if (1 == get_fail_cutoff_flag())
    {//34	5.7	mos关闭失败
        errcode = 34;
    }
    else if (1 == get_afe_status())
    {//26	4.8	AFE通信故障
        errcode = 26;
    }
    else if (0 != get_vol_two_comp())
    {//27	4.9	电压采样差别大故障
        errcode = 27;
    }
    else if (2 == get_vol_afe_status())
    {//28	5.1	电压采样不变故障
        errcode = 28;
    }
    else if (0 != get_curr_afe_flag())
    {//29	5.2	电压电流采样故障
        errcode = 29;
    }
    else if (E_SWITCH_INVALID == get_dch_switch_flag())
    {//24	4.6	放电开关失效
        errcode = 24;
    }
    else if (E_SWITCH_INVALID == get_ch_switch_flag())
    {//23	4.5	充电开关失效
        errcode = 23;
    }
    else if (2 == get_status_error_flag())
    {
        //25	4.7	状态互锁保护
        errcode = 25;
    }
    else if ((3 == get_curr_status())|| ( 0 != get_short_status() ))
    {//11	3.2	输出短路保护
        errcode = 11;
    }
    else if (protect_code[1] & 0x08)
    {//12	3.3	硬件放电过流保护
        errcode = 12;
    }
    else if ((E_C_SECOND_PROTECT ==  get_current_status()) || (E_C_DCH_PROTECT ==  get_current_status()))
    {//10	3.1	放电过流保护
        errcode = 10;
    }
    else if (flag & 0x04)
    {//8	2.8	硬件终极过压保护
        errcode = 8;
    }
    else if (flag & 0x08) 
    {//7	2.7	硬件终极欠压保护
        errcode = 7;
    }
    else if (flag & 0x01)
    {//6	2.6	软件终极过压保护
        errcode = 6;
    }
    else if (flag & 0x02)
    {//5	2.5	软件终极欠压保护+
        errcode = 5;
    }
    else if (E_VOL_OVER == get_cell_vol_status())
    {//2	2.2	单体过压保护
        errcode = 2;
    }
    else if (E_VOL_OVER == get_total_vol_status())
    {//4	2.4	总压过压保护
        errcode = 4;
    }
    else if (E_VOL_UNDER == get_cell_vol_status())
    {//1	2.1	单体欠压保护
        errcode = 1;
    }
    else if (E_VOL_UNDER == get_total_vol_status())
    {//3	2.3	总压欠压保护
        errcode = 3;
    }
    else if (E_C_CH_PROTECT ==  get_current_status())
    {//9	2.9	充电过流保护
        errcode = 9;
    }
    else if (E_C_FAULT ==  get_current_status())
    {//13	3.4	电流检测异常
        errcode = 13;
    }
    else if (E_TEMP_LOW_PROTECT ==  get_dch_temp_status())
    {//14	3.5	电芯低温禁放
        errcode = 14;
    }
    else if (E_TEMP_HIGH_PROTECT ==  get_dch_temp_status())
    {//15	3.6	电芯高温禁放 
        errcode = 15;
    }
    else if (E_TEMP_LOW_PROTECT ==  get_ch_temp_status())
    {//16	3.7	电芯低温禁充
        errcode = 16;
    }
    else if (E_TEMP_HIGH_PROTECT ==  get_ch_temp_status())
    {//17	3.8	电芯高温禁充
        errcode = 17;
    } 
    else if (E_TEMP_LOW_PROTECT ==  get_power_temp_status())
    {//18	3.9	功率低温保护
        errcode = 18;
    }
    else if (E_TEMP_HIGH_PROTECT ==  get_power_temp_status())
    {//19	4.1	功率高温保护
        errcode = 19;
    }
    else if (E_TEMP_LOW_PROTECT ==  get_environ_temp_status())
    {//20	4.2	环境低温保护
        errcode = 20;
    }
    else if (E_TEMP_HIGH_PROTECT ==  get_environ_temp_status())
    {//21	4.3	环境高温保护
        errcode = 21;
    }
    if (E_PROTECT_SOC == get_soc_status())
    {//22	4.4	容量保护
        errcode = 22;
    }

    return errcode;
}

/*=============================================================
* 函数名称：send_rxpdo_to_charger
* 函数功能：发送充电机
* 参数个数：3
* 函数参数：
*          [IN]       voltage      充电电压
*          [IN]       current      充电电流
*          [IN]       flag         充电禁止允许标志
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                             修改人             修改内容
* 2018-10-21          戴辉发             创建
==============================================================*/
void send_rxpdo_to_charger(uint16_t voltage, uint16_t current, uint8_t flag)
{
	uint8_t buf[8];

	/* 充电额定电压 */
	buf[0] = (uint8_t)((voltage >> 0) & 0xff);
	buf[1] = (uint8_t)((voltage >> 8) & 0xff);
	/* 最大充电电流 */
	buf[2] = (uint8_t)((current >> 0) & 0xff);
	buf[3] = (uint8_t)((current >> 8) & 0xff);
	/* 充电允许 */
	buf[4] = flag;
	/*充电状态*/
    if (1 == judge_ch_finished())
    {
        buf[5] = 0x02;
    }
    else
    {
        buf[5] = 0x00;
    }
    /* 故障码 */
	buf[6] = get_errcode_for_charger();
    //BMS的SOC电量：0-255对应0-100%
	buf[7] = g_run_sys_data.soc;
	can_std_transmit(0x200 + bChargerNodeID, buf, 0, 8);
   
}
/*=============================================================
* 函数名称：send_rystate_to_charger
* 函数功能：发送充电机
* 参数个数：3
* 函数参数：
*          [IN]       voltage      充电电压
*          [IN]       current      充电电流
*          [IN]       flag         充电禁止允许标志
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                             修改人             修改内容
* 2018-10-21          戴辉发             创建
==============================================================*/
void send_rystate_to_charger(uint16_t voltage, uint16_t current, uint8_t flag)
{
    uint8_t buf[8] = {0};
    uint8_t status = 0;

    buf[0] = (uint8_t)(voltage & 0xff);
    buf[1] = (uint8_t)((voltage >> 8) & 0xff);
    buf[2] = (uint8_t)(current & 0xff);
    buf[3] = (uint8_t)((current >> 8) & 0xff);
    buf[4] = (uint8_t)(100L * g_run_sys_data.soc / MAX_SOC_VALUE);

    if(flag == 1)
    {
        status |= 0x01;
    }
    if(judge_ch_finished() == 1)
    {
        status |= 0x02;
    }
    if((get_ch_temp_status() == E_TEMP_HIGH_PROTECT) ||
       (get_dch_temp_status() == E_TEMP_HIGH_PROTECT))
    {
        status |= 0x04;
    }
    if(E_CHARGE_STATUS == get_system_status())
    {
        if(get_current_status() == E_C_CH_PROTECT)
        {
            status |= 0x08;
        }
    }
    else
    {
        if((get_current_status() == E_C_DCH_PROTECT) ||
           (get_current_status() == E_C_SECOND_PROTECT) ||
           (get_short_status() != 0))
        {
            status |= 0x08;
        }
    }
    if(E_CHARGE_STATUS == get_system_status())
    {
        if((get_total_vol_status() == E_VOL_OVER) ||
           (get_cell_vol_status() == E_VOL_OVER))
        {
            status |= 0x10;
        }
    }
    else
    {
        if(get_cell_vol_status() == E_VOL_OVER)
        {
            status |= 0x10;
        }
    }

    buf[5] = status;

    if((judge_heat_on() == 1) && (judge_ch_finished() == 0))
    {
        buf[6] = 0x01;
    }
    buf[7] = 0;

    can_std_transmit(0x246, buf, 0, 8);
}

/*=============================================================
* 函数名称：send_nmt_to_charger
* 函数功能：发送充电机
* 参数个数：0
* 函数参数：
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-21          戴辉发             创建
==============================================================*/
void send_nmt_to_charger(void)
{
	uint8_t buf[2];
	buf[0] = 0x01;
	buf[1] = 0x00;
	can_std_transmit(0x00, buf, 0, 2);
    
}

/*=============================================================
 * 函数名称：send_closed_rxpdo_to_charger
 * 函数功能：发送关闭充电机命令请求
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-10-21          戴辉发             创建
==============================================================*/
void send_closed_rxpdo_to_charger(void)
{
	uint8_t buf[8];

	/* 充电额定电压 */
	buf[0] = (uint8_t)((g_ch_vol >> 0) & 0xff);
	buf[1] = (uint8_t)((g_ch_vol >> 8) & 0xff);
	/* 最大充电电流 */
	buf[2] = (uint8_t)((0 >> 0) & 0xff);
	buf[3] = (uint8_t)((0 >> 8) & 0xff);
	/* 充电允许为关闭 */
	buf[4] = 1;
	/* 保留 */
	buf[5] = 0x02;
	 /* 故障码 */
	buf[6] = get_errcode_for_charger();
    //BMS的SOC电量：0-255对应0-100%
	buf[7] = g_run_sys_data.soc;
	//can_std_transmit(0x200 + bChargerNodeID, buf, 0, 8); /* 关闭充电机报文 */
}

/*=============================================================
 * 函数名称：ch_status_msg_process
 * 函数功能：充电鉴权模块状态事件表处理函数
 * 参数个数：1
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 * 返回值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-16          戴辉发             创建
==============================================================*/
static void ch_status_msg_process(void)
{
	switch(g_ch_status)
	{
	case E_CH_INIT_STATUS:
		ch_init_status_msg_process();
		break;
	case E_CH_IDLE_STATUS:
	/* 充电状态 */
		ch_idle_status_msg_process();
		break;
	case E_CH_BOTTOM_STATUS:
		ch_bottom_status_msg_process();
		break;
	case E_CH_BALANCE_STATUS:
	/* 充电完成状态 */
		ch_balance_status_msg_process();
		break;
	default:
		break;
	}
}

/* status: E_CH_AUTH_DISABLED_STATUS,消息处理函数表 */
/* E_CH_IND, 充电指示 */
/***********************************************************
⒈使能请求
  ⑴启动
    ①电流调整次数 = 0
    ②调整电压 = 额定电压
    ③调整电流 = 0.05C(全部接入电池包)
    ④父状态 = ENABLED_STATUS
    ⑤当前平均电压
      ㈠ >= 97%电压(恒流百分比)
        ⅰ启动再次判决定时器
        ⅱ调整电流 = 额定电流 / 2
        ⅲ子状态 = BOTTOM_STATUS
      ㈡ >= 充电保护电压
        ⅰ调整电流 = 最小电流
        ⅱ子状态 = END_STATUS
      ㈢其他
        ⅰ启动电流调节定时器
        ⅱ子状态 = INIT_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：ch_disabled_ch_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-16          戴辉发             创建
==============================================================*/
static void ch_disabled_ch_indication(void)
{
	if (g_ch_grand_status == E_CH_DISABLED_STATUS)
	{
		uint16_t max_vol = get_max_cell_vol();

		g_full_timer_flag = 0;
		/*     ①电流调整次数 = 0 */
		g_cur_adjust_num = 0;
		/*     ②调整电压 = 额定电压 */
		g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
		/*     ③调整电流 = 0.05C */
		g_ch_adjust_current = CH_MIN_CURRENT_VALUE; /* 设定初始电流为8A */
		/*     ⑥父状态 = ENABLED_STATUS */
		g_ch_grand_status = E_CH_ENABLED_STATUS;
		/*     ⑤启动再次判决定时器 */
		judge_timer = CH_60S_TIME / 2;
	}
}

/* status: E_CH_AUTH_ENABLED_STATUS,消息处理函数表 */
/* E_CH_IND, 充电指示 */
/***********************************************************
⒈使能请求
  ⑴停止
    ①清除再次判决定时器
    ②清除满充判决定时器
    ③清除充电维护限时定时器
    ④清除电流调节定时器
    ⑤状态迁移
      ㈠父状态 = DISABLED_STATUS
      ㈡子状态 = INIT_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：ch_enabled_ch_indication
 * 参数个数：1
 * 函数参数：
 *          [IN]       pmsg         消息内内容
 * 返回值：
 *          无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2019-03-06          戴辉发             创建
==============================================================*/
static void ch_enabled_ch_indication(void)
{
	if (g_ch_grand_status == E_CH_ENABLED_STATUS)
	{
		g_ch_adjust_current = 0;
		/*     ②清除再次判决定时器 */
		judge_timer = 0;
		/*     ③清除满充判决定时器 */
		full_timer = 0;
		g_full_timer_flag = 0;
		/*     ⑦状态迁移 */
		/*       ㈠父状态 = DISABLED_STATUS */
		g_ch_grand_status = E_CH_DISABLED_STATUS;
		/*       ㈡子状态 = INIT_STATUS */
		g_ch_status = E_CH_INIT_STATUS;

        chargerOverWakeUpSleepQuick = 0;
		send_closed_rxpdo_to_charger();
	}
}

/**************************************************************************/
/************************init status deal**********************************/
/**************************************************************************/
/*=============================================================
 * 函数名称：ch_init_status_msg_process
 * 函数功能：充电器模块充电初始状态事件表
 * 参数个数：2
 * 参数描述：
 *          [IN]     msg_type        消息类型
 *          [IN]     pmsg            消息内容
 * 返 回 值：
 *          无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人            修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
static void ch_init_status_msg_process(void)
{
	uint16_t max_vol = get_max_cell_vol();
    uint16_t avl_vol = get_average_vol();

    /* 调整电压 = 额定电压 */
    g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
        
    if (E_VOL_OVER == get_vol_status())
	{
	   /* 调整电流 */
		g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
	   /*  子状态 = E_CH_IDLE_STATUS */
		g_ch_status = E_CH_IDLE_STATUS;
	}
	else if (( max_vol >= PROTECT_VOLTAGE ) || ( g_run_sys_data.total_vol >= 10*CH_MAX_VOLTAGE_VALUE ))
	{/* 已经满电了，最小充电电流 */
        if( voltageFilteredDelay == 0 )
        {
            /* 调整电流 */
            g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
           /*     ③子状态 = E_CH_IDLE_STATUS */
            g_ch_status = E_CH_IDLE_STATUS;
        }
	}
	else if ((max_vol >= BOTTOM_VOLTAGE)|| (avl_vol >= BOTTOM_AVL_VOLTAGE))
	{/* 进入平台期了，以这个电流继续充电，到截至 */
	    if( voltageFilteredDelay == 0 )
        {
            /*     ③子状态 = E_CH_IDLE_STATUS */
            g_ch_status = E_CH_IDLE_STATUS;
        }
	}	
	else
	{
        voltageFilteredDelay = CH_VOLTAGE_FILTER_TIME;
		if ( 0 == judge_timer )
		{
			/* 启动电流调节定时器 */
			judge_timer = CH_10S_TIME;
			/* ⒉电流调节次数 += 1 */
			g_cur_adjust_num += 1;
			/* ⒊调整电流 = 电流向前步进一步 */
			g_ch_adjust_current += 10;
			/* ⒋调整电流 >= 额定电流 */
			if ( g_ch_adjust_current >= CH_MAX_CURRENT_VALUE )
			/*   ⑴真 */
			{
			/*     ①调整电流 = 额定电流 */
				g_ch_adjust_current = CH_MAX_CURRENT_VALUE;
			/*     ②电流调节次数 = 0 */
				g_cur_adjust_num = 0;
			/*     ③子状态 = IDLE_STATUS */
				g_ch_status = E_CH_IDLE_STATUS;
			}
		}
	}
}

/**************************************************************************/
/************************idle status deal**********************************/
/**************************************************************************/
/*=============================================================
 * 函数名称：ch_idle_status_msg_process
 * 函数功能：充电器模块充电空闲状态事件表
 * 参数个数：2
 * 参数描述：
 *          [IN]     msg_type        消息类型
 *          [IN]     pmsg            消息内容
 * 返 回 值：
 *          无
 * 修改记录：
 *===============================================================
 * 日    期                     修改人            修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
static void ch_idle_status_msg_process(void)
{
	uint16_t max_vol = get_max_cell_vol();
    uint16_t avl_vol = get_average_vol();
    
	/* ⒈最高电压 */
	if ( E_VOL_OVER == get_vol_status() )
	{
	/*     ①调整电压 = 额定电压 */
		g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
	/*     ②调整电流 = 均衡电流 */
		g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
	/*     ③子状态 = BOTTOM_STATUS */
		g_ch_status = E_CH_BOTTOM_STATUS;
	/*     ⑥启动再次判决定时器 */
		judge_timer = CH_10S_TIME;
	}
	else 
    {
        if (( max_vol >= BOTTOM_VOLTAGE )|| (avl_vol >= BOTTOM_AVL_VOLTAGE))
        {/*  开始去降低电流了 */
        /*     ③子状态 = E_CH_IDLE_STATUS */
            if( voltageFilteredDelay == 0 )
            {
                judge_timer = 0;
                g_ch_status = E_CH_BOTTOM_STATUS;
                full_timer = CH_10S_TIME;
                voltageFilteredDelay = CH_VOLTAGE_FILTER_TIME;
            }
        }
        else
        {
            voltageFilteredDelay = CH_VOLTAGE_FILTER_TIME;
        }
        
    }
    	
   
}

/**************************************************************************/
/************************bottom status deal********************************/
/**************************************************************************/
/*=============================================================
 * 函数名称：ch_bottom_status_msg_process
 * 函数功能：充电器模块充电底端状态事件表
 * 参数个数：2
 * 参数描述：
 *          [IN]     msg_type        消息类型
 *          [IN]     pmsg            消息内容
 * 返 回 值：
 *          无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人            修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
static void ch_bottom_status_msg_process( void )
{
    uint16_t max_vol = get_max_cell_vol();
    uint16_t avl_vol = get_average_vol();
    
    if ( max_vol >= PROTECT_VOLTAGE )
    {/*维持10s，则降低电流*/
        if ( 0 == judge_timer )
        {/* ⒈重置再次判决定时器 */
            judge_timer = CH_10S_TIME;
            g_ch_adjust_current *= 0.75;
            if (g_ch_adjust_current < CH_MIN_CURRENT_VALUE)
            {
                g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
            }
        }
        
    }
    else
    {
        /* ⒈重置再次判决定时器 */
        judge_timer = CH_10S_TIME;
    }
    
    if (E_VOL_OVER == get_vol_status())
    /* 充电状态下过压保护 */
    {
        /*     ①调整电压 = 额定电压 */
        g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
    /*     ②调整电流 = 均衡电流 */
       /* g_ch_adjust_current = 0; 防止休眠唤醒后，实际电压已经跌落，但是过压
       还没有恢复，导致的，充电电流为0 */
        g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
    }
    else if (max_vol >= (get_cell_vol_high_protect() - 20))
    /*   ⑴ >= 充电保护电压 */
    {
        if( voltageFilteredDelay == 0 )
        {
        /*     ①调整电压 = 额定电压 */
            g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
        /*     ②调整电流 = 均衡电流 */
            g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
        }
    }
    else if (max_vol >= (get_cell_vol_high_protect() - 50))
    /*   ⑴ >= 充电保护电压 */
    {
        if( voltageFilteredDelay == 0 )
        {
        /*     ①调整电压 = 额定电压 */
            g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
        /*     ②调整电流 = 均衡电流 */
            g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
        }
    }
    else 
    {
        voltageFilteredDelay = CH_VOLTAGE_FILTER_TIME;
    /*     ①调整电压 = 额定电压 */
        g_ch_adjust_vol = CH_MAX_VOLTAGE_VALUE;
    }
    
    
    if (E_VOL_OVER == get_vol_status())
    /* 充电状态下过压保护 */
    {
        /*     ①满充判决定时器标志 */
        if (g_full_timer_flag == 0)
        /*       ㈠假 */
        {
            if( 0 == full_timer )
            {/*等待10s，看过压保护是否恢复*/
                /* ⒊清除再次判决定时器 */
                    judge_timer = 0;
                /*         ⅰ启动满充判决定时器 */
                    full_timer = CH_3S_TIME * 2;
                /*         ⅱ满充判决定时器标志 = 真 */
                    g_full_timer_flag = 1;
            } 
            else if( full_timer > CH_10S_TIME )
            {
                full_timer = CH_10S_TIME;
            }
        }
        else
        {
            if (full_timer > CH_3S_TIME * 2)
            {
                full_timer = CH_3S_TIME * 2;
            }
        }
    }
    else 
    {/*
        1.电压大于（28V铁锂） （29V 三元） 考虑单体过压的话 还是 不以电压为准
        2.电流小于1A
        3.时间长于3s
      */
        int16_t current = get_cur_current();
        
        chargerOverWakeUpSleepQuick = 0;
        if ( ( 1 == get_ch_switch_status() ) && ((g_run_sys_data.total_vol > (360 * BAT_NUM))||( max_vol >= get_cell_vol_high_protect() - 30 ))&& \
            (current < CH_MIN_CURRENT_VALUE  ) )  // <0.8A充电
        {       
               if( full_timer == 0 )
               {
                  /*     ①满充判决定时器标志 */
                  if ( g_full_timer_flag == 0 )
              /*       ㈠假 */
                  { 
              /*         ⅰ启动满充判决定时器 */
                      full_timer = CH_3S_TIME;
              /*         ⅱ满充判决定时器标志 = 真 */
                      g_full_timer_flag = 1;
                  }
               } 
        }
        else
        {
                  full_timer = CH_3S_TIME;//5min太长
             /*     ①满充判决定时器标志 = 假 */
                  g_full_timer_flag = 0;
        }
       
    }
    
	if ( 1 == g_full_timer_flag )
	{
		if (0 == full_timer)
		{
			/* ⒈清除满充判决定时器 */
			full_timer = 0;
			/* ⒊清除再次判决定时器 */
			judge_timer = 0;
			/* ⒋电池包均衡判决 */
			if (( 1 == judge_balance_status() )&&( 0 == chargerOverWakeUpSleepQuick ))
			/*   ⑴均衡 */
			{
				g_ch_adjust_current = CH_MIN_CURRENT_VALUE;
			/*     ①子状态 = BALANCE_STATUS */
				g_ch_status = E_CH_BALANCE_STATUS;
#if (defined(TIANFENG) || defined(TIANHONG)) && defined(BAT_8S)
                g_ch_status = E_CH_FINISHED_STATUS;
                chargerOverWakeUpSleepQuick = 1; 
                g_ch_adjust_current = 0;
#endif
			/*     ②启动均衡定时器 */
				judge_timer = CH_1H_TIMER;
			}
			else
			/*   ⑵非均衡 */
			{
				g_ch_adjust_current = 0;
			/*     ①子状态 = FINISHED_STATUS */
				g_ch_status = E_CH_FINISHED_STATUS;
                chargerOverWakeUpSleepQuick = 1; 
			}
            /* 充电满充完成 */
            g_ch_full_flag = 1;
		}
	}
    
}
/**************************************************************************/
/**************************balance status deal*****************************/
/**************************************************************************/
/*=========================================================================
 * 函数名称：ch_balance_status_msg_process
 * 函数功能：充电器模块维护状态事件表
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          无
 * 修改记录：
 *=========================================================================
 * 日    期          修改人           修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
static void ch_balance_status_msg_process(void)
{
	/* ⒈压差(各电芯包压差) */
	if ((1 != judge_balance_status()) || (0 == judge_timer))
	/*   ⑴压差正常 */
	{
	/*     ①调整电流 = 0 */
		g_ch_adjust_current = 0;
	/*     ③子状态 = FINISHED_STATUS */
		g_ch_status = E_CH_FINISHED_STATUS;
		judge_timer = 0;
        chargerOverWakeUpSleepQuick = 1; 
	}
}

/*=============================================================
 * 函数名称：judge_ch_small_current
 * 函数功能：判决是否为充电小电流
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0        不符合小电流
 *          1        符合小电流
 * 修改记录：
 *===============================================================
 * 日    期          修改人           修改类型
 * 2020-10-23        戴辉发           创建
==============================================================*/
static uint8_t judge_ch_small_current(int16_t curr)
{
	uint8_t ret = 0;
    uint8_t smallCurr = 10;
#if defined(TIANFENG) && defined(BAT_8S)
    smallCurr = 50;
#else
    smallCurr = 10;
#endif

	if (( 1 == get_ch_switch_status() ) &&(curr < smallCurr ) && (curr >= 0))
	{
		ret = 1;
	}
	return ret;
}

/*=============================================================
 * 函数名称：soc_update_process
 * 函数功能：SOC充电末端校准
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          无
 * 修改记录：
 *===============================================================
 * 日    期          修改人           修改类型
 * 2018-10-20        戴辉发           创建
==============================================================*/
void soc_update_process(void)
{
#if defined (BATTARY_LFP)

	if (E_CHARGE_STATUS == get_system_status() && (1 == get_ch_switch_status()))
	/* 充电状态，充电功率开关合上 */
	{
		switch (g_soc_update_status)
		{
		case 0:
			if (0 <= get_cur_current())
			{
				g_soc_update_status = 1;
				soc_full_timer = CH_10S_TIME;
                Soc95AdjustDelay = 10;
			}
			break;
		case 1:
			{
				uint16_t max_vol = get_max_cell_vol();
				uint16_t avl_vol = get_average_vol();
                uint32_t totalVol = (get_total_vol() / 100);
                
#if defined(TIANFENG) && defined(BAT_8S)
                if ((max_vol >= BOTTOM_VOLTAGE) || (totalVol >= CH_MAX_VOLTAGE_VALUE))
                    /*       ㈠ >= 95%电压(恒流百分比) */
				{
                    if(  Soc95AdjustDelay == 0 )
                    {
                        force_set_soc(95, 1);
                        g_soc_update_status = 2;
                        soc_full_timer = CH_5S_TIME;
                    }
				}
                else
                {
                    Soc95AdjustDelay = 100;
                }

#else
                if ((max_vol >= BOTTOM_VOLTAGE) && (avl_vol >= BOTTOM_AVL_VOLTAGE))
                    /*       ㈠ >= 95%电压(恒流百分比) */
                {
                    if(  Soc95AdjustDelay == 0 )
                    {
                        force_set_soc(95, 1);
                        g_soc_update_status = 2;
                        soc_full_timer = CH_10S_TIME;
                    }
                }
                else
                {
                    Soc95AdjustDelay = 100;
                }
                /* 充电器电压偏低，需要根据电流判决满充状态 */
                if ((1 == judge_ch_small_current(get_cur_current())) && 
                    /*     ①满充判决定时器标志 */
                    ( CH_MAX_VOLTAGE_VALUE <= (get_total_vol() / 100)))
                    /*   ⑴真 */
                {
                    if ( 0 == soc_full_timer )
                    {
                        force_set_soc(100, 1);
                        g_soc_update_status = 3;
                    }
                }
                else
                    /*   ⑵假 */
                {
                    /*     ①清除满充判决定时器 */
                    soc_full_timer = CH_10S_TIME;
                }
#endif
            }
            break;
		case 2:
            {
#if defined(TIANFENG) && defined(BAT_8S)    /* 天丰要求SOC置百条件 */
                uint16_t max_vol = get_max_cell_vol();
				uint16_t avl_vol = get_average_vol();
                uint32_t totalVol = get_total_vol();
                if((totalVol > (3600 * BAT_NUM)) || (max_vol >= get_cell_vol_high_protect() - 30))
#endif
                    if ( 1 == judge_ch_small_current(get_cur_current()))
                        
                        /*     ①满充判决定 时器标志 */
                        /*   ⑴真 */
                    {
                        if (0 == soc_full_timer)
                        {
                            force_set_soc(100, 1);
                            g_soc_update_status = 3;
#if defined(TIANFENG) && defined(BAT_8S)
                            SetFullSocFlag(2);/* 设置SOC可置百标志 */
#endif
                        }
                    }
                    else
                        /*   ⑵假 */
                    {
                        /*     ①清除满充判决定时器 */
                        soc_full_timer = CH_10S_TIME;
                    }
            }
			break;
		case 3:
			break;
		}
	}
	else
	{
        Soc95AdjustDelay = 100;
		g_soc_update_status = 0;
	}
#endif

}
