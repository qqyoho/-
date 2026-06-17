#include <string.h>
#include "balance.h"
#include "afe_app.h"
#include "vol_manage.h"
#include "parameter.h"
#include "system_control.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "storage_manage.h"
#include "run_record.h"
#include "ch_detect.h"
#include "ch_addition.h"

#define MAX_FAIL_TIME                     (1000)
#define MAX_BALANCE_HALT_TIMER            (1)
#define MAX_BALANCE_TIMER                 (50)
#define DELAY_BALANCE_STATUS              (500)

static volatile uint16_t time_fail;
static volatile uint16_t time_balance;
static volatile uint16_t time_status_balance;
static e_balance_status g_balance_status;
static e_battary_status g_bat_status;
uint8_t g_balance_index;
static uint8_t g_blance_main_flag;

/*=============================================================
 * 函数名称：balance_mem_init
 * 函数功能：均衡模块内存初始化
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
==============================================================*/
void balance_mem_init(void)
{
	g_balance_status = E_BALANCE_IDLE;
	g_bat_status = E_BAT_COMMON;
   
	g_balance_index = 0;
    time_fail = MAX_FAIL_TIME;
    time_balance = MAX_BALANCE_TIMER;
    time_status_balance = DELAY_BALANCE_STATUS/5;
    g_blance_main_flag = 0;
}

/*=============================================================
 * 函数名称：balance_record_default
 * 函数功能：均衡模块缺省值
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
static void balance_record_default(void)
{
    uint8_t i;
    for(i = 0; i < 16; i++)
    {
        cell_blance_numb.cellblance[i].conut = 0;
    }
}

/*=============================================================
 * 函数名称：balance_no_mem_init
 * 函数功能：均衡模块计数值初始化
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
==============================================================*/
void balance_no_mem_init(void)
{
	balance_record_default();
    g_blance_main_flag = 0;
}

/*=============================================================
 * 函数名称：balance_record_hard_init
 * 函数功能：均衡模块计数值初始化
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03        liyong       创建
==============================================================*/
void balance_record_hard_init(void)
{
    int16_t flag = 0;
    t_blance_time balance_record;

    /* 获取极限值1 */
    flag = read_data_from_storage(BLANCE_SECTOR_MAIN, (uint8_t *)&cell_blance_numb, sizeof(t_blance_time));
    if(flag == sizeof(t_blance_time))
    {   
        flag = read_data_from_storage(BLANCE_SECTOR_BACK, (uint8_t *)&balance_record, sizeof(t_blance_time));
        if(flag == sizeof(t_blance_time))
        {
            flag = judge_new_sector(cell_blance_numb.number, balance_record.number);
        }
    }

	if (flag == -1)
	{
		g_blance_main_flag = 1;
		memcpy(&cell_blance_numb, &balance_record, sizeof(t_blance_time));
	}
	else if (flag == 0)
	{
		g_blance_main_flag = 1;
		balance_record_default();
		cell_blance_numb.number = MAX_STORAGE_NUM;
	}
	else
	{
		g_blance_main_flag = 0;
	} 
}

/*=============================================================
 * 函数名称：cell_balance_count
 * 函数功能：保存均衡记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03                  liyong         创建
==============================================================*/
void cell_balance_count(uint8_t no)
{
   cell_blance_numb.cellblance[no].conut++;
   get_timdedata(cell_blance_numb.cellblance[no].time);
}

/*=============================================================
 * 函数名称：recordbalance_data
 * 函数功能：保存均衡记录
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-06-03                  liyong         创建
==============================================================*/
void recordbalance_data(void)
{
    uint16_t address;
    t_write_data_st data;

    cell_blance_numb.number = sector_num_next(cell_blance_numb.number);
    if (g_blance_main_flag)
    {
        g_blance_main_flag = 0;
        address = BLANCE_SECTOR_MAIN;
    }
    else
    {
        g_blance_main_flag = 1;
        address = BLANCE_SECTOR_BACK;
    }

    data.msg_type = E_BLANCE_RECORD_MSG;
    data.write_len = sizeof(cell_blance_numb);
    data.write_memery_address = (uint8_t *)&cell_blance_numb;
    data.write_storge_address = address;
    set_write_data_to_queue(&data);
}

/*=============================================================
 * 函数名称：reset_blance_num
 * 函数功能：恢复均衡次数
 * 参数个数：0
 * 参数描述：
 *
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期                     修改人         修改类型
 * 2020-10-13                  liyong         创建
==============================================================*/
void reset_blance_num(void)
{
    balance_no_mem_init();
    recordbalance_data();
}

/*=============================================================
 * 函数名称：judge_balance_status
 * 函数功能：判决均衡状态
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *          1       处于均衡状态
 *          0       不处于均衡状态
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
 * 2021-11-02       戴辉发     修改了判决依据，直接使用g_balance_status
 *                             删除g_blance_status_flag 这个变量作为依据
==============================================================*/
uint8_t judge_balance_status(void)
{
    if (E_BALANCE_ON == g_balance_status)
    {
        return 1;
    }
    return 0;
}

/*=============================================================
 * 函数名称：get_bat_status
 * 函数功能：获取电芯失效状态
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           当前电芯状态
 * 修改记录:
 *=============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
==============================================================*/
e_battary_status get_bat_status(void)
{
	return g_bat_status;
}

/*=============================================================
 * 函数名称：b_timer_ms_run
 * 函数功能：均衡模块定时器
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
==============================================================*/
void b_timer_ms_run(void)
{
	if (time_balance) time_balance --;
    if (time_fail) time_fail --;
    if (time_status_balance) time_status_balance--;
}

/*=============================================================
 * 函数名称：balance_close
 * 函数功能：关闭均衡模块
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2019-08-09       戴辉发     创建
==============================================================*/
void balance_close(void)
{
    cell_balance_off(g_balance_index);
    g_balance_status = E_BALANCE_IDLE;
    g_balance_index = 0;
}

/*=============================================================
 * 函数名称：balance_start
 * 函数功能：启动均衡模块
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2019-08-09       戴辉发     创建
==============================================================*/
void balance_start(void)
{
    cell_balance_off(g_balance_index);
    g_balance_status = E_BALANCE_IDLE;
    g_balance_index = 0;
}

#define MIN_FAIL_VALUE           3000
#define MAX_FAIL_TIMER           MAX_FAIL_TIME
/*=============================================================
 * 函数名称：bat_fail_process
 * 函数功能：电芯失效处理流程
 * 参数个数：2
 * 函数参数：
 *          [IN]    min_vol      单体最低电压
 *          [IN]    max_vol      单体最高电压
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                        修改人             修改内容
 * 2020-06-13       戴辉发             创建
==============================================================*/
static void bat_fail_process()
{
 
    uint16_t min_vol = get_min_cell_vol();
    uint16_t max_vol = get_max_cell_vol();
    switch(g_bat_status)
    {
    case E_BAT_COMMON:
        if (max_vol >= (get_cell_fail() + min_vol))
        /* 最大电压与最小电压之间压差满足电芯失效 */
        {
            g_bat_status = E_BAT_FAIL_WAIT;
            time_fail = MAX_FAIL_TIMER;
        }
        break;
    case E_BAT_FAIL_WAIT:
        if (max_vol >= (get_cell_fail() + min_vol))
        /* 最大电压与最小电压之间压差满足电芯失效 */
        {
            if (0 == time_fail)
            {
                record_protect = 1;
                g_bat_status = E_BAT_FAIL;
            }
        }
        else
        {
            g_bat_status = E_BAT_COMMON;
            time_fail = 0;
        }
        break;
    default:
        if (max_vol < (get_cell_recover() + min_vol))
        { /* 电芯失效恢复正常 */
            g_bat_status = E_BAT_COMMON;
        }
        break;
    }
	/*
    if((1 == ChargerIsConnect()) && (min_vol <  MIN_FAIL_VALUE))//((E_CHARGE_STATUS == get_system_status()) && (min_vol <  MIN_FAIL_VALUE))
    {
         g_bat_status = E_BAT_COMMON;
    }
    */

    if(E_BAT_FAIL == g_bat_status)
    {
        protect_code[6] |= 0x04;
    }
    else
    {
        protect_code[6] &= ~0x04;
    }
}

/*=============================================================
 * 函数名称：allow_balance_on
 * 函数功能：运行均衡开启
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2021-08-23       liy     创建
==============================================================*/
uint8_t allow_balance_on(void)
{
    uint16_t max_vol;
    uint16_t spin_vol;
    int16_t min_temp;
    int16_t max_temp;
    int16_t start_temp;
    int16_t end_temp;

    max_vol = get_max_cell_vol();
    spin_vol = max_vol - get_min_cell_vol();
    min_temp = get_min_cell_temp();
    max_temp = get_max_cell_temp();
    start_temp = get_balance_start_temp();
    end_temp = get_balance_end_temp();
    if ((spin_vol >= get_balance_start_difference()) && 
        (max_vol >= get_balance_start_voltage()) && 
        (min_temp >= start_temp) && 
        (max_temp <= end_temp) )
    {
        return 1;
    }

    return 0;
}

/*=============================================================
 * 函数名称：allow_balance_off
 * 函数功能：均衡关闭条件判决
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *          0       不满足均衡停止条件
 *          1       满足均衡停止条件
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2021-11-02       hfdai      创建
==============================================================*/
uint8_t allow_balance_off(void)
{
    uint16_t max_vol;
    uint16_t spin_vol;
    int16_t min_temp;
    int16_t max_temp;
    int16_t start_temp;
    int16_t end_temp;

    max_vol = get_max_cell_vol();
    spin_vol = max_vol - get_min_cell_vol();
    min_temp = get_min_cell_temp();
    max_temp = get_max_cell_temp();
    start_temp = get_balance_start_temp();
    end_temp = get_balance_end_temp();
    if ((spin_vol <= get_balance_end_difference()) ||  
        (max_vol <= get_balance_start_voltage()) ||  
        (min_temp < start_temp) || 
        (max_temp > end_temp) )
    {
        return 1;
    }

    return 0;
}

/*=============================================================
 * 函数名称：bat_balance_process
 * 函数功能：电芯均衡处理流程
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *          无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
 * 2021-11-02       戴辉发     在均衡状态中，将满足均衡条件修改为满足停止均衡条件
 *                             去除补丁g_blance_status_flag标志
 * 2021-11-03       戴辉发     在开始均衡的时候，添加 colse_blance_flag = 0;
 *                             用于采样均衡时标志初始化
==============================================================*/
static void bat_balance_process(void)
{
    static uint8_t colse_blance_flag;

    if(judge_vol_sample_finished() && \
        ((E_CHARGE_STATUS == get_system_status()) || (E_IDLE_STATUS == get_system_status())))
    {
        uint8_t temp;
        switch(g_balance_status)
        {
        case E_BALANCE_IDLE: /* 电芯正常 */
            {
                if (1 == allow_balance_on())
                {/* 电芯均衡启动 */
                    if((time_status_balance == 0) && (0 == get_cell_voltage_measure()))
                    {
                        g_balance_status = E_BALANCE_ON;
                        /* 找到电芯最高电压索引 */
                        g_balance_index =  get_max_cell_index();
                        /* 指定电芯开启 */
                        cell_balance_on(g_balance_index);
                        /* 电芯均衡定时 */
                        /* 0.5S,每次变化加1 相当于65535 * 65535 * 0.5 / 3600 / 24 /365  可以计算到68年；可行 */
                        time_balance = MAX_BALANCE_TIMER;
                        time_status_balance = DELAY_BALANCE_STATUS;
                        /* 初始关闭均衡标志为0，2021-11-03 hfdai@hznegt.com， */
                        colse_blance_flag = 0;
                    }
                }
                else
                {
                    time_status_balance = DELAY_BALANCE_STATUS / 5;
                }
            }
            break;
        case E_BALANCE_ON:
            if(0 != get_cell_voltage_measure())
            /* 需要暂停均衡20ms */
            {
                cell_balance_off(g_balance_index);
                colse_blance_flag = 1;
            }
            else
            {
                if(colse_blance_flag == 1)
                {
                    /* 指定电芯开启 */
                    cell_balance_on(g_balance_index);
                    colse_blance_flag = 0;
                }

                if (0 == time_balance)
                {
                    cell_balance_count(g_balance_index);
                    needstoreflag |= E_BLANCE_RECORD_MSG;

                    /* 判决均衡停止，2021-11-02, 修改，hfdai@hznegt.com，修改内容是将判决条件满足均衡状态修改为满足均衡停止状态 */
                    if (0 == allow_balance_off())
                    {
                         /* 找到电芯最高电压索引 */
                        temp =  get_max_cell_index();
                        if(temp != g_balance_index)
                        {
                            /* 均衡开启标志 */
                            g_balance_index = temp;
                            /* 指定电芯开启 */
                            cell_balance_on(g_balance_index);
                        }
                        /* 电芯均衡定时 */
                        /* 0.5S, 每次变化加1 相当于65535*65535*0.5/3600/24/365  可以计算到68年；可行 */
                        time_balance = MAX_BALANCE_TIMER;
                        time_status_balance = DELAY_BALANCE_STATUS;
                    }
                    else if(0 == time_status_balance)
                    {
                        /* 均衡开启标志 */
                        cell_balance_off(g_balance_index);
                        g_balance_status = E_BALANCE_IDLE;
                        g_balance_index = 0;
                        time_balance = 0;
                    }
                }
            }
            break;
        default: /*只有两个状态*/
            /* 去除均衡开启标志 */
            cell_balance_off(0);
            g_balance_status = E_BALANCE_IDLE;
            g_balance_index = 0;
            time_balance = 0;
            time_status_balance = DELAY_BALANCE_STATUS;
            break;
        }
    }
    else
    {
        if(E_BALANCE_ON == g_balance_status)
        /* 之前处于均衡状态，需要关闭均衡状态 */
        {
            cell_balance_off(g_balance_index);
            g_balance_index = 0;
        }
        g_balance_status = E_BALANCE_IDLE;
        /* 均衡开启标志 */
        time_status_balance = DELAY_BALANCE_STATUS/5;
    }
}

/*=============================================================
 * 函数名称：balance_mange
 * 函数功能：均衡模块管理流程
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2018-06-23       戴辉发     创建
==============================================================*/
void balance_mange(void)
{
    /* 电芯失效判决 */
    bat_fail_process();
    /* 电芯均衡判决 */
    bat_balance_process();
}
