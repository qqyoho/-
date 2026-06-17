#include "Appcommunication.h"
#include "dataProcess.h"
#include "system_control.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "balance.h"
#include "temp_manage.h"
#include "AppVolManage.h"
/*总压*/
AlarmConfig totalVoltageOverFirstAlarm[ALARM_NUM];
AlarmConfig totalVoltageUnderFirstAlarm[ALARM_NUM];
/*单体*/
AlarmConfig cellVoltageOverFirstAlarm[ALARM_NUM];
AlarmConfig cellVoltageUnderFirstAlarm[ALARM_NUM];

AlarmConfig cellVoltageDifferentFirstAlarm[ALARM_NUM];
AlarmConfig cellVoltageDifferentSecondAlarm[ALARM_NUM];
AlarmConfig cellVoltageDifferentThreeAlarm[ALARM_NUM];

AlarmState cellVoltageDifferentFirstState[ALARM_NUM];
AlarmState cellVoltageDifferentSecondState[ALARM_NUM];
AlarmState cellVoltageDifferentThreeState[ALARM_NUM];

AlarmState DiscellVoltageOverFirstState;
AlarmState DiscellVoltageUnderFirstState;

AlarmState DistotalVoltageOverFirstState;
AlarmState DistotalVoltageUnderFirstState;

#define  TIMES_TO_100MS   1

static void SetAlarmConfig(AlarmConfig *cfg, int16_t alarmValue, int16_t recoverValue, uint16_t alarmDelay, uint16_t recoverDelay)
{
    cfg->alarmValue = alarmValue;
    cfg->recoverValue = recoverValue;
    cfg->alarmDelay = alarmDelay;
    cfg->recoverDelay = recoverDelay;
}

/*=============================================================
 * 函数名称：APPVoltageTimerDelay
 * 函数功能：电压处理100ms延时
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
* 2024-03-19         liyong             创建
==============================================================*/
void APPVoltageTimerDelay(void)
{
    uint8_t i;
    for(i=0;i<ALARM_NUM;i++)
    {
        if (cellVoltageDifferentFirstState[i].delay) cellVoltageDifferentFirstState[i].delay --;	
        if (cellVoltageDifferentSecondState[i].delay) cellVoltageDifferentSecondState[i].delay --;
        if (cellVoltageDifferentThreeState[i].delay) cellVoltageDifferentThreeState[i].delay --;
    }
    if (DiscellVoltageOverFirstState.delay) DiscellVoltageOverFirstState.delay --;
    if (DiscellVoltageUnderFirstState.delay) DiscellVoltageUnderFirstState.delay --;
    
    if (DistotalVoltageOverFirstState.delay) DistotalVoltageOverFirstState.delay --; 
    if (DistotalVoltageUnderFirstState.delay) DistotalVoltageUnderFirstState.delay --;

}

/*=============================================================
 * 函数名称：VoltageMemoryWake_mem_init
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
void VoltageMemoryWake_mem_init(void)
{
    uint8_t i;

    /* discharge total voltage */
    SetAlarmConfig(&totalVoltageOverFirstAlarm[ALARM_DISCHARGE], 380 * BAT_NUM, 333 * BAT_NUM, 50, 50);
    SetAlarmConfig(&totalVoltageUnderFirstAlarm[ALARM_DISCHARGE], 2160, 2480, 50, 30);

    /* charge total voltage */
    SetAlarmConfig(&totalVoltageOverFirstAlarm[ALARM_CHARGE], 350 * BAT_NUM, 345 * BAT_NUM, 30, 30);
    SetAlarmConfig(&totalVoltageUnderFirstAlarm[ALARM_CHARGE], 240 * BAT_NUM, 270 * BAT_NUM, 50, 50);

    /* discharge cell voltage */
    SetAlarmConfig(&cellVoltageOverFirstAlarm[ALARM_DISCHARGE], 3800, 3330, 50, 50);
    SetAlarmConfig(&cellVoltageUnderFirstAlarm[ALARM_DISCHARGE], 2700, 3100, 50, 50);

    /* charge cell voltage */
    SetAlarmConfig(&cellVoltageOverFirstAlarm[ALARM_CHARGE], 3500, 3400, 50, 50);
    SetAlarmConfig(&cellVoltageUnderFirstAlarm[ALARM_CHARGE], 2400, 2700, 50, 50);

    SetAlarmConfig(&cellVoltageDifferentFirstAlarm[ALARM_DISCHARGE], 500, 300, 50, 50);
    SetAlarmConfig(&cellVoltageDifferentSecondAlarm[ALARM_DISCHARGE], 500, 300, 30, 30);
    SetAlarmConfig(&cellVoltageDifferentThreeAlarm[ALARM_DISCHARGE], 800, 300, 30, 30);

    SetAlarmConfig(&cellVoltageDifferentFirstAlarm[ALARM_CHARGE], 450, 300, 50, 50);
    SetAlarmConfig(&cellVoltageDifferentSecondAlarm[ALARM_CHARGE], 450, 300, 50, 50);
    SetAlarmConfig(&cellVoltageDifferentThreeAlarm[ALARM_CHARGE], 1000, 300, 30, 30);

    for(i = 0; i < ALARM_NUM; i++)
    {
        cellVoltageDifferentFirstState[i].status = NORMAL;
        cellVoltageDifferentSecondState[i].status = NORMAL;
        cellVoltageDifferentThreeState[i].status = NORMAL;
    }
    DiscellVoltageOverFirstState.status = NORMAL;
    DiscellVoltageUnderFirstState.status = NORMAL;
    DistotalVoltageOverFirstState.status = NORMAL;
    DistotalVoltageUnderFirstState.status = NORMAL;
}

/*=============================================================
* 函数名称：VoltageAlarmJudge
* 函数功能：电压告警判决
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
void VoltageAlarmJudge(void)
{
    uint8_t i;
    uint8_t alarmType = ALARM_DISCHARGE;
    uint8_t clearType = ALARM_CHARGE;
    uint16_t total_vol = get_total_vol()/10;
    uint16_t max_cell_vol = get_max_cell_vol();
    uint16_t min_cell_vol = get_min_cell_vol();
    uint16_t spin = max_cell_vol - min_cell_vol;

    if(E_CHARGE_STATUS == get_system_status())
    {
        alarmType = ALARM_CHARGE;
        clearType = ALARM_DISCHARGE;
    }
#if defined(TIANFENG) && defined(BAT_8S)
    else if(get_average_cell_temp() < 0)
    {
        cellVoltageUnderFirstAlarm[ALARM_DISCHARGE].alarmValue = 2600;
        cellVoltageUnderFirstAlarm[ALARM_DISCHARGE].recoverValue = 3100;
        totalVoltageUnderFirstAlarm[ALARM_DISCHARGE].alarmValue = 2080;
        totalVoltageUnderFirstAlarm[ALARM_DISCHARGE].recoverValue = 2480;
    }
    else
    {
        cellVoltageUnderFirstAlarm[ALARM_DISCHARGE].alarmValue = 2700;
        cellVoltageUnderFirstAlarm[ALARM_DISCHARGE].recoverValue = 3100;
        totalVoltageUnderFirstAlarm[ALARM_DISCHARGE].alarmValue = 2160;
        totalVoltageUnderFirstAlarm[ALARM_DISCHARGE].recoverValue = 2480;
    }
#endif
    for(i = 0; i < ALARM_NUM; i++)
    {
        if(i == clearType)
        {
            cellVoltageDifferentFirstState[i].status = NORMAL;
            cellVoltageDifferentFirstState[i].delay = 0;
            cellVoltageDifferentSecondState[i].status = NORMAL;
            cellVoltageDifferentSecondState[i].delay = 0;
            cellVoltageDifferentThreeState[i].status = NORMAL;
            cellVoltageDifferentThreeState[i].delay = 0;
        }
    }

    ValueCompare(max_cell_vol, &cellVoltageOverFirstAlarm[alarmType], 1,&DiscellVoltageOverFirstState);
    ValueCompare(min_cell_vol, &cellVoltageUnderFirstAlarm[alarmType], -1,&DiscellVoltageUnderFirstState);
#if defined(TIANFENG) && defined(BAT_8S)
    if(E_CHARGE_STATUS == get_system_status())
    {
        DistotalVoltageOverFirstState.status = NORMAL;
        DistotalVoltageOverFirstState.delay = 0;
    }
    else
#endif
    {
        ValueCompare(total_vol, &totalVoltageOverFirstAlarm[alarmType], 1,&DistotalVoltageOverFirstState);
    }
    ValueCompare(total_vol, &totalVoltageUnderFirstAlarm[alarmType], -1,&DistotalVoltageUnderFirstState);
    ValueCompare(spin, &cellVoltageDifferentFirstAlarm[alarmType], 1,&cellVoltageDifferentFirstState[alarmType]);
    ValueCompare(spin, &cellVoltageDifferentSecondAlarm[alarmType], 1,&cellVoltageDifferentSecondState[alarmType]);
    ValueCompare(spin, &cellVoltageDifferentThreeAlarm[alarmType], 1,&cellVoltageDifferentThreeState[alarmType]);
}

/*=============================================================
* 函数名称：GetcellVoltageFirstReport
* 函数功能：获取单体电芯状态
* 函数参数：
* 返回值：  
*           电芯电压状态
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetcellVoltageFirstReport()
{
    uint8_t ret = 0; 
    if( DiscellVoltageOverFirstState.status == ALARM )
      ret = 1;
    else if( DiscellVoltageUnderFirstState.status == ALARM )
      ret = 2;

    return ret;
}

/*=============================================================
* 函数名称：GettatolVoltageFirstReport
* 函数功能：获取单体电芯状态
* 函数参数：
* 返回值：  
*           电芯电压状态
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GettatolVoltageFirstReport()
{  
    uint8_t ret = 0;   
    if( DistotalVoltageOverFirstState.status == ALARM )
      ret = 1;
    else if( DistotalVoltageUnderFirstState.status == ALARM )
      ret = 2;
    
    return ret;
}
/*=============================================================
* 函数名称：GetVoltageDifferentReport
* 函数功能：获取单体电芯状态
* 函数参数：
* 返回值：  
*           电芯电压状态
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetVoltageDifferentReport()
{
    uint8_t ret = 0;
    uint8_t alarmType = ALARM_DISCHARGE;
    if(E_CHARGE_STATUS == get_system_status())
    {
        alarmType = ALARM_CHARGE;
    }
    if( cellVoltageDifferentFirstState[alarmType].status == ALARM )
      ret = 1;
    if( cellVoltageDifferentSecondState[alarmType].status == ALARM )
      ret = 2;
    if( cellVoltageDifferentThreeState[alarmType].status == ALARM )
      ret = 3;

      return ret;
}

