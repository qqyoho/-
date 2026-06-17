#include "uart.h"
#include "fm33lg0xx_fl.h"
#include "uart_dma.h"
#include "bluetooth.h"

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init UART3
**FuncName: uart_init(uint16_t bps)
**Create  : weihl
**Modify  : 
*******************************************************************************/
void uart_init(uint32_t bps)
{
    FL_UART_InitTypeDef UART_InitStruct;
    
    FL_UART_StructInit( &UART_InitStruct );
    /* USART configure */
    FL_UART_DeInit(UART0);
    UART_InitStruct.baudRate            = bps;                                 //波特率
    UART_InitStruct.dataWidth           = FL_UART_DATA_WIDTH_8B;                    //数据位数
    UART_InitStruct.stopBits            = FL_UART_STOP_BIT_WIDTH_1B;                //停止位
    UART_InitStruct.parity              = FL_UART_PARITY_NONE;                      //奇偶校验
    UART_InitStruct.transferDirection   = FL_UART_DIRECTION_TX_RX;                  //接收-发送使能
    FL_UART_Init(UART0, &UART_InitStruct);
    FL_UART_SetTXIFMode(UART0,FL_UART_TXIF_MODE_AFTER_DMA);
    UART_RD_REV;
    /*设置接受超时中断*/

/*    FL_UART_WriteRXTimeout( UART0 , 20 );
    FL_UART_ClearFlag_RXBuffTimeout(UART0);
    FL_UART_EnableIT_RXTimeout( UART0 );
    FL_UART_EnableRXTimeout( UART0 );*/
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init UART3
**FuncName: uart_deinit
**Create  : weihl
**Modify  : 
*******************************************************************************/
void uart_deinit(void)
{
	 FL_UART_DeInit(UART0);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: uart_send_data
**Create  : hfdai@hznegt.com
**Modify  : 
*******************************************************************************/
void uart_send_data(uint8_t *data_buf, uint8_t data_length)
{
    g_send_over_flag = 2;
    dma_uart_send_data(data_buf, data_length);
}

void UART0_IRQHandler()
{
    if( FL_UART_IsActiveFlag_RXBuffTimeout(UART0) != 0 )
    {
        UsartRecivedIdleGetData();
        FL_UART_ClearFlag_RXBuffTimeout(UART0);
    }
}