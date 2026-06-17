#ifndef __TIMER_H__
#define __TIMER_H__



/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 10mS IRQ
**FuncName: ATIM_Init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void ATIM_Init(uint16_t arr, uint16_t psc);

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 100mS IRQ
**FuncName: ATIM_Init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void GpTimer0_init(uint16_t pre,uint16_t reload );

/*******************************************************************************
**Input   : No
**Output  : No
**Explain : 
**FuncName: TIM7_IRQHandler
**Create  : weihl
**Modify  : 
*******************************************************************************/
void TIMER2_IRQHandler(void);
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : LPTIM16关闭  
**FuncName: LPTIM16_close_init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void LPTIM16_close_init(void);
/*******************************************************************************
**Input   : No
**Output  : No
**Explain : LPTIM16初始化  800ms唤醒一次，喂硬件狗
**FuncName: LPTIM16_wakeup_init();//
**Create  : ly
**Modify  : 
*******************************************************************************/
void LPTIM16_wakeup_init(void);
#endif
