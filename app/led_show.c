/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：bluetooth.how.h
* 文件功能描述：实现led闪烁功能
*
* 修改记录：
* 2018-12-05    戴辉发 创建
*----------------------------------------------------------*/

#include "led_show.h"
#include "system_control.h"
#include "fm33lg0xx_fl_gpio.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "vol_manage.h"
#include "stdlib.h"

#define IDLE_TIMER       20
#define DISCHARGE_TIMER  10
#define CHARGE_TIMER     5

static volatile uint16_t led_timer;
static volatile uint16_t alarm_led_timer;
int rand_data;
/*=============================================================
* 函数名称：led_show_mem_init
* 函数功能：显示
* 参数个数：0
* 参数描述：
* 返 回 值：
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-09       	戴辉发     	创建
==============================================================*/
void led_show_mem_init(void)
{
    led_timer = IDLE_TIMER;
    alarm_led_timer = CHARGE_TIMER;
}

/*=============================================================
 * 函数名称：led_show_timer_100ms_run
 * 函数功能：显示模块定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-05          戴辉发             创建
==============================================================*/
void led_show_timer_100ms_run(void)
{
	if (led_timer) led_timer --;
    if (alarm_led_timer) alarm_led_timer --;
}

/*=============================================================
 * 函数名称：led_off
 * 函数功能：关闭LED
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-10-29          戴辉发             创建
==============================================================*/
void led_off(void)
{
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_15);
}

/*=============================================================
 * 函数名称：led_on
 * 函数功能：打开LED
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-10-29          戴辉发             创建
==============================================================*/
void led_on(void)
{
    FL_GPIO_SetOutputPin(GPIOA, FL_GPIO_PIN_15);
}

/*=============================================================
 * 函数名称：led_tiggle
 * 函数功能：LED翻转
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-10-23          戴辉发             创建
==============================================================*/
void led_tiggle(void)
{
    FL_GPIO_ToggleOutputPin(GPIOA, FL_GPIO_PIN_15);
    rand_data = rand();
}
/*=============================================================
 * 函数名称：alarm_led_show_process
 * 函数功能：告警显示模块处理流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-05          戴辉发             创建
==============================================================*/
void alarm_led_show_process(void)
{
    if (alarm_led_timer) return;
    alarm_led_timer = CHARGE_TIMER;
    
    {
        e_voltage_status vol_status = get_vol_status();
        e_current_status curr_status = get_current_status();
        e_temp_status temp_status = get_temperature_status();
        
        if( E_ABATE_STATUS == get_system_status() )
        {
           FL_GPIO_SetOutputPin( GPIOA, FL_GPIO_PIN_0);
        }
        else if (E_CHARGE_STATUS == get_system_status())
        {
            if ((E_VOL_OVER == vol_status) || 
                (E_C_CH_PROTECT == curr_status) || (E_C_SECOND_PROTECT == curr_status)|| (E_C_DCH_PROTECT == curr_status)||
                (E_TEMP_DISCHDCH_STATUS == temp_status) || (E_TEMP_DISCH_STATUS == temp_status) )
            {
                FL_GPIO_SetOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
            else if ((E_VOL_OVER_ALARM == vol_status) ||
                (E_C_CH_ALARM == curr_status) || 
                (E_TEMP_ALARM_STATUS == temp_status) )
            {    
                
                FL_GPIO_ToggleOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
            else
            {
                FL_GPIO_ResetOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
        }
        else
        {
            if ((E_VOL_UNDER == vol_status) || 
                (E_C_DCH_PROTECT == curr_status) || (E_C_SECOND_PROTECT == curr_status) || 
                (E_TEMP_DISDCH_STATUS == temp_status) || (E_TEMP_DISCHDCH_STATUS == temp_status) )
            {
                FL_GPIO_SetOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
            else if ((E_VOL_UNDER_ALARM == vol_status) ||
                (E_C_CH_ALARM == curr_status) || 
                (E_TEMP_ALARM_STATUS == temp_status) )
            {
                FL_GPIO_ToggleOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
            else
            {
                FL_GPIO_ResetOutputPin( GPIOA, FL_GPIO_PIN_0);
            }
        }
    }
}
/*=============================================================
 * 函数名称：led_show_process
 * 函数功能：显示模块处理流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-12-05          戴辉发             创建
==============================================================*/
void led_show_process(void)
{
    static uint16_t back_timer = IDLE_TIMER;
    uint16_t temp;

    alarm_led_show_process();
    
    /* 获取当前显示定时器规格 */
    if (E_CHARGE_STATUS == get_system_status())
    {
        temp = CHARGE_TIMER;
    }
    else if (E_DISCHARGE_STATUS == get_system_status())
    {
        temp = DISCHARGE_TIMER;
    }
    else
    {
        temp = IDLE_TIMER;
    }
    if (back_timer > temp)
    /* 定时器短了 */
    {
        if ((led_timer + temp) <= back_timer)
        {
            led_timer = 0;
        }
        else
        {
            led_timer = temp - (back_timer - led_timer);
        }
        back_timer = temp;
    }
    else if (back_timer < temp)
    /* 定时器长了 */
    {
        led_timer += (temp - back_timer);
        back_timer = temp;
    }
    if (led_timer)  return;

    led_timer = temp;
    back_timer = led_timer;
	led_tiggle();
    
}
                      