#include "AppCurrManage.h"
#include "Appcommunication.h"
#include "dataProcess.h"
#include "system_control.h"
#include "AppCurrManage.h"
#include "vol_manage.h"
#include "current_manage.h"

AlarmConfig continuousDisCurrentOverFirstAlarm;
AlarmState continuousDisCurrentFirstState;

AlarmConfig chargeCurrentOverFirstAlarm;
AlarmState chargeCurrentOverFirstState;


#define FEEDBACK_CURRENT               100
#define FEEDBACK_RECOVER               50
#define FEEDBACK_FIRST_DELAY           3000
#define FEEDBACK_SECOND_DELAY          5000
#define FEEDBACK_THREE_DELAY           6000
#define FEEDBACK_RECOVER_DELAY         200  //3S

#define CURRENT_TIMER_TO_100MS         10

/*=============================================================
 * 函数名称：feedback_mem_init
 * 函数功能：电流管理模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-06-27          戴辉发             创建
==============================================================*/
void Alarm_current_mem_init(void)
{
    chargeCurrentOverFirstAlarm.alarmValue = 350;
    chargeCurrentOverFirstAlarm.recoverValue = 300;
    chargeCurrentOverFirstAlarm.alarmDelay = 50;
    chargeCurrentOverFirstAlarm.recoverDelay = 50;
    
    continuousDisCurrentOverFirstAlarm.alarmValue = -2200;
    continuousDisCurrentOverFirstAlarm.recoverValue = -1800;
    continuousDisCurrentOverFirstAlarm.alarmDelay = 50;
    continuousDisCurrentOverFirstAlarm.recoverDelay = 50;
    
    chargeCurrentOverFirstState.status = NORMAL;
    continuousDisCurrentFirstState.status = NORMAL; 

}
/*==============================================================
 * 函数名称：TempDifferentTimer
 * 函数功能：APP通信模块定时器，以10毫秒为单位
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-22          戴辉发             创建
 ==============================================================*/
void CurrentStateTimer(void)
{
    if (continuousDisCurrentFirstState.delay) continuousDisCurrentFirstState.delay--;
    if (chargeCurrentOverFirstState.delay) chargeCurrentOverFirstState.delay--;
}

/*=============================================================
* 函数名称：CurrentAlarmJudge
* 函数功能：电流告警判决
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
void CurrentAlarmJudge(void)
{
    int16_t current = get_real_current();
    /*非充电状态*/
    if( E_CHARGE_STATUS != get_system_status() )
    {
        /*持续放电电流过大*/
        ValueCompare(current, &continuousDisCurrentOverFirstAlarm, -1, &continuousDisCurrentFirstState);
        if ( get_g_grand_status() == E_ENABLE_STATUS )
        {
          chargeCurrentOverFirstState.status = NORMAL;
        }
    }
    else
    {      
        /*充电电流过大*/  
        ValueCompare(current, &chargeCurrentOverFirstAlarm , 1 , &chargeCurrentOverFirstState);
        if ( get_g_grand_status() == E_ENABLE_STATUS )
        {
          continuousDisCurrentFirstState.status = NORMAL; 
        }
    }
}

/*=============================================================
* 函数名称：GetChargeCurrentOverReport
* 函数功能：获取充电过流状态
* 函数参数：
* 返回值：  
*           充电过流状态
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetChargeCurrentOverReport(void)
{
   uint8_t ret = 0; 
   if (chargeCurrentOverFirstState.status == ALARM)
     ret = 1;
   return  ret;
}

/*=============================================================
* 函数名称：GetContinuousDisCurrentOverReport
* 函数功能：获取持续放电过流状态
* 函数参数：
* 返回值：  
*           瞬态放电过流状态
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetContinuousDisCurrentOverReport(void)
{
    uint8_t ret = 0; 
    if (continuousDisCurrentFirstState.status == ALARM)
      ret = 1;
    return  ret;
}
