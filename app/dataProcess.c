#include "dataProcess.h"
#include "system_control.h"
#include "switch_status.h"
#include "vol_manage.h"
#include "system_adjust.h"
#include "can.h"
#include "current_manage.h"
#include "soc.h"
#include "storage_manage.h"
#include "temp_manage.h"
#include "balance.h"
#include "AppCommunication.h"
#include "AppVolManage.h"
#include "AppTempManage.h"
#include "AppCurrManage.h"
#include "short.h"
#include "dataProcess.h"

REPORT_STATUS_DATA reportRealData;
REPORT_MAX_MIN_DATA reportMaxMinData;
PDO_DATA_SHOW pdoDataShow;

uint8_t alarmCodeNumber;
/*数据流程*/
GB_CHARGER_STATUS gbChargerStatus;
GB_CHARGER_DATA gbChargerData;
static volatile uint16_t timerBase;

#define   CYCLE_TIME_SIZE		(5)
#define   MIN_CYCLE_TIME		 1 
#define   CYCLE_TIME_2000MS		(2000/CYCLE_TIME_SIZE)
#define   CYCLE_TIME_1000MS		(1000/CYCLE_TIME_SIZE)
#define   CYCLE_TIME_200MS		(200/CYCLE_TIME_SIZE)
#define   CYCLE_TIME_100MS		(100/CYCLE_TIME_SIZE)
#define   CYCLE_TIME_500MS		(500/CYCLE_TIME_SIZE)
#define   CYCLE_TIME_20MS		(20/CYCLE_TIME_SIZE)


static volatile uint16_t timerBase;


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
void ValueCompare(int16_t realData, AlarmConfig *cfg,int8_t dir, AlarmState *state)
{
    uint8_t aflag = 0;
    uint8_t rflag = 0;

    if (dir == -1) 
    {
        if (realData < cfg->alarmValue) 
        {
            aflag = 1;
        } 
        else if (realData > cfg->recoverValue) 
        {
            rflag = 1;
        }
    } 
    else 
    {
        if (realData > cfg->alarmValue) 
        {
            aflag = 1;
        } else if (realData < cfg->recoverValue) 
        {
            rflag = 1;
        }
    }

    switch (state->status) {
    case NORMAL:
        if (aflag) {
            state->status = WAITING;
            state->delay = cfg->alarmDelay;
        }
        break;
        
    case WAITING:
        if (aflag) {
            if (state->delay == 0) {
                state->status = ALARM;
                state->delay = cfg->recoverDelay;
            }
        } else {
            state->status = NORMAL;
        }
        break;
        
    case ALARM:
        if (rflag) {
            if (state->delay == 0) {
                state->status = NORMAL;
            }
        } else {
            state->delay = cfg->recoverDelay;
        }
        break;
        
    default:
        state->status = NORMAL;
        break;
    }
}
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
void EventAbnormalJudge(EVENT_FAULT_JUDGE *fault)
{
    switch(fault->status)
    {
    case NORMAL_STATUS:
        if( fault->trigger == 1 )
        {
            if( fault->delay == 0 )
            {
                fault->status = ABNORMAL_STATUS;
                fault->delay = fault->recDelay;
            }
        }
        else
        {
            fault->delay = fault->setDelay;
        }
      break;
    case ABNORMAL_STATUS:
        if( fault->recover == 1 )
        {
            if( fault->delay == 0 )
            {
                fault->status = NORMAL_STATUS;
            }
        }
        else
        {
            fault->delay = fault->recDelay;
        }
      break;
    default:
      fault->status = NORMAL_STATUS;
      break;
    }
}

