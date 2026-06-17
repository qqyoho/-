/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：AFE_APP.C
 * 文件功能描述：实现AFE操作和控制
 * 
 * 修改记录：
 * 2021-05-06    戴辉发    创建
*----------------------------------------------------------*/
#include "parameter.h"
#include "afe_app.h"
#include "JW3370.h"
#include "fm33lg0xx_fl_gpio.h"
#include "adc_sampling.h"
#include "run_record.h"
#include "switch_status.h"
#include "balance.h"
#include "protect_record.h"
#include "short.h"
#include "idog.h"
#include "vol_manage.h"

#define TIME_TIMERS              5

#define ONE_SECOND_TIME          (1000 / TIME_TIMERS)

#define MAX_SPI_FAIL_CON_NUM     100    /* 最高SPI连续失败次数 */


/* 本次上电SPI通信连续失败次数 */
static uint8_t spi_con_fail_count;
/* 获取数据延时定时器 */
static volatile uint8_t t_get_data_delay;

static volatile uint8_t afeOvRecDelay;  /* AFE过压恢复delay */
static uint8_t afeOvStatus; /* Afe过压恢复Status */

/* AFE状态，0：通讯失效，1：正常通信状态 */
static uint8_t afe_status;
/* AFE保护状态，0：无保护状态，1：保护状态 */
static uint8_t afeProtectStatus;

static uint16_t cell_ov_status; /* 电芯电压过压保护状态 */
static uint16_t cell_uv_status; /* 电芯电压欠压保护状态 */
static uint16_t cell_open_status; /* 电芯开路检查状态 */
static uint8_t cell_temp_status; /* 电芯温度状态，高4位为电芯高温保护状态，低4位位电芯低温保护状态 */

/* 电流保护状态, bit4:充电过流状态；bit3:放电二次过流状态；bit2:放电一次过流状态；bit0:短路状态 */
static uint8_t curr_status;
static uint8_t for_get_voltage = 0;
/* static uint8_t blance_status; */
uint8_t sleep_status, charger_status;

static uint8_t load_status; /* 负载状态，0：负载连接，1：负载移除，2：无法判决状态 */
static uint8_t chg_status; /* 充电器接入状态，0：充电器接入，1：充电器移除，2：无法判决状态 */
static uint8_t dch_sw_afe_status; /* afe放电MOS状态 */
static uint8_t ch_sw_afe_status; /* afe充电MOS状态 */


static volatile uint16_t t_load_detect_timer; /* 负载检测定时器 */
static volatile uint16_t t_chg_detect_timer; /* 充电器检测定时器 */
static uint8_t out_load_status; /* 对外负载状态，0：负载连接，1：负载移除，2：无法判决状态 */
static uint8_t out_chg_status; /* 对外充电器接入状态，0：充电器接入，1：充电器移除，2：无法判决状态 */

/* 标识AFE通信状态 */
static uint8_t afe_status;

/* 函数地址，类似于void cell_get_voltage_process(uint8_t index, float ad_value)*/
static uint32_t cell_deal_proc_addr;

/* 函数地址，类似于void temp_deal_proc(float temp_rvalue, uint8_t index) */
static uint32_t temp_deal_proc_addr;

/* 函数地址，类似于void curr_deal_proc(uint16_t curr_vol) */
static uint32_t curr_deal_proc_addr;
/*读取备份AFE的电压和温度值*/
uint32_t seeBackupVoltageFlag;
/*根据均衡状态设置采样周期*/
static void load_detect_proc(void);
static void chg_detect_proc(void);

/*=============================================================
 * 函数名称：afe_app_mem_init
 * 函数功能：AFE应用内存初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_app_mem_init(void)
{
    /* 短路检测管理控制模块内存初始化 */
    afe_mem_init();

    spi_con_fail_count = 0; /* 本次上电SPI通信连续失败次数 */
    for_get_voltage = 0;
    t_get_data_delay = 50;
    
    afeOvRecDelay = 0;
    afeOvStatus = 0;

    cell_ov_status = 0;
    cell_uv_status = 0;
    cell_open_status = 0;
    cell_temp_status = 0;

    curr_status = 0;
    load_status = 0;
    chg_status = 0;
    afe_status = 1;
    dch_sw_afe_status = 0;
    ch_sw_afe_status = 0;

    out_load_status = 2;
    out_chg_status = 2;

    afe_status = 1;

    /* 电芯电压处理函数地址 */
    cell_deal_proc_addr = 0;
    /* 温度处理函数地址 */
    temp_deal_proc_addr = 0;
    /* 电流处理函数 */
    curr_deal_proc_addr = 0;
    
    afeProtectStatus = 0;
    
    g_curr_para.dch_current.short_protect = AFE_SHORT_CURRENT * 10;
}

/*=============================================================
 * 函数名称：set_vol_buf
 * 函数功能：设定上层处理电压缓冲地址
 * 参数个数：2
 * 参数描述：
 *          [IN]     buf        缓冲地址
 *          [IN]     cell_num   电芯个数
 * 返 回 值：
 *          无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-03        戴辉发      创建
==============================================================*/
void set_vol_buf(uint32_t addr)
{
    cell_deal_proc_addr = addr;
}

/*=============================================================
 * 函数名称：set_temp_buf
 * 函数功能：设定上层处理电芯温度缓冲地址
 * 参数个数：1
 * 参数描述：
 *          [IN]     addr        温度处理函数地址
 *                               函数型式void temp_deal_proc(uint16_t temp_rvalue, uint8_t index)
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-03        戴辉发      创建
==============================================================*/
void set_temp_buf(uint32_t addr)
{
    temp_deal_proc_addr = addr;
}

/*=============================================================
 * 函数名称：set_curr_deal_process
 * 函数功能：设定电流处理函数地址
 * 参数个数：1
 * 参数描述：
 *          [IN]     addr        电流处理函数地址
 *                               函数型式void curr_deal_proc(uint16_t cell_vol)
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-10        戴辉发      创建
==============================================================*/
void set_curr_deal_process(uint32_t addr)
{
    curr_deal_proc_addr = addr;
}

/*=============================================================
 * 函数名称：afe_app_timer
 * 函数功能：AFE应用定时器
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-10        戴辉发      创建
==============================================================*/
void afe_app_timer(void)
{
    if (t_get_data_delay > 0)
    {
        t_get_data_delay --;
    }
    if (t_load_detect_timer > 0)
    {
        t_load_detect_timer --;
    }
    if (t_chg_detect_timer > 0)
    {
        t_chg_detect_timer --;
    }
    if (seeBackupVoltageFlag > 0)
    {
        seeBackupVoltageFlag --;
    }      
    if (afeOvRecDelay > 0)
    {
        afeOvRecDelay --;
    }
}

/*=============================================================
 * 函数名称：afe_app_init
 * 函数功能：AFE应用初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_app_init(void)
{
    uint16_t AfeOvCellVol;
    /*参数重新配置*/
    afe_mem_init();

    spi_con_fail_count = 0; /* 本次上电SPI通信连续失败次数 */
    afe_status = 1;
    afeProtectStatus = 0;
    
    cell_ov_status = 0;
    cell_uv_status = 0;
    cell_open_status = 0;
    cell_temp_status = 0;
    for_get_voltage = 0;
    t_get_data_delay = 50;
    
    curr_status = 0;
    load_status = 0;
    chg_status = 0;
    afe_status = 1;
    dch_sw_afe_status = 0;
    ch_sw_afe_status = 0;
    seeBackupVoltageFlag = 0;
    out_load_status = 2;
    out_chg_status = 2;

    afe_status = 1;
    
#if defined(TIANFENG) && defined(BAT_8S)
    AfeOvCellVol = get_cell_vol_high_protect() + 250;
    if(AfeOvCellVol < 300)
    {
        AfeOvCellVol = AFE_OVP_CELL_VOL;
    }
#else
	AfeOvCellVol = AFE_OVP_CELL_VOL;
#endif
    
    /* AFE硬件初始化 */
    afe_hard_init();

    /* 给AFE上电 */
    afe_power_up();
   
    /* 查询地址是否正常 */
    //afe_update_address();
    
    /* 设置基本参数 */
	set_ctr_parameter(0, BAT_NUM);
    set_ctr_parameter(0, BAT_NUM);
    set_ctr_parameter(0, BAT_NUM);
    set_ctr_parameter(0, BAT_NUM);
    /* 喂狗 */
    feed_iwdg();
    alarm_set(0);
    alarm_set(0);
    alarm_set(0);
    /* 喂狗 */
    feed_iwdg();
    set_voltage_parameter(AfeOvCellVol/*AFE_OVP_CELL_VOL*/, AFE_UVP_CELL_VOL, AFE_OVT_CELL_TEMP, AFE_UVT_CELL_TEMP, 0x00, 0x01, 0x00);
    set_voltage_parameter(AfeOvCellVol/*AFE_OVP_CELL_VOL*/, AFE_UVP_CELL_VOL, AFE_OVT_CELL_TEMP, AFE_UVT_CELL_TEMP, 0x00, 0x01, 0x00);
    set_voltage_parameter(AfeOvCellVol/*AFE_OVP_CELL_VOL*/, AFE_UVP_CELL_VOL, AFE_OVT_CELL_TEMP, AFE_UVT_CELL_TEMP, 0x00, 0x01, 0x00);
    /* 喂狗 */
    feed_iwdg();
    /* 将充电过流设置到放电二次过流挡位，目的是为了屏蔽充电过流，防止误触发回馈电 */    
    uint16_t sc_delay = 400;
#if (defined(TIANHONG) && (defined(CAPCITY_100) || defined(CAPCITY_60)))
    sc_delay = 100;
#endif
    set_current_parameter(g_curr_para.dch_current.short_protect / 10, sc_delay, AFE_OVER_DCH_CURRENT, AFE_OVER_CH_CURRENT);
    set_current_parameter(g_curr_para.dch_current.short_protect / 10, sc_delay, AFE_OVER_DCH_CURRENT, AFE_OVER_CH_CURRENT);
    set_current_parameter(g_curr_para.dch_current.short_protect / 10, sc_delay, AFE_OVER_DCH_CURRENT, AFE_OVER_CH_CURRENT);
    /* 喂狗 */
    feed_iwdg();
    /* 初次设置值，清除告警值 */
    afe_enter_normal();
    /* 喂狗 */
    feed_iwdg();
    /* 初次设置值，清除告警值 */
    clear_alarm();
    clear_alarm();
    clear_alarm();
    afe_final_protect_clear();
}

/*=============================================================
 * 函数名称：afe_app_sleep
 * 函数功能：AFE应用进入休眠状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
uint8_t afe_app_sleep(void)
{
    uint8_t flag = 0;
    //uint8_t waittingSleep = 20;
     /* 初次设置值，清除告警值 */
    clear_alarm();
    close_load_detect_proc();
    SleepSysctrlSet(); 
    flag = afe_power_down();
    FL_SPI_DeInit(M_AFE_SPI);
    FL_SPI_DeInit(R_AFE_SPI);
    return flag;
}

/*=============================================================
 * 函数名称：get_adjust_cell_vol
 * 函数功能：获取校准时单体电压
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT]cell_buf    获取到的电芯电压
 * 返 回 值：
 *           0       获取失败
 *           1       获取成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-11-02        戴辉发      创建
==============================================================*/
uint8_t get_adjust_cell_vol(uint16_t *cell_buf)
{
    uint16_t vol_buf[CELLS_NUM];
    int16_t temp_buf[MAX_CELL_TEMP_NUM + 1];
    uint8_t ret;

    ret = read_voltage_temp(0x00, vol_buf, temp_buf);
    /* 读取电压温度数值 */
    if (1 == ret)
    {
        uint8_t i;

        /* 电压缓冲未设置 */
        if (0 != cell_deal_proc_addr)
        {
            /* 获取对应电芯电压 */
            for (i = CELLS_NUM - BAT_NUM; i < CELLS_NUM; i ++)
            {
                cell_buf[i - (CELLS_NUM - BAT_NUM)] = (vol_buf[i] & 0x7FFF);
            }
        }
    }

    return ret;
}

/*=============================================================
 * 函数名称：get_cell_vol_temp
 * 函数功能：获取电芯电压盒电芯温度处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       获取失败
 *           1       获取成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
static uint8_t get_cell_vol_temp(void)
{
#define BC_VALUE          0
    uint16_t vol_buf[CELLS_NUM];
    int16_t temp_buf[MAX_CELL_TEMP_NUM + 1];
    uint8_t ret;

    ret = read_voltage_temp(0x00, vol_buf, temp_buf);
    /* 读取电压温度数值 */
    if (( 1 == ret ))
    {
        uint8_t i;
        
        /* 电压缓冲未设置 */
        if (0 != cell_deal_proc_addr)
        {
            typedef void (*cell_voltage_process)(uint8_t index, float ad_value);
            cell_voltage_process cell_deal_func;
            float voltage;

            cell_deal_func = (cell_voltage_process )cell_deal_proc_addr;

            /* 获取对应电芯电压 */
            for (i = CELLS_NUM - BAT_NUM; i < CELLS_NUM; i ++)
            {
                voltage = (vol_buf[i] & 0x7FFF);
                
                cell_deal_func(i - (CELLS_NUM - BAT_NUM), voltage);   
            }
        }
        /* 温度，读取回来的电芯温度只是对应的电压，需要将电压转换为对应的温度
           计算公式如下所示：
           Rntc = Untc / Intc
                = Untc / [(Utb - Untc) / Rserial]
                = Untc / [(3.3V - Untc) / 10.0K]
                = Untc * 10K / (3.3V - Untc);
                = (10K / 3.3V) * Untc - 10 * Untc * Untc
                = ?K
        */
        if (0 != temp_deal_proc_addr)
        {
            if (0 != temp_deal_proc_addr)
            {
                typedef void (*temp_deal_func_addr)(int32_t temp_rvalue, uint8_t index);
                temp_deal_func_addr temp_deal_func;
                int32_t temp;

                temp_deal_func = (temp_deal_func_addr )temp_deal_proc_addr;
                /* 修改温度采集数据为2个，最后一个为AD参考电压 */
                for (i = 0; i < MAX_CELL_TEMP_NUM; i++)
                {
                    temp = 10000 * temp_buf[i] / (3300 - temp_buf[i]);
                    temp_deal_func(temp, i);
                }
            }
        }
    }

    return ret;
}

/*=============================================================
 * 函数名称：get_curr_data
 * 函数功能：获取系统电流处理流程
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       获取失败
 *           1       获取成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
static uint8_t get_curr_data(void)
{
    uint16_t temp[3];
    float ftemp[2];
    
    uint8_t ret;

    /* 读取电流数值 */
    ret = read_current(temp);

    if (1 == ret)
    /* 读取电流数值 */
    {
        if (0 != curr_deal_proc_addr)
        {
            typedef void (*curr_dael_func_addr)(float curr);

            curr_dael_func_addr curr_deal_proc;

            curr_deal_proc = (curr_dael_func_addr )curr_deal_proc_addr;
            ftemp[0] = temp[0];
            ftemp[1] = temp[1];

            ftemp[0] = -( ftemp[0] * 20000.0 / 0x8000 - 20000.0 );/* 0.1A为单位 */  
            ftemp[1] = -( ftemp[1] * 20000.0 / 0x8000 - 20000.0 );/* 0.1A为单位 */  
            if(( ftemp[0] > ftemp[1]+780 )||( ftemp[0] < ftemp[1]-780 ))
            {/*超出门限值，则取高9bit的电流值为实际的电流值 127*6uv = 780uv 放大十倍误差*/
                //SendTestData1WithCan(ftemp);
                ftemp[0] = ftemp[1];
            }
            curr_deal_proc(ftemp[0]);
        } /*数据暂时*/
    }

    return ret;
}

/*=============================================================
 * 函数名称：get_curr_vol_data
 * 函数功能：获取电流采样分流器两端的电压
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT] *curr_vol  分流器两端的电压
 * 返 回 值：
 *           0       获取失败
 *           1       获取成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-10-25        戴辉发      创建
==============================================================*/
uint8_t get_curr_vol_data(uint16_t *curr_vol)
{
    return read_current(curr_vol);
}

/*=============================================================
 * 函数名称：get_alarm_status_data
 * 函数功能：获取AFE各种状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       获取失败
 *           1       获取成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
static uint8_t get_alarm_status_data(void)
{
    uint8_t temp_alarm_data[10];
    uint8_t ret;

    /* 读取电压温度数值 */
    ret = read_alarm(0x00, temp_alarm_data);
    if (1 == ret)
    /* 读取告警信息 */
    {
        /* 电芯电压过压状态 */
        cell_ov_status = temp_alarm_data[0];
        cell_ov_status <<= 2;
        cell_ov_status = cell_ov_status | ((temp_alarm_data[1] >> 6) & 0x03);

        /* 电芯电压欠压状态 */
        cell_uv_status = (temp_alarm_data[1] & 0x03);
        cell_uv_status <<= 8;
        cell_uv_status |= temp_alarm_data[2];

        /* 电芯温度过温状态 */
        cell_temp_status = (temp_alarm_data[3] & 0x07);
        cell_temp_status <<= 4;
        /* 电芯温度低温状态 */
        cell_temp_status |= ((temp_alarm_data[4] >> 5) & 0x07);

        /* 充电过流保护状态 */
        curr_status = ((temp_alarm_data[4] >> 3) & 0x01);
        curr_status <<= 1;
        /* 放电过流保护状态，二级过流 */
        curr_status |= (temp_alarm_data[4] & 0x01);
        curr_status <<= 1;
        curr_status |= ((temp_alarm_data[5] >> 7) & 0x01);
        curr_status <<= 2;
        /* 放电回路短路状态 */
        curr_status |= ((temp_alarm_data[5] >> 4) & 0x01);

        /* 断线检测状态 */
        if ((temp_alarm_data[6] & 0x02))
        /* 开路检测完成状态 */
        {
            cell_open_status = (temp_alarm_data[5] & 0x0F);
            cell_open_status <<= 6;
            cell_open_status |= ((temp_alarm_data[6] >> 2) & 0x3F);
        }
        else
        {
            cell_open_status = 0;
        }

        /* 放电MOS状态 */
        dch_sw_afe_status = ((temp_alarm_data[7] >> 7) & 0x01);
        /* 充电MOS状态 */
        ch_sw_afe_status = ((temp_alarm_data[7] >> 6) & 0x01);
        
        /* 负载移除状态 */
        load_status = ((temp_alarm_data[7] >> 2) & 0x01);
        
        /* blance_status = ((temp_alarm_data[8] >> 6) & 0x01); */
        charger_status = ((temp_alarm_data[8] >> 5) & 0x01);
        /* 芯片睡眠状态指示标志*/
        sleep_status = ((temp_alarm_data[8] >> 4) & 0x01); 
        /* 充电移除状态 */
        chg_status = ((temp_alarm_data[8] >> 2) & 0x01);
    }
        
    /* 读取备用AFE的告警值 */
    ret = read_reserve_alarm(0x00, temp_alarm_data);
    if (1 == ret)
    /* 读取告警信息 */
    {
        /* 电芯电压过压状态 */
        if( cell_ov_status == 0 )
        {
            cell_ov_status = temp_alarm_data[0];
            cell_ov_status <<= 2;
            cell_ov_status = cell_ov_status | ((temp_alarm_data[1] >> 6) & 0x03);
        }

        /* 电芯电压欠压状态 */
        if( cell_uv_status == 0 )
        {
            cell_uv_status = (temp_alarm_data[1] & 0x03);
            cell_uv_status <<= 8;
            cell_uv_status |= temp_alarm_data[2];
        }
        
        /* 电芯温度过温状态 */
        if( cell_temp_status == 0 )
        {
            cell_temp_status = (temp_alarm_data[3] & 0x07);
            cell_temp_status <<= 4;
            /* 电芯温度低温状态 */
            cell_temp_status |= ((temp_alarm_data[4] >> 5) & 0x07);
        }
        

        /* 电流保护状态 */
        if( curr_status == 0 )
        {
            curr_status = ((temp_alarm_data[4] >> 3) & 0x01);
            curr_status <<= 1;
            /* 放电过流保护状态，二级过流 */
            curr_status |= (temp_alarm_data[4] & 0x01);
            curr_status <<= 1;
            curr_status |= ((temp_alarm_data[5] >> 7) & 0x01);
            curr_status <<= 2;
            /* 放电回路短路状态 */
            curr_status |= ((temp_alarm_data[5] >> 4) & 0x01);
        }
        

        /* 断线检测状态 */
        if (( cell_open_status == 0 )&&((temp_alarm_data[6] & 0x02)))
        /* 开路检测完成状态 */
        {
            cell_open_status = (temp_alarm_data[5] & 0x0F);
            cell_open_status <<= 6;
            cell_open_status |= ((temp_alarm_data[6] >> 2) & 0x3F);
        }

        /* 放电MOS状态 */
        dch_sw_afe_status &= ((temp_alarm_data[7] >> 7) & 0x01);
        /* 充电MOS状态 */
        ch_sw_afe_status &= ((temp_alarm_data[7] >> 6) & 0x01);
        
        /* 负载移除状态 */
        //load_status = ((temp_alarm_data[7] >> 2) & 0x01);
        
        /* blance_status = ((temp_alarm_data[8] >> 6) & 0x01); */
        //charger_status = ((temp_alarm_data[8] >> 5) & 0x01);
        /* 芯片睡眠状态指示标志*/
        /* sleep_status = ((temp_alarm_data[8] >> 4) & 0x01); */
        /* 充电移除状态 */
        //chg_status = ((temp_alarm_data[8] >> 2) & 0x01);
    }


    return ret;
}
/*=============================================================
 * 函数名称：get_afe_control_mos_status
 * 函数功能：实时获取afe充放电mos状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
           0x0  断开
           0x1  闭合
           0x2  无效状态
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
uint8_t get_afe_control_mos_status(void)
{
    uint8_t mos = 0x02;

    if(1 == get_alarm_status_data())
    {
        mos = ch_sw_afe_status & dch_sw_afe_status & 0x01;
    }

    return mos;
}
/*=============================================================
 * 函数名称：get_afe_control_chg_mos_status
 * 函数功能：实时获取afe充电mos状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
           0x0  断开
           0x1  闭合
           0x2  无效状态
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
uint8_t get_afe_chg_mos_status(void)
{
    uint8_t chg = 0x02;

    if(1 == get_alarm_status_data())
    {
        chg = ch_sw_afe_status;
    }

    return chg;
}

/*=============================================================
 * 函数名称：get_afe_dis_mos_status
 * 函数功能：实时获取afe放电mos状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
           0x0  断开
           0x1  闭合
           0x2  无效状态
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
uint8_t get_afe_dis_mos_status(void)
{
    uint8_t dis = 0x02;
    if(1 == get_alarm_status_data())
    {
        dis = dch_sw_afe_status;
    }

    return dis;
}

/*******************************************************************************
 ** FuncName: get_cell_voltage_measure
 ** Function: 电压采集前，关闭均衡20ms;
 ** Output  :  0：不需要关闭 1：需要关闭;
 ** input   : 无;
 ** Create date : liyong @2021.8.24
 ** Modify  : 
*******************************************************************************/
uint8_t get_cell_voltage_measure(void)
{
   return for_get_voltage;
}

/*******************************************************************************
 ** FuncName: afe_open_line_detect
 ** Function: 断线检测;
 ** Output  : 无;
 ** input   : 无;
 ** Create date : liyong @2021.8.30
 ** Modify  : 
*******************************************************************************/
void afe_open_line_detect(void)
{
#if 0
    static uint8_t breakline_step;
    static uint8_t breakline_delay;

    /* 60s做一次断线检测 */
    switch(breakline_step)
    {
    case 0:/* 禁止断线检测 */
        set_voltage_parameter(AFE_OVP_CELL_VOL, AFE_UVP_CELL_VOL, AFE_OVT_CELL_TEMP, AFE_UVT_CELL_TEMP, 0x00, 0x01, 0x00); 
        breakline_step = 1;
        break;
    case 1:/* 禁止参数设置成功 */
        if(get_para_set_flag() & 0x04)
        {
            breakline_step = 2;
        }
        break;
    case 2:/* 使能断线检测功能 */
        set_voltage_parameter(AFE_OVP_CELL_VOL, AFE_UVP_CELL_VOL, AFE_OVT_CELL_TEMP, AFE_UVT_CELL_TEMP, 0x00, 0x01, 0x01); 
        breakline_step = 3;
        break;
    case 3:/* 延时50s */
        breakline_delay++;
        if(breakline_delay > 250)
        {
            breakline_delay = 0;
            breakline_step = 0;
        }
        break;
    default:
        breakline_step = 0;
        break;
    }
#endif
}

/*=============================================================
 * 函数名称：get_data_process
 * 函数功能：获取数据流程
 * 参数c个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-06        戴辉发      创建
==============================================================*/
#define CURRENT_CYCLE           10
#define ALARM_CYCLE             2
#define VOLTAGE_TEMP_CYCLE      20
void get_data_process(void)
{
    static uint8_t spiConFlag = 0x0;
    static uint8_t read_current_delay;
    static uint8_t read_current_alarm;
    static uint8_t read_current_voltage_temp;

    /*读取AlARM管脚电平 第一时间获取故障*/
    if( afeProtectStatus == 0 )
    {
        if( FL_GPIO_GetInputPin(AFE_ALARM_DETECT_GPIO_TYPE, AFE_ALARM_DETECT_GPIO_PIN) ||
            FL_GPIO_GetInputPin(R_AFE_ALARM_DETECT_GPIO_TYPE, R_AFE_ALARM_DETECT_GPIO_PIN) )
        {
            afeProtectStatus = 1; 
            get_alarm_status_data();
        }
    }
    
    if (t_get_data_delay) return;
    t_get_data_delay = 2;

     read_current_alarm ++;
    /* 读取告警信息 */
    if ( read_current_alarm >= ALARM_CYCLE )
    {
        if ( 0 == get_alarm_status_data() )
        {
           spiConFlag |= 0x01;
        }
        else
        {
           spiConFlag &= ~0x01; 
        }
        read_current_alarm = 0;
    }
    
    read_current_delay ++;
    if( read_current_delay >= CURRENT_CYCLE )
    {
        /* 读取电流数值 */
        if ( 0 == get_curr_data() )
        {
           spiConFlag |= 0x02;
            //afe_open_line_detect();
        }
        else
        {
           spiConFlag &= ~0x02;  
        }
        read_current_delay = 0;
    }

    read_current_voltage_temp ++;
    /* 读取电压温度数值 */
    if ( read_current_voltage_temp >= VOLTAGE_TEMP_CYCLE )
    {
        if ( 0 == get_cell_vol_temp() )
        {
            spiConFlag |= 0x04;
        }   
        else
        {
            spiConFlag &= ~0x04;  
        }
        for_get_voltage = 0;
        read_current_voltage_temp = 0;
    }
    else if ( read_current_voltage_temp >= ( VOLTAGE_TEMP_CYCLE - 5 ))
    {
        for_get_voltage = 1;
    }

    if( spiConFlag != 0 )
    {
        spi_con_fail_count += 1;
    }
    else if ( spi_con_fail_count > 0 )
    {
        spi_con_fail_count--;
    }
    

    /* SPI通信连续失败次数超过设定值 */
    if ( MAX_SPI_FAIL_CON_NUM <= spi_con_fail_count )
    /* 通讯失效 */
    {
        spi_con_fail_count = MAX_SPI_FAIL_CON_NUM;

        if (afe_status != 0)
        {
            uint8_t i;

            afe_status = 0;
            for (i = 0; i < BAT_NUM; i ++)
            {
                g_run_sys_data.cell_vol[i] &= 0x8000;
            }
            g_run_sys_data.current = 0;

            set_afe_commu();
        }
    }
    else if ( 0 == spi_con_fail_count )
    {
        afe_status = 1;
    }

    /* 负载移除检测 */
    load_detect_proc();
    /* 充电器移除检测 */
    //chg_detect_proc();
}
/*=============================================================
 * 函数名称：get_cell_vol_ovp_status
 * 函数功能：获取AFE检测到的过压保护标志
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       没有过压
 *           1       发生过压
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_cell_vol_ovp_status(void)
{
    if (cell_ov_status > 0)
    {
        return 1;
    }
    return 0;
}

/*=============================================================
 * 函数名称：get_cell_vol_uvp_status
 * 函数功能：获取AFE检测到的欠压保护标志
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       没有欠压
 *           1       发生欠压
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_cell_vol_uvp_status(void)
{
    if (cell_uv_status > 0)
    {
        return 1;
    }
    return 0;
}

/*=============================================================
 * 函数名称：get_curr_status
 * 函数功能：获取AFE检测到的过流保护标志
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       没有电流保护
 *           1       发生充电电流保护
 *           2       放电过流保护
 *           3       短路保护
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_curr_status(void)
{
    /* 电流保护状态，bit4:充电过流状态；bit3:放电二次过流状态；bit2:放电一次过流状态；bit0:短路状态 */
    /* 充电过流状态 */
    if (((curr_status >> 4) & 0x01) > 0)
    {
        return 1;
    }
    /* 放电过流状态 */
    else if (((curr_status >> 2) & 0x03) > 0)
    {
        return 2;
    }
    /* 短路状态 */
    else if ((curr_status & 0x01) > 0)
    {
        return 3;
    }

    return 0;
}

/*=============================================================
 * 函数名称：get_load_status
 * 函数功能：获取AFE检测到的负载状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       负载接入
 *           1       负载移除
 *           2       无法判断状态
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_load_status(void)
{
    return out_load_status;
}

/*=============================================================
 * 函数名称：get_ch_remove_status
 * 函数功能：获取AFE检测到的充电器移除状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       充电器接入
 *           1       充电器移除
 *           2       无法判断状态
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_ch_remove_status(void)
{
    uint8_t flag = 2;    
    return flag;
}
/*=============================================================
 * 函数名称：get_cell_open_status
 * 函数功能：获取AFE检测到的电芯断线检测状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       正常
 *           >0      电芯断线
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_cell_open_status(void)
{
    if(cell_open_status > 0)
    {
        return 1;
    }
    return 0;
}

/*=============================================================
 * 函数名称：get_cell_temp_status
 * 函数功能：获取AFE检测到的电芯温度保护状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       电芯温度正常
 *           1       电芯高温保护
 *           2       电芯低温保护
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t get_cell_temp_status(void)
{
    if(cell_temp_status & 0x03)
    {
        return 1;
    }
    else if(cell_temp_status & 0x60)
    {
        return 2;
    }
    return 0;
}

/*=============================================================
 * 函数名称：cell_balance_off
 * 函数功能：关闭指定电芯均衡
 * 参数个数：1
 * 参数描述：
 *           [IN]    index        索引
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
void cell_balance_off(uint8_t index)
{
    balance_set(0x0000);
}
/*=============================================================
 * 函数名称：cell_balance_on
 * 函数功能：打开指定电芯均衡
 * 参数个数：1
 * 参数描述：
 *           [IN]    index        索引
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
void cell_balance_on(uint8_t index)
{
    /* 不考虑没有设置电芯节数 */
    balance_set((1 << index) << (CELLS_NUM - BAT_NUM));
}

#define DELAY_TIME_US        8
/*=============================================================
 * 函数名称：ch_disch_mos_ctrl
 * 函数功能：MOS控制
 * 参数个数：1
 * 参数描述：
 *           [IN]    swt_status   索引
 * 返 回 值：
 *           0: 控制不成功
             1：控制成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t ch_disch_mos_ctrl(e_switch_status swt_status)
{
    uint8_t ctrl = 0;
    
    switch(swt_status)
    {
    case E_DISCHARGE_ON: /* 充放电开关都闭合 */
        if ( 0 == dch_sw_afe_status )
        {   
            sysctrl_set(ch_sw_afe_status, 1);
            get_afe_control_mos_status();
        }
        else 
        {
            ctrl = 1;
        }
        break;
    case E_DISCHARGE_OFF: /* 充放电开关都闭合 */
        if (( 1 == dch_sw_afe_status ))
        {   
            sysctrl_set(ch_sw_afe_status, 0);
            get_afe_control_mos_status();
        }
        else 
        {
            ctrl = 1;
        }
        break;
	case E_CHARGE_ON: /* 充电开关都闭合 */
        if ( 0 == ch_sw_afe_status )
        {   
            sysctrl_set(1, dch_sw_afe_status);
            get_afe_control_mos_status();
        }
        else 
        {
            ctrl = 1;
        }
        break;
    case E_CHARGE_OFF: /* 充电开关都闭合 */
        if (( 1 == ch_sw_afe_status ))
        {   
            sysctrl_set(0, dch_sw_afe_status);
            get_afe_control_mos_status();
        }
        else 
        {
            ctrl = 1;
        }
        break;
    default: /* 充放电开关断开 */
        if ( ( 1 == dch_sw_afe_status )||( 1 == ch_sw_afe_status ) )
        {
            sysctrl_set(0, 0); 
            get_afe_control_mos_status();
        }
        else 
        {
            ctrl = 1;
        }
        break;
    }
    
    return ctrl;
}

/*=============================================================
 * 函数名称：afe_control_off_dis
 * 函数功能：AFE导致的放电MOS控制
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           0       不存在AFE自动切断MOS操作
 *           1       存在AFE保护自动切断MOS操作
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-08        戴辉发      创建
==============================================================*/
uint8_t afe_control_off_dis(void)
{
    /* 电芯采样线开路 */
    if (cell_open_status > 0)
    {
        return 1;
    }
    /* 电流保护状态 */
    if (curr_status > 0)
    {
        return 1;
    }
    /* 电芯电压欠压保护状态 */
    if (cell_uv_status > 0)
    {
        return 1;
    }
    /* 电芯电压过压保护状态 */
    if (cell_ov_status > 0)
    {
        return 1;
    }
    /* 电芯温度保护状态 */
    if (cell_temp_status > 0)
    {
        return 1;
    }

    return 0;
}

/*=============================================================
 * 函数名称：afe_control_off_chg
 * 函数功能：AFE导致的充电MOS控制
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           0       不存在AFE自动切断MOS操作
 *           1       存在AFE保护自动切断MOS操作
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-11        戴辉发      创建
==============================================================*/
uint8_t afe_control_off_chg(void)
{
    if (cell_open_status > 0)
    {
        return 1;
    }
    if (curr_status > 0)
    {
        return 1;
    }
    if (cell_uv_status > 0)
    {
        return 1;
    }
    if (cell_ov_status > 0)
    {
        return 1;
    }
    if (cell_temp_status > 0)
    {
        return 1;
    }

    return 0;
}

/*=============================================================
 * 函数名称：get_afe_status
 * 函数功能：获取AFE状态
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           0       正常通信状态
 *           1       自检或通信失败状态
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-11        戴辉发      创建
==============================================================*/
uint8_t get_afe_status(void)
{
    if (afe_status == 1)
    {
        return 0;
    }

    return 1;
}
/*=============================================================
 * 函数名称：afe_final_protect_clear
 * 函数功能：AFE终极保护清零
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-22         liy      创建
==============================================================*/
void afe_final_protect_clear(void)
{
    uint16_t flag = get_final_set_switch();

    flag &= ~0x3fc;
    protect_code[0] &= ~0x80;
    protect_code[0] &= ~0x40;
    protect_code[1] &= ~0x04;
    protect_code[1] &= ~0x08;
    protect_code[1] &= ~0x20;
    protect_code[1] &= ~0x80;
    protect_code[3] &= ~0x10;
    protect_code[1] &= ~0x40;

    /* 0x01  软件终极过压  0x02 软件终极欠压
       0x04  AFE终极过压  0x08  AFE终极欠压
       0x10  AFE短路保护  0x20  AFE放电过流
       0x40  AFE充电过流  0x80  AFE高温保护
       0x100 AFE低温保护  0x200 断线保护
    */
    final_set_switch(flag);
}

/*=============================================================
 * 函数名称：afe_final_protect
 * 函数功能：AFE终极保护处理流程
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-11        戴辉发      创建
==============================================================*/
void afe_final_protect(void)
{
    if (0 == get_afe_status())
    {
        uint16_t flag = get_final_set_switch();

        /* AFE终极欠压 */
        if (get_cell_vol_uvp_status())
        {
            if (0 == (flag & 0x08))
            {
                flag |= 0x08;
                record_protect = 1;
                protect_code[0] |= 0x40;
            }
        }
        //else
        //{
        //    flag &= ~0x08;
        //    protect_code[0] &= ~0x40;  
        //}

        /* 电流保护 */
        /* AFE短路保护 */
        if ((3 == get_curr_status()))
        {
            if ((flag & 0x10) == 0)
            {
                flag |= 0x10;
                record_protect = 1;
                protect_code[1] |= 0x04;
            }
        }
        else if (1 == get_load_status())
        {
            flag &= ~0x10;
            protect_code[1] &= ~0x04;
        }

        /* AFE放电过流 */
        if (2 == get_curr_status())
        {
            if (0 == (flag & 0x20))
            {
                flag |= 0x20;
                record_protect = 1;
                protect_code[1] |= 0x08;
            }
        }
        else if (1 == get_load_status())
        {
            flag &= ~0x20;
            protect_code[1] &= ~0x08;
        }

        /* AFE充电过流 */
        if (1 == get_curr_status())
        {
            if (0 == (flag & 0x40))
            {
                flag |= 0x40;
                record_protect = 1;
                protect_code[1] |= 0x20;
            }
        }
        //else
        //{
        //    flag &= ~0x40;
        //    protect_code[1] &= ~0x20;
        //}

        /* 温度保护 */
        /* AFE高温保护 */
        if (1 == get_cell_temp_status())
        {
            if (0 == (flag & 0x80))
            {
                flag |= 0x80;
                record_protect = 1;
                protect_code[1] |= 0x80;
            }
        }
        //else
        //{
        //    flag &= ~0x80;
        //    protect_code[1] &= ~0x80;
        //}

        /* AFE低温保护 */
        if (2 == get_cell_temp_status())
        {
            if (0 == (flag & 0x100))
            {
                flag |= 0x100;
                record_protect = 1;
                protect_code[3] |= 0x10;
            }
        }
        //else
        //{
        //    flag &= ~0x100;
        //    protect_code[3] &= ~0x10;
        //}

        /* 断线保护 */
        if (0 < get_cell_open_status())
        {
            if (0 == (flag & 0x200))
            {
                flag |= 0x200;
                record_protect = 1;
                protect_code[1] |= 0x40;
            }
        }
        //else
        //{
        //    flag &= ~0x200;
        //    protect_code[1] &= ~0x40;
        //}

        /* AFE终极过压 */
        if (get_cell_vol_ovp_status())
        {
            if (0 == (flag & 0x04))
            {
                afeOvStatus = 0;
                flag |= 0x04;
                record_protect = 1;
                protect_code[0] |= 0x80;
            }
#if defined(TIANFENG) && defined(BAT_8S)    /* 为适配天丰生产，增加AFE过压恢复功能 */
            uint16_t afeCellRecVol = get_cell_vol_high_recover();
            uint16_t max_vol = get_max_cell_vol();
            if(max_vol < afeCellRecVol)
            {
                switch(afeOvStatus)
                {
                case 0:
                    afeOvRecDelay = 200; /* 1s */
                    afeOvStatus = 1;
                    break;
                case 1:
                    if(((flag & 0x3F8) == 0))
                    {
                        if(afeOvRecDelay <= 0)
                        {
                            clear_alarm();
                            afeOvStatus = 0;
                            cell_ov_status = 0;
                            flag &= ~0x04;
                            protect_code[0] &= ~0x80; 
                        }
                    }
                    break;
                default:
                    afeOvStatus = 0;
                    break;
                }
            }
            else
            {
                afeOvStatus = 0;
            }
#endif
        }
//        else
//        {
//            flag &= ~0x04;
//            protect_code[0] &= ~0x80; 
//        }
        
        
        /* 0x01  软件终极过压  0x02 软件终极欠压 
           0x04  AFE终极过压  0x08  AFE终极欠压
           0x10  AFE短路保护  0x20  AFE放电过流
           0x40  AFE充电过流  0x80  AFE高温保护
           0x100 AFE低温保护  0x200 断线保护
        */
        final_set_switch(flag);  
    }
}

/*=============================================================
 * 函数名称：load_detect_proc
 * 函数功能：负载移除检测流程
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-22        戴辉发      创建
==============================================================*/
static void load_detect_proc(void)
{
    if ( 0 == get_dch_switch_status() )
    /* 放电MOS断开时启用负载移除检测 */
    {
        switch(out_load_status)
        /* 未设置启用负载检测功能 */
        {
        case 2:
            /* 启用负载检测 */
            set_load_config(1);
            /* 等待判决状态 */
            out_load_status = 0;
            /* 强制设定5秒延时 */
            t_load_detect_timer = ONE_SECOND_TIME * 5;
            break;
        case 0:/* 开始检测负载移除 */
            if (1 == load_status)
            {
                if (0 == t_load_detect_timer)
                {
                    out_load_status = 1;
                    /* 强制设定5秒延时 */
                    t_load_detect_timer = ONE_SECOND_TIME * 5;
                }
            }
            else
            {
                /* 强制设定5秒延时 */
                t_load_detect_timer = ONE_SECOND_TIME * 5;
            }
            break;
        default:/* 开始检测负载接入 */
            if (0 == load_status)
            {
                if (0 == t_load_detect_timer)
                {
                    out_load_status = 0;
                    /* 强制设定5秒延时 */
                    t_load_detect_timer = ONE_SECOND_TIME * 5;
                }
            }
            else
            {
                /* 强制设定5秒延时 */
                t_load_detect_timer = ONE_SECOND_TIME * 5;
            }
            break;
        }
    }
    else
    {
        if (2 != out_load_status)
        /* 关闭负载检测 */
        {
            /* 禁用负载检测 */
            set_load_config(0);
        }
        /* 无法检测状态 */
        out_load_status = 2;
        /* 强制设定2秒延时 */
        t_load_detect_timer = ONE_SECOND_TIME * 2;
    }
}
/*=============================================================
 * 函数名称：chg_detect_proc
 * 函数功能：充电器移除检测流程
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-22        戴辉发      创建
==============================================================*/
static void chg_detect_proc(void)
{
    if (0 == get_ch_switch_status())
    /* 充电MOS断开后才启动充电器接入检测 */
    {
        switch(out_chg_status)
        /* 未设置启用充电机检测功能 */
        {
        case 2:
            /* 启用充电机检测 */
            set_charger_config(1);
            /* 等待判决状态 */
            out_chg_status = 0;
            /* 强制设定5秒延时 */
            t_chg_detect_timer = ONE_SECOND_TIME * 5;
            break;
        case 0: /* 开始检测充电机移除 */
            if (1 == chg_status)
            {
                if (0 == t_chg_detect_timer)
                {
                    out_chg_status = 1;
                    /* 强制设定5秒延时 */
                    t_chg_detect_timer = ONE_SECOND_TIME * 5;
                }
            }
            else
            {
                /* 强制设定5秒延时 */
                t_chg_detect_timer = ONE_SECOND_TIME * 5;
            }
            break;
        default: /* 开始检测充电机接入 */
            if (0 == chg_status)
            {
                if (0 == t_chg_detect_timer)
                {
                    out_chg_status = 0;
                    /* 强制设定5秒延时 */
                    t_chg_detect_timer = ONE_SECOND_TIME * 5;
                }
            }
            else
            {
                /* 强制设定5秒延时 */
                t_chg_detect_timer = ONE_SECOND_TIME * 5;
            }
            break;
        }
    }
    else
    {
        if (2 != out_chg_status)
        /* 关闭充电机检测 */
        {
            /* 禁用充电机检测 */
            set_charger_config(0);
        }
        /* 无法检测状态 */
        out_chg_status = 2;
        /* 强制设定2秒延时 */
        t_chg_detect_timer = ONE_SECOND_TIME * 2;
    }
    
    /* 禁用充电机检测 */

}
/*=============================================================
 * 函数名称：close_load_detect_proc
 * 函数功能：关闭load检测
 * 参数个数：0
 * 参数描述：
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-08-22        li     创建
==============================================================*/
void close_load_detect_proc()
{
    if(2 != out_load_status)
    /* 关闭负载检测 */
    {
        /* 禁用负载检测 */
        set_load_config(0);
    }
    /* 无法检测状态 */
    out_load_status = 2;
}
