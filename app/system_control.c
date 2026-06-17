/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：system_control.c
* 文件功能描述：实现电池模块系统控制
*
* 修改记录：
* 2018-06-25 戴辉发 创建
*----------------------------------------------------------*/
#include "system_control.h"
#include "ch_detect.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "vol_manage.h"
#include "short.h"
#include "parameter.h"
#include "switch_status.h"
#include "can_app.h"
#include "run_record.h"
#include "soc.h"
#include "soc_update.h"
#include "ch_detect.h"
#include "vol_curr_addi_deal.h"
#include "balance.h"
#include "fault_manage.h"
#include "ch_addition.h"
#include "afe_app.h"

static void charge_on(void);
static void discharge_on(void);

#define WAIT_CHARGER_TIME               (3)
#define TIMER_INVALID_VALUE             (0xffff)
#define TIMER_30S_TIME                  3000
#define TIMER_3S_TIME                   300

#define MAX_SHORT_COUNT                 (4)
#define TIMER_SHORT_RECOVER             (900)
#define CUTOFF_3S_DELAY_TIME             300
#define CUTOFF_2S_DELAY_TIME             200
#define CUTOFF_1S_DELAY_TIME             150

/* 子状态类型定义 */
typedef enum E_SYSTEM_CHILD_STATUS
{
	E_CHILD_IDLE_STATUS, /* 空闲状态 */
	E_CHILD_LOW_VOL_STATUS, /* 欠压保护状态 */
	E_CHILD_HIGH_VOL_STATUS, /* 过压保护状态 */
	E_CHILD_HIGH_CURR_STATUS, /* 过流保护状态 */
	E_CHILD_OVER_TEMP_STATUS, /* 温度保护状态 */
	E_CHILD_ABATE_STATUS, /* 短路保护状态 */
	E_CHILD_WAIT_STATUS, /* 充电等待通知完成指示 */
	E_CHILD_MAX_STATUS_NUM 
}e_system_child_status;

/* 模块事件类型定义 */
typedef enum E_SYSTEM_MSG_TYPE
{
	E_ENABLED_REQUEST_MSG, /* 模块使能请求 */
	E_ELECTRIC_INDICATION_MSG, /* 市电指示 */
	E_CURRENT_INDICATION_MSG, /* 电流指示 */
	E_VOLTAGE_INDICATION_MSG, /* 电压指示 */
	E_TEMPERATURE_INDICATION_MSG, /* 温度指示 */
	E_AFE_INDICATION_MSG,      /* AFE故障指示 */
	E_FINAL_INDICATION_MSG,      /* 终极保护指示 */
	E_NOTIFY_INDICATION_MSG, /* 通知完成 */
	E_MAX_MSG_NUM 
}e_system_msg_type;

static e_system_grandfather_status g_grand_status;
static e_system_status g_sys_status;
static uint8_t enter_heat;
static uint16_t heatControlDelay;
/** 最外层状态事件定义 **/
static void disabled_status_enabled_request_process(void *pmsg);
static void enabled_status_enabled_request_process(void *pmsg);
typedef void (*control_status_fun)(void *pmsg);
static void idle_status_msg_process(e_system_msg_type msg_type, void *pmsg);
static void dch_status_msg_process(e_system_msg_type msg_type, void *pmsg);
static void ch_status_msg_process(e_system_msg_type msg_type, void *pmsg);
static void abate_status_msg_process(e_system_msg_type msg_type, void *pmsg);

static uint16_t control_cutoff_delay;
static uint8_t control_cutoff_flag;
static uint16_t heat_dismoson_delay;
static uint16_t chLowTempDelay;

/*=============================================================
 * 函数名称：control_mem_init
 * 函数功能：业务控制模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_mem_init(void)
{
	g_grand_status = E_DISABLE_STATUS;
	g_sys_status = E_IDLE_STATUS;
    
    control_cutoff_flag = 0;
    control_cutoff_delay = 0;
	heat_dismoson_delay = 0;
    enter_heat = 0;
    chLowTempDelay = CUTOFF_2S_DELAY_TIME;
    heatControlDelay = TIMER_3S_TIME;
}
/*=============================================================
 * 函数名称：control_wake_mem_init
 * 函数功能：业务控制休眠唤醒内存初始化
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_wake_mem_init(void)
{
    control_cutoff_flag = 0;
    control_cutoff_delay = 0;
    heat_dismoson_delay = 0;
    enter_heat = 0;
    chLowTempDelay = CUTOFF_2S_DELAY_TIME;
    heatControlDelay = TIMER_3S_TIME;
}
/*=============================================================
 * 函数名称：control_mem_init
 * 函数功能：业务控制模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_timer_run(void)
{
	if( heatControlDelay ) heatControlDelay--;
    if(heat_dismoson_delay) heat_dismoson_delay--;
    if((E_CHARGE_STATUS == g_sys_status) && chLowTempDelay) chLowTempDelay--;
    if(control_cutoff_delay) control_cutoff_delay--;  
}


/*******************************************************************************
**FuncName: get_g_grand_status;//
**Function: 获取控制模块状态;
**Output  : 1;
**input   : 无;
**Create date : liyong @2021.8.23
**Modify  : 
*******************************************************************************/
e_system_grandfather_status get_g_grand_status(void)	
{
	return g_grand_status;
}

/*=============================================================
 * 函数名称：control_status_msg_process
 * 函数功能：业务控制模块状态事件表处理函数
 * 参数个数：4
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 *           [IN]      packed_no          电池组包号，从0开始
 *           [IN]      frame_no           电芯在组内的序号，从0开始
 *           [IN]      pmsg               消息内容
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void control_status_msg_process(e_system_msg_type msg_type, void *pmsg)
{
	if (msg_type == E_ENABLED_REQUEST_MSG)
	{/* 最外层处理 */
		if (g_grand_status == E_ENABLE_STATUS)
		{
			enabled_status_enabled_request_process(pmsg);
		}
		else if (g_grand_status == E_DISABLE_STATUS)
		{
			disabled_status_enabled_request_process(pmsg);
		}
	}
	else 
	{     
       
		if (g_grand_status == E_ENABLE_STATUS)
		{
			if (E_IDLE_STATUS == g_sys_status)
			{
				idle_status_msg_process(msg_type, pmsg);
			}
            
            if ( E_DISCHARGE_STATUS == g_sys_status)
			{
				dch_status_msg_process(msg_type, pmsg);
			}
            
		    if ( E_CHARGE_STATUS == g_sys_status)
			{
				ch_status_msg_process(msg_type, pmsg);
			}
		    
		    if (E_ABATE_STATUS == g_sys_status)
			{
				abate_status_msg_process(msg_type, pmsg);
			}
		}
	}
    
    if( get_backup_relay_fault() == 1 )
    {
        protect_code[6] |= 0x10;
    }
    else
    {
        protect_code[6] &= ~0x10;
    }
    
    if( get_backup_relay_fault() == 2 )
    {
        protect_code[6] |= 0x20;
    }
    else
    {
        protect_code[6] &= ~0x20;
    }
    
    if( get_backup_relay_in() )
    {
        protect_code[7] |= 0x40;
    }
    else
    {
        protect_code[7] &= ~0x40;
    } 
   
    
}


/** 最外层状态事件定义 **/
/***********************************************************
⒈使能请求
  ⑴使能
    ①上电内存初始化
  ⑵外层状态 = ENABLED_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：disabled_status_enabled_request_process
 * 参数个数：1
 * 函数参数：
 *           [IN]      pmsg               使能请求，0禁止，1使能
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void disabled_status_enabled_request_process(void *pmsg)
{
	uint32_t enable_flag = *((unsigned int *)pmsg);

	/* ⒈使能请求 */
	if (enable_flag == 1)
	/*   ⑴使能 */
	{
	/*     ①上电内存初始化 */
	/*   ⑵外层状态 = ENABLED_STATUS */
		g_grand_status = E_ENABLE_STATUS;
 
        if(ChargerIsConnect() == 1)         
        {
            g_sys_status = E_CHARGE_STATUS;
            
        }
        else
        {
            g_sys_status = E_IDLE_STATUS;
        }
	}
}

/***********************************************************
⒈使能请求
  ⑴禁止
    ①外层状态 = DISABLED_STATUS
***********************************************************/
/*=============================================================
 * 函数名称：enabled_status_enabled_request_process
 * 参数个数：1
 * 函数参数：
 *           [IN]      pmsg               使能请求，0禁止，1使能
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void enabled_status_enabled_request_process(void *pmsg)
{
	uint32_t enable_flag = *((uint32_t *)pmsg);
	/* ⒈使能请求 */
	if (enable_flag == 0)
	/*   ⑴禁止 */
	{
           /*     关闭相关硬件 */
		  heat_off();
          discharge_off();
          charge_off();
          /*     ①外层状态 = DISABLED_STATUS */
          g_grand_status = E_DISABLE_STATUS;
	}
}
/*=============================================================
 * 函数名称：get_fault_flag_of_bms
 * 参数个数：0
 * 函数参数：
 *           
 * 返回值：  
 *           1：表示有故障
             0：表示无故障
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
uint8_t get_fault_flag_of_bms(void)
{/* 超出终极保护范围 */
    uint8_t ret = 0;

#if 0
    if (  
        (0 != get_final_set_switch()) || 
        /* 电池压差过大 */
        (E_BAT_FAIL == get_bat_status()) || 
        /* AFE自检过程中 获取0点电平*/
        (0 == get_current_offset_poweron()) || 
        /*电流采集故障*/
        (E_C_FAULT ==  get_current_status()) || 
        //AFE通讯失败
        (1 == get_afe_status()) || 
        //AFE电压检测不一致
        (0 != get_vol_two_comp()) || 
        //AFE电压不变化
        (2 == get_vol_afe_status()) || 
        //AFE电压电流检测故障
        (0 != get_curr_afe_flag()) || 
        //放电mos失效  
        (E_SWITCH_INVALID == get_dch_switch_flag()) || 
        //充电mos失效 电流小于 3A 转入故障模式
        ((E_SWITCH_INVALID == get_ch_switch_flag()) && (g_run_sys_data.current < 30)) ||
        //充电状态互斥 
        (2 == get_status_error_flag()))
    {
        ret = 1;
    }
#endif
    if(0 == get_current_offset_poweron())
    {
        ret = 1;
    }
    /* 终极保护 */
    if (0 != get_final_set_switch())
    {
        ret = 1;
    }
    /* 电芯失效 */
    if (E_BAT_FAIL == get_bat_status())
    {
        ret = 1;
    }
   
    /* 电流采集故障 */
    if (E_C_FAULT ==  get_current_status())
    {
        ret = 1;
    }
    /* AFE通讯失败 */
    if (1 == get_afe_status())
    {
        ret = 1;
    }
    /* AFE电压检测不一致 */
    if (0 != get_vol_two_comp())
    {
        ret = 1;
    }
    /* AFE电压不变化 */
    if (2 == get_vol_afe_status())
    {
        ret = 1;
    }
    /* AFE电压电流检测故障 */
    if (0 != get_curr_afe_flag())
    {
        ret = 1;
    }
    /* 放电mos失效 */
    if (E_SWITCH_INVALID == get_dch_switch_flag())
    {
        ret = 1;
    }
    /* 充电mos失效 电流小于 3A 转入故障模式 */
    if ((E_SWITCH_INVALID == get_ch_switch_flag()) && (g_run_sys_data.current < 30))
    {
        ret = 1;
    }
    /* 充电状态互斥 */
    if ( get_status_error_flag() == 1)//
    {
        ret = 1;
    }
    
    /*预充短路*/
    if ( 0 != get_short_status() )
    {
        ret = 1;
    }
    return ret;
}
/*=============================================================
 * 函数名称：idle_status_msg_process
 * 函数功能：业务控制模块父状态为IDLE_STATUS状态事件表处理函数
 * 参数个数：2
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 *           [IN]      pmsg               消息内容
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void idle_status_msg_process(e_system_msg_type msg_type, void *pmsg)
{
    /* 状态迁移 */
    if (get_fault_flag_of_bms() != 0)
    {
        g_sys_status = E_ABATE_STATUS;
    }
    else if(1 == ChargerIsConnect())      
    {
        g_sys_status = E_CHARGE_STATUS;
        
    }
    else if(CURRENT_IN_DISCHARGE_COMMON(get_current_status()))         
    {
        g_sys_status = E_DISCHARGE_STATUS;
    }
    else
    {
        /* 开关控制 */
		heat_off(); 
        uint8_t dFlag;
        uint8_t cFlag;

        dFlag = 1;
        cFlag = 1;
        if(E_VOL_OVER == get_vol_status()) 
        {
            cFlag = 0;
        }
        if(E_C_CH_PROTECT ==  get_current_status())
        {
            cFlag = 0;
        }
        if(E_TEMP_DISDCH_STATUS == get_temperature_status())
        {
            cFlag = 0;
        }
        if(E_VOL_UNDER == get_vol_status())
        {
            cFlag = 0;
            dFlag = 0;
        }
        if ((E_C_DCH_PROTECT == get_current_status()) || (E_C_SECOND_PROTECT == get_current_status()))
        {
            cFlag = 0;
            dFlag = 0;
        }
        if(E_TEMP_DISCHDCH_STATUS == get_temperature_status())
        {
            cFlag = 0;
            dFlag = 0; 
        }

         
        /* 充电mos */
        if (0 == cFlag)
        {
            charge_off();
        }
        else
        {
            charge_on();
        }
        /* 放电mos */
        if (0 == dFlag)
        {
            discharge_off();
            control_cutoff_flag = 1;
        }
        else
        {
            discharge_on();
        }
    }
}

/*=============================================================
 * 函数名称：GetCellTempSpinFlag
 * 函数功能：加热温差大
 * 参数个数：无
 * 函数参数：
 *           
 * 返回值：  
 *           0 :电芯温度差在正常范围之内
             1 :电芯温度差不在正常范围之内
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-12-1           liy                创建
==============================================================*/
uint8_t GetCellTempSpinFlag(void)
{
    uint8_t CellTempSpinFlag = 0;
    int16_t max_temp;
    int16_t min_temp;
    int16_t spin_temp;
    
    max_temp = get_max_cell_temp();
    min_temp = get_min_cell_temp();
    spin_temp = max_temp - min_temp;
    if (spin_temp >= 200)
    {
        CellTempSpinFlag = 1;
    }
    
    return CellTempSpinFlag;
}
/*=============================================================
 * 函数名称：HeatModeControl
 * 函数功能：充电状态下，加热控制
 * 参数个数：无
 * 函数参数：
 *           
 * 返回值：  
 *           0 :不加热
             1 : 加热
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-12-1           liy                创建
==============================================================*/
static uint8_t HeatModeControl(void)
{
    uint8_t flag = heat_switch_detect();

    if( enter_heat == 0 )
    {
        int16_t minCellTemp = get_min_cell_temp();
        if(( minCellTemp > -600 )&&\
           (( E_TEMP_LOW_PROTECT == get_ch_temp_status() )||( minCellTemp < get_hot_start_temp()))&&\
           ( get_max_cell_temp() < 400 )&&\
           ( minCellTemp < get_hot_end_temp()) && (GetCellTempSpinFlag() == 0))
        {
            if( heatControlDelay == 0 )
            {
                enter_heat = 1;
                heat_dismoson_delay = TIMER_30S_TIME;
            }
        }
        else
        {
            heatControlDelay = TIMER_3S_TIME;
        }       
    }
    else 
    {
        int16_t minCellTemp = get_min_cell_temp();
        if(( minCellTemp == -600 )||( minCellTemp > get_hot_end_temp())||( get_max_cell_temp() > 400 ) || (GetCellTempSpinFlag() == 1))
        {
            if( heatControlDelay == 0 )
            {
                enter_heat = 0;
                heat_dismoson_delay = TIMER_30S_TIME;
            }
        }
        else
        {
            heatControlDelay = TIMER_3S_TIME;
        } 
    }      
   
   return (flag & enter_heat);
}
/*=============================================================
 * 函数名称：ch_status_msg_process
 * 函数功能：业务控制模块父状态为CHARGE_STATUS状态事件表处理函数
 * 参数个数：2
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 *           [IN]      pmsg               消息内容
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
修改加热模式，充电不再分加热还是不加热了
加热到了起始温度就开启，到了停止温度就关闭。
==============================================================*/
static void ch_status_msg_process(e_system_msg_type msg_type, void *pmsg)
{
    /* 状态迁移 */
    if( get_fault_flag_of_bms() != 0 )
    {
        enter_heat = 0;
        g_sys_status = E_ABATE_STATUS;
    }
    else if ( ChargerIsConnect() == 0 )       
    {
        enter_heat = 0;
        g_sys_status = E_IDLE_STATUS;
    }
    else
    {//本状态下的开关的控制
        //本状态下的开关的控制
       uint8_t flag = HeatModeControl();
	   uint8_t lowTempDisChflag = 0;
#if defined(TIANFENG) && defined(BAT_8S)
		lowTempDisChflag = getTfLowTempCurrDisCHFlag();
#endif
      
       if(( flag == 1 ))
       {
           heat_on(); 
       }
       else
       {
           heat_off();
       }
   
        /* 充电mos */
        if((E_VOL_OVER != get_vol_status()) && \
           (E_C_DCH_PROTECT != get_current_status())&&\
           (E_C_SECOND_PROTECT != get_current_status())&&\
           (E_C_CH_PROTECT !=  get_current_status()) &&\
           (1 != lowTempDisChflag))
        {
            if((E_TEMP_DISCHDCH_STATUS != get_temperature_status()) && \
               (E_TEMP_DISCH_STATUS != get_temperature_status()))
            {
                chLowTempDelay = CUTOFF_2S_DELAY_TIME;
                charge_on();
            }
            else if(E_TEMP_LOW_PROTECT == get_ch_temp_status())
            {
                if(chLowTempDelay != 0)
                {
                    charge_on();
                }
                else
                {
                    charge_off();
                }
            }
            else
            {
                chLowTempDelay = CUTOFF_2S_DELAY_TIME;
                charge_off();
            }
        }
        else
        {
            chLowTempDelay = CUTOFF_2S_DELAY_TIME;
            charge_off();
        }
       
        if(  ( E_TEMP_DISCHDCH_STATUS != get_temperature_status() )&&\
            ( E_C_DCH_PROTECT != get_current_status() )&&\
            ( E_C_SECOND_PROTECT != get_current_status() ))
       {
            if(( flag == 1 )&&( heat_dismoson_delay == 0 )&&( E_TEMP_LOW_PROTECT == get_ch_temp_status() )) 
            {/*在温度禁止充电的时候，处于加热状态，前30s已经运行完*/
                discharge_off();
            } 
            else
            {
                discharge_on();
            }        
       }
       else
       {
            discharge_off();
       }
        
    }
}

/*=============================================================
 * 函数名称：control_dch_status_msg_process
 * 函数功能：业务控制模块父状态为DISCHARGE_STATUS状态事件表处理函数
 * 参数个数：2
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 *           [IN]      pmsg               消息内容
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void dch_status_msg_process(e_system_msg_type msg_type, void *pmsg)
{	  
    /* 状态迁移 */
    if( get_fault_flag_of_bms() != 0 )
    {
        g_sys_status = E_ABATE_STATUS;
    }
    else if (ChargerIsConnect() == 1)       
    {
        g_sys_status = E_CHARGE_STATUS;
    }
    else if ( 0 == get_cur_current() )         
    {
        g_sys_status = E_IDLE_STATUS;
    }
    else 
    {
        /* 开关控制 */
		heat_off(); 
        /* 充电mos */
        
        /* 放电mos */
        
        if ((E_VOL_UNDER != get_vol_status()) && \
            (E_TEMP_DISCHDCH_STATUS != get_temperature_status()) && \
            (E_TEMP_DISDCH_STATUS != get_temperature_status()) && \
            (E_C_DCH_PROTECT !=  get_current_status()) && \
            (E_C_SECOND_PROTECT !=  get_current_status()))
        {
            discharge_on(); 
            charge_on();
        }
        else
        {
            if(control_cutoff_flag == 0)
            {
                control_cutoff_flag = 1;
                control_cutoff_delay = 0;//CUTOFF_1S_DELAY_TIME; /* 注释掉、取消切断延时 */
            }

            if(control_cutoff_delay == 0)
            {
                control_cutoff_flag = 2;
                discharge_off();
                charge_off();
            }
        }
    }
}
/*=============================================================
 * 函数名称：abate_status_msg_process
 * 函数功能：业务控制模块父状态为ABATE_STATUS状态事件表处理函数
 * 参数个数：2
 * 函数参数：
 *           [IN]      msg_type           输入的消息事件
 *           [IN]      pmsg               消息内容
 * 返回值：  
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void abate_status_msg_process(e_system_msg_type msg_type, void *pmsg)
{
    if( get_fault_flag_of_bms() == 0 )
    {

        if ( ChargerIsConnect() == 1 )       
        {
            g_sys_status = E_CHARGE_STATUS;
        }
        else  
        {
            g_sys_status = E_IDLE_STATUS;
        }
    }
    else
    {//本状态下的开关的控制
         heat_off(); 
         discharge_off();
         charge_off();
         control_cutoff_delay = 0;
         control_cutoff_flag = 2;
    }
}
/*=============================================================
 * 函数名称：control_enable_request
 * 参数个数：1
 * 函数参数：
 *           [IN]      enabled_flag       模块使能标识，0禁止,1激活
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_enable_request(unsigned int enabled_flag)
{
	control_status_msg_process(E_ENABLED_REQUEST_MSG, (void *)(&enabled_flag));
}

/*=============================================================
 * 函数名称：control_source_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_source_indication(void)
{
	control_status_msg_process(E_ELECTRIC_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_current_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_current_indication(void)
{
	control_status_msg_process(E_CURRENT_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_voltage_indication
 * 参数个数：0
 * 函数参数：
 * 
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_voltage_indication(void)
{
	control_status_msg_process(E_VOLTAGE_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_temp_indication
 * 参数个数：0
 * 函数参数：
 *           
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_temp_indication(void)
{
	control_status_msg_process(E_TEMPERATURE_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_notify_indication
 * 参数个数：0
 * 函数参数：
 *           
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_notify_indication(void)
{
	control_status_msg_process(E_NOTIFY_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_AFE_indication
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_AFE_indication(void)
{
	control_status_msg_process(E_AFE_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：control_final_indication
 * 参数个数：0
 * 函数参数：
 *           
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void control_final_indication(void)
{
	control_status_msg_process(E_FINAL_INDICATION_MSG, (void *)0);
}

/*=============================================================
 * 函数名称：get_system_status
 * 参数功能：获取当前系统状态
 * 参数个数：0
 * 函数参数：
 *           
 * 返 回 值：
 *           返回系统状态
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-28          戴辉发             创建
==============================================================*/
e_system_status get_system_status(void)
{
	return g_sys_status;
}

/*=============================================================
 * 函数名称：control_charge_switch_on
 * 函数功能：控制模块充电开关合上，充电模式下使用
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void charge_on(void)
{
    ch_connect_request();
}

/*=============================================================
 * 函数名称：charge_off
 * 函数功能：控制模块充电开关断开
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void charge_off(void)
{
    ch_disconnect_request();
}

/*=============================================================
 * 函数名称：discharge_on
 * 函数功能：控制模块放电开关合上
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
static void discharge_on(void)
{
    control_cutoff_flag = 0;
    dch_connect_request();
}

/*=============================================================
 * 函数名称：discharge_off
 * 函数功能：控制模块放电开关断开
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void discharge_off(void)
{
    dch_disconnect_request();
}

/*=============================================================
 * 函数名称：get_switch_msg
 * 函数功能：获取开关量事件
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           返回开关量事件代码
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
uint8_t get_switch_msg(void)
{
	uint8_t ret = 0;

	if (get_dch_switch_flag())
	{
		ret |= 0x01;
	}
	if (get_ch_switch_flag())
	{
		ret |= 0x02;
	}

	return ret;
}

/*=============================================================
 * 函数名称：get_system_code
 * 函数功能：获取系统代码
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           返回系统代码
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
uint8_t get_system_code(void)
{
	uint8_t ret = 0;

	if (g_grand_status == E_ENABLE_STATUS)
	{
		if ( g_sys_status == E_DISCHARGE_STATUS )
		{
			ret |= 0x01;
		}
		if ( g_sys_status == E_CHARGE_STATUS )
		{
			ret |= 0x02;
		}
		if ( g_sys_status == E_IDLE_STATUS )
		{
			ret |= 0x10;
		}
	}

	return ret;
}

/*=============================================================
 * 函数名称：get_cutoff_flag
 * 函数功能：获取是否需要关闭放电，用于设置充电标志;
 * 参数描述：无
 * 返 回 值：是否需要关闭放电
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2021-03-30          liyong             创建
==============================================================*/
uint8_t get_cutoff_flag(void)
{
    if( control_cutoff_flag == 1 )
      return 0x01;
    
    return 0x0;
}
