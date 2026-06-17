/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：adc_sampling.c
 * 文件功能描述：实现AD采集
 *
 * 修改记录：
 * 2018-06-25 戴辉发 创建
*----------------------------------------------------------*/
#include "hardware.h"
#include "adc_sampling.h"
#include "current_manage.h"
#include "vol_manage.h"
#include "temp_manage.h"
#include "hardware.h"
#include "balance.h"
#include "fm33lg0xx_fl.h"
#include "afe_app.h"
#include "parameter.h"
#include "system_adjust.h"

#if (defined(BAT_13S) || defined(BAT_14S)||defined(BAT_15S))
#define  TOTAL_VOLTAGE_A    (2111)
#else
#define  TOTAL_VOLTAGE_A    (3111)
#endif

#define MAX_ADC_VALUE_NUM       8
#define VREFCAL                 0x1FFFFB08
#define VREFRAW                 0x1FFFFB0C

static volatile uint8_t t_get_curr_delay;
static uint16_t g_adc_value_buf[MAX_ADC_VALUE_NUM];

static uint8_t g_step_adc;
static e_ad_ch_type g_t_status;
static uint32_t g_adc_vref;
static uint16_t vcc3v3_vaule;
#if 0
static int16_t hallCurrent;
#endif
static uint32_t g_vref_cal;
static uint32_t g_vref_raw;
uint16_t totalVoltage;
static uint16_t packVoltage;
static uint8_t needCollectPackVoltageFlag;
/*==============================================================
 * 函数名称：adc_mem_init
 * 函数功能：ADC MEM管脚初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2019-05-14         戴辉发             创建
==============================================================*/
void adc_mem_init(void)
{
    t_get_curr_delay = 0;
    g_step_adc = 0;
    g_t_status = CH_AMBIENT_TEMP_ADS;
    g_vref_cal = *(uint32_t *)(VREFCAL);
    g_vref_cal &= 0x0ffff;
    g_vref_raw = *(uint32_t *)(VREFRAW);
    g_vref_raw &= 0x0ffff;
    needCollectPackVoltageFlag = 0;
}

/*==============================================================
 * 函数名称：adc_sample_timer
 * 函数功能：ADC定时器流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2019-05-14         戴辉发             创建
==============================================================*/
void adc_sample_timer(void)
{
    if (t_get_curr_delay) t_get_curr_delay --;
}

void AdcInit(void)
{
	FL_ADC_CommonInitTypeDef    ADC_CommonInitStruct;
	FL_ADC_InitTypeDef          ADC_InitStruct ;
 
	//ADC 时钟设置
	ADC_CommonInitStruct.operationSource    	    = FL_ADC_CLK_SOURCE_ADCCLK;
	ADC_CommonInitStruct.clockSource 				= FL_CMU_ADC_CLK_SOURCE_PLL;
	ADC_CommonInitStruct.clockPrescaler    	        = FL_CMU_ADC_PSC_DIV4;
	ADC_CommonInitStruct.APBClockPrescaler		    = FL_ADC_APBCLK_PSC_DIV1;
	ADC_CommonInitStruct.referenceSource            = FL_ADC_REF_SOURCE_VDDA;
	ADC_CommonInitStruct.bitWidth     				= FL_ADC_BIT_WIDTH_12B;	
	FL_ADC_CommonInit(&ADC_CommonInitStruct);
	
	//ADC 寄存器设置
	ADC_InitStruct.conversionMode                   = FL_ADC_CONV_MODE_SINGLE;//单次模式
	ADC_InitStruct.autoMode                         = FL_ADC_SINGLE_CONV_MODE_AUTO;//自动
	ADC_InitStruct.waitMode 		                = FL_ENABLE;//等待 
	ADC_InitStruct.overrunMode 		   	            = FL_ENABLE;//覆盖上次数据		
	ADC_InitStruct.scanDirection      		        = FL_ADC_SEQ_SCAN_DIR_FORWARD;//通道正序扫描
	ADC_InitStruct.externalTrigConv   		        = FL_ADC_TRIGGER_EDGE_NONE;//使能触发信号
	ADC_InitStruct.triggerSource					= FL_ADC_TRGI_LUT0;
	ADC_InitStruct.fastChannelTime   			    = FL_ADC_FAST_CH_SAMPLING_TIME_8_ADCCLK;//快速通道采样时间
	ADC_InitStruct.lowChannelTime  				    = FL_ADC_SLOW_CH_SAMPLING_TIME_256_ADCCLK;//慢速通道采样时间
	ADC_InitStruct.oversamplingMode                 = FL_ENABLE;//过采样关闭
	ADC_InitStruct.overSampingMultiplier            = FL_ADC_OVERSAMPLING_MUL_8X;//8倍过采样
	ADC_InitStruct.oversamplingShift                = FL_ADC_OVERSAMPLING_SHIFT_3B;//数据右移, /8	
	FL_ADC_Init(ADC, &ADC_InitStruct);
    
    FL_ADC_EnableBuffer(ADC);
    FL_ADC_EnableBufferChopper(ADC);
    FL_ADC_DisableOverSampling(ADC);
    FL_ADC_Enable(ADC);
    FL_ADC_EnableCalibration(ADC);
    while( FL_ADC_IsActiveFlag_EndOfCalibration(ADC) == 0 );
    FL_ADC_ClearFlag_EndOfCalibration(ADC);
    FL_ADC_DisableCalibration(ADC);
    FL_ADC_EnableOverSampling(ADC);
    FL_ADC_Disable(ADC);    // 关闭ADC
}

void ADC_DMA(uint16_t *buffer, uint32_t length)
{
    FL_DMA_InitTypeDef DMA_InitStruct={0};
	FL_DMA_ConfigTypeDef DMA_ConfigStruct={0};
    DMA_InitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;		
    DMA_InitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;	
    DMA_InitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;	
    DMA_InitStruct.dataSize = FL_DMA_BANDWIDTH_16B;	
    DMA_InitStruct.priority = FL_DMA_PRIORITY_HIGH;		
    DMA_InitStruct.circMode = FL_ENABLE;
    FL_DMA_Init(DMA, &DMA_InitStruct, FL_DMA_CHANNEL_4);		
     
    FL_DMA_Enable(DMA); 
    DMA_ConfigStruct.memoryAddress = (uint32_t)buffer; 
    DMA_ConfigStruct.transmissionCount = length - 1;
	FL_DMA_StartTransmission(DMA, &DMA_ConfigStruct, FL_DMA_CHANNEL_4);	
}

uint32_t GetVREF1P2Sample(void)
{
	uint32_t tmp;
	uint8_t i;
    
  //  FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV8);
    FL_VREF_EnableVREFBuffer(VREF);//使能VREF BUFFER
	FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);//通道选择VREF
	ADC_DMA(g_adc_value_buf,MAX_ADC_VALUE_NUM);
	FL_ADC_SetConversionMode(ADC, FL_ADC_CONV_MODE_CONTINUOUS);
    
	FL_ADC_ClearFlag_EndOfConversion(ADC);//清标志			
    FL_ADC_Enable(ADC);   	 // 启动ADC
	FL_ADC_EnableSWConversion(ADC);  // 开始转换
     // 等待转换完成
	while (!FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4));
	FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4);

    FL_ADC_Disable(ADC);    // 关闭ADC
	FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);//通道关闭VREF	
    FL_VREF_DisableVREFBuffer(VREF);//关闭VREF BUFFER	
    
    tmp = 0;
    for (i = MAX_ADC_VALUE_NUM / 2; i < MAX_ADC_VALUE_NUM;i++ )
    {
        tmp += g_adc_value_buf[i];
    }
    tmp = tmp / (MAX_ADC_VALUE_NUM / 2);
    return tmp;
}

/*==============================================================
 * 函数名称：adc_init
 * 函数功能：ADC初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
void adc_init(void)
{
    AdcInit();
}

/*==============================================================
 * 函数名称：adc_get_sampling_value
 * 函数功能：ADC获取采样数据
 * 参数个数：1
 * 函数参数：
 *           [IN]      adc_chanel         ADC通道
 * 返 回 值：
 *           无
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
int32_t adc_get_sampling_value(e_ad_ch_type adc_chanel )
{
	uint16_t i;
    int32_t tmp; 
    uint8_t open = 0;

    if(( FL_ADC_EXTERNAL_CH14 == adc_chanel )||
       ( FL_ADC_EXTERNAL_CH15 == adc_chanel )||
       ( FL_ADC_EXTERNAL_CH16 == adc_chanel )||
       ( FL_ADC_EXTERNAL_CH17 == adc_chanel ))
        open = 1;
     
    if( open == 1 )
        FL_VREF_EnableVREFBuffer(VREF);//使能VREF BUFFER
	FL_ADC_EnableSequencerChannel(ADC, adc_chanel);//通道选择ADC_1
    ADC_DMA(g_adc_value_buf,MAX_ADC_VALUE_NUM);
	FL_ADC_SetConversionMode(ADC, FL_ADC_CONV_MODE_CONTINUOUS);
     
	FL_ADC_ClearFlag_EndOfConversion(ADC);//清标志			
	FL_ADC_Enable(ADC);   	 // 启动ADC
	FL_ADC_EnableSWConversion(ADC);  // 开始转换
   // 等待转换完成
	while (!FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4));
	FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_4);

    FL_ADC_Disable(ADC);    // 关闭ADC
	FL_ADC_DisableSequencerChannel(ADC, adc_chanel);//通道关闭ADC_1
	//FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);//通道关闭VREF	
    if( open == 1 )
        FL_VREF_DisableVREFBuffer(VREF);//关闭VREF BUFFER			
    // 转换结果 
      
    tmp = 0;
    for (i = 0; i < MAX_ADC_VALUE_NUM;i++ )
    {
        tmp += g_adc_value_buf[i];
    }
    tmp = tmp / MAX_ADC_VALUE_NUM;
    return tmp;
}

/*=============================================================
 * 函数名称：get_voltage_process
 * 函数功能：获取流程
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-05-05        戴辉发     	创建
==============================================================*/
uint16_t get_voltage_process(int32_t ad_value)
{
    float temp;
    /* 转换为电压值 */
    temp = 3.0 * g_vref_cal * ad_value / g_adc_vref / 4095;
	return (uint16_t)(temp * TOTAL_VOLTAGE_A);
}

/*=============================================================
 * 函数名称：HallCurrentAdcProcess
 * 函数功能：霍尔电流的adc处理
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2021-11-17        liyong     	创建
==============================================================*/
#if 0
void HallCurrentAdcProcess(int32_t ad_value)
{
   /* 转换为电压值 */
    hallCurrent = (int16_t)(3000.0 * g_vref_cal * ad_value / g_adc_vref / 4095); 
}
#endif
/*=============================================================
 * 函数名称：HallCurrentAdcProcess
 * 函数功能：霍尔电流的adc处理
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2021-11-17        liyong     	创建
==============================================================*/
void PackVoltageAdcProcess(int32_t ad_value)
{
    float temp;
   /* 转换为电压值 */
    temp = 3.0 * g_vref_cal * ad_value / g_adc_vref / 4095; 
    packVoltage = (uint16_t)(temp * TOTAL_VOLTAGE_A);
}
/*=============================================================
 * 函数名称：uint16_t GetTotalVoltage(void)
 * 函数功能：获取总电压
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-03-31         李勇     	创建
==============================================================*/
uint16_t GetTotalVoltage(void)
{ 
    return totalVoltage; //0.01V/bit
}

/*=============================================================
 * 函数名称：uint16_t GetPackVoltage(void)
 * 函数功能：获取pack电压
 * 参数个数：1
 * 参数描述：
 *          [IN]     ad_value    AD值
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2020-03-31         李勇     	创建
==============================================================*/
uint16_t GetPackVoltage(void)
{
    return packVoltage; //0.01V/bit
}
/*=============================================================
 * 函数名称：setvcc3v3_vaule()
 * 函数功能：设置VCC3V3的值
 * 参数个数：1
 * 参数描述：
 *          [IN]     value    
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2021-07-12         李勇     	创建
==============================================================*/
void setvcc3v3_vaule(uint16_t vaule)
{ 
    vcc3v3_vaule = 2 * vaule;//1mv/bit
}

/*=============================================================
 * 函数名称：uint16_t get_vcc3v3_vaule( void )
 * 函数功能：获取VCC3V3的值
 * 参数个数：1
 * 参数描述：
 *          [IN]     value    
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2021-07-12         李勇     	创建
==============================================================*/
uint16_t get_vcc3v3_vaule( void )
{ 
    return vcc3v3_vaule; //1mv/bit
}
/*=============================================================
 * 函数名称：SetCollectPackVoltageFlag
 * 函数功能：开启或者停止pack电压采集
 * 参数个数：1
 * 参数描述：
 *          [IN]     value    
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2021-11-22         李勇     	创建
==============================================================*/
void SetCollectPackVoltageFlag(uint8_t flag)
{
     needCollectPackVoltageFlag = flag;
}

/*=============================================================
 * 函数名称：adc_data_get_process
 * 函数功能：电流获取过程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 * 修改记录：
 *==============================================================
 * 日期                修改人             修改内容
 * 2019-05-14          戴辉发             创建
==============================================================*/
void adc_data_get_process(void)
{
    int32_t temp;

    
	if(t_get_curr_delay) return;
	
    t_get_curr_delay = 25;  //0.125s更新一次

	switch( g_step_adc )
	{
        case 0: /* 总电压 辅助判断值 */
            //g_adc_vref = GetVREF1P2Sample();
            totalVoltage = g_run_sys_data.total_vol;//calaclute_total_voltage(get_voltage_process(adc_get_sampling_value(TOTAL_VOLTAGE_ADS)));   
            g_step_adc = 1;
            break;
        case 1: /* 功率和环境温度 */       
           temp = adc_get_sampling_value(g_t_status);
           setvcc3v3_vaule(adc_get_sampling_value(CH_V3V3_ADS));
           temperature_get_data(temp, &g_t_status);
           g_step_adc = 0;
           break;
        default:
            g_step_adc = 0;
            break;
	}
#if 0
            /*霍尔电流采集*/    
           temp = adc_get_sampling_value(HALL_CURRENT_ADS);
           HallCurrentAdcProcess(temp);
#endif
     
}
