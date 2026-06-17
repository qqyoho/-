#ifndef DATAPROCESS_H
#define DATAPROCESS_H
#include <stdint.h>
#include <string.h>
#include "parameter.h"
#include "temp_manage.h"

#define   ALARM_DISCHARGE       0
#define   ALARM_CHARGE          1
#define   ALARM_NUM             2

extern uint8_t alarmCodeNumber;
typedef enum
{
    NORMAL_STATUS,
    ABNORMAL_STATUS,
}FAULT_STATUS;
typedef struct
{
    FAULT_STATUS status; 
    uint8_t trigger;
    uint16_t delay;
    uint16_t setDelay;
    uint8_t recover;
	uint16_t recDelay;
}EVENT_FAULT_JUDGE;

typedef struct{
   uint8_t soh;
   uint8_t status;
   uint8_t status2;
   uint8_t errorNumber;
   uint16_t pInsulationR;
   uint16_t nInsulationR;
   uint32_t  dischargeTimeAccu;/*累计放电时间*/
   uint32_t  chargeTimeAccu;   /*累计充电时间*/
   uint16_t  nowDischargeTime; /*当前放电时间*/ 
   uint16_t  nowChargeTime;    /*当前充电时间*/
   uint16_t  nowDischargeCapa; /*当前放电容量*/ 
   uint16_t  nowChargeCapa;    /*当前充电容量*/ 
   uint16_t  disIdleTime;      /*放电待机时间*/
   uint16_t  disSleepResTime;  /*放电待机剩余时间*/
   uint16_t  chIdleTime;       /*充电待机时间*/
   uint16_t  chSleepResTime;   /*充电待机剩余时间*/
   uint16_t  version;          /*协议版本*/
}REPORT_STATUS_DATA;

typedef struct{
   uint16_t  maxCellVoltage;
   uint8_t   maxCellSerNumber;
   uint8_t   maxCellPackNumber; 
   
   uint16_t  minCellVoltage;
   uint8_t   minCellSerNumber;
   uint8_t   minCellPackNumber;
   
   int16_t   maxTemp;
   uint8_t   maxTempSerNumber;
   uint8_t   maxTempPackNumber; 
   
   int16_t   minTemp;
   uint8_t   minTempSerNumber;
   uint8_t   minTempPackNumber;  
}REPORT_MAX_MIN_DATA;

/*国标充电器信息转发到整车网络*/
typedef struct
{
   uint16_t RequstVol;
   uint16_t RequstCur;
   uint16_t voltage;
   uint16_t current;
   uint8_t status;
   uint8_t status2;
   uint16_t delay;
}GB_CHARGER_STATUS;

typedef struct
{
   uint16_t RequstVol;
   uint16_t RequstCur;
   uint16_t voltage;
   uint16_t current;
}GB_CHARGER_DATA;

typedef struct 
{
    uint8_t status:4;
    uint8_t version:4;  
}XIEYI_VERSION_BITS;

typedef union
{
    uint8_t byte;
    XIEYI_VERSION_BITS bits;
}XIEYI_VERSION;

typedef struct 
{   
    /*0x2F0*/
    uint16_t totalVoltage;
    int16_t current;
    uint16_t capacity;
    uint8_t soc;
    XIEYI_VERSION version;
    /*0x3F0*/
    uint16_t maxCellV;
    uint16_t minCellV;
    uint8_t maxCellT;
    uint8_t minCellT;
    uint8_t allarm;
    uint8_t location;  
    /*0x270*/
    uint8_t disPower[3];
    uint8_t chPower[3];
    uint8_t res0x270[2];
    uint8_t cell_temp[2];
    uint8_t power_temp;
}PDO_DATA_SHOW;



extern REPORT_STATUS_DATA reportRealData;
extern REPORT_MAX_MIN_DATA reportMaxMinData;
extern GB_CHARGER_STATUS gbChargerStatus;
extern GB_CHARGER_DATA gbChargerData;
extern PDO_DATA_SHOW pdoDataShow;
/*实时数据*/


/*报文发送类型*/
typedef enum{
	CYCLE_SEND, /*周期触发*/
	EVEN_SEND,  /*事件触发类型*/
}MESSAGE_SEND_TYPE;
/*回调函数*/
typedef void (*CALL_BACK_FUCTION)(void);
/*报文发送管理单元*/
typedef struct{
	uint32_t id;
	MESSAGE_SEND_TYPE type;
	CALL_BACK_FUCTION callBackF;
	uint16_t cycle;
	uint16_t timer;
	uint16_t event;	
}MESSAGE_SEND_MANAGE;

typedef struct {
    int16_t alarmValue;
    int16_t recoverValue;
    uint16_t alarmDelay;
    uint16_t recoverDelay;
} AlarmConfig;

typedef enum {
    NORMAL,
    WAITING,
    ALARM
} AlarmStatus;

typedef struct {
    uint16_t delay;
    AlarmStatus status;
} AlarmState;

void NobCanOpenSdoObjdictProcess();
/*==============================================================
* 函数名称：SendDataJudge
* 函数功能：数据发送判决
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2023-08-30           李勇              创建
==============================================================*/
static void SendDataJudge(MESSAGE_SEND_MANAGE *me,uint8_t len);
/*==============================================================
* 函数名称：SendDataProcess
* 函数功能：数据发送处理
* 函数参数：
* 返 回 值： 
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2023-08-30           李勇              创建
==============================================================*/
void SendDataProcess();
/*==============================================================
* 函数名称：VehicleProtocolTimeDelay
* 函数功能：整车协议定时器
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2023-08-30           李勇              创建
==============================================================*/
void VehicleProtocolTimeDelay();
  /*=============================================================
* 函数名称：GetAlarmCodeNumber
* 函数功能：获取告警码
* 函数参数：
* 返回值：  
*           告警码
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetAlarmCodeNumber();
/*=============================================================
* 函数名称：VauleCompare
* 函数功能：根据参数，进行告警判决
* 函数参数：
            st 状态  
            para 参数  
            dir 值比较方向
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-02-19         liyong             创建
==============================================================*/
void ValueCompare(int16_t realData, AlarmConfig *cfg,int8_t dir, AlarmState *state);
/*=============================================================
* 函数名称：EventAbnormalJudge
* 函数功能：事件异常判决
* 函数参数：fault 待判决故障
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-04-08         liyong             创建
==============================================================*/
void EventAbnormalJudge(EVENT_FAULT_JUDGE *fault);


void SendID_0x2F0(void);
void SendID_0x3F0(void);
void SendID_0x270(void);
#endif