#include "low_power.h"
#include "string.h"
#include "parameter.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "soc.h"
#include "system_control.h"
#include "mode_manage.h"
#include "key_status.h"
#include "ch_detect.h"
#include "system_adjust.h"
#include "ch_addition.h"
#include "afe_app.h"
#include "storage_manage.h"

#define TIME_POWER_1S                  110
#define MIN_LOW_POWER_DELAY            2
#define MIN_DELAY_MINTUE               4
#define CH_OVER_TIME                   60 /* 充电完成后关机时间 */
#define CH_SMALL_MINTUE                90

#define CHARGER_LOWPOWER_3S            3
#define CHARGER_LOWPOWER_5S            5
#define POWER_ON_FACTORY_COUNT         (60 * 24 * 10) /* 生产模式保持10天 */
#define POWER_ON_KEEP_COUNT            POWER_ON_FACTORY_COUNT
#define FACTORY_CELL_LOW_RECOVER       2450
#define NORMAL_CELL_LOW_RECOVER        2900
#define POWER_ON_PDR_RESET_FLAG        0x100
#define POWER_ON_POR_RESET_FLAG        0x200

static volatile uint16_t t_afe_error_time;
static volatile uint16_t t_low_power_time;
static volatile uint16_t t_deactive_time;
static volatile uint16_t t_ch_over_time;

static volatile uint8_t send_charger_flag;
static volatile uint8_t send_charger_delay;
uint32_t PowerOnCount;
uint32_t setFactroyModeDelay;
static uint8_t lastPowerOnFlag;
static uint32_t powerOnStoreFlag;
extern uint32_t mcu_reset;

static void PowerOnSetFlag(uint8_t flag);
static uint8_t PowerOnGetFlag(uint8_t *flag);
/*=============================================================
 * 函数名称：low_m_timer_process
 * 函数功能：低功耗定时器处理模块
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-23        戴辉发     	创建
==============================================================*/
void low_m_timer_process(void)
{
	static uint8_t s_timer = 0;
    static uint8_t hourTimer = 0;

	s_timer ++;
	if (t_ch_over_time) t_ch_over_time --;
    if (send_charger_delay) send_charger_delay --;
	if (s_timer >= 60)
    {
		s_timer = 0;
		if (t_deactive_time) t_deactive_time --;
        if (t_low_power_time) t_low_power_time --;
        if (t_afe_error_time) t_afe_error_time--;
        AddSocPowerOnCount();
        hourTimer++;
        if (hourTimer >= 60)
        {
            hourTimer = 0;
            if (setFactroyModeDelay != 0)
            {
                setFactroyModeDelay--;
            }
        }
	}
}

static void PowerOnSetFlag(uint8_t flag)
{
    t_write_data_st data;

    powerOnStoreFlag = 0xa4;
    powerOnStoreFlag <<= 16;
    powerOnStoreFlag += flag;
    powerOnStoreFlag <<= 8;
    powerOnStoreFlag += 0x4a;

    data.rsvd = 0;
    data.msg_type = E_POWER_ON_FLAG_MSG;
    data.write_len = sizeof(powerOnStoreFlag);
    data.write_memery_address = (uint8_t *)&powerOnStoreFlag;
    data.write_storge_address = POWER_ON_SECTOR_MAIN;
    set_write_data_to_queue(&data);
    storage_manage_process();
}

static uint8_t PowerOnGetFlag(uint8_t *flag)
{
    uint8_t ret = 0;
    uint32_t powerOnFlag = 0;
    
    if (sizeof(powerOnFlag) == read_data_from_storage(POWER_ON_SECTOR_MAIN, (uint8_t *)&powerOnFlag, sizeof(powerOnFlag)))
    {
        if ((powerOnFlag & 0xFF0000FF) == 0xa400004a)
        {
            *flag = (powerOnFlag >> 8) & 0xFF;
            ret = 1;
        }
    }
    
    return ret;
}

void AddSocPowerOnCount(void)
{
    if (PowerOnCount < 0xFFFFFF00)
    {
        PowerOnCount++;
    }
}

uint8_t GetPowerOnOverOneDay(void)
{
    uint8_t flag = 0;
    
    if (PowerOnCount >= POWER_ON_FACTORY_COUNT)
    {
        flag = 1;
    }
    
    return flag;
}

void UpdatePowerOnCount(uint32_t times)
{
    uint32_t timerMin = times / 60;

    if (PowerOnCount < 0xFFFFFF00)
    {
        if ((0xFFFFFF00 - PowerOnCount) > timerMin)
        {
            PowerOnCount += timerMin;
        }
        else
        {
            PowerOnCount = 0xFFFFFF00;
        }
    }

}

void PowerCountInit(void)
{
    uint8_t powerOnFlag = 0;
    uint8_t successRead;
    
    successRead = PowerOnGetFlag(&powerOnFlag);
    if (successRead == 1)
    {
        if (((mcu_reset & POWER_ON_PDR_RESET_FLAG) != 0) ||
            ((mcu_reset & POWER_ON_POR_RESET_FLAG) != 0))
        {
            PowerOnCount = 0;
            lastPowerOnFlag = 0;
            if (powerOnFlag != 0)
            {
                PowerOnSetFlag(lastPowerOnFlag);
            }
        }
        else if (powerOnFlag != 0)
        {
            PowerOnCount = POWER_ON_KEEP_COUNT;
            lastPowerOnFlag = 1;
        }
        else
        {
            lastPowerOnFlag = 0;
        }
    }
    else
    {
        lastPowerOnFlag = GetPowerOnOverOneDay();
    }

#if defined(TIANFENG) && defined(BAT_8S)
    if (0 == GetPowerOnOverOneDay())
    {
        set_cell_vol_low_recover(FACTORY_CELL_LOW_RECOVER);
    }
    else
    {
        set_cell_vol_low_recover(NORMAL_CELL_LOW_RECOVER);
    }
#endif
}

void UpdateFactroyModeDelay(uint32_t times)
{
    uint32_t timerh = times / 3600;
    
    if (setFactroyModeDelay > timerh)
    {
        setFactroyModeDelay -= timerh;
    }
    else
    {
        setFactroyModeDelay = 0;
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
 * 2018-10-30        戴辉发     	创建
==============================================================*/
void init_low_power_status(void)
{
	t_low_power_time = get_low_deactive_delay();
    t_afe_error_time = MIN_LOW_POWER_DELAY;
    if (t_low_power_time < MIN_LOW_POWER_DELAY) t_low_power_time = MIN_LOW_POWER_DELAY;
    
	set_t_deactive_time(get_deactive_delay());
    send_charger_flag = 0;
    send_charger_delay = 0;   
}

/*******************************************************************************
**FuncName: get_send_charger_flag;//
**Function: 获取send_charger_flag;
**Output  : 无;
**input   : 无;
**Create date : liyong @2021/4/25
**Modify  : 
*******************************************************************************/
uint8_t get_send_charger_flag(void)	//
{
	return send_charger_flag;
}

/*=============================================================
 * 函数名称：low_power_manage
 * 函数功能：低功耗管理模块处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-23        戴辉发     	创建
==============================================================*/
void low_power_manage(void)
{
	if ((judge_vol_sample_finished()))
	/* 电压采样完成 */
	{
        uint16_t temp = get_low_deactive_delay();

		/* 欠压保护且为放电状态 */
		if (( 1 == judge_low_voltage() ) && (E_CHARGE_STATUS != get_system_status()))
		{
			/* 欠压保护状态 */
			if( t_low_power_time == 0 )
			{
                mode_low_power_overload_indication();    
			}
			else
			{
				if (t_low_power_time > temp)
				{
					if (temp >= MIN_LOW_POWER_DELAY)
					{
						t_low_power_time = temp;
					}
				}
			}
		}
		else
		{
			t_low_power_time = temp;
			if (t_low_power_time < MIN_LOW_POWER_DELAY)
            {
                t_low_power_time = MIN_LOW_POWER_DELAY;
            }
		}
	}

	if ( 1 == get_afe_status() )
	/* 通讯故障 */
    {
        if( t_afe_error_time == 0 )
        {
            t_afe_error_time = MIN_LOW_POWER_DELAY;
            mode_low_power_timer_indication();
        }
    }
    else 
    {
		t_afe_error_time = MIN_LOW_POWER_DELAY;
    }
}

void set_t_deactive_time(uint16_t delay_num)
{
	if ( delay_num < MIN_DELAY_MINTUE )
	{
		t_deactive_time = MIN_DELAY_MINTUE;
	}
	else
	{
		t_deactive_time = delay_num;
	}
}

/*=============================================================
 * 函数名称：deactive_manage
 * 函数功能：系统不活动管理模块处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-23       	戴辉发     	创建
==============================================================*/
void deactive_manage(void)
{
	static uint8_t back_flag = 0xff;
	uint8_t flag = get_current_power_flag();

	/* 低功耗开启标准 */
	switch (flag)
	{
	case 0:
		set_t_deactive_time(get_deactive_delay());
		break;
	case 1:
		if (back_flag != flag)
		{
			set_t_deactive_time(get_deactive_delay());
		}
		if (t_deactive_time == 0)
		/* 小电流状态维持了设定时间，需要进入睡眠 */
		{
            mode_idle_timer_indication();
		}
		else
		{
			if ((t_deactive_time > get_deactive_delay()) && (t_deactive_time > MIN_DELAY_MINTUE))
			{
				set_t_deactive_time(get_deactive_delay());
			}
		}
		break;
	case 2:
#if defined (TIANHONG)/* 天宏8串磷酸铁锂 */
        
#else
		if (back_flag != flag)
		{          
            if( send_charger_flag == 0 )
            {
               send_charger_flag = 1;
               send_charger_delay = CHARGER_LOWPOWER_5S;
            }           
		}
#endif
		break;
    case 3:
		if (back_flag != flag)
		{
            set_t_deactive_time(get_deactive_delay() + 30);
		}
        if (0 == t_deactive_time)
        {
            mode_low_power_overload_indication();
        }
        break;
	}
    
    if( send_charger_flag == 1 )
    {
      if( send_charger_delay == 0 )
      {
        send_charger_flag = 2;
        mode_low_power_overload_indication();
      }
    }
    
	back_flag = flag;
}

/*=============================================================
 * 函数名称：ch_over_deactive_manage
 * 函数功能：过充保护休眠流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2020-02-28        戴辉发     	创建
==============================================================*/
static void ch_over_deactive_manage(void)
{
    if ( get_g_grand_status() == E_ENABLE_STATUS )
    {
        if ( 1 == judge_ch_finished() )
        /* 充电完成 */
        {
            if (0 == t_ch_over_time)
            {
                mode_low_power_overload_indication();
                t_ch_over_time = CH_OVER_TIME;
            }
        }
        else
        {
            t_ch_over_time = CH_OVER_TIME;
        }
    }
    else
    {
        t_ch_over_time = CH_OVER_TIME;
    }
}

/*=============================================================
 * 函数名称：low_power_process
 * 函数功能：低功耗开关处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-10-16        戴辉发     	 创建
==============================================================*/
void low_power_process(void)
{
#if defined(TIANFENG) && defined(BAT_8S)
    if ((setFactroyModeDelay != 0) || (GetPowerOnOverOneDay() == 0))
    {
        if (get_cell_vol_low_recover() != FACTORY_CELL_LOW_RECOVER)
        {
            set_cell_vol_low_recover(FACTORY_CELL_LOW_RECOVER);
        }
    }
    else
    {
        if (get_cell_vol_low_recover() != NORMAL_CELL_LOW_RECOVER)
        {
            set_cell_vol_low_recover(NORMAL_CELL_LOW_RECOVER);
        }
    }
#endif

    if (lastPowerOnFlag != GetPowerOnOverOneDay())
    {
        lastPowerOnFlag = GetPowerOnOverOneDay();
        PowerOnSetFlag(lastPowerOnFlag);
    }

    /* 不活动低功耗处理流程 */
     deactive_manage();
    /* 低功耗处理流程 */
     low_power_manage();
    /* 充电过流保护低功耗休眠 */
     ch_over_deactive_manage();
}
