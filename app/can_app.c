/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：can_app.c
* 文件功能描述：实现can app功能
*
* 修改记录：
* 2018-07-11 戴辉发 创建
*----------------------------------------------------------*/
#include "can_app.h"
#include "parameter.h"
#include "can.h"
#include "system_control.h"
#include "soc.h"
#include "vol_manage.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "system_adjust.h"
#include "system_control.h"
#include "parameter.h"
#include "temp_manage.h"
#include "states.h"
#include "can_open_timer.h"
#include "pdo.h"
#include "balance.h"
#include "switch_status.h"
#include "fault_manage.h"
#include "short.h"
#include "mode_manage.h"
#include "vol_curr_addi_deal.h"
#include "power.h"
#include "fault_manage.h"
#include "rtc.h"
#include "eeprom.h"
#include "final_protect.h"
#include "low_power.h"
#include "afe_app.h"
#include "soc.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "storage_manage.h"
#include "Appcommunication.h"
#include "AppVolManage.h"
#include "ch_addition.h"
#include "AppTempManage.h"
#include "AppCurrManage.h"
#include "ChHeart.h"
#include "chargerProtocol.h"

#define SYSTEM_STATUS_NODE_ID     (0x280 + NODE_ID)
#define CELL_VOL_NODE_ID          (0x380 + NODE_ID)
#define TEMP_VALUE_NODE_ID        (0x480 + NODE_ID)

uint8_t  HandshakeState;
static uint8_t handshake_status;
static uint8_t handshake_delay;

static uint8_t g_can_app_s_timer;
static uint8_t g_can_app_20ms_timer;
static uint8_t sendTHJLPdoDelay;
static volatile uint16_t RY_CommDelay;
static volatile uint8_t RY_step;

int16_t g_report_current;
uint8_t g_report_status;
uint8_t g_report_soc;
uint32_t RandomData;
uint32_t EncryptedCRCdata;
uint8_t  HandshakeState;
uint8_t  recived180MessageDelay;
extern UNS8 bDeviceNodeId;

static void voltage_event_process(void);
static void current_event_process(void);
static void temperature_event_process(void);
static void balance_event_process(void);
static void capcity_event_process(void);
static void other_event_process(void);

static void system_status_report(void);
/*==============================================================
* 函数名称：handshake_awakeup_init
* 函数功能：CAN APP内存初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-06-22          李勇             创建
==============================================================*/
void handshake_awakeup_init(void)
{
    handshake_status = 0;
    handshake_delay = 0;
    recived180MessageDelay = 0;
}
/*==============================================================
* 函数名称：can_app_mem_init
* 函数功能：CAN APP内存初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-11          戴辉发             创建
==============================================================*/
void can_app_mem_init(void)
{
    sendTHJLPdoDelay = 20;
	g_can_app_s_timer = 0;
	g_report_status = 0;
    g_report_soc = 0;
    g_can_app_20ms_timer = 0;
    RY_CommDelay = 0;
    RY_step = 0;
    handshake_awakeup_init();
}

/*==============================================================
* 函数名称：can_app_timer_s_timer
* 函数功能：CAN APP内存初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-11          戴辉发             创建
==============================================================*/
void can_app_timer_s_timer(void)
{
	if (g_can_app_s_timer) g_can_app_s_timer --;
    if (g_can_app_20ms_timer) g_can_app_20ms_timer --;
    if(sendTHJLPdoDelay)sendTHJLPdoDelay--;
    if(RY_CommDelay)RY_CommDelay--; 
}

/*==============================================================
 * 函数名称：voltage_event_process
 * 函数功能：电压事件处理
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-09-09          戴辉发             创建
==============================================================*/
static void voltage_event_process(void)
{
	e_voltage_status vol_status = get_total_vol_status();
	e_voltage_status cell_vol_status = get_cell_vol_status();

	g_run_sys_data.vol_event = 0;

    if (E_VOL_OVER_ALARM == cell_vol_status)
    {
        g_run_sys_data.vol_event |= 0x01;
    }
    
    if (E_VOL_OVER == cell_vol_status)
    {
        g_run_sys_data.vol_event |= 0x02;
    }

    if (E_VOL_UNDER_ALARM == cell_vol_status)
    {
        g_run_sys_data.vol_event |= 0x04;
    }

    if (E_VOL_UNDER == cell_vol_status)
    {
        g_run_sys_data.vol_event |= 0x08;
    }

    if (E_VOL_OVER_ALARM == vol_status)
    {
        g_run_sys_data.vol_event |= 0x10;
    }

    if (E_VOL_OVER == vol_status)
    {
        g_run_sys_data.vol_event |= 0x20;
    }

    if (E_VOL_UNDER_ALARM == vol_status)
    {
        g_run_sys_data.vol_event |= 0x40;
    }

    if (E_VOL_UNDER == vol_status)
    {
        g_run_sys_data.vol_event |= 0x80;
    }
}

/*==============================================================
* 函数名称：current_event_process
* 函数功能：电流事件处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-09-09          戴辉发             创建
==============================================================*/
static void current_event_process(void)
{
	e_current_status cur_status = get_current_status();

	g_run_sys_data.cur_event = 0;

	if (CURRENT_IN_CHARGE_ALARM(cur_status))
	{
		g_run_sys_data.cur_event |= 0x01;
	}

    if (CURRENT_IN_CHARGE_PROTECT(cur_status))
	{
		g_run_sys_data.cur_event |= 0x02;
	}

    if (CURRENT_IN_DISCHARGE_ALARM(cur_status))
    {
        g_run_sys_data.cur_event |= 0x04;
    }

    if (CURRENT_IN_DISCHARGE_PROTECT(cur_status))
    {
        g_run_sys_data.cur_event |= 0x08;
    }

    if (CURRENT_IN_DISCHARGE_SECOND(cur_status))
    {
        g_run_sys_data.cur_event |= 0x10;
    }

    /* 短路状态 */
    if (( 0 != get_short_status() ))
    {
        g_run_sys_data.cur_event |= 0x20;
    }

    if (E_SWITCH_INVALID == get_ch_switch_flag())
    /* 充电开关失效 */
    {
        g_run_sys_data.cur_event |= 0x40;
    }

    if (E_SWITCH_INVALID == get_dch_switch_flag())
    /* 放电开关失效 */
    {
        g_run_sys_data.cur_event |= 0x80;
    }
}

/*==============================================================
* 函数名称：temperature_event_process
* 函数功能：温度事件处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-09-09          戴辉发             创建
==============================================================*/
static void temperature_event_process(void)
{
	e_item_status ch_temp_status = get_ch_temp_status();
	e_item_status dch_temp_status = get_dch_temp_status();
	e_item_status ev_temp_status = get_environ_temp_status();
	e_item_status power_temp_status = get_power_temp_status();

	g_run_sys_data.temp_event = 0;

	if ((E_TEMP_LOW_ALARM == ch_temp_status) || 
		(E_TEMP_HIGH_ALARM == ch_temp_status) || 
		(E_TEMP_LOW_ALARM == dch_temp_status) || 
		(E_TEMP_HIGH_ALARM == dch_temp_status))
	{
		g_run_sys_data.temp_event |= 0x01;
	}
	if ((E_TEMP_LOW_PROTECT == ch_temp_status) || (E_TEMP_HIGH_PROTECT == ch_temp_status))
	{
		g_run_sys_data.temp_event |= 0x02;
	}
	if ((E_TEMP_LOW_PROTECT == dch_temp_status) || (E_TEMP_HIGH_PROTECT == dch_temp_status))
	{
		g_run_sys_data.temp_event |= 0x04;
	}
	if ((E_TEMP_LOW_ALARM == ev_temp_status) || (E_TEMP_HIGH_ALARM == ev_temp_status))
	{
		g_run_sys_data.temp_event |= 0x08;
	}
	if ((E_TEMP_LOW_PROTECT == ev_temp_status) || (E_TEMP_HIGH_PROTECT == ev_temp_status))
	{
		g_run_sys_data.temp_event |= 0x10;
	}
	if ((E_TEMP_LOW_ALARM == power_temp_status) || (E_TEMP_HIGH_ALARM == power_temp_status))
	{
		g_run_sys_data.temp_event |= 0x40;
	}
	if ((E_TEMP_LOW_PROTECT == power_temp_status) || (E_TEMP_HIGH_PROTECT == power_temp_status))
	{
		g_run_sys_data.temp_event |= 0x80;
	}
}

/*==============================================================
* 函数名称：balance_event_process
* 函数功能：均衡事件处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-09-09          戴辉发             创建
==============================================================*/
static void balance_event_process(void)
{
	g_run_sys_data.ba_event = 0;

	if (judge_balance_status() )
	{
		g_run_sys_data.ba_event |= 0x01;
	}  

    if ( 2 == g_system_protect.vol_low_status )   
    {
        g_run_sys_data.ba_event |= 0x02;
    }

    if ( 2 == g_system_protect.vol_high_status )
    {
        g_run_sys_data.ba_event |= 0x04;
    }
    
	if (E_BAT_FAIL == get_bat_status())
	{
		g_run_sys_data.ba_event |= 0x10;
	}
}

/*==============================================================
* 函数名称：capcity_event_process
* 函数功能：容量事件处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-09-09          戴辉发             创建
==============================================================*/
static void capcity_event_process(void)
{
	g_run_sys_data.cap_event = 0;	
	if(E_PROTECT_SOC == get_soc_status())
	{
		g_run_sys_data.cap_event |= 0x01;
	}
    if(E_LOW_SOC == get_soc_status())
	{
		g_run_sys_data.cap_event |= 0x02;
	}
    /* 主电源状态，0：正常，1：未打开 */
    if(get_main_power_detect_status() == 0)
    {
        g_run_sys_data.cap_event |= 0x04;
    }
    /* AFE检测总压与软件检测总压差距 0：相近 1：差距大 */
    if(get_vol_two_comp())
    {
        g_run_sys_data.cap_event |= 0x08;
    }
    /* AFE检测电流与电压比对 0：正常  1：异常 */
    if(0 != get_curr_afe_flag())
    {
        g_run_sys_data.cap_event |= 0x10;
    }
    /* AFE通讯状态 0：通讯正常  1：通讯异常 */
    if(1 == get_afe_status())
    {
        g_run_sys_data.cap_event |= 0x20;
    }
    /* bit6:存储器故障指示 0：无故障  1：有故障 */
    if(get_eeprom_status())
    {
        g_run_sys_data.cap_event |= 0x40;
    }
    /* bit7: AFE电压不变 0：正常 1：异常 */
    if(2 == get_vol_afe_status())
    {
        g_run_sys_data.cap_event |= 0x80;
    }
}

/*==============================================================
* 函数名称：other_event_process
* 函数功能：其他事件处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-09-09          戴辉发             创建
==============================================================*/
static void other_event_process(void)
{
	e_system_status sys_status = get_system_status();

	g_run_sys_data.other_event = 0;

    if( get_system_mode_status() != E_MODE_SLEEP_STATUS )
    {
         if (E_CHARGE_STATUS == sys_status)
        {
            g_run_sys_data.other_event = 0x02;
        }
        else if (E_DISCHARGE_STATUS == sys_status)
        {
            g_run_sys_data.other_event = 0x01;
        }
        else if (E_ABATE_STATUS == sys_status)
        {
            g_run_sys_data.other_event = 0x03;
        }
    }
    else
    {
       g_run_sys_data.other_event = 0x04;
    }

    if(get_current_offset_poweron() == 0)
    {
        g_run_sys_data.other_event |= 0x08;
        protect_code[6] |= 0x08;
    }
    else
    {
        protect_code[6] &= ~0x08;
    }

	if( get_dch_switch_status() )
	{
		g_run_sys_data.other_event |= 0x10;
	}

	if( get_ch_switch_status() )
	{
		g_run_sys_data.other_event |= 0x20;
	}

    if(0 == get_load_status())
	{
		g_run_sys_data.other_event |= 0x40;
        protect_code[7] |= 0x04;
	}  
    else
    {
        protect_code[7] &= ~0x04; 
    } 
    //bit7:盲充检测，0：移除，1：充电机接上   
    if ( 1 == get_ch_remove_status() )
    { 
        g_run_sys_data.other_event |= 0x80;
        protect_code[7] |= 0x08; 
    }
    else
    {
        protect_code[7] &= ~0x08; 
    } 
}

/*==============================================================
 * 函数名称：new_event1_process
 * 函数功能：新增事件1处理
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-06-01          李勇               创建
==============================================================*/
static void new_event1_process(void)
{
    uint16_t flag = get_final_set_switch();
    g_run_sys_data.new_event1 = 0x0;

    // bit0:AFE过压保护
    if((flag & 0x04))
    g_run_sys_data.new_event1 |= 0x01;
    // bit1:AFE欠压保护
    if((flag & 0x08))
        g_run_sys_data.new_event1 |= 0x02;
    // bit2:AFE过流保护
    if((flag & 0x20) || (flag & 0x40))
        g_run_sys_data.new_event1 |= 0x04;
    // bit3:AFE短路保护
    if((flag & 0x10))
        g_run_sys_data.new_event1 |= 0x08;
    // bit4:AFE其它保护
    if((flag & 0x80) || (flag & 0x100) || (flag & 0x200))
        g_run_sys_data.new_event1 |= 0x10;

    // bit5:预充继电器状态 0：断开 1：闭合
    //if (1 == FL_GPIO_GetOutputPin(PRECHARGER_RELAY_GPIO, PRECHARGER_RELAY_GPIO_PIN))
    //    g_run_sys_data.new_event1 |= 0x20;
    // bit6:二次保护继电器状态 0：断开 1：闭合
    if ((1 == FL_GPIO_GetOutputPin(RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO)) && (get_backup_relay_in() == 1))
        g_run_sys_data.new_event1 |= 0x40;
    // bit7:加热继电器状态 0：断开 1：闭合
    if (1 == FL_GPIO_GetOutputPin(HEAT_RELAY_CONTROL_GPIO, HEAT_RELAY_CONTROL_GPIO_PIN))
        g_run_sys_data.new_event1 |= 0x80;
}

/*==============================================================
* 函数名称：new_event2_process
* 函数功能：新增事件2处理
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2020-06-01           李勇              创建
==============================================================*/
static void new_event2_process(void)
{

    g_run_sys_data.new_event2 = 0x0;
    //bit0:干接点输入状态 0：低电平 1：高电平
   if( 0 != recived180MessageDelay ) 
   {
      g_run_sys_data.new_event2 |= 0x01;
   }
    /* bit1:干接点输出状态 0：低电平 1：高电平 */ 

    /* bit2:电流采集状态 0：正常 1：异常 */
    if(E_C_FAULT == get_current_status()) 
    {
        g_run_sys_data.new_event2 |= 0x04;
    }
    /* bit3: 状态互斥检测 0：正常 1：互斥 */
    if( get_status_error_flag() ) 
    {
        g_run_sys_data.new_event2 |= 0x08;
    }

    if( get_backup_relay_fault() == 1 )
    {
        g_run_sys_data.new_event2 |= 0x10;
    }

    if( get_backup_relay_fault() == 2 )
    {
        g_run_sys_data.new_event2 |= 0x20;
    }

    if( get_backup_relay_in() )
    {
        g_run_sys_data.new_event2 |= 0x40;
    }
}

/*==============================================================
* 函数名称：system_status_report
* 函数功能：系统状态上报
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-11          戴辉发             创建
==============================================================*/
static void system_status_report(void)
{
	g_report_status = 0;

	/* 过压保护标志 */
	if (E_CHARGE_STATUS == get_system_status())
	{
		if (E_VOL_OVER == get_vol_status())
		{
			g_report_status |= 0x01;
		}
	}

	/* Over Discharge或者充放电MOS失效 */
	if (E_CHARGE_STATUS != get_system_status())
	{
		if ((E_PROTECT_SOC == get_soc_status()) || 
            (E_SWITCH_INVALID == get_dch_switch_flag()) || 
            (E_SWITCH_INVALID == get_ch_switch_flag()))
		{
			g_report_status |= 0x02;
		}
	}

	/* Communication Outage */
    if ( ((3 == get_curr_status())|| ( 0 != get_short_status() )) || 
         (0 != get_curr_afe_flag()) || 
         (0 < get_vol_afe_status()) || 
         (E_BAT_FAIL == get_bat_status()))
    {
        g_report_status |= 0x04;
    }

	/* Under Voltage */
	if (E_CHARGE_STATUS != get_system_status())
	{
		if ( E_VOL_UNDER == get_vol_status() )
		{
			g_report_status |= 0x08;
		}
	}

	/* Over Current */
	if ( E_CHARGE_STATUS == get_system_status() )
	{
		if ( E_C_CH_PROTECT == get_current_status() )
		{
			g_report_status |= 0x10;
		}
	}
	else
	{
		if (E_C_DCH_PROTECT == get_current_status() || 
			E_C_SECOND_PROTECT == get_current_status() ||
            E_C_FAULT ==  get_current_status() ||
            2 == get_current_power_flag() || 
            ((3 == get_curr_status())|| ( 0 != get_short_status() )) )
		{
			g_report_status |= 0x10;
		}
	}

	/* Over Temperature Protect */
	if (E_CHARGE_STATUS != get_system_status())
	{
		if (E_TEMP_DISDCH_STATUS == get_temperature_status() || 
			E_TEMP_DISCHDCH_STATUS == get_temperature_status())
		{
			g_report_status |= 0x20;
		}
	}
	else
	{
		if (E_TEMP_DISCH_STATUS == get_temperature_status() || 
			E_TEMP_DISCHDCH_STATUS == get_temperature_status())
		{
			g_report_status |= 0x20;
		}
	}

	/* Temperature Protect */
    /* 做其它故障的 */
    if((E_BAT_FAIL == get_bat_status()) || 
        /* AFE通讯失败 */
        (1 == get_afe_status()) || 
        /* AFE电压检测不一致 */
        (0 != get_vol_two_comp()) || 
        /*充电状态互斥 */
        (2 == get_status_error_flag()))
    {
        g_report_status |= 0x40;
    }

	/* Battery Charging Status */
	if ((E_CHARGE_STATUS == get_system_status()) || 
        (E_SWITCH_INVALID == get_dch_switch_flag()) || 
        (E_SWITCH_INVALID == get_ch_switch_flag()) || 
        (1 < get_curr_afe_flag()) || 
        (1 < get_vol_afe_status())|| 
        (1 == get_cutoff_flag())|| 
        (1 == get_send_charger_flag()))
	{
		g_report_status |= 0x80;
	}

	/* 电流 */
	if ( get_cur_current() < 0 )
	{
		g_report_current = (uint16_t)((0 - get_cur_current()));
	}
	else
	{
		g_report_current = (uint16_t)(get_cur_current());
	}

    if ((E_SWITCH_INVALID == get_ch_switch_flag()) || 
        (E_SWITCH_INVALID == get_dch_switch_flag()))
    /* MOS失效，强制报SOC为0 */
    {
        g_report_soc = 1;
    }
    else
    {
        g_report_soc = (uint8_t)g_run_sys_data.soc;
    }
}

/*=============================================================
 * 函数名称：canopen_init
 * 函数功能：CANOPEN内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-23          戴辉发             创建
==============================================================*/
void canopen_init( void )
{
    can_mem_init();
	setNodeId(NODE_ID);
}

/*=============================================================
 * 函数名称：event_deal_process
 * 函数功能：事件处理流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2019-08-09          戴辉发             创建
==============================================================*/
void event_deal_process(void)
{
	/* 形成上报系统状态 */
	voltage_event_process();
	current_event_process();
	temperature_event_process();
	balance_event_process();
	capcity_event_process();
	other_event_process();
    new_event1_process();
    new_event2_process();
	system_status_report();
}
/*=============================================================
 * 函数名称：send_protectcode
 * 函数功能：事件功能
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-06-14          李勇                创建
==============================================================*/
void send_protectcode(void)
{
   if(g_can_app_20ms_timer) return;
   g_can_app_20ms_timer = 20;
   can_std_transmit(0x2F1, protect_code, 0, 8);
}

/*=============================================================
 * 函数名称：send_addition_data
 * 函数功能：事件功能
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-11-11          李勇                创建
==============================================================*/
void send_addition_data( void )
{
   uint8_t data[8];
   uint16_t temp = GetTotalVoltage();
   data[0] = temp;
   data[1] = temp>>8;
   temp = get_min_cell_vol();
   data[2] = temp;
   data[3] = temp>>8;
   temp = get_max_cell_vol();
   data[4] = temp;
   data[5] = temp>>8;
   data[6] = 0;
   data[7] = 0;
   can_std_transmit(0x2F2,data,0,8);
}

/*==============================================================
 * 函数名称：send_read_random_data
 * 函数功能：发送读取随机数据请求命令
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2019-11-11        戴辉发     	    创建
==============================================================*/
static void send_read_random_data(void)
{
    uint8_t buf[8];
   
    /* CCS = 2 */
    buf[0] = 0x40;
    /* index = 0x5000 */
    buf[1] = 0x00;
    buf[2] = 0x50;
    /* subindex = 0x00 */
    buf[3] = 0x00;
    /* 保留 */
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;

    can_std_transmit(0x600 + CONTROL_ID, buf, 0, 8);
}

/*==============================================================
 * 函数名称：send_read_handshake_status
 * 函数功能：发送读取握手信号状态命令
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2019-11-11        戴辉发     	    创建
==============================================================*/
static void send_read_handshake_status(void)
{
    uint8_t buf[8];
      /* CCS = 2 */
      buf[0] = 0x40;
      /* index = 0x5003 */
      buf[1] = 0x03;
      buf[2] = 0x50;
      /* subindex = 0x00 */
      buf[3] = 0x00;
      /* 保留 */
      buf[4] = 0x00;
      buf[5] = 0x00;
      buf[6] = 0x00;
      buf[7] = 0x00;

      can_std_transmit(0x600 + CONTROL_ID, buf, 0, 8);
}

void can_handshakestate_process()
{
    if((1 == get_dch_switch_status()) && (E_CHARGE_STATUS != get_system_status())) 
    {
        switch( handshake_status )
        {
        case 0:/* Idle State (initialization)等待手柄 */
            handshake_delay++;
            if( handshake_delay > 10 ) 
            {
                handshake_delay = 0;
                handshake_status = 2;
            }
            send_read_random_data();
            send_read_handshake_status();
            break;
        case 1:/* Handshake Passed State 握手成功 */
            send_read_handshake_status();
            break;
        case 2:/* Handshake Failed State 握手不成功 */
            send_read_handshake_status();
            break;
        }
    }
}

/*==============================================================
 * 函数名称：send_encrypt_data
 * 函数功能：发送加密后的数据请求命令
 * 参数个数：1
 * 参数描述：
 *          [IN]     encrypt_data   密文数据
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2019-11-11        戴辉发     	    创建
==============================================================*/
static void send_encrypt_data(uint32_t encrypt_data)
{
    uint8_t buf[8];
    
    /* CCS = 1, n = 0, e = 1, s = 1 */
    buf[0] = 0x23;
    /* index = 0x5002 */
    buf[1] = 0x02;
    buf[2] = 0x50;
    /* subindex = 0x00 */
    buf[3] = 0x00;
    /* 加密数据，由低到高 */
    buf[4] = (encrypt_data >> 0x00) & 0xFF;
    buf[5] = (encrypt_data >> 0x08) & 0xFF;
    buf[6] = (encrypt_data >> 0x10) & 0xFF;
    buf[7] = (encrypt_data >> 0x18) & 0xFF;

    can_std_transmit(0x600 + CONTROL_ID, buf, 0, 8);
}

/*******************************************************************************
**FuncName: ;//
**Function: 根据获取的随机数据，计算叉车的crc值;
**Output  : 无;
**input   : 无;
**Create date : liyong @2021.5.11
**Modify  : 
*******************************************************************************/
void  handshake_crc_data( void )
{
    char KeyString[17] = "Nobelift1212cEM!";
    char key_selector[4];
    uint8_t key;
    
    key = (( RandomData&0x00000f00 )>>8);  
    key_selector[0] = KeyString[key&0x0f];key++;//%16
    key_selector[1] = KeyString[key&0x0f];key++;//%16
    key_selector[2] = KeyString[key&0x0f];key++;//%16
    key_selector[3] = KeyString[key&0x0f];//%16
    
    EncryptedCRCdata = key_selector[3];
    EncryptedCRCdata <<= 8;
    EncryptedCRCdata += key_selector[2];
    EncryptedCRCdata <<= 8;
    EncryptedCRCdata += key_selector[1];
    EncryptedCRCdata <<= 8;
    EncryptedCRCdata += key_selector[0];
    EncryptedCRCdata ^= RandomData;
    send_encrypt_data(EncryptedCRCdata);
}

void recived_handshake_data(uint32_t cob_id, uint8_t *data)
{
    if( cob_id == (0x180 + CONTROL_ID))
    {
       recived180MessageDelay = 10; 
    }
    if (cob_id == (0x580 + CONTROL_ID))
    {
        uint16_t index;

        index = data[2];
        index = (index << 8) + data[1];
        
        if (index == 0x5000)
        {//读取到随机数
            if (0 == data[3])
            {
                RandomData = data[7];
                RandomData <<= 8;
                RandomData += data[6];
                RandomData <<= 8;
                RandomData += data[5];
                RandomData <<= 8;
                RandomData += data[4];
                handshake_crc_data();
            }
        }
        else if(index == 0x5002)
        {
        }
        else if(index == 0x5003)
        {
            if (0 == data[3])
            {//Handshake State
                if(data[4] == 0)
                {
                    handshake_status = 0;
                }
                else if(data[4] == 1)
                {
                    handshake_status = 1;
                }
                else if(data[4] == 0xff)
                {
                    handshake_status = 2;
                }
            }
        }
    }
}

/***********************************************************************************************
使用 ID： 这些ID都不在canopen的范围内，刚刚好
ID        发送方向        byte0   byte1  byte2  byte3  byte4  byte5  byte6  byte7
0x7F1 ：上位机 -> BMS     0xa1    size   adr1   adr2   adr3   adr4    0x0    0x0    //查数据
0x7F1 ：上位机 -> BMS     0xa2    size   adr1   adr2   data1  data2  data3  data4  //设置数据
0x7F2 ：BMS -> 上位机     adr1    adr2   adr3    adr4  data1  data2  data3  data4  //回应查数据

cmd ：0xa1: 获取数据
      0xa2：设置数据
      0xa3: 发送数据
size：数据的长度 <= 4
adr1:数据的地址高字节
adr2:数据的地址低字节
data1:数据高字节 bit32-25
***********************************************************************************************/
void debug_bms_variable_process( CanRxMsg rx_msg )
{
    if(( rx_msg.rxcanId.canid_bits.StdId == 0x7F1 )&&( rx_msg.rxcanId.canid_bits.IDE == 0 )&&( rx_msg.rxcanId.canid_bits.RTR == 0 )&&( rx_msg.DLC == 8 ))
    {
        if((rx_msg.Data[0] == 0xa1) && (rx_msg.Data[1] <= 4) && (rx_msg.Data[1] > 0))
        /* 读取 变量值 */
        {
            uint32_t  data=0xffffffff;
            uint8_t buf[8];
            uint32_t  address = rx_msg.Data[2];
            address <<= 8;
            address += rx_msg.Data[3];
            address <<= 8;
            address += rx_msg.Data[4];
            address <<= 8;
            address += rx_msg.Data[5];

            if((rx_msg.Data[6] == 0) && (rx_msg.Data[7] == 0))
            {
                switch( rx_msg.Data[1] )
                {
                case 1:
                    data = (uint8_t)*((uint8_t *)address);
                    break;
                case 2:
                    data = (uint16_t)*((uint16_t *)address);
                    break;
                case 4:
                    data = (uint32_t)*((uint32_t *)address);
                    break;
                }
                buf[0] = rx_msg.Data[2];
                buf[1] = rx_msg.Data[3];
                buf[2] = rx_msg.Data[4];   
                buf[3] = rx_msg.Data[5];
                buf[4] = (data >> 24) & 0xFF;
                buf[5] = (data >> 16) & 0xFF;
                buf[6] = (data >> 8) & 0xFF;
                buf[7] = (data ) & 0xFF;
                can_std_transmit(0x7F2, buf, 0, 8);      
            }
            else
            {//可以扩充用于查询变量的子地址
            }
        }
        else if(rx_msg.Data[0] == 0xa2)
        {//设置变量值
            uint32_t  data = rx_msg.Data[4];

            uint32_t  address = rx_msg.Data[2];
            address <<= 8;
            address += rx_msg.Data[3];
            address += ((uint32_t)(&data)&0x20000000);

            data <<= 8;
            data += rx_msg.Data[5];
            data <<= 8;
            data += rx_msg.Data[6];
            data <<= 8;
            data += rx_msg.Data[7];

            switch( rx_msg.Data[1] )
            {
            case 1:
                *((uint8_t *)address) = (uint8_t)data;
                break;
            case 2:
                *((uint16_t *)address) = (uint16_t)data;
                break;
            case 4:
                *((uint32_t *)address) = (uint32_t)data;
                break;
            }
        }
        else if(rx_msg.Data[0] == 0xa3)
        {/*调试模式*/
            
            if( rx_msg.Data[1] == 0x0a )
            {/*继电器测试*/
                if( rx_msg.Data[2] == 0x01 )
                {/*speaker */
                    if( rx_msg.Data[3] == 0x01 )
                    {/* 导通测试 */
                         if( rx_msg.Data[4] == 0x00 )
                         {
                             SpeakerControlOperate(E_OFF_STATUS);
                         }
                         else if( rx_msg.Data[4] == 0xff )
                         {
                             SpeakerControlOperate(E_ON_STATUS);
                         }
                    }
                    else if( rx_msg.Data[3] == 0x02 )
                    { /* speaker 是否存在 查询 */
                            uint8_t buf[8];
                            buf[0] = rx_msg.Data[0];
                            buf[1] = rx_msg.Data[1];
                            buf[2] = rx_msg.Data[2];   
                            buf[3] = rx_msg.Data[3];
                            buf[4] = SpeakerControlExist();
                            buf[5] = 0;
                            buf[6] = 0;
                            buf[7] = 0;
                            can_std_transmit(0x7F3, buf, 0, 8);       
                    }
                     
                }
                else if( rx_msg.Data[2] == 0x03 )
                {/*dry out1 */
                    if( rx_msg.Data[3] == 0x01 )
                    {/* 导通测试 */
                         if( rx_msg.Data[4] == 0x00 )
                         {
                             DryOut1ControlOperate(E_OFF_STATUS);
                         }
                         else if( rx_msg.Data[4] == 0xff )
                         {
                             DryOut1ControlOperate(E_ON_STATUS);
                         }
                    }
                }
                else if( rx_msg.Data[2] == 0x04 )
                {/*dry out2 */
                    if( rx_msg.Data[3] == 0x01 )
                    {/* 导通测试 */
                         if( rx_msg.Data[4] == 0x00 )
                         {
                             DryOut2ControlOperate(E_OFF_STATUS);
                         }
                         else if( rx_msg.Data[4] == 0xff )
                         {
                             DryOut2ControlOperate(E_ON_STATUS);
                         }
                    }
                }
                
            }
            
        }
    }
}

/*=============================================================
* 函数名称：canopen_main
* 函数功能：CANOPEN状态处理流程
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-23          戴辉发             创建
==============================================================*/
void canopen_main(void)
{
	/* CanOpen slave state machine         */
	/* ------------------------------------*/
    event_deal_process();

	TimeDispatch();
     /* CAN底层发送处理流程 */
    CAN_TX_Process();
     /* CAN底层接受处理流程 */
	receiveMsgHandler();
    
#if defined(TIANHONG)
    SendTHJLmessage();
#endif
#if defined(TIANFENG)
    RY_CommProc();
#endif

	if (g_can_app_s_timer) return;
	g_can_app_s_timer = 200;

    get_rtc_time();

    //can_handshakestate_process();
	if( recived180MessageDelay != 0 )
    {
        SetCarLoadRecivedFlag(1);
        recived180MessageDelay--;
    }
    else
    {
        SetCarLoadRecivedFlag(0);
    }
}

/***************************************************************************/
/* receiveMsgHandler : call the receive function 
 and call the implied process function
 Parameters : the function used to receive can messages. 
 Function used only when platform is Linux or hcs12*/
uint8_t receiveMsgHandler(void)
{
	CanRxMsg rx_msg;
    uint8_t received = 0;

	if(get_data_from_can_recv_buff(&rx_msg))
	{
		Message m;

        if(rx_msg.rxcanId.canid_bits.IDE == 0)
        {
            m.cob_id.w = rx_msg.rxcanId.canid_bits.StdId;
            m.rtr = rx_msg.rxcanId.canid_bits.SRR;
            m.len = rx_msg.DLC;
            memcpy(m.data, rx_msg.Data, m.len);
            if(m.cob_id.w  == 0x357)
            {
                ChHeartData(1);
                SetRecivedChargerMessFlag(1);
                received = 1;
            }
            else
            {
                canDispatch(&m);
                recived_handshake_data(m.cob_id.w, rx_msg.Data);
                debug_bms_variable_process(rx_msg);
            }
        }
        else
        {
            uint32_t extId;

            extId = rx_msg.rxcanId.canid_bits.StdId;
            extId <<= 18;
            extId |= rx_msg.rxcanId.canid_bits.extId;
            extId &= 0x1fffffff;

            if(extId == 0x18FFF4A0)
            {
                recived180MessageDelay = 10;
            }
            if(extId == 0x1826F456)
            {
                ChHeartData(1);
                received = 1;
            }
            RecivedChargerMessageProcess(rx_msg);
        }
	}
    if( received == 1)
       ch_data_indication(rx_msg.Data);
	return 0;   
}

/*==============================================================
 * 函数名称：SendTHJLmessage
 * 函数功能：发送天宏加力数据
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void SendTHJLmessage(void)
{
    static uint8_t sendTHJLmessageNum = 0;
    if(sendTHJLPdoDelay <= 0)
    {
        sendTHJLPdoDelay = 20;  /* 5ms计时器，100ms发一次 */
        sendTHJLmessageNum++;
        SetBatStatus();
        if((sendTHJLmessageNum % 5) == 0)
        {
            SetSocData();
        }
        if((sendTHJLmessageNum % 10) == 0)
        {
            SetCellTempVol();
            sendTHJLmessageNum = 0;
        }
        if(sendTHJLmessageNum > 10)
        {
            sendTHJLmessageNum = 0;
        }
    }
}

/*==============================================================
 * 函数名称：SetBatStatus
 * 函数功能：填充天宏加力数据0x224
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void SetBatStatus(void)
{
    uint8_t buf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    /* 0x224 */
    /* 0.4-0.5(battery temperature) 0:normal;1:high_temp.warning;2:high_temp.alarm;3:not used */
    if((get_power_temp_status() == E_TEMP_HIGH_PROTECT) || (get_environ_temp_status() == E_TEMP_HIGH_PROTECT))
    {
        buf[0] |= (2 << 4); 
    }
    else if((get_power_temp_status() == E_TEMP_HIGH_ALARM) || (get_environ_temp_status() == E_TEMP_HIGH_ALARM)
            || (get_ch_temp_status() == E_TEMP_HIGH_ALARM) || (get_dch_temp_status() == E_TEMP_HIGH_ALARM))
    {
        buf[0] |= (1 << 4);
    }

    if(E_CHARGE_STATUS == get_system_status())
    {
        if(get_ch_temp_status() == E_TEMP_HIGH_PROTECT)
        {
            buf[0] |= (2 << 4);
        }
    }
    else
    {
        if(get_dch_temp_status() == E_TEMP_HIGH_PROTECT)
        {
            buf[0] |= (2 << 4);
        }
    }
    
    /* 0.6-0.7 (battery leakage) 0:normal;1:warning;2:serious leakage; 3:not use */
    buf[0] |= (3 << 6); 
    
    /* 2.2-2.3(batter status) 0:normal;1:warning;2-3:not use */
    if(E_CHARGE_STATUS == get_system_status())
    {
        if((g_run_sys_data.vol_event & 0x33) != 0)
        {
            buf[2] |= (1 << 2); 
        }
        else if((g_run_sys_data.cur_event & 0xE3) != 0)
        {
            buf[2] |= (1 << 2); 
        }
        else if(((g_run_sys_data.temp_event & 0xD8) != 0) || 
                (get_ch_temp_status() == E_TEMP_HIGH_PROTECT) || (get_ch_temp_status() == E_TEMP_HIGH_ALARM)
             || (get_ch_temp_status() == E_TEMP_LOW_PROTECT) || (get_ch_temp_status() == E_TEMP_LOW_ALARM))
        {
            buf[2] |= (1 << 2); 
        }
    }
    else
    {
        if((g_run_sys_data.vol_event & 0xCC) != 0)
        {
            buf[2] |= (1 << 2); 
        }
        else if((g_run_sys_data.cur_event & 0xFC) != 0)
        {
            buf[2] |= (1 << 2);
        }
        else if(((g_run_sys_data.temp_event & 0xD8) != 0) || 
                (get_dch_temp_status() == E_TEMP_HIGH_PROTECT) || (get_dch_temp_status() == E_TEMP_HIGH_ALARM)
             || (get_dch_temp_status() == E_TEMP_LOW_PROTECT) || (get_dch_temp_status() == E_TEMP_LOW_ALARM))
        {
            buf[2] |= (1 << 2); 
        }
    }
    
    if((g_run_sys_data.ba_event & 0x16) != 0)
    {
        buf[2] |= (1 << 2); 
    }
    if((g_run_sys_data.cap_event & 0xFE) != 0)
    {
        buf[2] |= (1 << 2); 
    }
    if((g_run_sys_data.new_event1 & 0x1F) != 0)
    {
        buf[2] |= (1 << 2);
    }
    
    can_std_transmit(0x244, buf, 0, 8);
}

/*==============================================================
 * 函数名称：SetSocData
 * 函数功能：填充天宏加力数据0x444
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void SetSocData(void)
{
     uint8_t buf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

     buf[5] = get_soc_value() * 100 / 255;
     
     can_std_transmit(0x444, buf, 0, 8);
}

/*==============================================================
 * 函数名称：SetCellTempVol
 * 函数功能：填充天宏加力数据0x447+0x446
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void SetCellTempVol(void)
{
    uint8_t buf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t buf1[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    uint16_t cellVol = GetReportVol();
    int16_t temp = (int16_t)get_report_temp() - 100 + 40;
    if(temp < 0)
    {
        temp = 0;
    }
    else if(temp > 200)
    {
        temp = 200;
    }
    
    buf[1] = (uint8_t)temp;
    
    can_std_transmit(0x447, buf, 0, 8);
    
    buf1[5] = (cellVol >> 8);
    buf1[6] = (cellVol & 0xFF);
    
    can_std_transmit(0x446, buf1, 0, 8);
}
/*==============================================================
 * 函数名称：RY_CommProc
 * 函数功能：福德尔-中力电池通讯
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-08-29        Fanyl          创建
 ==============================================================*/                    
void RY_CommProc(void)
{
    if (0 == RY_CommDelay)
    {
        RY_CommDelay = 20;    /* 100ms */
        RYSetBattst();
        RY_step++;
        if(RY_step >=2 )
        {
          RY_step = 0;
          RYSetCellStatus();
          RYSetCellVoltage();
          RYSetCellTemp();
          RYSetAlmInf0();
        }
    }
}
/*==============================================================
 * 函数名称：RYSetBattst
 * 函数功能：填充如意数据0x18FFA0F4
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void RYSetBattst(void)
{
    uint8_t buf[8] = {0};

    if(GetCarLoadRecivedFlag() == 0)
    {
        buf[0] = 0x7F;
    }
    else
    {
        buf[0] = 0x05;
    }
    can_ext_transmit(0x18FFA0F4, buf, 8);
}
/*==============================================================
 * 函数名称：RYSetCellStatus
 * 函数功能：填充如意数据0x18FFA2F4
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void RYSetCellStatus(void)
{
    uint16_t u16_temp;
    int32_t s32_temp;
    uint8_t buf[8] = {0};
    
    u16_temp = get_total_vol() / 100; /*总电压, 0.1V */
    /* Byte1 总电压低字节 */
    buf[0] = 0xFF & (u16_temp >> 0);
    /* Byte2 总电压高字节 */
    buf[1] = 0xFF & (u16_temp >> 8);
    /* Byte3 电流低字节,单位：0.1A */
    s32_temp = (int32_t)get_cur_current() + 4000;
    if(s32_temp < 0)
    {
        s32_temp = 0;
    }
    else if(s32_temp > 65535)
    {
        s32_temp = 65535;
    }
    u16_temp = (uint16_t)s32_temp;
    buf[2] = 0xFF & (u16_temp >> 0);
    /* Byte4 电流高字节,单位：0.1A */
    buf[3] = 0xFF & (u16_temp >> 8);
    /* Byte5 SOC，0-100 */
    buf[4] = (uint8_t)(100L * g_run_sys_data.soc / MAX_SOC_VALUE);
    buf[5] = 0;
    u16_temp = g_dch_circle.circle_num.conut; 
    /* Byte6 总电压低字节 */
    buf[6] = 0xFF & (u16_temp >> 0);
    /* Byte7 总电压高字节 */
    buf[7] = 0xFF & (u16_temp >> 8);
    
    can_ext_transmit(0x18FFA2F4, buf, 8);
}
/*==============================================================
 * 函数名称：RYSetCellVoltage
 * 函数功能：填充如意数据0x18FFA4F4
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void RYSetCellVoltage(void)
{
    uint16_t u16_temp;
    uint8_t buf[8] = {0};

    u16_temp = get_max_cell_vol();
    /* Byte0 单体最大电压低字节,分辨率mV*/
    buf[0] = 0xFF & (u16_temp >> 0);
    /* Byte1 单体最大电压高字节,分辨率mV*/
    buf[1] = 0xFF & (u16_temp >> 8);
    /* Byte2 单体最大电压位置*/
    buf[2] = get_max_cell_index()+1;
    
    u16_temp = get_min_cell_vol();
    /* Byte3 单体最小电压低字节,分辨率mV*/
    buf[3] = 0xFF & (u16_temp >> 0);
    /* Byte4 单体最小电压高字节,分辨率mV*/
    buf[4] = 0xFF & (u16_temp >> 8);   
    /* Byte5 单体最小电压位置*/
    buf[5] = get_min_cell_index()+1;
    
    can_ext_transmit(0x18FFA4F4, buf, 8);
}
/*==============================================================
 * 函数名称：RYSetCellTEMP
 * 函数功能：填充如意数据0x18FFA5F4
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void RYSetCellTemp(void)
{
    uint8_t buf[8] = {0};

    /* Byte0 最高电芯温度，分辨率°C*/
    buf[0] = get_max_cell_temp()/10 + 50;
    /* Byte1 最高温度位置*/
    buf[1] = get_max_cell_temp_no()+1;
        /* Byte2 最低电芯温度，分辨率°C*/
    buf[2] = get_min_cell_temp()/10 + 50;
    /* Byte3 最低温度位置*/
    buf[3] = get_min_cell_temp_no()+1;
    /* Byte4 平均电芯温度，分辨率°C*/
    buf[4] = get_average_cell_temp()/10 + 50;

    can_ext_transmit(0x18FFA5F4, buf, 8);
}
/*==============================================================
 * 函数名称：RYSetAlmInf0
 * 函数功能：填充如意数据0x18FFAFF4
 * 参数描述：无
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人         修改类型
 * 2024-07-15        Fanyl     	    创建
==============================================================*/
void RYSetAlmInf0(void)
{
    uint8_t buf[8] = {0};
    uint32_t u32_temp = 0;

    /*1. 单体过压*/    
    if (E_CHARGE_STATUS == get_system_status())
    {
        if (get_cell_vol_status() == E_VOL_OVER)
           u32_temp |= 0x01;
        else if (get_cell_vol_status() == E_VOL_OVER_ALARM)
           u32_temp |= 0x02;
        else if ( GetcellVoltageFirstReport() == 1)
           u32_temp |= 0x03;
    }
    else
    {
        if(get_cell_vol_ovp_status() == 1)
           u32_temp |= 0x01 ;
    }
    /*2. 单体欠压*/    
    if (E_CHARGE_STATUS != get_system_status())
    {
        if (get_cell_vol_status() == E_VOL_UNDER)
           u32_temp |= (0x01<<2);
        else if (get_cell_vol_status() == E_VOL_UNDER_ALARM)
           u32_temp |= (0x02<<2);
        else if ( GetcellVoltageFirstReport() == 2)
           u32_temp |= (0x03<<2);
    }
    else
    {
        if(get_cell_vol_uvp_status() == 1)
           u32_temp |= (0x01<<2);
    }
    /*3.总压过高*/
    if (E_CHARGE_STATUS == get_system_status())
    {
        if(get_total_vol_status() == E_VOL_OVER)
           u32_temp |= (0x01<<4);
        else if (get_total_vol_status() == E_VOL_OVER_ALARM)
           u32_temp |= (0x02<<4);
        else if ( GettatolVoltageFirstReport() == 1)
           u32_temp |= (0x03<<4);
    }
    else
    {
        if(get_cell_vol_ovp_status() == 1)
           u32_temp |= (0x01<<4);
    }
    /*4.总压欠压*/
	if (E_CHARGE_STATUS != get_system_status())
	{
        if ((get_total_vol_status() == E_VOL_UNDER)||(get_cell_vol_uvp_status() == 1))
           u32_temp |= (0x01<<6);
        else if (get_total_vol_status() == E_VOL_UNDER_ALARM)
            u32_temp |= (0x02<<6);
        else if ( GettatolVoltageFirstReport() == 2)
            u32_temp |= (0x03<<6);
	}
    else if(get_cell_vol_uvp_status() == 1)
    {
        u32_temp |= (0x01<<6);
    }
    /*5. 单体压差大*/
	if ((E_BAT_FAIL == get_bat_status()) || (GetVoltageDifferentReport() == 3))
    {
        u32_temp |= (0x01<<8);
    }
    else if ((E_BAT_FAIL_WAIT == get_bat_status()) || (GetVoltageDifferentReport() == 2))
    {
        u32_temp |= (0x02 << 8);
    }
    else if(GetVoltageDifferentReport() == 1)
    {
        u32_temp |= (0x03 << 8);
    }
    /*6. 放电过流*/
    if (E_CHARGE_STATUS != get_system_status())
    {
        if ( (E_C_DCH_PROTECT ==  get_current_status())|| 
             (E_C_SECOND_PROTECT ==  get_current_status()) || 
             (2 == get_curr_status()) || 
             (0 != get_short_status()) )
        {
            u32_temp |= (0x01<<10);
        }
        else if (E_C_DCH_ALARM ==  get_current_status())
            u32_temp |= (0x02<<10);
        else if (GetContinuousDisCurrentOverReport())
            u32_temp |= (0x03<<10);
    }

    /*7. 充电过流*/
    if (E_CHARGE_STATUS == get_system_status())
    {
        if(E_C_CH_PROTECT ==  get_current_status())
            u32_temp |= (0x01<<12);
        else if(E_C_CH_ALARM ==  get_current_status())
            u32_temp |= (0x02<<12);
        else if (GetChargeCurrentOverReport())
            u32_temp |= (0x03<<12);
    }
        /*8. 温度过高*/
    if(E_CHARGE_STATUS == get_system_status())
    {
       if((E_TEMP_HIGH_PROTECT == get_ch_temp_status())||
          (E_TEMP_HIGH_PROTECT == get_power_temp_status()))
         u32_temp |= (0x01<<14);
       else if((E_TEMP_HIGH_ALARM == get_ch_temp_status())||
                (E_TEMP_HIGH_ALARM == get_power_temp_status()))
         u32_temp |= (0x02<<14);
       else if(GetCellTempFirstReport() == 4)
         u32_temp |= (0x03<<14);
    }
    else 
    {
      if((E_TEMP_HIGH_PROTECT == get_dch_temp_status()) || 
         (E_TEMP_HIGH_PROTECT == get_power_temp_status()))
        {
            u32_temp |= (0x01<<14);
        }
      else if((E_TEMP_HIGH_ALARM == get_dch_temp_status()) || 
               (E_TEMP_HIGH_ALARM == get_power_temp_status()))
       {
            u32_temp |= (0x02<<14);
       }
      else if(GetCellTempFirstReport() == 4)
            u32_temp |= (0x03<<14);
    }
    
    /*9. 温度过低*/
    if(get_system_status() == E_CHARGE_STATUS)
    {
        if((E_TEMP_LOW_PROTECT == get_ch_temp_status()) ||
           (E_TEMP_LOW_PROTECT == get_power_temp_status()))
        {
            u32_temp |= (0x01<<16);
        }
        else if((E_TEMP_LOW_ALARM == get_ch_temp_status())||
                 (E_TEMP_LOW_ALARM == get_power_temp_status()))
        {
            u32_temp |= (0x02<<16);
        }
        else if(GetCellTempFirstReport() == 1)
            u32_temp |= (0x03<<16);
    }
    else
    {
        if((E_TEMP_LOW_PROTECT == get_dch_temp_status()) || 
           (E_TEMP_LOW_PROTECT == get_power_temp_status()))
        {
            u32_temp |= (0x01<<16);
        }
        else if((E_TEMP_LOW_ALARM == get_dch_temp_status()) || 
                (E_TEMP_LOW_ALARM == get_power_temp_status())) 
        {
            u32_temp |= (0x02<<16);
        }
        else if(GetCellTempFirstReport() == 1)
            u32_temp |= (0x03<<16);
    }
    /*10. 温差过大*/
    if(3 == GetCellTempDifferentReport())
        u32_temp |= (0x01<<18);
    else if(2 == GetCellTempDifferentReport()) 
        u32_temp |= (0x02<<18);
    else if(1 == GetCellTempDifferentReport())
        u32_temp |= (0x03<<18);
    
    /*11. SOC 过低*/
    if(get_system_status() != E_CHARGE_STATUS)
    {
        if(E_PROTECT_SOC ==get_soc_status())
            u32_temp |= (0x01<<20);
        else if(E_LOW_SOC ==get_soc_status())
            u32_temp |= (0x02<<20);
        else if(E_SUB_LOW_SOC ==get_soc_status())
            u32_temp |= (0x03<<20);
    }
    /*12.绝缘过低，预留*/
    /*13.握手报文超时，预留*/
    /*14.加热膜温度状态，预留*/
    /*15.加热状态，预留*/
    buf[0] = 0xFF&(u32_temp >> 0);
    buf[1] = 0xFF&(u32_temp >> 8);
    buf[2] = 0xFF&(u32_temp >> 16);
    buf[3] = 0xFF&(u32_temp >> 24);

    can_ext_transmit(0x18FFAFF4, buf, 8);
}
