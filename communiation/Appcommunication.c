#include "Appcommunication.h"
#include "dataProcess.h"
#include "system_control.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "AppVolManage.h"
#include "AppTempManage.h"
#include "AppCurrManage.h"


/*==============================================================
 * 函数名称：AppCommonMemInit
 * 函数功能：APP模块内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-17          戴辉发             创建
 ==============================================================*/
void AppCommonMemInit(void)
{
    VoltageMemoryWake_mem_init();
    Alarm_current_mem_init();
    TempDifferentMemInit();
}
/*==============================================================
 * 函数名称：AppCommonTimer
 * 函数功能：APP模块定时器，100毫秒定时器
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-18          戴辉发             创建
 ==============================================================*/
void AppCommonTimer(void)
{
    APPVoltageTimerDelay();
    CurrentStateTimer();
    TempDifferentTimer();
}
/*==============================================================
 * 函数名称：AppCommonProcess
 * 函数功能：APP模块处理流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2022-07-18          戴辉发             创建
 ==============================================================*/
void AppCommonProcess(void)
{
    if (judge_vol_sample_finished())
    {
        VoltageAlarmJudge();
        TempDifferentProcess();
        CurrentAlarmJudge();
    }
}