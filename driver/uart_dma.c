#include "uart_dma.h"
#include "bluetooth.h"
#include "uart.h"

uint8_t uart_rx_dma_buf[UART_RX_DMA_NUM*2];

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init dma for uart1
**FuncName: dma_uart_init()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void dma_uart_init(uint8_t *data_buf)
{
    FL_DMA_InitTypeDef dmaInitStruct = {0};
	
     // RX
    FL_DMA_DisableChannel(DMA, UART_RX_DMA_Channel);

    FL_DMA_StructInit(&dmaInitStruct);
    dmaInitStruct.circMode = FL_DISABLE;
    dmaInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    dmaInitStruct.direction = FL_DMA_DIR_PERIPHERAL_TO_RAM;
    dmaInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    dmaInitStruct.priority = FL_DMA_PRIORITY_HIGH ;
    dmaInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION2;
    FL_DMA_Init(DMA, &dmaInitStruct, UART_RX_DMA_Channel);

    // TX
    FL_DMA_DisableChannel(DMA, UART_TX_DMA_Channel);
    FL_DMA_StructInit(&dmaInitStruct);
    dmaInitStruct.circMode = FL_DISABLE;
    dmaInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    dmaInitStruct.direction = FL_DMA_DIR_RAM_TO_PERIPHERAL;
    dmaInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    dmaInitStruct.priority = FL_DMA_PRIORITY_MEDIUM ;
    dmaInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION3;
    FL_DMA_Init(DMA, &dmaInitStruct, UART_TX_DMA_Channel);
      
	NVIC_DisableIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, 2); //中断优先级配置
    NVIC_EnableIRQ(DMA_IRQn);
    
    //Enable DMA Transfer complete interrupt
    FL_DMA_WriteTransmissionSize(DMA, UART_RX_DMA_NUM - 1, UART_RX_DMA_Channel);
    FL_DMA_WriteMemoryAddress(DMA, (uint32_t)uart_rx_dma_buf, UART_RX_DMA_Channel);
    FL_DMA_ClearFlag_TransferComplete(DMA, UART_RX_DMA_Channel);
    FL_DMA_EnableIT_TransferComplete(DMA, UART_RX_DMA_Channel);
    FL_DMA_EnableChannel(DMA, UART_RX_DMA_Channel);
    
    FL_DMA_ClearFlag_TransferComplete(DMA, UART_TX_DMA_Channel);
    FL_DMA_EnableIT_TransferComplete(DMA, UART_TX_DMA_Channel);
    FL_DMA_EnableChannel(DMA, UART_TX_DMA_Channel);
 
    FL_DMA_Enable(DMA);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init dma for uart1
**FuncName: dma_uart_deinit()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void dma_uart_deinit(void)
{     
	FL_DMA_ClearFlag_TransferComplete(DMA, UART_RX_DMA_Channel);
	FL_DMA_ClearFlag_TransferComplete(DMA, UART_TX_DMA_Channel);
    
    FL_DMA_DisableIT_TransferComplete(DMA, UART_RX_DMA_Channel);
    FL_DMA_DisableIT_TransferComplete(DMA, UART_TX_DMA_Channel);
    
    //Deinitializes the UART_RX_DMA_Channel to default reset values
    FL_DMA_DisableChannel(DMA, UART_RX_DMA_Channel);
    FL_DMA_DisableChannel(DMA, UART_TX_DMA_Channel);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : uart1 send data through dma
**FuncName: dma_uart_send_data()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void dma_uart_send_data(uint8_t *data_buf, uint8_t txlength)
{
    FL_DMA_DisableChannel(DMA, UART_TX_DMA_Channel);
    FL_DMA_WriteTransmissionSize(DMA, txlength-1, UART_TX_DMA_Channel);
    FL_DMA_WriteMemoryAddress(DMA, (uint32_t)data_buf, UART_TX_DMA_Channel);
    FL_DMA_EnableChannel(DMA, UART_TX_DMA_Channel);
}


/*******************************************************************************
**Input   : No
**Output  : No
**Explain : uart receive data thirough dma
**FuncName: dma_uart1_receive_data()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void dma_uart_receive_data(uint8_t len)
{
	uint8_t i;

	/* disable DMA channel4 */
    FL_DMA_DisableChannel(DMA, UART_RX_DMA_Channel);	
	for (i = 0; i < len; i ++)
	{
		blueRecvData_from_dma(uart_rx_dma_buf[i]);
	}
    FL_DMA_WriteTransmissionSize(DMA, UART_RX_DMA_NUM-1,UART_RX_DMA_Channel);
    FL_DMA_WriteMemoryAddress(DMA, (uint32_t)uart_rx_dma_buf, UART_RX_DMA_Channel);
	/* enable DMA channel4 */
    FL_DMA_EnableChannel(DMA, UART_RX_DMA_Channel);

////    send_data_to_can(len);
}
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
void  UsartRecivedIdleGetData(void)
{
    uint8_t *p1 = uart_rx_dma_buf;
    uint8_t *p2 = (uint8_t *)(DMA->CH1MAR);
    if( p2 > p1 )
        dma_uart_receive_data(p2-p1);
}

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: DMA_IRQHandler
**Create  : ly
**Modify  : 
*******************************************************************************/
void DMA_IRQHandler(void)
{
	//DMA1_Channel3 IRQ
	if ( FL_DMA_IsActiveFlag_TransferComplete( DMA, UART_TX_DMA_Channel) != 0 )
	{
		FL_DMA_ClearFlag_TransferComplete( DMA, UART_TX_DMA_Channel );
        g_send_over_flag = 1;
	}
    
    if ( FL_DMA_IsActiveFlag_TransferComplete( DMA, UART_RX_DMA_Channel) != 0 )
	{
		FL_DMA_ClearFlag_TransferComplete( DMA, UART_RX_DMA_Channel );
        dma_uart_receive_data(UART_RX_DMA_NUM);
	}
}