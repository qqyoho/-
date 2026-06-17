#include "Appcommunication.h"
#include "dataProcess.h"
#include "system_control.h"
#include "AppTempManage.h"

AlarmConfig cellTempUnderFirstAlarm[ALARM_NUM];
AlarmConfig cellTempUnderSecondAlarm[ALARM_NUM];
AlarmConfig cellTempUnderThreeAlarm[ALARM_NUM];

AlarmConfig cellTempOverFirstAlarm[ALARM_NUM];
AlarmConfig cellTempOverSecondAlarm[ALARM_NUM];
AlarmConfig cellTempOverThreeAlarm[ALARM_NUM];

AlarmState cellTempUnderFirstState[ALARM_NUM];
AlarmState cellTempUnderSecondState[ALARM_NUM];
AlarmState cellTempUnderThreeState[ALARM_NUM];

AlarmState cellTempOverFirstState[ALARM_NUM];
AlarmState cellTempOverSecondState[ALARM_NUM];
AlarmState cellTempOverThreeState[ALARM_NUM];

AlarmConfig cellTempDifferentFirstAlarm[ALARM_NUM];
AlarmConfig cellTempDifferentSecondAlarm[ALARM_NUM];
AlarmConfig cellTempDifferentThreeAlarm[ALARM_NUM];

AlarmState cellTempDifferentFirstState[ALARM_NUM];
AlarmState cellTempDifferentSecondState[ALARM_NUM];
AlarmState cellTempDifferentThreeState[ALARM_NUM];

/* 温差告警参数，单位0.1℃，延时以100ms为单位 */
#define TEMP_SPIN_FISRT_RESTORE             (50)

#define TEMP_SPIN_VALUE_FISRT               (80)
#define TEMP_SPIN_VALUE_SECOND              (100)
#define TEMP_SPIN_VALUE_THREE               (150)

#define TEMP_SPIN_RESTORE              (50)

#define TEMP_TIMES_3S                   30
#define TEMP_TIMES_5S                   50
#define TEMP_TIMES_500MS                5

#define TEMP_TIMES_TO_100MS             10

/*==============================================================
 * 函数名称：SetAlarmConfig
 * 函数功能：配置告警参数
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-22          戴辉发             创建
 ==============================================================*/
static void SetAlarmConfig(AlarmConfig *cfg, int16_t alarmValue, int16_t recoverValue, uint16_t alarmDelay, uint16_t recoverDelay)
{
    cfg->alarmValue = alarmValue;
    cfg->recoverValue = recoverValue;
    cfg->alarmDelay = alarmDelay;
    cfg->recoverDelay = recoverDelay;
}
/*==============================================================
 * 函数名称：TempDifferentTimer
 * 函数功能：APP通信模块定时器，以100毫秒为单位
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-22          戴辉发             创建
 ==============================================================*/
void TempDifferentTimer(void)
{
    uint8_t i;
    for(i = 0; i < ALARM_NUM; i++)
    {
        if (cellTempDifferentFirstState[i].delay) cellTempDifferentFirstState[i].delay--;
        if (cellTempDifferentSecondState[i].delay) cellTempDifferentSecondState[i].delay--;
        if (cellTempDifferentThreeState[i].delay) cellTempDifferentThreeState[i].delay--;
        if (cellTempUnderFirstState[i].delay) cellTempUnderFirstState[i].delay--;
        if (cellTempUnderSecondState[i].delay) cellTempUnderSecondState[i].delay--;
        if (cellTempUnderThreeState[i].delay) cellTempUnderThreeState[i].delay--;
        if (cellTempOverFirstState[i].delay) cellTempOverFirstState[i].delay--;
        if (cellTempOverSecondState[i].delay) cellTempOverSecondState[i].delay--;
        if (cellTempOverThreeState[i].delay) cellTempOverThreeState[i].delay--;
    }
}

/*==============================================================
 * 函数名称：AppCommunicationMemInit
 * 函数功能：APP通信模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-17          戴辉发             创建
 ==============================================================*/
void TempDifferentMemInit(void)
{
    uint8_t i;
    SetAlarmConfig(&cellTempDifferentFirstAlarm[ALARM_DISCHARGE], TEMP_SPIN_VALUE_FISRT, TEMP_SPIN_FISRT_RESTORE, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempDifferentSecondAlarm[ALARM_DISCHARGE], TEMP_SPIN_VALUE_SECOND, TEMP_SPIN_RESTORE, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempDifferentThreeAlarm[ALARM_DISCHARGE], TEMP_SPIN_VALUE_THREE, TEMP_SPIN_VALUE_SECOND, TEMP_TIMES_3S, TEMP_TIMES_3S);
    cellTempDifferentFirstAlarm[ALARM_CHARGE] = cellTempDifferentFirstAlarm[ALARM_DISCHARGE];
    cellTempDifferentSecondAlarm[ALARM_CHARGE] = cellTempDifferentSecondAlarm[ALARM_DISCHARGE];
    cellTempDifferentThreeAlarm[ALARM_CHARGE] = cellTempDifferentThreeAlarm[ALARM_DISCHARGE];

    SetAlarmConfig(&cellTempUnderFirstAlarm[ALARM_DISCHARGE], -100, 0, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempUnderSecondAlarm[ALARM_DISCHARGE], -150, 0, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempUnderThreeAlarm[ALARM_DISCHARGE], -200, -150, TEMP_TIMES_3S, TEMP_TIMES_3S);
    SetAlarmConfig(&cellTempOverFirstAlarm[ALARM_DISCHARGE], 500, 450, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempOverSecondAlarm[ALARM_DISCHARGE], 550, 500, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempOverThreeAlarm[ALARM_DISCHARGE], 600, 550, TEMP_TIMES_3S, TEMP_TIMES_3S);

    SetAlarmConfig(&cellTempUnderFirstAlarm[ALARM_CHARGE], 100, 150, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempUnderSecondAlarm[ALARM_CHARGE], 50, 150, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempUnderThreeAlarm[ALARM_CHARGE], 0, 50, TEMP_TIMES_3S, TEMP_TIMES_3S);
    SetAlarmConfig(&cellTempOverFirstAlarm[ALARM_CHARGE], 430, 400, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempOverSecondAlarm[ALARM_CHARGE], 450, 400, TEMP_TIMES_5S, TEMP_TIMES_5S);
    SetAlarmConfig(&cellTempOverThreeAlarm[ALARM_CHARGE], 550, 450, TEMP_TIMES_3S, TEMP_TIMES_3S);
    for(i = 0; i < ALARM_NUM; i++)
    {
        cellTempDifferentFirstState[i].status = NORMAL;
        cellTempDifferentSecondState[i].status = NORMAL;
        cellTempDifferentThreeState[i].status = NORMAL;
        cellTempUnderFirstState[i].status = NORMAL;
        cellTempUnderSecondState[i].status = NORMAL;
        cellTempUnderThreeState[i].status = NORMAL;
        cellTempOverFirstState[i].status = NORMAL;
        cellTempOverSecondState[i].status = NORMAL;
        cellTempOverThreeState[i].status = NORMAL;
    }
}

/*==============================================================
 * 函数名称：TempDifferentProcess
 * 函数功能：电芯温度温差3级告警分析
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-18          戴辉发             创建
 ==============================================================*/
void TempDifferentProcess(void)
{
    uint8_t i;
    uint8_t alarmType = ALARM_DISCHARGE;
    uint8_t clearType = ALARM_CHARGE;
    uint16_t spin;
    int16_t min_cell_temp;
    int16_t max_cell_temp;
    spin = get_max_cell_temp() - get_min_cell_temp();
    min_cell_temp = get_min_cell_temp();
    max_cell_temp = get_max_cell_temp();
    if(E_CHARGE_STATUS == get_system_status())
    {
        alarmType = ALARM_CHARGE;
        clearType = ALARM_DISCHARGE;
    }
    for(i = 0; i < ALARM_NUM; i++)
    {
        if(i == clearType)
        {
            cellTempDifferentFirstState[i].status = NORMAL;
            cellTempDifferentFirstState[i].delay = 0;
            cellTempDifferentSecondState[i].status = NORMAL;
            cellTempDifferentSecondState[i].delay = 0;
            cellTempDifferentThreeState[i].status = NORMAL;
            cellTempDifferentThreeState[i].delay = 0;
            cellTempUnderFirstState[i].status = NORMAL;
            cellTempUnderFirstState[i].delay = 0;
            cellTempUnderSecondState[i].status = NORMAL;
            cellTempUnderSecondState[i].delay = 0;
            cellTempUnderThreeState[i].status = NORMAL;
            cellTempUnderThreeState[i].delay = 0;
            cellTempOverFirstState[i].status = NORMAL;
            cellTempOverFirstState[i].delay = 0;
            cellTempOverSecondState[i].status = NORMAL;
            cellTempOverSecondState[i].delay = 0;
            cellTempOverThreeState[i].status = NORMAL;
            cellTempOverThreeState[i].delay = 0;
        }
    }
    ValueCompare(min_cell_temp, &cellTempUnderFirstAlarm[alarmType] , -1 , &cellTempUnderFirstState[alarmType]);
    ValueCompare(min_cell_temp, &cellTempUnderSecondAlarm[alarmType] , -1 , &cellTempUnderSecondState[alarmType]);
    ValueCompare(min_cell_temp, &cellTempUnderThreeAlarm[alarmType] , -1 , &cellTempUnderThreeState[alarmType]);
    ValueCompare(max_cell_temp, &cellTempOverFirstAlarm[alarmType] , 1 , &cellTempOverFirstState[alarmType]);
    ValueCompare(max_cell_temp, &cellTempOverSecondAlarm[alarmType] , 1 , &cellTempOverSecondState[alarmType]);
    ValueCompare(max_cell_temp, &cellTempOverThreeAlarm[alarmType] , 1 , &cellTempOverThreeState[alarmType]);
    ValueCompare(spin,&cellTempDifferentFirstAlarm[alarmType],1,&cellTempDifferentFirstState[alarmType]);
    ValueCompare(spin,&cellTempDifferentSecondAlarm[alarmType],1,&cellTempDifferentSecondState[alarmType]);
    ValueCompare(spin,&cellTempDifferentThreeAlarm[alarmType],1,&cellTempDifferentThreeState[alarmType]);
}

/*=============================================================
* 函数名称：GetCellTempDifferentReport
* 函数功能：获取电芯温度温差状态
* 函数参数：
* 返回值：  
*           电芯温度温差状态
* 修改记录:
*==============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
 uint8_t GetCellTempDifferentReport()
 {
   uint8_t ret = 0;
   uint8_t alarmType = ALARM_DISCHARGE;
   if(E_CHARGE_STATUS == get_system_status())
   {
     alarmType = ALARM_CHARGE;
   }
   if( cellTempDifferentFirstState[alarmType].status == ALARM )
     ret = 1;
   if( cellTempDifferentSecondState[alarmType].status == ALARM )
     ret = 2;
   if( cellTempDifferentThreeState[alarmType].status == ALARM )
     ret = 3;
   return ret;
 }

/*=============================================================
* 函数名称：GetDischTempFirstReport
* 函数功能：获取单体电芯状态
* 函数参数：
* 返回值：  
*           电芯电压状态
* 修改记录:
*==============================================================
* 日期                修改人             修改内容
* 2024-03-19         liyong             创建
==============================================================*/
uint8_t GetCellTempFirstReport()
{
  uint8_t ret = 0;
  uint8_t alarmType = ALARM_DISCHARGE;
  if(E_CHARGE_STATUS == get_system_status())
  {
    alarmType = ALARM_CHARGE;
  }
  if( cellTempUnderFirstState[alarmType].status == ALARM )
    ret = 1;
  if( cellTempUnderSecondState[alarmType].status == ALARM )
    ret = 2;
  if( cellTempUnderThreeState[alarmType].status == ALARM )
    ret = 3;
  if ((cellTempOverFirstState[alarmType].status == ALARM) ||
      (cellTempOverSecondState[alarmType].status == ALARM) ||
      (cellTempOverThreeState[alarmType].status == ALARM))
    ret = 4;
  return ret;
}
