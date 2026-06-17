/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：fault_manage.c
* 文件功能描述：实现控制板故障管理管理
*
* 修改记录：
* 2018-11-14 戴辉发 创建
*----------------------------------------------------------*/
#include "fault_manage.h"
#include "fm33lg0xx_fl.h"
#include "switch_status.h"
#include "system_control.h"
#include "parameter.h"
#include "ch_detect.h"
#include "afe_app.h"

#define MAX_FET_OPERATE_COUNT   10
#define MAX_FET_JUDGE_COUNT     100

typedef enum _E_FET_DETECT_STATUS_
{
    E_FET_OPEN_DETECT_WAIT_STATUS, /* 断开检测等待稳定状态 */
    E_FET_OPEN_DETECT_IDLE_STATUS, /* 断开检测稳定状态 */
}e_fet_detect_status;

static uint8_t backup_relay_in; /*二次执行模块是否存在标志 0：检测中 1：存在 2 ：不存在*/
static uint8_t backup_relay_deal; /*二次执行模块操作状态 0 初始化状态  1：断开状态 2 ：闭合状态*/
static uint8_t backup_relay_fault=0; /*二次执行模块操作故障状态 0 初始化状态  1：粘连状态 2 ：无法闭合状态*/
static uint8_t backup_relayon_delay; /***/
static uint8_t backup_relayoff_delay; /***/
static uint8_t backup_on_mos_delay; 
static volatile uint8_t fault_timer; /* 故障定时器 */
static uint8_t fail_cutoff_flag = 0; 

/*=============================================================
* 函数名称：fault_manage_mem_init
* 函数功能：充放电开关故障管理内存初始化
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-11-14       	戴辉发     	创建
==============================================================*/
void fault_manage_mem_init(void)
{
    fault_timer = 1;
    backup_relay_in = 0;
    backup_relay_deal = 0;
    if( backup_relay_fault == 2 )
       backup_relay_fault = 0;
    //backup_relayon_delay = 20;
    backup_relayoff_delay = 20; 
    backup_on_mos_delay = 25;
}

/*=============================================================
* 函数名称：relay_on
* 函数功能：二次保护继电器闭合
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-11-14       	戴辉发     	创建
==============================================================*/
void relay_on(void)
{
    FL_GPIO_SetOutputPin( RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO);
}

/*=============================================================
* 函数名称：relay_off
* 函数功能：二次保护继电器断开
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-11-14       	戴辉发     	创建
==============================================================*/
void relay_off(void)
{
    FL_GPIO_ResetOutputPin( RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO);
}


/*=============================================================
* 函数名称：set_fail_cutoff_flag
* 函数功能：设置通讯断开辅助继电器
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2020-06-16      	liyong     	创建
==============================================================*/
void set_fail_cutoff_flag(uint8_t vaule)
{
  if( vaule == 1 )
    fail_cutoff_flag = 1;
  else
    fail_cutoff_flag = 0;
}

/*=============================================================
* 函数名称：get_fail_cutoff_flag
* 函数功能：设置通讯断开辅助继电器
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2020-06-16      	liyong     	创建
==============================================================*/
uint8_t get_fail_cutoff_flag(void)
{
   return fail_cutoff_flag;
}
/*=============================================================
 * 函数名称：fault_manage_timer_ms_run
 * 函数功能：故障管理模块定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-11-14          戴辉发             创建
==============================================================*/
void fault_manage_timer_ms_run(void)
{
	if (fault_timer) fault_timer --;
    if (backup_relayon_delay) backup_relayon_delay --;
    if (backup_relayoff_delay) backup_relayoff_delay --;
    if (backup_on_mos_delay) backup_on_mos_delay --;
}
/*=============================================================
* 函数名称：get_backup_relay_in
* 函数功能：获取二次模块是否存在
* 参数个数：0
* 函数参数：
* 返 回 值：
*           1: 存在 
            0：不存在
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2020-06-15          liyong             创建
==============================================================*/
uint8_t get_backup_relay_in(void)
{
   if( backup_relay_in == 1 )
	 return 1;
   else
     return 0;
}
/*=============================================================
* 函数名称：get_backup_relay_deal
* 函数功能：获取二次执行单元控制状态
* 参数个数：0
* 函数参数：
* 返 回 值：
*           0: 初始化
            1: 闭合
            2: 断开
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2020-06-15          liyong             创建
==============================================================*/
uint8_t get_backup_relay_deal(void)
{
	return backup_relay_deal;
}
/*=============================================================
* 函数名称：get_backup_relay_fault
* 函数功能：获取二次执行单元故障状态
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2020-06-15          liyong             创建
==============================================================*/
uint8_t get_backup_relay_fault(void)
{
	return backup_relay_fault;
}

/*=============================================================
* 函数名称：fet_fault_deal_process
* 函数功能：充放电开关故障处理流程
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：未启用二次保护
*===============================================================
* 日    期          修改人      修改类型
* 2020-6-22      	李勇     	创建
只在充电模式使用二次执行模块

1. 闭合过程
   检测到二次执行单元，如果需要闭合充电继电器，则先闭合二次执行单元
   然后去执行其它继电器的闭合流程
2. 断开过程
   检测到二次执行单元，如果需要断开充电继电器，则先断开充电继电器
   然后去断开二次执行继电器
==============================================================*/
void fet_fault_deal_process(void)
{
     //二次执行模块逻辑：
    //1.在继电器未闭合的前提下：首先检测spmDet是否为低电平 来判断 是否存在二次模块 
    //2.如果存在二次模块 则执行 二次模块操作 
    //3.如果没有则不执行
    if( ( get_ch_switch_flag() != E_SWITCH_VALID )||\
        ( get_dch_switch_flag() != E_SWITCH_VALID ) )
    {
         relay_off();
         backup_relay_in = 0; //断开后重新检测
    }
    else
    {
          if( backup_relay_in == 0 )
          {//关闭继电器 检测二次执行单元是否存在
                relay_off();
                if ( 0 == FL_GPIO_GetOutputPin(RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO) )
                {
                   if ( 0 != FL_GPIO_GetInputPin(RELAY_EXIST_GPIO_GROUP, RELAY_EXIST_GPIO))
                   {//存在
                       backup_relay_in = 1;     
                   } 
                   else
                   {
                       backup_relay_in = 2; 
                   }
                }
          }         
          else /*if (( backup_relay_in == 1 ))*/ /*&&( E_ON_STATUS == g_ch_mos_status )*/
          {//存在，则控制
               relay_on();   
          }
    }  
    
/*延时切断二次执行继电器*/         
   if( 0 != get_ch_switch_status() ) 
   {
      backup_relayoff_delay = 20;
   }

 
/*二次继电器闭合后，延时闭合其它继电器*/   
   if ( 0 == FL_GPIO_GetOutputPin(RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO) )
   {
       backup_on_mos_delay = 20;
       backup_relay_deal = 2;
   }
  
   if((( 0 != FL_GPIO_GetOutputPin(RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO) )&&( backup_on_mos_delay == 0 ))||( backup_relay_in == 2 ))
   {
       backup_relay_deal = 1;
   }
            
   if( 1 == get_fail_cutoff_flag() )
   {
       protect_code[7] |= 0x80;
   }
   else
   {
       protect_code[7] &= ~0x80;
   }
   
   //等待二次模块执行延时中
   if( 1 != backup_relay_deal  )
   {
       protect_code[6] |= 0x40;
   }
   else
   {
       protect_code[6] &= ~0x40;
   }
}

/*=============================================================
* 函数名称：GetBackupRelayStatus
* 函数功能：获取二次执行继电器状态
* 参数个数：0
* 参数描述：
* 返 回 值：
*           0       断开
*           1       闭合
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2023-07-06        李勇              创建
==============================================================*/
uint8_t GetBackupRelayStatus(void)
{ 
    uint8_t flag = 0;
    if( FL_GPIO_GetOutputPin( RELAY_CTRL_GPIO_GROUP, RELAY_CTRL_GPIO ) != 0 )
    {
        flag = 1;
    }
    return flag; 
}
