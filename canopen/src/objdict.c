
/**************************************************************************/
/* Object Dictionary for the node Linux_slave, default node_id : 0x01     */
/*                                                                        */
/* Computed by "makeobjetdict.php"                                        */
/**************************************************************************/
/* Computed by strComputed */

/*********************************************************                   
 *                                                       *
 *             Master/slave CANopen Library              *
 *                                                       *
 *  LIVIC : Laboratoire Interractions Vicule             * 
 *          Infrastructure Conducteur                    *
 *                       ----                            *
 *  INRETS/LIVIC : http://www.inrets.fr                  *
 *      Institut National de Recherche sur               *
 *      les Transports                                   *
 *      et leur Surit                                    * 
 *  LCPC Laboratoire Central des Ponts et Chausss        *
 *  Laboratoire Interractions Vicule Infrastructure      *
 *  Conducteur                                           *
 *                                                       *
 *  Authors  : Camille BOSSARD                           *
 *             Francis DUPIN                             *
 *             Laurent ROMIEUX                           *
 *             Zakaria BELAMRI                           *
 *  Contact : bossard.ca@voila.fr                        *
 *            francis.dupin@inrets.fr                    *
 *            zakaria_belamri@hotmail.com                *
 *  Date    : 2003                                       *
 * This work is based on                                 *
 * -     CanOpenMatic by  Edouard TISSERANT              *
 *       http://sourceforge.net/projects/canfestival/    * 
 * -     slavelib by    Raphael Zulliger                 *
 *       http://sourceforge.net/projects/canopen/        *
 *********************************************************
 *                                                       *
 *********************************************************
 * This program is free software; you can redistribute   *
 * it and/or modify it under the terms of the GNU General*
 * Public License as published by the Free Software      *
 * Foundation; either version 2 of the License, or (at   *
 * your option) any later version.                       *
 *                                                       *
 * This program is distributed in the hope that it will  *
 * be useful, but WITHOUT ANY WARRANTY; without even the *
 * implied warranty of MERCHANTABILITY or FITNESS FOR A  *
 * PARTICULAR PURPOSE.  See the GNU General Public       *
 * License for more details.                             *
 *                                                       *
 * You should have received a copy of the GNU General    *
 * Public License along with this program; if not, write *
 * to 	The Free Software Foundation, Inc.               *
 *	675 Mass Ave                                     *
 *	Cambridge                                        *
 *	MA 02139                                         *
 * 	USA.                                             *
 *********************************************************
           File : objdict.c
 *-------------------------------------------------------*
 * This is where you defined the dictionary of your      *
 *  application                                          *      
 *                                                       *
 *********************************************************/
 /* Computed by strEntete */

#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include "parameter.h"
#include "vol_manage.h"
#include "soc.h"
#include "can_app.h"
#include "temp_manage.h"
#include "can_open_timer.h"
#include "current_manage.h"
#include "system_adjust.h"
#include "serial_no.h"
#include "clear_swd.h"
#include "rtc.h"
#include "run_record.h"
#include "peak_record.h"
#include "storage_manage.h"
#include "afe_app.h"
#include "low_power.h"
#include <def.h>
#include <pdo.h>
#include <sdo.h>
#include <sync.h>
#include <objdictdef.h>
#include <nmtSlave.h>
#include <lifegrd.h>
#include "final_protect.h"

#if defined (TIANHONG) && defined (BAT_8S)/* 天宏8串磷酸铁锂 */
#define HEART_TIMER      0x000A
#else
#define HEART_TIMER      0x0064
#endif

//static const char hard_version[] = "BF24-MNS5_V0.02.001";

#if defined(HUAFU)
#define  CUST STRINGS(HF)       
#elif defined(FUDEER)
#define  CUST STRINGS(FD) 
#elif defined(YUHENG)
#define  CUST STRINGS(YH)  
#elif defined(NEWNOB)
#define  CUST STRINGS(ND)
#elif defined(TIANFENG)
#define  CUST STRINGS(TF)
#elif defined(TIANHONG)
#define  CUST STRINGS(TH)
#endif
#if defined(BAT_16S)
#define  CELLS  STRINGS(16S)    
#elif defined(BAT_15S)
#define  CELLS  STRINGS(15S) 
#elif defined(BAT_14S)
#define  CELLS  STRINGS(14S)   
#elif defined(BAT_13S)
#define  CELLS  STRINGS(13S)       
#elif defined(BAT_8S)  
#define  CELLS  STRINGS(08S)
#elif defined(BAT_7S) 
#define  CELLS  STRINGS(07S)
#else
#define  CELLS  STRINGS(00S)    
#endif 
    
#if defined(CAPCITY_12)
#define  AHS  STRINGS(12)    
#elif defined(CAPCITY_20)
#define  AHS  STRINGS(20)
#elif defined(CAPCITY_30)
#define  AHS  STRINGS(30)   
#elif defined(CAPCITY_36)
#define  AHS  STRINGS(36)       
#elif defined(CAPCITY_40)  
#define  AHS  STRINGS(40)
#elif defined(CAPCITY_50) 
#define  AHS  STRINGS(50)
#elif defined(CAPCITY_55) 
#define  AHS  STRINGS(55)
#elif defined(CAPCITY_60) 
#define  AHS  STRINGS(60)
#elif defined(CAPCITY_100) 
#define  AHS  STRINGS(100)
#elif defined(CAPCITY_125) 
#define  AHS  STRINGS(125)
#else
#define  AHS  STRINGS(00)    
#endif    
 
#define STRINGS(x)         #x
#ifdef JUNH_COTP
#define SOFT_HEAR(c,x,y,z)   ""c"JH1-"x""y"RPX1_"z""
#elif defined(TIANFENG)
#define SOFT_HEAR(c,x,y,z)   ""c"RY2-"x""y"MNS5_"z""
#elif defined(TIANHONG)
#define SOFT_HEAR(c,x,y,z)   ""c"JL2-"x""y"MNS5_"z""
#elif defined(YUHENG)
#define SOFT_HEAR(c,x,y,z)   ""c"RY1-"x""y"MNS5_"z""
#else
#define SOFT_HEAR(c,x,y,z)   ""c"RY2-"x""y"MNS5_"z""
#endif    
const uint16_t OVPThreshold = AFE_OVP_CELL_VOL;
const uint16_t OVPRThreshold = 0;
const uint16_t UVPThreshold = AFE_UVP_CELL_VOL;
const uint16_t UVPRThreshold = 0;
const uint16_t OCDThresh = AFE_OVER_DCH_CURRENT;

static void ReadFirstRunRecordProc(void)
{
    (void)read_first_run_record();
}

static void ReadNextRunRecordProc(void)
{
    (void)read_next_run_record();
}

static void ReadFirstProtectRecordProc(void)
{
    (void)read_first_protect_record();
}

static void ReadNextProtectRecordProc(void)
{
    (void)read_next_protect_record();
}

//2023.12.21根据BF24-RPX1的软件修改而来
/* 2024-03-08, 去掉了低电量大电流保护 ，增加了天丰宏定义 V4.00.02*/
/* 2024-03-27, 天丰去掉了切断放电MOS1S延时，把二次保护延时颗粒度从50ms改到了5ms V4.00.03*/
/* 2024-04-01, 天丰由于加二次保护模块，导致盲充电压压差增加到1.2V（实测0.8V以上有效），修改逻辑使充满电后直接进休眠V4.00.04*/
/* 2024-05-09, 天丰生产测试对单个电芯过充，会在待机情况下触发AFE过压保护，需要手动开关机。
                所以增加AFE过压解除条件：最高单体电压<单体过压恢复值持续1s，且无其他AFE保护时，才解除AFE过压保护。V4.00.05*/
/* 2024-06-12, 修改了满充检测时间3s->6s，因为由于电压保护设置5s，导致过压后休眠后，唤醒时若有欠压保护，仍然会进入满充休眠  V4.00.06 */
/* 2024-06-13, 增加了温度差值大于20°不开启加热的情况 V4.00.07*/
/* 2024-06-13, 满充不休眠后直接放电，会误报单体过压 V4.00.08*/
/* 2024-06-17，天丰要求放电高温保护 65℃调整到60℃ 。增加了硬件版本号写入功能 V4.00.09*/
/* 2024-07-15, 送样天丰，软件去掉天丰特殊逻辑，增加了天宏宏定义和参数，按要求增加了CAN的PDO V4.01.01 */
/* 2024-07-16, 去掉0x2F0和0x62C帧ID的报文，只保留加力要求部分 V4.01.02*/
/*V4.01.03 2024-08-02, 经测试100Ah短路保护电流值最佳为1040A，100uS */
/*V4.01.01 2025-08-01, 宇恒如意样品55Ah，改整车协议，充电协议，协议进充电，待机状态充电保护，充电互斥保护，zk，参数使用天丰*/       
/*V4.01.02 2025-09-17, 宇恒如意增加三级告警，去掉充电互斥 */
/*V4.01.03 2025-09-26, 宇恒如意增加can唤醒功能 */
/*V4.01.03 2026-02-05, 宇恒如意修改参数，低温充电告警温度从10度调整为5度，优化0x18FFAFF4故障上报 */

/* V4.00.01, 2026-06-08 如意2460mos管方案，充电低温保护延时2s切断mos，待机30min休眠，修改参数为如意参数（三级告警） */
/* V4.00.02, 2026-06-16 如意2460mos管方案，针对继电器方案测试遇到的问题进行调整，增加工厂模式、修改波特率为250K */
#define  VERSION        STRINGS(V4.00.02)

#define  SOFT_VERSION   SOFT_HEAR(CUST,CELLS,AHS,VERSION)

const char soft_version[] = SOFT_VERSION;  
static const uint32_t negt_version = 0x040001;
uint32_t g_soft_version = 0x040000;                  
uint32_t g_start_soft_version = 0x0400000;
uint32_t g_end_soft_version = 0x0404000;
extern uint16_t totalVoltage;
/**************************************************************************/
/*                             The node id                                */
/**************************************************************************/
/* Computed by strNode */
/* node_id default value. 
   This default value is deprecated.
   You should always overwrite this by using the function setNodeId(UNS8 nodeId) in your C code.
*/
UNS32 canopenErrNB;	/* Mapped at index 0x6000, subindex 0x0 */
UNS32 canopenErrVAL;/* Mapped at index 0x6001, subindex 0x0 */
UNS8  bDeviceNodeId = NODE_ID;
/* Object dictionary */
const indextable *objdict;
quick_index obj_firstIndex;
quick_index obj_lastIndex;
UNS16 *ObjdictSize;
UNS8  *iam_a_slave;
extern UNS32 mcu_reset;
int GetSoftVersionForApp(void)
{
    int version;
    int xh,h,l;
    sscanf(VERSION,"V%x.%x.%x",&xh,&h,&l);
    version = xh;
    version <<= 8;
    version += h;
    version <<= 8;
    version += l;
    return version; 
}
int GetHardVersionForApp(void)
{
    int version;
    int xh,h,l;
    sscanf((char const *)hardwareVersionNo.no,"BF24-MNS5_V%x.%x.%x",&xh,&h,&l);
    version = xh;
    version <<= 8;
    version += h;
    version <<= 12;
    version += l;
    return version; 
}
/**************************************************************************/
/* Declaration of the mapped variables                                    */
/**************************************************************************/
/* Computed by strDeclareMapVar */
//*************************************************************************/
/* Computed by strStartDico */
#if 0
/* Array of message processing information */
/* Should not be modified */
proceed_info proceed_infos[] = 
{
	{NMT,		    "NMT",	        proceedNMTstateChange},
	{SYNC,          "SYNC",         proceedSYNC},
	{TIME_STAMP,	"TIME_STAMP",   NULL},
	{PDO1tx,	    "PDO1tx",       proceedPDO},
	{PDO1rx,	    "PDO1rx",	    proceedPDO},
	{PDO2tx,	    "PDO2tx",	    proceedPDO},
	{PDO2rx,	    "PDO2rx",	    proceedPDO},
	{PDO3tx,	    "PDO3tx",	    proceedPDO},
	{PDO3rx,	    "PDO3rx",	    proceedPDO},
	{PDO4tx,	    "PDO4tx",	    proceedPDO},
	{PDO4rx,	    "PDO4rx",	    proceedPDO},
	{SDOtx,         "SDOtx",	    proceedSDO},
	{SDOrx,         "SDOrx",        proceedSDO},
	{0xD,		    "Unknown",	    NULL},
	{NODE_GUARD,	"NODE GUARD",   proceedNMTerror},
	{0xF,		    "Unknown",	    NULL}
};
#endif

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
//                       OBJECT DICTIONARY
//                   
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Make your change, depending of your application
 
/** index 1000h: device type. You have to change the value below, so
 *  it fits your canopen-slave-module
 */
/* Not used, so, should not be modified */
#define OBJNAME devicetype
/*const*/ UNS32 OBJNAME = 0x0;
/*const*/ subindex Index1000[] =
{
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME, 0}
};
#undef OBJNAME

/** index 1001: error register. Change the entries to fit your application
 */
/* Not used, so, should not be modified */
#define OBJNAME errorRegister
/*const*/ UNS8 OBJNAME = 0x0;
/*const*/ subindex Index1001[] =
{
	{ RO, uint8_t, sizeof(UNS8), (void*)&OBJNAME, 0  }
};
#undef OBJNAME

/** index 1005: COB_ID SYNC
*/
/* Should not be modified */
#define OBJNAME CobIdSync
/*const*/ UNS32 OBJNAME = 0x40000080; // bit 30 = 1 : device can generate a SYNC message
                                      // Beware, it is over written when the node enters in reset mode
                                      // See initResetMode() in init.c
/*const*/ subindex Index1005[] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&OBJNAME, 0}
};
#undef OBJNAME

/** index 1006: SYNC period
*/

#define OBJNAME SyncPeriod
// For producing the SYNC signal every n micro-seconds.
// Put O to not producing SYNC
/*const*/ UNS32 OBJNAME = 0x0; /* Default 0 to not produce SYNC */
                               // Beware, it is over written when the node enters in reset mode.
                               // See initResetMode() in init.c
/*const*/ subindex Index1006[] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&OBJNAME, 0}
};
#undef OBJNAME

/** index 1007: Synchronous Window Length
*/
// Seems to be needed by DS401 to generate the SYNC signal.

#define OBJNAME SyncWindowLength
// For producing the SYNC signal every n micro-seconds.
// Put O to not producing SYNC
/*const*/ UNS32 OBJNAME = 0x0; /* Default 0 */
/*const*/ subindex Index1007[] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&OBJNAME, 0}
};
#undef OBJNAME

/**************************************************************************/
/* HeartBeat consumers : The nodes which can send a heartbeat             */
/**************************************************************************/
/* Computed by strHeartBeatConsumers */
static UNS32 HBConsumerTimeArray[] = {0x00000000}; // Format 0x00NNTTTT (N=Node T=time in ms)

static UNS8 HBConsumerCount = 1; // 1 nodes could send me their heartbeat.

subindex Index1016[] = {
	{ RO, uint8_t,sizeof(UNS8),  (void*)&HBConsumerCount, 0},
	{ RW, uint32, sizeof(UNS32), (void*)&HBConsumerTimeArray[0], 0}
};

/**************************************************************************/
/* The node produce an heartbeat                                          */
/**************************************************************************/
/* Computed by strHeartBeatProducer */
/* Every HBProducerTime, the node sends its heartbeat */
static UNS16 HBProducerTime = 0;  /* in ms. If 0 : not activated */ 
                                  // Beware, it is over written when the node enters in reset mode.
                                  // See initResetMode() in init.c
subindex Index1017[] =
{
	{ RW, uint16, sizeof(UNS16), &HBProducerTime, 0}
};

/**************************************************************************/
/* Next to 0x1018                                                         */
/**************************************************************************/
/* Computed by strVaria1 */
/** index 1018: identify object. Adjust the entries for your node/company
 */
/* Values can be modified */
#define OBJNAME theIdentity
/*const*/ s_identity OBJNAME = 
{
	4,       // number of supported entries
	0x1234,  // Vendor-ID (given by the can-cia)
	0x5678,  // Product Code
	0x1364,  // Revision number
	0x7964,  // serial number
};

/* Should not be modified */
/*const*/ subindex Index1018[] = 
{
	{ RO, uint8_t,sizeof(UNS8),  (void*)&OBJNAME.count, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.vendor_id, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.product_code, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.revision_number, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.serial_number, 0}
};
#undef OBJNAME

/** now the communication profile entries are grouped together, so they
 *  can be accessed in a standardised manner. This could be memory-optimized
 *  if the empty entries wouldn't be added, but then the communication profile
 *  area must be accessed different (see objacces.c file)
 */
/* Should not be modified */
const indextable CommunicationProfileArea[] = 
{
	DeclareIndexTableEntry(Index1000), /* 1000, 设备类型 */
	DeclareIndexTableEntry(Index1001), /* 1001, 错误寄存器 */
	{ NULL, 0 }, /* 1002, 制造商状态寄存器 */
	{ NULL, 0 }, /* 1003, 预定义错误域 */
	{ NULL, 0 }, /* 1004,  */
	DeclareIndexTableEntry(Index1005), /* 1005, COB-ID 同步消息 */
	DeclareIndexTableEntry(Index1006), /* 1006, 通信循环周期 */
	DeclareIndexTableEntry(Index1007), /* 1007, 同步窗长度 */
	{ NULL, 0 }, /* 1008, 制造商设备名称 */
	{ NULL, 0 }, /* 1009, 制造商的硬件版本 */
	{ NULL, 0 }, /* 100A, 制造商软件版本 */
	{ NULL, 0 }, /* 100B,  */
	{ NULL, 0 }, /* 100C, 监护周期 */
	{ NULL, 0 }, /* 100D, 生存周期因子 */
	{ NULL, 0 }, /* 100E,  */
	{ NULL, 0 }, /* 100F,  */
	{ NULL, 0 }, /* 1010, 存储参数 */
	{ NULL, 0 }, /* 1011, 恢复缺省参数 */
	{ NULL, 0 }, /* 1012, 时间戳对象COB-ID */
	{ NULL, 0 }, /* 1013, 高分辨率时间戳 */
	{ NULL, 0 }, /* 1014, EMCY COB-ID */
	{ NULL, 0 }, /* 1015, EMCY 抑制时间 */
	DeclareIndexTableEntry(Index1016), /* 1015, 消费者心跳超时 */
	DeclareIndexTableEntry(Index1017), /* 1015, 生产者心跳超时 */
	DeclareIndexTableEntry(Index1018), /* 1015, 对象身份 */
};

/**************************************************************************/
/* The SDO Server parameters                                              */
/**************************************************************************/
/* Computed by strSdoServer */
/* BEWARE You cannot define more than one SDO server */

#define INDEX_LAST_SDO_SERVER           0x1200
#define DEF_MAX_COUNT_OF_SDO_SERVER     INDEX_LAST_SDO_SERVER - 0x11FF

/* The values should not be modified here, but can be changed at runtime */
// Beware that the default values that you could put here
// will be over written at the initialisation of the node. See setNodeId() in init.c
#define OBJNAME serverSDO1
static s_sdo_parameter OBJNAME = 
{
	3,                   // Number of entries. Always 3 for the SDO	       
	0x600 + NODE_ID,     // The cob_id transmited in CAN msg to the server     
	0x580 + NODE_ID,     // The cob_id received in CAN msg from the server  
	NODE_ID              // The node id of the client. Should not be modified 
};
static subindex Index1200[] = 
{
	{ RO, uint8_t,sizeof(UNS8 ), (void*)&OBJNAME.count, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.cob_id_client, 0},
	{ RO, uint32, sizeof(UNS32), (void*)&OBJNAME.cob_id_server, 0},
	{ RW, uint8_t,sizeof(UNS8),  (void*)&OBJNAME.node_id, 0}
};
#undef OBJNAME

/** Create the server SDO Parameter area.
 */
/* Should not be modified */
const indextable serverSDOParameter[] =
{
	DeclareIndexTableEntry(Index1200)
};

/**************************************************************************/
/* The SDO(s) clients                                                     */
/**************************************************************************/
/* Computed by strSdoClients */
/* For a slave node, declare only one SDO client to send data to the master */
/* The master node must have one SDO client for each slave */
#define INDEX_LAST_SDO_CLIENT           0x1280
#define DEF_MAX_COUNT_OF_SDO_CLIENT     1

#define _CREATE_SDO_CLIENT_(SDONUM) \
static s_sdo_parameter clientSDO##SDONUM = \
{\
	3,      /* Number of entries. Always 3 for the SDO*/\
	0x600,  /* The cob_id transmited in CAN msg to the server. Put the "good" value instead : 0x600 + server node */\
	0x580,  /* The cob_id received in CAN msg from the server. Put the "good" value instead : 0x600 + server node*/\
	0x00    /* The node id of the server. Put the "good" value : server node*/\
};\
static  subindex Index##SDONUM[] =\
{\
	{ RO, uint8_t,sizeof(UNS8 ), (void*)&clientSDO##SDONUM.count, 0},\
	{ RO, uint32, sizeof(UNS32), (void*)&clientSDO##SDONUM.cob_id_client, 0},\
	{ RO, uint32, sizeof(UNS32), (void*)&clientSDO##SDONUM.cob_id_server, 0},\
	{ RO, uint8_t,sizeof(UNS8),  (void*)&clientSDO##SDONUM.node_id, 0}\
};

_CREATE_SDO_CLIENT_(1280)

/* Create the client SDO Parameter area. */
const indextable clientSDOParameter[] =
{
	DeclareIndexTableEntry(Index1280)
};

/**************************************************************************/
/* The PDO(s) Which could be received                                     */
/**************************************************************************/
/* Computed by strPdoReceive */
#define INDEX_LAST_PDO_RECEIVE   0x1403
#define MAX_COUNT_OF_PDO_RECEIVE 4

// Beware that for index 0x1400 to 0x1403, the defaut cobid that you could put here
// will be over written at the initialisation of the node. See setNodeId() in init.c

#define _CREATE_RXPDO_(RXPDO) \
	static s_pdo_communication_parameter RxPDO##RXPDO = \
	{\
		2,           /* Number of entries. Always 2 */\
		0x000,       /* Default cobid*/\
		0x00,        /* Transmission type. See objetdictdef.h */\
	};\
	static  subindex Index##RXPDO[] =\
	{\
		{ RO, uint8_t,sizeof(UNS8),  (void*)&RxPDO##RXPDO.count, 0},\
		{ RW, uint32, sizeof(UNS32), (void*)&RxPDO##RXPDO.cob_id, 0},\
		{ RW, uint8_t,sizeof(UNS8),  (void*)&RxPDO##RXPDO.type, 0}\
	};

//This define the PDO receive entries from index 0x1400 to 0x1402 
_CREATE_RXPDO_(1400)
_CREATE_RXPDO_(1401)
_CREATE_RXPDO_(1402)
_CREATE_RXPDO_(1403)

/* Create the Receive PDO Parameter area. */
const indextable receivePDOParameter[] =
{ 
	DeclareIndexTableEntry(Index1400), 
	DeclareIndexTableEntry(Index1401), 
	DeclareIndexTableEntry(Index1402), 
	DeclareIndexTableEntry(Index1403), 
};

/**************************************************************************/
/* The PDO(s) Which could be transmited                                   */
/**************************************************************************/
/* Computed by strPdoTransmit */
#define INDEX_LAST_PDO_TRANSMIT   0x1803
#define MAX_COUNT_OF_PDO_TRANSMIT 4
/**Usually the ID of a transmitting PDO is 0x180 + device_node_id,
*  but the node_id is not known during compilation... so what to do?!
*  the correct values have to be setted up during bootup of the device...
*  So, Beware that for index 0x1800 to 0x1803, the defaut cobid that you could put here
*  will be over written at the initialisation of the node. See setNodeId() in init.c
*/
#define _CREATE_TXPDO_(TXPDO) \
static s_pdo_communication_parameter TxPDO##TXPDO = \
{\
	3,           /* Number of entries. Always 2 */\
	0x000,       /* Default cobid */\
	0x00,        /* Transmission type. See objetdictdef.h */\
	0x0000,      /* inhibit time: not supported yet */\
	0x00,        /* reserved (by CANopen standard) */\
    HEART_TIMER,      /* event timer: 10ms为单位., 定义为100MS */\
};\
static subindex Index##TXPDO[] =\
{\
	{ RO, uint8_t,sizeof(UNS8),  (void*)&TxPDO##TXPDO.count, 0},\
	{ RW, uint32, sizeof(UNS32), (void*)&TxPDO##TXPDO.cob_id, 0},\
	{ RW, uint8_t,sizeof(UNS8),  (void*)&TxPDO##TXPDO.type, 0}, \
	{ RW, uint16, sizeof(UNS8),  (void*)&TxPDO##TXPDO.event_timer, 0}, \
};

// This define the PDO transmit entries from index 0x1800 to 0x1802 
_CREATE_TXPDO_(1800)
_CREATE_TXPDO_(1801)
_CREATE_TXPDO_(1802)
_CREATE_TXPDO_(1803)

/* Create the Transmit PDO Parameter area. */
const indextable transmitPDOParameter[] =
{ 
	DeclareIndexTableEntry(Index1800), 
	DeclareIndexTableEntry(Index1801), 
	DeclareIndexTableEntry(Index1802), 
	DeclareIndexTableEntry(Index1803)
};

/**************************************************************************/
/* PDO Mapping parameters                                                 */
/**************************************************************************/
/* Computed by strPdoParam */

#define PDO_MAP(index, sub_index, size_variable_in_bits)  0x##index##sub_index##size_variable_in_bits

/* Beware : 
index                 *must* be writen 4 numbers in hexa
sub_index             *must* be writen 2 numbers in hexa
size_variable_in_UNS8 *must* be writen 2 numbers in hexa
*/
/* Max number of data which can be put in a PDO
   Example, one PDO contains 2 objects, an other contains 5 objects.
   put 
   MAX_COUNT_OF_PDO_MAPPING 5
*/
#define MAX_COUNT_OF_PDO_MAPPING   8

typedef struct td_s_pdo_mapping_parameter
{
	/** count of mapping entries
	*/
	UNS8 count;
	/** mapping entries itself.
	*/
	UNS32 object[MAX_COUNT_OF_PDO_MAPPING];
} s_pdo_mapping_parameter;

/**************************************************************************/
/* The mapping area of the PDO received                                   */
/**************************************************************************/
/* Computed by strPdoReceiveMapTop */
/* Note, The index 160x is used to map the PDO 140x. The relation between the two is automatic */
/* Computed by  strCreateRxMap */
static s_pdo_mapping_parameter RxMap1600 = 
/* 上报前1--4节单体电压 */
{
	0, 
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1600 [] =
{
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1600.object[0], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1600.object[1], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1600.object[2], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1600.object[3], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1600.object[4], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1600.object[5], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1600.object[6], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1600.object[7], 0  }
};

static s_pdo_mapping_parameter RxMap1601 = 
/* 上报前5--8节单体电压 */
{
	0, 
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1601 [] = 
{
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1601.object[0], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1601.object[1], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1601.object[2], 0  },
	{ RW, uint16, sizeof(UNS16), (void*)&RxMap1601.object[3], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1601.object[4], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1601.object[5], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1601.object[6], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1601.object[7], 0  }
};

static s_pdo_mapping_parameter RxMap1602 = 
/* 上报温度数值 */
{
	0,
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1602 [] = 
{
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[0], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[1], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[2], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[3], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[4], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[5], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[6], 0  },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1602.object[7], 0  }
};

static s_pdo_mapping_parameter RxMap1603 = 
{
	0,
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1603 [] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[0], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[1], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[2], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[3], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[4], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[5], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[6], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&RxMap1603.object[7], 0 }
};

/* Computed by strPdoReceiveMapBot */
#define INDEX_LAST_PDO_MAPPING_RECEIVE  0x1603

const indextable RxPDOMappingTable[] =
{
	DeclareIndexTableEntry(Index1600), 
	DeclareIndexTableEntry(Index1601), 
	DeclareIndexTableEntry(Index1602), 
	DeclareIndexTableEntry(Index1603), 
};

/**************************************************************************/
/* The mapping area of the PDO transmited                                   */
/**************************************************************************/
/* Computed by strPdoTransmitMapTop */
/* Note, The index 18xx is used to map the PDO 1Axxx. The relation between the two is automatic */
/* Computed by  strCreateRxMap */
static s_pdo_mapping_parameter TxMap1a00 = 
{
	0, 
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};

subindex Index1a00[] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[0], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[1], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[2], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[3], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[4], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[5], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[6], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a00.object[7], 0 }
};

static s_pdo_mapping_parameter TxMap1a01 = 
{
    6, 
	{
		PDO_MAP(2009, 05, 10), /*总压*/
		PDO_MAP(2009, 01, 10), /*电流*/
		PDO_MAP(2009, 04, 08), /*soc*/
		PDO_MAP(2004, 01, 08), /*系统容量*/
		PDO_MAP(2009, 02, 08), /*故障上报*/
		PDO_MAP(2009, 03, 08), /*温度*/
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1a01[] =
{
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[0], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[1], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[2], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[3], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[4], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[5], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[6], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a01.object[7], 0 }
};

static s_pdo_mapping_parameter TxMap1a02 = 
{
	0, 
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1a02[] = 
{
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[0], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[1], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[2], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[3], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[4], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[5], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[6], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a02.object[7], 0 }
};

static s_pdo_mapping_parameter TxMap1a03 = 
{
	0, 
	{
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00),
		PDO_MAP(0000, 00, 00)
	}
};
subindex Index1a03[] = 
{
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[0], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[1], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[2], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[3], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[4], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[5], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[6], 0 },
	{ RW, uint32, sizeof(UNS32), (void*)&TxMap1a03.object[7], 0 }
};

/* Computed by strPdoTransmitMapBot */
#define INDEX_LAST_PDO_MAPPING_TRANSMIT  0x1a03

const indextable TxPDOMappingTable[] = 
{
	DeclareIndexTableEntry(Index1a00), 
	DeclareIndexTableEntry(Index1a01), 
	DeclareIndexTableEntry(Index1a02), 
	DeclareIndexTableEntry(Index1a03), 
};

/**************************************************************************/
/* The mapped variables at index 0x2000 - 0x5fff                          */
/**************************************************************************/
/* Computed by strMapVar */

/********* Index 2000 *********/
static UNS8 voltageSubIndex = 0x13; // number of subindex - 1
subindex Index2000[] = 
{
	{ RO, uint8_t,sizeof(UNS8),  (void*)&voltageSubIndex },
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.low.alarm), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.low.alarm_recover), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.low.protect), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.low.recover), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.high.alarm), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.high.alarm_recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.high.protect), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.cell_vol.high.recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.low.alarm), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.low.alarm_recover), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.low.protect), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.low.recover), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.high.alarm), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.high.alarm_recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.high.protect), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.vol_para.voltage.high.recover), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.pretect_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_vol_para.dis_charge_level), 0}, 
	{ RW, uint8_t,sizeof(UNS8),  (void*)&(g_vol_para.voltage_flag), 0}, 
};

/********* Index 2001 *********/
static UNS8 tempSubIndex = 0x1A;       // number of subindex - 1
subindex Index2001[] = 
{
	{ RO, uint8_t,sizeof(UNS8),  (void*)&tempSubIndex, 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.low.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.low.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.low.recover), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.high.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.high.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.ch_temp.high.recover), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.low.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.low.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.low.recover), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.high.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.high.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.cell_temp.dch_temp.high.recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_temp_para.cell_temp.protect_delay), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.low.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.low.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.low.recover), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.high.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.high.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.temp.high.recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_temp_para.enviror_temp.protect_delay), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.low.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.low.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.low.recover), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.high.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.high.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_temp_para.power_temp.temp.high.recover), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_temp_para.power_temp.protect_delay), 0},
	{ RW, uint8_t,sizeof(UNS8),  (void*)&(g_temp_para.temp_flag), 0}, 
};

/********* Index 2002 *********/
static UNS8 currentSubIndex = 0x10;    // number of subindex - 1
subindex Index2002[] = 
{
	{ RO, uint8_t,sizeof(UNS8),  (void*)&currentSubIndex, 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.ch_current.in_level), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.ch_current.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.ch_current.protect), 0},
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.ch_current.protect_delay), 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.ch_current.limit), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.dch_current.in_level), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.dch_current.alarm), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.dch_current.protect), 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.dch_current.second_protect), 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_curr_para.dch_current.short_protect), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.protect_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.second_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.short_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.short_recover_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.protect_recover_delay), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_curr_para.dch_current.locked_num), 0}, 
	{ RW, uint8_t,sizeof(UNS8),  (void*)&(g_curr_para.current_flag), 0}, 
};

/********* Index 2003 *********/
static UNS8 balanceSubIndex = 0x0B;    // number of subindex - 1
subindex Index2003[] = 
{
	{ RO, uint8_t,sizeof(UNS8),  (void*)&balanceSubIndex, 0},
	{ RW, int16,  sizeof(UNS16), (void*)&(g_hot_balance_para.hot.start), 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_hot_balance_para.hot.end), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.fail_level), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.recover_level), 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_hot_balance_para.balance.end_temp), 0}, 
	{ RW, int16,  sizeof(UNS16), (void*)&(g_hot_balance_para.balance.start_temp), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.start), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.diff_start), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.diff_end), 0}, 
	{ RW, uint16, sizeof(UNS16), (void*)&(g_hot_balance_para.balance.delay), 0}, 
	{ RW, uint8_t,sizeof(UNS8),  (void*)&(g_hot_balance_para.balance_flag), 0},
};

/********* Index 2004 *********/
static UNS8 capcitySubIndex = 0x0A;    // number of subindex - 1
subindex Index2004[] = 
{
	{ RO, uint8_t, sizeof(UNS8),  (void*)&capcitySubIndex, 0},
	{ RW, uint16,  sizeof(UNS16), (void*)&(g_capcity_para.rated_capcity), 0}, 
	{ RW, uint16,  sizeof(UNS16), (void*)&(g_capcity_para.low_power_delay), 0}, 
	{ RW, uint16,  sizeof(UNS16), (void*)&(g_capcity_para.deactive_delay), 0}, 
	{ RW, uint16,  sizeof(UNS16), (void*)&(g_capcity_para.low_deactive_delay), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.smart.soc_level), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.smart.delay), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.capcity.low), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.capcity.protect), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.capcity.high), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_capcity_para.capcity_flag), 0},
};

/********* Index 2005 *********/
static UNS8 otherSubIndex = 0x03;      // number of subindex - 1
subindex Index2005[] = 
{
	{ RO, uint8_t, sizeof(UNS8),  (void*)&otherSubIndex, 0},
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_other_para.cell_num), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_other_para.show_flag), 0}, 
	{ RW, uint8_t, sizeof(UNS8),  (void*)&(g_other_para.outside_flag), 0}, 
};

static uint16_t temp_default;

/********* Index 2006 *********/
static UNS8 highestSubIndex_2006 = 24;  // number of subindex - 1
subindex Index2006[] = 
{
    { RW,               uint8_t,sizeof (UNS8),                  (void*)&highestSubIndex_2006, 0 },   /* 0x00 */ 
	{ RW | TO_BE_SAVED, domain, sizeof(g_vol_para) - 1,         (void*)&g_vol_para,  save_vol_para_parameter},   /* 0x01 */
	{ RW | TO_BE_SAVED, domain, sizeof(g_temp_para) - 1,        (void*)&g_temp_para, save_temp_para_parameter},   /* 0x02 */
	{ RW | TO_BE_SAVED, domain, sizeof(g_curr_para) - 1,        (void*)&g_curr_para, save_curr_para_parameter},   /* 0x03 */
	{ RW | TO_BE_SAVED, domain, sizeof(g_hot_balance_para) - 1, (void*)&g_hot_balance_para, save_balance_para_parameter},   /* 0x04 */
	{ RW | TO_BE_SAVED, domain, sizeof(g_capcity_para) - 1,     (void*)&g_capcity_para, save_capcity_para_parameter},   /* 0x05 */
	{ RW | TO_BE_SAVED, domain, sizeof(g_other_para) - 1,       (void*)&g_other_para, save_other_para_parameter},   /* 0x06 */
	{ RO,               domain, sizeof(g_run_sys_data),         (void*)&g_run_sys_data, 0},   /* 0x07 */
	{ RW | TO_BE_SAVED, uint16, sizeof(UNS16),                  (void*)&temp_default, system_para_default},   /* 0x08 */
	{ RW | TO_BE_SAVED, uint8_t,sizeof(g_datetime),             (void*)&g_datetime, rtc_set_datetime},   /* 0x09 */
    { RO,               domain, sizeof(g_read_run_record),      (void*)&g_read_run_record, 0}, /* 0x0A */
	{ RW | TO_BE_SAVED, uint16, sizeof(UNS16),                  (void*)&temp_default, ReadFirstRunRecordProc},   /* 0x0B */
	{ RW | TO_BE_SAVED, uint16, sizeof(UNS16),                  (void*)&temp_default, ReadNextRunRecordProc},   /* 0x0C */
    { RO,               domain, sizeof(g_read_protect),         (void*)&g_read_protect, 0},                           /* 0x0D */
    { RW | TO_BE_SAVED, uint16, sizeof(UNS16),                  (void*)&temp_default, ReadFirstProtectRecordProc},   /* 0x0E */
	{ RW | TO_BE_SAVED, uint16, sizeof(UNS16),                  (void*)&temp_default, ReadNextProtectRecordProc},   /* 0x0F*/  
    { RO ,              domain, sizeof(g_peak_record1) ,    (void*)&g_peak_record1,  0},        /* 0x10 */
    { RO ,              domain, sizeof(g_peak_record2) ,    (void*)&g_peak_record2,  0},        /* 0x11 */
    { RO ,              domain, sizeof(g_dch_circle) ,      (void*)&g_dch_circle,  0},          /* 0x12 */
    { RO ,              domain, sizeof(protect1_numb) ,     (void*)&protect1_numb,  0},         /* 0x13 */
    { RO ,              domain, sizeof(protect2_numb) ,     (void*)&protect2_numb,  0},         /* 0x14 */
    { RO ,              domain, sizeof(fault1_numb_record) ,(void*)&fault1_numb_record,  0},   /* 0x15 */
    { RO ,              domain, sizeof(fault2_numb_record) ,(void*)&fault2_numb_record,  0},   /* 0x16 */
    { RO ,              domain, (sizeof(cell_blance_numb)-1)/2 ,  (void*)&cell_blance_numb,  0},     /* 0x17 */
    { RO ,              domain, (sizeof(cell_blance_numb)-1)/2 ,  (void*)(&cell_blance_numb.cellblance[8]),  0},     /* 0x18 */
    { RO ,              domain, sizeof(last_two_update) ,   (void*)&last_two_update,  0},      /* 0x19 */
    
};

/********* Index 2007 *********/
static UNS8 highestSubIndex_2007 = 32;    // number of subindex - 1
subindex Index2007[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_2007, 0},           /* 0x00 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.total_vol), 0},     /* 0x01 */ //
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_run_sys_data.current), 0},       /* 0x02 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.soc), 0},           /* 0x03 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[0]), 0},   /* 0x04 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[1]), 0},   /* 0x05 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[2]), 0},   /* 0x06 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[3]), 0},   /* 0x07 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[4]), 0},   /* 0x08 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[5]), 0},   /* 0x09 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[6]), 0},   /* 0x0A */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[7]), 0},   /* 0x0B */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[8]), 0},   /* 0x0C */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[9]), 0},   /* 0x0D */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[10]), 0},  /* 0x0E */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[11]), 0},  /* 0x0F */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[12]), 0},  /* 0x10 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[13]), 0},  /* 0x11 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[14]), 0},  /* 0x12 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_run_sys_data.cell_vol[15]), 0},  /* 0x13 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_run_sys_data.power_temp), 0},    /* 0x14 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_run_sys_data.ambient_temp), 0},  /* 0x15 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_run_sys_data.cell_temp[0]), 0},  /* 0x16 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_run_sys_data.cell_temp[1]), 0},  /* 0x17 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.vol_event), 0},     /* 0x18 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.cur_event), 0},     /* 0x19 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.temp_event), 0},    /* 0x1A */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.ba_event), 0},      /* 0x1B */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.cap_event), 0},     /* 0x1C */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.other_event), 0},   /* 0x1D */
    { RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.new_event1), 0},     /* 0x1E */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_run_sys_data.new_event2), 0},   /* 0x1F */
    { RO, uint8_t, sizeof(g_run_sys_data.write_time),(void*)&(g_run_sys_data.write_time),                  0},   /* 0x20 */
};

/********* Index 2008 *********/
uint32_t g_dry_switch_flag;
static UNS8 highestSubIndex_2008 = 33;
subindex Index2008[] = 
{
    { RW,               uint8_t,        sizeof(UNS8),        (void*)&highestSubIndex_2008, 0}, /* 0x00 */
	{ RW | TO_BE_SAVED, int16,          sizeof(INTEGER16),   (void*)&correct_curr, current_adjust},        /* 0x01 */
	{ RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&correct_vol1, voltage1_adjust},       /* 0x02 */
	{ RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&correct_vol2, voltage2_adjust},       /* 0x03 */
	{ RO,               visible_string, sizeof(soft_version),(void*)soft_version, 0},  /* 0x04 */
{ RO,               visible_string, sizeof(hardwareVersionNo.no),(void*)hardwareVersionNo.no, 0},  /* 0x05 */
	{ RO ,              uint16,         sizeof(UNS16),       (void*)&g_dch_circle.circle_num.conut, 0},  /* 0x06 */	
	{ RO ,              uint16,         sizeof(UNS16),       (void*)&g_dch_circle.charge_num.conut, 0},  /* 0x07 */	
	{ RW | TO_BE_SAVED, visible_string, MAX_SERIAL_NO_LENGTH,(void*)&g_serial_no.serial_no, write_serial_no_to_eeprom},  /* 0x08 */	
	{ RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_balance_test, balance_adjust},  /* 0x09 */
    //{ RW | TO_BE_SAVED, uint8_t,        sizeof(UNS8),        (void*)&g_clear_swd_flag, (UNS32)config_swd_function}, /* 0x0A */
	{ RO,               uint32,       sizeof(negt_version),(void*)&(negt_version), 0},  /* 0x0A */	
    { RO,               uint32,       sizeof(negt_version),(void*)&(negt_version), 0},  /* 0x0B */	
    { RW | TO_BE_SAVED, visible_string, MAX_SERIAL_NO_LENGTH,(void*)&g_serial_no.battery_no, write_serial_no_to_eeprom}, /* 0x0C */   
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_break_test, Break_detection_test},  /* 0x0D */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_software_reset, software_reset},  /* 0x0E */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_clear_protect, clear_protect_numb},  /* 0x0F */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_clear_fault, clear_fault_numb},  /* 0x10 */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_clear_voltage_Adjust, clear_voltage_Adjust},  /* 0x11 */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_clear_current_Adjust, clear_current_Adjust},  /* 0x12 */
    { RW | TO_BE_SAVED, uint16,         sizeof(UNS16),       (void*)&g_clear_limit, clear_limit_vaule},  /* 0x13 */
    { RW | TO_BE_SAVED, uint32,         sizeof(UNS32),       (void*)&g_test_cmd, test_bms_cmd},  /* 0x14 */
    { RO ,              uint16,         sizeof(UNS16),       (void*)&OVPThreshold, 0},  /* 0x15 */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&OVPRThreshold, 0},  /* 0x16 */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&UVPThreshold, 0},  /* 0x17 */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&UVPRThreshold, 0},  /* 0x18 */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&g_system_protect.vol_high_protect, 0},  /* 0x19 */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&g_system_protect.vol_high_recover, 0},  /* 0x1A */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&g_system_protect.vol_low_protect, 0},  /* 0x1B */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&g_system_protect.vol_low_recover, 0},  /* 0x1C */	
    { RO ,              uint16,         sizeof(UNS16),       (void*)&OCDThresh, 0},  /* 0x1D */
    { RO,               uint32,         sizeof(mcu_reset),   (void*)&(mcu_reset), 0},   /* 0x1E */		   
    { RO ,              uint16,         sizeof(UNS16),       (void*)&totalVoltage, 0},  /* 0x1F */	
    { RW | TO_BE_SAVED, visible_string, 24,(void*)&hardwareVersionNo.no, WriteHardwareVersionNoToEeprom},  /* 0x20 */	
    { RW | TO_BE_SAVED, uint32,         sizeof(UNS32),       (void*)&PowerOnCount, 0},  /* 0x21 */
};

/********* Index 2009 *********/
static UNS8 highestSubIndex_2009 = 5;    // number of subindex - 1
subindex Index2009[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_2009, 0},   /* 0x00 */
	{ RO, uint16,  sizeof(INTEGER16), (void*)&g_report_current, 0},       /* 0x01 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&g_report_status, 0},        /* 0x02 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&g_report_cell_temp, 0},     /* 0x03 */
    { RO, uint8_t, sizeof(UNS8),      (void*)&g_report_soc, 0},           /* 0x04 */
    { RO, uint16,  sizeof(UNS16),     (void*)&g_report_Voltage, 0},       /* 0x05 */
};

/********* Index 200A *********/
static UNS8 highestSubIndex_200A = 32;    // number of subindex - 1
subindex Index200A[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_200A, 0},           /* 0x00 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.total_vol), 0},     /* 0x01 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_run_record.current), 0},       /* 0x02 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.soc), 0},           /* 0x03 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[0]), 0},   /* 0x04 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[1]), 0},   /* 0x05 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[2]), 0},   /* 0x06 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[3]), 0},   /* 0x07 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[4]), 0},   /* 0x08 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[5]), 0},   /* 0x09 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[6]), 0},   /* 0x0A */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[7]), 0},   /* 0x0B */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[8]), 0},   /* 0x0C */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[9]), 0},   /* 0x0D */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[10]), 0},   /* 0x0E */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[11]), 0},   /* 0x0F */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[12]), 0},   /* 0x10 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[13]), 0},   /* 0x11 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[14]), 0},   /* 0x12 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_run_record.cell_vol[15]), 0},   /* 0x13 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_run_record.power_temp), 0},    /* 0x14 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_run_record.ambient_temp), 0},  /* 0x15 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_run_record.cell_temp[0]), 0},  /* 0x16 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_run_record.cell_temp[1]), 0},  /* 0x17 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.vol_event), 0},  /* 0x18 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.cur_event), 0},  /* 0x19 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.temp_event), 0},  /* 0x1A */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.ba_event), 0},  /* 0x1B */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.cap_event), 0},  /* 0x1C */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.other_event), 0},  /* 0x1D */
    { RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.new_event1), 0},  /* 0x1E */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_run_record.new_event2), 0},  /* 0x1F */
	{ RO, uint8_t, sizeof(g_read_run_record.write_time),      (void*)&(g_read_run_record.write_time), 0},  /* 0x20 */
    { RO, uint32, sizeof(g_read_run_record.number),      (void*)&(g_read_run_record.number), 0},  /* 0x21 */
};

/********* Index 200B *********/
static UNS8 highestSubIndex_200B = 14;    // number of subindex - 1
subindex Index200B[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_200B, 0},                              /* 0x00 */
	{ RW, uint16,  sizeof(UNS16),     (void*)&(g_peak_record1.max_vol.ultimatevaule), 0},   /* 0x01 */
    { RW, uint8_t, sizeof(UNS8),     (void*)&(g_peak_record1.max_vol.location), 0},                 /* 0x02 */ 
	{ RW, uint8_t,  6,                 (void*)&(g_peak_record1.max_vol.time), 0},           /* 0x03 */
	{ RW, int16,   sizeof(INTEGER16), (void*)&(g_peak_record1.min_vol.ultimatevaule), 0},  /* 0x04 */
	{ RW, uint8_t, sizeof(UNS8),      (void*)&(g_peak_record1.min_vol.location), 0},                /* 0x05*/
	{ RW, uint8_t,  6,                 (void*)&(g_peak_record1.min_vol.time), 0},          /* 0x06*/  
    { RW, uint16,  sizeof(UNS16), (void*)&(g_peak_record1.max_totalvol.ultimatevaule), 0},          /* 0x07 */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record1.max_totalvol.time), 0},                   /* 0x08 */
	{ RW, uint16,  sizeof(UNS8), (void*)&(g_peak_record1.min_totalvol.ultimatevaule), 0},           /* 0x09 */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record1.min_totalvol.time), 0},                   /* 0x0A*/  
    { RW, uint16,  sizeof(UNS16), (void*)&(g_peak_record1.max_current.ultimatevaule), 0},           /* 0x0B */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record1.max_current.time), 0},                    /* 0x0C */
	{ RW, uint16,  sizeof(UNS8), (void*)&(g_peak_record1.min_current.ultimatevaule), 0},            /* 0x0D */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record1.min_current.time), 0},                     /* 0x0E */
};

/********* Index 200C *********/
static UNS8 highestSubIndex_200C = 14;    // number of subindex - 1
subindex Index200C[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_200C, 0},                                  /* 0x00 */
	{ RW, uint16,  sizeof(UNS16),     (void*)&(g_peak_record2.max_celltemp.ultimatevaule), 0},  /* 0x01 */
    { RW, uint8_t, sizeof(UNS8),      (void*)&(g_peak_record2.max_celltemp.location), 0},                 /* 0x02 */ 
	{ RW, uint8_t,  6,                 (void*)&(g_peak_record2.max_celltemp.time), 0},           /* 0x03 */
	{ RW, int16,   sizeof(INTEGER16), (void*)&(g_peak_record2.min_celltemp.ultimatevaule), 0},   /* 0x04 */
	{ RW, uint8_t, sizeof(UNS8),      (void*)&(g_peak_record2.min_celltemp.location), 0},                 /* 0x05*/
	{ RW, uint8_t,  6,                 (void*)&(g_peak_record2.min_celltemp.time), 0},            /* 0x06*/   
    { RW, uint16,  sizeof(UNS16), (void*)&(g_peak_record2.max_powtemp.ultimatevaule), 0}, /* 0x07 */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record2.max_powtemp.time), 0},           /* 0x08 */   
	{ RW, uint16,  sizeof(UNS8), (void*)&(g_peak_record2.min_powtemp.ultimatevaule), 0},  /* 0x09 */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record2.min_powtemp.time), 0},           /* 0x0A */   
    { RW, uint16,  sizeof(UNS16), (void*)&(g_peak_record2.max_envtemp.ultimatevaule), 0}, /* 0x0B */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record2.max_envtemp.time), 0},           /* 0x0C */  
	{ RW, uint16,  sizeof(UNS8), (void*)&(g_peak_record2.min_envtemp.ultimatevaule), 0},  /* 0x0D */
	{ RW, uint8_t,   6,           (void*)&(g_peak_record2.min_envtemp.time), 0},           /* 0x0E */
};

/********* Index 200D *********/
static UNS8 highestSubIndex_200D = 5;    // number of subindex - 1
subindex Index200D[] = 
{
    { RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_200D, 0},                  /* 0x00 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(g_dch_circle.charge_num.conut), 0},          /* 0x01 */
	{ RW, uint8_t,   6,             (void*)&(g_dch_circle.charge_num.time), 0},           /* 0x02 */ 
	{ RW, uint32,  sizeof(UNS32),  (void*)&(g_dch_circle.circle_num.conut), 0},          /* 0x03 */
	{ RW, uint8_t,   6,            (void*)&(g_dch_circle.circle_num.time), 0},           /* 0x04 */ 
    { RW, uint32 , sizeof(UNS32), (void*)&(g_dch_circle.total), 0},           /* 0x05 */
};

/********* Index 200E *********/
static UNS8 highestSubIndex_200E = 13;    // number of subindex - 1
subindex Index200E[] = 
{
    { RW, uint8_t, sizeof(UNS8),   (void*)&highestSubIndex_200E, 0},                    /* 0x00 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.over_vol_num.conut), 0},       /* 0x01 */
	{ RW, uint8_t,   6,             (void*)&(protect1_numb.over_vol_num.time), 0},       /* 0x02 */ 
	{ RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.low_vol_num.conut), 0},        /* 0x03 */
	{ RW, uint8_t,   6,            (void*)&(protect1_numb.low_vol_num.time), 0},         /* 0x04 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.disch_temp_num.conut), 0},    /* 0x05 */
	{ RW, uint8_t,   6,            (void*)&(protect1_numb.disch_temp_num.time), 0},      /* 0x06 */    
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.disdch_temp_num.conut), 0},   /* 0x07 */
	{ RW, uint8_t,   6,            (void*)&(protect1_numb.disdch_temp_num.time), 0},     /* 0x08 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.ch_ov_curr_num.conut), 0},    /* 0x09 */
	{ RW, uint8_t,   6,            (void*)&(protect1_numb.ch_ov_curr_num.time), 0},      /* 0x0A */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect1_numb.dch_over_curr_num.conut), 0}, /* 0x0B */
	{ RW, uint8_t,   6,            (void*)&(protect1_numb.dch_over_curr_num.time), 0},   /* 0x0C */
    { RW, uint32,  sizeof(UNS32),  (void*)&setFactroyModeDelay, 0},                      /* 0x0D */
};

/********* Index 200F *********/
static UNS8 highestSubIndex_200F = 12;    // number of subindex - 1
subindex Index200F[] = 
{
    { RW, uint8_t, sizeof(UNS8), (void*)&highestSubIndex_200F, 0},                   /* 0x00 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.short_num.conut), 0},      /* 0x01 */
	{ RW, uint8_t,   6,             (void*)&(protect2_numb.short_num.time), 0},      /* 0x02 */ 
	{ RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.mcu_ov_num.conut), 0},    /* 0x03 */
	{ RW, uint8_t,   6,            (void*)&(protect2_numb.mcu_ov_num.time), 0},      /* 0x04 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.mcu_uv_num.conut), 0},    /* 0x05 */
	{ RW, uint8_t,   6,            (void*)&(protect2_numb.mcu_uv_num.time), 0},      /* 0x06 */    
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.afe_ov_num.conut), 0},    /* 0x07 */
	{ RW, uint8_t,   6,            (void*)&(protect2_numb.afe_ov_num.time), 0},     /* 0x08 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.afe_uv_num.conut), 0},    /* 0x09 */
	{ RW, uint8_t,   6,            (void*)&(protect2_numb.afe_uv_num.time), 0},      /* 0x0A */
    { RW, uint32,  sizeof(UNS32),  (void*)&(protect2_numb.afe_ocd_num.conut), 0},   /* 0x0B */
	{ RW, uint8_t,   6,            (void*)&(protect2_numb.afe_ocd_num.time), 0},     /* 0x0C */    
};

/********* Index 2010 *********/
static UNS8 highestSubIndex_2010 = 12;    // number of subindex - 1
subindex Index2010[] = 
{
    { RW, uint8_t, sizeof(UNS8), (void*)&highestSubIndex_2010, 0},                   /* 0x00 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.afe_error_num.conut), 0},      /* 0x01 */
	{ RW, uint8_t,   6,             (void*)&(fault1_numb_record.afe_error_num.time), 0},      /* 0x02 */ 
	{ RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.afe_vol_s_num.conut), 0},    /* 0x03 */
	{ RW, uint8_t,   6,            (void*)&(fault1_numb_record.afe_vol_s_num.time), 0},      /* 0x04 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.afe_vol_d_num.conut), 0},    /* 0x05 */
	{ RW, uint8_t,   6,            (void*)&(fault1_numb_record.afe_vol_d_num.time), 0},      /* 0x06 */    
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.afe_vol_c_num.conut), 0},    /* 0x07 */
	{ RW, uint8_t,   6,            (void*)&(fault1_numb_record.afe_vol_c_num.time), 0},     /* 0x08 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.poweron_fault_num.conut), 0},    /* 0x09 */
	{ RW, uint8_t,   6,            (void*)&(fault1_numb_record.poweron_fault_num.time), 0},      /* 0x0A */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault1_numb_record.poweroff_fault_num.conut), 0},   /* 0x0B */
	{ RW, uint8_t,   6,            (void*)&(fault1_numb_record.poweroff_fault_num.time), 0},     /* 0x0C */    
};

/********* Index 2011 *********/
static UNS8 highestSubIndex_2011 = 12;    // number of subindex - 1
subindex Index2011[] = 
{
    { RW, uint8_t, sizeof(UNS8), (void*)&highestSubIndex_2011, 0},                           /* 0x00 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.eeprom_fault_num.conut), 0},      /* 0x01 */
	{ RW, uint8_t,   6,             (void*)&(fault2_numb_record.eeprom_fault_num.time), 0},      /* 0x02 */ 
	{ RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.can_fault_num.conut), 0},    /* 0x03 */
	{ RW, uint8_t,   6,            (void*)&(fault2_numb_record.can_fault_num.time), 0},      /* 0x04 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.chmos_fault_time.conut), 0},    /* 0x05 */
	{ RW, uint8_t,   6,            (void*)&(fault2_numb_record.chmos_fault_time.time), 0},      /* 0x06 */    
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.dismos_fault_time.conut), 0},    /* 0x07 */
	{ RW, uint8_t,   6,            (void*)&(fault2_numb_record.dismos_fault_time.time), 0},     /* 0x08 */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.mcu_reset_num.conut), 0},    /* 0x09 */
	{ RW, uint8_t,   6,            (void*)&(fault2_numb_record.mcu_reset_num.time), 0},      /* 0x0A */
    { RW, uint32,  sizeof(UNS32),  (void*)&(fault2_numb_record.system_run_time.conut), 0},   /* 0x0B */
	{ RW, uint8_t,   6,            (void*)&(fault2_numb_record.system_run_time.time), 0},     /* 0x0C */
    
};

/********* Index 2012 *********/
static UNS8 highestSubIndex_2012 = 32;    // number of subindex - 1
subindex Index2012[] = 
{
    { RW, uint8_t, sizeof(UNS8), (void*)&highestSubIndex_2012, 0},                  /* 0x00 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[0].conut), 0},     /* 0x01 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[0].time), 0},      /* 0x02 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[1].conut), 0},     /* 0x03 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[1].time), 0},      /* 0x04 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[2].conut), 0},     /* 0x05 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[2].time), 0},      /* 0x06 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[3].conut), 0},     /* 0x07 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[3].time), 0},      /* 0x08 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[4].conut), 0},     /* 0x09 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[4].time), 0},      /* 0x0a */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[5].conut), 0},     /* 0x0b */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[5].time), 0},      /* 0x0C */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[6].conut), 0},     /* 0x0D */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[6].time), 0},      /* 0x0E */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[7].conut), 0},     /* 0x0F */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[7].time), 0},      /* 0x10 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[8].conut), 0},     /* 0x11 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[8].time), 0},      /* 0x12 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[9].conut), 0},     /* 0x13 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[9].time), 0},      /* 0x14 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[10].conut), 0},     /* 0X15 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[10].time), 0},      /* 0X16 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[11].conut), 0},     /* 0X17 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[11].time), 0},      /* 0X18 */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[12].conut), 0},     /* 0X19 */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[12].time), 0},      /* 0x1A */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[13].conut), 0},     /* 0x1B */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[13].time), 0},      /* 0x1C */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[14].conut), 0},     /* 0x1D */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[14].time), 0},      /* 0x1E */
    { RO, uint32,  sizeof(UNS32),  (void*)&(cell_blance_numb.cellblance[15].conut), 0},     /* 0x1F */
	{ RO, uint8_t, 6,   (void*)&(cell_blance_numb.cellblance[15].time), 0},      /* 0x20 */
};



/********* Index 2013 *********/
static UNS8 highestSubIndex_2013 = 4;    // number of subindex - 1
subindex Index2013[] = 
{
    { RW, uint8_t, sizeof(UNS8), (void*)&highestSubIndex_2013, 0},                  /* 0x00 */
    { RO, uint32,  24,  (void*)&(last_two_update.updatetime[0].softversion), 0},    /* 0x01 */
	{ RO, uint8_t, 6,   (void*)&(last_two_update.updatetime[0].time), 0},           /* 0x02 */
	{ RO, uint32,  24,  (void*)&(last_two_update.updatetime[1].softversion), 0},    /* 0x03 */
	{ RO, uint8_t, 6,   (void*)&(last_two_update.updatetime[1].time), 0},           /* 0x04 */
};


/********* Index 2014 *********/
static UNS8 highestSubIndex_2014 = 32;    // number of subindex - 1
subindex Index2014[] = 
{
	{ RW, uint8_t, sizeof(UNS8),      (void*)&highestSubIndex_2014, 0},           /* 0x00 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.total_vol), 0},     /* 0x01 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_protect.current), 0},       /* 0x02 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.soc), 0},           /* 0x03 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[0]), 0},   /* 0x04 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[1]), 0},   /* 0x05 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[2]), 0},   /* 0x06 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[3]), 0},   /* 0x07 */ 
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[4]), 0},   /* 0x08 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[5]), 0},   /* 0x09 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[6]), 0},   /* 0x0A */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[7]), 0},   /* 0x0B */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[8]), 0},   /* 0x0C */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[9]), 0},   /* 0x0D */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[10]), 0},   /* 0x0E */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[11]), 0},   /* 0x0F */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[12]), 0},   /* 0x10 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[13]), 0},   /* 0x11 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[14]), 0},   /* 0x12 */
	{ RO, uint16,  sizeof(UNS16),     (void*)&(g_read_protect.cell_vol[15]), 0},   /* 0x13 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_protect.power_temp), 0},    /* 0x14 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_protect.ambient_temp), 0},  /* 0x15 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_protect.cell_temp[0]), 0},  /* 0x16 */
	{ RO, int16,   sizeof(INTEGER16), (void*)&(g_read_protect.cell_temp[1]), 0},  /* 0x17 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.vol_event), 0},  /* 0x18 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.cur_event), 0},  /* 0x19 */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.temp_event), 0},  /* 0x1A */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.ba_event), 0},  /* 0x1B */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.cap_event), 0},  /* 0x1C */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.other_event), 0},  /* 0x1D */
    { RO, uint8_t, sizeof(g_read_protect.write_time),      (void*)&(g_read_protect.write_time), 0},  /* 0x20 */
    { RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.new_event1), 0},  /* 0x1E */
	{ RO, uint8_t, sizeof(UNS8),      (void*)&(g_read_protect.new_event2), 0},  /* 0x1F */
	{ RO, uint8_t, sizeof(g_read_protect.write_time),      (void*)&(g_read_protect.write_time), 0},  /* 0x20 */
    { RO, uint32, sizeof(g_read_protect.number),      (void*)&(g_read_protect.number), 0},  /* 0x21 */
    
};
/********* Index 5000 *********/
static UNS8 highestSubIndex_5000 = 1;    // number of subindex - 1
subindex Index5000[] = 
{
	{ RW,               uint8_t,   sizeof(UNS8),    (void*)&highestSubIndex_5000, 0                   },     /* 0x00 */
	{ RW | TO_BE_SAVED, uint32,    sizeof(UNS32),   (void*)&RandomData,     handshake_crc_data },     /* 0x00 */
};
/********* Index 5001 *********/
static UNS8 highestSubIndex_5001 = 1;    // number of subindex - 1
subindex Index5001[] = 
{
	{ RW,               uint8_t,   sizeof(UNS8),    (void*)&highestSubIndex_5001, 0                   },     /* 0x00 */
};
/********* Index 5002 *********/
//static UNS8 highestSubIndex_5002 = 1;    // number of subindex - 1
subindex Index5002[] = 
{
	//{ RW,               uint8_t,   sizeof(UNS8),    (void*)&highestSubIndex_5002, 0                   },     /* 0x00 */
	{ RO, uint32,    sizeof(UNS32),   (void*)&EncryptedCRCdata,    0 },     /* 0x00 */
};

/********* Index 5003 *********/
//static UNS8 highestSubIndex_5003 = 1;    // number of subindex - 1
subindex Index5003[] = 
{
	{ RW, uint8_t,    sizeof(UNS8),   (void*)&HandshakeState,    0 },     /* 0x00 */
};

const indextable manufacturerProfileTable[] = 
{
	DeclareIndexTableEntry(Index2000), /* index 0x2000. voltage parameter mapped */
	DeclareIndexTableEntry(Index2001), /* index 0x2001. temperature parameter mapped */
	DeclareIndexTableEntry(Index2002), /* index 0x2002. current parameter mapped */
	DeclareIndexTableEntry(Index2003), /* index 0x2003. balance parameter mapped */
	DeclareIndexTableEntry(Index2004), /* index 0x2004. capcity parameter mapped */
	DeclareIndexTableEntry(Index2005), /* index 0x2005. other parameter mapped */
	DeclareIndexTableEntry(Index2006), /* index 0x2006. capcity parameter mapped */
	DeclareIndexTableEntry(Index2007), /* index 0x2007. other parameter mapped */
	DeclareIndexTableEntry(Index2008), /* index 0x2008. other parameter mapped */
	DeclareIndexTableEntry(Index2009), /* index 0x2009. other parameter mapped */
	DeclareIndexTableEntry(Index200A), /* index 0x200A. other parameter mapped */
    DeclareIndexTableEntry(Index200B), /* index 0x200B. other parameter mapped */
    DeclareIndexTableEntry(Index200C), /* index 0x200C. other parameter mapped */
    DeclareIndexTableEntry(Index200D), /* index 0x200D. other parameter mapped */
    DeclareIndexTableEntry(Index200E), /* index 0x200E. other parameter mapped */
    DeclareIndexTableEntry(Index200F), /* index 0x200F. other parameter mapped */
    DeclareIndexTableEntry(Index2010), /* index 0x2010. other parameter mapped */
    DeclareIndexTableEntry(Index2011), /* index 0x2011. other parameter mapped */
    DeclareIndexTableEntry(Index2012), /* index 0x2012. other parameter mapped */
    DeclareIndexTableEntry(Index2013), /* index 0x2013. other parameter mapped */
    DeclareIndexTableEntry(Index2014), /* index 0x2014. other parameter mapped */
};

#define MANUFACTURER_SPECIFIC_LAST_INDEX 0x2014

const indextable newadd1ProfileTable[] = 
{
    DeclareIndexTableEntry(Index5000), /* index 0x5000. other parameter mapped */
    DeclareIndexTableEntry(Index5001), /* index 0x5000. other parameter mapped */
    DeclareIndexTableEntry(Index5002), /* index 0x5002. other parameter mapped */
    DeclareIndexTableEntry(Index5003), /* index 0x5003. other parameter mapped */
};
/**************************************************************************/
/* The mapped variables at index 0x6000 - 0x61ff                          */
/**************************************************************************/
/* Computed by strMapVar */

/********* Index 6000 *********/
static UNS8 highestSubIndex_6000 = 1; // number of subindex - 1
subindex Index6000[] = 
{
    { RW, uint8_t, sizeof (UNS8),  (void*)&highestSubIndex_6000, 0}, 
    { RW, uint32,  sizeof (UNS32), (void*)&canopenErrNB, 0}
};

/********* Index 6001 *********/
static UNS8 highestSubIndex_6001 = 1; // number of subindex - 1
subindex Index6001[] = 
{
    { RW, uint8_t, sizeof (UNS8),  (void*)&highestSubIndex_6001, 0}, 
    { RW, uint32,  sizeof (UNS32), (void*)&canopenErrVAL, 0}
};

#define DIGITAL_INPUT_LAST_TABLE_INDEX 0x6001
const indextable digitalInputTable[] = 
{
	DeclareIndexTableEntry(Index6000), 
	DeclareIndexTableEntry(Index6001), 
};

/**************************************************************************/
/* The mapped variables at index 0x6200 - 0x9fff                          */
/**************************************************************************/
/* Computed by strMapVar */
#define DIGITAL_OUTPUT_LAST_TABLE_INDEX 0x0
const indextable digitalOutputTable[] = 
{
	{NULL, 0}
};

/**************************************************************************/
/* Varia used by other files                                              */
/**************************************************************************/
/* Computed by strVaria2 */
UNS8 count_sync[MAX_COUNT_OF_PDO_TRANSMIT];

#define NB_OF_HEARTBEAT_PRODUCERS    128

#define COMM_PROFILE_LAST            0x1018

/* Should not be modified */const dict_cste dict_cstes = {	
	COMM_PROFILE_LAST, 	
	INDEX_LAST_SDO_SERVER, 
	DEF_MAX_COUNT_OF_SDO_SERVER, 
	INDEX_LAST_SDO_CLIENT, 
	DEF_MAX_COUNT_OF_SDO_CLIENT, 
	INDEX_LAST_PDO_RECEIVE, 
	MAX_COUNT_OF_PDO_RECEIVE, 
	INDEX_LAST_PDO_TRANSMIT, 
	MAX_COUNT_OF_PDO_TRANSMIT, 
	INDEX_LAST_PDO_MAPPING_RECEIVE,	
	INDEX_LAST_PDO_MAPPING_TRANSMIT, 
	MANUFACTURER_SPECIFIC_LAST_INDEX, 
	DIGITAL_INPUT_LAST_TABLE_INDEX, 
	DIGITAL_OUTPUT_LAST_TABLE_INDEX, 
	NB_OF_HEARTBEAT_PRODUCERS
};


