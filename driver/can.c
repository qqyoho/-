/*---------------------------------------------------------*
* Copyright (C) 2017 艾启新能源有限公司。版权所有。
*
* 文件名：can.c
* 文件功能描述：实现CAN硬件底层驱动，根据示例代码修改
*
* 修改记录：
* 2017-10-06 戴辉发 创建
*----------------------------------------------------------*/
#include "can.h"
#include "fm33lg0xx_fl.h"
#include "string.h"
#include "can_app.h"
#include "protect_record.h"
#include "parameter.h"
#include "power.h"

#define MAX_FILTER_NUM           (14)
#define MAX_CAN_RECV_NUM         (64)
#define MAX_CAN_SEND_NUM         (64)

typedef struct _T_CAN_BAUDRATE_CONFIG_TYPE_
{
	uint16_t prescaler;   /*!< Specifies the length of a time quantum. 
	                             It ranges from 1 to 1024. */

	uint8_t sjw;          /*!< Specifies the maximum number of time quanta 
	                             the CAN hardware is allowed to lengthen or 
	                             shorten a bit to perform resynchronization.
	                             This parameter can be a value of 
	                             @ref CAN_synchronisation_jump_width */

	uint8_t bs1;          /*!< Specifies the number of time quanta in Bit 
	                             Segment 1. This parameter can be a value of 
	                             @ref CAN_time_quantum_in_bit_segment_1 */

	uint8_t bs2;          /*!< Specifies the number of time quanta in Bit 
	                             Segment 2.
	                             This parameter can be a value of 
	                             @ref CAN_time_quantum_in_bit_segment_2 */
}t_can_baudrate_config;
//使用PLL的配置
//static const t_can_baudrate_config g_can_baudrate_config[E_CAN_BAUDRATE_MAX_NUM] = 
//{
//	/* 100K */
//	{
//		59, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
//	}, 
//	/* 125K */
//	{
//		47, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
//	}, 
//	/* 250K */
//	{
//		23, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
//	}, 
//	/* 500K */
//	{
//		11, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
//	},     
//	/* 800k */
//	{
//		11, FL_CAN_SJW_1Tq, FL_CAN_TS1_3Tq, FL_CAN_TS2_1Tq
//	},
//	/* 1M */
//	{
//		11, FL_CAN_SJW_1Tq, FL_CAN_TS1_2Tq, FL_CAN_TS2_1Tq
//	} 
//};

static const t_can_baudrate_config g_can_baudrate_config[E_CAN_BAUDRATE_MAX_NUM] = 
{
	/* 100K */
	{
		9, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
	}, 
	/* 125K */
	{
		7, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
	}, 
	/* 250K */
	{
		3, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_1Tq
	}, 
	/* 500K */
	{
		0, FL_CAN_SJW_1Tq, FL_CAN_TS1_12Tq, FL_CAN_TS2_3Tq
	},     
	/* 800k */
	{
		0, FL_CAN_SJW_1Tq, FL_CAN_TS1_6Tq, FL_CAN_TS2_3Tq
	},
	/* 1M */
	{
		0, FL_CAN_SJW_1Tq, FL_CAN_TS1_4Tq, FL_CAN_TS2_3Tq
	} 
};
static uint8_t g_can_filter_flag[MAX_FILTER_NUM];
static uint8_t g_recv_head;
static uint8_t g_recv_tail;
static CanRxMsg g_recv_buf[MAX_CAN_RECV_NUM];
static uint8_t g_send_head;
static uint8_t g_send_tail;
static CanTxMsg g_send_buf[MAX_CAN_SEND_NUM];

const static uint16_t std_buf[] = {0x580 + NODE_ID, 0x600 + NODE_ID, /* 0x2F0, NODE_ID, */0x7F1, 0x5ac, 0x0000,0x180 + CONTROL_ID,0x357};
const static uint32_t ext_buf[] = {0x18FFF4A0, 0x1826F456, 0x1CECFF3C, 0x1CEBFF3C};

/*=============================================================
 * 函数名称：can_mem_init
 * 函数功能：can模块底层内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2017-11-13       戴辉发     创建
==============================================================*/
void can_mem_init(void)
{
	memset(g_can_filter_flag, 0, MAX_FILTER_NUM * sizeof(uint8_t));
	g_recv_head = 0;
	g_recv_tail = 0;
	g_send_head = 0;
	g_send_tail = 0;
}

/*=============================================================
 * 函数名称：clear_can_send_buff
 * 函数功能：清除发送缓冲
 * 参数个数：0
 * 函数参数：
 *           [IN]   tx_msg     本次发送数据
 * 返 回 值：
 *           写入数据长度
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-07-09       戴辉发     创建
==============================================================*/
void clear_can_send_buff(void)
{
	g_send_head = 0;
	g_send_tail = 0;
}

/*=============================================================
 * 函数名称：send_data_to_can_send_buff
 * 函数功能：将准备写入数据放入到发送缓冲中
 * 参数个数：1
 * 函数参数：
 *           [IN]   tx_msg     本次发送数据
 * 返 回 值：
 *           写入数据长度
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-07-09       戴辉发     创建
==============================================================*/
static uint8_t send_data_to_can_send_buff(CanTxMsg *tx_msg)
{
	memcpy(&g_send_buf[g_send_tail], tx_msg, sizeof(CanTxMsg));
	g_send_tail += 1;
	if (g_send_tail >= MAX_CAN_SEND_NUM) g_send_tail = 0;

	return sizeof(CanTxMsg);
}

/*=============================================================
 * 函数名称：get_data_from_can_send_buff
 * 函数功能：从发送缓冲中读取一包数据
 * 参数个数：1
 * 函数参数：
 *           [IN/OUT]   tx_msg     数据缓冲
 * 返 回 值：
 *           0                     表示没有取到数据
 *           sizeof(CanTxMsg)      获取到一包数据
 * 修改记录:
 *==============================================================
 * 日期                 修改人     修改内容
 * 2018-07-09           戴辉发     创建
==============================================================*/
static uint8_t get_data_from_can_send_buff(CanTxMsg *tx_msg)
{
	if (g_send_head != g_send_tail)
	{
		memcpy(tx_msg, &g_send_buf[g_send_head], sizeof(CanTxMsg));

		return sizeof(CanTxMsg);
	}
	return 0;
}

/*=============================================================
 * 函数名称：remove_data_from_can_send_buff
 * 函数功能：从发送缓冲中移除一包数据
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期                 修改人     修改内容
 * 2018-07-09           戴辉发     创建
==============================================================*/
static void remove_data_from_can_send_buff(void)
{
	g_send_head += 1;
	if (g_send_head >= MAX_CAN_SEND_NUM) g_send_head = 0;
}

/*=============================================================
 * 函数名称：send_data_to_can_recv_buff
 * 函数功能：将数据放入到接收缓冲中
 * 参数个数：1
 * 函数参数：
 *           [IN]   rx_msg     本次写入数据缓冲
 * 返 回 值：
 *           写入数据长度
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-07-09       戴辉发     创建
==============================================================*/
static uint8_t send_data_to_can_recv_buff(CanRxMsg *rx_msg)
{
	memcpy(&g_recv_buf[g_recv_tail], rx_msg, sizeof(CanRxMsg));
	g_recv_tail += 1;
	if (g_recv_tail >= MAX_CAN_RECV_NUM) g_recv_tail = 0;

	return sizeof(CanRxMsg);
}

/*=============================================================
 * 函数名称：get_data_from_can_recv_buff
 * 函数功能：从接收缓冲中读取一包数据
 * 参数个数：1
 * 函数参数：
 *           [IN/OUT]   Rx_msg     数据缓冲
 * 返 回 值：
 *           0                     表示没有取到数据
 *           sizeof(CanRxMsg)      获取到一包数据
 * 修改记录:
 *===============================================================
 * 日期                 修改人     修改内容
 * 2018-07-09           戴辉发     创建
==============================================================*/
uint8_t get_data_from_can_recv_buff(CanRxMsg *rx_msg)
{
	if (g_recv_head != g_recv_tail)
	{
		memcpy(rx_msg, &g_recv_buf[g_recv_head], sizeof(CanRxMsg));

		g_recv_head += 1;
		if (g_recv_head >= MAX_CAN_RECV_NUM) g_recv_head = 0;

		return sizeof(CanRxMsg);
	}
	return 0;
}

/* Private function prototypes -----------------------------------------------*/

/**
  * @file   CAN_Configuration
  * @brief  Configures the different GPIO ports.
  * @param  无
  * @retval 无
  */
void GPIO_Configuration(void)
{
    FL_GPIO_InitTypeDef      GPIO_InitStruct = {0};

    /*-----------------------------------GPIO初始化---------------------------------------*/
    if( get_main_power_detect_status() == 1 )
    {
        GPIO_InitStruct.pin = FL_GPIO_PIN_7|FL_GPIO_PIN_6;
        GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
        GPIO_InitStruct.pull = FL_ENABLE;
        GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.remapPin = FL_DISABLE;
        FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else
    {
        FL_GPIO_DeInit(GPIOA,FL_GPIO_PIN_6);
        FL_GPIO_DeInit(GPIOA,FL_GPIO_PIN_7);
    }
}

/*=============================================================
 * 函数名称：set_can_std_filter
 * 函数功能：CAN滤波器标准标识符屏蔽模式设置
 * 参数个数：2
 * 函数参数：
 *           [IN]   id_buff    标识符列表
 *           [IN]   id_num     标识符列表中标识符个数
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2020-09-29       戴辉发     创建
==============================================================*/
void set_can_std_filter(const uint16_t *id_buff, uint32_t id_num)
{
	if (id_num > 0)
	/* 设置标准帧滤器 */
	{
        FL_CAN_FilterInitTypeDef CAN_FilterInitStructure = {0};
		uint16_t mask, mask1, i;
		uint32_t id;

		/* 计算子网掩码 */
		mask = id_buff[0];
		mask1 = ~id_buff[0];
		id = id_buff[0];
		for (i = 1; i < sizeof(std_buf) / sizeof(std_buf[0]); i ++)
		{
			mask &= id_buff[i];
			mask1 &= (~id_buff[i]);
			id |= id_buff[i];
		}
		mask |= mask1;

        /*----------------------------接收滤波结构体初始化--------------------------------------*/
        CAN_FilterInitStructure.filterIdStandard = id; //标准ID
        CAN_FilterInitStructure.filterIdSRR = 0;  //数据帧
        CAN_FilterInitStructure.filterIdIDE = 0;  //标准帧
        CAN_FilterInitStructure.filterIdRTR = 0;  //扩展帧指示数据帧

        CAN_FilterInitStructure.filterMaskIdHigh = mask;
        CAN_FilterInitStructure.filterMaskIdSRR = 0x01;
        CAN_FilterInitStructure.filterMaskIdIDE = 0x01;  //滤波器掩码,1:该位参与滤波器比较，0：不参与
        CAN_FilterInitStructure.filterMaskIdRTR = 0x01;
        CAN_FilterInitStructure.filterEn = FL_ENABLE;
        FL_CAN_FilterInit(CAN, &CAN_FilterInitStructure, FL_CAN_FILTER1);
	}
}

/*=============================================================
 * 函数名称：set_can_ext_filter
 * 函数功能：CAN滤波器扩展标识符屏蔽模式设置
 * 参数个数：2
 * 函数参数：
 *           [IN]   id_buff    标识符列表
 *           [IN]   id_num     标识符列表中标识符个数
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2020-09-29       戴辉发     创建
==============================================================*/
void set_can_ext_filter(const uint32_t *id_buff, uint32_t id_num)
{
	if (id_num > 0)
	{
        FL_CAN_FilterInitTypeDef CAN_FilterInitStructure = {0};
		uint32_t mask, mask1, i;
		uint32_t id;

		/* 计算子网掩码 */
		mask = id_buff[0];
		mask1 = ~id_buff[0];
		id = id_buff[0];
		for (i = 1; i < id_num; i ++)
		{
			mask &= id_buff[i];
			mask1 &= (~id_buff[i]);
			id |= id_buff[i];
		}
		mask |= mask1;

		/*----------------------------接收滤波结构体初始化--------------------------------------*/
        CAN_FilterInitStructure.filterIdExtend = id; //扩展ID
        CAN_FilterInitStructure.filterIdSRR = 1;     //数据帧
        CAN_FilterInitStructure.filterIdIDE = 1;     //扩展帧
        CAN_FilterInitStructure.filterIdRTR = 0;     //扩展帧指示数据帧

        CAN_FilterInitStructure.filterMaskIdHigh = (mask >> 18) & 0x7FF;
        CAN_FilterInitStructure.filterMaskIdLow = mask & 0x3FFFF;
        CAN_FilterInitStructure.filterMaskIdSRR = 0x01;
        CAN_FilterInitStructure.filterMaskIdIDE = 0x01;//滤波器掩码,1:该位参与滤波器比较，0：不参与
        CAN_FilterInitStructure.filterMaskIdRTR = 0x01;
        CAN_FilterInitStructure.filterEn = FL_ENABLE;
        FL_CAN_FilterInit(CAN, &CAN_FilterInitStructure, FL_CAN_FILTER2);
	}
}

/**
  * @file   CAN_Configuration
  * @brief  Configures the CAN
  * @param  无
  * @retval 无
  */
void CAN_Configuration(e_can_baudrate baudrate)
{
    FL_CAN_InitTypeDef       CAN_InitStructure = {0};
   
    
    if (E_CAN_BAUDRATE_MAX_NUM > baudrate)
    {
		GPIO_Configuration();    
        
        /*----------------------------CAN结构体初始化--------------------------------------*/
        //波特率设置= CAN_CLK/(BRP+1)/(TS1_Tq+TS2_Tq+1);  
        CAN_InitStructure.TS1 = g_can_baudrate_config[baudrate].bs1;
        CAN_InitStructure.TS2 = g_can_baudrate_config[baudrate].bs2;       //位时序设置
        CAN_InitStructure.SJW = g_can_baudrate_config[baudrate].sjw;
        CAN_InitStructure.BRP = g_can_baudrate_config[baudrate].prescaler;   //波特率预分频
        CAN_InitStructure.mode = FL_CAN_MODE_NORMAL;  //工作模式设置
        CAN_InitStructure.clockSource = FL_CMU_CAN_CLK_SOURCE_XTHF; //时钟源设置
        FL_CAN_Init(CAN, &CAN_InitStructure);
        
        set_can_std_filter( std_buf,sizeof(std_buf)/sizeof(std_buf[0]) );
        set_can_ext_filter( ext_buf,sizeof(ext_buf)/sizeof(ext_buf[0]) );
         /* configure CAN0 NVIC */
        
        FL_CAN_ClearFlag_CRXOK(CAN);
        FL_CAN_EnableIT_RXOK(CAN);      //接收中断使能
        
        WRITE_REG(CAN->ICR, CAN_ICR_CRXOFLW_Msk);
        SET_BIT(CAN->IER, CAN_IER_RXOFLWIE_Msk);
        //FL_CAN_ClearFlag_CRXOK(CAN);
        //FL_CAN_EnableIT_RXOK(CAN);      //接收中断使能

        NVIC_DisableIRQ(CAN_IRQn);
        NVIC_SetPriority(CAN_IRQn, 2); //中断优先级配置
        NVIC_EnableIRQ(CAN_IRQn);    
    }
}

/*=============================================================
 * 函数名称：can_sleep
 * 函数功能：CAN 进入睡眠状态
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2017-10-06       戴辉发     创建
==============================================================*/
static void can_sleep(void)
{
    FL_CAN_Disable(CAN);
    FL_CAN_SetSoftwareReset(CAN,FL_CAN_SOFTWARE_RESET);
}

/*=============================================================
 * 函数名称：can_close
 * 函数功能：can硬件指定端口关闭
 * 参数个数：1
 * 函数参数：
 *           [IN]   handle     CAN句柄
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2017-11-16       戴辉发     创建
==============================================================*/
void can_close(void)
{
	can_sleep();
}

/*=============================================================
 * 函数名称：can_ext_transmit
 * 函数功能：CAN发送带扩展标识数据
 * 参数个数：3
 * 函数参数：
 *           [IN]   id         标识符     
 *           [IN]   data_buf   发送数据缓冲     
 *           [IN]   len        数据长度，字节为单位     
 * 返 回 值：
 *           发送有效数据长度
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2017-10-06       戴辉发     创建
==============================================================*/
uint8_t can_ext_transmit(unsigned int id, uint8_t *data_buf, uint8_t len)
{
	uint8_t i;
	CanTxMsg transmit_message;
    
	transmit_message.txcanId.canid_bits.StdId = (id & 0x1FFFFFFF)>>18;
    transmit_message.txcanId.canid_bits.extId = (id & 0x0003FFFF);
    transmit_message.txcanId.canid_bits.IDE = 1;
    transmit_message.txcanId.canid_bits.RTR = 0;
    transmit_message.txcanId.canid_bits.SRR = 1;
    
    if (len > 8)
	{
		len = 8;
	}
    transmit_message.DLC = len;
	for (i = 0; i < len; i ++)
	{
		transmit_message.Data[i] = data_buf[i];
	}
	for (; i < 8; i ++)
	{
		transmit_message.Data[i] = 0;
	}

	return send_data_to_can_send_buff(&transmit_message);
}

/*=============================================================
 * 函数名称：can_std_transmit
 * 函数功能：CAN发送带扩展标识数据
 * 参数个数：4
 * 函数参数：
 *           [IN]   id         标识符     
 *           [IN]   data_buf   发送数据缓冲     
 *           [IN]   rtr        RTR标志
 *           [IN]   len        数据长度，字节为单位     
 * 返 回 值：
 *           发送有效数据长度
 * 修改记录:
 *=============================================================
 * 日期             修改人     修改内容
 * 2017-10-06       戴辉发     创建
==============================================================*/
uint8_t can_std_transmit(unsigned int id, uint8_t *data_buf, uint8_t rtr, uint8_t len)
{
	uint8_t i;
	CanTxMsg transmit_message;
    
    transmit_message.txcanId.canid_bits.StdId = (id & 0x00007FF);
    transmit_message.txcanId.canid_bits.extId = 0x0;
    transmit_message.txcanId.canid_bits.IDE = 0;
    transmit_message.txcanId.canid_bits.RTR = 0;
    transmit_message.txcanId.canid_bits.SRR = 0;
    
    if (len > 8)
	{
		len = 8;
	}
    transmit_message.DLC = len;
	for (i = 0; i < len; i ++)
	{
		transmit_message.Data[i] = data_buf[i];
	}
	for (; i < 8; i ++)
	{
		transmit_message.Data[i] = 0;
	}

	return send_data_to_can_send_buff(&transmit_message);
}

/*=============================================================
 * 函数名称：can_rx_irq_process
 * 函数功能：CAN接收数据中断处理
 * 参数个数：1
 * 函数参数：
 *           [IN]   can        CAN外设
 *           [IN]   fifo       对应外设
 * 返 回 值：
 *           无
 * 修改记录:
 *=============================================================
 * 日期             修改人     修改内容
 * 2017-10-06       戴辉发     创建
==============================================================*/
static void can_rx_irq_process(void)
{
    CanRxMsg canrx;
    uint32_t data1 = 0;
    uint32_t data2 = 0;

    canrx.rxcanId.canId = FL_CAN_ReadRXMessageID(CAN);
    canrx.DLC = FL_CAN_ReadRXMessageLength(CAN);
    data1 = FL_CAN_ReadRXMessageWord1(CAN);
    data2 = FL_CAN_ReadRXMessageWord2(CAN);
    canrx.Data[0] = (uint8_t)data1 & 0xff;
    canrx.Data[1] = (uint8_t)(data1 >> 8) & 0xff;
    canrx.Data[2] = (uint8_t)(data1 >> 16) & 0xff;
    canrx.Data[3] = (uint8_t)(data1 >> 24) & 0xff;
    canrx.Data[4] = (uint8_t)data2 & 0xff;
    canrx.Data[5] = (uint8_t)(data2 >> 8) & 0xff;
    canrx.Data[6] = (uint8_t)(data2 >> 16) & 0xff;
    canrx.Data[7] = (uint8_t)(data2 >> 24) & 0xff;
    send_data_to_can_recv_buff(&canrx);
}

/*=============================================================
 * 函数名称：CAN0_RX0_IRQHandler
 * 函数功能：CAN中断服务函数
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *=============================================================
 * 日期             修改人     修改内容
 * 2020-09-08       liyong     创建
==============================================================*/
void CAN_IRQHandler(void)            //中断接收
{
    if ((FL_ENABLE == FL_CAN_IsEnabledIT_RXOK(CAN)) && (FL_SET == FL_CAN_IsActiveFlag_RXOK(CAN)))
    {
        can_rx_irq_process();
        FL_CAN_ClearFlag_CRXOK(CAN);
    }
    
    if ((FL_ENABLE == FL_CAN_IsEnabledIT_RXOverflow(CAN)) && (FL_SET == FL_CAN_IsActiveFlag_RXOverflow(CAN)))
    {
        can_rx_irq_process();
        WRITE_REG(CAN->ICR, CAN_ICR_CRXOFLW_Msk);
        //FL_CAN_ClearFlag_CRXOK(CAN);
    }
}

/*=============================================================
 * 函数名称：FL_CAN_FIFO_Write
 * 函数功能：使用普通发送邮箱
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2021-06-02       liyong     创建
==============================================================*/
void FL_CAN_FIFO_Write( CanTxMsg * TxMessage )
{
    uint32_t data1,data2;
    static uint16_t full_count;

    if((FL_CAN_IsActiveFlag_TXBuffFull(CAN) == FL_RESET))
    {
        FL_CAN_WriteTXMessageID(CAN, TxMessage->txcanId.canId);
        FL_CAN_WriteTXMessageLength(CAN, TxMessage->DLC);
        data1 = (((uint32_t)TxMessage->Data[3] << 24) |
        ((uint32_t)TxMessage->Data[2] << 16) |
        ((uint32_t)TxMessage->Data[1] << 8) |
        ((uint32_t)TxMessage->Data[0]));
        data2 = (((uint32_t)TxMessage->Data[7] << 24) |
        ((uint32_t)TxMessage->Data[6] << 16) |
        ((uint32_t)TxMessage->Data[5] << 8) |
        ((uint32_t)TxMessage->Data[4]));
        FL_CAN_WriteTXMessageWord1(CAN, data1);
        FL_CAN_WriteTXMessageWord2(CAN, data2);
        remove_data_from_can_send_buff();

        protect_code[6] &= ~0x02;
        full_count = 0;
    }
    else 
    {
        full_count++;
        if(full_count > 0xFF00)
        {
            protect_code[6] |= 0x02;
        }
    }
}

/*=============================================================
 * 函数名称：FL_CAN_HPBUF_Write
 * 函数功能：使用高优先级发送邮箱
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2021-06-02       liyong     创建
==============================================================*/
void FL_CAN_HPBUF_Write( CanTxMsg * TxMessage )
{
    uint32_t data1, data2;

    if((FL_CAN_IsActiveFlag_TXHighPriorBuffFull(CAN) == FL_RESET))
    {
        FL_CAN_WriteHighPriorTXMessageID(CAN, TxMessage->txcanId.canId);
        FL_CAN_WriteHighPriorMessageLength(CAN, TxMessage->DLC);
        data1 = (((uint32_t)TxMessage->Data[3] << 24) |
            ((uint32_t)TxMessage->Data[2] << 16) |
            ((uint32_t)TxMessage->Data[1] << 8) |
            ((uint32_t)TxMessage->Data[0]));
            data2 = (((uint32_t)TxMessage->Data[7] << 24) |
            ((uint32_t)TxMessage->Data[6] << 16) |
            ((uint32_t)TxMessage->Data[5] << 8) |
            ((uint32_t)TxMessage->Data[4]));
        FL_CAN_WriteHighPriorMessageWord1(CAN, data1);
        FL_CAN_WriteHighPriorMessageWord2(CAN, data2);
        remove_data_from_can_send_buff();
    }
}

/*=============================================================
 * 函数名称：CAN_TX_Process
 * 函数功能：CAN发送数据流程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *==============================================================
 * 日期             修改人     修改内容
 * 2017-11-13       戴辉发     创建
==============================================================*/
void CAN_TX_Process(void)
{
    /*  定义CAN报文结构体           */
    CanTxMsg TxMessage;
    
   if (get_data_from_can_send_buff(&TxMessage) > 0)
   {       
       FL_CAN_FIFO_Write(&TxMessage);
    }
}
/*********************************************************************************************************
**                                        End Of File
*********************************************************************************************************/
