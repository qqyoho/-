/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：key_status.c
* 文件功能描述：实现钥匙开关状态检测
* 
* 修改记录：
* 2018-07-05 戴辉发 创建
*----------------------------------------------------------*/
#include "key_status.h"
#include "fm33lg0xx_fl.h"
#include "power.h"
#include "hardware.h"
#include "ch_detect.h"
#include "current_manage.h"
#include "low_power.h"
#include "parameter.h"
#include "mode_manage.h"

#define KEY_DETECT_TIME    1
#define KEY_DETECT_ON_NUM  105
#define KEY_DETECT_OFF_NUM 80

#define KEY_DETECT_GPIO    FL_GPIO_PIN_5
#define KEY_DETECT_GPROUP  GPIOA

static volatile uint16_t key_check_timer;


static e_key_status g_key_status;
static uint8_t g_active_key_status;
static uint8_t g_sleep_key_status;
static uint8_t g_key_num;

/*=============================================================
* 函数名称：key_mem_init
* 函数功能：钥匙连接状态检测模块内存初始化
* 参数个数：0
* 参数描述：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void key_mem_init(void)
{
	key_check_timer = KEY_DETECT_TIME;
	g_key_status = E_KEY_INIT_STATUS;
	g_active_key_status = 0;
	g_sleep_key_status = 0;
	set_key_check_wake_flag(0);    
    g_key_num = 0;
}



/*=============================================================
* 函数名称：key_timer_ms_run
* 函数功能：钥匙连接状态定时器
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2018-06-27          戴辉发             创建
==============================================================*/
void key_timer_ms_run(void)
{
	if (key_check_timer) key_check_timer --;
}

/*=============================================================
* 函数名称：get_key_status
* 函数功能：获取钥匙连接状态
* 参数个数：0
* 参数描述：
* 返 回 值：
*          开关机键状态
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-05       	戴辉发     	创建
==============================================================*/
e_key_status get_key_status(void)
{
	return g_key_status;
}

/*=============================================================
* 函数名称：set_key_off_status
* 函数功能：设定开关机键断开状态
* 参数个数：0
* 参数描述：
* 返 回 值：
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-20       	戴辉发     	创建
==============================================================*/
void set_key_off_status(void)
{
	g_key_status = E_KEY_OPENED_STATUS;
	g_sleep_key_status = 0;
    g_active_key_status = 0;
    g_key_num = 0;
}

/*=============================================================
* 函数名称：set_key_on_status
* 函数功能：设定开关机键闭合状态
* 参数个数：0
* 参数描述：
* 返 回 值：
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-20       	戴辉发     	创建
==============================================================*/
void set_key_on_status(void)
{
	g_key_status = E_KEY_CLOSED_STATUS;
	g_sleep_key_status = 0;
    g_active_key_status = 0;
    g_key_num = 0;
}

/*=============================================================
* 函数名称：get_key_gpio_status
* 函数功能：获取开关机键管脚状态
* 参数个数：0
* 参数描述：
* 返 回 值：
*           0       低电平
*           1       高电平
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2019-08-08       	戴辉发     	创建
==============================================================*/
uint8_t get_key_gpio_status(void)
{
    if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) != 0 )
    {
        return 1;
    }
    return 0;
}

/*=============================================================
* 函数名称：key_init_detect_process
* 函数功能：检测开关机信号流程
* 参数个数：0
* 参数描述：
* 返 回 值：
*           0       未完成关钥匙过程
*           1       检测到开关键断开
*           2       检测到开关键闭合
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
static uint8_t key_init_detect_process(void)
{
    static uint8_t init_status = 0;
    static uint8_t init_num = 0;
	uint8_t ret = 0;

    init_num += 1;
	switch(init_status)
	{
	case 0:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) == 0 )
		/* 首次检查到关机信号 */
		{
			g_key_num = 1;
			init_status = 1;
		}
        else
        {
			g_key_num = 1;
			init_status = 2;
        }
		break;
	case 1:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) == 0 )
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_key_num += 1;
			if (g_key_num >= KEY_DETECT_OFF_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				ret = 1;
                init_status = 0;
				g_key_num = 0;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			init_status = 2;
            g_key_num = 1;
		}
		break;
    case 2:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) != 0 )
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_key_num += 1;
			if (g_key_num >= KEY_DETECT_OFF_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				ret = 2;
                init_status = 0;
				g_key_num = 0;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			init_status = 1;
            g_key_num = 1;
		}
        break;
	default:
		init_status = 0;
		break;
	}
    if ((ret == 0) && (init_num >= (KEY_DETECT_OFF_NUM + KEY_DETECT_OFF_NUM / 2)))
    {
        ret = init_status;
        init_status = 0;
        g_key_num = 0;
        init_num = 0;
    }
	return ret;
}

/*=============================================================
* 函数名称：key_closed_detect_process
* 函数功能：检测关机信号流程
* 参数个数：0
* 参数描述：
* 返 回 值：
*           0       未完成关钥匙过程
*           1       完成了该过程
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
static uint8_t key_closed_detect_process(void)
{
	uint8_t ret = 0;

	switch(g_sleep_key_status)
	{
	case 0:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) == 0 )
		/* 首次检查到关机信号 */
		{
			g_key_num = 1;
			g_sleep_key_status = 1;
		}
		break;
	case 1:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) == 0 )
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_key_num += 1;
			if (g_key_num >= KEY_DETECT_ON_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				g_sleep_key_status = 2;
				g_key_num = 0;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			g_sleep_key_status = 0;
		}
		break;
	default:
		ret = 1;
		break;
	}
	return ret;
}

/*=============================================================
* 函数名称：key_sleep_detect_process
* 函数功能：检测开机信号流程
* 参数个数：0
* 参数描述：
* 返 回 值：
*           0       未检测到开机信号
*           1       检测到开机信号
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
static uint8_t key_active_detect_process(void)
{
	uint8_t ret = 0;

	switch(g_active_key_status)
	{
	case 0:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) != 0 )
		/* 检查到开机信号 */
		{
			g_key_num = 1;
			g_active_key_status = 1;
		}
		break;
	case 1:
		if (FL_GPIO_GetInputPin( KEY_DETECT_GPROUP,KEY_DETECT_GPIO) != 0 )
		/* 继首次检查到开机信号后，后续再次检查到开机信号 */
		{
			g_key_num += 1;
			if (g_key_num >= KEY_DETECT_OFF_NUM)
			/* 连续检查到开机信号到10次，确认开机信号有效 */
			{
                g_key_num = 0;
				g_active_key_status = 2;
			}
		}
		else
		/* 在判断有效开机信号过程中，检查到关机信号，前述开机信号无效 */
		{
			g_active_key_status = 0;
		}
		break;
	default:
		ret = 1;
		break;
	}
    return ret;
}

/*=============================================================
* 函数名称：key_detect_process
* 函数功能：钥匙连接状态检测流程
* 参数个数：0
* 参数描述：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void key_detect_process(void)
{
	if (key_check_timer > 0) return;
	key_check_timer = KEY_DETECT_TIME;

    switch(g_key_status)
    {
    case E_KEY_INIT_STATUS:
        {
            uint8_t init_flag = key_init_detect_process();
            if (1 == init_flag)
            {
                g_key_status = E_KEY_OPENED_STATUS;
                g_sleep_key_status = 0;
            }
            else if (2 == init_flag)
            {
                g_key_status = E_KEY_CLOSED_STATUS;
                g_active_key_status = 0;
            }
        }
        break;
    case E_KEY_OPENED_STATUS:
		if (key_active_detect_process())
		{
			g_key_status = E_KEY_CLOSED_STATUS;
			g_active_key_status = 0;
		}
        break;
    case E_KEY_CLOSED_STATUS:
        if (key_closed_detect_process())
        {
            g_key_status = E_KEY_OPENED_STATUS;
            g_sleep_key_status = 0;
        }
        break;
    }
    mode_key_status_indication();
}

/*=============================================================
* 函数名称：clear_key_status
* 函数功能：清除钥匙开关机状态
* 参数个数：0
* 参数描述：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void clear_key_status(void)
{
	g_sleep_key_status = 0;
	g_active_key_status = 0;
}
