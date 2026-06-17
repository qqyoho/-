/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：switch_status.c
* 文件功能描述：实现开关状态管理
*
* 修改记录：
* 2018-06-25 戴辉发 创建
*----------------------------------------------------------*/
#include "switch_status.h"
#include "parameter.h"
#include "afe_app.h"
#include "short.h"
#include "hardware.h"
#include "ch_detect.h"
#include "balance.h"
#include "temp_manage.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "vol_curr_addi_deal.h"
#include "can.h"
#include "system_control.h"
#include "protect_record.h"
#include "mode_manage.h"
#include "run_record.h"
#include "fault_manage.h"
#include "adc_sampling.h"
#include "can_app.h"
#include "ch_addition.h"
#include "low_power.h"
#define TIMER_TIMES                      (5)
#define TIMER_1000TIMES                  (1000/TIMER_TIMES)
#define TIMER_100TIMES                   (100/TIMER_TIMES)
#define CURRENT_STABLE_TIMER             (10000 / TIMER_TIMES)
#define PRE_WAIT_DELAY                   3
#define MAIN_WAIT_DELAY                  4
#define PRE_WAIT_VOL_DELAY               11
#define PRE_CLOSED_WAIT_VOL_DELAY        10



e_mos_operate_status g_ch_mos_status;
e_mos_operate_status g_dch_mos_status;
e_mos_operate_status g_heat_relay_status;
e_mos_operate_status speakerControl;
e_mos_operate_status odControl;
e_mos_operate_status dryOut1Control;
e_mos_operate_status dryOut2Control;

static e_switch_switch_type g_ch_switch; /* 充电开关正常失效标识 */
static e_switch_switch_type g_dch_switch; /* 放电开关正常失效标识 */
static volatile uint16_t t_ch_stable_delay;/* 放电电流稳定延迟 */
static volatile uint16_t t_dch_stable_delay;/* 充电电流稳定延迟 */

static uint16_t g_final_switch_status;
static uint16_t switch_fault_delay = 0;
static uint16_t status_error_delay = 0;
static uint8_t status_error_flag = 0;

static uint8_t relay_process_status;
static uint8_t precharge_fault_num;
static volatile uint16_t precharge_delay;

static uint8_t dch_switch_status; /* 放电MOS状态 */
static uint8_t ch_switch_status; /* 充电MOS状态 */

uint8_t  heat_relay_step;
static uint8_t  heat_relay_exist;
static uint16_t  heat_relay_delay;
static uint8_t  heat_delay;
/*=============================================================
 * 函数名称：switch_status_innermem_init
 * 函数功能：开关状态内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：   
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-02-21          戴辉发             创建
==============================================================*/
void switch_status_innermem_init(void)
{
    relay_process_status = 0;
    precharge_fault_num = 0;
    precharge_delay = 0;
    g_ch_mos_status = E_OFF_STATUS;
    g_dch_mos_status = E_OFF_STATUS;
	g_heat_relay_status = E_OFF_STATUS; 
    dch_switch_status = 0;
    ch_switch_status = 0;

    t_ch_stable_delay = CURRENT_STABLE_TIMER;
    t_dch_stable_delay = CURRENT_STABLE_TIMER;

    g_final_switch_status = 0;

	switch_fault_delay = 0;

	status_error_flag = 0;

	status_error_delay = 24000;
    heat_relay_step = 0;
    
    speakerControl = E_OFF_STATUS;
    odControl = E_OFF_STATUS;
    dryOut1Control = E_OFF_STATUS;
    dryOut2Control = E_OFF_STATUS;
}

/*=============================================================
 * 函数名称：wakeup_init_switch_status
 * 函数功能：开关状态内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：   
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-06-08          liyong             创建
==============================================================*/
void wakeup_init_switch_status()
{
    relay_process_status = 0;
    precharge_fault_num = 0;
    precharge_delay = 0;
    g_ch_mos_status = E_OFF_STATUS;
    g_dch_mos_status = E_OFF_STATUS;
	g_heat_relay_status = E_OFF_STATUS; 
    dch_switch_status = 0;
    ch_switch_status = 0;

    status_error_flag = 0;
    status_error_delay = 24000;
    heat_relay_step = 0;
    
    speakerControl = E_OFF_STATUS;
    odControl = E_OFF_STATUS;
    dryOut1Control = E_OFF_STATUS;
    dryOut2Control = E_OFF_STATUS;
}

/*=============================================================
 * 函数名称：switch_status_mem_init
 * 函数功能：开关状态内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-12-25          戴辉发             创建
==============================================================*/
void switch_status_mem_init(void)
{
	g_ch_switch = E_SWITCH_VALID;
    g_dch_switch = E_SWITCH_VALID;
    
    switch_status_innermem_init();
}

/*=============================================================
 * 函数名称：switch_status_ms_process
 * 函数功能：开关状态定时器运行 5ms
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-02-21          戴辉发             创建
==============================================================*/
void switch_status_ms_process(void)
{
    if (t_ch_stable_delay) t_ch_stable_delay --;
    if (t_dch_stable_delay) t_dch_stable_delay --;
    if (switch_fault_delay) switch_fault_delay --;
    if( status_error_delay) status_error_delay--; 
    if( precharge_delay) precharge_delay--; 
    if( heat_delay) heat_delay--; 
}
/*=============================================================
 * 函数名称：heat_on
 * 函数功能：加热打开
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-01-13          李勇          创建
==============================================================*/
void heat_on(void)
{
   g_heat_relay_status = E_ON_STATUS;
}
/*=============================================================
 * 函数名称：judge_heat_on
 * 函数功能：判断加热是否打开
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-10-09          李勇          创建
==============================================================*/
uint8_t judge_heat_on(void)
{
   if( g_heat_relay_status == E_ON_STATUS )
   {
       return 1;
   }
   return 0;
}

/*=============================================================
 * 函数名称：heat_off
 * 函数功能：加热关闭
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-01-13          李勇          创建
==============================================================*/
void heat_off(void)
{
   g_heat_relay_status = E_OFF_STATUS;
}
/*=============================================================
 * 函数名称：get_ch_switch_flag
 * 函数功能：获取充电开关失效标识
 * 参数个数：0
 * 函数参数：
 * 返回值：     
 *          0          正常
 *          1          失效
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-25          戴辉发             创建
==============================================================*/
e_switch_switch_type get_ch_switch_flag(void)
{
	return g_ch_switch;
}

/*=============================================================
 * 函数名称：get_dch_switch_flag
 * 函数功能：获取放电开关失效标识
 * 参数个数：0
 * 函数参数：
 * 返回值：     
 *          0          开关正常
 *          1          开关失效
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-25          戴辉发             创建
==============================================================*/
e_switch_switch_type get_dch_switch_flag(void)
{
	return g_dch_switch;
}

/*=============================================================
 * 函数名称：dch_disconnect_request
 * 函数功能：放电保护断开请求
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-05-24        戴辉发       创建
==============================================================*/
void dch_disconnect_request(void)
{
    g_dch_mos_status = E_OFF_STATUS;
    handshake_awakeup_init();
}

/*=============================================================
 * 函数名称：dch_connect_request
 * 函数功能：放电请求连接
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-05-24        戴辉发       创建
==============================================================*/
void dch_connect_request(void)
{
    g_dch_mos_status = E_ON_STATUS;
}

/*=============================================================
 * 函数名称：ch_disconnect_request
 * 函数功能：充电保护断开请求
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-05-24        戴辉发       创建
==============================================================*/
void ch_disconnect_request(void)
{
    g_ch_mos_status = E_OFF_STATUS;
}

/*=============================================================
 * 函数名称：ch_connect_request
 * 函数功能：充电请求连接
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-05-24        戴辉发       创建
==============================================================*/
void ch_connect_request(void)
{
    g_ch_mos_status = E_ON_STATUS;
}
/*=============================================================
 * 函数名称：main_relay_on
 * 函数功能：主继电器闭合
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void main_relay_on(void)
{		
#if defined( BF24_PEU1_S2H )
    /*充电控制 当作主继电器*/
    FL_GPIO_SetOutputPin( CHARGER_RELAY_GPIO, CHARGER_RELAY_GPIO_PIN);
#endif 
}
/*=============================================================
 * 函数名称：main_relay_off
 * 函数功能：主继电器断开
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void main_relay_off(void)
{
#if defined( BF24_PEU1_S2H )
    /*充电控制 当作主继电器*/
    FL_GPIO_ResetOutputPin( CHARGER_RELAY_GPIO, CHARGER_RELAY_GPIO_PIN);
#endif 
}
/*=============================================================
 * 函数名称：pre_relay_on
 * 函数功能：预充继电器闭合
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void pre_relay_on(void)
{
    FL_GPIO_SetOutputPin( PRECHARGER_RELAY_GPIO, PRECHARGER_RELAY_GPIO_PIN);
}
/*=============================================================
 * 函数名称：pre_relay_off
 * 函数功能：主继电器断开
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void pre_relay_off(void)
{
    FL_GPIO_ResetOutputPin( PRECHARGER_RELAY_GPIO, PRECHARGER_RELAY_GPIO_PIN);
}

/*=============================================================
 * 函数名称：SpeakerControlExist
 * 函数功能：Speaker检查存在与否
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：
             0：不存在
             1：存在
             2：无效
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
uint8_t SpeakerControlExist(void)
{
    uint8_t flag = 2;
    if( FL_GPIO_GetOutputPin( SPEAKER_RELAY_CONTROL_GPIO, SPEAKER_RELAY_CONTROL_GPIO_PIN) == 0 )
    {
        flag = FL_GPIO_GetInputPin( SPEAKER_RELAY_DETECT_GPIO, SPEAKER_RELAY_DETECT_GPIO_PIN);
    }
    return flag;
}
#if 0
/*=============================================================
 * 函数名称：SpeakerControlOn
 * 函数功能：蜂鸣器闭合
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void SpeakerControlOn(void)
{
    FL_GPIO_SetOutputPin( SPEAKER_RELAY_CONTROL_GPIO, SPEAKER_RELAY_CONTROL_GPIO_PIN);
}
/*=============================================================
 * 函数名称：SpeakerControlOff
 * 函数功能：蜂鸣器关闭
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void SpeakerControlOff(void)
{
    FL_GPIO_ResetOutputPin( SPEAKER_RELAY_CONTROL_GPIO, SPEAKER_RELAY_CONTROL_GPIO_PIN);
}
#endif
/*=============================================================
 * 函数名称：OdControlOperate
 * 函数功能：操作od 
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
void OdControlOperate( e_mos_operate_status status )
{
     odControl = status;
}
/*=============================================================
 * 函数名称：DryOut1ControlOperate
 * 函数功能：操作DryOut1
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
void DryOut1ControlOperate( e_mos_operate_status status )
{
     dryOut1Control = status;
}
/*=============================================================
 * 函数名称：DryOut2ControlOperate
 * 函数功能：操作DryOut2
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
void DryOut2ControlOperate( e_mos_operate_status status )
{
     dryOut2Control = status;
}
/*=============================================================
 * 函数名称：DryOut2ControlOperate
 * 函数功能：操作DryOut2
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
void SpeakerControlOperate( e_mos_operate_status status )
{
     speakerControl = status;
}
/*=============================================================
 * 函数名称：DryOut1ControlOn
 * 函数功能：DryOut1闭合
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void DryOut1ControlOn(void)
{
    FL_GPIO_SetOutputPin( DRY_RELAY_GPIO, DRY_RELAY_GPIO_PIN);
}
/*=============================================================
 * 函数名称：DryOut1ControlOff
 * 函数功能：DryOut1关闭
 * 参数个数：
 *         
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-01-21         liyong       创建
==============================================================*/
static void DryOut1ControlOff(void)
{
    FL_GPIO_ResetOutputPin( DRY_RELAY_GPIO, DRY_RELAY_GPIO_PIN);
}
/*******************************************************************************
**FuncName: cutoff_all_relay;//
**Function: 同时关闭所有的继电器;
**Output  : 无;
**input   : 无;
**Create date : liyong @2021.5.31
**Modify  : 
*******************************************************************************/
void cutoff_all_switch(void)	//
{
    ch_disch_mos_ctrl( E_ALL_OFF );
	main_relay_off();
    pre_relay_off();
    dch_switch_status = 0;
    ch_switch_status = 0;
}
/*=============================================================
 * 函数名称：switch_status_innermem_init
 * 函数功能：开关状态内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：   
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2022-03-03          liyong             创建
==============================================================*/
void ClearPreSwitchFlow(void)
{
    relay_process_status = 0;
    precharge_fault_num = 0;
    precharge_delay = 0;
    pre_relay_off(); 
}
/*=============================================================
 * 函数名称：RelayOnControlProcess
 * 函数功能：继电器闭合流程  
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-18        liyong       创建
==============================================================*/
static void MainDisChargerRelayOn(void)
{  
    if(  relay_process_status == 0 )
    {//状态1：主继电器 和 预充继电器都断开 状态
      main_relay_off();
      pre_relay_off();  
      dch_switch_status = 0;
      ch_switch_status = 0;
      SetCollectPackVoltageFlag(0);
      if( precharge_fault_num < 3 )
      {//预充超时为3s
            if( 0 == precharge_delay )
            {
                relay_process_status = 1;
                precharge_delay = 3 * TIMER_1000TIMES;
            }        
      }
      else
      {//设置短路标志
          set_afe_short();
          record_protect = 1;
          set_short_status(1);
          relay_process_status = 4;
      }   
    }
    else if(  relay_process_status == 1 )
    {//状态2：主继电器断开 和 预充继电器闭合状态
      main_relay_off();
      pre_relay_on();  
      SetCollectPackVoltageFlag(1);
       if( precharge_delay == 0 )
       {//预充超时
          /*休息2s后，再次进行预充过程*/
          precharge_delay = 2 * TIMER_1000TIMES; 
          precharge_fault_num++;
          relay_process_status = 0;
       }
       else if( GetPackVoltage() > 0.8*GetTotalVoltage() )
       {//预充成功
          relay_process_status = 2;
          precharge_delay = 2 * TIMER_100TIMES;
       }
    }
    else if (relay_process_status == 2)
    {//状态3：预充继电器闭合 ，主继电器闭合
       main_relay_on();
       pre_relay_on();  
        if (precharge_delay == 0)
        {//预充延时200ms
            relay_process_status = 3;
        }
    }
    else if (relay_process_status == 3)
    {//状态4：预充继电器断开 ，主继电器闭合
      main_relay_on();
      pre_relay_off(); 
      
      precharge_fault_num = 0;
      SetCollectPackVoltageFlag(0);
#if defined(BF24_PEU1_S2H) 
      dch_switch_status = 1;
      ch_switch_status = 1;
#endif                 
    }
    else if(  relay_process_status == 4 )
    {//状态5：短路状态 等待重启后，清空标志
       main_relay_off();
       pre_relay_off();  
    }   
}

/*=============================================================
 * 函数名称：RelayOnControlProcess
 * 函数功能：继电器闭合流程控制
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-18        liyong       创建
==============================================================*/
void RelayOnControlProcess(void)
{  
     if (E_CHARGE_STATUS == get_system_status())
     {/*清除预充中间参数*/
         ClearPreSwitchFlow();
         if( ch_disch_mos_ctrl( E_ALL_ON ) == 1 )
         {
              main_relay_on();
              relay_process_status = 3;
#if defined(BF24_PEU1_S2H) 
              dch_switch_status = 1;
              ch_switch_status = 1;
#endif  
         }     
     }
     else
     {/*增加预充过程*/
         if( ch_disch_mos_ctrl( E_ALL_ON ) == 1 )
            MainDisChargerRelayOn();
     }
}
/*=============================================================
 * 函数名称：RelayOffControlProcess
 * 函数功能：继电器断开流程控制
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-18        liyong       创建
==============================================================*/
void RelayOffControlProcess(void)
{  //继电器切断流程
    ch_disch_mos_ctrl( E_ALL_OFF );
    main_relay_off();
    pre_relay_off();
    dch_switch_status = 0;
    ch_switch_status = 0;
    SetCollectPackVoltageFlag(0);  
    if(  relay_process_status != 4 )
    {/* 除了状态5：短路状态 等待重启后，清空标志，其它状态回到初始位置*/
          relay_process_status = 0;
    }
    ClearPreSwitchFlow();
}
/*=============================================================
 * 函数名称：get_ch_switch_status
 * 函数功能：获取充电MOS状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       充电MOS断开
 *           1       充电MOS闭合
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_ch_switch_status(void)
{
    return ch_switch_status;
}

/*=============================================================
 * 函数名称：get_dch_switch_status
 * 函数功能：获取放电MOS状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       放电MOS断开
 *           1       放电MOS闭合
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_dch_switch_status(void)
{
    return dch_switch_status;
}

//加热模块部分代码
#define    HEAT_EXIST_TIME          400
/*=============================================================
 * 函数名称：heat_relay_on
 * 函数功能：加热闭合
 * 参数个数：0
 *         
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-19         李勇         创建
==============================================================*/
void heat_relay_on()
{
    FL_GPIO_SetOutputPin( HEAT_RELAY_CONTROL_GPIO, HEAT_RELAY_CONTROL_GPIO_PIN);	
}
/*=============================================================
 * 函数名称：heat_switch_detect
 * 函数功能：加热关闭
 * 参数个数：0
 *         
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-19         李勇         创建
==============================================================*/
void heat_relay_off()
{
    FL_GPIO_ResetOutputPin( HEAT_RELAY_CONTROL_GPIO, HEAT_RELAY_CONTROL_GPIO_PIN);	
}
/*=============================================================
 * 函数名称：heat_switch_detect
 * 函数功能：加热检测
 * 参数个数：0
 *         
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2021-11-19         李勇         创建
==============================================================*/
uint8_t heat_switch_detect( void )
{
   if( heat_delay == 0 ) 
   {
         heat_delay = 1;
         
         switch( heat_relay_step )
         {
           case 0:  
                    heat_relay_delay = HEAT_EXIST_TIME/2;
                    g_heat_relay_status = E_OFF_STATUS;
                    heat_relay_step = 1;
                    heat_relay_exist = 0;
                    break;
           case 1:
                    if (( 0 == FL_GPIO_GetOutputPin( HEAT_RELAY_CONTROL_GPIO,HEAT_RELAY_CONTROL_GPIO_PIN ) ))
                    {
                       if (  FL_GPIO_GetInputPin( HEAT_RELAY_DETECT_GPIO,HEAT_RELAY_DETECT_GPIO_PIN ) == 1  )
                       {//存在
                           if( heat_relay_delay++ > HEAT_EXIST_TIME )
                           {
                              heat_relay_delay = HEAT_EXIST_TIME;
                              heat_relay_exist = 1; 
                           }         
                       } 
                       else 
                       {
                          if( heat_relay_delay > 0 )
                          {
                             heat_relay_delay--;
                          }  
                          else
                          {
                             heat_relay_exist = 2; 
                          }
                       }
                    }
                    break;
           default:
             heat_relay_step = 0;
             break;
         }
   
   }
          
     return heat_relay_exist;
}

/*=============================================================
 * 函数名称：switch_operate_process
 * 函数功能：开关操作流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期             修改人      修改类型
 * 2018-01-17       	戴辉发     	创建
==============================================================*/
void switch_operate_process(void)
{ 
    if( E_ON_STATUS == g_dch_mos_status )
    {
        if( ch_disch_mos_ctrl( E_DISCHARGE_ON ) == 1 )
        { 
            dch_switch_status = 1; 
        }  
    }
    else
    {
        if( ch_disch_mos_ctrl( E_DISCHARGE_OFF ) == 1 )
        { 
            dch_switch_status = 0;  
        }  
    }
    
    if( E_ON_STATUS == g_ch_mos_status )
    {
        if( ch_disch_mos_ctrl( E_CHARGE_ON ) == 1 )
        { 
            ch_switch_status = 1; 
        }  
    }
    else
    {
        if( ch_disch_mos_ctrl( E_CHARGE_OFF ) == 1 )
        { 
            ch_switch_status = 0; 
        }  
    }
           
    
    if( E_ON_STATUS == g_heat_relay_status )
    {
        heat_relay_on();
    }
    else 
    { 
        heat_relay_off();
    }

    
    if( E_ON_STATUS == dryOut1Control )
    {
        DryOut1ControlOn();
    }
    else
    {
        DryOut1ControlOff();
    }
    
   
    
    if ( 0 != ch_switch_status )
    {
        protect_code[7] |= 0x10;
    }
    else
    {
        protect_code[7] &= ~0x10;
    }

    if ( 0 != dch_switch_status )
    {
        protect_code[7] |= 0x20;
    }
    else
    {
        protect_code[7] &= ~0x20;
    }
}

/*=============================================================
 * 函数名称：final_set_switch
 * 函数功能：终极保护设置开关状态
 * 参数个数：1
 *          [IN]     flag         终极保护开关标志，0：无终极保护，1：终极保护
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2018-01-17        戴辉发       创建
==============================================================*/
void final_set_switch(uint16_t flag)
{
    g_final_switch_status = flag;
}

/*=============================================================
 * 函数名称：get_final_set_switch
 * 函数功能：获取终极保护设置开关状态
 * 参数个数：0
 *         
 * 参数描述：
 * 返 回 值：终极保护开关标志
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2018-01-17        戴辉发       创建
==============================================================*/
uint16_t get_final_set_switch(void)
{
    return g_final_switch_status;
}

/*=============================================================
 * 函数名称：get_status_error_flag
 * 函数功能：获取状态互斥标志位
 * 参数个数：0
 *         
 * 参数描述：
 * 返 回 值：互斥标志位
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2020-6-9          李勇        创建
==============================================================*/
uint8_t get_status_error_flag(void)
{
    return status_error_flag;
}

/*=============================================================
 * 函数名称：switch_fault_process
 * 函数功能：开关失效判决流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void switch_fault_process(void)
{
    int16_t work_current = get_real_current();

    if (0 == get_dch_switch_status())
    {
        if (E_SWITCH_VALID == g_dch_switch)
        /* 放电MOS失效判决 */
        {
            if ( work_current <= -20 ) 
            {
                if (0 == t_dch_stable_delay)
                {
                    if( g_dch_switch != E_SWITCH_INVALID )
                    {
                        g_dch_switch = E_SWITCH_INVALID;
                        set_dismos_fault();
                        record_protect = 1;
                    }
                }
            }
            else
            {
                t_dch_stable_delay = CURRENT_STABLE_TIMER;
            }
        }
    }
    else
    {
        t_dch_stable_delay = CURRENT_STABLE_TIMER;
    }

    if ((0 == get_ch_switch_status()) && (E_CHARGE_STATUS == get_system_status()))
    {
        if (E_SWITCH_VALID == g_ch_switch)
        {
            if (work_current > 20)
            {
                if (0 == t_ch_stable_delay)
                {
                    if (g_ch_switch != E_SWITCH_INVALID)
                    {
                        g_ch_switch = E_SWITCH_INVALID;
                        set_chmos_fault();
                        record_protect = 1;
                    }
                }
            }
            else
            {
                t_ch_stable_delay = CURRENT_STABLE_TIMER;
            }
        }
    }
    else
    {
        t_ch_stable_delay = CURRENT_STABLE_TIMER;
    }

    /* 互锁状态： 充电不能放电，放电不能充电 长度 2min */
    if (ChargerIsConnect() == 1)
    {
        if (work_current <= -20)
        {
            /* 可以切断 */
            if (status_error_delay == 0)
            {
                if( status_error_flag != 2 )
                {
                    status_error_flag = 2; 
                    record_protect = 1;
                }
            }
        }
        else
        {
            status_error_delay = 24000;
        }
    }
    else
    {
        if (work_current >= 20)
        {
            if (status_error_delay == 0)//切不掉
            {
                if (status_error_flag != 1)
                {
                    status_error_flag = 1;
                    record_protect = 1;
                }
            }
        }
        else
        {
            status_error_delay = 24000;
        }
    }

    if ((setFactroyModeDelay != 0) || (GetPowerOnOverOneDay() == 0))
    {
        /* 工厂模式，关闭互斥功能 */
        status_error_flag = 0;
        status_error_delay = 24000;
    }

    if (E_SWITCH_INVALID == get_ch_switch_flag())
    {
        protect_code[4] |= 0x01;
    }
    else
    {
        protect_code[4] &= ~0x01;
    }      

    if (E_SWITCH_INVALID == get_dch_switch_flag())
    {
        protect_code[4] |= 0x02;
    }
    else
    {
        protect_code[4] &= ~0x02;
    }      

    if (0 != status_error_flag)
    {
        protect_code[4] |= 0x04;
    }
    else
    {
        protect_code[4] &= ~0x04;
    }
}
