/*
 * mode_manage.c
 *
 *  Created on: 2018年10月19日
 *      Author: daihuifa
 */

#include <stdint.h>
#include "mode_manage.h"
#include "system_control.h"
#include "key_status.h"
#include "ch_detect.h"
#include "power.h"
#include "low_power.h"
#include "switch_status.h"
#include "vol_manage.h"
#include "balance.h"
#include "adc_sampling.h"
#include "current_manage.h"
#include "hardware.h"
#include "ch_addition.h"
#include "run_record.h"
#include "afe_app.h"
   
#define E_MODE_LOWPOWER_NORMAL_STATUS            (0 << 0)  /* 正常状态 */
#define E_MODE_LOWPOWER_KEY_STATUS               (1 << 0)  /* 开关断开状态 */
#define E_MODE_LOWPOWER_IDLE_STATUS              (1 << 1)  /* 小电流状态 */
#define E_MODE_LOWPOWER_STATUS                   (1 << 2)  /* 低功耗状态 */
#define E_MODE_LOWPOWER_OVERLOAD_STATUS          (1 << 3)  /* 低功耗过载状态 */
#define E_MODE_LOWPOWER_POWER_STATUS             (1 << 4)  /* 主电源坏状态 */

/* 模式管理事件 */
typedef enum _E_MODE_MSG_
{
	E_MODE_KEY_STATUS_MSG, /* 开关状态指示 */
	E_MODE_IDLE_TIMER_MSG, /* 小电流定时指示 */
	E_MODE_LOW_POWER_TIMER_MSG, /* 低功耗定时器指示 */
	E_MODE_LOW_POWER_OVERLOAD_MSG, /* 低功耗过载指示 */
	E_MODE_SAMPLE_STATUS_MSG, /* 采样状态指示 */
    E_MODE_POWER_INVALID_MSG, /* 主电源失效指示 */
	E_MODE_MAX_MSG_NUM
}e_mode_msg;

typedef void (*mode_status_fun)(void);

/* status: E_MODE_INIT_STATUS,消息处理函数表 */
/* E_MODE_KEY_STATUS_MSG */
static void mode_init_key_status_indication(void);
/* E_MODE_IDLE_TIMER_MSG */
static void mode_init_idle_timer_indication(void);
/* E_MODE_LOW_POWER_TIMER_MSG */
static void mode_init_low_power_timer_indication(void);
/* E_MODE_LOW_POWER_OVERLOAD_MSG */
static void mode_init_low_power_overload_indication(void);
/* E_MODE_SAMPLE_STATUS_MSG */
static void mode_init_sample_status_indication(void);
/* E_MODE_POWER_INVALID_MSG, 主电源失效指示 */
static void mode_init_power_invalid_indication(void);

/* status: E_MODE_NORMAL_STATUS,消息处理函数表 */
/* E_MODE_KEY_STATUS_MSG */
static void mode_normal_key_status_indication(void);
/* E_MODE_IDLE_TIMER_MSG */
static void mode_normal_idle_timer_indication(void);
/* E_MODE_LOW_POWER_TIMER_MSG */
static void mode_normal_low_power_timer_indication(void);
/* E_MODE_LOW_POWER_OVERLOAD_MSG */
static void mode_normal_low_power_overload_indication(void);
/* E_MODE_SAMPLE_STATUS_MSG */
static void mode_normal_sample_status_indication(void);
/* E_MODE_POWER_INVALID_MSG, 主电源失效指示 */
static void mode_normal_power_invalid_indication(void);

/* status: E_MODE_SLEEP_STATUS,消息处理函数表 */
/* E_MODE_KEY_STATUS_MSG */
/* E_MODE_IDLE_TIMER_MSG */
/* E_MODE_LOW_POWER_TIMER_MSG */
/* E_MODE_LOW_POWER_OVERLOAD_MSG */
/* E_MODE_SAMPLE_STATUS_MSG */

static const mode_status_fun mode_status_fun_buff[E_MODE_MAX_STATUS_NUM][E_MODE_MAX_MSG_NUM] =
{
	{/* STATUS: E_MODE_INIT_STATUS */
		mode_init_key_status_indication, /* E_MODE_KEY_STATUS_MSG */
		mode_init_idle_timer_indication, /* E_MODE_IDLE_TIMER_MSG */
		mode_init_low_power_timer_indication, /* E_MODE_LOW_POWER_TIMER_MSG */
		mode_init_low_power_overload_indication, /* E_MODE_LOW_POWER_OVERLOAD_MSG */
		mode_init_sample_status_indication, /* E_MODE_SAMPLE_STATUS_MSG */
        mode_init_power_invalid_indication, /* E_MODE_POWER_INVALID_MSG */
	},
	{/* STATUS: E_MODE_NORMAL_STATUS */
		mode_normal_key_status_indication, /* E_MODE_KEY_STATUS_MSG */
		mode_normal_idle_timer_indication, /* E_MODE_IDLE_TIMER_MSG */
		mode_normal_low_power_timer_indication, /* E_MODE_LOW_POWER_TIMER_MSG */
		mode_normal_low_power_overload_indication, /* E_MODE_LOW_POWER_OVERLOAD_MSG */
		mode_normal_sample_status_indication, /* E_MODE_SAMPLE_STATUS_MSG */
        mode_normal_power_invalid_indication, /* E_MODE_POWER_INVALID_MSG */
	},
	{/* STATUS: E_MODE_SLEEP_STATUS */
		(mode_status_fun)0, /* E_MODE_KEY_STATUS_MSG */
		(mode_status_fun)0, /* E_MODE_IDLE_TIMER_MSG */
		(mode_status_fun)0, /* E_MODE_LOW_POWER_TIMER_MSG */
		(mode_status_fun)0, /* E_MODE_LOW_POWER_OVERLOAD_MSG */
		(mode_status_fun)0, /* E_MODE_SAMPLE_STATUS_MSG */
		(mode_status_fun)0, /* E_MODE_POWER_INVALID_MSG */
	},
};

static e_mode_status g_mode_status; /* 系统模式状态 */
static uint32_t g_mode_lowpower_status; /* 低功耗标识 */
static volatile uint16_t g_init_timer;
static volatile uint8_t g_harddwg_flag; /* 片外看门狗喂狗标志 */

/*=============================================================
* 函数名称：mode_manage_mem_init
* 函数功能：模式管理模块内存初始化
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人            修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_manage_mem_init(void)
{
	g_mode_status = E_MODE_INIT_STATUS;
	g_mode_lowpower_status = E_MODE_LOWPOWER_NORMAL_STATUS;
	g_init_timer = 20;
    g_harddwg_flag = 0;
    write_rundata_flag = 1;
    write_rundata_delay = 5;
}

/*==============================================================
* 函数名称：mode_manage_timer
* 函数功能：模式管理模块定时器
* 参数个数：0
* 函数参数：
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期              修改人             修改内容
* 2019-07-10       	戴辉发     	  创建
==============================================================*/
void mode_manage_timer(void)
{
	if (g_init_timer > 0) g_init_timer --;
}

/*=============================================================
 * 函数名称：get_harddwg_flag
 * 函数功能：获取硬件喂狗标志
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0           硬件喂狗
 *          1           硬件不喂狗
 * 修改记录：
 *===============================================================
 * 日    期             修改人       修改类型
 * 2020-03-20           戴辉发     	创建
==============================================================*/
uint8_t get_harddwg_flag(void)
{
	return g_harddwg_flag;
}

/*=============================================================
* 函数名称：get_system_mode_status
* 函数功能：获取系统模式
* 参数个数：0
* 参数描述：
* 返 回 值：
*          当前系统模式
* 修改记录：
*===============================================================
* 日    期                     修改人            修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
e_mode_status get_system_mode_status(void)
{
	return g_mode_status;
}

/*=============================================================
* 函数名称：mode_status_msg_process
* 函数功能：模式管理状态事件表处理函数
* 参数个数：1
* 函数参数：
*           [IN]      msg_type           输入的消息事件
* 返回值：
*           无
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
static void mode_status_msg_process(e_mode_msg msg_type)
{
	if (g_init_timer > 0) return;

	if ((msg_type < E_MODE_MAX_MSG_NUM) && (g_mode_status < E_MODE_MAX_STATUS_NUM))
	{
		mode_status_fun func_addr;

		func_addr = mode_status_fun_buff[g_mode_status][msg_type];
		if (func_addr != ((mode_status_fun) 0))
		{
			func_addr();
		}
	}
}

/* status: E_MODE_INIT_STATUS,消息处理函数表 */
/* E_MODE_KEY_STATUS_MSG */
/***********************************************************
⒈开关状态
  ⑴断开
    ①低功耗标识 = 开关
    ②关闭系统控制模块
    ③采样芯片状态 = SLEEP_STATUS
    ④状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_init_key_status_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_key_status_indication(void)
{
    e_key_status key_status;
   
    key_status = get_key_status();
    /* ⒈开关状态 */
    if ((E_KEY_OPENED_STATUS == key_status) && (ChargerIsConnect() == 0) )
    /*  ⑴断开 */
    {
	/*     ①低功耗标识 = 开关 */
    	g_mode_lowpower_status |= E_MODE_LOWPOWER_KEY_STATUS;
	/*     ①关闭系统控制模块 */
		control_enable_request(0);
        balance_close();
	/*     ③采样芯片状态 = SLEEP_STATUS */
	/*     ④状态 = SLEEP_STATUS */
        if(g_mode_status != E_MODE_SLEEP_STATUS)
		{
            g_mode_status = E_MODE_SLEEP_STATUS;
            write_rundata_flag = 1;
        }
    }
}

/* E_MODE_IDLE_TIMER_MSG */
/***********************************************************
⒈低功耗标识 = 小电流
⒉采样芯片状态 = INIT_STATUS
⒊状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_init_idle_timer_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_idle_timer_indication(void)
{
	/* ⒈低功耗标识 = 小电流 */
	g_mode_lowpower_status |= E_MODE_LOWPOWER_IDLE_STATUS;
	/* ⒉采样芯片状态 = INIT_STATUS */
	/* ⒊状态 = SLEEP_STATUS */
	if( g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
}

/* E_MODE_LOW_POWER_TIMER_MSG */
/***********************************************************
⒈低功耗标识 = 低功耗
⒉采样芯片状态 = INIT_STATUS
⒊状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_init_low_power_timer_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_low_power_timer_indication(void)
{
	/* ⒈低功耗标识 = 低功耗 */
	g_mode_lowpower_status |= E_MODE_LOWPOWER_STATUS;
	/* ⒉采样芯片状态 = INIT_STATUS */
	/* ⒊状态 = SLEEP_STATUS */
	if( g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
}

/* E_MODE_LOW_POWER_OVERLOAD_MSG */
/***********************************************************
⒈低功耗标识 = 低功耗过载
⒉采样芯片状态 = INIT_STATUS
⒊状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_init_low_power_overload_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_low_power_overload_indication(void)
{
	/* ⒈低功耗标识 = 低功耗过载 */
	g_mode_lowpower_status |= E_MODE_LOWPOWER_OVERLOAD_STATUS;
	/* ⒉采样芯片状态 = INIT_STATUS */
	/* ⒊状态 = SLEEP_STATUS */
	if( g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
}

/* E_MODE_SAMPLE_STATUS_MSG */
/***********************************************************
⒈采样芯片状态 = 采样芯片状态指示中芯片状态
⒉采样芯片状态 == 正常
  ⑴真
    ①使能系统控制模块
    ②状态 = NORMAL_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_init_sample_status_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_sample_status_indication(void)
{
    e_key_status key_status;
    
    key_status = get_key_status();
    /* ⒈采样芯片状态 = 采样芯片状态指示中芯片状态 */
	/* ⒉采样芯片状态 == 正常 */
	if ((judge_vol_sample_finished()) && 
        ((E_KEY_CLOSED_STATUS == key_status)
          || (ChargerIsConnect() == 1)))

	/*   ⑴真 */
	{
	/*     ①使能系统控制模块 */
		control_enable_request(1);
        balance_start();
	/*     ②状态 = NORMAL_STATUS */
		g_mode_status = E_MODE_NORMAL_STATUS;
	}
}

/* E_MODE_POWER_INVALID_MSG, 主电源失效指示 */
/***********************************************************
⒈采样芯片状态 = 采样芯片状态指示中芯片状态
⒉采样芯片状态 == 正常 && 网络状态 == 正常
  ⑴真
    ①使能系统控制模块
    ②状态 = NORMAL_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：mode_init_power_invalid_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_init_power_invalid_indication(void)
{
	/* ⒈低功耗标识 = 低功耗过载 */
	g_mode_lowpower_status |= E_MODE_LOWPOWER_POWER_STATUS;
	/* ⒉采样芯片状态 = INIT_STATUS */
	/* ⒊状态 = SLEEP_STATUS */
	if(g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
}

/* status: E_MODE_NORMAL_STATUS,消息处理函数表 */
/* E_MODE_KEY_STATUS_MSG */
/***********************************************************
⒈开关状态
  ⑴断开
    ①低功耗标识 = 开关
    ②关闭系统控制模块
    ③采样芯片状态 = SLEEP_STATUS
    ④状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：mode_normal_key_status_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_normal_key_status_indication(void)
{
    e_key_status key_status;
    
    key_status = get_key_status();
    /* ⒈开关状态 */
    if ((E_KEY_OPENED_STATUS == key_status)&& (ChargerIsConnect() == 0))
    /*  ⑴断开 */
    {
		e_system_status sys_status = get_system_status();
	/*     ①低功耗标识 = 开关 */
    	g_mode_lowpower_status |= E_MODE_LOWPOWER_KEY_STATUS;
	/*     ①关闭系统控制模块 */
		control_enable_request(0);
        balance_close();
	/*     ③采样芯片状态 = SLEEP_STATUS */
	/*     ④状态 = SLEEP_STATUS */
		if(g_mode_status != E_MODE_SLEEP_STATUS)
        {
            g_mode_status = E_MODE_SLEEP_STATUS;
            write_rundata_flag = 1;
        }
	/* 关闭充电器 */
		if (E_CHARGE_STATUS == sys_status)
		{
			send_closed_rxpdo_to_charger();
		}
    }
}

/* E_MODE_IDLE_TIMER_MSG */
/***********************************************************
⒈低功耗标识 = 小电流
⒉关闭系统控制模块
⒊采样芯片状态 = SLEEP_STATUS
⒋状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_normal_idle_timer_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-20          戴辉发             创建
==============================================================*/
static void mode_normal_idle_timer_indication(void)
{
	e_system_status sys_status = get_system_status();

	/* ⒈低功耗标识 = 小电流 */
    g_mode_lowpower_status |= E_MODE_LOWPOWER_IDLE_STATUS;
    /* ⒉关闭系统控制模块 */
    control_enable_request(0);
    balance_close();
    /* ⒊采样芯片状态 = SLEEP_STATUS */
    /* ⒋状态 = SLEEP_STATUS */
    if(g_mode_status != E_MODE_SLEEP_STATUS)
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
	/* 关闭充电器 */
	if (E_CHARGE_STATUS == sys_status)
	{
		send_closed_rxpdo_to_charger();
	}
}

/* E_MODE_LOW_POWER_TIMER_MSG */
/***********************************************************
⒈低功耗标识 = 低功耗
⒉关闭系统控制模块
⒊采样芯片状态 = INIT_STATUS
⒋联网状态 = INIT_STATUS
⒌状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_normal_low_power_timer_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-20          戴辉发             创建
==============================================================*/
static void mode_normal_low_power_timer_indication(void)
{
	e_system_status sys_status = get_system_status();

	/* ⒈低功耗标识 = 低功耗 */
    g_mode_lowpower_status |= E_MODE_LOWPOWER_STATUS;
    /* ⒉关闭系统控制模块 */
    control_enable_request(0);
    balance_close();
    /* ⒊采样芯片状态 = SLEEP_STATUS */
    /* ⒋状态 = SLEEP_STATUS */
    if(g_mode_status != E_MODE_SLEEP_STATUS)
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
	/* 关闭充电器 */
	if (E_CHARGE_STATUS == sys_status)
	{
		send_closed_rxpdo_to_charger();
	}
}

/* E_MODE_LOW_POWER_OVERLOAD_MSG */
/***********************************************************
⒈低功耗标识 = 低功耗过载
⒉关闭系统控制模块
⒊采样芯片状态 = INIT_STATUS
⒋联网状态 = INIT_STATUS
⒌状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_normal_low_power_overload_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-20          戴辉发             创建
==============================================================*/
static void mode_normal_low_power_overload_indication(void)
{
	e_system_status sys_status = get_system_status();

	/* ⒈低功耗标识 = 低功耗过载 */
    g_mode_lowpower_status |= E_MODE_LOWPOWER_OVERLOAD_STATUS;
    /* ⒉关闭系统控制模块 */
    control_enable_request(0);
    balance_close();
    /* ⒊采样芯片状态 = SLEEP_STATUS */
    /* ⒋状态 = SLEEP_STATUS */
    if( g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
	/* 关闭充电器 */
	if (E_CHARGE_STATUS == sys_status)
	{
		send_closed_rxpdo_to_charger();
	}
}

/* E_MODE_SAMPLE_STATUS_MSG */
/***********************************************************
⒈采样芯片状态 = 采样芯片状态指示中芯片状态
⒉采样芯片状态
  ⑴配置状态
    ①关闭系统控制模块
    ②状态 = INIT_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_normal_key_status_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-19          戴辉发             创建
==============================================================*/
static void mode_normal_sample_status_indication(void)
{
	/* ⒈采样芯片状态 = 采样芯片状态指示中芯片状态 */
	/* ⒉采样芯片状态 */
	if (0 == judge_vol_sample_finished())
	/*   ⑴配置状态 */
	{
		e_system_status sys_status = get_system_status();
	/*     ①关闭系统控制模块 */
		control_enable_request(0);
        balance_close();
	/*     ②状态 = INIT_STATUS */
		g_mode_status = E_MODE_INIT_STATUS;
	/* 关闭充电器 */
		if (E_CHARGE_STATUS == sys_status)
		{
			send_closed_rxpdo_to_charger();
		}
	}
}

/* E_MODE_POWER_INVALID_MSG, 主电源失效指示 */
/***********************************************************
⒈低功耗标识 = 主电源失效
⒉关闭系统控制模块
⒊采样芯片状态 = INIT_STATUS
⒋联网状态 = INIT_STATUS
⒌状态 = SLEEP_STATUS
***********************************************************/
/*=============================================================
* 函数名称：mode_normal_power_invalid_indication
* 参数个数：0
* 函数参数：
* 返回值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-10-20          戴辉发             创建
==============================================================*/
static void mode_normal_power_invalid_indication(void)
{
	e_system_status sys_status = get_system_status();

	/* ⒈低功耗标识 = 低功耗过载 */
    g_mode_lowpower_status |= E_MODE_LOWPOWER_POWER_STATUS;
    /* ⒉关闭系统控制模块 */
    control_enable_request(0);
    balance_close();
    /* ⒊采样芯片状态 = SLEEP_STATUS */
    /* ⒋状态 = SLEEP_STATUS */
    if( g_mode_status != E_MODE_SLEEP_STATUS )
    {
        g_mode_status = E_MODE_SLEEP_STATUS;
        write_rundata_flag = 1;
    }
	/* 关闭充电器 */
	if (E_CHARGE_STATUS == sys_status)
	{
		send_closed_rxpdo_to_charger();
	}
}


/*=============================================================
* 函数名称：mode_sleep_status_process
* 函数功能：模式管理模块睡眠状态处理流程
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人            修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_sleep_status_process(void)
{
    if (g_mode_status == E_MODE_SLEEP_STATUS)
    {/* 低功耗标识 */
        if (((0 == get_ch_switch_status()) && (0 == get_dch_switch_status())) || (get_afe_status() != 0))
        /* 功率开关状态下断开允许关机 如果通讯失败也进入*/
        {   
            /* 进入低功耗模式 */
             mode_into_low_power_mode(200);               
        }
    }
}

/*=============================================================
* 函数名称：mode_key_status_indication
* 函数功能：模式管理模块开关状态指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_key_status_indication(void)
{
	mode_status_msg_process(E_MODE_KEY_STATUS_MSG);
}
/*=============================================================
* 函数名称：mode_idle_timer_indication
* 函数功能：模式管理模块小电流指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_idle_timer_indication(void)
{
	mode_status_msg_process(E_MODE_IDLE_TIMER_MSG);
}

/*=============================================================
* 函数名称：mode_low_power_timer_indication
* 函数功能：模式管理模块低功耗定时指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_low_power_timer_indication(void)
{
	mode_status_msg_process(E_MODE_LOW_POWER_TIMER_MSG);
}

/*=============================================================
* 函数名称：mode_low_power_overload_indication
* 函数功能：模式管理模块低功耗过载指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_low_power_overload_indication(void)
{
	mode_status_msg_process(E_MODE_LOW_POWER_OVERLOAD_MSG);
}

/*=============================================================
* 函数名称：mode_sample_status_indication
* 函数功能：模式管理模块采样状态指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_sample_status_indication(void)
{
	mode_status_msg_process(E_MODE_SAMPLE_STATUS_MSG);
}

/*=============================================================
* 函数名称：mode_power_invalid_indication
* 函数功能：模式管理模块主电源不正确指示
* 参数个数：0
* 参数描述：
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-19       	戴辉发     	创建
==============================================================*/
void mode_power_invalid_indication(void)
{
	mode_status_msg_process(E_MODE_POWER_INVALID_MSG);
}
