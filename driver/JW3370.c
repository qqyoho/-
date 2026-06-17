/*---------------------------------------------------------*
 * Copyright (C) 2018 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：AFE.C
 * 文件功能描述：实现AFE操作和控制
 *
 * 修改记录：
 * 2021-05-03    戴辉发    创建
*----------------------------------------------------------*/

#include "JW3370.h"
#include "spi.h"
#include "afe_app.h"
#include "system_adjust.h"

#define AD_V_Gain          305 /* uV/LSB */


//#define WAKEUP_AFE_GPIO_PIN         FL_GPIO_PIN_11
//#define WAKEUP_AFE_GPIO_TYPE        GPIOD

/* 控制参数1 */
static AFE_ctrl_data_byte1 CTR_byte1;
/* 控制参数2 */
static AFE_ctrl_data_byte2 CTR_byte2;

#define TIMER_SPIN0         (100) /* 定时器节拍 */
#define MAX_WAIT_TIME0      (500 / TIMER_SPIN0)


uint8_t Para_Set_Flag;
/*=============================================================
 * 函数名称：afe_mem_init
 * 函数功能：AFE模块内存初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_mem_init(void)
{  
    CTR_byte1.Charger_det = 1;//1：禁止充电器移除检测，0：使能充电器检测
    CTR_byte1.Short_det = 0;//使能短路检测
    CTR_byte1.Low_currrent = 1;//禁止小电流检测
    CTR_byte1.IC_ADD = 1; 
    CTR_byte1.rsvd1 = 0;
    CTR_byte1.rsvd10 = 0;
    CTR_byte1.Pdsg_out = 0;
    CTR_byte1.Pchg_out = 0;

    CTR_byte2.Rsvd2 = 0;
    CTR_byte2.Charger_cur = 0;
    CTR_byte2.Load_det = 0;  //VM负载检测模块启用控制 0:禁用  1:启用
    CTR_byte2.Rsvd3 = 0;
    CTR_byte2.CHG_CTR = 1;
    CTR_byte2.CHG_OUT = 0;
    CTR_byte2.DSG_CTR = 1;
    CTR_byte2.DSG_OUT = 0;
    
    seeBackupVoltageFlag = 0;
}
/*=============================================================
 * 函数名称：afe_hard_init
 * 函数功能：AFE模块接口初始化
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
void afe_hard_init(void)
{
    /* AFE操作的SPI脚 */
    spi_init(M_AFE_SPI);
    spi_init(R_AFE_SPI);
}

/*=============================================================
 * 函数名称：awake_up_afe
 * 函数功能：执行唤醒AFE操作
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-03        戴辉发      创建
==============================================================*/
static void awake_up_afe(void)
{
    
    FL_SPI_SetSSNPin(M_AFE_SPI, FL_SPI_SSN_HIGH);
    FL_SPI_SetSSNPin(R_AFE_SPI, FL_SPI_SSN_HIGH);
    FL_DelayMs(10);
      /* 低电平10ms */
    FL_SPI_SetSSNPin(M_AFE_SPI, FL_SPI_SSN_LOW);
    FL_SPI_SetSSNPin(R_AFE_SPI, FL_SPI_SSN_LOW);
    /* 高电平40ms */
    FL_DelayMs(40);
    FL_SPI_SetSSNPin(M_AFE_SPI, FL_SPI_SSN_HIGH);
    FL_SPI_SetSSNPin(R_AFE_SPI, FL_SPI_SSN_HIGH);
}

/*=============================================================
 * 函数名称：afe_power_down
 * 函数功能：AFE模块进入休眠
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       下电成功
 *           1       失败成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t afe_power_down(void)
{
    uint8_t m_ret = 0;
    uint8_t r_ret = 0;
    uint8_t temp = 3; /* 最多操作3次 */
    uint8_t cmd;
    uint8_t paraFlag;
    /* 开启充电器检测 */
    temp = 3;
    do
    {
        clear_alarm();
        clear_alarm();
        sleep_enter_ship();
        sleep_enter_ship();
        FL_DelayMs(5);
        if( 3 == ReadParameteSetOverFlag(&paraFlag))
        {
           if(( paraFlag & 0x22 ) == 0x22 )
           {
               break;
           }
        }
        
    }while ( -- temp );
   
    temp = 3;
    /*连续发送三次*/
    cmd = POWER_DOWN;
    do
    {
        spi_write(M_AFE_SPI, &cmd, 1);
        FL_DelayMs(20);
        /* 查询AFE TB引脚状态 */
        if( 0 == FL_GPIO_GetInputPin(AFE_TB_DETECT_GPIO_TYPE, AFE_TB_DETECT_GPIO_PIN) )
        {
            m_ret = 1;
        }
    }while (( -- temp ) && ( 0 == m_ret ));
    
    temp = 3;
    cmd = POWER_DOWN;
    do
    {
        spi_write(R_AFE_SPI, &cmd, 1);
        FL_DelayMs(20);
        /* 查询AFE TB引脚状态 */   
        if( 0 == FL_GPIO_GetInputPin(R_AFE_TB_DETECT_GPIO_TYPE, R_AFE_TB_DETECT_GPIO_PIN) )
        {
            r_ret = 1; 
        }  
    }while ((-- temp) && (0 == r_ret ));
    
    return (r_ret|m_ret);
}

/*=============================================================
 * 函数名称：afe_power_up
 * 函数功能：AFE模块退出休眠
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t afe_power_up(void)
{
    uint8_t ret=0;
    uint8_t temp = 4;
    do
    {
        awake_up_afe();
        /* 检测AFE上电状态，使用AFE的TB引脚 */
        if(( FL_GPIO_GetInputPin(AFE_TB_DETECT_GPIO_TYPE, AFE_TB_DETECT_GPIO_PIN) != 0 )&&\
           ( FL_GPIO_GetInputPin(R_AFE_TB_DETECT_GPIO_TYPE, R_AFE_TB_DETECT_GPIO_PIN) != 0 ))
            ret = 1;
        
    }while ((-- temp) && (0 == ret));
    set_charger_config(0);

    temp = 4;
    return ret;
}

/*=============================================================
 * 函数名称：Crc_R
 * 函数功能：读数据CRC校验
 * 参数个数：2
 * 参数描述：
 *           [IN]    pdata        传入数据缓冲
 *           [IN]    Len          传入数据长度
 * 返 回 值：
 *           CRC的结果
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
static uint8_t Crc_R(uint8_t *pdata, uint8_t Len)
{  
    uint8_t temp = 0xff;
    uint8_t legth = Len;

    while (legth --)
    {     
        temp -= *pdata;
        pdata ++;
    }
    return temp;  
}

/*=============================================================
 * 函数名称：Crc_W
 * 函数功能：写数据CRC校验
 * 参数个数：2
 * 参数描述：
 *           [IN]    pdata        传入数据缓冲
 *           [IN]    Len          传入数据长度
 * 返 回 值：
 *           CRC的结果
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
static uint8_t Crc_W(uint8_t *pdata, uint8_t Len)
{  
    uint8_t temp = 0xff;
    uint8_t legth = Len - 1;

    while (legth --)
    {     
       temp -= *(pdata+1);
       pdata ++;
    }
    return temp;  
}

/*=============================================================
 * 函数名称：cmd_from_data
 * 函数功能：形成命令字
 * 参数个数：2
 * 参数描述：
 *           [IN]    addr        地址
 *           [IN]    cmd         命令
 * 返 回 值：
 *           返回命令字
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
static uint8_t cmd_from_data(uint8_t addr, uint8_t cmd)
{
    uint8_t i;
    uint8_t ret, odd;

    odd = 0x01;
    ret = (addr << 4) | (cmd << 1);
    for (i = 0; i < 7; i ++)
    {
        if (ret & (1 << (i + 1)))
        {
            odd  ^= 0x01;
        }
    }
    ret |= odd;
    return ret;
}

/*=============================================================
 * 函数名称：read_voltage_temp
 * 函数功能：从AFE中读取电压数据和温度数字
 * 参数个数：3
 * 参数描述：
 *           [IN]     addr        地址
 *           [IN/OUT] vol_buf     电压，以mV为单位
 *           [IN/OUT] temp_buf    温度，以mV为单位
 * 返 回 值：
 *           0        读取数据失效
 *           1        读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期           修改人      修改类型
 * 2021-05-04         戴辉发      创建
 * 2021-10-26         戴辉发      去除了单体电压和温度计算采用浮点
 *                                采取整形计算变通的方法
==============================================================*/
uint8_t read_voltage_temp(uint8_t addr, uint16_t *vol_buf, int16_t *temp_buf)
{
    uint32_t temp;
    uint8_t rec_data[28]; /* 接收到的数据 */
    uint8_t i;
    uint8_t cmd;

    SPI_Type *spi = M_AFE_SPI;
    
    if( seeBackupVoltageFlag != 0 )
        spi = R_AFE_SPI;
    
    cmd = cmd_from_data(addr, READ_VOLTAGE);

    if ( 27 == spi_read(spi, cmd, rec_data, 27) )
    {
        if (Crc_R(rec_data, 26) != rec_data[26])
        {
            return 0;
        }

        /* 读取正确的值 */
        for (i = 0; i < CELLS_NUM; i ++)
        {
            temp = (rec_data[2 * i] << 8) | rec_data[2 * i + 1];
            /* 采用单位为uV的计算，避免了浮点计算的方法 */
            temp = AD_V_Gain * temp + 500;
            if ((temp % 1000) >= 500)
            {
                /* 返回值采用的是mV */
                vol_buf[i] = (uint16_t)(temp / 1000) + 1;
            }
            else
            {
                vol_buf[i] = (uint16_t)(temp / 1000);
            }
        }
        for (; i < 3 + CELLS_NUM; i ++)
        {
            temp = (uint16_t)((rec_data[2 * i] << 8) | rec_data[2 * i + 1]);
            /* 采用单位为uV的计算，避免了浮点计算的方法,返回值采用的是mV */
            temp_buf[i - CELLS_NUM] = (uint16_t)((AD_V_Gain * temp + 500) / 1000);
        }
    }

    return 1;
}

/*=============================================================
 * 函数名称：read_current
 * 函数功能：从AFE中读取电流数据
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT] *curr_vol  分流器两端的电压
 * 返 回 值：
 *           0        读取数据失效
 *           1        读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_current(uint16_t *curr_vol)
{
    uint8_t rec_data[18];
    uint8_t ret_curr = 0;
    uint8_t cmd;
    SPI_Type *spi = M_AFE_SPI;
    
    if( seeBackupVoltageFlag != 0 )
        spi = R_AFE_SPI;
    cmd = cmd_from_data(0x00, READ_CURRENT);
    
    if ( 17 == spi_read(spi, cmd, rec_data, 17) )
    {
        if ( Crc_R(rec_data, 3) == rec_data[3] )
        {
            Para_Set_Flag = rec_data[0]>>4;
            //CurPara_Set_Flag = (uint8_t)((rec_data[0] & 0x80) >> 7); /* 电流参数设置OK标识 */
            //VolPara_Set_Flag = (uint8_t)((rec_data[0] & 0x40) >> 6); /* 电压参数设置OK标识 */
            //CtrPara_Set_Flag = (uint8_t)((rec_data[0] & 0x20) >> 5); /* 控制参数设置OK标识 */
            //BalPara_Set_Flag = (uint8_t)((rec_data[0] & 0x10) >> 4); /* 均衡设置OK */
            curr_vol[0] = rec_data[1];
            curr_vol[0] = (curr_vol[0] << 8) | rec_data[2];
            
            curr_vol[1] = rec_data[12]&0x0f;
            curr_vol[1] <<= 5;
            curr_vol[1] += (rec_data[13]>>3);
            curr_vol[1] <<= 7;

            ret_curr = 1;
        }
    }

    return ret_curr;
}
/*=============================================================
 * 函数名称：ReadParameteSetOverFlag
 * 函数功能：从AFE中读取电流数据
 * 参数个数：1
 * 参数描述：
 *           [IN/OUT] *curr_vol  分流器两端的电压
 * 返 回 值：
 *           0        读取数据失效
 *           1        读取数据成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t ReadParameteSetOverFlag(uint8_t *paraFlag)
{
    uint8_t rec_data[10];
    uint8_t ret_curr = 0;
    uint8_t cmd;
    *paraFlag = 0;
    SPI_Type *spi = M_AFE_SPI;     
    cmd = cmd_from_data(0x00, READ_CURRENT);    
    if ( 4 == spi_read(spi, cmd, rec_data, 4) )
    {
        if ( Crc_R(rec_data, 3) == rec_data[3] )
        {
            *paraFlag = rec_data[0]>>4;
            ret_curr |= 0x01;
        }
    }
    spi = R_AFE_SPI;
    cmd = cmd_from_data(0x00, READ_CURRENT);    
    if ( 4 == spi_read(spi, cmd, rec_data, 4) )
    {
        if ( Crc_R(rec_data, 3) == rec_data[3] )
        {
            *paraFlag += rec_data[0]&0xF0;
            ret_curr |= 0x02;
        }
    }
    return ret_curr;
}
/*=============================================================
 * 函数名称：get_para_set_flag
 * 函数功能：获取参数设置状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-22        liy      创建
==============================================================*/
uint8_t get_para_set_flag(void)
{
    return Para_Set_Flag;
}

/*=============================================================
 * 函数名称：clear_alarm
 * 函数功能：从AFE中读取电流数据
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void clear_alarm(void)
{
    uint8_t cmd = (CHIP0_ADDR << 4) | (CLEAR_IT_FLAG << 1) | (0x01);

    spi_write(M_AFE_SPI, &cmd, 1);
    spi_write(R_AFE_SPI, &cmd, 1);
}
/*=============================================================
 * 函数名称：afe_enter_normal
 * 函数功能：AFE模块进入正常状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t GetVoltageNotZero(uint8_t adr,SPI_Type * spi)
{
    uint8_t ret=0,i;
    uint8_t temp = 4;
    uint8_t cmd;
    uint8_t rec_data[27];
    
    cmd = cmd_from_data(adr, READ_VOLTAGE);
    
    if (( ret == 0 )&&(27 == spi_read(spi, cmd, rec_data, 27)))
    {
        if (Crc_R(rec_data, 26) == rec_data[26])
        {
            /* 读取正确的值 */
            for (i = 0; i < CELLS_NUM; i ++ )
            {
                temp = (uint16_t)((rec_data[2 * i] << 8) | rec_data[2 * i + 1]);
                
                if (temp == 0)
                {
                    break;
                }   
            }
            
            if ( i == CELLS_NUM )
            {
                ret = 1;
            }  
        }
    }
    
    return ret;
}
/*=============================================================
 * 函数名称：afe_enter_normal
 * 函数功能：AFE模块进入正常状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t afe_update_address(void)
{
    uint8_t ret=0;
    uint16_t i,j;


    /* 闲置状态 更新地址 */
    j = 0;
    do
    {
       ret = 0;
       for (i = 0; i < 1000; i ++);
       set_ctr_updateaddress();

       for (i = 0; i < 1000; i ++);
       clear_alarm();

       for (i = 0; i < 1000; i ++);
              
       ret =  GetVoltageNotZero(0x0,M_AFE_SPI);
       if( ret == 1 )
       {
           ret =  GetVoltageNotZero(0x0,R_AFE_SPI);     
	   }
       
       j++;
    }while(( ret != 1 ) && (j < 20));

    return ret;
}

/*=============================================================
 * 函数名称：afe_enter_normal
 * 函数功能：AFE模块进入正常状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t afe_enter_normal(void)
{
    uint8_t ret=0,bret=0;
    uint8_t temp = 4;
    uint8_t cmd;

    /* 在睡眠状态，则使用power up指令 */
    do
    {
        cmd = POWER_UP;
        if (( ret == 0 )&&(1 == spi_write(M_AFE_SPI, &cmd, 1)))
        {
            FL_DelayMs(5);
            /* 查询AFE TB引脚状态 */
            if( 0 != FL_GPIO_GetInputPin(AFE_TB_DETECT_GPIO_TYPE, AFE_TB_DETECT_GPIO_PIN) )
                ret = 1;
        }
        
        if (( bret == 0 )&&(1 == spi_write(R_AFE_SPI, &cmd, 1)))
        {
            FL_DelayMs(5);
            /* 查询AFE TB引脚状态 */
            if( 0 != FL_GPIO_GetInputPin(R_AFE_TB_DETECT_GPIO_TYPE, R_AFE_TB_DETECT_GPIO_PIN) )
                bret = 1;
        }
    }
    while ((-- temp) && (0 == (bret&ret)));

    return ret;
}
/*=============================================================
 * 函数名称：afe_update_adr
 * 函数功能：AFE更新地址指令
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           0       操作失败
 *           1       操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16         liyong      创建
==============================================================*/
uint8_t afe_update_adr(void)
{
    uint8_t ret=0;
    uint8_t temp = 4;
    uint8_t cmd;

    /* 在睡眠状态，则使用power up指令 */
    do
    {
        cmd = ADDR_UPDATE;
        ret=0;
        if (( ret == 0 )&&(1 == spi_write(M_AFE_SPI, &cmd, 1)))
        {
            ret = 1;
        }
        
        if (( ret == 1 )&&(1 == spi_write(R_AFE_SPI, &cmd, 1)))
        {    
            ret = 2;
        }
        
    }
    while ((-- temp) && ( 2 != ret));

    return ret;
}
/*=============================================================
 * 函数名称：read_alarm
 * 函数功能：从AFE中读取告警信息
 * 参数个数：2
 * 参数描述：
 *           [IN]    addr        AFE地址
 *           [IN]    data_buf    告警数据缓冲
 * 返 回 值：
 *           0       读取失败
 *           1       读取成功，数据有效
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_alarm(uint8_t addr, uint8_t *data_buf)
{
    uint8_t cmd;

    cmd = cmd_from_data(addr, READ_DETAIL_ALARM);

    if (10 == spi_read(M_AFE_SPI, cmd, data_buf, 10))
    {
        if (Crc_R(data_buf, 9) != data_buf[9])
        {
            return 0;
        }
    }

    return 1;
}
/*=============================================================
 * 函数名称：read_reserve_alarm
 * 函数功能：从 备用 AFE中读取告警信息
 * 参数个数：2
 * 参数描述：
 *           [IN]    addr        AFE地址
 *           [IN]    data_buf    告警数据缓冲
 * 返 回 值：
 *           0       读取失败
 *           1       读取成功，数据有效
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
uint8_t read_reserve_alarm(uint8_t addr, uint8_t *data_buf)
{
    uint8_t cmd;

    cmd = cmd_from_data(addr, READ_DETAIL_ALARM);

    if (10 == spi_read(R_AFE_SPI, cmd, data_buf, 10))
    {
        if (Crc_R(data_buf, 9) != data_buf[9])
        {
            return 0;
        }
    }

    return 1;
}
/*=============================================================
 * 函数名称：write_sysctrl_cmd
 * 函数功能：写系统告警设置命令
 * 参数个数：2
 * 参数描述：
 *           [IN]    ctrl1       控制字1
 *           [IN]    ctrl2       控制字2
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void write_sysctrl_cmd(uint8_t adr,uint8_t ctrl1, uint8_t ctrl2)
{
    uint8_t data[5];

    /* 系统控制命令，和均衡和告警设置为同一个寄存器表 */
    data[0] = adr| (CTR_SET << 1) | 0x00;
    /* 0x04命令码配置位，固定值 */
    data[1] = 0x00 | (SYS_SET << 5);
    data[2] = ctrl1;
    data[3] = ctrl2;
    data[4] = Crc_W(data,4);	

    spi_write(M_AFE_SPI, data, 5);
    spi_write(R_AFE_SPI, data, 5);
}

/*=============================================================
 * 函数名称：sysctrl_set
 * 函数功能：系统控制功率开关
 * 参数个数：2
 * 参数描述：
 *           [IN]    cflag       充电MOS控制
 *           [IN]    dflag       放电MOS控制
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void sysctrl_set(uint8_t cflag, uint8_t dflag)
{
    if (0 == cflag)
    {
        CTR_byte2.CHG_OUT = 0;
    }
    else
    {
        CTR_byte2.CHG_OUT = 1;
    }
    if (0 == dflag)
    {
        CTR_byte2.DSG_OUT = 0;
    }
    else
    {
        CTR_byte2.DSG_OUT = 1;
    }
    write_sysctrl_cmd(0x0,*((uint8_t *)(&CTR_byte1)), *((uint8_t *)(&CTR_byte2)));
}

/*=============================================================
 * 函数名称：SleepSysctrlSet
 * 函数功能：休眠系统控制功率开关
 * 参数个数：2
 * 参数描述：
 *           [IN]    cflag       充电MOS控制
 *           [IN]    dflag       放电MOS控制
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void SleepSysctrlSet(void)
{
    CTR_byte1.Charger_det = 1;//1：禁止充电器移除检测，0：使能充电器检测
    CTR_byte1.Short_det = 1;//使能短路检测
    CTR_byte1.Low_currrent = 1;//禁止小电流检测
    CTR_byte1.IC_ADD = 1; 
    CTR_byte1.rsvd1 = 0;
    CTR_byte1.rsvd10 = 0;
    CTR_byte1.Pdsg_out = 0;
    CTR_byte1.Pchg_out = 0;

    CTR_byte2.Rsvd2 = 0;
    CTR_byte2.Charger_cur = 0;
    CTR_byte2.Load_det = 0;  //VM负载检测模块启用控制 0:禁用  1:启用
    CTR_byte2.Rsvd3 = 0;
    CTR_byte2.CHG_CTR = 0;
    CTR_byte2.CHG_OUT = 0;
    CTR_byte2.DSG_CTR = 0;
    CTR_byte2.DSG_OUT = 0;
    write_sysctrl_cmd(0x0,*((uint8_t *)(&CTR_byte1)), *((uint8_t *)(&CTR_byte2)));
}

/*=============================================================
 * 函数名称：set_load_config
 * 函数功能：负载移除控制设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    statue       负载移除配置使能0：禁用   1：启用
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_load_config(uint8_t statue)
{
    if (0 == statue)
    {
        CTR_byte2.Load_det = 0;
    }
    else
    {
        CTR_byte2.Load_det = 1;
    }
     write_sysctrl_cmd(0x0,*((uint8_t *)(&CTR_byte1)), *((uint8_t *)(&CTR_byte2)));
}
/*=============================================================
 * 函数名称：set_charger_config
 * 函数功能：充电器移除控制设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    status       充电器移除配置使能0：禁用  1：启用
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_charger_config(uint8_t status)
{
    if (0 == status)
    {
        CTR_byte1.Charger_det = 1;
        CTR_byte2.Charger_cur = 0;
    }
    else
    {
        CTR_byte1.Charger_det = 0;
        CTR_byte2.Charger_cur = 0;
    }
    write_sysctrl_cmd(0x0,*((uint8_t *)(&CTR_byte1)), *((uint8_t *)(&CTR_byte2)));
}

/*=============================================================
 * 函数名称：Alarm_Set
 * 函数功能：AFE告警设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    mflag       告警设置标识
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void alarm_set(uint8_t mflag)
{
    uint8_t data[5];

    data[0] = (0x00 << 4) | (CTR_SET << 1) | (0x00);
    data[1] = 0x00 | (ALM_SET << 5);//02
    data[2] = 0x00 & (mflag << 3);/* 1:invalid 0:enable  */
    data[3] = 0x00;// Reserved
    data[4] = Crc_W(data, 4);	 

    spi_write(M_AFE_SPI, data, 5);
    spi_write(R_AFE_SPI, data, 5);
}

/*=============================================================
 * 函数名称：balsate
 * 函数功能：均衡设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    balsate     均衡状态, 按10节设置
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void balance_set(uint16_t balsate)
{
    uint8_t data[5];

    data[0] = (0x00 << 4) | (CTR_SET << 1) | (0x00);
    data[1] = 0x00 | (BAL_SET << 1);
    data[2] = (uint8_t)((balsate >> 8) & 0x03);
    data[3] = (uint8_t)(balsate & 0xFF);
    data[4] = Crc_W(data, 4);

    spi_write(M_AFE_SPI, data, 5);
}

/*=============================================================
 * 函数名称：inline_set_ctr_parameter
 * 函数功能：控制参数设置
 * 参数个数：3
 * 参数描述：
 *           [IN]    chip_num    AFE级联个数(0:表示1个)
 *           [IN]    cell_num    电池串数，实际串数
 *           [IN]    ship_mode   休眠是否进入SHIP_MODE
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
static void inline_set_ctr_parameter(uint8_t adr,uint8_t chip_num, uint8_t cell_num, uint8_t ship_mode, uint8_t updateAdr)
{
    uint8_t data[10];

    // CHIP0
    data[0] = (adr<<4 )|0x0d;
    // allow alarm pin to turn off the sending of CO / DO
    data[1] = ((0x00 << 7) | (0x00 << 2) | (0x00 << 1) | (0x00));
    /*  */
    data[2] = 0x64|(updateAdr<<7); //0x7 和 0x06 的区别似乎是 sleep状态的SPI功能被关闭掉了
    data[3] = 0x28;//使能CSB唤醒
    /* FORCE 1M clk */
    data[4] = (uint8_t)((chip_num << 4) | (0x00 << 3) | (0x00 << 2) | ((ship_mode & 0x01) << 1) | (0x00));
    /* Set the failure time of SPI communication; when no SPI 
       communication state lasts for [31:24] * 512ms, if the balancing is on, 
       the balancing will be turned off; if any SPI instruction is sent, it will 
       recover */
    data[5] = 0x78;
    /* ADC POLL status */
    data[6] = 0x00;
    /*  */
    data[7] = 0x11;
    /* 配置电池节数为8节 */
    data[8] = 0xF0 | ((cell_num - 1) & 0x0f);
    data[9] = Crc_W(data, 9);

    spi_write(M_AFE_SPI, data, 10);
    spi_write(R_AFE_SPI, data, 10);
}

/*=============================================================
 * 函数名称：set_ctr_parameter
 * 函数功能：控制参数设置
 * 参数个数：2
 * 参数描述：
 *         
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-16        liyong      创建
==============================================================*/
void set_ctr_updateaddress(void)
{
    //inline_set_ctr_parameter(CHIP0_ADDR,0x00, BAT_NUM, 0, 0);
    //afe_update_adr();
}
/*=============================================================
 * 函数名称：set_ctr_parameter
 * 函数功能：控制参数设置
 * 参数个数：2
 * 参数描述：
 *           [IN]    chip_num    AFE级联个数(0:表示1个)
 *           [IN]    cell_num    电池串数，实际串数
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_ctr_parameter(uint8_t chip_num, uint8_t cell_num)
{
    inline_set_ctr_parameter(CHIP0_ADDR,0x0, BAT_NUM, 0, 1);
}
/*=============================================================
 * 函数名称：sleep_enter_ship
 * 函数功能：从sleep进入ship
 * 参数个数：
 * 参数描述：
 *           
 *           
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-07-15         李勇      创建
==============================================================*/
void sleep_enter_ship(void)
{  
    inline_set_ctr_parameter(CHIP0_ADDR,0x0, BAT_NUM, 1, 1);    
}
/*=============================================================
 * 函数名称：set_voltage_parameter
 * 函数功能：电压参数设置
 * 参数个数：4
 * 参数描述：
 *           [IN]    ovp_vol     单体过压参数
 *           [IN]    uvp_vol     单体欠压参数
 *           [IN]    ovt_temp    温度欠温参数
 *           [IN]    uvt_temp    温度过温参数
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_voltage_parameter(uint16_t ovp_vol, uint16_t uvp_vol, int16_t ovt_temp, int16_t uvt_temp,uint8_t blance_mode,uint8_t blance_mode_time ,uint8_t break_line)
{
    uint8_t data[12];

    data[0] = SET_VOLTAGE_PRAM;
    /* 设置放电过温保护阈值[79:72] * 64 * 5 / 16384 */
    data[1] = ovt_temp;
    /* 设置放电欠温保护阈值[79:72] * 64 * 5 / 16384 */
    data[2] = uvt_temp;
    data[3] = (uint8_t)((0x01 << 6) | /* 固定设置为512MS的步进 */
                        (0x00 << 4) | /* 固定设置为512MS的步进 */
                        (0x00 << 3) | /* TS3作为PDSG电路使用 ，启用，作为AD采样基准电压使用 */
                        (0x01 << 2) | /* Forced to 1 */
                        (0x00 << 1) | /* Forced to 0 */
                        (break_line & 0x01)); /* 1: the open line detection function is enabled */
    data[4] = (uint8_t)((0x00 << 7) | /* Forced to 0 */
                        (0x00 << 6) | /* Forced to 0 */
                        (0x01 << 4) | /* Forced to 1 */
                        (blance_mode << 3) | /* 0: fast mode (Recommended) */
                        (blance_mode_time << 2) | /* Voltage ADC measurement handing, 1: The processing time is determined by bit[49:48] setting */
                        (0x03 & 0x03)); /* waiting time:1024uS, sampling time:1024uS */
    data[5] = (uint8_t)(((0x06 & 0x0f) << 4) | (0x06 & 0x0f));//设置过压保护时间3072mS,设置欠压保护时间为3072mS
    data[6] = (uint8_t)(((0x06 & 0x0f) << 4) | (0x06 & 0x0f));//设置过温保护时间3072mS,设置欠温保护时间为3072mS
    data[7] = (uint8_t)((ovp_vol / 1000.0) * (16384.0 / 320));//设置高压阈值
    data[8] = (uint8_t)((uvp_vol / 1000.0) * (16384.0 / 320));//设置低压阈值2305mV 76 1490mV  4c                                                  
    data[9] = (uint8_t)(ovt_temp);  /* 设置高温阈值 */
    data[10] = (uint8_t)(uvt_temp); /* 设置低温阈值 */
    data[11] = Crc_W(data, 11);

    spi_write(M_AFE_SPI, data, 12);
    spi_write(R_AFE_SPI, data, 12);
}
/*=============================================================
 * 函数名称：GetOverCurrent
 * 函数功能：电流参数设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    cur   过流值
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2022-03-10        liyong     创建
==============================================================*/
uint8_t GetOverCurrent(int16_t cur)
{
    uint8_t setOverVaule=0xff;
    float curr=cur;
    
    if( curr < 0 ) curr=-curr;
 
    if ( 0 == g_adjust_curr_para.k_value )
    {
      curr = curr * CURRENT_RVALUE; 
    }
    else
    {
      curr = curr * g_adjust_curr_para.k_value ;
    }
    
    if( curr > 200.0 ) 
    {
        curr = 200.0;
    }

    setOverVaule = (uint8_t)(curr*256/200+0.5);
 
    return setOverVaule;
}
/*=============================================================
 * 函数名称：set_current_parameter
 * 函数功能：电流参数设置
 * 参数个数：1
 * 参数描述：
 *           [IN]    short_cur   短路电流
 *           [IN]    sc_delay    短路延时，以uS为单位
 *           [IN]    ocp_dch     AFE放电过流保护门限，以mV为单位
 *           [IN]    ocp_ch      AFE充电过流保护门限，以mV为单位
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-05-04        戴辉发      创建
==============================================================*/
void set_current_parameter(int16_t short_cur, uint16_t sc_delay, int16_t ocp_dch, int16_t ocp_ch)
{
    uint8_t data[12];
    uint8_t c_temp;
    uint16_t ocp_dch2 = ocp_dch+20;
    float curr;

    /* 计算短路电流设置值 */
    curr = short_cur;
    if (short_cur < 0)
    {
        curr = 0 - short_cur;
    }
    
    if ( 0 == g_adjust_curr_para.k_value )
    {
      curr = curr * CURRENT_RVALUE - 42.5;
    }
    else
    {
      curr = curr * g_adjust_curr_para.k_value - 42.5;
    }
    
    if (curr >= 240)
    {
        c_temp = 1;
        curr -= 240;
    }
    else
    {
        c_temp = 0;
    }
    c_temp <<= 1;
    if (curr >= 120)
    {
        curr -= 120;
    }
    else
    {
        c_temp += 1;
    }
    c_temp <<= 1;
    if (curr >= 60)
    {
        curr -= 60;
    }
    else
    {
        c_temp += 1;
    }
    c_temp <<= 1;
    if (curr >= 30)
    {
        curr -= 30;
    }
    else
    {
        c_temp += 1;
    }
    c_temp <<= 1;
    if (curr >= 15)
    {
        curr -= 15;
    }
    else
    {
        c_temp += 1;
    }
    c_temp <<= 1;
    if (curr < 7.5)
    {
        c_temp += 1;
    }
    /* 短路延时 */
    sc_delay = ((sc_delay + 63) / 64) & 0x1F;
    if (sc_delay > 0)
    {
        sc_delay -= 1;
    }

    /* 计算AFE放电过流保护电流设置值  设置一级过流延时为4s */
    ocp_dch = GetOverCurrent(ocp_dch);
     /* 计算AFE放电过流保护电流设置值  设置二级过流延时为1s */
    ocp_dch2 = GetOverCurrent(ocp_dch2);
    /* 计算AFE充电过流保护电流设置值 0.96s */
    ocp_ch = GetOverCurrent(ocp_ch);

    data[0] = SET_CURRENT_PRAM;
    data[1] = (uint8_t)((0x07 << 4) | (c_temp >> 2));/* 设置充放电检测阈值为0.6836mV,设置短路保护阈值为270mVRcs = 2.5mR  Isht=108A */
    data[2] = (uint8_t)(((c_temp & 0x0003) << 6) | 
                        (0x01 << 5) | /* Forced to 1 */
                        sc_delay); /* Short circuit delay time in discharge: ([68:64] + 1) * 64us */
    data[3] = (uint8_t)((0x02 << 6) | /* Low current wakeup threshold (VLC): */
                        (0x00 << 4) | /* Forced to 0 ,Note:The current whill jump when set to "1" !*/
                       0x0F); /* charge over current protection delay time；[59:56]*64ms */
    /*1s*/
    data[4] = (uint8_t)((0x1f << 3) |  /* The second grade over current protection delay time: [55:51]*step */
                        ((15) >> 2)); /* The first grade over current protection delay time: [50:46]*256ms+128ms */
    /*4s*/
    data[5] = (uint8_t)(((15 & 0x03) << 6) | /* The first grade over current protection delay time: [50:46]*256ms+128ms */
                        (0x01 << 5) | /* time step for secondary discharge over current protection, 0 : 4ms 1 : 32ms */
                        0x00); /* Reserved */
    data[6] = (uint8_t)(ocp_dch2); /* The second grade over current protection threshold voltage in discharge:[39:32]*0.4/512 */
    data[7] = (uint8_t)(ocp_dch); /* The first grade over current protection threshold voltage in discharge:[31:24]*0.4/512 */
    data[8] = (uint8_t)(0x00); /* Reserved */
    data[9] = (uint8_t)(ocp_ch); /* The over current protection threshold voltage in charge: -[15:8])*0.4/512 */
    data[10]= (uint8_t)(0x00); /* Reserved */
    data[11]= Crc_W(data,11);

    spi_write(M_AFE_SPI, data, 12);
    spi_write(R_AFE_SPI, data, 12);
}
