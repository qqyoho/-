/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：power.c
* 文件功能描述：实现控制板电源管理
*
* 修改记录：
* 2018-06-27 戴辉发 创建
*----------------------------------------------------------*/
#include "power.h"
#include "parameter.h"
#include "hardware.h"
#include "ch_detect.h"
#include "key_status.h"
#include "rtc.h"
#include "idog.h"
#include "soc.h"
#include "low_power.h"
#include "fault_manage.h"
#include "can.h"
#include "vol_manage.h"
#include "mode_manage.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "temp_manage.h"
#include "balance.h"
#include "switch_status.h"
#include "soc.h"
#include "soc_update.h"
#include "vol_curr_addi_deal.h"
#include "short.h"
#include "protect_record.h"
#include "led_show.h"
#include "ch_addition.h"
#include "fm33lg0xx_fl.h"
#include "afe_app.h"
#include "timer.h"
#include "storage_manage.h"

#define RTC_WAKE_TIME            5  /* 500毫秒 */
#define ONE_SECOND               (10 / RTC_WAKE_TIME)
#define ON_DELAY_MS              (20*8/48)
#define ON_CONTINUE_NUM          600
#define ON_VALID_NUM             500
#define VCC3V3_POWERDOWN         2995

#define MAINPOWER_DETECT_GPIO_PIN    FL_GPIO_PIN_1
#define MAINPOWER_DETECT_GPIO        GPIOA
#define MAINPOWER_CONTROL_GPIO_PIN   FL_GPIO_PIN_3
#define MAINPOWER_CONTROL_GPIO       GPIOD

static volatile uint16_t main_power_timer = 5;
static volatile uint8_t charge_wake_flag;
static volatile uint8_t key_check_wake_flag;
static volatile uint8_t charger_check_wake_flag;
static uint32_t g_sleep_time;

static uint8_t g_main_power_status;
static uint8_t g_main_power_active_status;
static uint8_t g_main_power_deactive_status;

/*=============================================================
 * 函数名称：power_hard_init
 * 函数功能：电源硬件初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-09       	戴辉发     	创建
==============================================================*/
void power_hard_init(void)
{
    main_power_on();
}

/*=============================================================
 * 函数名称：main_power_on
 * 函数功能：电源上电
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-09       	戴辉发     	创建
==============================================================*/
void main_power_on(void)
{
	uint8_t delay = 0;
   
    do
    {
        FL_GPIO_ResetOutputPin( MAINPOWER_CONTROL_GPIO, MAINPOWER_CONTROL_GPIO_PIN);
        //delay 5ms to wait for the power to be stable
        delay_ms(5);
        delay ++;
        if (delay > 10) break;
    }while ( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) == 0 );

    if( delay > 10 )
    {
      set_power_on_fault();
      g_main_power_status = 0;
    }
    else
    {
      g_main_power_status = 1;
    }  
    
      g_main_power_active_status = 0;
      g_main_power_deactive_status = 0;
}

/*=============================================================
 * 函数名称：main_power_off
 * 函数功能：电源掉电
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-09       	戴辉发     	创建
==============================================================*/
void main_power_off(void)
{
    uint8_t delay = 0;
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
     
    FL_GPIO_StructInit(&GPIO_InitStruct);
    FL_GPIO_ResetOutputPin( MAINPOWER_CONTROL_GPIO, MAINPOWER_CONTROL_GPIO_PIN);    
    GPIO_InitStruct.pin = MAINPOWER_CONTROL_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    FL_GPIO_Init(MAINPOWER_CONTROL_GPIO, &GPIO_InitStruct);
    
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = MAINPOWER_DETECT_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;  
    FL_GPIO_Init(MAINPOWER_DETECT_GPIO, &GPIO_InitStruct); 
    
    
    do
    {
        FL_GPIO_SetOutputPin( MAINPOWER_CONTROL_GPIO, MAINPOWER_CONTROL_GPIO_PIN);
        //delay 5ms to wait for the power to be stable
        delay_ms(5);
        delay ++;
        if (delay > 10) break;
    }while ( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) != 0 );

    if (delay > 10)
    {
        set_power_off_fault();     
    }
    else
    {
        g_main_power_status = 0;
    }

    g_main_power_active_status = 0;
    g_main_power_deactive_status = 0;
    
}

/*=============================================================
 * 函数名称：main_power_active_detect_process
 * 函数功能：检测主电源无效状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       未完成主电源失效过程
 *           1       完成了主电源失效过程
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-12       	戴辉发     	创建
==============================================================*/
static uint8_t main_power_active_detect_process(void)
{
	static uint8_t tmp_num = 0;
	uint8_t ret = 0;
  
	switch(g_main_power_active_status)
	{
	case 0:
		if (( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) == 0 )||( get_vcc3v3_vaule() < VCC3V3_POWERDOWN ))
		/* 首次检查到MAIN_POWER无效信号 */
		{
			tmp_num = 1;
			g_main_power_active_status = 1;
		}
		break;
	case 1:
		if (( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) == 0 )||( get_vcc3v3_vaule() < VCC3V3_POWERDOWN ))
		/* 继首次检查到MAIN_POWER无效信号后，后续再次检查到MAIN_POWER无效信号 */
		{
			tmp_num += 1;
			if (tmp_num >= 40)
			/* 主电源失效确认 */
			{
				/* 主电源失效 */
				g_main_power_active_status = 2;
				tmp_num = 0;
			}
		}
		else
		/* 主电源失效无效 */
		{
			g_main_power_active_status = 0;
		}
		break;
	default:
		ret = 1;
		break;
	}
	return ret;
}
/*=============================================================
 * 函数名称：get_main_power_detect_status
 * 函数功能：获取主电源有效状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       主电源失效过程
 *           1       主电源正常过程
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-06-1       	李勇     	创建
==============================================================*/
 uint8_t get_main_power_detect_status(void)
{
    return g_main_power_status;
}
/*=============================================================
 * 函数名称：main_power_deactive_detect_process
 * 函数功能：检测主电源有效状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       未完成主电源失效过程
 *           1       完成了主电源失效过程
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-12       	戴辉发     	创建
==============================================================*/
static uint8_t main_power_deactive_detect_process(void)
{
	static uint8_t tmp_num = 0;
	uint8_t ret = 0;

	switch(g_main_power_deactive_status)
	{
	case 0:
		if (( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) != 0  )&&( get_vcc3v3_vaule() > VCC3V3_POWERDOWN+5 ))
		/* 首次检查到MAIN_POWER有效信号 */
		{
			tmp_num = 1;
			g_main_power_deactive_status = 1;
		}
		break;
	case 1:
		if (( FL_GPIO_GetInputPin( MAINPOWER_DETECT_GPIO, MAINPOWER_DETECT_GPIO_PIN ) != 0  )&&( get_vcc3v3_vaule() > VCC3V3_POWERDOWN+5 ))
		/* 继首次检查到MAIN_POWER有效信号后，后续再次检查到MAIN_POWER有效信号 */
		{
			tmp_num += 1;
			if (tmp_num >= 10)
			/* 主电源有效确认 */
			{
				/* 主电源有效 */
				g_main_power_deactive_status = 2;
				tmp_num = 0;
			}
		}
		else
		/* 主电源有效无效 */
		{
			g_main_power_deactive_status = 0;
		}
		break;
	default:
		ret = 1;
		break;
	}
	return ret;
}

/*=============================================================
 * 函数名称：main_power_detect_process
 * 函数功能：主电源模块检测流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-12       	戴辉发     	创建
==============================================================*/
void main_power_detect_process(void)
{
	if (main_power_timer > 0) return;
	main_power_timer = 5;

	if (1 == g_main_power_status)
	/* 主电源有效情况下检测主电源有效 */
	{
		if ( main_power_active_detect_process() )
		{
            set_power_on_fault();
            g_main_power_status = 0;
            g_main_power_deactive_status = 0;
		}
	}
	else
	{
		if ( main_power_deactive_detect_process() )
		{
			g_main_power_status = 1;
			g_main_power_active_status = 0;
		}
	}
    if ((0 == g_main_power_status))
    {
        mode_power_invalid_indication();
    }
    
	if ( 0 == g_main_power_status )
	{
		protect_code[7] |= 0x02;
	}
	else
	{
		protect_code[7] &= ~0x02;
	} 
}

/*=============================================================
 * 函数名称：power_timer_ms_run
 * 函数功能：功率模块定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void power_timer_ms_run(void)
{
	if (main_power_timer) main_power_timer --;
}

/*=============================================================
 * 函数名称：exti_wake_config
 * 函数功能：外部唤醒配置
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-11-01          戴辉发             创建
==============================================================*/
static void exti_wake_config(void)
{
    FL_EXTI_CommonInitTypeDef EXTI_CommonInitStruct;
    FL_EXTI_InitTypeDef EXTI_InitStruct;
   
    //休眠使能外部中断采样
    FL_CMU_EnableEXTIOnSleep();
    FL_CMU_EnableGroup3OperationClock(FL_CMU_GROUP3_OPCLK_EXTI);
  
    EXTI_CommonInitStruct.clockSource = FL_CMU_EXTI_CLK_SOURCE_LSCLK;
    FL_EXTI_CommonInit( &EXTI_CommonInitStruct );

    /*c key pb6*/
    EXTI_InitStruct.filter = FL_ENABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP2;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_5,&EXTI_InitStruct);
    
    /*can awake pb13*/
    EXTI_InitStruct.filter = FL_ENABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP1;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_7,&EXTI_InitStruct);
    /*key pa5*/
    EXTI_InitStruct.filter = FL_ENABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP1;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_BOTH;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_1,&EXTI_InitStruct);
    
    
    
    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(GPIO_IRQn);
     
}

/*=============================================================
 * 函数名称：exti_close_config
 * 函数功能：外部唤醒关闭配置
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-11-01          戴辉发             创建
==============================================================*/
static void exti_close_config(void)
{
    FL_EXTI_DeInit(FL_GPIO_EXTI_LINE_5);
    FL_EXTI_DeInit(FL_GPIO_EXTI_LINE_1);   
    FL_EXTI_DeInit(FL_GPIO_EXTI_LINE_7);   
    FL_EXTI_CommonDeinit();
}
/*=============================================================
 * 函数名称：GPIO_IRQHandler
 * 函数功能：开关 exti检测脚中断
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-12        戴辉发     	创建
==============================================================*/
void GPIO_IRQHandler(void)
{

    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_5))
    {
        //PB6~ C K Switch wakeup  
        charge_wake_flag = 1;
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_5);
    }
    
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_7))
    {
        //PB6~ C K Switch wakeup  
        charge_wake_flag = 1;
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_7);
    }
    
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_1))
    {
        //PA5~Key Switch wakeup   
        key_check_wake_flag  = 1;
        FL_GPIO_ClearFlag_EXTI(GPIO,FL_GPIO_EXTI_LINE_1);
    } 
}
/*=============================================================
 * 函数名称：bms_enter_sheep
 * 函数功能：mcu进入休眠前配置
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2020-09-08         李勇     	创建
==============================================================*/
void bms_enter_sheep(void)
{
    /* 关闭均衡 */
    cell_balance_off(0);
    //AFE get into <Power Save> mode  
    
    if( 0 == afe_app_sleep())
    {/*AFE下电失败*/
        protect_code[7] |= 0x01;
        set_power_off_fault();
    }
    else
    {
        protect_code[7] &= ~0x01;
    }
    __disable_irq();
    
    FL_ATIM_DeInit(ATIM);
    FL_GPTIM_DeInit(GPTIM0);
    FL_I2C_DeInit(I2C);
    FL_ADC_DeInit(ADC);
    FL_ADC_CommonDeInit();
    FL_DMA_DeInit(DMA);
    FL_CAN_DeInit(CAN);
    main_power_off();
    gpio_sleep_init();
    
    __enable_irq();
}

void delay_sleep_ms(uint16_t delay)
{
  uint16_t i,j;
  for(j=delay;j>0;j--)
   for(i=0;i<50;i++);
}
void DeepSleep(void)
{
    FL_PMU_SleepInitTypeDef lpmInitStruct;

    //  FL_CMU_RCLF_Enable();               // 暂开启RCLF
    //FL_CMU_RCLP_Disable();              // 休眠下关闭RCLP
    FL_RMU_PDR_Enable(RMU);             // 打开PDR
    FL_RMU_BOR_Disable(RMU);            // 关闭BOR 2uA
    lpmInitStruct.deepSleep = FL_PMU_SLEEP_MODE_DEEP;
    lpmInitStruct.powerMode = FL_PMU_POWER_MODE_SLEEP_OR_DEEPSLEEP;
    lpmInitStruct.wakeupFrequency = FL_PMU_RCHF_WAKEUP_FREQ_8MHZ;
    lpmInitStruct.wakeupDelay = FL_PMU_WAKEUP_DELAY_2US;
    lpmInitStruct.coreVoltageScaling = FL_DISABLE;
    FL_PMU_Sleep_Init(PMU, &lpmInitStruct);
}
/*=============================================================
* 函数名称：get_charge_wake_up_flag
* 函数功能：获取充电器连接唤醒标志
* 参数个数：0
* 参数描述：
* 返 回 值：
*           充电器接入唤醒标志
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
uint8_t get_charge_wake_up_flag(void)
{
	return charge_wake_flag;
}

uint8_t awake_detect_proc()
{
  uint8_t ret = 0;
  uint32_t chCount;
  
  /* 赋初值 */
  uint32_t temp = ON_CONTINUE_NUM;
  /* 延时20毫秒 */
  delay_sleep_ms(ON_DELAY_MS);
  /* 判决高低电平次数 */
  while (temp --)
  {
    if (1 == getChConnectGpioStatus())
    {
      chCount += 1;
    }
    
    if (chCount < ON_VALID_NUM)
    {
      set_charge_wake_up_flag(0);
    }
    else if (chCount >= ON_VALID_NUM)
    {
      set_charge_wake_up_flag(1);
      set_ch_exist_status(1);
      ret = 1;
      break;
    }
  }
  return ret;
}
/*=============================================================
 * 函数名称：main_power_off
 * 函数功能：电源掉电
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-13       	戴辉发     	创建
==============================================================*/
void low_power_stop_mode_config(void)
{
    uint8_t key_is_open = 0;
    uint8_t charger_is_open = 0;

	bms_enter_sheep();  
	//--------------------------------------------------------------------------
	//<5>.Config RTC wakeup Timer for feeding IWDG
    g_sleep_time = 0;
    feed_iwdg();
	//rtc_wake_up_config();
    LPTIM16_wakeup_init();
	/* Get into STOP mode */
    if (get_key_gpio_status())
    {
        key_is_open = 1; 
    }
    else
    {
        key_is_open = 0;  
    }
    
    if (get_ch_detect_gpio_status())
    {
        charger_is_open = 1;
    } 
    else
    {
        charger_is_open = 0;
    }
    /* 读取开关状态，判断开关是否切断 */
	while(1)
	{
        set_key_check_wake_flag(0);
        set_charge_wake_up_flag(0);
        charger_check_wake_flag = 0;
		exti_wake_config();		
        //进入深度休眠
        DeepSleep();       
		/* 关闭下降沿触发配置 */
		exti_close_config();

		/* 计时器 */
        if ( g_sleep_time < 0xFFFFFF00 )
        {
            g_sleep_time += 1;
        }
		feed_iwdg();
        
        if( g_sleep_time > 5 )
        {
            FL_GPIO_DeInit(GPIOC,  FL_GPIO_PIN_7);
            FL_GPIO_DeInit(GPIOB,  FL_GPIO_PIN_8);
        }
        
        if( charger_check_wake_flag )
        {
           break;
        }
        if (get_key_check_wake_flag() != 0)
        {
			if ((key_is_open == 0) && (get_key_gpio_status() != 0))
			/* 上升沿退出 */
			{
				uint16_t temp = ON_CONTINUE_NUM;
				uint16_t key_link_count = 0;

				/* 延时20毫秒 */
				delay_sleep_ms(ON_DELAY_MS);
				while (temp --)
				{
					if (get_key_gpio_status())
					{
						key_link_count += 1;
					}
					if (key_link_count >= ON_VALID_NUM)
					{
						set_key_on_status();
						break;
					}
				}

				if ((key_link_count >= ON_VALID_NUM))
				{
					break;
				}
			}
			else if ((key_is_open == 1) && (get_key_gpio_status() == 0))
			/* 先判下降沿 */
			{
				uint16_t temp = ON_CONTINUE_NUM;
				uint16_t key_link_count = 0;

				/* 延时20毫秒 */
				delay_sleep_ms(ON_DELAY_MS);
				while (temp --)
				{       
					if (get_key_gpio_status() == 0)
					{
						key_link_count += 1;
					}
					if (key_link_count >= ON_VALID_NUM)
					{    
                        set_key_on_status();   
						break;
					}
				}

				if ((key_link_count >= ON_VALID_NUM))
				{
					key_is_open = 0;
				}
			}
        }
        
        if(get_charge_wake_up_flag() != 0)
        {
            if(awake_detect_proc() == 1)
              break;
        }
        else 
        {
            set_charge_wake_up_flag(0);
        }
    }

    clear_can_send_buff();
    exti_close_config();
	//rtc_wake_up_close_config();
    LPTIM16_close_init();
    if (g_sleep_time > (24L * (3600L * ONE_SECOND)))
    /* 关机时间超过24小时，需要重新校准SOC */
    {
#if !defined (BATTARY_LFP)
        set_soc_adjust_flag();
#endif
        if (g_sleep_time > (240L * (3600L * ONE_SECOND)))
        /* 关机时间超过240小时，需要重新校准SOC */
        {
            soc_update_dch_capcity_stop();
        }
    }
    /*g_sleep_time = 5184000*12;*/
    /*计算损耗容量，根据静态功耗*/
    if( g_sleep_time > ONE_SECOND )
    {
        float capacityLoss = g_sleep_time/ONE_SECOND;
        /*按照200ua计算 mas*/
        capacityLoss = capacityLoss * 200/1000; 
        /*转换成soc的 10mas/bit */
        capacityLoss /= 10;
        /*soc衰减*/
        if( g_dch_circle.residue > capacityLoss )
            g_dch_circle.residue -= capacityLoss;
        else
            g_dch_circle.residue = 0;

		/* 计算本次SOC值  soc循环里面去计算*/        
    }
    UpdatePowerOnCount(g_sleep_time/ONE_SECOND);
    UpdateFactroyModeDelay(g_sleep_time/ONE_SECOND);
}

/*=============================================================
 * 函数名称：mode_into_low_power_mode
 * 函数功能：进入低功耗模式
 * 参数个数：1
 * 函数参数：
 *           [IN]      timer_count        延时毫秒数进入休眠
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void mode_into_low_power_mode(uint16_t timer_count)
{
    relay_off();
    delay_ms(timer_count);
    set_key_off_status();
    set_ch_exist_status(0);
    /* 判定是否满充，满充后强制设定当前最高单体为单体过压保护 */
    if(1 == getChFullFlag())
    {
        set_max_cell_over_voltage();
    }
    /* 进入休眠 */
	low_power_stop_mode_config();
    /* 退出休眠 */
#if defined (BATTARY_LFP)
    /* 充电附加模块 */
    ch_addition_mem_init();
#endif
    /* SOC模块内存初始化 */
    soc_manage_wakeup_mem_init();
    /* 电流处理模块内存初始化 */
    current_mem_init();
    /* switch 状态*/
    wakeup_init_switch_status();
    /* 采样模块内存初始化 */
    adc_mem_init();
    /* 电流电压辅助判断 */
    vol_curr_addi_wakeup_mem_init();
    /* 低功耗初始化 */
    init_low_power_status();
    /* 二次保护模块*/
    fault_manage_mem_init();
    vol_wakeup_init();
    /* 模式管理 */
    mode_manage_mem_init();
    /* 硬件初始化 */
    hardware_wake_init();
    /*短路故障解除*/
    short_mem_init();
    ch_detect_wakeup_mem_init();
}



/*=============================================================
* 函数名称：set_charge_wake_up_flag
* 函数功能：设定充电器连接唤醒标志
* 参数个数：1
* 参数描述：
*           [IN]    wake_flag   唤醒标志
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void set_charge_wake_up_flag(uint8_t wake_flag)
{
	charge_wake_flag = wake_flag;
}

/*=============================================================
* 函数名称：get_key_check_wake_flag
* 函数功能：获取钥匙连接口唤醒标志
* 参数个数：0
* 参数描述：
* 返 回 值：
*           钥匙连接口唤醒标志
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
uint8_t get_key_check_wake_flag(void)
{
	return key_check_wake_flag;
}

/*=============================================================
* 函数名称：set_key_check_wake_flag
* 函数功能：设定钥匙连接口唤醒标志
* 参数个数：0
* 参数描述：
* 返 回 值：
*           钥匙连接口唤醒标志
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void set_key_check_wake_flag(uint8_t wake_flag)
{
	key_check_wake_flag = wake_flag;
}
