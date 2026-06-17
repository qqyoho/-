/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：parameter.h
 * 文件功能描述：实现系统参数管理
 *
 * 修改记录：
 * 2018-06-22 戴辉发   创建
*----------------------------------------------------------*/

#ifndef _SYSTEM_PARAMETER_DEFINE_
#define _SYSTEM_PARAMETER_DEFINE_

#include <stdint.h>

#define MAX_BAT_NUM                         (16)
#define MAX_CELL_TEMP_NUM                   (2)

/* 分流器阻值 */
#define CURRENT_RVALUE                      (1.0 / 4)


#if defined(BAT_8S) /* 福得尔8串磷酸铁锂 */
    #define BAT_NUM                             (8)
#elif defined(BAT_13S) /* 福得尔13串三元锂 */
    #define BAT_NUM                             (13)
#elif defined(BAT_15S) /* 福得尔15串磷酸铁锂 */
    #define BAT_NUM                             (15)
#endif

#if defined(JUNH_COTP)
    #define DETIVE_TIME                         (30)
#elif defined(TIANFENG) && defined(BAT_8S)
    #define DETIVE_TIME                         (30)
#else
    #define DETIVE_TIME                         (30)
#endif

#if defined(CAPCITY_10)
    #define RATE_CAPCITY                        (10)
#elif defined(CAPCITY_12)
    #define RATE_CAPCITY                        (12)
#elif defined(CAPCITY_15)
    #define RATE_CAPCITY                        (15)
#elif defined(CAPCITY_30)
    #define RATE_CAPCITY                        (30)
#elif defined(CAPCITY_36)
    #define RATE_CAPCITY                        (36)
#elif defined(CAPCITY_28)
    #define RATE_CAPCITY                        (28)
#elif defined(CAPCITY_55)
    #define RATE_CAPCITY                        (55)
#elif defined(CAPCITY_60)
    #define RATE_CAPCITY                        (60)
#elif defined(CAPCITY_100)
    #define RATE_CAPCITY                        (100)
#elif defined(CAPCITY_125)
    #define RATE_CAPCITY                        (125)
#endif

#define         AH_VALUE                   (360000L)

/*****************电压警报开关*******************/
#define CELL_VOL_HIGH_ALARM_SWITCH          (1 << 0) /* 单体过压告警开关 */
#define CELL_VOL_HIGH_PROTECT_SWITCH        (1 << 1) /* 单体过压保护开关 */
#define CELL_VOL_LOW_ALARM_SWITCH           (1 << 2) /* 单体欠压告警开关 */
#define CELL_VOL_LOW_PROTECT_SWITCH         (1 << 3) /* 单体欠压保护开关 */
#define VOL_HIGH_ALARM_SWITCH               (1 << 4) /* 总压过压告警开关 */
#define VOL_HIGH_PROTECT_SWITCH             (1 << 5) /* 总压过压保护开关 */
#define VOL_LOW_ALARM_SWITCH                (1 << 6) /* 总压欠压告警开关 */
#define VOL_LOW_PROTECT_SWITCH              (1 << 7) /* 总压欠压保护开关 */

#if 0
#define get_cell_vol_high_alarm_switch()    ((CELL_VOL_HIGH_ALARM_SWITCH & get_voltage_flag())?1:0)
#define get_cell_vol_high_protect_switch()  ((CELL_VOL_HIGH_PROTECT_SWITCH & get_voltage_flag())?1:0)
#define get_cell_vol_low_alarm_switch()     ((CELL_VOL_LOW_ALARM_SWITCH & get_voltage_flag())?1:0)
#define get_cell_vol_low_protect_switch()   ((CELL_VOL_LOW_PROTECT_SWITCH & get_voltage_flag())?1:0)
#define get_vol_high_alarm_switch()         ((VOL_HIGH_ALARM_SWITCH & get_voltage_flag())?1:0)
#define get_vol_high_protect_switch()       ((VOL_HIGH_PROTECT_SWITCH & get_voltage_flag())?1:0)
#define get_vol_low_alarm_switch()          ((VOL_LOW_ALARM_SWITCH & get_voltage_flag())?1:0)
#define get_vol_low_protect_switch()        ((VOL_LOW_PROTECT_SWITCH & get_voltage_flag())?1:0)
#endif

#define get_cell_vol_high_alarm_switch()    1
#define get_cell_vol_high_protect_switch()  1
#define get_cell_vol_low_alarm_switch()     1
#define get_cell_vol_low_protect_switch()   1
#define get_vol_high_alarm_switch()         1
#define get_vol_high_protect_switch()       1
#define get_vol_low_alarm_switch()          1
#define get_vol_low_protect_switch()        1

/*****************电流警报开关*******************/
#define CURRENT_CH_HIGH_ALARM_SWITCH        (1 << 0) /* 充电过流告警开关 */
#define CURRENT_CH_HIGH_PROTECT_SWITCH      (1 << 1) /* 充电过流保护开关 */
#define CURRENT_DCH_HIGH_ALARM_SWITCH       (1 << 2) /* 放电过流告警开关 */
#define CURRENT_DCH_HIGH_PROTECT_SWITCH     (1 << 3) /* 放电过流保护开关 */
#define CURRENT_SECOND_PROTECT_SWITCH       (1 << 4) /* 二次过流保护开关 */
#define CURRENT_SHORT_PROTECT_SWITCH        (1 << 5) /* 短路过流保护开关 */
#define CURRENT_SECOND_LOCKED_SWITCH        (1 << 6) /* 二次过流锁定开关 */
#define CURRENT_SHORT_LOCKED_SWITCH         (1 << 7) /* 短路锁定开关 */

#if 0
#define get_current_ch_alarm_switch()       ((CURRENT_CH_HIGH_ALARM_SWITCH & get_current_flag())?1:0)
#define get_current_ch_protect_switch()     ((CURRENT_CH_HIGH_PROTECT_SWITCH & get_current_flag())?1:0)
#define get_current_dch_alarm_switch()      ((CURRENT_DCH_HIGH_ALARM_SWITCH & get_current_flag())?1:0)
#define get_current_dch_protect_switch()    ((CURRENT_DCH_HIGH_PROTECT_SWITCH & get_current_flag())?1:0)
#define get_current_second_protect_switch() ((CURRENT_SECOND_PROTECT_SWITCH & get_current_flag())?1:0)
#define get_current_short_protect_switch()  ((CURRENT_SHORT_PROTECT_SWITCH & get_current_flag())?1:0)
#define get_current_second_locked_switch()  ((CURRENT_SECOND_LOCKED_SWITCH & get_current_flag())?1:0)
#define get_current_short_locked_switch()   ((CURRENT_SHORT_LOCKED_SWITCH & get_current_flag())?1:0)
#endif

#define get_current_ch_alarm_switch()       1
#define get_current_ch_protect_switch()     1
#define get_current_dch_alarm_switch()      1
#define get_current_dch_protect_switch()    1
#define get_current_second_protect_switch() 1
#define get_current_short_protect_switch()  1
#define get_current_second_locked_switch()  0
#define get_current_short_locked_switch()   0

/*****************温度警报开关*******************/
#define TEMP_ALARM_SWITCH                   (1 << 0) /* 电芯温度告警开关 */
#define TEMP_CH_PROTECT_SWITCH              (1 << 1) /* 电芯温度禁充开关 */
#define TEMP_DCH_PROTECT_SWITCH             (1 << 2) /* 电芯温度禁放开关 */
#define TEMP_ENVIROR_ALARM_SWITCH           (1 << 3) /* 环境温度告警开关 */
#define TEMP_ENVIROR_PROTECT_SWITCH         (1 << 4) /* 环境温度保护开关 */
#define TEMP_HOT_SWITCH                     (1 << 5) /* 环境加热开关 */
#define TEMP_POWER_ALARM_SWITCH             (1 << 6) /* 功率温度告警开关 */
#define TEMP_POWER_PROTECT_SWITCH           (1 << 7) /* 功率温度保护开关 */

#if 0
#define get_cell_temp_alarm_switch()        ((TEMP_ALARM_SWITCH & get_temperature_flag())?1:0)
#define get_cell_ch_protect_switch()        ((TEMP_CH_PROTECT_SWITCH & get_temperature_flag())?1:0)
#define get_cell_dch_protect_switch()       ((TEMP_DCH_PROTECT_SWITCH & get_temperature_flag())?1:0)
#define get_cell_enviror_alarm_switch()     ((TEMP_ENVIROR_ALARM_SWITCH & get_temperature_flag())?1:0)
#define get_cell_enviror_protect_switch()   ((TEMP_ENVIROR_PROTECT_SWITCH & get_temperature_flag())?1:0)
#define get_hot_switch()                    ((TEMP_HOT_SWITCH & get_temperature_flag())?1:0)
#define get_power_alarm_switch()            ((TEMP_POWER_ALARM_SWITCH & get_temperature_flag())?1:0)
#define get_power_protect_switch()          ((TEMP_POWER_PROTECT_SWITCH & get_temperature_flag())?1:0)
#endif

#define get_cell_temp_alarm_switch()        1
#define get_cell_ch_protect_switch()        1
#define get_cell_dch_protect_switch()       1
#define get_enviror_alarm_switch()          1
#define get_enviror_protect_switch()        1
#define get_hot_switch()                    0
#define get_power_alarm_switch()            1
#define get_power_protect_switch()          1

/*****************容量警报开关*******************/
#define CAPCITY_ALARM_SWITCH                (1 << 0) /* 剩余容量告警开关 */
#define CAPCITY_PROTECT_SWITCH              (1 << 2) /* 剩余容量保护开关 */
#define CAPCITY_SMART_SWITCH                (1 << 3) /* 只能补电开关 */

#define get_capcity_alarm_switch()          ((CAPCITY_ALARM_SWITCH & get_capcity_flag())?1:0)
#define get_capcity_protect_switch()        ((CAPCITY_PROTECT_SWITCH & get_capcity_flag())?1:0)
#define get_capcity_smart_switch()          ((CAPCITY_SMART_SWITCH & get_capcity_flag())?1:0)

/*****************均衡功能开关*******************/
#define BALANCE_FUNC_SWITCH                 (1 << 0) /* 均衡功能开关 */
#define BALANCE_STATIC_FUNC_SWITCH          (1 << 1) /* 静态均衡功能开关 */
#define BALANCE_TIMER_SWITCH                (1 << 2) /* 均衡定时功能开关 */
#define BALANCE_TEMPERATURE_SWITCH          (1 << 3) /* 均衡温度判决开关 */
#define BALANCE_CELL_FAIL_SWITCH            (1 << 4) /* 电芯失效开关 */

#if 0
#define get_balance_func_switch()           ((BALANCE_FUNC_SWITCH & get_balance_flag())?1:0)
#define get_balance_static_func_switch()    ((BALANCE_STATIC_FUNC_SWITCH & get_balance_flag())?1:0)
#define get_balance_timer_switch()          ((BALANCE_TIMER_SWITCH & get_balance_flag())?1:0)
#define get_balance_temp_switch()           ((BALANCE_TEMPERATURE_SWITCH & get_balance_flag())?1:0)
#define get_cell_fail_switch()              ((BALANCE_CELL_FAIL_SWITCH & get_balance_flag())?1:0)
#endif

#define get_balance_func_switch()           1
#define get_balance_static_func_switch()    0
#define get_balance_timer_switch()          1
#define get_balance_temp_switch()           1
#define get_cell_fail_switch()              1

/*****************UI接口功能开关*****************/
#define SHOW_FUNC_SWITCH                    (1 << 0) /* 显示功能开关 */
#define BEEP_FUNC_SWITCH                    (1 << 1) /* 声音功能开关 */
#define DRYCONTACT1_FUNC_SWITCH             (1 << 2) /* 干结点1功能开关 */
#define DRYCONTACT2_FUNC_SWITCH             (1 << 3) /* 干结点2功能开关 */
#define DRYCONTACT3_FUNC_SWITCH             (1 << 4) /* 干结点3功能开关 */
#define DRYCONTACT4_FUNC_SWITCH             (1 << 5) /* 干结点4功能开关 */
#define DRYCONTACT5_FUNC_SWITCH             (1 << 6) /* 干结点5功能开关 */
#define DRYCONTACT6_FUNC_SWITCH             (1 << 7) /* 干结点6功能开关 */

#define get_show_func_switch()              ((SHOW_FUNC_SWITCH & get_show_flag())?1:0)
#define get_beep_func_switch()              ((BEEP_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact1_func_switch()       ((DRYCONTACT1_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact2_func_switch()       ((DRYCONTACT2_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact3_func_switch()       ((DRYCONTACT3_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact4_func_switch()       ((DRYCONTACT4_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact5_func_switch()       ((DRYCONTACT5_FUNC_SWITCH & get_show_flag())?1:0)
#define get_drycontact6_func_switch()       ((DRYCONTACT6_FUNC_SWITCH & get_show_flag())?1:0)

/***************外部开关功能开关*****************/
#define OUTSIDE_FUNC_SWITCH                 (1 << 0) /* 外部开关功能开关 */

#define get_outside_func_switch()           ((OUTSIDE_FUNC_SWITCH & get_show_flag())?1:0)

#define RVSD_BUFFER_LENGTH            (4)

/* 单个参数结构体 */
typedef struct _T_ALARM_ITEM_STRUCT_
{
	short alarm; /* 告警门限值 */
	short alarm_recover; /* 告警恢复门限值 */
	short protect; /* 保护门限值 */
	short recover; /* 退出保护恢复门限值 */
}t_alarm_item_st;

/* 电压告警参数结构体 */
typedef struct _VOLTAGE_ITEM_STRUCT_
{
	t_alarm_item_st low; /* 欠压告警参数 */
	t_alarm_item_st high; /* 过压告警参数 */
}t_voltage_item_st;

/* 电压告警参数结构体 */
typedef struct _T_VOLTAGE_STRUCT_
{
	t_voltage_item_st cell_vol; /* 单体电压参数 */
	t_voltage_item_st voltage; /* 总压参数 */
}t_voltage_st;

/* 单个参数结构体 */
typedef struct _T_ITEM_STRUCT_
{
	short alarm; /* 告警门限值 */
	short protect; /* 保护门限值 */
	short recover; /* 退出保护恢复门限值 */
}t_item_st;

/* 温度告警参数结构体 */
typedef struct _T_TEMP_ITEM_STRUCT_
{
	t_item_st low; /* 低温告警参数 */
	t_item_st high; /* 高温告警参数 */
}t_temp_item_st;

/* 电芯温度告警 */
typedef struct _T_CELL_TEMP_STRUCT_
{
  t_temp_item_st ch_temp; /* 充电温度参数 */
  t_temp_item_st dch_temp; /* 放电温度参数 */
  uint16_t protect_delay; /* 保护延时 */
}t_cell_temp_st;

/* 功率温度参数结构体 */
typedef struct _T_POWER_TEMP_STRUCT_
{
  t_temp_item_st temp; /* 温度参数 */
  uint16_t protect_delay; /* 保护延时 */
}t_power_temp_st;

/* 环境温度告警参数结构体 */
typedef t_power_temp_st t_enviror_temp_st;

/* 充电过流告警参数结构体 */
typedef struct _T_CH_CURRENT_STRUCT_
{
	short in_level; /* 充电电流进入门限 */
	short alarm; /* 过流告警门限 */
	short protect; /* 过流保护门限值 */
	uint16_t protect_delay; /* 确认保护状态延时,以s为单位 */
	short limit; /* 充电限流值 */
}t_ch_current_st;

/* 放电过流告警参数结构体 */
typedef struct _T_DCH_CURRENT_STRUCT_
{
	short in_level; /* 放电电流进入门限 */
	short alarm; /* 过流告警门限 */
	short protect; /* 过流保护门限值 */
	short second_protect; /* 二级过流保护门限值 */
	short short_protect; /* 短路过流保护门限值 */
	uint16_t protect_delay; /* 确认保护状态延时,以s为单位 */
	uint16_t second_delay; /* 确认二级过流状态延时,以s为单位 */
	uint16_t short_delay; /* 短路保护状态下维持的时间, 以us为单位 */
	uint16_t short_recover_delay; /* 短路恢复延时 */
	uint16_t protect_recover_delay; /* 过流保护恢复延迟, 以s为单位 */
	uint16_t locked_num; /* 过流锁定次数 */
}t_dch_current_st;

/* 电池加热参数结构体 */
typedef struct _T_HOT_STRUCT_
{
	short start; /* 加热起始温度值 */
	short end; /* 加热停止温度值 */
}t_hot_st;

/* 均衡参数结构 */
typedef struct _T_BALANCE_STRUCT_
{
	uint16_t fail_level; /* 电芯失效压差 */
	uint16_t recover_level; /* 电芯失效恢复压差 */
	short end_temp; /* 电芯均衡工作环境结束温度值 */
	short start_temp; /* 电芯均衡工作环境开始温度值 */
	uint16_t start; /* 开始电压 */
	uint16_t diff_start; /* 起始压差 */
	uint16_t diff_end; /* 结束压差 */
	uint16_t delay; /* 均衡时间, 以小时为单位 */
}t_balance_st;

/* 智能补电参数结构 */
typedef struct _T_SMART_CH_STRUCT_
{
	uint8_t soc_level; /* 智能补电门限 */
	uint8_t delay; /* 定时补电定时，以小时为单位 */
}t_smart_st;

/* 剩余容量告警参数结构 */
typedef struct _T_CAPCITY_STRUCT_
{
	uint8_t low; /* 一级剩余容量告警门限 */
	uint8_t protect; /* 剩余容量保护门限 */
	uint8_t high; /* 高容量门限 */
}t_capcity_st;

/* 电压参数 */
typedef struct _T_VOLTAGE_PARA_STRUCT_
{
	t_voltage_st vol_para;
	uint16_t pretect_delay; /* 保护延时 */
	uint16_t dis_charge_level; /* 单体电压低压禁充门限 */
	uint8_t voltage_flag; /* 电压警报开关 */
	uint8_t end_id;
}t_vol_para_st;
extern t_vol_para_st g_vol_para; /* 电压参数 */

/* 电流参数 */
typedef struct _T_CURRENT_PARA_STRUCT_
{
	t_ch_current_st ch_current; /* 充电电流告警参数 */
	t_dch_current_st dch_current; /* 放电电流告警参数 */
	uint8_t current_flag; /* 电流警报开关 */
	uint8_t end_id;
}t_curr_para_st;
extern t_curr_para_st g_curr_para;

/* 温度参数 */
typedef struct _T_TEMPERATURE_PARA_STRUCT_
{
	t_cell_temp_st cell_temp; /* 电芯温度告警参数 */
	t_enviror_temp_st enviror_temp; /* 环境温度告警参数 */
	t_power_temp_st power_temp; /* 功率温度告警参数 */
	uint8_t temp_flag; /* 温度警报开关 */
	uint8_t end_id;
}t_temp_para_st;
extern t_temp_para_st g_temp_para;

/* 加热和均衡参数 */
typedef struct _T_HOT_BALANCE_PARA_STRUCT_
{
	t_hot_st hot; /* 加热参数 */
	t_balance_st balance; /* 均衡参数 */
	uint8_t balance_flag; /* 均衡功能开关 */
	uint8_t end_id;
}t_hot_balance_para_st;
extern t_hot_balance_para_st g_hot_balance_para;

/* 容量参数 */
typedef struct _T_CAPCITY_PARA_STRUCT_
{
	uint16_t rated_capcity; /* 额定容量,以0.1安时为单位 */
	uint16_t low_power_delay; /* 低功耗延时，以分钟为单位 */
	uint16_t deactive_delay; /* 不活动延时，以分钟为单位 */
	uint16_t low_deactive_delay; /* 低电量不活动延时，以分钟为单位 */
	t_smart_st smart; /* 智能补电参数 */
	t_capcity_st capcity;
	uint8_t capcity_flag; /* 容量警报开关 */
	uint8_t end_id;
}t_capcity_para_st;
extern t_capcity_para_st g_capcity_para;

/* 系统参数结构体 */
typedef struct _OTHER_PARA_STRUCT_
{
	uint8_t cell_num; /* 电池节数 */
	uint8_t show_flag; /* 显示开关 */
	uint8_t outside_flag; /* 外部开关 */
	uint8_t rvsd_buf[RVSD_BUFFER_LENGTH];
	uint8_t end_id;
}t_other_para_st;
extern t_other_para_st g_other_para;

/* 实时数据 */
typedef struct _SYSTEM_DATA_STRUCT_
{
	uint16_t total_vol; /* 当前总压 */
	short current; /* 当前工作电流 */
	uint16_t soc; /* 当前SOC */
	uint16_t cell_vol[MAX_BAT_NUM]; /* 电芯电压 */
	short power_temp; /* 功率温度 */
	short ambient_temp; /* 环境温度 */
	short cell_temp[MAX_CELL_TEMP_NUM]; /* 电芯温度 */
	uint8_t vol_event; /* 电压事件 */
	uint8_t cur_event; /* 电流事件 */
	uint8_t temp_event; /* 温度事件 */
	uint8_t ba_event; /* 均衡事件 */
	uint8_t cap_event; /* 容量事件 */
	uint8_t other_event; /* 其他事件 */
    uint8_t new_event1; /* 新增事件1 */
    uint8_t new_event2; /* 新增事件2 */
    uint8_t write_time[7]; /* 写入事件时时间 */ //61byte
}t_system_data;
extern t_system_data g_run_sys_data;
extern uint8_t protect_code[8];

typedef struct
{
  uint8_t RTC_Hours;    /*!< Specifies the RTC Time Hour.
                        This parameter must be set to a value in the 0-12 range
                        if the RTC_HourFormat_12 is selected or 0-23 range if
                        the RTC_HourFormat_24 is selected. */

  uint8_t RTC_Minutes;  /*!< Specifies the RTC Time Minutes.
                        This parameter must be set to a value in the 0-59 range. */
  
  uint8_t RTC_Seconds;  /*!< Specifies the RTC Time Seconds.
                        This parameter must be set to a value in the 0-59 range. */

  uint8_t RTC_Month;   /*!< Specifies the RTC Date Month.
                        This parameter can be a value of @ref RTC_Month_Date_Definitions */

  uint8_t RTC_Date;     /*!< Specifies the RTC Date.
                        This parameter must be set to a value in the 1-31 range. */
  
  uint16_t RTC_Year;     /*!< Specifies the RTC Date Year.
                        This parameter must be set to a value in the 0-99 range. */
}RTC_DateTimeTypeDef; 



/*=============================================================
* 函数名称：system_parameter_mem_init
* 函数功能：系统参数内存初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void system_parameter_mem_init(void);

/*==============================================================
* 函数名称：save_vol_para_parameter
* 函数功能：保存电压参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_vol_para_parameter(void);

/*==============================================================
* 函数名称：save_curr_para_parameter
* 函数功能：保存电流参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_curr_para_parameter(void);

/*==============================================================
* 函数名称：save_temp_para_parameter
* 函数功能：保存温度参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_temp_para_parameter(void);

/*==============================================================
* 函数名称：save_balance_para_parameter
* 函数功能：保存均衡参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_balance_para_parameter(void);

/*==============================================================
* 函数名称：save_capcity_para_parameter
* 函数功能：保存容量参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_capcity_para_parameter(void);

/*==============================================================
* 函数名称：save_other_para_parameter
* 函数功能：保存容量参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void save_other_para_parameter(void);

/*=============================================================
* 函数名称：system_para_default
* 函数功能：设定系统参数为缺省值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-11-17         戴辉发             创建
==============================================================*/
void system_para_default(void);

/*=============================================================
* 函数名称：system_parameter_hard_init
* 函数功能：系统参数内存读取存储区后初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void system_parameter_hard_init(void);

/*=============================================================
* 函数名称：set_cell_vol_high_alarm_level
* 函数功能：设置单体过压告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_alarm_level(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_cell_vol_high_alarm_level
* 函数功能：获取单体过压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_alarm_level(void);

/*=============================================================
* 函数名称：set_cell_vol_high_alarm_recover
* 函数功能：设置单体过压告警恢复门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_alarm_recover(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_cell_vol_high_alarm_recover
* 函数功能：获取单体过压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压告警恢复门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_alarm_recover(void);

/*=============================================================
* 函数名称：set_cell_vol_high_protect
* 函数功能：设置单体过压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_protect(uint16_t protect);

/*=============================================================
* 函数名称：get_cell_vol_high_protect
* 函数功能：获取单体过压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_protect(void);

/*=============================================================
* 函数名称：set_cell_vol_high_recover
* 函数功能：设置单体过压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_high_recover(uint16_t level);

/*=============================================================
* 函数名称：get_cell_vol_high_recover
* 函数功能：获取单体过压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体过压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_high_recover(void);

/*=============================================================
* 函数名称：set_cell_vol_low_alarm_level
* 函数功能：设置单体欠压告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_alarm_level(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_cell_vol_low_alarm_level
* 函数功能：获取单体欠压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_alarm_level(void);

/*=============================================================
* 函数名称：set_cell_vol_low_alarm_recover
* 函数功能：设置单体欠压告警恢复门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_alarm_recover(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_cell_vol_low_alarm_level
* 函数功能：获取单体欠压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压告警恢复门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_alarm_recover(void);

/*=============================================================
* 函数名称：set_cell_vol_low_protect
* 函数功能：设置单体欠压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_protect(uint16_t protect);

/*=============================================================
* 函数名称：get_cell_vol_low_protect
* 函数功能：获取单体欠压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_protect(void);

/*=============================================================
* 函数名称：set_cell_vol_low_recover
* 函数功能：设置单体欠压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_vol_low_recover(uint16_t level);

/*=============================================================
* 函数名称：get_cell_vol_low_recover
* 函数功能：获取单体欠压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回单体欠压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_vol_low_recover(void);

/*=============================================================
* 函数名称：set_vol_high_alarm_level
* 函数功能：设置总压过压告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_alarm_level(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_vol_high_alarm_level
* 函数功能：获取总压过压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_alarm_level(void);

/*=============================================================
* 函数名称：set_vol_high_alarm_recover
* 函数功能：设置总压过压告警恢复门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_alarm_recover(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_vol_high_alarm_recover
* 函数功能：获取总压过压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压告警恢复门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_alarm_recover(void);

/*=============================================================
* 函数名称：set_vol_high_protect
* 函数功能：设置总压过压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_protect(uint16_t protect);

/*=============================================================
* 函数名称：get_vol_high_protect
* 函数功能：获取总压过压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_protect(void);

/*=============================================================
* 函数名称：set_vol_high_recover
* 函数功能：设置总压过压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_high_recover(uint16_t level);

/*=============================================================
* 函数名称：get_vol_high_recover
* 函数功能：获取总压过压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压过压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_high_recover(void);

/*=============================================================
* 函数名称：set_vol_low_alarm_level
* 函数功能：设置总压欠压告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_alarm_level(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_vol_low_alarm_level
* 函数功能：获取总压欠压告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_alarm_level(void);

/*=============================================================
* 函数名称：set_vol_low_alarm_recover
* 函数功能：设置总压欠压告警恢复门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_alarm_recover(uint16_t alarm_level);

/*=============================================================
* 函数名称：get_vol_low_alarm_recover
* 函数功能：获取总压欠压告警恢复门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压告警恢复门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_alarm_recover(void);

/*=============================================================
* 函数名称：set_vol_low_protect
* 函数功能：设置总压欠压保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_protect(uint16_t protect);

/*=============================================================
* 函数名称：get_vol_low_protect
* 函数功能：获取总压欠压保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_protect(void);

/*=============================================================
* 函数名称：set_vol_low_recover
* 函数功能：设置总压欠压保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_low_recover(uint16_t level);

/*=============================================================
* 函数名称：get_vol_low_protect_recover
* 函数功能：获取总压欠压保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回总压欠压保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_low_recover(void);

/*=============================================================
* 函数名称：set_vol_protect_dealy
* 函数功能：设置电压保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_protect_delay(unsigned int delay);

/*=============================================================
* 函数名称：get_vol_protect_delay
* 函数功能：获取电压保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电压保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_vol_protect_delay(void);

/*=============================================================
* 函数名称：set_vol_dis_charge_level
* 函数功能：设置单体电压低压禁充门限
* 参数个数：1
* 函数参数：
*           [IN]      level              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_vol_dis_charge_level(uint16_t level);

/*=============================================================
* 函数名称：get_vol_dis_charge_level
* 函数功能：获取单体电压低压禁充门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           单体电压低压禁充门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-12          戴辉发             创建
==============================================================*/
uint16_t get_vol_dis_charge_level(void);

/*=============================================================
* 函数名称：set_ch_current_alarm_level
* 函数功能：设置充电过流告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_alarm_level(short alarm_level);

/*=============================================================
* 函数名称：get_ch_current_alarm_level
* 函数功能：获取充电过流告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_alarm_level(void);

/*=============================================================
* 函数名称：set_ch_current_in_level
* 函数功能：设置充电电流进入门限
* 参数个数：1
*           [IN]      level              门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_in_level(short level);

/*=============================================================
* 函数名称：get_ch_current_in_level
* 函数功能：获取充电电流进入门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电电流进入门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_in_level(void);

/*=============================================================
* 函数名称：set_ch_current_protect
* 函数功能：设置充电过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_protect(short level);

/*=============================================================
* 函数名称：get_ch_current_protect
* 函数功能：获取充电过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_protect(void);

/*=============================================================
* 函数名称：set_ch_current_protect_delay
* 函数功能：设置充电过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      level              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_protect_delay(uint16_t level);

/*=============================================================
* 函数名称：get_ch_current_protect_delay
* 函数功能：获取充电过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电过流保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_ch_current_protect_delay(void);

/*=============================================================
* 函数名称：set_ch_current_limit
* 函数功能：设置充电限流值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_current_limit(short level);

/*=============================================================
* 函数名称：get_ch_current_limit
* 函数功能：获取充电限流值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电限流值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_current_limit(void);

/*=============================================================
* 函数名称：set_dch_current_in_level
* 函数功能：设置放电电流进入门限
* 参数个数：1
*           [IN]      in_level           门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_in_level(short level);

/*=============================================================
* 函数名称：get_dch_current_in_level
* 函数功能：获取放电电流进入门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电电流进入门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_in_level(void);

/*==============================================================
* 函数名称：set_dch_current_locked_num
* 函数功能：设置放电过流锁定次数
* 参数个数：1
* 函数参数：
*           [IN]      value              放电过流锁定次数
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_locked_num(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_locked_num
* 函数功能：获取放电过流锁定次数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流锁定次数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_locked_num(void);

/*=============================================================
* 函数名称：set_dch_current_alarm_level
* 函数功能：设置放电过流告警门限
* 参数个数：1
*           [IN]      level              门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_high_level(short level);

/*=============================================================
* 函数名称：get_dch_current_alarm_level
* 函数功能：获取放电过流告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_alarm_level(void);

/*==============================================================
* 函数名称：set_dch_current_protect
* 函数功能：设置放电过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect(short level);

/*==============================================================
* 函数名称：get_dch_current_protect
* 函数功能：获取放电过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_protect(void);

/*==============================================================
* 函数名称：set_dch_current_protect_delay
* 函数功能：设置放电过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              过流保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect_delay(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_protect_delay
* 函数功能：获取放电过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电过流保护延时
* 修改记录：
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_protect_delay(void);

/*==============================================================
* 函数名称：set_dch_current_second_protect
* 函数功能：设置放电二次过流保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_second_protect(short protect);

/*==============================================================
* 函数名称：get_dch_current_second_protect
* 函数功能：获取放电二次过流保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电二次过流保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_second_protect(void);

/*==============================================================
* 函数名称：set_dch_current_second_delay
* 函数功能：设置放电二次过流保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              放电过流保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_second_delay(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_second_delay
* 函数功能：获取放电二次过流保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电二次过流保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_second_delay(void);

/*==============================================================
* 函数名称：set_dch_current_short_protect
* 函数功能：设置短路保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_protect(short protect);

/*==============================================================
* 函数名称：get_dch_current_short_protect
* 函数功能：获取短路保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_current_short_protect(void);

/*==============================================================
* 函数名称：set_dch_current_short_delay
* 函数功能：设置短路保护延时
* 参数个数：1
* 函数参数：
*           [IN]      value              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_delay(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_short_delay
* 函数功能：获取短路保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_short_delay(void);

/*==============================================================
* 函数名称：set_dch_current_short_recover_delay
* 函数功能：设置短路保护恢复延时
* 参数个数：1
* 函数参数：
*           [IN]      value              恢复延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_short_recover_delay(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_short_recover_delay
* 函数功能：获取短路保护恢复延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回短路保护恢复延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_short_recover_delay(void);

/*==============================================================
* 函数名称：set_dch_current_protect_recover_delay
* 函数功能：设置过流保护恢复延时
* 参数个数：1
* 函数参数：
*           [IN]      value              恢复延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_current_protect_recover_delay(uint16_t value);

/*==============================================================
* 函数名称：get_dch_current_protect_recover_delay
* 函数功能：获取过流保护恢复延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回过流保护恢复延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_dch_current_protect_recover_delay(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_high_alarm_level
* 函数功能：设置充电高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_alarm_level(short alarm_level);

/*=============================================================
* 函数名称：get_ch_cell_temp_high_alarm_level
* 函数功能：获取充电高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_high_alarm_level(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_high_protect
* 函数功能：设置高温禁充门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_protect(short protect);

/*=============================================================
* 函数名称：get_ch_cell_temp_high_protect
* 函数功能：获取高温禁充门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温禁充门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_high_protect(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_high_recover
* 函数功能：设置高温充释门限值
* 参数个数：1
* 函数参数：
*           [IN]      levle              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_high_recover(short levle);

/*=============================================================
* 函数名称：get_ch_cell_temp_high_recover
* 函数功能：获取高温充释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温充释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_ch_cell_temp_high_recover(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_low_alarm_level
* 函数功能：设置充电低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_alarm_level(short alarm_level);

/*=============================================================
* 函数名称：get_ch_cell_temp_low_alarm_level
* 函数功能：获取充电低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回充电低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_alarm_level(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_low_protect
* 函数功能：设置低温禁充门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_protect(short protect);

/*=============================================================
* 函数名称：get_ch_cell_temp_low_protect
* 函数功能：获取低温禁充门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温禁充门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_protect(void);

/*=============================================================
* 函数名称：set_ch_cell_temp_low_recover
* 函数功能：设置低温充释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_ch_cell_temp_low_recover(short level);

/*=============================================================
* 函数名称：get_ch_cell_temp_low_recover
* 函数功能：获取低温充释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温充释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_ch_cell_temp_low_recover(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_high_alarm_level
* 函数功能：设置放电高温告警门限
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_alarm_level(short level);

/*=============================================================
* 函数名称：get_dch_cell_temp_high_alarm_level
* 函数功能：获取放电高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_alarm_level(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_high_protect
* 函数功能：设置高温禁放门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_protect(short protect);

/*=============================================================
* 函数名称：get_dch_cell_temp_high_protect
* 函数功能：获取高温禁放门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温禁放门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_protect(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_high_recover
* 函数功能：设置高温放释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_high_recover(short level);

/*=============================================================
* 函数名称：get_dch_cell_temp_high_recover
* 函数功能：获取高温放释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回高温放释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_high_recover(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_low_alarm_level
* 函数功能：设置放电低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      level              门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_alarm_level(short level);

/*=============================================================
* 函数名称：get_dch_cell_temp_low_alarm_level
* 函数功能：获取放电低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回放电低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_alarm_level(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_low_protect
* 函数功能：设置低温禁放门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_protect(short level);

/*=============================================================
* 函数名称：get_dch_cell_temp_low_protect
* 函数功能：获取低温禁放门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温禁放门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_protect(void);

/*=============================================================
* 函数名称：set_dch_cell_temp_low_recover
* 函数功能：设置低温放释门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_dch_cell_temp_low_recover(short level);

/*=============================================================
* 函数名称：get_dch_cell_temp_low_recover
* 函数功能：获取低温放释门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低温放释门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_dch_cell_temp_low_recover(void);

/*=============================================================
* 函数名称：set_cell_tempprotect_delay
* 函数功能：设置电芯温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_tempprotect_delay(uint16_t delay);

/*=============================================================
* 函数名称：get_cell_tempprotect_delay
* 函数功能：获取电芯温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_tempprotect_delay(void);

/*=============================================================
* 函数名称：set_power_temp_high_alarm_level
* 函数功能：设置功率高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_alarm_level(short alarm_level);

/*=============================================================
* 函数名称：get_power_temp_high_alarm_level
* 函数功能：获取功率高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_alarm_level(void);

/*=============================================================
* 函数名称：set_power_temp_high_protect
* 函数功能：设置功率高温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_protect(short protect);

/*=============================================================
* 函数名称：get_power_temp_high_protect
* 函数功能：获取功率高温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_protect(void);

/*=============================================================
* 函数名称：set_power_temp_high_recover
* 函数功能：设置功率高温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_high_recover(short level);

/*=============================================================
* 函数名称：get_power_temp_high_recover
* 函数功能：获取功率高温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率高温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_high_recover(void);

/*=============================================================
* 函数名称：set_power_temp_low_alarm_level
* 函数功能：设置功率低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_alarm_level(short alarm_level);

/*=============================================================
* 函数名称：get_power_temp_low_alarm_level
* 函数功能：获取功率低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_alarm_level(void);

/*==============================================================
* 函数名称：set_power_temp_low_protect
* 函数功能：设置功率低温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_protect(short protect);

/*==============================================================
* 函数名称：get_power_temp_low_protect
* 函数功能：获取功率低温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_protect(void);

/*==============================================================
* 函数名称：set_power_temp_low_recover
* 函数功能：设置功率低温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_low_recover(short level);

/*==============================================================
* 函数名称：get_power_temp_low_recover
* 函数功能：获取功率低温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率低温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_power_temp_low_recover(void);

/*==============================================================
* 函数名称：set_power_temp_protect_delay
* 函数功能：设置功率温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_power_temp_protect_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_power_temp_protect_delay
* 函数功能：获取功率温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回功率温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_power_temp_protect_delay(void);

/*==============================================================
* 函数名称：set_enviror_temperature_high_alarm_level
* 函数功能：设置环境高温告警门限
* 参数个数：1
*           [IN]      alarm_level        门限值
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_alarm_level(short alarm_level);

/*==============================================================
* 函数名称：get_enviror_temp_high_alarm_level
* 函数功能：获取环境高温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_alarm_level(void);

/*==============================================================
* 函数名称：set_enviror_temp_high_protect
* 函数功能：设置环境高温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      protect            门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_protect(short protect);

/*==============================================================
* 函数名称：get_enviror_temp_high_protect
* 函数功能：获取环境高温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_protect(void);

/*==============================================================
* 函数名称：set_enviror_temp_high_recover
* 函数功能：设置环境高温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_high_recover(short level);

/*==============================================================
* 函数名称：get_enviror_temp_high_recover
* 函数功能：获取环境高温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境高温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_high_recover(void);

/*==============================================================
* 函数名称：set_enviror_temp_low_alarm_level
* 函数功能：设置环境低温告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           [IN]      alarm_level        门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_alarm_level(short alarm_level);

/*==============================================================
* 函数名称：get_enviror_temp_low_alarm_level
* 函数功能：获取环境低温告警门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温告警门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_alarm_level(void);

/*==============================================================
* 函数名称：set_enviror_temp_low_protect
* 函数功能：设置环境低温保护门限值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_protect(short level);

/*==============================================================
* 函数名称：get_enviror_temp_low_protect
* 函数功能：获取环境低温保护门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温保护门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_protect(void);

/*==============================================================
* 函数名称：set_enviror_temp_low_recover
* 函数功能：设置环境低温保护恢复值
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_recover(short level);

/*==============================================================
* 函数名称：get_enviror_temp_low_recover
* 函数功能：获取环境低温保护恢复值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境低温保护恢复值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_enviror_temp_low_recover(void);

/*==============================================================
* 函数名称：set_enviror_temp_protect_delay
* 函数功能：设置环境温度保护延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              保护延时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_enviror_temp_low_protect_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_enviror_temp_protect_delay
* 函数功能：获取环境温度保护延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回环境温度保护延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_enviror_temp_protect_delay(void);

/*==============================================================
* 函数名称：set_hot_start_temp
* 函数功能：设置加热起始温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              加热起始温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_hot_start_temp(short value);

/*==============================================================
* 函数名称：get_hot_start_temp
* 函数功能：获取加热起始温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回加热起始温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_hot_start_temp(void);

/*==============================================================
* 函数名称：set_hot_end_temp
* 函数功能：设置加热结束温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              加热结束温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_hot_end_temp(short value);

/*==============================================================
* 函数名称：get_hot_end_temp
* 函数功能：获取加热结束温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回加热结束温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_hot_end_temp(void);

/*==============================================================
* 函数名称：set_balance_start_temp
* 函数功能：设置均衡工作最低温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡工作最低温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_temp(short value);

/*==============================================================
* 函数名称：get_balance_start_temp
* 函数功能：获取均衡工作最低温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡工作最低温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_balance_start_temp(void);

/*==============================================================
* 函数名称：set_balance_end_temp
* 函数功能：设置均衡工作最高温度值
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡工作最高温度值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_end_temp(short value);

/*==============================================================
* 函数名称：get_balance_end_temp
* 函数功能：获取均衡工作最高温度值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡工作最高温度值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
short get_balance_end_temp(void);

/*==============================================================
* 函数名称：set_balance_start_difference
* 函数功能：设置均衡起始压差
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡起始压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_difference(uint16_t value);

/*==============================================================
* 函数名称：get_balance_start_difference
* 函数功能：获取均衡起始压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡起始压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_start_difference(void);

/*==============================================================
* 函数名称：set_balance_end_difference
* 函数功能：设置均衡结束压差
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡结束压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_end_difference(uint16_t value);

/*==============================================================
* 函数名称：get_balance_end_difference
* 函数功能：获取均衡结束压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡结束压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_end_difference(void);

/*==============================================================
* 函数名称：set_balance_start_voltage
* 函数功能：设置均衡起始电压
* 参数个数：1
* 函数参数：
*           [IN]      value              均衡起始电压
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_start_voltage(uint16_t value);

/*==============================================================
* 函数名称：get_balance_start_voltage
* 函数功能：获取均衡起始电压
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡起始电压
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_balance_start_voltage(void);

/*==============================================================
* 函数名称：set_cell_fail
* 函数功能：设置电芯失效门限值
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_fail(uint16_t value);

/*==============================================================
* 函数名称：get_cell_fail
* 函数功能：获取电芯失效门限值
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯失效门限值
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_fail(void);

/*==============================================================
* 函数名称：set_cell_recover
* 函数功能：设置电芯失效恢复压差
* 参数个数：1
* 函数参数：
*           [IN]      value              电芯失效恢复压差
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_cell_recover(uint16_t value);

/*==============================================================
* 函数名称：get_cell_recover
* 函数功能：获取电芯失效恢复压差
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回电芯失效恢复压差
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_cell_recover(void);

/*==============================================================
* 函数名称：set_balance_delay
* 函数功能：设置均衡时间限时
* 参数个数：1
* 函数参数：
*           [IN]      delay              均衡时间限时
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_balance_delay
* 函数功能：获取均衡时间限时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡时间限时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
unsigned int get_balance_delay(void);

/*==============================================================
* 函数名称：set_smart_soc_level
* 函数功能：设置智能补电门限
* 参数个数：1
* 函数参数：
*           [IN]      level              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_smart_soc_level(uint8_t level);

/*==============================================================
* 函数名称：get_smart_soc_level
* 函数功能：获取智能补电门限
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回智能补电门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_smart_soc_level(void);

/*==============================================================
* 函数名称：set_smart_delay
* 函数功能：设置智能补电周期
* 参数个数：1
* 函数参数：
*           [IN]      value              智能补电周期
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_smart_delay(uint8_t value);

/*==============================================================
* 函数名称：get_smart_delay
* 函数功能：获取智能补电周期
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回智能补电周期
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_smart_delay(void);

/*==============================================================
* 函数名称：set_low_capcity
* 函数功能：设置一级容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_low_capcity(uint8_t value);

/*==============================================================
* 函数名称：get_low_capcity
* 函数功能：获取一级容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           一级容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_low_capcity(void);

/*==============================================================
* 函数名称：set_protect_capcity
* 函数功能：设置保护容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_protect_capcity(uint8_t value);

/*==============================================================
* 函数名称：get_protect_capcity
* 函数功能：获取保护容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           保护容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_protect_capcity(void);

/*==============================================================
* 函数名称：set_high_capcity
* 函数功能：设置高容量告警门限
* 参数个数：1
* 函数参数：
*           [IN]      value              门限值
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_high_capcity(uint8_t value);

/*==============================================================
* 函数名称：get_high_capcity
* 函数功能：获取高容量告警门限
* 参数个数：1
* 函数参数：
* 返 回 值：
*           高容量告警门限
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_high_capcity(void);

/*==============================================================
* 函数名称：set_rated_capcity
* 函数功能：设置额定容量
* 参数个数：1
* 函数参数：
*           [IN]      value              额定容量
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_rated_capcity(uint16_t value);

/*==============================================================
* 函数名称：get_rated_capcity
* 函数功能：获取额定容量
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回额定容量
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint16_t get_rated_capcity(void);

/*==============================================================
* 函数名称：set_low_power_delay
* 函数功能：设置低功耗延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
void set_low_power_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_low_power_delay
* 函数功能：获取低功耗延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低功耗延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
uint16_t get_low_power_delay(void);
                                   
/*==============================================================
* 函数名称：set_deactive_delay
* 函数功能：设置不活动延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
void set_deactive_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_deactive_delay
* 函数功能：获取不活动延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回不活动延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-24          戴辉发             创建
==============================================================*/
uint16_t get_deactive_delay(void);

/*==============================================================
* 函数名称：set_low_deactive_delay
* 函数功能：设置低功耗不活动延时
* 参数个数：1
* 函数参数：
*           [IN]      delay              延时时间，以分钟为单位
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
void set_low_deactive_delay(uint16_t delay);

/*==============================================================
* 函数名称：get_low_deactive_delay
* 函数功能：获取低功耗不活动延时
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回低功耗不活动延时
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-07-06          戴辉发             创建
==============================================================*/
uint16_t get_low_deactive_delay(void);

/*=============================================================
* 函数名称：set_voltage_flag
* 函数功能：设定电压功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_voltage_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_voltage_flag
* 函数功能：获取电压功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电压功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_voltage_flag(void);

/*=============================================================
* 函数名称：set_current_flag
* 函数功能：设定电流功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_current_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_current_flag
* 函数功能：获取电流功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电流功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_current_flag(void);

/*=============================================================
* 函数名称：set_temperature_flag
* 函数功能：设定温度功能开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_temperature_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_temperature_flag
* 函数功能：获取温度功能开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           温度功能开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_temperature_flag(void);

/*=============================================================
* 函数名称：set_balance_flag
* 函数功能：设置均衡开关参数
* 参数个数：1
* 函数参数：
*           [IN]      flag               开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_balance_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_balance_flag
* 函数功能：获取均衡开关参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回均衡开关参数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_balance_flag(void);

/*=============================================================
* 函数名称：set_capcity_flag
* 函数功能：设定电池容量告警开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               电池容量告警开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_capcity_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_capcity_flag
* 函数功能：获取电池容量告警开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           电池容量告警开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_capcity_flag(void);

/*=============================================================
* 函数名称：set_show_switch_flag
* 函数功能：设置显示开关参数
* 参数个数：1
* 函数参数：
*           [IN]      flag               显示开关参数
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_show_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_show_flag
* 函数功能：获取显示开关参数
* 参数个数：0
* 函数参数：
* 返 回 值：
*           返回显示开关参数
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_show_flag(void);

/*=============================================================
* 函数名称：set_outside_flag
* 函数功能：设定外部开关
* 参数个数：1
* 函数参数：
*           [IN]      flag               外部开关
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
void set_outside_flag(uint8_t flag);

/*=============================================================
* 函数名称：get_outside_flag
* 函数功能：获取外部开关
* 参数个数：0
* 函数参数：
* 返 回 值：
*           外部开关
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-22          戴辉发             创建
==============================================================*/
uint8_t get_outside_flag(void);

#endif

