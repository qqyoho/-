/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：AFE.C
 * 文件功能描述：实现AFE操作和控制
 *
 * 修改记录：
 * 2021-05-03    戴辉发    创建
*----------------------------------------------------------*/

#ifndef _AFE_DEFINE_
#define _AFE_DEFINE_

#include <stdint.h>
#include "parameter.h"

#define M_AFE_SPI            SPI0
#define R_AFE_SPI            SPI2
#define AFE_TB_DETECT_GPIO_PIN      FL_GPIO_PIN_5  
#define AFE_TB_DETECT_GPIO_TYPE     GPIOC 
#define AFE_ALARM_DETECT_GPIO_PIN      FL_GPIO_PIN_6  
#define AFE_ALARM_DETECT_GPIO_TYPE     GPIOC 

#define R_AFE_TB_DETECT_GPIO_PIN      FL_GPIO_PIN_11  
#define R_AFE_TB_DETECT_GPIO_TYPE     GPIOC 
#define R_AFE_ALARM_DETECT_GPIO_PIN      FL_GPIO_PIN_12  
#define R_AFE_ALARM_DETECT_GPIO_TYPE     GPIOC 

#define CELLS_NUM             10    /* 当前AFE支持的电芯节数 */
//#define MAX_CELL_TEMP_NUM     3     /* 当前AFE支持的电芯温度个数 */

//--------------------宏定义-----------------------//
//JW3370相关指令定义
//chip addr
#define CHIP0_ADDR            0x00
#define CHIP1_ADDR            0x01
#define BROAD_ADDR            0x0F

#define POWER_DOWN            0xf1
#define POWER_UP              0xf2
#define CHG_SET               0xf4
#define DISCHG_SET            0xf7
#define ADDR_UPDATE           0xf8
#define SET_CTR_PRAM          0xfd
#define SET_VOLTAGE_PRAM      0xfe

#define SET_CURRENT_PRAM      0x08

#define CLEAR_IT_FLAG         0x00
#define CTR_SET               0x01
#define READ_VOLTAGE          0x02
#define READ_CURRENT          0x03

#define READ_DETAIL_ALARM     0x05

/* cmds set */ 
#define BAL_SET               0x00
#define ALM_SET               0x02
#define SYS_SET               0x04
//---------------------结构体定义-------------------------------//
typedef enum {
    ODD_ON            =  ((uint8_t)0x00),    /*奇均衡*/
    EVEN_ON           =  ((uint8_t)0x01),    /*偶均衡*/
    All_OFF           =  ((uint8_t)0x02)     /*均衡全关*/
} BalFlg;

//--------------------结构体参数定义-----------------------//
/*********Control commands for Balance /Alarm/ System *******************/
typedef struct CtrCmd {
    uint8_t addr : 4;              //IC 片选地址，1111：广播
    uint8_t cmd  : 3;              //000: balance, 010: alarm, 100:system
    uint8_t odd  : 1;              //odd 校验
} CtrCmd;

/*********Control commands for System *******************/
typedef struct  SysPara {
    uint8_t open_line_detect_pluse_period: 1;       //开路检查脉冲宽度 0：8us 1:32us
    uint8_t reserved  : 5; //
    uint8_t codo_function_off_by_alarm_pin_en : 1;  //通过alarm_pin 关闭 CO/DO 使能
    uint8_t codo_function_off_by_alarm_pin_flg : 1; //alarm_pin 关闭CO/DO 使能标志位
    uint8_t raises_V5A_to_5v5  : 1;
    uint8_t forced_seven : 3;
    uint8_t forced_two : 3;
    uint8_t forced_zero :3;
    uint8_t forced_two1 :2;
    uint8_t csb_wakeup_en  :1;
    uint8_t forced_two2 : 3;
    uint8_t cascade_chip_num :4;
    uint8_t tb_3v3_en :1;
    uint8_t ldo_3v3_en :1;
    uint8_t sleep_to_ship_en :1; //0 :禁止由sleep模式进入ruship 模式
    uint8_t sys_clk_set :1; //0:1Mhz  1:500khz //forced 0
    uint8_t spi_fail_tim; //spi_fail_tim = [31:24] * 512ms
    uint8_t ref_of_ADC_select :1; //选择ADC基准源
    uint8_t cur_ADC_rev_en :1; // 电流adc输出16bit数据取反
    uint8_t vol_ADC_sample_mode :1; // 0:poll mode 1: appointed ADC only
    uint8_t vol_ADC_sample_chanl_select :5; //valid when vol_ADC_sample_mode is 1;
    uint8_t vol_ADC_sample_delay_tim :1; // 0 : no delay, 1:delay half clk_cycle
    uint8_t analog_filter_tim_for_minus_cur :1; //小电流检测模拟滤波时间
    uint8_t reserved1 :1;
    uint8_t cur_ADC_power_consump :1; //1:75uA 0:150uA
    uint8_t spi_timeout_detect_en :1; //0: sleep模式禁止SPI通信超时检测
    uint8_t filter_resistor_corrects_en :1; // 1K 滤波电阻使能控制
    uint8_t reserved2 :1;
    uint8_t digit_clk_control_insleep :1;   // 0:turn on 
    uint8_t tim_counter_control_insleep :1;
    uint8_t spi_communicate_upward_en :1;
    uint8_t spi_communicate_upward_wait_tim :2;
    uint8_t battery_nums :4; //电芯数量设置
} SysPara;

/*********Control commands for Current *******************/
typedef struct  CurPara {
    uint16_t chg_dischg_thrd : 4;   //充电放电阈值
    uint16_t dischg_sc_thrd  : 6; //放电短路电流阈值
    uint16_t slope_correct_of_cur_en : 1;  //电流ADC采样线性校准使能
    uint16_t dischg_sc_delay_tim : 5;  //放电短路电流判断延时
    uint8_t low_current_wakeup_thrd  : 2;  //小电流唤醒阈值
    uint8_t adc_rev_tim : 2;  //16bit ADC 电流采样时长
    uint8_t chg_ocp_delay_tim : 4;  //充电过流保护延时
    uint16_t dischg_sec_ocp_delay_tim : 5;  //放电过流二级保护延时
    uint16_t dischg_fir_ocp_delay_tim : 5;  //放电过流一级保护延时
    uint16_t ocp_delay_tim_step :1;  //放电过流二级级保护延时步长0 : 4ms ,1 : 32ms
    uint16_t reserved  :5;  //
    uint8_t dischg_sec_ocp_thrd; //放电二级过流保护阈值
    uint8_t dischg_fir_ocp_thrd; //放电一级过流保护阈值
    uint8_t reserved1;
    uint8_t chg_ocp_thrd;  //充电过流保护阈值
    uint8_t reserved2;  
} CurPara;

/*********Control commands for Voltage *******************/
typedef struct  VolPara {
    uint8_t dischg_otp_thrd;   //放电过温保护阈值
    uint8_t dischg_utp_thrd; //放电低温保护阈值
    uint8_t reserved :1;  //
    uint8_t ovp_delay_tim_step :1; //过压保护延时步长 0 : 128ms 1 : 512ms
    uint8_t reserved1 :1;  //
    uint8_t uvp_delay_tim_step :1; //欠压保护延时步长 0 : 128ms 1 : 512ms
    uint8_t detect_of_ts3_en :1; // TS3检测使能 0 ：禁止 1：使能
    uint8_t slope_correct_of_vol_en :1;  //电压ADC采样线性校准使能
    uint8_t open_line_detect_cur :1; //0:1uA 1:1.5uA
    uint8_t open_line_detect_en :1; //开路检测功能0:禁止 1:使能
    uint8_t tim_inter_vol_temp :1; //第10节电芯电压采样与第一路温度采样采样时间间隔0:30us 1:50us
    uint8_t open_line_detect_thrd :1; //0:200mv 1:1V
    uint8_t bias_cur_of_voladc :2; //([53:52]+1)*0.5?A 电压ADC采样底电流
    uint8_t voladc_sample_mode :1; // 0:fast mode 1:filtering mode
    uint8_t voladc_sample_handing :1; // 0: saimple count 1:sinc filtering
    uint8_t wait_sample_time_of_sinc_filter :2; //accord datasheet
    uint8_t ovp_delay_tim :4; // 过压保护延时
    uint8_t uvp_delay_tim :4; // 低压保护延时
    uint8_t otp_delay_tim :4; // 过温保护延时
    uint8_t utp_delay_tim :4; // 低温保护延时
    uint8_t ovp_thrd; //过压保护阈值
    uint8_t uvp_thrd; //低压保护阈值
    uint8_t otp_thrd; //过温保护阈值
    uint8_t utp_thrd; //低温保护阈值 
} VolPara;

typedef struct AFE_ctrl_data1//系统控制参数位操作
{
    uint8_t Pchg_out:1;
    uint8_t Pdsg_out:1;
    uint8_t rsvd10:1;
    uint8_t rsvd1:1;
    uint8_t IC_ADD:1;
    uint8_t Low_currrent:1;
    uint8_t Short_det:1;
    uint8_t Charger_det:1;
}AFE_ctrl_data_byte1;

typedef struct AFE_ctrl_data2
{
   uint8_t DSG_OUT:1;
   uint8_t DSG_CTR:1;
   uint8_t CHG_OUT:1;
   uint8_t CHG_CTR:1;
   uint8_t Rsvd3:1;
   uint8_t Load_det:1;
   uint8_t Charger_cur:1;
   uint8_t Rsvd2:1;
}AFE_ctrl_data_byte2;

/*=============================================================
 * 函数名称：afe_mem_init
 * 函数功能：AFE模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_mem_init(void);

/*=============================================================
 * 函数名称：afe_hard_init
 * 函数功能：AFE模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_hard_init(void);

/*=============================================================
 * 函数名称：afe_power_down
 * 函数功能：AFE模块进入休眠
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t afe_power_down(void);

/*=============================================================
 * 函数名称：afe_power_up
 * 函数功能：AFE模块退出休眠
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t afe_power_up(void);

/*=============================================================
 * 函数名称：read_voltage_temp
 * 函数功能：从AFE中读取电压数据和温度数字
 * 参数个数：3
 * 参数描述：
 *           [IN]    addr        地址
 *           [IN]    vol_buf     电压参数
 *           [IN]    temp_buf    温度参数
 * 返 回 值：
 *           0       读取数据失效
 *           1       读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_voltage_temp(uint8_t addr, uint16_t *vol_buf, int16_t *temp_buf);

/*=============================================================
 * 函数名称：read_current
 * 函数功能：从AFE中读取电流数据
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT] *curr_vol  分流器两端的电压
 * 返 回 值：
 *           0        读取数据失效
 *           1        读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_current(uint16_t *curr_vol);

/*=============================================================
 * 函数名称：clear_alarm
 * 函数功能：从AFE中读取电流数据
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void clear_alarm(void);

/*=============================================================
 * 函数名称：read_alarm
 * 函数功能：从AFE中读取告警信息
 * 参数个数：2
 * 参数描述：
 *           [IN]    addr        AFE地址
 *           [IN]    data_buf    告警数据缓冲
 * 返 回 值：
 *           0       读取失败
 *           1       读取成功，数据有效
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_alarm(uint8_t addr, uint8_t *data_buf);

/*=============================================================
 * 函数名称：sysctrl_set
 * 函数功能：系统控制功率开关
 * 参数个数：2
 * 参数描述：
 *           [IN]    cflag       充电MOS控制
 *           [IN]    dflag       放电MOS控制
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void sysctrl_set(uint8_t cflag, uint8_t dflag);

/*=============================================================
 * 函数名称：Alarm_Set
 * 函数功能：AFE告警设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    mflag       告警设置标识
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void alarm_set(uint8_t mflag);

/*=============================================================
 * 函数名称：balsate
 * 函数功能：均衡设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    balsate     均衡状态, 按10节设置
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void balance_set(uint16_t balsate);

/*=============================================================
 * 函数名称：set_ctr_parameter
 * 函数功能：控制参数设置
 * 参数个数：2
 * 参数描述：
 *           [IN]    chip_num    AFE级联个数(0:表示1个)
 *           [IN]    cell_num    电池串数
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_ctr_parameter(uint8_t chip_num, uint8_t cell_num);
/*=============================================================
 * 函数名称：set_ctr_parameter
 * 函数功能：控制参数设置
 * 参数个数：2
 * 参数描述：
 *         
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16        liyong      创建
==============================================================*/
void set_ctr_updateaddress(void);
/*=============================================================
 * 函数名称：set_voltage_parameter
 * 函数功能：电压参数设置
 * 参数个数：4
 * 参数描述：
 *           [IN]    ovp_vol     电压和温度参数
 *           [IN]    uvp_vol     电压和温度参数
 *           [IN]    ovt_temp    电压和温度参数
 *           [IN]    uvt_temp    电压和温度参数
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_voltage_parameter(uint16_t ovp_vol, uint16_t uvp_vol, int16_t ovt_temp, int16_t uvt_temp, uint8_t blance_mode, uint8_t balance_time ,uint8_t break_line);

/*=============================================================
 * 函数名称：set_current_parameter
 * 函数功能：电流参数设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    short_cur   短路电流
 *           [IN]    sc_delay    短路延时，以uS为单位
 *           [IN]    ocp_dch     AFE放电过流保护门限，以A为单位
 *           [IN]    ocp_ch      AFE充电过流保护门限，以A为单位
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/

/*=============================================================
 * 函数名称：afe_enter_normal
 * 函数功能：AFE模块进入正常状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t afe_enter_normal(void);
/*=============================================================
 * 函数名称：afe_enter_normal
 * 函数功能：AFE模块进入正常状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t afe_update_address(void);
/*=============================================================

 * 函数名称：sleep_enter_ship
 * 函数功能：从sleep进入ship
 * 参数个数：
 * 参数描述：
 *           
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-15         李勇      创建
==============================================================*/

void sleep_enter_ship(void);
/*=============================================================
 * 函数名称：get_para_set_flag
 * 函数功能：获取参数设置状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-22        liy      创建
==============================================================*/
uint8_t get_para_set_flag(void);
/*=============================================================
 * 函数名称：read_reserve_alarm
 * 函数功能：从 备用 AFE中读取告警信息
 * 参数个数：2
 * 参数描述：
 *           [IN]    addr        AFE地址
 *           [IN]    data_buf    告警数据缓冲
 * 返 回 值：
 *           0       读取失败
 *           1       读取成功，数据有效
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_reserve_alarm(uint8_t addr, uint8_t *data_buf);
/*=============================================================
 * 函数名称：SleepSysctrlSet
 * 函数功能：休眠系统控制功率开关
 * 参数个数：2
 * 参数描述：
 *           [IN]    cflag       充电MOS控制
 *           [IN]    dflag       放电MOS控制
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void SleepSysctrlSet(void);
/*=============================================================
 * 函数名称：ReadParameteSetOverFlag
 * 函数功能：从AFE中读取电流数据
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT] *curr_vol  分流器两端的电压
 * 返 回 值：
 *           0        读取数据失效
 *           1        读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t ReadParameteSetOverFlag(uint8_t *paraFlag);
void set_current_parameter(int16_t short_cur, uint16_t sc_delay, int16_t ocp_dch, int16_t ocp_ch);
void set_load_config(uint8_t statue);
void set_charger_config(uint8_t statue);
void AFE_5ms_timer(void);
#endif
