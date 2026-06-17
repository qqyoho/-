#include "storage_manage.h"
#include "string.h"
#include "eeprom.h"
#include "peak_record.h"
#include "protect_record.h"
#include "soc_update.h"
#include "balance.h"
#include "run_record.h"

#define MAX_WRITE_MSG_NUM                          (24)

#define MAX_GT3_ADDRESS                            (0x4000)

/* 状态类型 */
typedef enum _E_WRITE_STATUS_TYPE_
{
	E_IDLE_STATUS, /* 空闲状态 */
	E_WAIT_WRITE_FINSHED_TYPE, /* 等待本次写完成状态 */
	E_WAIT_I2C_STANDBY_TYPE, /* 等待I2C进入就绪状态 */
	E_WRITE_MSG_NUMBER
}e_write_status_type;

static uint8_t g_write_head; /* 写起始索引 */
static uint8_t g_write_tail; /* 写结束索引 */
static t_write_data_st g_msg_type[MAX_WRITE_MSG_NUM]; /* 保存数据消息队列 */

static uint8_t get_write_data_from_queue(t_write_data_st *write_data);
uint32_t needstoreflag; /* 是否有数据需要存储 */
uint16_t timestore30min; /* 30min计数 */

t_protect1_numb protect1_numb;
t_protect2_numb protect2_numb;
t_fault1_numb_record fault1_numb_record;
t_fault2_numb_record fault2_numb_record;
t_update_record last_two_update;
t_blance_time cell_blance_numb;

/*=============================================================
* 函数名称：storage_manage_mem_init
* 函数功能：存储管理内存初始化
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2018-07-05       戴辉发     创建
==============================================================*/
void storage_manage_mem_init(void)
{
	g_write_head = 0;
	g_write_tail = 0;
    needstoreflag = 0;
    timestore30min = 1800;
}

/*=============================================================
* 函数名称：storage_manage_hard_init
* 函数功能：存储管理硬件初始化
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2018-06-22       戴辉发     创建
==============================================================*/
void storage_manage_hard_init(void)
{
}

/*=============================================================
* 函数名称：time_s_storage
* 函数功能：存储管理定时器
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2020-06-03       李勇       创建
==============================================================*/
void time_s_storage(void)
{
    if( timestore30min ) timestore30min--;
}

/*=============================================================
* 函数名称：querydatastore
* 函数功能：是否需要存储数据，半小时存储一次管理
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录:
*===============================================================
* 日期             修改人     修改内容
* 2020-06-03       李勇       创建

30min = 1800s 每100ms一段
==============================================================*/
void querydatastore(void)
{
    /* 故障2存储 */
    if(timestore30min > 1700)
    {
        if((needstoreflag & E_FAULT2_RECORD_MSG))
        {
            needstoreflag &= ~E_FAULT2_RECORD_MSG;
            recordfault2_data();
            timestore30min = 1699;
        }
    }
    /* 历史累计极限值1 */
    else if(timestore30min > 1600)
    {
        if((needstoreflag & E_PEAK1_RECORD_MSG))
        {
            needstoreflag &= ~E_PEAK1_RECORD_MSG;
            save_peak1_record();
            timestore30min = 1599;
        }
    }
    /* 历史累计极限值2 */
    else if(timestore30min > 1500)
    {
        if((needstoreflag & E_PEAK2_RECORD_MSG))
        {
            needstoreflag &= ~E_PEAK2_RECORD_MSG;
            save_peak2_record();
            timestore30min = 1499;
        }
    }
    /* 累计保护1次数 */
    else if(timestore30min > 1400)
    {
        if((needstoreflag & E_PROTECT1_RECORD_MSG))
        {
            needstoreflag &= ~E_PROTECT1_RECORD_MSG;
            recordprotect1_data();
            timestore30min = 1399;
        }
    }
    /* 累计保护2次数 */
    else if(timestore30min > 1300)
    {
        if((needstoreflag & E_PROTECT2_RECORD_MSG))
        {
            needstoreflag &= ~E_PROTECT2_RECORD_MSG;
            recordprotect2_data();
            timestore30min = 1299;
        }
    }
    /* 故障1存储 */
    else if(timestore30min > 1200)
    {
        if((needstoreflag & E_FAULT1_RECORD_MSG))
        {
            needstoreflag &= ~E_FAULT1_RECORD_MSG;
            recordfault1_data();
            timestore30min = 1199;
        }
    }
    /* 容量循环相关参数 */
    else if(timestore30min > 1100)
    {
        if((needstoreflag & E_SOC_CIRCLE_PARA_MSG))
        {
            needstoreflag &= ~E_SOC_CIRCLE_PARA_MSG;
            write_capcity_para();
            timestore30min = 1099;
        }
    }
    /* 均衡记录存储 */
    else if(timestore30min > 1000)
    {
        if((needstoreflag & E_BLANCE_RECORD_MSG))
        {
            needstoreflag &= ~E_BLANCE_RECORD_MSG;
            recordbalance_data();
            timestore30min = 999;
        }
    }

    if(timestore30min == 0)
    {
        timestore30min = 1800;
        set_bms_run_time();
        write_rundata_flag = 1;
    }

    if((1 == write_rundata_flag) && (0 == write_rundata_delay))
    {
        write_run_record();
        write_rundata_flag = 0;
    }
}

/*=============================================================
 * 函数名称：get_timdedata
 * 函数功能：获取当前时间戳数据
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           
 * 修改记录：
 *===============================================================
 * 日    期          修改人       修改类型
 * 2019-11-18        戴辉发       创建
==============================================================*/
void get_timdedata( uint8_t *time)
{
    uint16_t year = g_run_sys_data.write_time[0];

    year <<= 8;
    year += g_run_sys_data.write_time[1];
    /* 获取当前时间 */
    time[0] = year - 2000;
    time[1] = g_run_sys_data.write_time[2] ;
    time[2] = g_run_sys_data.write_time[3] ;
    time[3] = g_run_sys_data.write_time[4] ;
    time[4] = g_run_sys_data.write_time[5] ;
    time[5] = g_run_sys_data.write_time[6] ;
}

/*=============================================================
 * 函数名称：get_write_data_from_queue
 * 函数功能：从消息队列中获取数据，数据结构体为t_write_data_st
 * 参数个数：1
 * 函数参数：
 *           [IN/OUT]          write_data
 * 返 回 值：
 *           0                 未获取到数据
 *           sizeof(t_write_data_st)
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-06-22       戴辉发     创建
==============================================================*/
static uint8_t get_write_data_from_queue(t_write_data_st *write_data)
{
	uint8_t ret = 0;
	if (g_write_head != g_write_tail)
	{
		memcpy(write_data, &g_msg_type[g_write_head], sizeof(t_write_data_st));
		ret = sizeof(t_write_data_st);
		g_write_head += 1;
		if (g_write_head >= MAX_WRITE_MSG_NUM) g_write_head = 0;
	}
	return ret;
}

/*=============================================================
 * 函数名称：set_write_data_to_queue
 * 函数功能：将要写入的数据写入消息队列
 * 参数个数：1
 * 函数参数：
 *           [IN/OUT]          write_data
 * 返 回 值：
 *           0                 未获取到数据
 *           sizeof(t_write_data_st)
 * 修改记录:
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-06-22       戴辉发     创建
==============================================================*/
void set_write_data_to_queue(t_write_data_st *write_data)
{
	uint8_t tail = g_write_tail;
	
	tail += 1;
	if (tail >= MAX_WRITE_MSG_NUM) tail = 0;
	if (tail != g_write_head)
	{
		write_data->write_storge_address *= sEE_PAGESIZE;
		memcpy(&g_msg_type[g_write_tail], write_data, sizeof(t_write_data_st));
		g_write_tail = tail;
	}
}

/*=============================================================
 * 函数名称：storage_manage_process
 * 函数功能：存储管理处理流程
 * 参数个数：
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录: 
 *===============================================================
 * 日期             修改人     修改内容
 * 2018-07-05       戴辉发     创建
==============================================================*/
void storage_manage_process(void)
{
	t_write_data_st write_data_msg;

    querydatastore();

    /* 是否需要写故障保护记录 */
    if(record_protect == 1)
    {
        write_protect_record();
        record_protect = 0;
    }

	if (sizeof(t_write_data_st) == get_write_data_from_queue(&write_data_msg))
	/* 写数据队列有数据 */
	{/* 调用写函数 */
		sEE_WriteBuffer(write_data_msg.write_memery_address, 
			write_data_msg.write_storge_address, 
			write_data_msg.write_len);
	}
}

/*=============================================================
 * 函数名称：read_data_from_storage
 * 函数功能：从指定扇区读取指定长度数据并通过参数返回
 * 参数个数：3
 * 函数参数：
 *           [IN]      sector_num    扇区编号
 *           [IN/OUT]  buf           数据缓冲
 *           [IN]      len           读取数据缓冲长度
 * 返 回 值：
 *           实际读取到的数据长度
 * 修改记录: 
 *===============================================================
 * 日期                修改人        修改内容
 * 2018-07-05          戴辉发        创建
==============================================================*/
uint8_t read_data_from_storage(uint16_t sector_num, uint8_t *buf, uint8_t len)
{
	if (0 == sEE_ReadBuffer(buf, sector_num * sEE_PAGESIZE, len))
		return len;

    return 0;
}

/*=============================================================
 * 函数名称：judge_new_sector
 * 函数功能：根据提供的两个扇区序列号判断当前最新数据存储的扇区
 * 参数个数：2
 * 函数参数：
 *           [IN]      value1        扇区序号1
 *           [IN]      value2        扇区序号2
 * 返 回 值：
 *           -1        value2为最新数据扇区
 *           0         value1和value2都为未写过数据扇区
 *           1         value1为最新数据扇区
 * 修改记录: 
 *===============================================================
 * 日期                修改人        修改内容
 * 2018-07-05          戴辉发        创建
==============================================================*/
int16_t judge_new_sector(uint8_t value1, uint8_t value2)
{
	int16_t ret;
	uint8_t flag1 = 0, flag2 = 0;

	if ( value1 > MAX_STORAGE_NUM || value1 < MIN_STORAGE_NUM )
	/* 扇区1未写过 */
	{
		flag1 = 1;
	}
	if ( value2 > MAX_STORAGE_NUM || value2 < MIN_STORAGE_NUM )
	/* 扇区2未写过 */
	{
		flag2 = 1;
	}
	if ( flag1 && flag2 )
	/* 扇区1和扇区2未写过 */
	{
		ret = 0;
	}
	else
	{
		if (flag1)
		/* 扇区1未写过，扇区2写过 */
		{
			ret = -1;
		}
		else if (flag2)
		/* 扇区1写过，扇区2未写过 */
		{
			ret = 1;
		}
		else
		/* 扇区1写过，扇区2写过 */
		{
			if (value1 > value2)
			{
				if (value1 > (value2 + 1))
				{
					ret = -1;
				}
				else
				{
					ret = 1;
				}
			}
			else
			{
				if (value2 > (value1 + 1))
				{
					ret = 1;
				}
				else
				{
					ret = -1;
				}
			}
		}
	}

	return ret;
}

/*=============================================================
 * 函数名称：sector_num_next
 * 函数功能：扇区序列号向前推进一格
 * 参数个数：1
 * 函数参数：
 *           [IN]      num           扇区序号
 * 返 回 值：
 *           向前推进一步后的序列号
 * 修改记录: 
 *===============================================================
 * 日期                修改人        修改内容
 * 2018-07-05          戴辉发        创建
==============================================================*/
uint8_t sector_num_next(uint8_t num)
{
	if (num < MIN_STORAGE_NUM || num >= MAX_STORAGE_NUM)
	{
		num = MIN_STORAGE_NUM;
	}
	else
	{
		num += 1;
		if (num >= MAX_STORAGE_NUM)
		{
			num = MIN_STORAGE_NUM;
		}
	}
	return num;
}
