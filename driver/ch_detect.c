/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：ch_detect.c
 * 文件功能描述：实现充电器检测
 * 
 * 修改记录：
 * 2020-07-08    戴辉发        创建
 *---------------------------------------------------------*/

#include "ch_detect.h"
#include "switch_status.h"
#include "fm33lg0xx_fl.h"
#include "adc_sampling.h"
#include "current_manage.h"
#include "system_control.h"
#include "ch_addition.h"
#include "ch_detect.h"
#include "parameter.h"
#include "vol_manage.h"
#include "key_status.h"
#include "ch_addition.h"
#include "soc.h"
#include "storage_manage.h"
#include "ChHeart.h"

#define CH_EXIT_TIME          300
#define CH_DETECT_TIME        1
#define CH_DETECT_ON_NUM      105
#define CH_DETECT_OFF_NUM     80
#define MAX_CH_STABLE_TIME    1001
#define MIN_CH_STABLE_TIME    201
#define MAX_CH_CHANGE_TIME    201

#define CHARGER_DETECT_DELAY_500MS  50
#define CHARGER_DETECT_DELAY_30S    2500

#define MAX_CH_VOL            3000 /* 最大充电电压 */
#define MIN_CH_VOL            2000 /* 最小充电电压 */
#define CH_DETECT_GROUP       GPIOB
#define CH_DETECT_PORT        FL_GPIO_PIN_6

#if defined (CAN_AWAKE)
#define CAN_AWAKEUP_GPIO       FL_GPIO_PIN_13
#define CAN_AWAKEUP_GPROUP     GPIOB
#endif

volatile static uint16_t g_ch_detect_timer;
volatile static uint16_t g_ch_addi_timer;
volatile static uint16_t g_ch_addi1_timer;

static uint8_t g_ch_detect_status; /* 充电器连接状态 */
static uint8_t g_ch_active_status;
static uint8_t g_ch_sleep_status;
static uint16_t g_ch_num;
static uint8_t g_ch_init_status;
static uint8_t g_ch_init_num;
static volatile uint8_t g_ch_detect_flag; /* 唤醒用 */
static uint16_t chargerType1Delay1;
static uint16_t chargerType1Delay2;
typedef enum{
  CHARGER_KEY_CONNECT,
  CHARGER_CURRENT_CONTINUE,
  CHARGER_CONFIRM
}CHARGER_TYPE_1;
CHARGER_TYPE_1 chargerType1Status;
/*==============================================================
 * 函数名称：ch_detect_mem_init
 * 函数功能：充电器检测模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
void ch_detect_mem_init(void)
{
    g_ch_detect_status = CH_INIT_VALUE;
    g_ch_active_status = 0;
    g_ch_sleep_status = 0;
    g_ch_init_status = 0;
    g_ch_init_num = 0;
    g_ch_num = 0;
    g_ch_detect_flag = 0;
    g_ch_addi_timer = MAX_CH_STABLE_TIME;
    g_ch_addi1_timer = MAX_CH_STABLE_TIME;
    chargerType1Status = CHARGER_KEY_CONNECT;
    chargerType1Delay1 = CHARGER_DETECT_DELAY_500MS;
    chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
}
/*==============================================================
 * 函数名称：ch_detect_wakeup_mem_init
 * 函数功能：充电器检测模块唤醒内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2023-02-24          李勇               创建
==============================================================*/
void ch_detect_wakeup_mem_init(void)
{
    chargerType1Status = CHARGER_KEY_CONNECT;
    chargerType1Delay1 = CHARGER_DETECT_DELAY_500MS;
    chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
}
/*=============================================================
 * 函数名称：getChConnectGpioStatus
 * 函数功能：设置充电器检测的管脚状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       检测管脚为低电平
 *           1       检测管脚为高电平
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2022-05-07        戴辉发      创建
 ==============================================================*/
uint8_t getChConnectGpioStatus(void)
{
    uint8_t ret = 0;

    ret = get_ch_detect_gpio_status();


#if defined (CAN_AWAKE)
    if (FL_GPIO_GetInputPin(CAN_AWAKEUP_GPROUP, CAN_AWAKEUP_GPIO) != 0)
    {
        ret = 1;
    } 
#endif

    return ret;
}

/*=============================================================
 * 函数名称：get_ch_detect_gpio_status
 * 函数功能：获取盲充电源检测管脚状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       低电平
 *           1       高电平
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-07-23        戴辉发      创建
==============================================================*/
uint8_t get_ch_detect_gpio_status(void)
{
    if (FL_GPIO_GetInputPin(CH_DETECT_GROUP, CH_DETECT_PORT) == 1)
    {
        return 1;
    }
    return 0;
}
/*=============================================================
 * 函数名称：set_ch_detect_flag
 * 函数功能：设置盲充电压检测状态
 * 参数个数：1
 * 参数描述：
 *          [IN]     flag        标志
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-07-23        戴辉发      创建
==============================================================*/
void set_ch_detect_flag(uint8_t flag)
{
    g_ch_detect_flag = flag;
}

/*=============================================================
 * 函数名称：get_ch_detect_flag
 * 函数功能：获取盲充电压检测状态
 * 参数个数：0
 * 参数描述： 
 * 返 回 值：
 *          返回当前盲充接入状态
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-07-23        戴辉发      创建
==============================================================*/
uint8_t get_ch_detect_flag(void)
{
    return g_ch_detect_flag;
}

/*==============================================================
 * 函数名称：ch_detect_10ms_timer
 * 函数功能：充电器检测模块10毫秒定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
void ch_detect_10ms_timer(void)
{
    if (g_ch_detect_timer > 0) g_ch_detect_timer --;
    if (g_ch_addi_timer > 0) g_ch_addi_timer --;
    if (g_ch_addi1_timer > 0) g_ch_addi1_timer --;
    if (chargerType1Delay1 > 0) chargerType1Delay1 --;
    if (chargerType1Delay2 > 0) chargerType1Delay2 --;
}

/*==============================================================
 * 函数名称：get_ch_exist_status
 * 函数功能：判决充电器存在状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *          CH_INIT_VALUE          初始状态
 *          CH_NOEXIST_VALUE       充电器断开
 *          CH_EXIST_VALUE         充电器接入
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
uint8_t get_ch_exist_status(void)
{
    return g_ch_detect_status;
}

/*==============================================================
 * 函数名称：get_ch_exist_status
 * 函数功能：判决充电机是否接入
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *         1：接入
           0：未接入
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-11-04          liyong             创建
==============================================================*/
uint8_t get_charger_connect_status(void)
{
    if(( CH_EXIST_VALUE == g_ch_detect_status )||(E_CHARGE_STATUS == get_system_status())||(GetChHeartStatus() == 1))
        return 1;
    
    return 0;
}

/*==============================================================
 * 函数名称：set_ch_exist_status
 * 函数功能：设定充电器存在状态
 * 参数个数：1
 *           [IN]      flag               充电器状态
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
void set_ch_exist_status(uint8_t flag)
{
    if (0 == flag)
    {
        g_ch_detect_status = CH_NOEXIST_VALUE;
        g_ch_active_status = 0;
    }
    else
    {
        g_ch_detect_status = CH_EXIST_VALUE;
        g_ch_sleep_status = 0;
    }
}

/*==============================================================
 * 函数名称：set_ch_exist_status
 * 函数功能：设定充电器存在状态
 * 参数个数：1
 *           [IN]      flag               充电器状态
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
uint8_t BeginDetectCharger()
{
    uint8_t flag = 0;
    if (g_ch_detect_timer ==  0) 
    {
        g_ch_detect_timer = CH_DETECT_TIME;
        if (0 == get_ch_switch_status())
        {
           if (1 == get_ch_detect_gpio_status())
           {
               if( chargerType1Delay1 == 0 )
               {
                   flag = 2;
               }
           }
           else
           {
               chargerType1Delay1 = CHARGER_DETECT_DELAY_500MS;
           }
        }
        else
        {
           chargerType1Delay1 = CHARGER_DETECT_DELAY_500MS;
           chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
           if( get_real_current() > 10 )
           {
              flag = 1;      
           }
        }
    }
    
    return flag;
}

/*==============================================================
 * 函数名称：ChargerType1Process
 * 函数功能：类型1充电机处理流程（带电压检测型充电机，且带key接入）
 * 函数参数：无
 * 返 回 值：无
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2023-02-24          李勇               创建
==============================================================*/
void ChargerType1Process(void)
{
    uint8_t result=0;
    if( get_key_status() != E_KEY_CLOSED_STATUS )
    {
       chargerType1Status = CHARGER_KEY_CONNECT;
       chargerType1Delay1 = CHARGER_DETECT_DELAY_500MS;
       chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
    }
    switch(chargerType1Status)
    {
      case CHARGER_KEY_CONNECT:
        result = BeginDetectCharger();
        if( 2 == result )
        {
           chargerType1Status = CHARGER_CONFIRM;
           chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
        }
        else if( 1 == result )
        {
           chargerType1Status = CHARGER_CURRENT_CONTINUE;
        }
        break;
      case CHARGER_CURRENT_CONTINUE:
        if( get_real_current() < 10 )
        {
            chargerType1Status = CHARGER_KEY_CONNECT;
        } 
        else if( chargerType1Delay2 == 0 )
        {
            chargerType1Status = CHARGER_CONFIRM;
            chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
        }
        break;
      case CHARGER_CONFIRM:
        if( CURRENT_IN_DISCHARGE(get_current_status()) )
        {
           if( chargerType1Delay2 == 0 )
           {
              chargerType1Status = CHARGER_KEY_CONNECT;
           }
        }
        else
        {
            chargerType1Delay2 = CHARGER_DETECT_DELAY_30S;
        }
        break;
      default:
        chargerType1Status = CHARGER_KEY_CONNECT;
        break;
    }
}
/*=============================================================
 * 函数名称：ch_init_detect_process
 * 函数功能：检测充电机信号流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0        未完成充电机检测过程
 *          1        检测到充电机未接入
 *          2        检测到充电机接入
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2020-08-30        戴辉发      创建
==============================================================*/
static uint8_t ch_init_detect_process(void)
{
	uint8_t ret = 0;

    g_ch_init_num += 1;
	switch(g_ch_init_status)
	{
	case 0:
		if (0 == get_ch_detect_gpio_status())
		/* 首次检查到关机信号 */
		{
			g_ch_num = 1;
			g_ch_init_status = 1;
		}
        else
        {
			g_ch_num = 1;
			g_ch_init_status = 2;
        }
		break;
	case 1:
		if (0 == get_ch_detect_gpio_status())
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_ch_num += 1;
			if (g_ch_num >= CH_DETECT_OFF_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				ret = 1;
                g_ch_init_status = 0;
				g_ch_num = 0;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			g_ch_init_status = 2;
            g_ch_num = 1;
		}
		break;
    case 2:
		if (1 == get_ch_detect_gpio_status())
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_ch_num += 1;
			if (g_ch_num >= CH_DETECT_OFF_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				ret = 2;
                g_ch_init_status = 0;
				g_ch_num = 0;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			g_ch_init_status = 1;
            g_ch_num = 1;
		}
        break;
	default:
		g_ch_init_status = 0;
		break;
	}
    if ((ret == 0) && (g_ch_init_num >= (CH_DETECT_OFF_NUM + CH_DETECT_OFF_NUM / 2)))
    {
        ret = g_ch_init_status;
        g_ch_init_status = 0;
        g_ch_num = 0;
        g_ch_init_num = 0;
    }
	return ret;
}

/*=============================================================
 * 函数名称：ch_closed_detect_process
 * 函数功能：检测充电机断开流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0        未完成检测过程
 *          1        完成了检测过程
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2020-08-30        戴辉发      创建
==============================================================*/
static uint8_t ch_closed_detect_process(void)
{
	uint8_t ret = 0;

	switch(g_ch_sleep_status)
	{
	case 0:
		if (0 == get_ch_detect_gpio_status())
		/* 首次检查到关机信号 */
		{
			g_ch_num = 1;
			g_ch_sleep_status = 1;
		}
		break;
	default:
		if (0 == get_ch_detect_gpio_status())
		/* 继首次检查到关机信号后，后续再次检查到关机信号 */
		{
			g_ch_num += 1;
			if (g_ch_num >= CH_DETECT_ON_NUM)
			/* 连续检查到关机信号到10次，确认关机信号有效 */
			{
				/* 切换到检查开机信号过程 */
				g_ch_sleep_status = 0;
				g_ch_num = 0;
				ret = 1;
			}
		}
		else
		/* 在判断有效关机信号过程中，检查到开机信号，前述关机信号无效 */
		{
			g_ch_sleep_status = 0;
		}
		break;
	}
	return ret;
}

/*=============================================================
 * 函数名称：ch_sleep_detect_process
 * 函数功能：检测充电机接入流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          0        未完成检测过程
 *          1        完成检测过程
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-08-30        戴辉发      创建
==============================================================*/
static uint8_t ch_sleep_detect_process(void)
{
	uint8_t ret = 0;

	switch(g_ch_active_status)
	{
	case 0:
		if (1 == get_ch_detect_gpio_status())
		/* 检查到开机信号 */
		{
			g_ch_num = 1;
			g_ch_active_status = 1;
		}
		break;
	default:
		if (1 == get_ch_detect_gpio_status())
		/* 继首次检查到开机信号后，后续再次检查到开机信号 */
		{
			g_ch_num += 1;
			if (g_ch_num >= CH_DETECT_OFF_NUM)
			/* 连续检查到开机信号到10次，确认开机信号有效 */
			{
                g_ch_num = 0;
				g_ch_active_status = 0;
				ret = 1;
			}
		}
		else
		/* 在判断有效开机信号过程中，检查到关机信号，前述开机信号无效 */
		{
			g_ch_active_status = 0;
		}
		break;
	}
    return ret;
}

/*=============================================================
 * 函数名称：ch_detect_process
 * 函数功能：充电器检测流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2020-07-08          戴辉发             创建
==============================================================*/
void ch_detect_process(void)
{
#define MIN_CURRENT        15
#define CH_MAX_VOL         2850 /* 28.5V */
#define CH_SPIN_VOL        120 /* 相差0.3V 改为1.2V*/

    static e_key_status key_status;

    if (0 == get_ch_switch_status())
    /* 充电开关断开，检测外部盲充电压有效 */
    {
        if (g_ch_detect_timer > 0) return;
        g_ch_detect_timer = CH_DETECT_TIME;

        switch(g_ch_detect_status)
        {
        case CH_INIT_VALUE: /* 充电器初始状态 */
            {
                uint8_t init_flag = ch_init_detect_process();

                if (1 == init_flag)
                {
                    g_ch_detect_status = CH_NOEXIST_VALUE;
                    g_ch_sleep_status = 0;
					g_ch_active_status = 0;
                }
                else if (2 == init_flag)
                {
                    g_dch_circle.charge_num.conut++;
                    get_timdedata(g_dch_circle.charge_num.time);
                    needstoreflag |= E_SOC_CIRCLE_PARA_MSG;
                    g_ch_detect_status = CH_EXIST_VALUE;
                    g_ch_active_status = 0;
					g_ch_sleep_status = 0;
                }
            }
            break;
        case CH_NOEXIST_VALUE: /* 充电器未接入状态 */
            if (ch_sleep_detect_process())
            {
                g_dch_circle.charge_num.conut++;
                get_timdedata(g_dch_circle.charge_num.time);
                needstoreflag |= E_SOC_CIRCLE_PARA_MSG;
                g_ch_detect_status = CH_EXIST_VALUE;
                g_ch_active_status = 0;
				g_ch_sleep_status = 0;
            }
            break;
        default: /* 充电器接入状态 */
            {
                if ((ch_closed_detect_process()))  
                {
                    uint16_t total_vol = get_total_vol() / 10;
                    
	                if ((total_vol <= (CH_MAX_VOL - CH_SPIN_VOL))&&( 1 == get_small_current_in_sheep()))
                        /* 不允许有充电电流 充电器接入比较在压差达到一定程度以后才有效 */
	                {
                        g_ch_detect_status = CH_NOEXIST_VALUE;
                        g_ch_active_status = 0;
                        g_ch_sleep_status = 0;
	                    
	                }
	                else
	                {
	                    if (CURRENT_IN_DISCHARGE_COMMON(get_current_status()))
	                    {
	                        g_ch_detect_status = CH_NOEXIST_VALUE;
	                        g_ch_active_status = 0;
	                        g_ch_sleep_status = 0;
	                    }
	                }
                }
            }
            break;
        }
        g_ch_addi_timer = MAX_CH_STABLE_TIME;
    }
    else   
    {
        g_ch_num = 0;
        g_ch_init_num = 0;

        switch(g_ch_detect_status)
        {
		case CH_EXIST_VALUE:
			if (CURRENT_IN_DISCHARGE(get_current_status()))
			{
				g_ch_detect_status = CH_NOEXIST_VALUE;
				key_status = get_key_status();
			}
			else
			{
				int16_t current = get_real_current();
				uint16_t total_vol = get_total_vol() / 10;

				if (( 1 == get_small_current_in_sheep())&&(total_vol <= (CH_MAX_VOL - CH_SPIN_VOL))) //(0 == HeatModeKeepChargeStatus())&&
				{
                    if (0 == g_ch_addi_timer)
                    {
                        g_ch_detect_status = CH_NOEXIST_VALUE;
                    }         
				}
				else
				{
					g_ch_addi_timer = MIN_CH_STABLE_TIME;
				}
			}
			break;
		case CH_INIT_VALUE:
			g_ch_detect_status = CH_NOEXIST_VALUE;
			break;
        default:
            if (CURRENT_IN_CHARGE_COMMON(get_current_status()))
            /* 充电电流时，进入充电状态 */
            {
                g_dch_circle.charge_num.conut++;
                get_timdedata(g_dch_circle.charge_num.time);
                needstoreflag |= E_SOC_CIRCLE_PARA_MSG;
				g_ch_detect_status = CH_EXIST_VALUE;
            }
            else
            {
                g_ch_detect_status = CH_NOEXIST_VALUE;
            }
            break;
        }
        g_ch_active_status = 0;
        g_ch_sleep_status = 0;
    }
}

/*==============================================================
 * 函数名称：ChargerIsConnect
 * 函数功能：充电机连接了吗
 * 函数参数：无
 * 返 回 值：
             0：充电机已经连接
             1：充电机未连接
 * 修改记录:
 *==============================================================
 * 日期                修改人             修改内容
 * 2023-02-24          李勇               创建
==============================================================*/
uint8_t ChargerIsConnect(void)
{
    uint8_t flag = 0;
#if defined( WITH_HEAT )
    if( CHARGER_CONFIRM == chargerType1Status ) 
      flag = 1;
#else
    if( CH_EXIST_VALUE == g_ch_detect_status ) 
      flag = 1;
#endif
    
#if defined (CAN_AWAKE)
    if (get_ch_link_status() == 1)     
      flag = 1;
#endif
    if( GetChHeartStatus() == 1 )
      flag = 1;
    
    return flag;
}