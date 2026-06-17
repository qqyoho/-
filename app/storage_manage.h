#ifndef _STORAGE_MANAGE_DEFINE_
#define _STORAGE_MANAGE_DEFINE_

#include <stdint.h>

#define MIN_STORAGE_NUM           (0x01)
#define MAX_STORAGE_NUM           (0xE0)
#define MAX_SECTOR_NUM            (0x100)


//地址划分如下
//BMS序列号（20bytes）  
//电池序列号（20bytes）
#define MAIN_SECTOR_NUM               (0)
#define BACK_SECTOR_NUM               (1)
#define BACK_OFFSET                   (0x06)
//电压校准值
#define ADJUST_VOLTAGE_START          (0x02)  
#define ADJUST_VOLTAGE_BACK           (ADJUST_VOLTAGE_START + BACK_OFFSET)
//电流校准值
#define ADJUST_CURRENT_START          (0x05)
#define ADJUST_CURRENT_BACK           (ADJUST_CURRENT_START + BACK_OFFSET)
//硬件版本号
#define HARDWARE_VER_START            (0x06)
#define HARDWARE_VER_BACK             (HARDWARE_VER_START + BACK_OFFSET)


#define PARA_BACK_OFFESET             (30)
/* 电压参数存储扇区 */
#define VOL_PARA_SECTOR_MAIN          (20)
#define VOL_PARA_SECTOR_BACK          (VOL_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
/* 电流参数存储扇区 */
#define CURR_PARA_SECTOR_MAIN         (21)
#define CURR_PARA_SECTOR_BACK         (CURR_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
/* 温度参数存储扇区 */
#define TEMP_PARA_SECTOR_MAIN         (22)
#define TEMP_PARA_SECTOR_BACK         (TEMP_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
/* 加热及均衡参数存储扇区 */
#define BALANCE_PARA_SECTOR_MAIN      (23)
#define BALANCE_PARA_SECTOR_BACK      (BALANCE_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
/* 容量参数存储扇区 */
#define CAPCITY_PARA_SECTOR_MAIN      (24)
#define CAPCITY_PARA_SECTOR_BACK      (CAPCITY_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
/* 其他参数存储扇区 */
#define OTHER_PARA_SECTOR_MAIN        (25)
#define OTHER_PARA_SECTOR_BACK        (OTHER_PARA_SECTOR_MAIN + PARA_BACK_OFFESET)
//容量参数   
#define SOC_CAPCITY_MAIN_ADDRESS      (26)
#define SOC_CAPCITY_BACK_ADDRESS      (SOC_CAPCITY_MAIN_ADDRESS + PARA_BACK_OFFESET)
//极限值存储1
#define PEAK1_SECTOR_MAIN             (27)
#define PEAK1_SECTOR_BACK             (PEAK1_SECTOR_MAIN+ PARA_BACK_OFFESET)
//极限值存储2
#define PEAK2_SECTOR_MAIN             (28)
#define PEAK2_SECTOR_BACK             (PEAK2_SECTOR_MAIN+ PARA_BACK_OFFESET)
//保护次数累计1
#define PROTECT1_SECTOR_MAIN          (29)
#define PROTECT1_SECTOR_BACK          (PROTECT1_SECTOR_MAIN + PARA_BACK_OFFESET)
//保护次数累计2
#define PROTECT2_SECTOR_MAIN          (30)
#define PROTECT2_SECTOR_BACK          (PROTECT2_SECTOR_MAIN + PARA_BACK_OFFESET)
//异常记录1
#define FAULT1_SECTOR_MAIN            (31)
#define FAULT1_SECTOR_BACK            (FAULT1_SECTOR_MAIN + PARA_BACK_OFFESET)
//异常记录2
#define FAULT2_SECTOR_MAIN            (32)
#define FAULT2_SECTOR_BACK            (FAULT2_SECTOR_MAIN + PARA_BACK_OFFESET)
//最近升级记录
#define UPDATE_SECTOR_MAIN            (33)
#define UPDATE_SECTOR_BACK            (UPDATE_SECTOR_MAIN + PARA_BACK_OFFESET)
//均衡记录
#define BLANCE_SECTOR_MAIN            (34)  //34 35 36 37
#define BLANCE_SECTOR_BACK            (BLANCE_SECTOR_MAIN + PARA_BACK_OFFESET)
//上电一天工厂模式标志
#define POWER_ON_SECTOR_MAIN          (38)

//运行记录
#define RUNNING_RECORD_ADR_MAIN         (100)
#define RUNNING_RECORD_ADR_BACK         (101)
#define RUNNING_RECORD_STORE_START      (102)
#define RUNNING_RECORD_STORE_END        (299)
//保护记录
#define PROTECT_RECORD_ADR_MAIN         (300)
#define PROTECT_RECORD_ADR_BACK         (301)
#define PROTECT_RECORD_STORE_START      (302)
#define PROTECT_RECORD_STORE_END        (499)

/* 写消息类型 */
typedef enum _E_WRITE_MSG_TYPE_
{
	E_SERIAL_NO_PARA_MSG     = 0x001,      /* 产品系列号 */
	E_ADJUST_PARA_MSG        = 0x002,      /* 校准参数存储 */
	E_RUN_PARAMETER_PARA_MSG = 0x004,      /* 系统运行参数 */  
	E_SOC_CIRCLE_PARA_MSG    = 0x008,      /* 容量循环相关参数 */
	E_PEAK1_RECORD_MSG    = 0x010,         /* 历史累计极限值1 */
    E_PEAK2_RECORD_MSG    = 0x020,         /* 历史累计极限值2 */
    E_PROTECT1_RECORD_MSG = 0x040,         /* 累计保护1次数 */
    E_PROTECT2_RECORD_MSG = 0x080,         /* 累计保护2次数 */
    E_FAULT1_RECORD_MSG   = 0x100,         /* 故障1存储 */
    E_FAULT2_RECORD_MSG   = 0x200,         /* 故障2存储 */
    E_UPDATE_RECORD_MSG   = 0x400,         /* 升级记录存储 */
    E_BLANCE_RECORD_MSG   = 0x800,         /* 均衡记录存储 */
	E_RUN_RECORD_MSG      = 0x1000,        /* 运行记录存储 */
    E_PROTECT_RECORD_MSG  = 0x2000,        /* 系统极值处理流程 */
	E_HARDWARE_VERSION_MSG  = 0x4000,      /* 硬件版本号处理流程 */
    E_POWER_ON_FLAG_MSG   = 0x8000,        /* 上电一天工厂模式标志 */
    E_MAX_WRITEMSG_NUM 
}e_write_msg_type;

extern uint32_t needstoreflag; /* 是否有数据需要存储 */

typedef struct
{
  uint32_t conut;
  uint8_t time[6];
  uint16_t res;
}NUMBER_SOC;

typedef struct
{
  uint16_t conut;
  uint8_t time[6];
}NUMBER;

typedef struct
{
  uint8_t softversion[24];
  uint8_t time[6];
}UPDATE;


/* 充放电循环参数结构 */
typedef struct _PROTECT1_NUMB
{
    NUMBER over_vol_num; /* 过压次数 */
    NUMBER low_vol_num; /* 欠压次数 */
    NUMBER disch_temp_num;  /* 温度禁充次数 */
    NUMBER disdch_temp_num; /* 温度禁放次数 */
    NUMBER ch_ov_curr_num; /* 充电过流次数 */
    NUMBER dch_over_curr_num; /* 放电过流次数 */
	uint8_t number;
}t_protect1_numb;

/* 充放电循环参数结构 */
typedef struct _PROTECT2_NUMB
{
    NUMBER short_num;/* 短路次数 */
    NUMBER mcu_ov_num; /* 累计MCU过压终极保护次数 */
    NUMBER mcu_uv_num; /* 累计MCU欠压终极保护次数 */
    NUMBER afe_ov_num; /* 累计AFE过压保护次数 */
    NUMBER afe_uv_num;  /* 累计AFE欠压保护次数 */
    NUMBER afe_ocd_num; /* 累计AFE过流保护次数 */ 
	uint8_t number;
}t_protect2_numb;


typedef struct _ERROR_NUMB1
{
    NUMBER afe_error_num; /* AFE通信故障次数 */
    NUMBER afe_vol_s_num; /* AFE采集电压不变化次数 */
    NUMBER afe_vol_d_num; /* AFE电压对比异常采集次数 */
    NUMBER afe_vol_c_num;/* AFE电压电流异常次数 */
    NUMBER poweron_fault_num;/*电源模块上下电失败次数*/
    NUMBER poweroff_fault_num;/*电源模块上下电失败次数*/
	uint8_t number;
}t_fault1_numb_record;

typedef struct _ERROR_NUMB2
{
    NUMBER eeprom_fault_num; /* 存储器存储出错次数 */
    NUMBER can_fault_num;   /*CAN通讯失败次数*/ 
    NUMBER chmos_fault_time;/*充电MOS失效时间*/
    NUMBER dismos_fault_time;/*放电MOS失效时间*/
    NUMBER mcu_reset_num;  /*历史累计MCU复位次数*/
    NUMBER system_run_time;/*BMS累计运行时间*/
	uint8_t number;
}t_fault2_numb_record;

typedef struct _UPDATE_RECORD
{
	UPDATE updatetime[2]; /* 最后升级的两次时间记录*/ 
    uint8_t number;
}t_update_record;

typedef struct
{
  uint32_t conut;
  uint8_t time[6];
  uint16_t res;
}BLANCENUMBER;

typedef struct _BALANCE_TIME
{
	BLANCENUMBER cellblance[16]; /* 累计均衡次数及最后一次均衡时间*/ 
	uint8_t number;
}t_blance_time;

extern t_protect1_numb protect1_numb;
extern t_protect2_numb protect2_numb;
extern t_fault1_numb_record fault1_numb_record;
extern t_fault2_numb_record fault2_numb_record;
extern t_update_record last_two_update;
extern t_blance_time cell_blance_numb;


/* 准备写入存储区域数据结构 */
typedef struct _T_WRITE_DATA_STRUCT_
{
    uint32_t rsvd; /* 保留字段 */
	uint8_t *write_memery_address; /* 本次写内存地址 */
	uint16_t write_storge_address; /* 本次写存储器地址 */
	e_write_msg_type msg_type; /* 写消息类型 */
	uint8_t write_len; /* 本次写数据字节长度 */
}t_write_data_st;

/* 充放电循环参数结构 */
typedef struct _DISCHARGE_CIRCLE_STRUCT_
{
    NUMBER_SOC charge_num;    /* 充电次数 */
    NUMBER_SOC circle_num;    /* 放电循环次数 */
    uint32_t total; /* 实际总电量, 考虑老化系数之后的额定总容量, 单位10毫安秒 */
	uint32_t residue; /* 剩余电量, 单位10毫安秒 */
    uint32_t runtime;
	uint8_t number;
}t_dch_circle_st;


extern t_dch_circle_st g_dch_circle; /* 记录充放电循环 */

void storage_manage_mem_init(void);

void storage_manage_hard_init(void);
void storage_manage_process(void);

/*=============================================================
* 函数名称：set_write_data_to_queue
* 函数功能：将要写入的数据写入消息队列
* 参数个数：1
* 函数参数：
*           [IN/OUT]          write_data
* 返 回 值：
*           0                 未获取到数据
*           sizeof(t_write_data_st)
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2018-06-22       戴辉发     创建
==============================================================*/
void set_write_data_to_queue(t_write_data_st *write_data);

/*=============================================================
* 函数名称：read_data_from_storage
* 函数功能：从指定扇区读取指定长度数据并通过参数返回
* 参数个数：3
* 函数参数：
*           [IN]      sector_num    扇区编号
*           [IN/OUT]  buf           数据缓冲
*           [IN]      len           读取数据缓冲长度
* 返 回 值：
*           实际读取到的数据长度
* 修改记录: 
*===============================================================
* 日期                修改人        修改内容
* 2018-07-05          戴辉发        创建
==============================================================*/
uint8_t read_data_from_storage(uint16_t sector_num, uint8_t *buf, uint8_t len);

/*=============================================================
* 函数名称：judge_new_sector
* 函数功能：根据提供的两个扇区序列号判断当前最新数据存储的扇区
* 参数个数：2
* 函数参数：
*           [IN]      value1        扇区序号1
*           [IN]      value2        扇区序号2
* 返 回 值：
*           -1        value2为最新数据扇区
*           0         value1和value2都为未写过数据扇区
*           1         value1为最新数据扇区
* 修改记录: 
*===============================================================
* 日期                修改人        修改内容
* 2018-07-05          戴辉发        创建
==============================================================*/
int16_t judge_new_sector(uint8_t value1, uint8_t value2);

/*=============================================================
* 函数名称：sector_num_next
* 函数功能：扇区序列号向前推进一格
* 参数个数：1
* 函数参数：
*           [IN]      serial_num    扇区序号
* 返 回 值：
*           向前推进一步后的序列号
* 修改记录: 
*===============================================================
* 日期                修改人        修改内容
* 2018-07-05          戴辉发        创建
==============================================================*/
uint8_t sector_num_next(uint8_t serial_num);
/*=============================================================
 * 函数名称：get_timdedata
 * 函数功能：获取当前时间戳数据
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void get_timdedata( uint8_t *time);
/*=============================================================
* 函数名称：time_s_storage
* 函数功能：存储管理定时器
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2020-06-03       李勇       创建
==============================================================*/
void time_s_storage(void);
/*=============================================================
* 函数名称：querydatastore
* 函数功能：是否需要存储数据，半小时存储一次管理
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2020-06-03       李勇       创建

30min = 1800s 每100ms一段
==============================================================*/
void querydatastore(void);
#endif
