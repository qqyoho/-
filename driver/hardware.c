/*---------------------------------------------------------*
* Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
*
* 文件名：hardware.c
* 文件功能描述：实现系统底层驱动
*
* 修改记录：
* 2018-07-06 戴辉发 创建
*----------------------------------------------------------*/
#include <stdlib.h>
#include "hardware.h"
#include "eeprom.h"
#include "timer.h"
#include "fm33lg0xx_fl.h"
#include "idog.h"
#include "adc_sampling.h"
#include "bluetooth.h"
#include "power.h"
#include "can.h"
#include "rtc.h"
#include "afe_app.h"

uint8_t cr_irq;
void FDET_IRQHandler(void)
{
    cr_irq++;
    if(FL_CMU_IsActiveFlag_XTHFFail())
    {
        FL_CMU_ClearFlag_XTHFFail();
    }
}

/*=============================================================
 * 函数名称：init_sysclk
 * 函数功能：初始化系统时钟
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录: 
 *===============================================================
 * 日期                修改人           修改内容
 * 2021-06-01          李勇             创建
==============================================================*/
void system_clock_init(void)
{
    do/* 设置Flash读等待周期 */
    {
        FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_2CYCLE);
    }while( FL_FLASH_GetReadWait(FLASH) != FL_FLASH_READ_WAIT_2CYCLE );

    /* 设置XTHF(片外高速晶振)振荡强度，低0~高31 */
    do
    {
        FL_CMU_XTHF_WriteDriverStrength(31);
    }while( FL_CMU_XTHF_ReadDriverStrength() != 31 );

    do/* 使能XTHF时钟 */
    {
        FL_CMU_XTHF_Enable();
    }while(FL_CMU_XTHF_IsEnabled() != 1);

    delay_ms(10);

    /* 设置XTHF为系统时钟源 */
    do
    {
        /* 选择PLL输入时，必须保证RCHF或XTHF为使能状态 */
        FL_CMU_PLL_SetClockSource(FL_CMU_PLL_CLK_SOURCE_XTHF);
    }while(FL_CMU_PLL_GetClockSource() != FL_CMU_PLL_CLK_SOURCE_XTHF);

    /* 设置PLL时钟预分频，8分频产生1MHz时钟给PLL */
    do
    {
        FL_CMU_PLL_SetPrescaler(FL_CMU_PLL_PSC_DIV8);
    }while(FL_CMU_PLL_GetPrescaler() != FL_CMU_PLL_PSC_DIV8);

    /* 设置PLL时钟倍频比：32-1, 48-1, 64-1 */
    do
    {
        FL_CMU_PLL_WriteMultiplier(48-1);
    }while(FL_CMU_PLL_ReadMultiplier() != (48-1));

    /* 设置PLL时钟输出倍乘数，1MHz X1 = 1MHz */
    do
    {
        FL_CMU_PLL_SetOutputMultiplier(FL_CMU_PLL_OUTPUT_X1);
    }while(FL_CMU_PLL_GetOutputMultiplier() != FL_CMU_PLL_OUTPUT_X1);

    /* 使能PLL锁相环 */
    do
    {
        FL_CMU_PLL_Enable();
    }while(!FL_CMU_PLL_IsEnabled());

    /* 等待PLL锁定后，才能将SYSCLK配置为PLL输出 */
    while(!FL_CMU_IsActiveFlag_PLLReady());

    /* 设置SYSCLK时钟源为PLL时钟 */
    do
    {
        FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_PLL);
    }while(FL_CMU_GetSystemClockSource() != FL_CMU_SYSTEM_CLK_SOURCE_PLL);

    /* 设置APB时钟1分频 */
    do
    {
        FL_CMU_SetAPBPrescaler(FL_CMU_APBCLK_PSC_DIV1);
    }while(FL_CMU_GetAPBPrescaler() != FL_CMU_APBCLK_PSC_DIV1);

    /* 设置AHB时钟1分频 */
    do
    {
        FL_CMU_SetAHBPrescaler(FL_CMU_AHBCLK_PSC_DIV1);
    }while(FL_CMU_GetAHBPrescaler() != FL_CMU_AHBCLK_PSC_DIV1);

    /* 设置SysTick时钟源为系统时钟 */
    do
    {
        FL_CMU_SetSysTickClockSource(FL_CMU_SYSTICK_CLK_SOURCE_SYSCLK);
    }while(FL_CMU_GetSysTickClockSource() != FL_CMU_SYSTICK_CLK_SOURCE_SYSCLK);

    FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV8);
    FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_2CYCLE);
    FL_FLASH_EnablePrefetch(FLASH);
    FL_FLASH_EnablePftbuf(FLASH);
    FL_CMU_EnableIT_XTHFFail(); 
    FL_CMU_EnableLSCLKAutoSwitch();
}

/*=============================================================
 * 函数名称：init_sysclk
 * 函数功能：初始化系统时钟
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录: 
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-09-3           李勇               创建
==============================================================*/
void init_sysclk(void)
{
	system_clock_init();
}

/*=============================================================
 * 函数名称：delay_init
 * 函数功能：延时函数初始化
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void delay_init(void)	 
{
	FL_DelayInit();                        
}

/*=============================================================
 * 函数名称：close_systick
 * 函数功能：关闭systick
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-25          戴辉发             创建
==============================================================*/
void close_systick(void)	 
{
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk);
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

/*=============================================================
 * 函数名称：delay_us
 * 函数功能：延时指定us函数
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
   最长延时60ms；所以不用喂狗了。
 *=============================================================
 * 日期                修改人             修改内容
 * 2018-07-10          戴辉发             创建
==============================================================*/
void delay_us(uint16_t nus)
{
	FL_DelayUs(nus);
}

/*=============================================================
 * 函数名称：delay_ms
 * 函数功能：延时指定ms函数
 * 参数个数：0
 * 函数参数：
 * 返回值：  
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-10          戴辉发             创建
==============================================================*/
void delay_ms(uint16_t nms)
{
	FL_DelayMs(nms);
}

///*=============================================================
// * 函数名称：delay_ms_no_feed
// * 函数功能：延时指定ms函数
// * 参数个数：0
// * 函数参数：
// * 返回值：  
// *           无
// * 修改记录:
// *===============================================================
// * 日期                修改人             修改内容
// * 2020-10-23          戴辉发             创建
//==============================================================*/
//void delay_ms_no_feed(uint16_t nms)
//{
//	uint32_t temp = 0;
//    uint32_t pre_val = 0;
//
//    SysTick->LOAD = nms * fac_ms;               //时间加载
//	SysTick->VAL = 0x00;                        //清空计数器
//	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   //开始倒数
//	do
//	{
//		temp = SysTick->CTRL;
//
//        if (SysTick->VAL >= (pre_val + fac_ms))
//        {
//            pre_val = SysTick->VAL;
//        }
//	}
//	while((temp&0x01) && !(temp&(1<<16)));      //等待时间到达   
//	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //关闭计数器
//	SysTick->VAL = 0x00;                        //清空计数器	 
//}
//


/*=============================================================
* 函数名称：gpio_hard_init
* 函数功能：GPIO输入输出初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void gpio_hard_init(void)
{
      FL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /*
    Pin2：
        端 口 号：PA13
        网 络 名：FewdCtr
        电平描述：高低切换输出，看门狗芯片只检测高低电平沿的切换时间间隔
                  手册参考喂狗时间：1.12S(Min)，1.6S(Type)，2.4S(Max)
                  调试时禁用MCU片外看门狗，只需将该脚配置为！
        说    明：MCU片外看门狗喂狗脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉，高低切换输出高低切换时间以500mS±100mS为宜，不得超过1S！
                  睡眠模式：输出功能(Output)，禁用开漏和上拉，高低切换输出高低切换时间以500mS±100mS为宜，不得超过1S！
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_13 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_13 );
#endif
    
    /*
    Pin3：
        端 口 号：PA14
        网 络 名：PwrTempAds
        电平描述：模拟功能，ADC_IN11
        说    明：功率温度NTC两端电压AD采样
                  连接方式：V3V3电源连接10K/0.5%固定电阻，再连接NTC10K/B3435到地，
                            采样点为固定电阻和NTC电阻的中间节点！
                  Vntc = Vadc x 1,
                  Rntc = Vntc x 10K / (V3V3 - Vntc) = ?K,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_14;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin4：
        端 口 号：PA15
        网 络 名：LedBlueCtr
        电平描述：输入H~~~控制运行灯点亮
                  输入L~~~控制运行灯熄灭
        说    明：蓝色运行灯亮灭控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_15 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_15;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_15 );
    
    /*
    Pin5：
        端 口 号：PA0
        网 络 名：LedRedCtr
        电平描述：输入H~~~控制告警灯点亮
                  输入L~~~控制告警灯熄灭
        说    明：红色告警灯亮灭控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_0 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_0;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_0 );
    
    /*
    Pin6：
        端 口 号：PA1
        网 络 名：Iso5VDet
        电平描述：输入H~~~检测到隔离电源上电成功
                  输入L~~~检测到隔离电源下电成功
        说    明：隔离电源上下电检测脚
                  工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断*/
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /*Pin7：
        端 口 号：PA2
        网 络 名：Uart0_RX
        电平描述：数字功能，Uart0_RX
        说    明：Uart通信数据接收脚，用于UART或RS485通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin8：
        端 口 号：PA3
        网 络 名：Uart0_TX
        电平描述：数字功能，Uart0_TX
        说    明：Uart通信数据发送脚，用于UART或RS485通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin9：
        端 口 号：PA4
        网 络 名：RS485/RTCtr
        电平描述：输出H~~~控制RS485收发器为数据发送方向
                  输出L~~~控制RS485收发器为数据接收方向
        说    明：RS485收发器数据收发方向切换脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_4 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_4;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_4 );
    
    /*
    Pin10：
        端 口 号：PA5
        网 络 名：KeyWkupInt
        电平描述：输入H~~~检测到叉车钥匙开关接通
                  输入L~~~检测到叉车钥匙开关断开
        说    明：钥匙开关开关机检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin11：
        端 口 号：PA6
        网 络 名：CAN_RX
        电平描述：数字功能，CAN_RX
        说    明：CAN通信数据接收脚
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin12：
        端 口 号：PA7
        网 络 名：CAN_TX
        电平描述：数字功能，CAN_TX
        说    明：CAN通信数据发送脚
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin13：
        端 口 号：PA8
        网 络 名：ChgCntWkupInt
        电平描述：输入H~~~检测到充电机短接触点(干接点)接通
                  输入L~~~检测到充电机短接触点(干接点)断开
        说    明：充电机接入检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin14：
        端 口 号：PA9
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin15：
        端 口 号：PA10
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin16：
        端 口 号：PA11
        网 络 名：EepI2C_SCL
        电平描述：数字功能，I2C_SCL
        说    明：I2C1通信时钟脚，MCU做主机和Eeprom（BL24C256）通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin17：
        端 口 号：PA12
        网 络 名：EepI2C_SDA
        电平描述：数字功能，I2C_SDA
        说    明：I2C1通信数据脚，MCU做主机和Eeprom(BL24C256)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin18：
        端 口 号：PB0
        网 络 名：ChgAps12VInt
        电平描述：输入H~~~检测到充电机辅助电源上电
                  输入L~~~检测到充电机辅助电源下电
        说    明：充电机辅助电源上下电状态检测
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_0;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin19：
        端 口 号：PB1
        网 络 名：HeatRlyDet
        电平描述：输入H~~~检测到加热继电器接入
                  输入L~~~未检测到加热继电器接入
        说    明：加热继电器是否接入检测
    	          接入表示继电器焊接到执行单元上了，并不表示继电器接通了
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin20：
        端 口 号：PB2
        网 络 名：HeatRlyColNegCtr
        电平描述：输出H~~~控制加热继电器线圈(触点)接通
                  输出L~~~控制加热继电器线圈(触点)断开
        说    明：加热继电器线圈(触点)通断控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_2 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_2 );
    
    /*
    Pin21：
        端 口 号：PB3
        网 络 名：DryCntOutCtr
        电平描述：输出H~~~控制信号干接点接通
                  输出L~~~控制信号干接点断开
        说    明：信号干接点通断控制脚
                  注：干接点额定持续电流50mA，接通等效阻抗30Ω
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输出功能(Output)，禁用开漏和上拉，输出低
    */
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_3 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_3 );
    
    /*
    Pin22：
        端 口 号：PB4
        网 络 名：SPRlyDet
        电平描述：输入H~~~检测到二次保护继电器接入
                  输入L~~~未检测到二次保护继电器接入
        说    明：二次保护继电器是否接入检测
    	          接入表示继电器焊接到执行单元上了，并不表示继电器接通了
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_4;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin23：
        端 口 号：PB5
        网 络 名：SPRlyColNegCtr
        电平描述：输出H~~~控制二次保护继电器线圈(触点)接通
                  输出L~~~控制二次保护继电器线圈(触点)断开
        说    明：二次保护继电器线圈(触点)控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_5 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_5 );
    
    /*
    Pin24：
        端 口 号：PB6
        网 络 名：ChgVolWkupInt
        电平描述：输入H~~~检测到充电机盲充电压接入
                  输入L~~~检测到充电机盲充电压移除
        说    明：充电机盲充电压检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin25：
        端 口 号：PB7
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin26：
        端 口 号：PB8
        网 络 名：SPI0_NSS
        电平描述：输出H~~~禁用从机SPI控制器
                  输出L~~~使能从机SPI控制器
        说    明：SPI通信片选脚，低电平有效
                  MCU作为主机和备份AFE(JW3370)通信
                  在备份AFE(JW3370)处于储运模式时，低电可唤醒AFE退出ShipMode
                  注意：通信结束后，SPI0_NSS脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin27：
        端 口 号：PB9
        网 络 名：SPI0_SCLK
        电平描述：数字功能
        说    明：SPI通信时钟脚
                  MCU作为主机和备份AFE(JW3370)通信
                  注意：时钟相位(CPHA)和极性(CPOL)都必须设置为输出高电平的模式！
                  注意：通信结束后，SPI0_SCLK脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin28：
        端 口 号：PB10
        网 络 名：SPI0_MISO
        电平描述：数字功能
        说    明：SPI通信主机接收从机发送数据脚
                  MCU作为主机和备份AFE(JW3370)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin29：
        端 口 号：PB11
        网 络 名：SPI0_MOSI
        电平描述：数字功能
        说    明：SPI通信主机发送从机接收数据脚
                  MCU作为主机和备份AFE(JW3370)通信
                  注意：通信结束后，SPI0_MOSI脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin30：
        端 口 号：PB12
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin31：
        端 口 号：PB13
        网 络 名：CanWkupDet
        电平描述：输入H~~~检测到有CAN通信信号
                  输入L~~~检测到无CAN通信信号
        说    明：CAN通信信号检测脚，可用于睡眠唤醒，需要配置外部中断
                  工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断*/
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*Pin32：
        端 口 号：PB14
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    
    /*
    Pin33：
        端 口 号：PD12
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin34：
        端 口 号：PC0
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin35：
        端 口 号：PC1
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin36：
        端 口 号：PC2
        网 络 名：OSC8MHzI
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振！
        说    明：MCU外部8MHz无源晶振输入脚
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：模拟功能(Analog)
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin37：
        端 口 号：PC3
        网 络 名：OSC8MHzO
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振！
        说    明：MCU外部8MHz无源晶振输出脚
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：模拟功能(Analog)
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin38：
        端 口 号：PC4
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin39：
        端 口 号：PC5
        网 络 名：MAT3V3Det
        电平描述：输入H~~~检测到主AFE的NTC电源上电
                  输入L~~~检测到主AFE的NTC电源下电
        说    明：主AFE的NTC电源上下电状态检测
                  AFE Normal模式：AV3V3 = 3.3V, AT3V3 = 3.3V;
                  AFE Sleep 模式：AV3V3 = 3.3V, AT3V3 = 0.0V;
                  AFE Ship  模式：AV3V3 = 0.0V, AT3V3 = 0.0V;
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin40：
        端 口 号：PC6
        网 络 名：MAfeAlmDet
        电平描述：输入H~~~检测到主AFE有重要事件输出
                  输入L~~~检测到主AFE无重要事件输出
        说    明：主AFE重要事件输出状态检测脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin41：
        端 口 号：PC7
        网 络 名：SPI2_NSS
        电平描述：输出H~~~禁用从机SPI控制器
                  输出L~~~使能从机SPI控制器
        说    明：SPI通信片选脚
                  MCU作为主机和主AFE(JW3370)通信
                  在主AFE(JW3370)处于睡眠模式时，低电平可唤醒主AFE退出ShipMode
                  注意：通信结束后，SPI2_NSS脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin42：
        端 口 号：PC8
        网 络 名：SPI2_SCLK
        电平描述：数字功能
        说    明：SPI通信时钟脚
                  MCU作为主机和主AFE(JW3370)通信
                  注意：时钟相位(CPHA)和极性(CPOL)都必须设置为输出高电平的模式！
                  注意：通信结束后，SPI2_SCLK脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin43：
        端 口 号：PC9
        网 络 名：SPI2_MISO
        电平描述：数字功能
        说    明：SPI通信主机接收从机发送数据脚
                  MCU作为主机和主AFE(JW3370)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin44：
        端 口 号：PC10
        网 络 名：SPI2_MOSI
        电平描述：数字功能
        说    明：SPI通信主机发送从机接收数据脚
                  MCU作为主机和主AFE(JW3370)通信
                  注意：通信结束后，SPI2_MOSI脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin45：
        端 口 号：PC11
        网 络 名：BAT3V3Det
        电平描述：输入H~~~检测到备份AFE的NTC电源上电
                  输入L~~~检测到备份AFE的NTC电源下电
        说    明：备份AFE的NTC电源上下电状态检测
                  AFE Normal模式：AV3V3 = 3.3V, AT3V3 = 3.3V;
                  AFE Sleep 模式：AV3V3 = 3.3V, AT3V3 = 0.0V;
                  AFE Ship  模式：AV3V3 = 0.0V, AT3V3 = 0.0V;
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin46：
        端 口 号：PC12
        网 络 名：BAfeAlmDet
        电平描述：输入H~~~检测到备份AFE有重要事件输出
                  输入L~~~检测到备份AFE无重要事件输出
        说    明：备份AFE重要事件输出状态检测脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin47：
        端 口 号：PH15
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin48：
        端 口 号：XT32KO
        网 络 名：OSC32KHzO
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振
        说    明：MCU外部32.768KHz无源晶振固定输出脚
    --------------------------------------------------------------------------------
    <5.4> Pin49～Pin64
    Pin49：
        端 口 号：XT32KI
        网 络 名：OSC32KHzI
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振
        说    明：MCU外部32.768KHz无源晶振固定输入脚
    Pin50：
        端 口 号：VDD15
        网 络 名：VDD15
        电平描述：固定脚，不可编程
        说    明：MCU内核1.5V电压外接电容脚
    Pin51：
        端 口 号：VSS(VSSA)
        网 络 名：GND
        电平描述：固定脚，不可编程
        说    明：MCU模拟地和数字地
    Pin52：
        端 口 号：VDD(VDDA)
        网 络 名：M3V3
        电平描述：固定脚，不可编程
        说    明：MCU模拟电源和数字电源
    Pin53：
        端 口 号：VREFP
        网 络 名：M3V3
        电平描述：固定脚，不可编程
                  可通过跳线电阻选择使用外部专用高精度3.3V参考电压Ref3V3
                  由M3V3供电时，只能使用MCU片内1.2V参考电压。
        说    明：MCU外部输入参考电压正端
    Pin54：
        端 口 号：VBAT
        网 络 名：M3V3
        电平描述：固定脚，不可编程
                  该脚可以测量M3V3电源电压
        说    明：MCU内部RTC时钟独立供电正端
    Pin55：
        端 口 号：PD11
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin56：
        端 口 号：PD0
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin57：
        端 口 号：PD1
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin58：
        端 口 号：PD2
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin59：
        端 口 号：PD3
        网 络 名：/V3V3PwrCtr
        电平描述：输出H~~~断开可控电源(V3V3和Iso5V隔离电源)
                  输出L~~~导通可控电源(V3V3和Iso5V隔离电源)
        说    明：主电源(V3V3和Iso5V)开关控制脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_SetOutputPin(GPIOD ,FL_GPIO_PIN_3 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    FL_GPIO_SetOutputPin(GPIOD ,FL_GPIO_PIN_3 );
    
    /*
    Pin60：
        端 口 号：PD4
        网 络 名：V3V3/2Ads
        电平描述：模拟功能，ADC_IN9
        说    明：V3V3电源半压AD采样
                  Vv3v3 = Vadc x 2 = ?V,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_4;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /*
    Pin61：
        端 口 号：PD5
        网 络 名：AmbTempAds
        电平描述：模拟功能，ADC_IN3
        说    明：环境温度NTC两端电压AD采样
                  连接方式：V3V3电源连接10K/0.5%固定电阻，再连接NTC10K/B3435到地，
                            采样点为固定电阻和NTC电阻的中间节点！
                  Vntc = Vadc x 1,
                  Rntc = Vntc x 10K / (V3V3 - Vntc) = ?K,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /*
    Pin62：
        端 口 号：PD6
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin63：
        端 口 号：PD7
        网 络 名：SWCLK
        电平描述：SWD调试
        说    明：SWD调试时钟脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：模拟功能(Analog)
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#endif
    
    /*
    Pin64：
        端 口 号：PD8
        网 络 名：SWDIO
        电平描述：SWD调试
        说    明：SWD调试数据脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：模拟功能(Analog)
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#endif
    
}
/*=============================================================
* 函数名称：gpio_sleep_init
* 函数功能：GPIO输入输出初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void gpio_sleep_init(void)
{
      FL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    /*
    Pin2：
        端 口 号：PA13
        网 络 名：FewdCtr
        电平描述：高低切换输出，看门狗芯片只检测高低电平沿的切换时间间隔
                  手册参考喂狗时间：1.12S(Min)，1.6S(Type)，2.4S(Max)
                  调试时禁用MCU片外看门狗，只需将该脚配置为！
        说    明：MCU片外看门狗喂狗脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉，高低切换输出高低切换时间以500mS±100mS为宜，不得超过1S！
                  睡眠模式：输出功能(Output)，禁用开漏和上拉，高低切换输出高低切换时间以500mS±100mS为宜，不得超过1S！
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_13 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA ,FL_GPIO_PIN_13 );
#endif
    
    /*
    Pin3：
        端 口 号：PA14
        网 络 名：PwrTempAds
        电平描述：模拟功能，ADC_IN11
        说    明：功率温度NTC两端电压AD采样
                  连接方式：V3V3电源连接10K/0.5%固定电阻，再连接NTC10K/B3435到地，
                            采样点为固定电阻和NTC电阻的中间节点！
                  Vntc = Vadc x 1,
                  Rntc = Vntc x 10K / (V3V3 - Vntc) = ?K,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_14);
    
    /*
    Pin4：
        端 口 号：PA15
        网 络 名：LedBlueCtr
        电平描述：输入H~~~控制运行灯点亮
                  输入L~~~控制运行灯熄灭
        说    明：蓝色运行灯亮灭控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_15);
    
    /*
    Pin5：
        端 口 号：PA0
        网 络 名：LedRedCtr
        电平描述：输入H~~~控制告警灯点亮
                  输入L~~~控制告警灯熄灭
        说    明：红色告警灯亮灭控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_0);
    
    /*
    Pin6：
        端 口 号：PA1
        网 络 名：Iso5VDet
        电平描述：输入H~~~检测到隔离电源上电成功
                  输入L~~~检测到隔离电源下电成功
        说    明：隔离电源上下电检测脚
                  工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断*/
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_1);
    /*Pin7：
        端 口 号：PA2
        网 络 名：Uart0_RX
        电平描述：数字功能，Uart0_RX
        说    明：Uart通信数据接收脚，用于UART或RS485通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_2);
    
    /*
    Pin8：
        端 口 号：PA3
        网 络 名：Uart0_TX
        电平描述：数字功能，Uart0_TX
        说    明：Uart通信数据发送脚，用于UART或RS485通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_3);
    
    /*
    Pin9：
        端 口 号：PA4
        网 络 名：RS485/RTCtr
        电平描述：输出H~~~控制RS485收发器为数据发送方向
                  输出L~~~控制RS485收发器为数据接收方向
        说    明：RS485收发器数据收发方向切换脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_4);
    
    /*
    Pin10：
        端 口 号：PA5
        网 络 名：KeyWkupInt
        电平描述：输入H~~~检测到叉车钥匙开关接通
                  输入L~~~检测到叉车钥匙开关断开
        说    明：钥匙开关开关机检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /*
    Pin11：
        端 口 号：PA6
        网 络 名：CAN_RX
        电平描述：数字功能，CAN_RX
        说    明：CAN通信数据接收脚
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_6);
    
    /*
    Pin12：
        端 口 号：PA7
        网 络 名：CAN_TX
        电平描述：数字功能，CAN_TX
        说    明：CAN通信数据发送脚
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_7);
    
    /*
    Pin13：
        端 口 号：PA8
        网 络 名：ChgCntWkupInt
        电平描述：输入H~~~检测到充电机短接触点(干接点)接通
                  输入L~~~检测到充电机短接触点(干接点)断开
        说    明：充电机接入检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_8);
    
    /*
    Pin14：
        端 口 号：PA9
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin15：
        端 口 号：PA10
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin16：
        端 口 号：PA11
        网 络 名：EepI2C_SCL
        电平描述：数字功能，I2C_SCL
        说    明：I2C1通信时钟脚，MCU做主机和Eeprom（BL24C256）通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_11);
    
    /*
    Pin17：
        端 口 号：PA12
        网 络 名：EepI2C_SDA
        电平描述：数字功能，I2C_SDA
        说    明：I2C1通信数据脚，MCU做主机和Eeprom(BL24C256)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOA ,FL_GPIO_PIN_12);
    
    /*
    Pin18：
        端 口 号：PB0
        网 络 名：ChgAps12VInt
        电平描述：输入H~~~检测到充电机辅助电源上电
                  输入L~~~检测到充电机辅助电源下电
        说    明：充电机辅助电源上下电状态检测
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_0);
    
    /*
    Pin19：
        端 口 号：PB1
        网 络 名：HeatRlyDet
        电平描述：输入H~~~检测到加热继电器接入
                  输入L~~~未检测到加热继电器接入
        说    明：加热继电器是否接入检测
    	          接入表示继电器焊接到执行单元上了，并不表示继电器接通了
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_1);
    
    /*
    Pin20：
        端 口 号：PB2
        网 络 名：HeatRlyColNegCtr
        电平描述：输出H~~~控制加热继电器线圈(触点)接通
                  输出L~~~控制加热继电器线圈(触点)断开
        说    明：加热继电器线圈(触点)通断控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_2);
    
    /*
    Pin21：
        端 口 号：PB3
        网 络 名：DryCntOutCtr
        电平描述：输出H~~~控制信号干接点接通
                  输出L~~~控制信号干接点断开
        说    明：信号干接点通断控制脚
                  注：干接点额定持续电流50mA，接通等效阻抗30Ω
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输出功能(Output)，禁用开漏和上拉，输出低
    */
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_3 );
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOB ,FL_GPIO_PIN_3 );
    
    /*
    Pin22：
        端 口 号：PB4
        网 络 名：SPRlyDet
        电平描述：输入H~~~检测到二次保护继电器接入
                  输入L~~~未检测到二次保护继电器接入
        说    明：二次保护继电器是否接入检测
    	          接入表示继电器焊接到执行单元上了，并不表示继电器接通了
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_4);
    
    /*
    Pin23：
        端 口 号：PB5
        网 络 名：SPRlyColNegCtr
        电平描述：输出H~~~控制二次保护继电器线圈(触点)接通
                  输出L~~~控制二次保护继电器线圈(触点)断开
        说    明：二次保护继电器线圈(触点)控制
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_5);
    
    /*
    Pin24：
        端 口 号：PB6
        网 络 名：ChgVolWkupInt
        电平描述：输入H~~~检测到充电机盲充电压接入
                  输入L~~~检测到充电机盲充电压移除
        说    明：充电机盲充电压检测，可用于睡眠模式下唤醒BMS
        配    置：工作模式：输入功能(Input)，使能输入，禁用上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入和上拉，使能外部中断
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*
    Pin25：
        端 口 号：PB7
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin26：
        端 口 号：PB8
        网 络 名：SPI0_NSS
        电平描述：输出H~~~禁用从机SPI控制器
                  输出L~~~使能从机SPI控制器
        说    明：SPI通信片选脚，低电平有效
                  MCU作为主机和备份AFE(JW3370)通信
                  在备份AFE(JW3370)处于储运模式时，低电可唤醒AFE退出ShipMode
                  注意：通信结束后，SPI0_NSS脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_8);
    
    /*
    Pin27：
        端 口 号：PB9
        网 络 名：SPI0_SCLK
        电平描述：数字功能
        说    明：SPI通信时钟脚
                  MCU作为主机和备份AFE(JW3370)通信
                  注意：时钟相位(CPHA)和极性(CPOL)都必须设置为输出高电平的模式！
                  注意：通信结束后，SPI0_SCLK脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_9);
    
    /*
    Pin28：
        端 口 号：PB10
        网 络 名：SPI0_MISO
        电平描述：数字功能
        说    明：SPI通信主机接收从机发送数据脚
                  MCU作为主机和备份AFE(JW3370)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_10);
    
    /*
    Pin29：
        端 口 号：PB11
        网 络 名：SPI0_MOSI
        电平描述：数字功能
        说    明：SPI通信主机发送从机接收数据脚
                  MCU作为主机和备份AFE(JW3370)通信
                  注意：通信结束后，SPI0_MOSI脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOB ,FL_GPIO_PIN_11);
    
    /*
    Pin30：
        端 口 号：PB12
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin31：
        端 口 号：PB13
        网 络 名：CanWkupDet
        电平描述：输入H~~~检测到有CAN通信信号
                  输入L~~~检测到无CAN通信信号
        说    明：CAN通信信号检测脚，可用于睡眠唤醒，需要配置外部中断
                  工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断*/
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_13;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*
    Pin32：
        端 口 号：PB14
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    
    /*
    Pin33：
        端 口 号：PD12
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin34：
        端 口 号：PC0
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin35：
        端 口 号：PC1
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin36：
        端 口 号：PC2
        网 络 名：OSC8MHzI
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振！
        说    明：MCU外部8MHz无源晶振输入脚
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：模拟功能(Analog)
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin37：
        端 口 号：PC3
        网 络 名：OSC8MHzO
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振！
        说    明：MCU外部8MHz无源晶振输出脚
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：模拟功能(Analog)
    */
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    /*
    Pin38：
        端 口 号：PC4
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin39：
        端 口 号：PC5
        网 络 名：MAT3V3Det
        电平描述：输入H~~~检测到主AFE的NTC电源上电
                  输入L~~~检测到主AFE的NTC电源下电
        说    明：主AFE的NTC电源上下电状态检测
                  AFE Normal模式：AV3V3 = 3.3V, AT3V3 = 3.3V;
                  AFE Sleep 模式：AV3V3 = 3.3V, AT3V3 = 0.0V;
                  AFE Ship  模式：AV3V3 = 0.0V, AT3V3 = 0.0V;
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_5);
    
    /*
    Pin40：
        端 口 号：PC6
        网 络 名：MAfeAlmDet
        电平描述：输入H~~~检测到主AFE有重要事件输出
                  输入L~~~检测到主AFE无重要事件输出
        说    明：主AFE重要事件输出状态检测脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_6);
    
    /*
    Pin41：
        端 口 号：PC7
        网 络 名：SPI2_NSS
        电平描述：输出H~~~禁用从机SPI控制器
                  输出L~~~使能从机SPI控制器
        说    明：SPI通信片选脚
                  MCU作为主机和主AFE(JW3370)通信
                  在主AFE(JW3370)处于睡眠模式时，低电平可唤醒主AFE退出ShipMode
                  注意：通信结束后，SPI2_NSS脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_7);
    
    /*
    Pin42：
        端 口 号：PC8
        网 络 名：SPI2_SCLK
        电平描述：数字功能
        说    明：SPI通信时钟脚
                  MCU作为主机和主AFE(JW3370)通信
                  注意：时钟相位(CPHA)和极性(CPOL)都必须设置为输出高电平的模式！
                  注意：通信结束后，SPI2_SCLK脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_8);
    
    /*
    Pin43：
        端 口 号：PC9
        网 络 名：SPI2_MISO
        电平描述：数字功能
        说    明：SPI通信主机接收从机发送数据脚
                  MCU作为主机和主AFE(JW3370)通信
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_9);
    
    /*
    Pin44：
        端 口 号：PC10
        网 络 名：SPI2_MOSI
        电平描述：数字功能
        说    明：SPI通信主机发送从机接收数据脚
                  MCU作为主机和主AFE(JW3370)通信
                  注意：通信结束后，SPI2_MOSI脚要输出高电平或配置为高阻输入模式！
        配    置：工作模式：数字功能(Digital)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_10);
    
    /*
    Pin45：
        端 口 号：PC11
        网 络 名：BAT3V3Det
        电平描述：输入H~~~检测到备份AFE的NTC电源上电
                  输入L~~~检测到备份AFE的NTC电源下电
        说    明：备份AFE的NTC电源上下电状态检测
                  AFE Normal模式：AV3V3 = 3.3V, AT3V3 = 3.3V;
                  AFE Sleep 模式：AV3V3 = 3.3V, AT3V3 = 0.0V;
                  AFE Ship  模式：AV3V3 = 0.0V, AT3V3 = 0.0V;
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_11);
    
    /*
    Pin46：
        端 口 号：PC12
        网 络 名：BAfeAlmDet
        电平描述：输入H~~~检测到备份AFE有重要事件输出
                  输入L~~~检测到备份AFE无重要事件输出
        说    明：备份AFE重要事件输出状态检测脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOC ,FL_GPIO_PIN_12);
    
    /*
    Pin47：
        端 口 号：PH15
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin48：
        端 口 号：XT32KO
        网 络 名：OSC32KHzO
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振
        说    明：MCU外部32.768KHz无源晶振固定输出脚
    --------------------------------------------------------------------------------
    <5.4> Pin49～Pin64
    Pin49：
        端 口 号：XT32KI
        网 络 名：OSC32KHzI
        电平描述：模拟功能
                  注意：该脚只有配置为模拟功能，才能使用外部晶振
        说    明：MCU外部32.768KHz无源晶振固定输入脚
    Pin50：
        端 口 号：VDD15
        网 络 名：VDD15
        电平描述：固定脚，不可编程
        说    明：MCU内核1.5V电压外接电容脚
    Pin51：
        端 口 号：VSS(VSSA)
        网 络 名：GND
        电平描述：固定脚，不可编程
        说    明：MCU模拟地和数字地
    Pin52：
        端 口 号：VDD(VDDA)
        网 络 名：M3V3
        电平描述：固定脚，不可编程
        说    明：MCU模拟电源和数字电源
    Pin53：
        端 口 号：VREFP
        网 络 名：M3V3
        电平描述：固定脚，不可编程
                  可通过跳线电阻选择使用外部专用高精度3.3V参考电压Ref3V3
                  由M3V3供电时，只能使用MCU片内1.2V参考电压。
        说    明：MCU外部输入参考电压正端
    Pin54：
        端 口 号：VBAT
        网 络 名：M3V3
        电平描述：固定脚，不可编程
                  该脚可以测量M3V3电源电压
        说    明：MCU内部RTC时钟独立供电正端
    Pin55：
        端 口 号：PD11
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin56：
        端 口 号：PD0
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin57：
        端 口 号：PD1
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin58：
        端 口 号：PD2
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin59：
        端 口 号：PD3
        网 络 名：/V3V3PwrCtr
        电平描述：输出H~~~断开可控电源(V3V3和Iso5V隔离电源)
                  输出L~~~导通可控电源(V3V3和Iso5V隔离电源)
        说    明：主电源(V3V3和Iso5V)开关控制脚
        配    置：工作模式：输出功能(Output)，禁用开漏和上拉
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOD ,FL_GPIO_PIN_3);
    
    /*
    Pin60：
        端 口 号：PD4
        网 络 名：V3V3/2Ads
        电平描述：模拟功能，ADC_IN9
        说    明：V3V3电源半压AD采样
                  Vv3v3 = Vadc x 2 = ?V,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOD ,FL_GPIO_PIN_4);
    
    /*
    Pin61：
        端 口 号：PD5
        网 络 名：AmbTempAds
        电平描述：模拟功能，ADC_IN3
        说    明：环境温度NTC两端电压AD采样
                  连接方式：V3V3电源连接10K/0.5%固定电阻，再连接NTC10K/B3435到地，
                            采样点为固定电阻和NTC电阻的中间节点！
                  Vntc = Vadc x 1,
                  Rntc = Vntc x 10K / (V3V3 - Vntc) = ?K,
                  Fadc=16MHz，最小采样时间0.36uS，采样时间配置6个ADC时钟周期！
                  注意：使用MCU片内1.200V参考电压，并MCU出厂时的校准数据折算采样值！
        配    置：工作模式：模拟功能(Analog)
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    FL_GPIO_DeInit(GPIOD ,FL_GPIO_PIN_5);
    
    /*
    Pin62：
        端 口 号：PD6
        网 络 名：NC
        电平描述：无
        说    明：悬空无连接脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：输入功能(Input)，禁用输入、上拉和外部中断
    */
    
    /*
    Pin63：
        端 口 号：PD7
        网 络 名：SWCLK
        电平描述：SWD调试
        说    明：SWD调试时钟脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：模拟功能(Analog)
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#endif
    
    /*
    Pin64：
        端 口 号：PD8
        网 络 名：SWDIO
        电平描述：SWD调试
        说    明：SWD调试数据脚
        配    置：工作模式：输入功能(Input)，禁用输入、上拉和外部中断
                  睡眠模式：模拟功能(Analog)
    */
#if defined(NEGT_DEBUG)
   
#else
    FL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch   = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
#endif
   
}
/*=============================================================
* 函数名称：gpio_hard_init
* 函数功能：GPIO输入输出初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void osc_gpio_hard_init(void)
{
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    //PC2.3配置成模拟功能，外接XTHF
    GPIO_InitStruct.pin = FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

///*=============================================================
//* 函数名称：get_the_CANH_L
//* 函数功能：获取CAN的状态
//* 参数个数：0
//* 函数参数：
//* 返回值：  
//*           CAN状态
//* 修改记录:
//*===============================================================
//* 日期                修改人             修改内容
//* 2020-04-30          李勇               创建
//==============================================================*/
//uint8_t get_the_CANH_L(void)
//{
//	if ( gpio_output_bit_get( GPIOA , GPIO_PIN_1 ) != 0 )
//      return 1;
//    else
//      return 0;     
//}
/*=============================================================
* 函数名称：hardware_init
* 函数功能：系统硬件初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void hardware_init(void)
{
    __disable_irq(); 
    /* 延时硬件初始化 */
    delay_init();
    osc_gpio_hard_init();
    /* 系统时钟配置 */
    init_sysclk();
    /* RTC时间硬件初始化 */
    rtc_datetime_init();
    /* GPIO管脚初始化 */
	gpio_hard_init();
	/* 电源硬件初始化 */
	power_hard_init();
	/* 看门狗初始化 */
    iwdg_init();
    /* */
    feed_iwdg();
	/* 定时器硬件初始化, 5毫秒, 原来1毫秒修改为5毫秒 */
	ATIM_Init(49, 4799);
	/* 定时器0硬件初始化 */
	GpTimer0_init(9999, 4799);  //48M/4800/10000 = 1s
	/* EEPROM硬件初始化 */
	I2C_Device_Init();
	/* ADC初始化 */
	adc_init();
	/* can初始化 */
	CAN_Configuration(E_CAN_BAUDRATE_250K);
#if defined( BLUETOOTH ) 
    BluetoothHardwareInit();
#endif
    /* 使能全局中断 */
    __enable_irq(); 
    /* AFE初始化配置 */
	afe_app_init();
}

/*=============================================================
* 函数名称：hardware_wake_init
* 函数功能：系统在睡眠唤醒后硬件初始化
* 参数个数：0
* 函数参数：
* 返回值：  
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2018-06-25          戴辉发             创建
==============================================================*/
void hardware_wake_init(void)
{
     __disable_irq(); 
     /* 延时硬件初始化 */
	delay_init();  
    osc_gpio_hard_init();
	/* 系统时钟配置 */
	init_sysclk(); 
	/* GPIO管脚初始化 */
	gpio_hard_init();  
    /* 电源硬件初始化 */
	power_hard_init();
    /* */
    feed_iwdg();
	/* 定时器硬件初始化, 5毫秒, 原来1毫秒修改为5毫秒 */
	ATIM_Init(49, 4799);
	/* 定时器0硬件初始化 */
	GpTimer0_init(9999, 4799);   /*48M/4800/10000 = 1s*/  
	/* EEPROM硬件初始化 */
	I2C_Device_Init();   
	/* ADC初始化 */
	adc_init();
	/* can初始化 */
	CAN_Configuration(E_CAN_BAUDRATE_250K);  
#if defined( BLUETOOTH ) 
    BluetoothHardwareInit();
#endif
    /* 使能全局中断 */
    __enable_irq(); 
    /* AFE初始化配置 */
	afe_app_init();
}
