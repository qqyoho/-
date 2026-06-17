/*---------------------------------------------------------*
 *  Copyright (c) [2021] [杭州优恩捷科技有限公司]
 *
 * 文件名： blutooth.c
 * 文件功能描述：实现 蓝牙通讯流程
 *
 * 修改记录： 
 * 日    期： 2021.12.28
 * 修 改 人： liyong  
 * 修改内容： 创建
 * 
*----------------------------------------------------------*/
#define EVENT_BLE_CONNECTED             0X02 /*蓝牙 BLE 连接建立（主从）*/
#define EVENT_BLE_DISCONNECTED          0X05 /*蓝牙 BLE 连接已经断开（主从）*/
#define EVENT_CMD_COMPLETE              0X06 /*命令已完成（主从）*/
#define EVENT_BLE_DATA_RECEIVED         0X08 /*接收到蓝牙 BLE(ATT)数据（主从）*/
#define EVENT_SYSTEM_READY              0X09 /*模块已准备好（主从）*/
#define EVENT_STAUS_RESPONSE            0X0A /*状态回复（主从）*/
#define EVENT_UART_EXCEPTION            0X0F /*UART 传输包格式错误（主从）*/
#define EVENT_BLE_CONN_PARAM_UPDATE_COMPLETE   0X10 /*BLE 更新连接参数完成（主从）*/

#define IS_EVENT_OP_MATCH(INTENCE)      (((INTENCE) == EVENT_CMD_COMPLETE)||\
                                        ((INTENCE) == EVENT_BLE_DATA_RECEIVED))/*(((INTENCE) == EVENT_BLE_CONNECTED)||\
                                        ((INTENCE) == EVENT_BLE_DISCONNECTED)||\
                                        ((INTENCE) == EVENT_CMD_COMPLETE)||\
                                        ((INTENCE) == EVENT_BLE_DATA_RECEIVED)||\
                                        ((INTENCE) == EVENT_SYSTEM_READY)||\
                                        ((INTENCE) == EVENT_STAUS_RESPONSE)||\
                                        ((INTENCE) == EVENT_UART_EXCEPTION)||\
                                        ((INTENCE) == EVENT_BLE_CONN_PARAM_UPDATE_COMPLETE))*/
#include <string.h>
#include "bluetooth.h"
#include "parameter.h"
#include "vol_manage.h"
#include "system_adjust.h"
#include "fm33lg0xx_fl.h"
#include "uart_dma.h"
#include "uart.h"
#include "soc.h"
#include "can.h"
#include "storage_manage.h"
#include "temp_manage.h"
#include "system_control.h"
#include "switch_status.h"
#include "storage_manage.h"
#include "soc.h"
#include "serial_no.h"
#include "rtc.h"
#include "current_manage.h"
#include "afe_app.h"
#include "short.h"
                                          
#define MAX_RECV_DATA_NUM       16
#define MAX_RECV_UART_NUM       64

static uint8_t processDataHead;
static uint8_t processDataTail;
static uint8_t blueRecvData[MAX_RECV_UART_NUM];
uint8_t g_send_over_flag;
static uint8_t g_uart_send_buf[UART_TX_DMA_NUM];
static uint8_t g_uart_send_len;

static uint8_t buttonkey_wake_flag;

static uint8_t bluetoothProcessStep;
static uint8_t processDataStep;
static uint8_t blueSendDataStep;
static uint8_t usartCycleDelay;

typedef struct  PROCESS_BLUETOOTH_DATA
{
    uint8_t op;
    uint8_t rx;
    uint8_t length;
    uint8_t data[MAX_RECV_UART_NUM];
}ProcessBluetoothData;

ProcessBluetoothData processBluetoothData;

extern int GetSoftVersionForApp(void);
extern int GetHardVersionForApp(void);
/*==============================================================
* 函数名称：BluetoothPowerOn
* 函数功能：蓝牙模块电源上电
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void BluetoothPowerOn(void)
{
    FL_GPIO_SetOutputPin( GPIOD, FL_GPIO_PIN_0);	
}

/*==============================================================
* 函数名称：BluetoothPowerOff
* 函数功能：蓝牙模块电源下电
* 参数个数：0
* 函数参数：
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void BluetoothPowerOff(void)
{
    FL_GPIO_ResetOutputPin( GPIOD, FL_GPIO_PIN_0);	
}
/*==============================================================
* 函数名称：BluetoothSetName
* 函数功能：设置蓝牙名称
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
#if 0
static const char bluetoothName[10] = "HZNEGT-->";
static void BluetoothSetName(void)
{
    int i;
    UART_RD_SND;
    
    g_uart_send_buf[0] = 0x01;
    g_uart_send_buf[1] = 0x04;
    g_uart_send_buf[2] = 10;
    for(i=0;i<10;i++)
    {
        g_uart_send_buf[3+i] = bluetoothName[i];
    }  
    uart_send_data(g_uart_send_buf, 13);
}
#endif
/*==============================================================
* 函数名称：BluetoothHardwareInit
* 函数功能：蓝牙模块硬件初始化
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void BluetoothHardwareInit(void)
{ 
	processDataHead = 0;
	processDataTail = 0;
    processDataStep = 0;    
    g_send_over_flag = 0;
    buttonkey_wake_flag = 0;
    dma_uart_init(g_uart_send_buf);
    uart_init(UART_BPS);   
    bluetoothProcessStep = 0;
    blueSendDataStep = 0;
}
/*==============================================================
* 函数名称：blueRecvData_from_dma
* 函数功能：蓝牙模块串口接收
* 参数个数：1
* 函数参数：
*          [IN]         data               接收到的串口数据
* 返 回 值：
*          无
* 修改记录:
*===============================================================
* 日期                  修改人             修改内容
* 2018-09-26            liyong             创建
==============================================================*/
void blueRecvData_from_dma(uint8_t data)
{
    blueRecvData[processDataTail ++] = data;
    if ( processDataTail >= MAX_RECV_UART_NUM )
    {
        processDataTail = 0;
    }
}


/*==============================================================
* 函数名称：check
* 函数功能：crc校验
* 参数个数：2
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static uint8_t check(uint8_t * buf, uint8_t len)
{
    uint8_t i, chk= 0;
    uint8_t sum = 0;
    for(i = 0; i < len; i++)
    { 
        chk ^= buf[i];
        sum += buf[i];
    } 
    return ((chk^sum)&0xFF);
} 
/*==============================================================
* 函数名称：BluetoothRecivedDataProcess
* 函数功能：接收数据协议解析
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28                 liyong            创建
==============================================================*/
static uint8_t BluetoothRecivedDataProcess(void)
{
	uint8_t indata;
    uint8_t falg = 0;
    UsartRecivedIdleGetData();
	while ( processDataTail != processDataHead )
	{
		indata = blueRecvData[processDataHead ++];
		if( processDataHead >=  MAX_RECV_UART_NUM ) processDataHead = 0;

        switch( processDataStep )
        {     
        case 0x00:/*数据链路层 cmd*/
            if( indata == 0x02 )
            {
                processDataStep = 0x01;
            }
            break;
        case 0x01:/*数据链路层 op*/
            if( IS_EVENT_OP_MATCH(indata) )
            {
                processBluetoothData.op = indata;
                processDataStep = 0x02;
            }
            else
            {
                processDataStep = 0x0;
            }
            break;
        case 0x02:/*数据链路层 Length*/
            if( indata < MAX_RECV_UART_NUM )
            {
                processBluetoothData.rx = 0;
                processBluetoothData.length = indata;
                processDataStep = 0x03;
            }
            else
            {
                processDataStep = 0x0;
            }
            break;
        case 0x03:/*数据链路层 Payload*/          
            processBluetoothData.data[processBluetoothData.rx++] = indata;
            if( processBluetoothData.rx == processBluetoothData.length )
            {
                processDataStep = 0;
                falg = 1;
            }
            break; 
        default:
            processDataStep = 0x0; 
            break;
            
        }
        
        if( 1 == falg ) break;
	}
    return falg;
}

/*==============================================================
* 函数名称：App1ProtocolFillData
* 函数功能：数据1协议填充数据
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static void BluetoothsendData(uint8_t *buf,uint8_t len)
{
    int i;
    UART_RD_SND;
    /*App数据*/
    g_uart_send_buf[len] = check(buf,len);
    g_uart_send_buf[len+1] = 0x0D;
    /*添加链路层头*/
    for(i=len+1;i>=0;i--)
    {
        g_uart_send_buf[i+5] = g_uart_send_buf[i];
    }
    g_uart_send_buf[0] = 0x01;
    g_uart_send_buf[1] = 0x09;
    g_uart_send_buf[2] = len+4;
    g_uart_send_buf[3] = 0x08;
    g_uart_send_buf[4] = 0x00;    
    uart_send_data(g_uart_send_buf, len+7);
}
/*==============================================================
* 函数名称：App1ProtocolFillData
* 函数功能：数据1协议填充数据
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static void App1ProtocolFillData(uint8_t data)
{
    uint16_t temp;
    g_uart_send_len = 0;
    /*主要信息读取*/
    switch( data )
    {
    /*01:读取电压极值*/
    case 1:   
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x09;
        /*BYTE1  单体极值标识 01
        BYTE2  最高单体电压高字节 单位:mV
        BYTE3  最高单体电压低字节 单位:mV
        BYTE4  最低单体电压高字节 单位:mV
        BYTE5  最低单体电压低字节 单位:mV
        BYTE6  平均单体电压高字节 单位:mV
        BYTE7  平均单体电压低字节 单位:mV
        BYTE8  压差高字节 单位:mV
        BYTE9  压差低字节 单位:mV*/
        g_uart_send_buf[3] = 0x01;
        temp = get_max_cell_vol();
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = get_min_cell_vol();
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        temp = get_average_vol();
        g_uart_send_buf[8] = temp>>8;
        g_uart_send_buf[9] = temp;
        temp = get_max_cell_vol()-get_min_cell_vol();
        g_uart_send_buf[10] = temp>>8;
        g_uart_send_buf[11] = temp;
        g_uart_send_len = 12;
        break;
    /*02:读取总电压*/
    case 2:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x05;
        /*  BYTE1  单体电压标识 02
            BYTE2  总体电压高字节 单位 10mV
            BYTE3  总体电压低字节 单位 10mV

            BYTE4  预充电压高字节 单位 10mV
            BYTE5  预充电压低字节 单位 10mV*/
        g_uart_send_buf[3] = 0x02;
        temp = GetTotalVoltage();
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = GetTotalVoltage();
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        g_uart_send_len = 8;
        break;
    /*03:读取总电流*/
    case 3:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x03;
        /*  BYTE1  总电流标识 03
            BYTE2  总体电流高字节 单位:10mA 偏移量:30000
            BYTE3  总体电压低字节 单位 10mV*/
        g_uart_send_buf[3] = 0x03;
        temp = (3000+g_run_sys_data.current)*10;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        g_uart_send_len = 6;
        break;
    /*04:读取温度极值*/
    case 4:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x04;
        /*  BYTE1  温度极值标识 04
            BYTE2  最高温度 单位:℃ 偏移量:50
            BYTE3  最低温度 单位:℃ 偏移量:50
            BYTE4  温差 单位:℃ 偏移量:0*/
        g_uart_send_buf[3] = 0x04;
        g_uart_send_buf[4] = get_min_cell_temp()/10+50;
        g_uart_send_buf[5] = get_max_cell_temp()/10+50;
        g_uart_send_buf[6] = (get_max_cell_temp()-get_min_cell_temp())/10;
        g_uart_send_len = 7;
        break;
    /*05:读取额定容量*/
    case 5:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x03;
        /*  BYTE1  额定容量标识 05
            BYTE2  额定容量高字节 单位:10mAh
            BYTE3  额定容量低字节 单位:10mAh*/
        g_uart_send_buf[3] = 0x05;
        temp = get_rated_capcity();
        temp *= 100;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
  
        g_uart_send_len = 6;
        break;
    /*06:读取充电与循环次数*/
    case 6:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x05;
        /*  BYTE1  充电与循环次数标识 06
            BYTE2  充电次数高字节 单位:次
            BYTE3  充电次数低字节 单位:次
            BYTE4  循环次数高字节 单位:次
            BYTE5  循环次数低字节 单位:次*/
        g_uart_send_buf[3] = 0x06;
        temp = g_dch_circle.charge_num.conut;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = g_dch_circle.circle_num.conut;
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        g_uart_send_len = 8;
        break;
    /*07:读取 SOC*/
    case 7:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x03;
        /*  BYTE1  SOC 标识 07
            BYTE2  SOC 高字节 单位:0.01%
            BYTE3  SOC 高字节 单位:0.01%*/
        g_uart_send_buf[3] = 0x07;
        temp =(uint16_t)( 10000L*g_run_sys_data.soc/MAX_SOC_VALUE);
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        g_uart_send_len = 6;
        break;
    /*08:读取 SOH*/
    case 8:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x03;
        /*  BYTE1  SOH 标识 08
            BYTE2  SOH 高字节 单位:0.01%
            BYTE3  SOH 高字节 单位:0.01%*/
        g_uart_send_buf[3] = 0x08;
        temp = 10000;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        g_uart_send_len = 6;
        break;
    /*09:读取 MOS 状态*/
    case 9:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x03;
        /*  BYTE1  MOS 状态标识 09
            BYTE2  充电 MOS 状态 0x01:打开 0x02:关闭 其他:错误
            BYTE3  放电 MOS 状态 0x01:打开 0x02:关闭 其他:错误*/   
        g_uart_send_buf[3] = 0x09;
        if((0 == get_ch_switch_status()))
            g_uart_send_buf[4] = 0x02;
        else
            g_uart_send_buf[4] = 0x01;
        if((0 == get_dch_switch_status()))  
            g_uart_send_buf[5] = 0x02;
        else
            g_uart_send_buf[5] = 0x01;
        
        g_uart_send_len = 6;
        break;
    /*0A:读取绝缘电阻值*/
    case 0x0a:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x05;
        /*  BYTE1  绝缘电阻值标识 0A
            BYTE2  绝缘电阻+高字节 单位:kΩ 若电阻值>=10000Ω则显示电阻为∞
            BYTE3  绝缘电阻+低字节 单位:kΩ 若电阻值>=10000Ω则显示电阻为∞
            BYTE4  绝缘电阻-高字节 单位:kΩ 若电阻值>=10000Ω则显示电阻为∞
            BYTE5  绝缘电阻-低字节 单位:kΩ 若电阻值>=10000Ω则显示电阻为∞*/   
        g_uart_send_buf[3] = 0x0a;
         temp = 9998;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
         temp = 9999;
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        
        g_uart_send_len = 8;
        break;
    /*0B:读取电池类型串数通信方式*/
    case 0x0b:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x05;
        /*  BYTE1  类型 串数 通信方式标识 0B
            BYTE2  电池类型 0x01 磷酸铁锂 0x02 锰酸锂 0x03 三元
            BYTE3  电池串数 单位:串
            BYTE4
            通信方式高字节 每个 bit 代表一种通信方式 1 有效 0 无效
            Bit8-bit15 预留
            BYTE5
            通信方式低字节 每个 bit 代表一种通信方式 1 有效 0 无效
            bit0 RS485
            bit1 CAN
            bit2 一线通
            bit3 4G+GPS
            bit4 蓝牙
            bit5-bit7 预留*/   
        g_uart_send_buf[3] = 0x0b;
        g_uart_send_buf[4] = 0x01;
        g_uart_send_buf[5] = BAT_NUM;
         temp = 0x0|0x02|0x10;
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        
        g_uart_send_len = 8;
        break;
    /*0C:读取软硬件版本号机种名*/
    case 0x0c:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x15;
        /*  BYTE1  产品型号标识 0C
            BYTE2  硬件版本号高位
            BYTE3  硬件版本号低位
            BYTE4  软件版本号高位
            BYTE5  软件版本号低位
            BYTE6-
            BYTE21
            读取后 Ascii 码显示*/   
        g_uart_send_buf[3] = 0x0c;
        temp = GetHardVersionForApp();
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = GetSoftVersionForApp();
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        memcpy(g_uart_send_buf+8,g_serial_no.serial_no,16);
        
        g_uart_send_len = 24;
       break;
    /*0D:读取电池状态*/
    case 0x0d:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x01;
        g_uart_send_buf[2] = 0x02;
        /*  BYTE1  产品型号标识 0D
            BYTE2  充放电状态 0x01 充电 0x02 放电 0x03 静置*/   
        g_uart_send_buf[3] = 0x0D;
        if (E_CHARGE_STATUS == get_system_status())
        {
           g_uart_send_buf[4] = 0x01;
        }
        else if (E_DISCHARGE_STATUS == get_system_status())
        {
           g_uart_send_buf[4] = 0x02; 
        }
        else
        {
           g_uart_send_buf[4] = 0x03;  
        }
        
        g_uart_send_len = 5;
        break;
    }
    
    
    
    /* Send a comm message */
    if( g_uart_send_len > 0)
        BluetoothsendData(g_uart_send_buf, g_uart_send_len);
}
/*==============================================================
* 函数名称：App2ProtocolFillData
* 函数功能：数据2协议填充数据
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static void App2ProtocolFillData(uint8_t data)
{
    uint8_t i;
    uint16_t temp;
     g_uart_send_len = 0;
    /*主要信息读取*/
    switch( data )
    {
    /*01:读取单体电压值*/
    case 1:   
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = (BAT_NUM+1)*2;
        /*  BYTE1  单体电压标识 01
            BYTE2  电池数量 单位:个
            BYTE3  单体电压 1 高字节 单位:mV 偏移量:0
            BYTE4  单体电压 1 低字节 单位:mV 偏移量:0
            BYTE5  单体电压 2…… 此处根据电池数量决定*/
        g_uart_send_buf[3] = 0x01;
        g_uart_send_buf[4] = BAT_NUM;
        for( i=0;i<BAT_NUM;i++)
        {
            temp = g_run_sys_data.cell_vol[i];
            g_uart_send_buf[5+2*i] = temp>>8;
            g_uart_send_buf[6+2*i] = temp;
        }
        
        g_uart_send_len = (BAT_NUM+1)*2+3;
        break;
    /*02:读取温度值*/
    case 2:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = MAX_CELL_TEMP_NUM+2;
        /*  BYTE1  温度标识 02
            BYTE2  温度数量 单位:个
            BYTE3  温度 1 单位:℃ 偏移量:50
            BYTE4  温度 2……根据温度数量决定*/
        g_uart_send_buf[3] = 0x02;
        g_uart_send_buf[4] = MAX_CELL_TEMP_NUM;
        for( i=0;i<MAX_CELL_TEMP_NUM;i++)
        {
            g_uart_send_buf[5+i] = g_run_sys_data.cell_temp[i]/10+50;
        }
        
        g_uart_send_len = MAX_CELL_TEMP_NUM+5;
        break;
    /*03:读取快慢充信号*/
    case 3:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = 0x0a;
        /*  BYTE1  快慢充标识 03
            BYTE2  CC 阻值高字节 单位:Ω
            BYTE3  CC 阻值低字节 单位:Ω
            BYTE4  CC2 阻值高字节 单位:Ω
            BYTE5  CC2 阻值低字节 单位:Ω
            BYTE6  CP 频率高字节 单位:Hz
            BYTE7  CP 频率低字节 单位:Hz
            BYTE8  CP 电压高字节 单位:mV
            BYTE9  CP 电压低字节 单位:mV
            BYTE10  CP 占空比 单位:%*/
        g_uart_send_buf[3] = 0x03;
        temp = 100;
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = 100;
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
        temp = 100;
        g_uart_send_buf[8] = temp>>8;
        g_uart_send_buf[9] = temp;
        temp = 100;
        g_uart_send_buf[10] = temp>>8;
        g_uart_send_buf[11] = temp;
        temp = 100;
        g_uart_send_buf[12] = temp;
        
        g_uart_send_len = 13;
        break;
    /*04:读取单体 SOC*/
    case 4:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = BAT_NUM+1;
        /*  BYTE1  单体 SOC 标识 04
            BYTE2  SOC1 单位:%
            BYTE3  SOC2…… 此处根据电池数量决定*/
        g_uart_send_buf[3] = 0x04;
        temp =(uint16_t) (100.0*g_run_sys_data.soc/MAX_SOC_VALUE);
        for( i=0;i<BAT_NUM;i++)
        {
            g_uart_send_buf[4+i] = temp;
        }
        
        g_uart_send_len = BAT_NUM+4;
        break;
    /*05:读取单体 SOH*/
    case 5:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = BAT_NUM+1;
        /*  BYTE1  单体 SOH 标识 05
            BYTE2  SOH1 单位:%
            BYTE3  SOC2…… 此处根据电池数量决定*/
        g_uart_send_buf[3] = 0x05;
        temp = 100;
        for( i=0;i<BAT_NUM;i++)
        {
            g_uart_send_buf[4+i] = temp;
        }
        
        g_uart_send_len = BAT_NUM+4;
        break;
    /*06:读取 DI/DO 信息*/
    case 6:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x02;
        g_uart_send_buf[2] = 0x0b;
        /*  BYTE1  DI/DO 标识 06
            BYTE2  DI 数量(单位:个)
            BYTE3  DI 每个 bit 表示一个 DI bit0 表示 DI1 0:无效 1:有效
            BYTE4  DI
            BYTE5  DI
            BYTE6  DI 此 DI 数据四个字节必须写完 若数量不足也相应填入数据
            BYTE7  DO 数量(单位:个)
            BYTE8  DO 每个 bit 表示一个 DO bit0 表示 DO1 0:无效 1:有效
            BYTE9  DO
            BYTE10  DO
            BYTE11  DO 此 DO 数据四个字节必须写完 若数量不足也相应填入数据*/
        g_uart_send_buf[3] = 0x06;
        
        g_uart_send_buf[4] = 0;
        g_uart_send_buf[5] = 0;
        g_uart_send_buf[6] = 0;
        g_uart_send_buf[7] = 0;
        g_uart_send_buf[8] = 0;
        
        g_uart_send_buf[9] = 0;
        g_uart_send_buf[10] = 0;
        g_uart_send_buf[11] = 0;
        g_uart_send_buf[12] = 0;
        g_uart_send_buf[13] = 0;
        g_uart_send_len = 14;
        break;
    }
    
    
    
    /* Send a comm message */
    if( g_uart_send_len > 0)
        BluetoothsendData(g_uart_send_buf, g_uart_send_len);
}

/*==============================================================
* 函数名称：App3ProtocolFillData
* 函数功能：数据3协议填充数据
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static void App3ProtocolFillData(uint8_t data)
{
    uint8_t i;
    uint16_t temp;
     g_uart_send_len = 0;
    /*主要信息读取*/
    switch( data )
    {
    /*01:读取 PCB 条码*/
    case 1:   
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x13;
        /*  BYTE1  PCB 条码标识 01
            BYTE2  PCB 条码第一个字节 ASCII
            BYTE3  PCB 条码第二个字节 ASCII
            BYTE4  PCB 条码第三个字节 ASCII
            BYTE5  PCB 条码第四个字节 ASCII
            BYTE6  PCB 条码第五个字节 ASCII
            BYTE7  PCB 条码第六个字节 ASCII
            BYTE8  PCB 条码第七个字节 ASCII
            BYTE9  PCB 条码第八个字节 ASCII
            BYTE10  PCB 条码第九个字节 ASCII
            BYTE11  PCB 条码第十个字节 ASCII
            BYTE12  PCB 条码第十一个字节 ASCII
            BYTE13  PCB 条码第十二个字节 ASCII
            BYTE14  PCB 条码第十三个字节 ASCII
            BYTE15  PCB 条码第十四个字节 ASCII
            BYTE16  PCB 条码第十五个字节 ASCII
            BYTE17  PCB 条码第十六个字节 ASCII
            BYTE18  PCB 条码第十七个字节 ASCII
            BYTE19  PCB 条码第十八个字节 ASCII*/
        g_uart_send_buf[3] = 0x01;
        for( i=0;i<18;i++)
        {
            g_uart_send_buf[4+i] = '1';
        }
        
        g_uart_send_len = 22;
        break;
    /*02:读取机箱序列号*/
    case 2:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x15;
        /*  BYTE1  机箱序列号标识 02
            BYTE2  机箱序列号第一个字节 ASCII
            BYTE3  机箱序列号第二个字节 ASCII
            BYTE4  机箱序列号第三个字节 ASCII
            BYTE5  机箱序列号第四个字节 ASCII
            BYTE6  机箱序列号第五个字节 ASCII
            BYTE7  机箱序列号第六个字节 ASCII
            BYTE8  机箱序列号第七个字节 ASCII
            BYTE9  机箱序列号第八个字节 ASCII
            BYTE10  机箱序列号第九个字节 ASCII
            BYTE11  机箱序列号第十个字节 ASCII
            BYTE12  机箱序列号第十一个字节 ASCII
            BYTE13  机箱序列号第十二个字节 ASCII
            BYTE14  机箱序列号第十三个字节 ASCII
            BYTE15  机箱序列号第十四个字节 ASCII
            BYTE16  机箱序列号第十五个字节 ASCII
            BYTE17  机箱序列号第十六个字节 ASCII
            BYTE18  机箱序列号第十七个字节 ASCII
            BYTE19  机箱序列号第十八个字节 ASCII
            BYTE20  机箱序列号第十九个字节 ASCII
            BYTE21  机箱序列号第二十个字节 ASCII*/
        g_uart_send_buf[3] = 0x02;
        
        for( i=0;i<20;i++)
        {
            g_uart_send_buf[4+i] = '2';
        }
        
        g_uart_send_len = 24;
        break;
    /*03:读取生产商*/
    case 3:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x0B;
        /*  BYTE1  生产厂商标识 03
            BYTE2  生产厂商标识第一个字节 ASCII
            BYTE3  生产厂商标识第二个字节 ASCII
            BYTE4  生产厂商标识第三个字节 ASCII
            BYTE5  生产厂商标识第四个字节 ASCII
            BYTE6  生产厂商标识第五个字节 ASCII
            BYTE7  生产厂商标识第六个字节 ASCII
            BYTE8  生产厂商标识第七个字节 ASCII
            BYTE9  生产厂商标识第八个字节 ASCII
            BYTE10  生产厂商标识第九个字节 ASCII
            BYTE11  生产厂商标识第十个字节 ASCII*/
        g_uart_send_buf[3] = 0x03;
       
        g_uart_send_buf[4] = 'H';
        g_uart_send_buf[5] = 'Z';
        g_uart_send_buf[6] = 'N';
        g_uart_send_buf[7] = 'E';
        g_uart_send_buf[8] = 'G';
        g_uart_send_buf[9] = 'T';
        g_uart_send_buf[10] = 'Y';
        g_uart_send_buf[11] = 'F';
        g_uart_send_buf[12] = 'Z';
        g_uart_send_buf[13] = 'X';
        
        g_uart_send_len = 14;
        break;
    /*04:读取产品型号*/
    case 4:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0X0B;
        /*  BYTE1  产品型号标识 04
            BYTE2  产品型号第一个字节 ASCII
            BYTE3  产品型号第二个字节 ASCII
            BYTE4  产品型号第三个字节 ASCII
            BYTE5  产品型号第四个字节 ASCII
            BYTE6  产品型号第五个字节 ASCII
            BYTE7  产品型号第六个字节 ASCII
            BYTE8  产品型号第七个字节 ASCII
            BYTE9  产品型号第八个字节 ASCII
            BYTE10  产品型号第九个字节 ASCII
            BYTE11  产品型号第十个字节 ASCII*/
        g_uart_send_buf[3] = 0x04;
        
        g_uart_send_buf[4] = 'N';
        g_uart_send_buf[5] = 'O';
        g_uart_send_buf[6] = 'B';
        g_uart_send_buf[7] = '2';
        g_uart_send_buf[8] = '4';
        g_uart_send_buf[9] = '6';
        g_uart_send_buf[10] = '0';
        g_uart_send_buf[11] = 'H';
        g_uart_send_buf[12] = 'F';
        g_uart_send_buf[13] = '.';
        
        g_uart_send_len = 14;
        
        break;
    /*05:读取工号*/
    case 5:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0X05;
        /*  BYTE1  产品型号标识 05
            BYTE2  工号第一个字节 ASCII
            BYTE3  工号第二个字节 ASCII
            BYTE4  工号第三个字节 ASCII
            BYTE5  工号第四个字节 ASCII*/
        g_uart_send_buf[3] = 0x05;
        temp = 100;
        for( i=0;i<4;i++)
        {
            g_uart_send_buf[4+i] = i;
        }
        
        g_uart_send_len = 8;
        break;
    /*06:读取生产日期*/
    case 6:
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x09;
        /*  BYTE1  产品型号标识 06
            BYTE2  生产日期第一字节 ASCII 年
            BYTE3  生产日期第二字节 ASCII 年
            BYTE4  生产日期第三字节 ASCII 年
            BYTE5  生产日期第四字节 ASCII 年
            BYTE6  生产日期第五字节 ASCII 月
            BYTE7  生产日期第六字节 ASCII 月
            BYTE8  生产日期第七字节 ASCII 日
            BYTE9  生产日期第八字节 ASCII 日*/
        g_uart_send_buf[3] = 0x06;
        for( i=0;i<8;i++)
        {
            g_uart_send_buf[4+i] = g_datetime[i];
        }
        
       
        g_uart_send_len = 12;
        break;
     case 7:
        /*07:读取 GPS_ID*/
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x11;
        /*  BYTE1  产品型号标识 07
            BYTE2  GPS_ID 第一字节 ASCII
            BYTE3  GPS_ID 第二字节 ASCII
            BYTE4  GPS_ID 第三字节 ASCII
            BYTE5  GPS_ID 第四字节 ASCII
            BYTE6  GPS_ID 第五字节 ASCII
            BYTE7  GPS_ID 第六字节 ASCII
            BYTE8  GPS_ID 第七字节 ASCII
            BYTE9  GPS_ID 第八字节 ASCII
            BYTE10  GPS_ID 第九字节 ASCII
            BYTE11  GPS_ID 第十字节 ASCII
            BYTE12  GPS_ID 第十一字节 ASCII
            BYTE13  GPS_ID 第十二字节 ASCII
            BYTE14  GPS_ID 第十三字节 ASCII
            BYTE15  GPS_ID 第十四字节 ASCII
            BYTE16  GPS_ID 第十五字节 ASCII
            BYTE17  GPS_ID 第十六字节 ASCII*/
        g_uart_send_buf[3] = 0x07;
        for( i=0;i<16;i++)
        {
            g_uart_send_buf[4+i] = '3';
        }
        
       
        g_uart_send_len = 20;
        break;
        /*08:读取 RTC 时间*/
     case 8:
            
        g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x07;
        /*  BYTE1  产品型号标识 08
            BYTE2  年 (收到 0x14 表示为 2020 年)
            BYTE3  月
            BYTE4  日
            BYTE5  时
            BYTE6  分
            BYTE7  秒*/
        g_uart_send_buf[3] = 0x08;
        temp = g_run_sys_data.write_time[0];
        temp <<= 8;
        temp += g_run_sys_data.write_time[1];
        
        g_uart_send_buf[4] = temp-2000;
        g_uart_send_buf[5] = g_run_sys_data.write_time[2];
        g_uart_send_buf[6] = g_run_sys_data.write_time[3];
        g_uart_send_buf[7] = g_run_sys_data.write_time[4];
        g_uart_send_buf[8] = g_run_sys_data.write_time[5];
        g_uart_send_buf[9] = g_run_sys_data.write_time[6];
       
        g_uart_send_len = 10;
        break; 
        /*09:读取软硬件版本号*/
    case 9:
         g_uart_send_buf[0] = 0xC0;
        g_uart_send_buf[1] = 0x03;
        g_uart_send_buf[2] = 0x05;
        /*  BYTE1  产品型号标识 09
            BYTE2  硬件版本号高位
            BYTE3  硬件版本号低位
            BYTE4  软件版本号高位
            BYTE5  软件版本号低位*/
        g_uart_send_buf[3] = 0x09;
        
        temp = GetHardVersionForApp();
        g_uart_send_buf[4] = temp>>8;
        g_uart_send_buf[5] = temp;
        temp = GetSoftVersionForApp();
        g_uart_send_buf[6] = temp>>8;
        g_uart_send_buf[7] = temp;
       
        g_uart_send_len = 10;
        break;
    }
    
    
    
    /* Send a comm message */
    if( g_uart_send_len > 0)
        BluetoothsendData(g_uart_send_buf, g_uart_send_len);
}

/*==============================================================
* 函数名称：AppProtocolAnalysis
* 函数功能：数据协议解析
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28               liyong            创建
==============================================================*/
static uint8_t AppProtocolAnalysis(void)
{
     uint8_t flag = 0;
     uint8_t crc;
     /*验证数据长度*/
    if(( processBluetoothData.data[0] == 0xA0 )&&
       ( processBluetoothData.length == 5+processBluetoothData.data[2] )&&
       ( processBluetoothData.data[processBluetoothData.length-1] == 0x0D ))
    {
        crc = check( processBluetoothData.data,3+processBluetoothData.data[2] );
        if(  processBluetoothData.data[processBluetoothData.length-2] == crc )
        {
            if( processBluetoothData.data[1] == 0x01 )
            {
                App1ProtocolFillData( processBluetoothData.data[3] );
            }
            else if( processBluetoothData.data[1] == 0x02 )
            {
                App2ProtocolFillData( processBluetoothData.data[3] );
            }
            else if( processBluetoothData.data[1] == 0x03 )
            {
                App3ProtocolFillData( processBluetoothData.data[3] );
            }
            
            if( g_uart_send_len > 0)
                flag = 1;
        }
        
    }
    
    return flag;
}
/*==============================================================
* 函数名称：BluetoothSendDataOk
* 函数功能：蓝牙发送数据成功标志
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28      liyong            创建
==============================================================*/
static uint8_t BluetoothSendDataOk(void)
{
    uint8_t flag = 0;
    //02 06 02 09 00 
    if(( processBluetoothData.length == 2 )&&
       ( processBluetoothData.data[0] == 0x09 )&&
       ( processBluetoothData.data[1] == 0x00 ) )
    {
        flag = 1;
    }
    return flag;
}
/*==============================================================
* 函数名称：BlueRecivedataProtocolAnalysis
* 函数功能：蓝牙接受到的数据协议解析
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28      liyong            创建
==============================================================*/
static uint8_t BlueRecivedataProtocolAnalysis(void)
{
    uint8_t falg = 0;
    if( processBluetoothData.op == EVENT_BLE_DATA_RECEIVED )
    {/*处理App接受到的数据*/
        /*去除 Attribute handle*/
        if( processBluetoothData.length > 2 )
        {
            processBluetoothData.length -= 2;
            memcpy(processBluetoothData.data,processBluetoothData.data+2,processBluetoothData.length);
           
            if ( AppProtocolAnalysis() == 1 )
            {
                falg = 1;
            }
        }
        
    }
   
    return falg;
}
/*==============================================================
* 函数名称：BluetoothSendDataOver
* 函数功能：数据协议解析 + 数据发送
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28      liyong            创建
==============================================================*/
static uint8_t BluetoothSendDataOver(void)
{
    uint8_t falg = 0;
    if( processBluetoothData.op == EVENT_CMD_COMPLETE )
    {/*发送数据完成*/
        if ( BluetoothSendDataOk() == 1 )
        {
            falg = 1;
        }
    }
    return falg;
}
/*==============================================================
* 函数名称：BluetoothSendDataProcess
* 函数功能：数据协议解析 + 数据发送
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                     修改人             修改内容
* 2021-12-28      liyong            创建
==============================================================*/
static uint8_t BluetoothSendDataProcess(void)
{
    uint8_t falg = 0;
    switch( blueSendDataStep )
    {
    case 0:/*获取蓝牙数据，解析app协议*/
        falg = BlueRecivedataProtocolAnalysis();
       
        if( 1 == falg )
        {
            falg = 0;
            blueSendDataStep = 1;
        }
        else
        {/*数据无用*/
            falg = 2;
        }
    break;
    case 1:
    /*发送数据完成查询*/
        if( g_send_over_flag == 0 )
        {
            falg = 1;
            blueSendDataStep = 0;
            UART_RD_REV;
        }
        else if( g_send_over_flag == 1 )
        {/*再转一圈，延时*/
            g_send_over_flag = 0;
        }
    break;
    
    default:
        blueSendDataStep = 0;
        break;
    
    }
    return falg;
}
/*============================================================
* 函数名称：UsartCycleSendData1
* 函数功能：usart周期发生数据1
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void UsartCycleSendData1(void)
{
    /*电池发手柄共8个字节，每30秒发一次。（密码帧）
(1)	0xAA. (Byte1)
(2)	电量。(Byte2)
(3)	电池电压高字节 (byte3)
(4)	电池电压低字节 (byte4)
(5)	帧ID，（每帧加+1）(byte5)
(6)	校验值高字节 (byte6)
(7)	校验值低字节 (byte7)
(8)	0x55. (Byte8)

校验值算法：Val1 = byte2 ：byte3 组成16位值
		   Val2 = byte4 :  byte5 组成16位值
1.	校验值（byte6:byte7）= (Val1+Val2+0x1325) & 0x1234
上面加法如产生进位舍掉进位
*/
    uint16_t temp1,temp2,temp3;
    static uint8_t sendCount;
    g_uart_send_buf[0] = 0xAA;
    g_uart_send_buf[1] = 100L*g_run_sys_data.soc/MAX_SOC_VALUE;
    temp1 = GetTotalVoltage();
    g_uart_send_buf[2] = temp1>>8;
    g_uart_send_buf[3] = temp1;
    g_uart_send_buf[4] = sendCount++;
    temp1 = g_uart_send_buf[1];
    temp1 <<= 8;
    temp1 += g_uart_send_buf[2];
   
    temp2 = g_uart_send_buf[3];
    temp2 <<= 8;
    temp2 += g_uart_send_buf[4];
    temp3 = (temp1+temp2+0x1325) & 0x1234; 
    g_uart_send_buf[5] = temp3>>8;
    g_uart_send_buf[6] = temp3;
    g_uart_send_buf[7] = 0x55;
   
    g_uart_send_len = 8;
    /* Send a comm message */
    uart_send_data(g_uart_send_buf, g_uart_send_len);
}
/*============================================================
* 函数名称：UsartCycleSendData2
* 函数功能：usart周期发生数据2
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void UsartCycleSendData2(void)
{
/*电池发手柄共8个字节，每3秒发一次。(电量正常状态帧)
(1)	0xBB。 (Byte1)
(2)	电量。(Byte2)
(3)	电池电压高字节 (byte3)
(4)	电池电压低字节 (byte4)
(5)	BMS状态字 (Byte5)
(6)	电池温度（℃）---最高位为符号位，范围-128~+127 (Byte6)
(7)	校验字节 (byte7)
(8)	0x55. (Byte8)

校验值Byte7= Byte1+Byte2+Byte3+Byte4+Byte5+Byte6
上面加法如产生进位舍掉进位
*/
    uint16_t temp1,i;
    g_uart_send_buf[0] = 0xBB;
    g_uart_send_buf[1] = 100L*g_run_sys_data.soc/MAX_SOC_VALUE;
    temp1 = GetTotalVoltage();
    g_uart_send_buf[2] = temp1>>8;
    g_uart_send_buf[3] = temp1;
/*BMS状态定义如下：
bit0：过压保护
bit1：欠压保护
bit2：充电过流保护
bit3：放电过流/短路保护
bit4：充/放电过温保护
bit5：充/放电欠温保护
bit6：FET过温保护
bit7：其它错误*/
    g_uart_send_buf[4] = 0;
    if (E_CHARGE_STATUS == get_system_status())
	{
		if (E_VOL_OVER == get_vol_status())
		{
			g_uart_send_buf[4] |= 0x01;
		}
	}
    if (E_CHARGE_STATUS != get_system_status())
	{
		if ( E_VOL_UNDER == get_vol_status() )
		{
			g_uart_send_buf[4] |= 0x02;
		}
	}
    if ( E_CHARGE_STATUS == get_system_status() )
	{
		if ( E_C_CH_PROTECT == get_current_status() )
		{
			g_uart_send_buf[4] |= 0x04;
		}
	}
    else
	{
		if ( E_C_DCH_PROTECT == get_current_status() || 
			 E_C_SECOND_PROTECT == get_current_status() ||
             E_C_FAULT ==  get_current_status() ||
             (3 == get_curr_status())|| 
             (0 != get_short_status()))
		{
			g_uart_send_buf[4] |= 0x08;
		}
	}
    
    if (E_CHARGE_STATUS != get_system_status())
	{
		if ((E_TEMP_HIGH_PROTECT ==  get_environ_temp_status())||
            (E_TEMP_HIGH_PROTECT ==  get_dch_temp_status()))
		{
			g_uart_send_buf[4] |= 0x10;
		}
	}
	else
	{
		if ((E_TEMP_HIGH_PROTECT ==  get_environ_temp_status())||
            (E_TEMP_HIGH_PROTECT ==  get_ch_temp_status()))
		{
			g_uart_send_buf[4] |= 0x10;
		}
	}
    
    if (E_CHARGE_STATUS != get_system_status())
	{
		if ((E_TEMP_LOW_PROTECT ==  get_power_temp_status()) || 
			(E_TEMP_LOW_PROTECT ==  get_environ_temp_status())||
            (E_TEMP_LOW_PROTECT ==  get_dch_temp_status()))
		{
			g_uart_send_buf[4] |= 0x20;
		}
	}
	else
	{
		if ((E_TEMP_LOW_PROTECT ==  get_power_temp_status()) || 
			(E_TEMP_LOW_PROTECT ==  get_environ_temp_status())||
            (E_TEMP_LOW_PROTECT ==  get_ch_temp_status()))
		{
			g_uart_send_buf[4] |= 0x20;
		}
	}
    
    if(E_TEMP_HIGH_PROTECT ==  get_power_temp_status())
    {
        g_uart_send_buf[4] |= 0x40;
    }
    
    if(E_ABATE_STATUS == get_system_status())
    {
       g_uart_send_buf[4] |= 0x80;
    }
    
    g_uart_send_buf[5] = get_max_cell_temp()/10;
    g_uart_send_buf[6] = 0;
    for(i=0;i<6;i++)
    {
        g_uart_send_buf[6] += g_uart_send_buf[i];
    }
    g_uart_send_buf[7] = 0x55;
   
    g_uart_send_len = 8;
    /* Send a comm message */
    uart_send_data(g_uart_send_buf, g_uart_send_len);
}
/*============================================================
* 函数名称：BluetoothDataProcess
* 函数功能：蓝牙接收数据处理流程
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void BluetoothDataProcess(void)
{
    static uint8_t countData1;
    static uint16_t countData2;
    static uint8_t countData3;
    
    if( g_send_over_flag == 0 )
    {/*发送数组为空*/
        if( usartCycleDelay  == 0 )
        {
            usartCycleDelay = 30;
            UsartCycleSendData2();
            countData1++;
            countData3++;
        }
        else if( countData1 >= 10 )
        {
            countData1 = 0;
            UsartCycleSendData1();
        }
        else 
        {
            if( get_cutoff_flag() == 1 )
            {/*需要切断的时候，300ms发送一次*/
                if( countData3 >= 3 )
                {
                    UsartCycleSendData2();
                    countData3 = 0;
                }
            }
            else
            {
                countData3 = 3;
            }
        }
        
        countData2 = 0;
    }
    else 
    {
        if( g_send_over_flag == 1 )
        {/*再转一圈，延时*/
            g_send_over_flag = 0;
        }
        countData2++;
        if( countData2 > 60000 )
        {
            countData2 = 0;
            BluetoothHardwareInit();
        }
    } 
    
#if 0
    uint8_t flag = 0;
    switch(bluetoothProcessStep)
    {
        
    case 0:
        /*等待接受app数据，处理数据*/
        if( 1 == BluetoothRecivedDataProcess())
        {
            bluetoothProcessStep = 1;
        }
        break;
    case 1:
        /*发送数据*/
        flag = BluetoothSendDataProcess(); 
        if( 0 != flag )
        {
            bluetoothProcessStep = 0;
        }
        break;
    case 2:/*等待接受数据发送完成 */
        if( 1 == BluetoothRecivedDataProcess())
        {
            bluetoothProcessStep = 3;
        }
        break;
    case 3:
         /*等待接受数据发送完成 */
        if( 1 == BluetoothSendDataOver())
        {
            bluetoothProcessStep = 0;
        }
        else
        {
            bluetoothProcessStep = 1;
        }
        break;
    default:
        bluetoothProcessStep = 0;
        break;
    }
#endif
}

/*=============================================================
 *  函数名称：BluetoothDelayProcess
 * 函数功能：蓝牙通讯延时
 * 参数个数：
 * 参数描述：
 * 返 回 值：1
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2022-01-19         李勇    	创建
==============================================================*/
void BluetoothDelayProcess(void)
{
   if( usartCycleDelay ) usartCycleDelay--;
}
/*=============================================================
 *  函数名称：set_buttonkey_flag
 * 函数功能：OLED开关检测唤醒标志设置
 * 参数个数：1
 * 参数描述：
 * 返 回 值：
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2021-02-21         李勇    	创建
==============================================================*/
void set_buttonkey_flag(uint8_t flag)
{
   buttonkey_wake_flag = flag;
}
/*=============================================================
 *  函数名称：get_buttonkey_wake_flag
 * 函数功能：获取OLED开关检测唤醒标志
 * 参数个数：
 * 参数描述：
 * 返 回 值：1
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2021-02-21         李勇    	创建
==============================================================*/
uint8_t get_buttonkey_wake_flag(void)
{
   return buttonkey_wake_flag;
}
/*=============================================================
 *  函数名称：get_buttonkey_gpio_status
 * 函数功能：获取OLED开关检测管脚状态
 * 参数个数：
 * 参数描述：
 * 返 回 值：1
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2021-02-21         李勇    	创建
==============================================================*/
uint8_t get_buttonkey_gpio_status(void)
{
    return FL_GPIO_GetInputPin( GPIOC,FL_GPIO_PIN_5) != 0;
}