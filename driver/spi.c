/*---------------------------------------------------------*
 * Copyright (C) 2021 杭州优恩捷科技有限公司。版权所有。
 *
 * 文件名：spi.c
 * 文件功能描述：实现底层SPI通信控制操作
 *
 * 修改记录：
 * 2021-06-01 戴辉发 创建
*----------------------------------------------------------*/

#include "spi.h"

/* 戴总说是为了级联而做延时，暂时不起用这个延时选项 */
/* #define  SPI_CS_DELAY */

/*=============================================================
 * 函数名称：spi_init
 * 函数功能：SPI硬件初始化
 * 参数个数：1
 * 参数描述：
 *           [IN]     spi        指定SPI
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
void spi_init(SPI_Type *spi)
{
    FL_SPI_InitTypeDef spiInitStruct = {0};
#if 0
    FL_DMA_InitTypeDef dmaInitStruct = {0};
#endif

    /* 首先恢复SPI默认设置 */
    FL_SPI_DeInit(spi);

    /* SPI数据结构变量初始化 */
    FL_SPI_StructInit(&spiInitStruct);
    /* 初始化为750K波特率 */
    spiInitStruct.baudRate = FL_SPI_BAUDRATE_DIV64;
    spiInitStruct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    /* 时钟相位第二时钟沿 */
    spiInitStruct.clockPhase = FL_SPI_PHASE_EDGE2;
    /* 时钟极性定义为反转时钟 */
    spiInitStruct.clockPolarity = FL_SPI_POLARITY_INVERT;
    spiInitStruct.dataWidth = FL_SPI_DATA_WIDTH_8B;
    spiInitStruct.mode = FL_SPI_WORK_MODE_MASTER;
    spiInitStruct.softControl = FL_ENABLE;
    spiInitStruct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    FL_SPI_Init(spi, &spiInitStruct);

#if 0
    // RX
    dmaInitStruct.circMode = FL_DISABLE;
    dmaInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    dmaInitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;
    dmaInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    dmaInitStruct.priority = FL_DMA_PRIORITY_HIGH ;
    dmaInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;
    FL_DMA_Init(DMA, &dmaInitStruct, FL_DMA_CHANNEL_3);

    // TX
    dmaInitStruct.circMode = FL_DISABLE;
    dmaInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    dmaInitStruct.direction = FL_DMA_DIR_RAM_TO_PERIPHERAL;
    dmaInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    dmaInitStruct.priority = FL_DMA_PRIORITY_HIGH;
    dmaInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION2;
    FL_DMA_Init(DMA, &dmaInitStruct, FL_DMA_CHANNEL_4);
#endif
    /* 片选信号默认拉高 */
    FL_SPI_SetSSNPin(spi, FL_SPI_SSN_HIGH);
}

/*=============================================================
 * 函数名称：spi_deinit
 * 函数功能：SPI硬件休眠
 * 参数个数：1
 * 参数描述：
 *           [IN]     spi        指定SPI
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
void spi_deinit(SPI_Type *spi)
{
     /* 首先恢复SPI默认设置 */
    FL_SPI_DeInit(spi);
}

#if defined(SPI_CS_DELAY)
/*=============================================================
 * 函数名称：spi_cs_delay
 * 函数功能：SPI片选延迟
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           无
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
static void spi_cs_delay(void)
{
    uint16_t i;
    for (i = 0; i < 450; i ++);
}
#endif

/*=============================================================
 * 函数名称：spi_single_read_write
 * 函数功能：SPI发送和接收单个字节数据
 * 参数个数：3
 * 参数描述：
 *          [IN]      spi        操作的SPI接口
 *          [IN]      data       本次发送数据
 *          [IN/OUT]  recv_data  本次接收数据
 * 返 回 值：
 *          0         操作失败
 *          1         操作成功
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
static uint8_t spi_single_read_write(SPI_Type *spi, uint8_t data, uint8_t *recv_data)
{
    uint32_t temp;
    uint8_t ret = 0;

    temp = 1000;
    /* 判决SPI发送缓冲空 */
    while (1)
    {
        if (FL_SPI_IsActiveFlag_TXBuffEmpty(spi))
        {
            break;
        }
        temp -= 1;
        if (0 == temp)
        {
            break;
        }
    }
    /* 允许发送数据 */
    if (temp > 0)
    {
        FL_SPI_WriteTXBuff(spi, data);
        /* 判断接收寄存器满 */
        temp = 1000;
        while (1)
        {
            if (FL_SPI_IsActiveFlag_RXBuffFull(spi))
            {
                break;
            }
            temp -= 1;
            if (0 == temp)
            {
                break;
            }
        }
        /* 满足接收数据条件 */
        if (temp > 0)
        {
            *recv_data = FL_SPI_ReadRXBuff(spi);
            ret = 1;
        }
    }

    return ret;
}

/*=============================================================
 * 函数名称：spi_write
 * 函数功能：SPI发送指定字节数据
 * 参数个数：3
 * 参数描述：
 *          [IN]      spi        操作的SPI接口
 *          [IN]      data_buf   本次发送的数据缓冲
 *          [IN]      len        本次发送数据长度
 * 返 回 值：
 *          返回本次发送成功数据个数，以字节为单位
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
uint8_t spi_write(SPI_Type *spi, uint8_t *data_buf, uint8_t len)
{
    uint8_t i;
    uint8_t temp;

    /* 拉低片选信号引脚 */
    FL_SPI_SetSSNPin(spi, FL_SPI_SSN_LOW);
    /* 需要延时，则延时 */
#if defined(SPI_CS_DELAY)
    spi_cs_delay();
#endif
    for (i = 0; i < len; i ++)
    {
        /* 本次发送数据失败，退出本次数据发送 */
        if (0 == spi_single_read_write(spi, data_buf[i], &temp))
        {
            break;
        }
    }
    /* 需要延时，则延时 */
#if defined(SPI_CS_DELAY)
    spi_cs_delay();
#endif
    FL_SPI_SetSSNPin(spi, FL_SPI_SSN_HIGH);

    return i;
}

/*=============================================================
 * 函数名称：spi_write_read
 * 函数功能：SPI发送指定字节数据
 * 参数个数：4
 * 参数描述：
 *          [IN]      spi        操作的SPI接口
 *          [IN]      cmd        读取标识
 *          [IN/OUT]  data_buf   接收数据缓冲
 *          [IN]      len        需要接收数据长度
 * 返 回 值：
 *          返回本次成功读取到的数据个数，以字节为单位
 * 修改记录：
 *=============================================================
 * 日    期          修改人      修改类型
 * 2021-06-01        戴辉发      创建
==============================================================*/
uint8_t spi_read(SPI_Type *spi, uint8_t cmd, uint8_t *data_buf, uint8_t len)
{
    uint8_t i;
    uint8_t temp;

    /* 拉低片选信号引脚 */
    FL_SPI_SetSSNPin(spi, FL_SPI_SSN_LOW);
    /* 需要延时，则延时 */
#if defined(SPI_CS_DELAY)
    spi_cs_delay();
#endif
    /* 发送读取标识 */
    spi_single_read_write(spi, cmd, &temp);
    temp = 0xFF;
    for (i = 0; i < len; i ++)
    {
        /* 本次发送数据失败，退出本次数据发送 */
        if (0 == spi_single_read_write(spi, temp, &data_buf[i]))
        {
            break;
        }
    }
    /* 需要延时，则延时 */
#if defined(SPI_CS_DELAY)
    spi_cs_delay();
#endif
    FL_SPI_SetSSNPin(spi, FL_SPI_SSN_HIGH);

    return i;
}
