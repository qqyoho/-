#ifndef __UART_H__
#define __UART_H__

#include "stdint.h"

#if 1
#define UART_BPS 9600
#else
#define UART_BPS 115200
#endif

#define UART_RD_SND     FL_GPIO_SetOutputPin( GPIOA, FL_GPIO_PIN_4)
#define UART_RD_REV     FL_GPIO_ResetOutputPin( GPIOA, FL_GPIO_PIN_4)

void uart_init(uint32_t bps);

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Init UART3
**FuncName: uart_deinit
**Create  : weihl
**Modify  : 
*******************************************************************************/
void uart_deinit(void);

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: uart_send_data
**Create  : hfdai@hznegt.com
**Modify  : 
*******************************************************************************/
void uart_send_data(uint8_t *data_buf, uint8_t data_length);
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : Enable GPIO of UART3
**FuncName: uart_gpio_init()
**Create  : weihl
**Modify  : 
*******************************************************************************/
void uart_gpio_init(void);
#endif
