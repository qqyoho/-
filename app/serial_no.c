#include "serial_no.h"
#include "string.h"
#include "storage_manage.h"

t_serial_number_st g_serial_no;
static uint8_t g_main_flag;
static uint8_t g_written_flag;
HARDWARE_VERSION_NO hardwareVersionNo;
static uint8_t mainFlagHardware;
static uint8_t writtenFlagHardware;
/*=============================================================
* 函数名称：serial_no_mem_init
* 函数功能：产品系列号管理模块内存初始化
* 参数个数：0
* 参数描述：
* 
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期          修改人      修改类型
* 2018-06-22       	戴辉发     	创建
==============================================================*/
void serial_no_mem_init(void)
{
    static char sno[] = "BF24-MNS5_V";
	g_main_flag = 0;
	g_written_flag = MIN_STORAGE_NUM;
	memset(&g_serial_no, 0, sizeof(g_serial_no));

    mainFlagHardware = 0; 
    writtenFlagHardware = MIN_STORAGE_NUM;
    memcpy(hardwareVersionNo.no, &sno, sizeof(sno));
}

/*=============================================================
* 函数名称：serial_no_hard_init
* 函数功能：产品系列号管理模块硬件初始化
* 参数个数：0
* 参数描述：
* 
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期           修改人      修改类型
* 2018-06-22       	 戴辉发     	创建
==============================================================*/
void serial_no_hard_init(void)
{
	int16_t flag = 0;
	t_serial_number_st serial_number;

	flag = read_data_from_storage(MAIN_SECTOR_NUM, (uint8_t *)&g_serial_no, sizeof(g_serial_no));
    if( flag == sizeof(g_serial_no) )
    {
       flag = read_data_from_storage(BACK_SECTOR_NUM, (uint8_t *)&serial_number, sizeof(serial_number));
       if( flag == sizeof(serial_number) )
       {
          flag = judge_new_sector(g_serial_no.written_flag, serial_number.written_flag);
       }   
    }
	

	if ( flag == -1 )
	{
		g_main_flag = 1;
		g_written_flag = serial_number.written_flag;
		memcpy(&g_serial_no, &serial_number, sizeof(serial_number));
	}
	else if ( flag == 0 )
	{
		g_main_flag = 1;
		g_written_flag = MAX_STORAGE_NUM;
		memset(&g_serial_no, 0, sizeof(g_serial_no));
	}
	else
	{
		g_main_flag = 1;
		g_written_flag = g_serial_no.written_flag;
	}
}

/*=============================================================
* 函数名称：get_serial_no
* 函数功能：获取产品系列号
* 参数个数：1
* 参数描述：
*           [IN/OUT] buff         读取序列号内容存放缓冲
* 返 回 值：
*           0        未读取到
*           >0       读取到产品序列号长度
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2018-06-22       	 戴辉发       创建
==============================================================*/
uint8_t get_serial_no(uint8_t *buff)
{
	memcpy(buff, g_serial_no.serial_no, MAX_SERIAL_NO_LENGTH);

	return MAX_SERIAL_NO_LENGTH;
}

/*=============================================================
* 函数名称：set_serial_no
* 函数功能：设置产品系列号
* 参数个数：1
* 参数描述：
*           [IN/OUT] buff         读取序列号内容存放缓冲
* 返 回 值：
*           0        未读取到
*           >0       读取到产品序列号长度
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2018-06-22       	 戴辉发       创建
==============================================================*/
void set_serial_no(uint8_t *buff)
{
	memcpy(g_serial_no.serial_no, buff, MAX_SERIAL_NO_LENGTH);

	write_serial_no_to_eeprom();
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
void write_serial_no_to_eeprom(void)
{
	t_write_data_st data;

	g_written_flag = sector_num_next(g_written_flag);
	g_serial_no.written_flag = g_written_flag;
	if (g_main_flag)
    {
        g_main_flag = 0;
        data.write_storge_address = MAIN_SECTOR_NUM;
    }
	else
    {
        g_main_flag = 1;
        data.write_storge_address = BACK_SECTOR_NUM;
    }

	data.msg_type = E_SERIAL_NO_PARA_MSG;
	data.write_len = sizeof(g_serial_no);
	data.write_memery_address = (uint8_t *)&g_serial_no;
	set_write_data_to_queue(&data);
}

/*=============================================================
* 函数名称：HardwareVersionNoInit
* 函数功能：硬件版本号硬件初始化
* 参数个数：0
* 参数描述：
* 
* 返 回 值：无
* 修改记录：
*===============================================================
* 日    期           修改人      修改类型
* 2023-07-06       	 liyong     	创建
==============================================================*/
void HardwareVersionNoInit(void)
{
	int16_t flag = 0;
    HARDWARE_VERSION_NO hVNoB;

	flag = read_data_from_storage(HARDWARE_VER_START, (uint8_t *)&hardwareVersionNo, sizeof(hardwareVersionNo));
    if( flag == sizeof(hardwareVersionNo) )
    {
       flag = read_data_from_storage(HARDWARE_VER_BACK, (uint8_t *)&hVNoB, sizeof(hVNoB));
       if( flag == sizeof(hVNoB))
       {
          flag = judge_new_sector(hardwareVersionNo.writtenFlag, hVNoB.writtenFlag);
       }   
    }

	if ( flag == -1 )
	{
		mainFlagHardware = 1;
		writtenFlagHardware = hVNoB.writtenFlag;
		memcpy(&hardwareVersionNo, &hVNoB, sizeof(hardwareVersionNo));
	}
	else if ( flag == 0 )
	{
        static char sno[] = "BF24-MNS5_V";
		mainFlagHardware = 1;
		writtenFlagHardware = MAX_STORAGE_NUM;
		memcpy(hardwareVersionNo.no, &sno, sizeof(sno));
	}
	else
	{
		mainFlagHardware = 0;
		writtenFlagHardware = hardwareVersionNo.writtenFlag;
	}
    
}
/*=============================================================
* 函数名称：WriteHardwareVersionNoToEeprom
* 函数功能：写硬件版本号到EEPROM
* 参数个数：0
* 参数描述：
* 返 回 值：无
*           
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2023-07-06       	 liyong     	创建
==============================================================*/
void WriteHardwareVersionNoToEeprom(void)
{
	t_write_data_st data;

	writtenFlagHardware = sector_num_next(writtenFlagHardware);
	hardwareVersionNo.writtenFlag = writtenFlagHardware;
	if (mainFlagHardware)
    {
        mainFlagHardware = 0;
        data.write_storge_address = HARDWARE_VER_START;
    }
	else
    {
        mainFlagHardware = 1;
        data.write_storge_address = HARDWARE_VER_BACK;
    }

	data.msg_type = E_HARDWARE_VERSION_MSG;
	data.write_len = sizeof(hardwareVersionNo);
	data.write_memery_address = (uint8_t *)&hardwareVersionNo;
	set_write_data_to_queue(&data);
}

/*=============================================================
* 函数名称：SetHardwareVersionNo
* 函数功能：设置硬件版本系列号
* 参数个数：1
* 参数描述：
*           [IN/OUT] buff         硬件版本号内容存放缓冲
* 返 回 值：
            无
* 修改记录：
*===============================================================
* 日    期           修改人       修改类型
* 2023-07-06       	 liyong     	创建
==============================================================*/
void SetHardwareVersionNo(uint8_t *buff)
{
	memcpy(hardwareVersionNo.no, buff, sizeof(hardwareVersionNo.no));

	WriteHardwareVersionNoToEeprom();
}

