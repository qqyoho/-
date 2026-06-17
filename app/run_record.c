#include "run_record.h"
#include "string.h"
#include "temp_manage.h"
#include "storage_manage.h"
#include "vol_manage.h"
#include "parameter.h"
#include "idog.h"
#include "can_app.h"
#include "balance.h"
#include "mode_manage.h"

#define MIN_RECORD_STORAGE_NUM                     (0x0001)
#define MAX_RECORD_STORAGE_NUM                     (0x8000)

#define RECORD_START_NUM                           (0x50)  /* 第80个扇区开始 */

#define MAX_STATUS_RECORD_WRITE_NUM                (MAX_SECTOR_NUM - RECORD_START_NUM)  /* 保存记录最多个数 */

/* 运行状态记录 */
/* 寻找运行记录最后一次写的位置 */
typedef struct _FIND_ADDRESS
{
	uint32_t address;   /* 下一次运行记录的要写的地址 */ 
	uint8_t number;     /* 为了便于下次上电查询存储位置设置 */ 
    uint8_t next_store; /* 地址下一次写之后需要存储到 0:存储到主区， 1：存储到备份区 */ 
}t_find_run_adrress;

t_run_record_data g_read_run_record; /* 前次系统运行参数 */
t_run_record_data g_read_protect; /* 前次系统保护参数 */

t_run_record_data run_record_write; /* 本次系统运行参数 */
t_run_record_data protect_record_write; /* 本次系统保护参数 */

t_find_run_adrress now_run_address;
t_find_run_adrress now_protect_address;
uint8_t write_rundata_flag;
uint16_t write_rundata_delay; 
uint32_t read_rundata_adr;       /* 记录读取运行记录的地址 */ 
uint32_t read_protectdata_adr;   /* 记录读取保护记录的地址 */
uint8_t record_protect;
/*=============================================================
* 函数名称：run_recorde_mem_init
* 函数功能：运行记录管理内存初始化
* 参数个数：
* 函数参数：
* 返 回 值：
*          无
* 修改记录：
*===============================================================
* 日    期         修改人     修改内容
* 2018-06-22       戴辉发     创建
==============================================================*/
void run_recorde_mem_init(void)
{
    read_rundata_adr = RUNNING_RECORD_STORE_START;
    now_run_address.address = RUNNING_RECORD_STORE_START;
    read_protectdata_adr = PROTECT_RECORD_STORE_START;
    now_protect_address.address = PROTECT_RECORD_STORE_START;
    record_protect = 0;
}

/*=============================================================
* 函数名称：get_protect_record_write_address
* 函数功能：开机寻找运行记录的存储地址
* 参数个数：0
* 函数参数：
* 返 回 值：
* 修改记录：
*===============================================================
* 日期                修改人        修改内容
* 2020-06-2           李勇          创建
==============================================================*/
void get_protect_record_write_address(void)
{
    int16_t flag = 0;
	t_find_run_adrress serial;
    
	flag = read_data_from_storage(PROTECT_RECORD_ADR_MAIN, (uint8_t *)&now_protect_address, sizeof(t_find_run_adrress));
    if( flag == sizeof(t_find_run_adrress) )
    {
       flag = read_data_from_storage(PROTECT_RECORD_ADR_BACK, (uint8_t *)&serial, sizeof(t_find_run_adrress));
       if( flag == sizeof(t_find_run_adrress) )
       {
          flag = judge_new_sector(now_protect_address.number, serial.number);
       } 
    }
	
    
	if ( flag == -1 )
	{
		memcpy(&now_protect_address, &serial, sizeof(t_find_run_adrress));
        now_protect_address.next_store = 1;
	}
	else if ( flag == 0 )
	{
        now_protect_address.next_store = 1;
        now_protect_address.address = PROTECT_RECORD_STORE_START;
        now_protect_address.number = MAX_STORAGE_NUM;
	}
	else
	{
        now_protect_address.next_store = 0;   
	}
}

/*=============================================================
* 函数名称：write_protect_record_write_address
* 函数功能：写产品系列号到EEPROM
* 参数个数：0
* 参数描述：
* 返 回 值：无
*           
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2018-10-17       	 戴辉发       创建
==============================================================*/
void write_protect_record_write_address(void)
{
	t_write_data_st data;

	now_protect_address.number = sector_num_next(now_protect_address.number);
	now_protect_address.address++;
    if( now_protect_address.address > PROTECT_RECORD_STORE_END )
      now_protect_address.address = PROTECT_RECORD_STORE_START;
    
	if (now_protect_address.next_store)
    {
        now_protect_address.next_store = 0;
        data.write_storge_address = PROTECT_RECORD_ADR_MAIN;
    }
	else
    {
        now_protect_address.next_store = 1;
        data.write_storge_address = PROTECT_RECORD_ADR_BACK;
    }

	data.msg_type = E_PROTECT_RECORD_MSG;
	data.write_len = sizeof(t_find_run_adrress);
	data.write_memery_address = (uint8_t *)&now_protect_address;
	set_write_data_to_queue(&data);
}

/*=============================================================
* 函数名称：get_run_record_write_address
* 函数功能：开机寻找运行记录的存储地址
* 参数个数：0
* 函数参数：
* 返 回 值：
* 修改记录：
*===============================================================
* 日期                修改人        修改内容
* 2020-06-2           李勇          创建
==============================================================*/
void get_run_record_write_address(void)
{
    int16_t flag = 0;
    t_find_run_adrress serial;

    flag = read_data_from_storage(RUNNING_RECORD_ADR_MAIN, (uint8_t *)&now_run_address, sizeof(t_find_run_adrress));
    if(flag == sizeof(t_find_run_adrress))
    {
        flag = read_data_from_storage(RUNNING_RECORD_ADR_BACK, (uint8_t *)&serial, sizeof(t_find_run_adrress));
        if(flag == sizeof(t_find_run_adrress))
        {
            flag = judge_new_sector(now_run_address.number, serial.number);
        }
    }

    if (flag == -1)
    {
        memcpy(&now_run_address, &serial, sizeof(t_find_run_adrress));
        now_run_address.next_store = 1;
    }
    else if (flag == 0)
    {
        now_run_address.next_store = 1;
        now_run_address.address = RUNNING_RECORD_STORE_START;
        now_run_address.number = MAX_STORAGE_NUM;
    }
    else
    {
        now_run_address.next_store = 0;
    }
}

/*=============================================================
* 函数名称：write_serial_no_to_eeprom
* 函数功能：写产品系列号到EEPROM
* 参数个数：0
* 参数描述：
* 返 回 值：无
*           
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2018-10-17       	 戴辉发       创建
==============================================================*/
void write_run_record_write_address(void)
{
	t_write_data_st data;
	now_run_address.number = sector_num_next(now_run_address.number);
	now_run_address.address++;
    if( now_run_address.address > RUNNING_RECORD_STORE_END )
      now_run_address.address = RUNNING_RECORD_STORE_START;
    
	if (now_run_address.next_store)
    {
        now_run_address.next_store = 0;
        data.write_storge_address = RUNNING_RECORD_ADR_MAIN ;
    }
	else
    {
        now_run_address.next_store = 1;
        data.write_storge_address = RUNNING_RECORD_ADR_BACK ;
    }

	data.msg_type = E_RUN_RECORD_MSG;
	data.write_len = sizeof(t_find_run_adrress);
	data.write_memery_address = (uint8_t *)&now_run_address;
	set_write_data_to_queue(&data);
}

/*=============================================================
* 函数名称：run_record_hard_init
* 函数功能：运行记录管理硬件初始化
* 参数个数：
* 函数参数：
* 返 回 值：
*           无
* 修改记录：
*===============================================================
* 日期             修改人     修改内容
* 2018-06-22       戴辉发     创建
==============================================================*/
void run_record_hard_init(void)
{
    get_run_record_write_address();
    get_protect_record_write_address();
}
/*=============================================================
* 函数名称：write_run_record
* 函数功能：写入系统保护状态
* 参数个数：0
* 函数参数：
* 返 回 值：
* 修改记录：
*===============================================================
* 日期                修改人        修改内容
* 2020-06-2           李勇          创建
==============================================================*/
void write_protect_record(void)
{
	t_write_data_st data;
    /* 形成当前事件 */
    event_deal_process();
	/* 保存当前数据 */
    memcpy(&protect_record_write,&g_run_sys_data,sizeof(t_run_record_data));
	/* 获取当前写记录号 */

	data.msg_type = E_PROTECT_RECORD_MSG;
	data.write_len = sizeof(t_run_record_data);
	data.write_memery_address = (uint8_t *)&protect_record_write;
	data.write_storge_address = now_protect_address.address;
	set_write_data_to_queue(&data);
    write_protect_record_write_address();
}

/*=============================================================
* 函数名称：write_run_record_delay
* 函数功能：
* 参数个数：0
* 函数参数：
* 返 回 值：
*           
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
*2020/11/25          李勇      修改
==============================================================*/
void write_run_record_delay(void)
{
  if( write_rundata_delay ) write_rundata_delay--;
}
/*=============================================================
* 函数名称：write_run_record
* 函数功能：写入当前系统运行状态
* 参数个数：0
* 函数参数：
* 返 回 值：
* 修改记录：
*===============================================================
* 日期                修改人        修改内容
* 2018-06-22          戴辉发        创建
==============================================================*/
void write_run_record(void)
{
	t_write_data_st data;
    /* 形成当前事件 */
    event_deal_process();
	memcpy(&run_record_write,&g_run_sys_data,sizeof(t_run_record_data));
	/* 获取当前写记录号 */
	data.msg_type = E_RUN_RECORD_MSG;
	data.write_len = sizeof(t_run_record_data);
	data.write_memery_address = (uint8_t *)&run_record_write;
	data.write_storge_address = now_run_address.address;

	set_write_data_to_queue(&data);
    write_run_record_write_address();
}

/*=============================================================
* 函数名称：read_first_run_record
* 函数功能：读取第一条系统运行记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           读取到的数据长度，为0读取不成功
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
* 2018-06-22         戴辉发     创建
*2020/6/2             李勇      修改
==============================================================*/
uint8_t read_first_run_record(void)
{
	uint8_t ret = 0;
	t_run_record_data temp_record;   
    read_rundata_adr = now_run_address.address;
    if(( read_rundata_adr > RUNNING_RECORD_STORE_END )||( read_rundata_adr < RUNNING_RECORD_STORE_START ))
      read_rundata_adr = RUNNING_RECORD_STORE_START;
    else
    {
        read_rundata_adr--;
        if(( read_rundata_adr > RUNNING_RECORD_STORE_END )||( read_rundata_adr < RUNNING_RECORD_STORE_START ))
         read_rundata_adr = RUNNING_RECORD_STORE_END;
    }
    ret = read_data_from_storage(read_rundata_adr, (uint8_t *)&temp_record, sizeof(temp_record));
   
    if ((temp_record.total_vol == 0xffff)&&(temp_record.soc == 0xffff) && (temp_record.write_time[1] == 0xff)&& (temp_record.write_time[0] == 0xff))
    {/* 从未写过 */
        ret = 0;
    }
	
	if ( ret == sizeof(temp_record) )
	{
		memcpy(&g_read_run_record, &temp_record, sizeof(g_read_run_record));
        g_read_run_record.number = read_rundata_adr;
	}
	return ret;
}

/*=============================================================
* 函数名称：read_first_record
* 函数功能：读取下一条系统运行记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           读取到的数据长度，为0读取不成功
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
* 2018-06-22         戴辉发     创建
==============================================================*/
uint8_t read_next_run_record(void)
{
	uint8_t ret = 0;
	t_run_record_data temp_record;

     read_rundata_adr--;
     if(( read_rundata_adr > RUNNING_RECORD_STORE_END )||( read_rundata_adr < RUNNING_RECORD_STORE_START ))
        read_rundata_adr = RUNNING_RECORD_STORE_END;
    
	if (  read_rundata_adr != now_run_address.address )
	{
		ret = read_data_from_storage( read_rundata_adr, (uint8_t *)&temp_record, sizeof(temp_record));     
        if ((temp_record.total_vol == 0xffff)&&(temp_record.soc == 0xffff) && (temp_record.write_time[1] == 0xff)&& (temp_record.write_time[0] == 0xff))
   	    {/* 从未写过 */
			ret = 0;
		}
	}
    
	if ( ret == sizeof(temp_record) )
	{
		memcpy(&g_read_run_record, &temp_record, sizeof(g_read_run_record));
        g_read_run_record.number = read_rundata_adr;
	}

	return ret;
}


/*=============================================================
* 函数名称：read_first_protect_record
* 函数功能：读取第一条系统保护记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           读取到的数据长度，为0读取不成功
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
*2020/6/4             李勇      修改
==============================================================*/
uint8_t read_first_protect_record(void)
{
	uint8_t ret = 0;
	t_run_record_data temp_record;   
   
    read_protectdata_adr = now_protect_address.address;  
    
    if(( read_protectdata_adr > PROTECT_RECORD_STORE_END )||( read_protectdata_adr < PROTECT_RECORD_STORE_START ))
      read_protectdata_adr = PROTECT_RECORD_STORE_START;
    else
    {
        read_protectdata_adr--;
        if(( read_protectdata_adr > PROTECT_RECORD_STORE_END )||( read_protectdata_adr < PROTECT_RECORD_STORE_START ))
         read_protectdata_adr = PROTECT_RECORD_STORE_END;
    }
    
    ret = read_data_from_storage(read_protectdata_adr, (uint8_t *)&temp_record, sizeof(temp_record));
   
    if ((temp_record.total_vol == 0xffff)&&(temp_record.soc == 0xffff) && (temp_record.write_time[1] == 0xff)&& (temp_record.write_time[0] == 0xff))
    {/* 从未写过 */
        ret = 0;
    }
	
	if ( ret == sizeof(temp_record) )
	{
		memcpy(&g_read_protect, &temp_record, sizeof(g_read_protect));
        g_read_protect.number = read_protectdata_adr;
	}
	return ret;
}

/*=============================================================
* 函数名称：read_next_protect_record
* 函数功能：读取下一条系统保护记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           读取到的数据长度，为0读取不成功
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
*2020/6/4             李勇      修改
==============================================================*/
uint8_t read_next_protect_record(void)
{
	uint8_t ret = 0;
	t_run_record_data temp_record;

     read_protectdata_adr--;
     if(( read_protectdata_adr > PROTECT_RECORD_STORE_END )||( read_protectdata_adr < PROTECT_RECORD_STORE_START ))
        read_protectdata_adr = PROTECT_RECORD_STORE_END;
    
	if (  read_protectdata_adr != now_protect_address.address )
	{
		ret = read_data_from_storage( read_protectdata_adr, (uint8_t *)&temp_record, sizeof(temp_record));     
        if ((temp_record.total_vol == 0xffff) &&(temp_record.soc == 0xffff) && (temp_record.write_time[1] == 0xff)&& (temp_record.write_time[0] == 0xff))
 		{/* 从未写过 */
			ret = 0;
		}
	}
    
	if ( ret == sizeof(temp_record) )
	{
		memcpy(&g_read_protect, &temp_record, sizeof(g_read_protect));
        g_read_protect.number = read_protectdata_adr;
	}

	return ret;
}

/*=============================================================
* 函数名称：protect_record_clear
* 函数功能：清除所有系统保护记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
*2020/6/18             李勇      修改
==============================================================*/
void protect_record_clear(void)
{
    /*将eeprom的数据清0*/
    read_protectdata_adr = PROTECT_RECORD_STORE_START;
    now_protect_address.address = PROTECT_RECORD_STORE_START;
    record_protect = 0;
    write_protect_record_write_address();
}

/*=============================================================
* 函数名称：protect_record_clear
* 函数功能：清除所有系统保护记录
* 参数个数：0
* 函数参数：
* 返 回 值：
*           
* 修改记录：
*===============================================================
* 日期               修改人     修改内容
*2020/6/18             李勇      修改
==============================================================*/
void run_record_clear(void)
{
    /*将eeprom的数据清0*/
    read_rundata_adr = RUNNING_RECORD_STORE_START;
    now_run_address.address = RUNNING_RECORD_STORE_START;
    write_run_record_write_address();
}