#include "rtc.h"
#include "parameter.h"
#include "idog.h"
#include "fm33lg0xx_fl.h"

#define RTC_FLAG_BKP      0x5AA55AA5
#define BCD_TO_DECIMAL(x)   (((x&0xf0)>>4)*10+(x&0x0f)) 

uint8_t g_datetime[DATATIME_LENGTH] = {'2','0','2','1',  '0','1',  '0','1',  '0','0', '0','0', '0','0','4'};
//***********************************************1**2****3***4****5***6**7****8***9***10**11**12
#define  YEAR_SECOND    (3600*24*365)
#define  DAY_SECOND     (3600*24)
#define  HOUR_SECOND    (3600)
#define  MINUTE_SECOND  (60)

uint32_t test_rtc;
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName:rtc_datetime_init
**Create  : weihl @xxxx-xx-xx
**Modify  : 
*******************************************************************************/
void rtc_datetime_init(void)
{
    /* 使能时钟 */
    //FL_CMU_RCLP_Enable();
    //上电默认情况下，XTLF时钟已经使能
    FL_CMU_SetLSCLKClockSource( FL_CMU_LSCLK_SOURCE_XTLF ); //
    //SCLK的自动切换功能，由LSACTS寄存器配置，仅在XTLF使能的情况下有效，此时假设XTLF是
    //主时钟，RCLP是备份时钟，仅用于防止XTLF异常停振。所以LSCATS只在XTLF使能的情况下起作
    //用，当XTLF出现意外停振，FDET输出的停振信号将LSCLK自动切换到RCLP。
    do
    {
      FL_CMU_EnableLSCLKAutoSwitch();
    }while( FL_CMU_IsEnabledLSCLKAutoSwitch() == 0 );
    
    FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA); 
    FL_RTCA_WriteAdjustValue (RTCA, 0); 
//    FL_CMU_DisableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);
//    FL_RTCB_WriteAdjustValue (RTCB, 0); 
    
    /*使能CDIF*/
    FL_CDIF_EnableVAOToCPU(CDIF);
    FL_CDIF_EnableCPUToVAO(CDIF);
    FL_RTCA_Enable(RTCA);     
    //是否需要设置初始化时间  
    test_rtc = FL_RTCB_ReadBackupRegisters(RTCB,FL_RTCB_BACKUP_0);
    if( test_rtc != RTC_FLAG_BKP )
    {
        rtc_set_datetime();
        FL_RTCB_WriteBackupRegisters( RTCB, RTC_FLAG_BKP, FL_RTCB_BACKUP_0 );
    }
    
    NVIC_DisableIRQ(RTCx_IRQn); 
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: rtc_set_datetime
**Create  : weihl @xxxx-xx-xx
**Modify  : 
*******************************************************************************/
void rtc_set_datetime(void)
{
    FL_RTCA_InitTypeDef  TempTime;
    TempTime.year   = ((g_datetime[2]-0x30)<<4)+(g_datetime[3]-0x30);
    TempTime.month  = ((g_datetime[4]-0x30)<<4)+(g_datetime[5]-0x30);
    TempTime.day    = ((g_datetime[6]-0x30)<<4)+(g_datetime[7]-0x30);
    TempTime.week   = (g_datetime[14]-0x30);
    TempTime.hour   = ((g_datetime[8]-0x30)<<4)+(g_datetime[9]-0x30);
    TempTime.minute = ((g_datetime[10]-0x30)<<4)+(g_datetime[11]-0x30);
    TempTime.second = ((g_datetime[12]-0x30)<<4)+(g_datetime[13]-0x30);
    FL_RTCA_Init( RTCA, &TempTime );
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: rtc_wake_up_config();//
**Create  : weihl @xxxx-xx-xx
**Modify  : 
*******************************************************************************/
void rtc_wake_up_config(void)
{
    /* 使能时间配置 */
    FL_RTCA_WriteEnable(RTCA);
    FL_RTCA_ClearFlag_Second(RTCA);        //清除秒中断标志
    FL_RTCA_EnableIT_Second(RTCA);

    NVIC_DisableIRQ(RTCx_IRQn);                //NVIC中断控制器配置
    NVIC_SetPriority(RTCx_IRQn, 2);
    NVIC_EnableIRQ(RTCx_IRQn);
    /* 锁定时间配置 */
    FL_RTCA_WriteDisable(RTCA);
}

/*=============================================================
* 函数名称：rtc_wake_up_close_config
* 函数功能：关闭RTC唤醒中断
* 参数个数：0
* 参数描述：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-07-12       	戴辉发     	创建
==============================================================*/
void rtc_wake_up_close_config(void)
{
    /* 使能时间配置 */
    FL_RTCA_WriteEnable(RTCA);
    FL_RTCA_ClearFlag_Second(RTCA);        //清除秒中断标志
    FL_RTCA_DisableIT_Second(RTCA);

    NVIC_DisableIRQ(RTCx_IRQn);            //NVIC中断控制器配置
    /* 锁定时间配置 */
    FL_RTCA_WriteDisable(RTCA);
}

/*=============================================================
* 函数名称：RTC_Alarm_IRQHandler
* 函数功能：RTC唤醒中断
* 参数个数：0
* 参数描述：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2020-09-08        liyong     	创建
==============================================================*/
void RTC_IRQHandler(void)
{
	if(FL_ENABLE == FL_RTCA_IsEnabledIT_Second(RTCA) &&
            FL_SET == FL_RTCA_IsActiveFlag_Second(RTCA))//查询秒中断标志是否置起
    {
        feed_hard_owdg();
        FL_RTCA_ClearFlag_Second(RTCA);        //清除秒中断标志
    }
}
//*******************************************1***2***3***4****5**6***7****8***9***10**11**12

/*=============================================================
* 函数名称：get_rtc_time
* 函数功能：获取rtc时间
* 参数个数：0
* 参数描述：
* 返 回 值：
*           
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2020-06-14       	李勇    	创建
==============================================================*/
void get_rtc_time(void)
{
    uint8_t n,i,Result = 1;
    uint16_t year;
    FL_RTCA_InitTypeDef TempTime1, TempTime2;

    for( n = 0 ; n < 3; n++ )
    {
        FL_RTCA_GetTime(RTCA, &TempTime1); //读一次时间
        FL_RTCA_GetTime(RTCA, &TempTime2); //再读一次时间

        for( i = 0; i < 7; i++ ) //两者一致, 表示读取成功
        {
            if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(&TempTime2))[i]) 
            { 
               break; 
            }
        }

        if(i == 7)
        {
            Result = 0;
            break;
        }
    }
    
    if( Result == 0 )
    {
       /* 保留日期 */
        year = 2000+BCD_TO_DECIMAL(TempTime1.year);
        
        g_run_sys_data.write_time[0] = (year >> 8) & 0xff;
        g_run_sys_data.write_time[1] = (year & 0xff);
        g_run_sys_data.write_time[2] = BCD_TO_DECIMAL(TempTime1.month);
        g_run_sys_data.write_time[3] = BCD_TO_DECIMAL(TempTime1.day);
        /* 保留时间 */
        g_run_sys_data.write_time[4] = BCD_TO_DECIMAL(TempTime1.hour);
        g_run_sys_data.write_time[5] = BCD_TO_DECIMAL(TempTime1.minute);
        g_run_sys_data.write_time[6] = BCD_TO_DECIMAL(TempTime1.second);
    }
    
}
