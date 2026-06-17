#include "fm33lg0xx_fl.h"
#include "timer.h"
#include "parameter.h"
#include "system_control.h"
#include "current_manage.h"
#include "balance.h"
#include "low_power.h"
#include "afe_app.h"
#include "short.h"
#include "soc.h"
#include "temp_manage.h"
#include "vol_manage.h"
#include "ch_detect.h"
#include "storage_manage.h"
#include "can_app.h"
#include "can.h"
#include "key_status.h"
#include "can_open_timer.h"
#include "power.h"
#include "clear_swd.h"
#include "idog.h"
#include "fault_manage.h"
#include "led_show.h"
#include "adc_sampling.h"
#include "final_protect.h"
#include "mode_manage.h"
#include "switch_status.h"
#include "peak_record.h"
#include "vol_curr_addi_deal.h"
#include "ch_addition.h"
#include "bluetooth.h"
#include "run_record.h"
#include "Appcommunication.h"
#include "ChHeart.h"
#include "chargerProtocol.h"
#if defined (BATTARY_LFP)
#include "ch_addition.h"
#endif

uint32_t test_cr;
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 10mS IRQ
**FuncName: ATIM_Init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void ATIM_Init(uint16_t arr, uint16_t psc)	//(99, 4799@10KHz)
{
    FL_ATIM_InitTypeDef        InitStructer;

    InitStructer.clockSource           = FL_CMU_ATIM_CLK_SOURCE_APBCLK;  //时钟源选择APB2
    InitStructer.prescaler             = psc;                                         //分频系数
    InitStructer.counterMode           = FL_ATIM_COUNTER_DIR_UP;                    //向上计数
    InitStructer.autoReload            = arr;                                          //自动重装载值
    InitStructer.clockDivision         = FL_ATIM_CLK_DIVISION_DIV1;                   //死区和滤波设置
    InitStructer.repetitionCounter     = 0;                                            //重复计数
    InitStructer.autoReloadState       = FL_DISABLE;                                      //自动重装载禁止preload
    FL_ATIM_Init(ATIM, &InitStructer);

    NVIC_DisableIRQ(ATIM_IRQn);
    NVIC_SetPriority(ATIM_IRQn, 1); //中断优先级配置
    NVIC_EnableIRQ(ATIM_IRQn);

    FL_ATIM_ClearFlag_Update(ATIM);  //清除计数器中断标志位
    FL_ATIM_EnableIT_Update(ATIM); //开启计数器中断

    FL_ATIM_Enable(ATIM); //使能定时器
}

#define TIMER_10MS_TIME            2
#define TIMER_10_TIMES             10
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: ATIM_IRQHandler()
**Create  : ly
**Modify  : 
*******************************************************************************/
void ATIM_IRQHandler(void)
{
	static uint16_t count_10 = 0;
	static uint16_t count_100 = 0;
	static uint16_t count_1000 = 0;

    if(FL_ATIM_IsEnabledIT_Update(ATIM) && FL_ATIM_IsActiveFlag_Update(ATIM))
	{
		/* clear update interrupt bit */
        FL_ATIM_ClearFlag_Update(ATIM);

		c_timer_ms_run();
        adc_sample_timer();
        afe_app_timer();
        fault_manage_timer_ms_run();
        switch_status_ms_process();
        can_app_timer_s_timer();
		count_10 += 1;
		if (count_10 >= TIMER_10MS_TIME)
        /* 10MS */
		{
			count_10 = 0;
 			b_timer_ms_run();
            control_timer_run();
			c_open_10ms_timer_run();
            power_timer_ms_run();
            ch_detect_10ms_timer();
            ChHeartTimer();
            ChargerMessageDelay();
            key_timer_ms_run();
            final_protect_timer();
            mode_manage_timer();
            count_100 += 1;
            if (count_100 >= TIMER_10_TIMES)
            {/* 100MS */
                count_100 = 0;
                led_show_timer_100ms_run();
                VoltageProtectTimerDelay();
                t_timer_run();
                AppCommonTimer();
#if defined (BATTARY_LFP)
                ch_addition_timer();
#endif
#if defined( BLUETOOTH ) 
                BluetoothDelayProcess();
#endif 
                count_1000 += 1;
                if (0 == get_harddwg_flag())
                {
                    feed_hard_owdg();
                }
                if (count_1000 >= TIMER_10_TIMES)
               /* 1S */
               {
                    count_1000 = 0;
                      
                    v_timer_s_run();
                    s_timer_s_run();
                    low_m_timer_process();
                    time_s_storage();
                    write_run_record_delay();
                   
                    test_cr++;
               }
           }
		}
	}
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 100mS IRQ
**FuncName: ATIM_Init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void GpTimer0_init(uint16_t pre,uint16_t reload )
{
   FL_GPTIM_InitTypeDef gptimer_typedef;
   FL_GPTIM_DeInit(GPTIM0);
   FL_GPTIM_StructInit(&gptimer_typedef); 
   
   gptimer_typedef.prescaler         = pre;
   gptimer_typedef.autoReloadState   = FL_ENABLE;
   gptimer_typedef.counterMode       = FL_GPTIM_COUNTER_DIR_UP;
   gptimer_typedef.autoReload        = reload;
   gptimer_typedef.clockDivision     = FL_GPTIM_CLK_DIVISION_DIV1;
   
   FL_GPTIM_Init(GPTIM0, &gptimer_typedef);
   
   NVIC_DisableIRQ(GPTIM01_IRQn);
   NVIC_SetPriority(GPTIM01_IRQn, 2); //中断优先级配置
   NVIC_EnableIRQ(GPTIM01_IRQn);

   FL_GPTIM_ClearFlag_Update(GPTIM0);  //清除计数器中断标志位
   FL_GPTIM_EnableIT_Update(GPTIM0); //开启计数器中断

   FL_GPTIM_Enable(GPTIM0); //使能定时器
   
}
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: GPTIM0_1_IRQHandler()
**Create  : ly
**Modify  : 
*******************************************************************************/
void GPTIM0_1_IRQHandler(void)
{
	if( FL_GPTIM_IsEnabledIT_Update(GPTIM0) && FL_GPTIM_IsActiveFlag_Update(GPTIM0) )
	{
		/* clear update interrupt bit */
        FL_GPTIM_ClearFlag_Update(GPTIM0);
		set_soc_count();       
	}
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : LPTIM16初始化  500ms唤醒一次，喂硬件狗
**FuncName: LPTIM16_wakeup_init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void LPTIM16_wakeup_init(void)
{
    FL_LPTIM16_InitTypeDef      timInit;

    /*---------------- 定时器时间基准配置 ----------------*/
    FL_LPTIM16_StructInit(&timInit);

    timInit.clockSource          = FL_CMU_LPTIM16_CLK_SOURCE_LSCLK;
    timInit.mode                 = FL_LPTIM16_OPERATION_MODE_NORMAL;
    timInit.prescalerClockSource = FL_LPTIM16_CLK_SOURCE_INTERNAL;
    timInit.prescaler            = FL_LPTIM16_PSC_DIV1;
    timInit.autoReload           = 32768 / 2 - 1;  //32768hz 500ms
    timInit.encoderMode          = FL_LPTIM16_ENCODER_MODE_DISABLE;
    timInit.onePulseMode         = FL_LPTIM16_ONE_PULSE_MODE_CONTINUOUS;
    timInit.triggerEdge          = FL_LPTIM16_ETR_TRIGGER_EDGE_RISING;
    timInit.countEdge            = FL_LPTIM16_ETR_COUNT_EDGE_RISING;
    FL_LPTIM16_Init(LPTIM16, &timInit);
    /*---------------- 中断配置 ----------------*/
    /* 清除标志 */
    FL_LPTIM16_ClearFlag_Update(LPTIM16);
    /* 中断使能 */
    FL_LPTIM16_EnableIT_Update(LPTIM16);
    /* 使能并配置NVIC */
    NVIC_DisableIRQ(LPTIMx_IRQn);
    NVIC_SetPriority(LPTIMx_IRQn, 2); //设置中断优先级
    NVIC_EnableIRQ(LPTIMx_IRQn);
    /*---------------------------------------------*/
    /* 使能LPTIM16 */
    FL_LPTIM16_Enable(LPTIM16);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : LPTIM16关闭  
**FuncName: LPTIM16_close_init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void LPTIM16_close_init(void)
{
    /* 清除标志 */
    FL_LPTIM16_ClearFlag_Update(LPTIM16);
    NVIC_DisableIRQ(LPTIMx_IRQn);
    FL_LPTIM16_Disable(LPTIM16);
    FL_LPTIM16_DeInit(LPTIM16);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : LPTIM16 中断服务函数  500ms唤醒一次，喂硬件狗
**FuncName: LPTIM_IRQHandler();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void LPTIM_IRQHandler(void)
{
    //定时器溢出时翻转LED0
    if(FL_LPTIM16_IsActiveFlag_Update(LPTIM16))
    {
        feed_hard_owdg();
        FL_LPTIM16_ClearFlag_Update(LPTIM16);
    }
}
