#ifndef __DMA_H__
#define __DMA_H__

#include "stdint.h"

#include "fm33lg0xx_fl.h"

#define UART_TX_DMA_Channel	FL_DMA_CHANNEL_2
#define UART_RX_DMA_Channel	FL_DMA_CHANNEL_1

#define UART_TX_DMA_NUM		128
#define UART_RX_DMA_NUM		11

void dma_uart_init(uint8_t *data_buf);
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init dma for uart1
**FuncName: dma_uart_deinit()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void dma_uart_deinit(void);

void dma_uart_send_data(uint8_t *data_buf, uint8_t num);

/*==============================================================
* 函数名称：UsartRecivedIdleGetData
* 函数功能：usart空闲接受数据
* 参数个数：0
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期                修改人             修改内容
* 2021-12-28          liyong             创建
==============================================================*/
void  UsartRecivedIdleGetData(void);
#endif
