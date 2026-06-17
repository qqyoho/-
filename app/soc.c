#include <math.h>
#include "parameter.h"
#include "soc.h"
#include "vol_manage.h"
#include "string.h"
#include "storage_manage.h"
#include "system_control.h"
#include "temp_manage.h"
#include "vol_manage.h"
#include "current_manage.h"
#include "system_adjust.h"
#include "soc_update.h"
#include "switch_status.h"
#include "peak_record.h"
#include "vol_curr_addi_deal.h"
#include "ch_addition.h"
#include "fault_manage.h"

#define MAX_SOC_CURR_NUM            (10)
#define AH_RECORD                    RATE_CAPCITY //额定容量记录一次
#define SECOND_CURRENT_ADC_NUM       60        /* 最快20ms采集一次电流*/
#define CAPCITY_COMPENSATE_VALUE    (-2)        /* 自身功耗，按照20ma计算 单位10毫安秒/bit */
#define CARLOAD_CURRENT_LOSS        (-20)        /* 单位10毫安秒/bit */
#define DCH_SOC_NUM                 (4)        /* 放电补偿计算次数 */
#define SOC_SECOND_ALARM_DELAY       (5)
#define SOC_THREE_PROTECT_DELAY      (3)
#define SOC_SECOND_RECOVER_DELAY     (5)
#define SOC_THREE_RECOVER_DELAY      (3)

#if defined(BF24_PEU1_S2H)
#define   BACKUP_RELAY              (180)
#define   MAIN_DISCHARGER_RELAY     (180)
#elif defined(TIANFENG)
#define   BACKUP_RELAY              (170)
#endif


/* 温度与可放容量对应关系 */
typedef struct _T_TEMPERATURE_CAPCITY_STRUCT_ 
{
	int16_t temperature; /* 温度 */
	uint16_t temp_ratio; /* 温度系数 */
}t_temp_capcity_st;
uint8_t Soc_adjust_flag = 0;
static e_soc_flag_type g_soc_flag; /* 容量标识 */
static volatile uint16_t g_soc_alarm_delay;
static volatile uint16_t g_soc_protect_delay;
static uint8_t g_soc_alarm_wait;
static uint8_t g_soc_protect_wait;
t_dch_circle_st g_dch_circle; /* 记录充放电循环 */
static volatile uint8_t g_soc_tail;
static volatile uint8_t g_soc_count;
int16_t g_current_adc[SECOND_CURRENT_ADC_NUM]; /* 存储当前1秒钟之内采样的ADC值，总计100次 */
int16_t g_test_current;
uint8_t g_test_current_valid;

static uint32_t dch_capcity;

static uint8_t g_soc_adjust_flag; /* SOC校准标志 */

static void soc_alarm_process(void);

static void soc_adjust_addition(int16_t current);

static volatile uint8_t carLoadRecivedFlag;

static uint8_t fullSocFlag;

/*=============================================================
 * 函数名称：soc_manage_mem_init
 * 函数功能：soc管理模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
void soc_manage_mem_init(void)
{
	g_soc_tail = 0;
	g_soc_count = 0;
	memset(&g_dch_circle, 0, sizeof(g_dch_circle));
    dch_capcity = 0;
    g_soc_adjust_flag = 0;
    Soc_adjust_flag = 0;
    carLoadRecivedFlag = 0;
    g_test_current = 0;
    g_test_current_valid = 0;
    fullSocFlag = 0;
    g_soc_flag = E_COMMON_SOC;
    g_soc_alarm_delay = 0;
    g_soc_protect_delay = 0;
    g_soc_alarm_wait = 0;
    g_soc_protect_wait = 0;
    soc_update_mem_init();
}

/*=============================================================
 * 函数名称：soc_manage_wakeup_mem_init
 * 函数功能：soc管理模块唤醒内存初始化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-21       	戴辉发     	创建
==============================================================*/
void soc_manage_wakeup_mem_init(void)
{
	g_soc_tail = 0;
	g_soc_count = 0;
    carLoadRecivedFlag = 0;
    g_soc_alarm_delay = 0;
    g_soc_protect_delay = 0;
    g_soc_alarm_wait = 0;
    g_soc_protect_wait = 0;
    g_test_current = 0;
    g_test_current_valid = 0;
}
/*=============================================================
 * 函数名称：SetCarLoadRecivedFlag
 * 函数功能：收到整车指令，就设置为1
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
void SetCarLoadRecivedFlag(uint8_t flag)
{
    carLoadRecivedFlag = flag;
}
/*=============================================================
 * 函数名称：GetCarLoadRecivedFlag
 * 函数功能：获取 carLoadRecivedFlag 标志
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
uint8_t GetCarLoadRecivedFlag(void)
{
    return carLoadRecivedFlag;
}     
/*=============================================================
 * 函数名称：GetFullCapacity
 * 函数功能：获取当前的满电容量
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
按照循环800次
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
uint32_t GetFullCapacity(void)
{
    uint32_t defaultCapacity = get_rated_capcity();
    
    defaultCapacity *= AH_VALUE;
    
    if ( E_CHARGE_STATUS == get_system_status() )
    {
        float fullCapacity = defaultCapacity;

        if( g_dch_circle.circle_num.conut >= MAXIMUM_CYCLES )
        {
             fullCapacity = fullCapacity*0.8;
        }
        else
        {
             fullCapacity -= 0.2*g_dch_circle.circle_num.conut/MAXIMUM_CYCLES*fullCapacity;
        }
        
        g_dch_circle.total = (uint32_t)(fullCapacity+0.5);
    }
    
    if( g_dch_circle.total > defaultCapacity )
    {
        g_dch_circle.total = defaultCapacity;
    }
    return  g_dch_circle.total;
}
/*=============================================================
 * 函数名称：GetDischargeFullCapacity
 * 函数功能：获取放电循环一次的总容量
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
uint32_t GetDischargeFullCapacity(void)
{
    float dischargeCapacity = get_rated_capcity();
    
    dischargeCapacity = dischargeCapacity * AH_VALUE * 0.8;
    
    return (uint32_t)(dischargeCapacity+0.5); 
}
/*=============================================================
 * 函数名称：GetRelaySwitchOnCurrent
 * 函数功能：获取 继电器闭合 损耗电流
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
1. 主继电器闭合
2.二次执行继电器闭合
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
int32_t GetRelaySwitchOnCurrent(void)
{
    int32_t relayCurrent = 0;
    
    if(get_system_status() != E_CHARGE_STATUS)
    {
#if defined(TIANFENG) && defined(BAT_8S)
        if((get_backup_relay_in() == 1) && (GetBackupRelayStatus() == 1))
        {
            relayCurrent += g_run_sys_data.total_vol/BACKUP_RELAY;
        }
#endif
    }
    return relayCurrent;
}
/*=============================================================
 * 函数名称：GetSocAhCurrent
 * 函数功能：获取 SOC 计算电流
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
1. 高于门限值的电流，则直接
2. 门限值以内的电流
   
 *==============================================================
 * 日    期          修改人      修改类型
 * 2022-03-31        liyong     	创建
==============================================================*/
int32_t GetSocAhCurrent( float current )
{
    int32_t socCurrent = 0;
    int32_t powerLoss = CAPCITY_COMPENSATE_VALUE;
     /*0.1a 转换为0.01a*/
    current = (10*current); 
    /*电流四舍五入*/
    if( current < 0 )
        current -= 0.5;
    else
        current += 0.5; 
      
    if(( current < 10*get_dch_current_in_level() )||( current > 10*get_ch_current_in_level() ))
    {/*门限值以上的值*/
        socCurrent = (int32_t)current;
    }
    else 
    {/*门限值以内的充放电电流值*/
        
        if( E_CHARGE_STATUS == get_system_status() )
        {/*充电状态的充电小电流*/
            if(( 1== get_ch_switch_status() )&&( current > 0 ))
            {
                socCurrent = (int32_t)current;
            }
            else
            {
                socCurrent = 0;
            }
        }
        else 
        {/*门限值以内的放电电流值*/       
            if( carLoadRecivedFlag == 1 )
            {/*待机电流未检测到*/
                if( current > CARLOAD_CURRENT_LOSS + powerLoss  )
                {/*实际放电电流 < 车辆待机电流，取车辆待机电流*/
                    socCurrent = (int32_t)( CARLOAD_CURRENT_LOSS + powerLoss );
                }
                else if( current < 0 )
                {/*实际电流 > 车辆待机电流，取实际电流*/
                    socCurrent = (int32_t)current;
                }         
            }     
            else
            {/*只有自身功耗了*/
                socCurrent = (int32_t)(powerLoss);
            }
        }    
        
    }
    
         
    return socCurrent;
}
/*=============================================================
 * 函数名称：get_residue_capcity
 * 函数功能：获取当前剩余容量
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           当前剩余容量，单位AH
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
uint16_t get_residue_capcity(void)
{
	uint16_t residue;

	residue = (uint16_t)(g_dch_circle.residue / AH_VALUE);

	return residue;
}

/*=============================================================
 * 函数名称：set_soc_count
 * 函数功能：每秒钟设置一次
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
void set_soc_count(void)
{
	g_soc_count = g_soc_tail;
	g_soc_tail = 0;
}

/*=============================================================
 * 函数名称：set_soc_current_ad_value
 * 函数功能：设置电流AD值
 * 参数个数：1
 * 参数描述：
 *           [IN]    ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
void set_soc_current_ad_value(int16_t ad_value)
{
    g_current_adc[g_soc_tail ++] = ad_value;
    if (g_soc_tail >= SECOND_CURRENT_ADC_NUM)
    {
        g_soc_tail = 0;
    }
}

/*=============================================================
 * 函数名称：soc_manage_init
 * 函数功能：soc管理模块内存初始化
 *           本函数调用必须在paramerter硬件初始化完成后调用
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void soc_manage_init(void)
{
    /* SOC更新模块硬件初始化 */
    soc_update_hard_init();  
#if defined (HUAFU) && defined (BATTARY_LFP)
    uint64_t temp= g_dch_circle.total;
    /* 电池额定电量, 单位10毫安秒 */
    g_run_sys_data.soc = 95*MAX_SOC_VALUE/100;
    temp = temp * g_run_sys_data.soc / MAX_SOC_VALUE;
    g_dch_circle.residue = (uint32_t)temp;
    g_soc_adjust_flag = 1;
#endif
}

/*=============================================================
 * 函数名称：update_cur_soc
 * 函数功能：静态电池容量校准
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-09-14        戴辉发     	创建
==============================================================*/
void update_cur_soc(void)
{
	uint16_t cur_soc_percent;
	float temp;   
    /* 更新当前容量 */
    soc_update_capcity_config();

	/* 最小值 */
	cur_soc_percent = soc_static_adjust(get_min_cell_vol(), 0);

	g_run_sys_data.soc = cur_soc_percent;

	/* 电池额定电量, 单位10毫安秒 */
    temp = get_rated_capcity();
    temp *= AH_VALUE; /* 换算为10毫安S */  
	temp = temp * g_run_sys_data.soc / MAX_SOC_VALUE;
	g_dch_circle.residue = (uint32_t)temp;
}

/*=============================================================
 * 函数名称：default_capacity_adjust
 * 函数功能：缺省静态电池容量校准
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-12-9         戴辉发      创建
==============================================================*/
void default_capacity_adjust(void)
{
    float temp;
	long rated_capcity;

	/* 电池额定电量, 单位10毫安秒 */
	rated_capcity = get_rated_capcity();
	rated_capcity *= AH_VALUE; /* 换算为10毫安S */
	g_dch_circle.total = rated_capcity;

    g_run_sys_data.soc = soc_static_adjust(get_min_cell_vol(), 0);

    temp = rated_capcity;
    temp = temp * g_run_sys_data.soc / MAX_SOC_VALUE;
    g_dch_circle.residue = (uint32_t)temp;
}

/*=============================================================
 * 函数名称：s_timer_s_run
 * 函数功能：SOC定时器1S
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	 创建
==============================================================*/
void s_timer_s_run(void)
{
    if (g_soc_alarm_delay) g_soc_alarm_delay--;
    if (g_soc_protect_delay) g_soc_protect_delay--;
}

/*=============================================================
 * 函数名称：soc_alarm_process
 * 函数功能：SOC容量告警处理流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-20        戴辉发      创建
==============================================================*/
static void soc_alarm_process(void)
{
	e_system_status bms_state;
	e_soc_flag_type soc_flag;
    uint16_t soc;

    soc = (uint16_t)((uint32_t)g_run_sys_data.soc * 1000 / MAX_SOC_VALUE);
    soc_flag = g_soc_flag;
	bms_state = get_system_status();

	if (bms_state == E_CHARGE_STATUS)
    {
        soc_flag = E_COMMON_SOC;
        g_soc_alarm_wait = 0;
        g_soc_protect_wait = 0;
        g_soc_alarm_delay = 0;
        g_soc_protect_delay = 0;
    }
    else if (soc < 50)
    {
        g_soc_alarm_wait = 0;
        g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
        if (soc_flag != E_PROTECT_SOC)
        {
            if (0 == g_soc_protect_wait)
            {
                g_soc_protect_wait = 1;
                g_soc_protect_delay = SOC_THREE_PROTECT_DELAY;
            }
            else if (0 == g_soc_protect_delay)
            {
                soc_flag = E_PROTECT_SOC;
                g_soc_protect_wait = 0;
                g_soc_protect_delay = SOC_THREE_RECOVER_DELAY;
            }
        }
        else
        {
            g_soc_protect_delay = SOC_THREE_RECOVER_DELAY;
        }
    }
    else if (soc_flag == E_PROTECT_SOC)
    {
        g_soc_protect_wait = 0;
        if (soc > 200)
        {
            if (0 == g_soc_protect_delay)
            {
                if (soc > 250)
                {
                    soc_flag = E_COMMON_SOC;
                }
                else
                {
                    soc_flag = E_LOW_SOC;
                    g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
                }
            }
        }
        else
        {
            g_soc_protect_delay = SOC_THREE_RECOVER_DELAY;
        }
    }
    else if (soc < 100)
    {
        g_soc_protect_wait = 0;
        g_soc_protect_delay = SOC_THREE_PROTECT_DELAY;
        if (soc_flag != E_LOW_SOC)
        {
            if (0 == g_soc_alarm_wait)
            {
                g_soc_alarm_wait = 1;
                g_soc_alarm_delay = SOC_SECOND_ALARM_DELAY;
            }
            else if (0 == g_soc_alarm_delay)
            {
                soc_flag = E_LOW_SOC;
                g_soc_alarm_wait = 0;
                g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
            }
        }
        else
        {
            g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
        }
    }
    else if (soc_flag == E_LOW_SOC)
    {
        g_soc_alarm_wait = 0;
        g_soc_protect_wait = 0;
        g_soc_protect_delay = SOC_THREE_PROTECT_DELAY;
        if (soc > 250)
        {
            if (0 == g_soc_alarm_delay)
            {
                soc_flag = E_COMMON_SOC;
            }
        }
        else
        {
            g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
        }
    }
    else if (soc < 200)
    {
        g_soc_protect_wait = 0;
        g_soc_protect_delay = SOC_THREE_PROTECT_DELAY;
        if (soc_flag != E_SUB_LOW_SOC)
        {
            if (0 == g_soc_alarm_wait)
            {
                g_soc_alarm_wait = 1;
                g_soc_alarm_delay = SOC_SECOND_ALARM_DELAY;
            }
            else if (0 == g_soc_alarm_delay)
            {
                soc_flag = E_SUB_LOW_SOC;
                g_soc_alarm_wait = 0;
                g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
            }
        }
        else
        {
            g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
        }
    }
    else if (soc_flag == E_SUB_LOW_SOC)
    {
        g_soc_alarm_wait = 0;
        g_soc_protect_wait = 0;
        g_soc_protect_delay = SOC_THREE_PROTECT_DELAY;
        if (soc > 250)
        {
            if (0 == g_soc_alarm_delay)
            {
                soc_flag = E_COMMON_SOC;
            }
        }
        else
        {
            g_soc_alarm_delay = SOC_SECOND_RECOVER_DELAY;
        }
    }
    else 
    {
        soc_flag = E_COMMON_SOC;
        g_soc_alarm_wait = 0;
        g_soc_protect_wait = 0;
    }

	if( soc_flag == E_PROTECT_SOC )
	{
		protect_code[3] |= 0x01;
	}
	else
	{
		protect_code[3] &= ~0x01;
	}

	g_soc_flag = soc_flag;
}

/*=============================================================
 * 函数名称：get_soc_status
 * 函数功能：获取剩余电量状态
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           当前剩余电量状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-28        戴辉发      创建
==============================================================*/
e_soc_flag_type get_soc_status(void)
{
	return g_soc_flag;
}

/*=============================================================
 * 函数名称：get_soc_value
 * 函数功能：获取SOC
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           当前剩余电量状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-15        戴辉发      创建
==============================================================*/
uint8_t get_soc_value(void)
{
	return (uint8_t)g_run_sys_data.soc;
}

/*=============================================================
 * 函数名称：soc_vol_update
 * 函数功能：soc过压欠压更新容量
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-02-29        戴辉发     	创建
==============================================================*/
static void soc_vol_update(void)
{
	/* 过压欠压更新容量 */
	if ( E_CHARGE_STATUS == get_system_status() )
    /* 充电只判过压保护 */
	{
        switch(Soc_adjust_flag)
        {
        case 1:
            if ( E_VOL_OVER == get_vol_status() )
            /* 过压保护时需要校准当前剩余容量 */
            {
                Soc_adjust_flag = 2;
                force_set_soc(100, 1);
                /* 启动SOC计算偏差 */
                soc_update_dch_capcity_statrt();
#if defined(TIANFENG) && defined(BAT_8S)
                SetFullSocFlag(2); /* 过压也是满充条件，设置SOC可置百标志 */
#endif
            }
            break;
        case 2:
            break;
        default:
            Soc_adjust_flag = 1;
            break;
        }
	}
	else
    /* 其他判决欠压保护 */
	{
        switch( Soc_adjust_flag )
        {
        case 3:
            if ( E_VOL_UNDER == get_vol_status() )
            /* 欠压保护时需要校准当前剩余容量 */
            {
                Soc_adjust_flag = 4;
                force_set_soc(0, 0);
                /* 容量更新结束标识 */
                soc_update_dch_capcity_end();
#if defined(TIANFENG) && defined(BAT_8S)
                SetFullSocFlag(1);  /* 设置SOC可置零标志 */
#endif
            }
            break;
        case 4:
            break;
        default:
            Soc_adjust_flag = 3;
            break;
        }
	}
}

/*=============================================================
 * 函数名称：soc_manage
 * 函数功能：soc管理流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void soc_manage(void)
{
    uint8_t soc_count = g_soc_count;     
	if (soc_count > 0)
	{
	  	float total_adc;
	  	int32_t residue;
		uint16_t i;
	  	uint32_t temp;
	  	int32_t work_current;
		uint32_t rated_capcity;
		float f_temp;
		g_soc_count = 0;
        
		/* 计算上一秒内电流平均值 */
		/* 第一步：计算平均ADC值 */
		total_adc = 0;
		for ( i = 0; i < soc_count; i ++ )
        {
			total_adc += g_current_adc[i];
		}
        total_adc /= soc_count;
        if( total_adc < 0 ) total_adc -= 0.5;
        /*soc误差*/	
        if (g_test_current_valid)
        {
            vol_curr_soc_update(g_test_current);
        }
        else
        {
		    vol_curr_soc_update(get_work_current((int16_t)total_adc));
        }
        
        
	  	/* 计算当前剩余电量, 单位为10mAS */
        if (g_test_current_valid)
        {
            work_current = GetSocAhCurrent(g_test_current);
        }
        else
        {
		    work_current = GetSocAhCurrent(total_adc);
        }
        if( work_current < 0 )
          g_run_sys_data.current = (work_current-5)/10;
        else
          g_run_sys_data.current = (work_current)/10;  
        if (g_test_current_valid)
        {
            g_run_sys_data.current = g_test_current;
        }
        
        /*继电器版本增加继电器损耗*/
        work_current -= GetRelaySwitchOnCurrent();
         /* 显示电流值 */
       // g_run_sys_data.current = work_current/10; 
            
        /*获取满电容量*/
        rated_capcity = GetFullCapacity();
	    residue = g_dch_circle.residue;
       
        if (work_current <= 0)
        {/* 电流为负 */ 
#if defined(TIANFENG) && defined(BAT_8S)
            if(GetFullSocFlag() == 2 && (work_current < 0))
            {
                SetFullSocFlag(0);/* SOC置百后，若有放电电流且无过压保护，则进入正常SOC计算 */
            }
#endif
            work_current = -work_current;
            
            if (residue < work_current )
            /* 剩余容量不足以抵消本次放电电量 */
            {
                residue = 0;
            }
            else
            /* 剩余容量足以抵消本次放电电量 */
            {
                residue -= work_current;
            }
            
            /* 额定容量更新 */
            soc_update_dch_capcity( work_current );

            dch_capcity += ( work_current );
            
            /*获取计算一次满电循环的容量*/            
            if ( dch_capcity >= GetDischargeFullCapacity() )
            {
                dch_capcity -= GetDischargeFullCapacity();
                //放电累计次数
                g_dch_circle.circle_num.conut++;
                get_timdedata(g_dch_circle.circle_num.time);
                //设置存储标志
                needstoreflag |= E_SOC_CIRCLE_PARA_MSG;
            }
        }
        else  
        {/* 电流为正 */
#if defined(TIANFENG) && defined(BAT_8S)
            if(GetFullSocFlag() == 1)
            {
                SetFullSocFlag(0);/* SOC置0后，若有充电电流且无欠压保护，则进入正常SOC计算 */
            }
#endif
            residue += work_current;  
            if ( residue >= rated_capcity ) 
              residue = rated_capcity;
        }

	  	g_dch_circle.residue = residue;

		/* 计算本次SOC值 */
		f_temp = 1.0 * MAX_SOC_VALUE * residue / rated_capcity; 	
		temp = (uint16_t)f_temp;
		f_temp -= temp;
        
		if ((f_temp * 10) >= 5.0)
		{
			temp += 1;
		}
#if defined(TIANFENG) && defined(BAT_8S)
	  	if ((g_run_sys_data.soc < MAX_SOC_VALUE) && (temp >= ((MAX_SOC_VALUE * 99 + 99) / 100))) /* 未达到置百条件前，不能置百 */
        {
            if(2 == GetFullSocFlag())
            {
                temp = MAX_SOC_VALUE;
            }
            else
            {
                temp = ((MAX_SOC_VALUE * 99 + 99) / 100);
            }
	  	}
        if(temp < ((MAX_SOC_VALUE + 99) / 100))   /* 未达到置零条件前，不能低于1% */
        {
            if(1 == GetFullSocFlag())
            {
                temp = 0;
            }
            else
            {
                temp = ((MAX_SOC_VALUE + 99) / 100);
            }
        }
#else
        if (temp >= MAX_SOC_VALUE)
        {
            temp = MAX_SOC_VALUE;
	  	}
#endif
	  	g_run_sys_data.soc = temp;
	}

	soc_alarm_process();
}


/*=============================================================
 * 函数名称：soc_adjust_addition
 * 函数功能：SOC纠正过程
 * 参数个数：1
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-08-02        戴辉发      创建
==============================================================*/
static void soc_adjust_addition(int16_t current)
{
#if !defined (BATTARY_LFP)
  	if ((get_system_status() == E_CHARGE_STATUS) && (judge_vol_sample_finished()))
  	{
		uint16_t total_vol = get_total_vol() / 10;

		if (E_VOL_OVER != get_vol_status())
		{
			uint16_t middle_vol = (get_vol_high_alarm_level() + get_vol_high_protect()) / 2;

			if ( total_vol > middle_vol )
			{
                if ( current > 10 )
                {
                    if (g_run_sys_data.soc < (uint16_t)(MAX_SOC_VALUE * 0.95))
                    {
                        long rated_capcity;
                        /* 电池额定电量, 单位10毫安秒 */
                        rated_capcity = get_rated_capcity();
                        rated_capcity *= AH_VALUE; /* 换算为10毫安S */
                        
                        g_run_sys_data.soc = (uint16_t)(MAX_SOC_VALUE * 0.95);

                        g_dch_circle.residue = (uint32_t)(0.95 * rated_capcity);
                    }
                }
				else if ( (current <= 10 ) && (current >= -10) )
				{
					force_set_soc(100, 1);
                    /* 启动更新 */
                    soc_update_dch_capcity_statrt();
				}
			}
		}
    }
#endif
}

/*=============================================================
 * 函数名称：get_total_capcity
 * 函数功能：获取当前总容量
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：当前总容量，单位10mA/S
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-10-15        戴辉发     	 创建
==============================================================*/
uint32_t get_total_capcity(void)
{
    return g_dch_circle.total;
}

/*=============================================================
 * 函数名称：set_total_capcity
 * 函数功能：设置当前总容量
 * 参数个数：1
 * 参数描述：
 *           [IN]    total       容量值，单位10mA/S
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-10-15        戴辉发     	 创建
==============================================================*/
void set_total_capcity(uint32_t total)
{
    g_dch_circle.total = total;
}

/*=============================================================
* 函数名称：set_soc_adjust_flag
* 函数功能：设置SOC校准标志
* 参数个数：0
* 参数描述：
* 返 回 值：
*           返回SOC上一秒电流
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-10-16       	戴辉发     	创建
==============================================================*/
void set_soc_adjust_flag(void)
{
    g_soc_adjust_flag = 2;
}

/*=============================================================
 * 函数名称：bat_capacity_adjust
 * 函数功能：静态电池容量校准
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发      创建
==============================================================*/
void bat_capacity_adjust(void)
{
    /* 充电附加模块流程(专用于三元电池) */
    soc_adjust_addition(g_run_sys_data.current);
    /* 端点校准 */
    soc_vol_update();
    /* 长时间未工作校准 */
	if ((g_soc_adjust_flag == 0) || (g_soc_adjust_flag == 2))
	{
		if ((judge_vol_sample_finished()) && (E_ABATE_STATUS != get_system_status()))
		{
			float temp;
			uint16_t cur_soc_percent;
            long rated_capcity;
             /* 电池额定电量, 单位10毫安秒 */
            rated_capcity = get_rated_capcity();
            rated_capcity *= AH_VALUE; /* 换算为10毫安S */
			/* 最小值 */
			cur_soc_percent = soc_static_adjust(get_average_vol(), 0);

            if (g_soc_adjust_flag == 0)
            {
                
                g_run_sys_data.soc = cur_soc_percent;
               
                temp = rated_capcity;
                temp = temp * g_run_sys_data.soc / MAX_SOC_VALUE;
                g_dch_circle.residue = (uint32_t)temp;
            }
            else
            {
                if (g_run_sys_data.soc > cur_soc_percent)
                {
                    g_run_sys_data.soc = cur_soc_percent;

                    temp = rated_capcity;
                    temp = temp * g_run_sys_data.soc / MAX_SOC_VALUE;
                    g_dch_circle.residue = (uint32_t)temp;
                }
            }
            g_soc_adjust_flag = 1;
		}
	}
}

/*=============================================================
 * 函数名称：force_set_soc
 * 函数功能：强制设定SOC值，需要根据温度调整当前分母部分，主要指总容量
 * 参数个数：2
 * 参数描述：
 *          [IN]     soc         将要设定的 SOC值，
 *          [IN]     flag        向上向下判决，
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-01-18        戴辉发       创建
==============================================================*/
void force_set_soc(uint16_t soc, uint8_t flag)
{
	float temp = (MAX_SOC_VALUE / 100.00) * soc;
    long rated_capcity;
     /* 电池额定电量, 单位10毫安秒 */
    rated_capcity = get_rated_capcity();
    rated_capcity *= AH_VALUE; /* 换算为10毫安S */
	if (flag)
	{
		if (g_run_sys_data.soc < (uint16_t)temp)
		{
			g_run_sys_data.soc = (uint16_t)temp;
			g_dch_circle.residue = (int32_t)(soc / 100.0 * rated_capcity);
		}
	}
	else
	{
		if (g_run_sys_data.soc > (uint16_t)temp)
		{
			g_run_sys_data.soc = (uint16_t)temp;
			g_dch_circle.residue = (int32_t)(soc / 100.0 * rated_capcity);
		}
	}
}
#if defined(TIANFENG) && defined(BAT_8S)
/*=============================================================
 * 函数名称：SetFullSocFlag
 * 函数功能：设置SOC置百标志
 * 参数描述：1: SOC可置零
             0：正常更新SOC
             2：SOC可置百
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2024-03-11        Fanyl       创建
==============================================================*/
void SetFullSocFlag(uint8_t ret)
{
    fullSocFlag = ret;
}

/*=============================================================
 * 函数名称：GetFullSocFlag
 * 函数功能：设置SOC置百标志
 * 参数描述：无
 * 返 回 值：1: SOC可置零
             0：正常更新SOC
             2：SOC可置百
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2024-03-11        Fanyl       创建
==============================================================*/
uint8_t GetFullSocFlag(void)
{
    uint8_t ret;
    ret = fullSocFlag;
    return ret;
}
#endif
