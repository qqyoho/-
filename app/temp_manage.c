#include "temp_manage.h"
#include <math.h>
#include "adc_sampling.h"
#include "parameter.h"
#include "current_manage.h"
#include "system_control.h"
#include "peak_record.h"
#include "soc.h"
#include "protect_record.h"
#include "run_record.h"
#include "vol_manage.h"
#include "afe_app.h"

#define MAX_POWER_TEMP_NUM            (20)

#define MS_TO_SECOND                  (10)

#define MAX_RECOVER_TIME              ((30) * (60) * MS_TO_SECOND)/* 30分钟 */

#define MIN_TEMPERATURE_VALUE         (-1000)
#define MAX_TEMPERATURE_VALUE         (1250)

#define POWER_TEMP_RES   10   /* 与功率温感串联的限流电阻 */
#define POWER_NTC_B      3450 /* 功率温感B值 */

#define AMBIENT_TEMP_RES 10   /* 与环境温感串联的限流电阻 */
#define AMBIENT_NTC_B    3435 /* 环境温感B值 */

#define CELL_TEMP_RES    10   /* 与电芯温感串联的限流电阻 */
#define CELL_NTC_B       3450 /* 电芯温感B值 */

/* 温度阻值对照表 */
typedef struct _R_B3950_T_STRUCT_
{
	int16_t temp; /* 单位0.1℃ */
	int32_t r_value; /* 阻值，单位欧姆 */
}t_rt_st;

const static t_rt_st g_xinjian_rt_table[] = 
{
    {-500, 366428}, {-490, 344575}, {-480, 324179}, {-470, 305134}, 
    {-460, 287341}, {-450, 270709}, {-440, 255156}, {-430, 240603}, 
    {-420, 226981}, {-410, 214223}, {-400, 202269}, {-390, 191063}, 
    {-380, 180554}, {-370, 170694}, {-360, 161438}, {-350, 152746}, 
    {-340, 144580}, {-330, 136905}, {-320, 129687}, {-310, 122898}, 
    {-300, 116508}, {-290, 110493}, {-280, 104827}, {-270, 99488},  
    {-260, 94455},  {-250, 89710},  {-240, 85233},  {-230, 81008},  
    {-220, 77019},  {-210, 73252},  {-200, 69693},  {-190, 66329}, 
    {-180, 63148},  {-170, 60140},  {-160, 57293},  {-150, 54599}, 
    {-140, 52049},  {-130, 49633},  {-120, 47343},  {-110, 45174}, 
    {-100, 43117},  { -90, 41166},  { -80, 39315},  { -70, 37558}, 
    { -60, 35891},  { -50, 34307},  { -40, 32802},  { -30, 31373}, 
    { -20, 30014},  { -10, 28722},  {   0, 27493},  {  10, 26324}, 
    {  20, 25211},  {  30, 24152},  {  40, 23144},  {  50, 22183}, 
    {  60, 21268},  {  70, 20395},  {  80, 19564},  {  90, 18771}, 
    { 100, 18015},  { 110, 17293},  { 120, 16604},  { 130, 15947}, 
    { 140, 15319},  { 150, 14720},  { 160, 14147},  { 170, 13600}, 
    { 180, 13077},  { 190, 12577},  { 200, 12098},  { 210, 11641}, 
    { 220, 11203},  { 230, 10784},  { 240, 10383},  { 250, 10000}, 
    { 260, 9632},   { 270, 9280},   { 280, 8942},   { 290, 8619}, 
    { 300, 8309},   { 310, 8012},   { 320, 7727},   { 330, 7454}, 
    { 340, 7191},   { 350, 6940},   { 360, 6698},   { 370, 6466}, 
    { 380, 6244},   { 390, 6030},   { 400, 5825},   { 410, 5627}, 
    { 420, 5438},   { 430, 5255},   { 440, 5080},   { 450, 4911}, 
    { 460, 4749},   { 470, 4593},   { 480, 4443},   { 490, 4299}, 
    { 500, 4160},   { 510, 4026},   { 520, 3898},   { 530, 3773}, 
    { 540, 3654},   { 550, 3539},   { 560, 3428},   { 570, 3321}, 
    { 580, 3218},   { 590, 3119},   { 600, 3023},   { 610, 2931}, 
    { 620, 2841},   { 630, 2755},   { 640, 2672},   { 650, 2592}, 
    { 660, 2515},   { 670, 2441},   { 680, 2369},   { 690, 2299}, 
    { 700, 2232},   { 710, 2167},   { 720, 2104},   { 730, 2044}, 
    { 740, 1985},   { 750, 1928},   { 760, 1874},   { 770, 1821}, 
    { 780, 1769},   { 790, 1720},   { 800, 1672},   { 810, 1626}, 
    { 820, 1581},   { 830, 1537},   { 840, 1495},   { 850, 1455}, 
    { 860, 1415},   { 870, 1377},   { 880, 1340},   { 890, 1305}, 
    { 900, 1270},   { 910, 1236},   { 920, 1204},   { 930, 1172}, 
    { 940, 1142},   { 950, 1112},   { 960, 1083},   { 970, 1056}, 
    { 980, 1029},   { 990, 1002},   {1000, 977},    {1010, 952}, 
    {1020, 928},    {1030, 905},    {1040, 883},    {1050, 861}, 
    {1060, 839},    {1070, 819},    {1080, 799},    {1090, 779}, 
    {1100, 760},    {1110, 742},    {1120, 724},    {1130, 707}, 
    {1140, 690},    {1150, 674},    {1160, 658},    {1170, 642}, 
    {1180, 627},    {1190, 613},    {1200, 599},    {1210, 585}, 
    {1220, 571},    {1230, 558},    {1240, 546},    {1250, 533}
//    , 
//    {126, 0.5216},   {127, 0.5098},   {128, 0.4984},   {129, 0.4873}, 
//    {130, 0.4765},   {131, 0.4660},   {132, 0.4558},   {133, 0.4458}, 
//    {134, 0.4361},   {135, 0.4266},   {136, 0.4174},   {137, 0.4084}, 
//    {138, 0.3997},   {139, 0.3911},   {140, 0.3828},   {141, 0.3747}, 
//    {142, 0.3669},   {143, 0.3592},   {144, 0.3517},   {145, 0.3444}, 
//    {146, 0.3372},   {147, 0.3303},   {148, 0.3235},   {149, 0.3169}, 
//    {150, 0.3104},   {151, 0.3042},   {152, 0.2980},   {153, 0.2920}, 
//    {154, 0.2862},   {155, 0.2805},   {156, 0.2749},   {157, 0.2695}, 
//    {158, 0.2642},   {159, 0.2590},   {160, 0.2540},   {161, 0.2490}, 
//    {162, 0.2442},   {163, 0.2395},   {164, 0.2349},   {165, 0.2304}, 
//    {166, 0.2260},   {167, 0.2218},   {168, 0.2176},   {169, 0.2135}, 
//    {170, 0.2095},   {171, 0.2056},   {172, 0.2018},   {173, 0.1980}, 
//    {174, 0.1944},   {175, 0.1908},   {176, 0.1874},   {177, 0.1839}, 
//    {178, 0.1806},   {179, 0.1774},   {180, 0.1742},   {181, 0.1711}, 
//    {182, 0.1680},   {183, 0.1650},   {184, 0.1621},   {185, 0.1593}, 
//    {186, 0.1565},   {187, 0.1538},   {188, 0.1511},   {189, 0.1485}, 
//    {190, 0.1459},   {191, 0.1434},   {192, 0.1410},   {193, 0.1386}, 
//    {194, 0.1362},   {195, 0.1339},   {196, 0.1317},   {197, 0.1295}, 
//    {198, 0.1273},   {199, 0.1252},   {200, 0.1231},   {201, 0.1211}, 
//    {202, 0.1191},   {203, 0.1172},   {204, 0.1153},   {205, 0.1134}, 
//    {206, 0.1116},   {207, 0.1098},   {208, 0.1080},   {209, 0.1063}, 
//    {210, 0.1046},   {211, 0.1029},   {212, 0.1013},   {213, 0.0997}, 
//    {214, 0.0982},   {215, 0.0967},   {216, 0.0952},   {217, 0.0937}, 
//    {218, 0.0922},   {219, 0.0908},   {220, 0.0895},   {221, 0.0881}, 
//    {222, 0.0868},   {223, 0.0855},   {224, 0.0842},   {225, 0.0829}, 
//    {226, 0.0817},   {227, 0.0805},   {228, 0.0793},   {229, 0.0781}, 
//    {230, 0.0770},   {231, 0.0759},   {232, 0.0748},   {233, 0.0737}, 
//    {234, 0.0726},   {235, 0.0716},   {236, 0.0706},   {237, 0.0695}, 
//    {238, 0.0686},   {239, 0.0676},   {240, 0.0666},   {241, 0.0657}, 
//    {242, 0.0648},   {243, 0.0639},   {244, 0.0630},   {245, 0.0621}, 
//    {246, 0.0613},   {247, 0.0605},   {248, 0.0596},   {249, 0.0588}, 
//    {250, 0.0580},   {251, 0.0573},   {252, 0.0565},   {253, 0.0557}, 
//    {254, 0.0550},   {255, 0.0543},   {256, 0.0536},   {257, 0.0529}, 
//    {258, 0.0522},   {259, 0.0515},   {260, 0.0508},   {261, 0.0502}, 
//    {262, 0.0495},   {263, 0.0489},   {264, 0.0483},   {265, 0.0477}, 
//    {266, 0.0471},   {267, 0.0465},   {268, 0.0459},   {269, 0.0453}, 
//    {270, 0.0447},   {271, 0.0442},   {272, 0.0437},   {273, 0.0431}, 
//    {274, 0.0426},   {275, 0.0421},   {276, 0.0416},   {277, 0.0411}, 
//    {278, 0.0406},   {279, 0.0401},   {280, 0.0396},   {281, 0.0391}, 
//    {282, 0.0387},   {283, 0.0382},   {284, 0.0378},   {285, 0.0373}, 
//    {286, 0.0369},   {287, 0.0365},   {288, 0.0361},   {289, 0.0356}, 
//    {290, 0.0352},   {291, 0.0348},   {292, 0.0344},   {293, 0.0340}, 
//    {294, 0.0337},   {295, 0.0333},   {296, 0.0329},   {297, 0.0326}, 
//    {298, 0.0322},   {299, 0.0318},   {300, 0.0315}
};
/*
const static t_rt_st g_sl3950_rt_table[] = 
{
    {-25,  86.390},  {-24,  82.2327},  {-23,  78.2976},  {-22,  74.5717},  {-21,  71.0428},  
    {-20,  67.6996}, {-19,  64.5314},  {-18,  61.5282},  {-17,  58.6807},  {-16,  55.9799},  
    {-15,  53.4177}, {-14,  50.9863},  {-13,  48.6783},  {-12,  46.487},   {-11,  44.4058},  
    {-10,  42.4287}, {-9,   40.550},   {-8,   38.7643},  {-7,   37.0666},  {-6,   35.4521},  
    {-5,   33.9164}, {-4,   32.4553},  {-3,   31.0647},  {-2,   29.7409},  {-1,   28.4804},  
    {0,    27.280},  {1,    26.1363},  {2,    25.0465},  {3,    24.0078},  {4,    23.0176},  	
    {5,    22.0734}, {6,    21.1727},  {7,    20.3135},  {8,    19.4936},  {9,    18.7109},  
    {10,   17.9638}, {11,   17.2503},  {12,   16.5688},  {13,   15.9177},  {14,   15.2956},  
    {15,   14.701},  {16,   14.1325},  {17,   13.5889},  {18,   13.069},   {19,   12.5716},  
    {20,   12.0957}, {21,   11.6402},  {22,   11.2042},  {23,   10.7868},  {24,   10.387},  
    {25,   10.000},  {26,   9.6372},   {27,   9.28561},  {28,   8.94863},  {29,   8.62557},  
    {30,   8.313},   {31,   8.01872},  {32,   7.73374},  {33,   7.46031},  {34,   7.19792},  
    {35,   6.94607}, {36,   6.7043},   {37,   6.47214},  {38,   6.24918},  {39,   6.03501},  
    {40,   5.82924}, {41,   5.63151},  {42,   5.44147},  {43,   5.25878},  {44,   5.08312},  
    {45,   4.9142},  {46,   4.75173},  {47,   4.59543},  {48,   4.44504},  {49,   4.30031},  
    {50,   4.161},   {51,   4.02688},  {52,   3.89774},  {53,   3.77338},  {54,   3.65359},  
    {55,   3.53818}, {56,   3.42698},  {57,   3.31982},  {58,   3.21652},  {59,   3.11694},  
    {60,   3.02093}, {61,   2.92833},  {62,   2.83902},  {63,   2.75286},  {64,   2.66973},  
    {65,   2.58951}, {66,   2.51209},  {67,   2.43735},  {68,   2.36519},  {69,   2.29551},  
    {70,   2.22822}, {71,   2.16322},  {72,   2.10042},  {73,   2.03974},  {74,   1.98111},  
    {75,   1.92443}, {76,   1.86965},  {77,   1.81668},  {78,   1.76546},  {79,   1.71593},  
    {80,   1.66802}, {81,   1.62167},  {82,   1.57683},  {83,   1.53343},  {84,   1.49143},  
    {85,   1.451},   {86,   1.41142},  {87,   1.37332},  {88,   1.33642},  {89,   1.30068},  
    {90,   1.26606}, {91,   1.23253},  {92,   1.20003},  {93,   1.16854},  {94,   1.13803},  
    {95,   1.10844}, {96,   1.07977},  {97,   1.05196},  {98,   1.025},    {99,   0.998861},  
    {100,  0.9735},  {101,  0.948897}, {102,  0.925028}, {103,  0.901867}, {104,  0.87939},  
    {105,  0.857575},{106,  0.836399}, {107,  0.815842}, {108,  0.795881}, {109,  0.776499},  
    {110,  0.757675},{111,  0.739392}, {112,  0.721631}, {113,  0.704376}, {114,  0.68761},  
    {115,  0.671317},{116,  0.655483}, {117,  0.640092}, {118,  0.625131}, {119,  0.610585},  
    {120,  0.596441},{121,  0.582688}, {122,  0.569311}, {123,  0.556301}, {124,  0.543644},  
    {125,  0.531331},  
};
*/

const static t_rt_st g_ambient_rt_table[] = /* NTC 103F 3450 10K */
{
	{-400, 188500},  {-350, 144290}, 
	{-300, 111330},  {-250,  86560}, 
	{-200,  67790},  {-150,  53460},  
	{-100,  42450},  {- 50,  33930}, 
	{   0,  27280},  {  50,  22070}, 
	{ 100,  17960},  { 150,  14700}, 
	{ 200,  12090},  { 250,  10000}, 
	{ 300,   8310},  { 350,   6940}, 
	{ 400,   5830},  { 450,   4910}, 
	{ 500,   4160},  { 550,   3540}, 
	{ 600,   3020},  { 650,   2590}, 
	{ 700,   2230},  { 750,   1920}, 
	{ 800,   1670},  { 850,   1450}, 
	{ 900,   1270},  { 950,   1110}, 
	{1000,   975},   {1050,   860}, 
	{1100,   760},   {1150,   674}, 
	{1200,   599},   {1250,   534} 
};

/* 温度子状态 */
typedef enum _E_TEMP_CHILD_ITEM_
{
	E_CHILD_WAIT, /* 等待进入 */
	E_CHILD_IDLE, /* 等待进入 */
}e_temp_child_item;

/* 电芯充电温度状态 */
static e_item_status g_cht_status[MAX_CELL_TEMP_NUM];
static e_temp_child_item g_cht_child[MAX_CELL_TEMP_NUM];
static volatile uint16_t t_cht_timer[MAX_CELL_TEMP_NUM];

/* 电芯放电温度状态 */
static e_item_status g_dcht_status[MAX_CELL_TEMP_NUM];
static e_temp_child_item g_dcht_child[MAX_CELL_TEMP_NUM];
static volatile uint16_t t_dcht_timer[MAX_CELL_TEMP_NUM];

/* 环境温度状态 */
static e_item_status g_envt_status;
static e_temp_child_item g_evnt_child;
static volatile uint16_t t_envt_timer;

/* 功率温度状态 */
static e_item_status g_powt_status;
static e_temp_child_item g_powt_child;
static volatile uint16_t t_powt_timer;

/* 电芯充电温度对外标识 */
static e_item_status g_ch_temp_flag;
/* 电芯放电温度对外标识 */
static e_item_status g_dch_temp_flag;
/* 功率温度对外标识 */
static e_item_status powtTempFlag;
/* 环境温度对外标识 */
static e_item_status envtTempFlag;

/* 温度模块对外标识 */
static e_temp_status g_temp_flag;

uint8_t g_report_cell_temp;
/* 温差告警标识 */
static uint8_t tempDifferentFlag;
static volatile uint16_t tempDifferentDelay;
/*=============================================================
 * 函数名称：t_timer_run
 * 函数功能：温度逻辑模块S定时器
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
void t_timer_run(void)
{
	uint8_t i;

	if (t_powt_timer) t_powt_timer --;
	if (t_envt_timer) t_envt_timer --;
	for (i = 0; i < MAX_CELL_TEMP_NUM; i ++)
    {
		if (t_cht_timer[i]) t_cht_timer[i] --;
		if (t_dcht_timer[i]) t_dcht_timer[i] --;
    }
    if( tempDifferentDelay ) tempDifferentDelay--;
}

/*=============================================================
 * 函数名称：temperature_init
 * 函数功能：温度逻辑处理模块初始
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void temperature_init(void)
{
	uint8_t i;
	for (i = 0; i < MAX_CELL_TEMP_NUM; i ++){
		g_cht_status[i] = E_TEMP_IDLE;
		g_cht_child[i] = E_CHILD_IDLE;
		t_cht_timer[i] = 0;
		
		g_dcht_status[i] = E_TEMP_IDLE;
		g_dcht_child[i] = E_CHILD_IDLE;
		t_dcht_timer[i] = 0;
        
        g_run_sys_data.cell_temp[i] = 250;
	}
	g_envt_status = E_TEMP_IDLE;
	g_evnt_child = E_CHILD_IDLE;
	t_envt_timer = 0;
    g_run_sys_data.ambient_temp = 250;

	g_powt_status = E_TEMP_IDLE;
	g_powt_child = E_CHILD_IDLE;
	t_powt_timer = 0;
    g_run_sys_data.power_temp = 250;

	g_temp_flag = E_TEMP_IDLE_STATUS;
    
    set_temp_buf((uint32_t)get_cell_temerature);
    tempDifferentFlag = 0;
    tempDifferentDelay = 0;
}

/*=============================================================
 * 函数名称：get_temperature_status
 * 函数功能：获取当前温度状态
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           当前温度状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-27        戴辉发     	创建
==============================================================*/
e_temp_status get_temperature_status(void)
{
	return g_temp_flag;
}

/*=============================================================
 * 函数名称：get_temperature
 * 函数功能：查表获取温度值
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *          温度值, 单位为0.1℃
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2019-10-28        戴辉发     	创建
==============================================================*/
static int16_t get_temperature(int32_t r_value, const t_rt_st *st_buf, uint16_t st_num)
{
	int16_t temp;

	/* 通过查表找到对应阻值区间 */
    if (r_value > st_buf[0].r_value)
    {
        temp = -600 ;
    }
    else if (r_value < st_buf[st_num].r_value)
    {
        temp = 1250;
    }
    else
    {
        uint16_t num, last_num, middle;
        uint8_t flags = 0;

        last_num = st_num;
        num = 0;
        while (1)
        {
            middle = (last_num + num) / 2;
            if (r_value > st_buf[middle].r_value)
            {
                last_num = middle;
            }
            else if (r_value < st_buf[middle].r_value)
            {
                num = middle;
            }
            else
            /* 查找到当前位置刚好 */
            {
                temp = st_buf[middle].temp;
                flags = 1;
                break;
            }
            
            if (last_num == (num + 1))
            /* 找到对应位置 */
            {
                break;
            }
        }
        
        if (0 == flags)
        {
            const t_rt_st *cur_rt;
            const t_rt_st *next_rt;

            cur_rt = &st_buf[num];
            next_rt = &st_buf[last_num];

            temp = (int16_t)((10.0 * (r_value - cur_rt->r_value) * (next_rt->temp - cur_rt->temp)) / 10 / (next_rt->r_value - cur_rt->r_value));
            temp = cur_rt->temp + temp;
        }
    }

	return temp;
}

/*=============================================================
 * 函数名称：get_power_temerature
 * 函数功能：获取功率温度
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：
 *           当前功率温度, 单位为0.1℃
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-07        戴辉发     	创建
==============================================================*/
static int16_t get_power_temerature(int32_t ad_value)
{
    uint32_t r_value = get_vcc3v3_vaule();
    if(ad_value >= r_value)
      ad_value = r_value-1;
	/* 通过当前AD值获取当前热敏电阻对应阻值 */
	r_value = 10000 * ad_value / (r_value - ad_value);

    return get_temperature(r_value, g_ambient_rt_table, (sizeof(g_ambient_rt_table) / sizeof(g_ambient_rt_table[0])) - 1);
}

/*=============================================================
 * 函数名称：get_ambient_temerature
 * 函数功能：获取环境温度值
 * 参数个数：3
 * 参数描述：
 *           [IN]    r_value     NTC阻值
 *           [IN]    st_buf      阻值温度表
 *           [IN]    st_num      阻值温度表个数，比实际表小于1
 * 返 回 值：
 *          温度值, 单位为0.1℃
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2019-10-28        戴辉发     	创建
==============================================================*/
static int16_t get_ambient_temerature(int32_t ad_value)
{  
    uint32_t r_value = get_vcc3v3_vaule();
    if( ad_value >= r_value ) 
      ad_value = r_value-1;
	/* 通过当前AD值获取当前热敏电阻对应阻值 */ 
	r_value = 10000 * ad_value / (r_value - ad_value);
    return get_temperature(r_value, g_ambient_rt_table, sizeof(g_ambient_rt_table) / sizeof(g_ambient_rt_table[0]) -1 );
}

/*=============================================================
 * 函数名称：get_cell_temerature
 * 函数功能：获取电芯温度
 * 参数个数：1
 * 参数描述：
 * 
 * 返 回 值：
 *           当前电芯1温度, 单位为0.1℃
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-07        戴辉发     	创建
==============================================================*/
void get_cell_temerature(int32_t r_value, uint8_t no)
{
    g_run_sys_data.cell_temp[no] = get_temperature(r_value, g_xinjian_rt_table, sizeof(g_xinjian_rt_table) / sizeof(g_xinjian_rt_table[0])-1);
}
/*=============================================================
 * 函数名称：GetCellTemerature
 * 函数功能：获取电芯温度
 * 参数个数：1
 * 参数描述：
 * 
 * 返 回 值：
 *           当前电芯1温度, 单位为0.1℃
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2021-11-17        liyong    	 创建
==============================================================*/
int16_t GetCellTemerature(int32_t r_value)
{
    int16_t temp;
    temp = get_temperature(r_value, g_xinjian_rt_table, sizeof(g_xinjian_rt_table) / sizeof(g_xinjian_rt_table[0])-1);
    return temp;
}
/*=============================================================
 * 函数名称：get_report_temp
 * 函数功能：获取向上报告的温度
 * 参数个数：1
 * 参数描述：
 * 
 * 返 回 值：
 *           当前电芯1温度, 单位为℃，基准点为100
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-07-07        戴辉发      创建
 * 2021-11-03        戴辉发      修改上报逻辑，按照严重程度优先原则
==============================================================*/
uint8_t get_report_temp(void)
{
    uint8_t rep_temp;
    uint8_t i;
	int16_t max_temp = -3000, min_temp = 3000;

	for (i = 0; i < MAX_CELL_TEMP_NUM; i++)
	{
		if (max_temp < get_cell_temp(i))
		{
			max_temp = get_cell_temp(i);
		}
		if (min_temp > get_cell_temp(i))
		{
			min_temp = get_cell_temp(i);
		}
	}
	if (( max_temp >= 400 ) && ( min_temp >= 400))
	{
        rep_temp = ((max_temp / 10) + 100) & 0xff;
	}
    else if (max_temp >= 600)
    {
        rep_temp = ((max_temp / 10) + 100) & 0xff;
    }
    else
    {
        rep_temp = ((min_temp / 10) + 100) & 0xff;
    }

	return rep_temp;
}

/*=============================================================
 * 函数名称：temperature_alarm_process
 * 函数功能：温度逻辑处理
 * 参数个数：3
 * 参数描述：
 *           [IN]    temp        当前温度
 *           [IN/OUT]pt_timer    对应定时器
 *           [IN/OUT]temp_status 温度状态
 *           [IN/OUT]child_status温度子状态
 *           [IN]    temp_type   温度类型,
 *                   0:电芯充电温度
 *                   1:电芯放电温度
 *                   2:功率温度
 *                   3:环境温度
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-09-09        戴辉发     	创建
==============================================================*/
static void temperature_alarm_process(int16_t temp, uint16_t *pt_timer, e_item_status *temp_status, e_temp_child_item *child_status, uint8_t temp_type)
{
	int16_t low_alarm, high_alarm;
	int16_t low_alarm_recover, high_alarm_recover;
	int16_t low_protect, high_protect;
	int16_t low_recover, high_recover;
	int16_t lowTemp = temp;
	int16_t highTemp = temp;
	uint16_t alarm_delay;
	uint16_t protect_delay;
	uint16_t alarm_recover_delay;
	uint16_t protect_recover_delay;

	switch(temp_type)
	{
	case 0: 
		low_alarm = get_ch_cell_temp_low_alarm_level();
		high_alarm = get_ch_cell_temp_high_alarm_level();
		low_protect = get_ch_cell_temp_low_protect();
		high_protect = get_ch_cell_temp_high_protect();
		low_recover = get_ch_cell_temp_low_recover();
		high_recover = get_ch_cell_temp_high_recover();
		alarm_delay = get_cell_tempprotect_delay() * MS_TO_SECOND;
		protect_delay = alarm_delay;
		break;
	case 1: 
		low_alarm = get_dch_cell_temp_low_alarm_level();
		high_alarm = get_dch_cell_temp_high_alarm_level();
		low_protect = get_dch_cell_temp_low_protect();
		high_protect = get_dch_cell_temp_high_protect();
		low_recover = get_dch_cell_temp_low_recover();
		high_recover = get_dch_cell_temp_high_recover();
		alarm_delay = get_cell_tempprotect_delay() * MS_TO_SECOND;
		protect_delay = alarm_delay;
		break;
	case 2: 
		low_alarm = get_power_temp_low_alarm_level();
		high_alarm = get_power_temp_high_alarm_level();
		low_protect = get_power_temp_low_protect();
		high_protect = get_power_temp_high_protect();
		low_recover = get_power_temp_low_recover();
		high_recover = get_power_temp_high_recover();
		alarm_delay = get_power_temp_protect_delay() * MS_TO_SECOND;
		protect_delay = alarm_delay;
		break;
	default: 
		low_alarm = get_enviror_temp_low_alarm_level();
		high_alarm = get_enviror_temp_high_alarm_level();
		low_protect = get_enviror_temp_low_protect();
		high_protect = get_enviror_temp_high_protect();
		low_recover = get_enviror_temp_low_recover();
		high_recover = get_enviror_temp_high_recover();
		alarm_delay = get_enviror_temp_protect_delay() * MS_TO_SECOND;
		protect_delay = alarm_delay;
		break;
	}
	alarm_recover_delay = alarm_delay;
	protect_recover_delay = protect_delay;
	
	
	low_alarm_recover = low_alarm + 30;
	high_alarm_recover = high_alarm - 30;
#if defined(TIANFENG) && defined(BAT_8S)
	alarm_delay = 50;
	alarm_recover_delay = 50;
	protect_delay = 30;
	protect_recover_delay = 30;
	switch(temp_type)
	{
	case 0:
		low_alarm_recover = 150;
		high_alarm_recover = 400;
		break;
	case 1:
		low_alarm_recover = 0;
		high_alarm_recover = 500;
		break;
	default:
		low_alarm_recover = -350;
		high_alarm_recover = 750;
		break;
	}
	if(temp_type < 2)
	{
		lowTemp = get_min_cell_temp();
		highTemp = get_max_cell_temp();
	}
#endif

	switch(*temp_status)
    {
    case E_TEMP_LOW_PROTECT:
        if (*child_status == E_CHILD_WAIT)
        
        {
            if (lowTemp <= low_protect)
            
            {
                if (0 == *pt_timer)
                
                {
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = protect_recover_delay;
                }
            }
            else
            
            {
                *temp_status = E_TEMP_LOW_ALARM;
                *child_status = E_CHILD_IDLE;
                *pt_timer = alarm_recover_delay;
            }
        }
        else
        
        {
            if (lowTemp > low_recover)
            {
                if (0 == *pt_timer)
                {
                    *temp_status = E_TEMP_LOW_ALARM;
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = alarm_recover_delay;
                }
            }
            else
            {
                *pt_timer = protect_recover_delay;
            }
        }
        break;
    case E_TEMP_LOW_ALARM:
        if (*child_status == E_CHILD_WAIT)
        
        {
            if (lowTemp <= low_protect)
            
            {
                *temp_status = E_TEMP_LOW_PROTECT;
                *pt_timer = protect_delay;
            }
            else if (lowTemp <= low_alarm)
            {
                if(0 == *pt_timer)
                
                {
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = alarm_recover_delay;
                }
            }
            else
            {
                *temp_status = E_TEMP_IDLE;
                *child_status = E_CHILD_IDLE;
                *pt_timer = 0;
            }
        }
        else
        
        {
            if (lowTemp <= low_protect)
            
            {
                *temp_status = E_TEMP_LOW_PROTECT;
                *child_status = E_CHILD_WAIT;
                *pt_timer = protect_delay;
            }
            else if (lowTemp > low_alarm_recover)
            
            {
                if (0 == *pt_timer)
                {
                    *temp_status = E_TEMP_IDLE;
                }
            }
            else
            {
                *pt_timer = alarm_recover_delay;
            }
        }
        break;
    case E_TEMP_IDLE:
        if (lowTemp <= low_alarm)
        
        {
            *temp_status = E_TEMP_LOW_ALARM;
            *child_status = E_CHILD_WAIT;
            *pt_timer = alarm_delay;
        }
        else if (highTemp >= high_alarm)
        
        {
            *temp_status = E_TEMP_HIGH_ALARM;
            *child_status = E_CHILD_WAIT;
            *pt_timer = alarm_delay;
        }
        break;
    case E_TEMP_HIGH_ALARM:
        if (*child_status == E_CHILD_WAIT)
        
        {
            if (highTemp >= high_protect)
            {
                *temp_status = E_TEMP_HIGH_PROTECT;      
                *pt_timer = protect_delay;
            }
            else if (highTemp >= high_alarm)
            
            {
                if (0 == *pt_timer)
                
                {
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = alarm_recover_delay;
                }
            }
            else
            {
                *temp_status = E_TEMP_IDLE;
                *child_status = E_CHILD_WAIT;
                *pt_timer = 0;
            }
        }
        else
        
        {
            if (highTemp >= high_protect)
            {
                *temp_status = E_TEMP_HIGH_PROTECT;
                *child_status = E_CHILD_WAIT;
                *pt_timer = protect_delay;
            }
            else if (highTemp < high_alarm_recover)
            
            {
                if (0 == *pt_timer)
                {
                    *temp_status = E_TEMP_IDLE;
                }
            }
            else
            {
                *pt_timer = alarm_recover_delay;
            }
        }
        break;
    case E_TEMP_HIGH_PROTECT:
        if (*child_status == E_CHILD_WAIT)
        
        {
            if (highTemp >= high_protect)
            {
                if (0 == *pt_timer)
                
                {
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = protect_recover_delay;
                }
            }
            else
            {
                *temp_status = E_TEMP_HIGH_ALARM;
                *child_status = E_CHILD_IDLE;
                *pt_timer = alarm_recover_delay;
            }
        }
        else
        
        {
            if (highTemp < high_recover)
            
            {
                if (0 == *pt_timer)
                {
                    *temp_status = E_TEMP_HIGH_ALARM;
                    *child_status = E_CHILD_IDLE;
                    *pt_timer = alarm_recover_delay;
                }
            }
            else
            {
                *pt_timer = protect_recover_delay;
            }
        }
        break;
    
    default:
        *temp_status = E_TEMP_IDLE;
        break;
    }
}

/*=============================================================
 * 函数名称：temperature_get_data
 * 函数功能：温度逻辑处理模块温度采集温度模块
 * 参数个数：2
 * 参数描述：
 *          [IN]     ad_value    AD值
 *          [IN]     cur_temp    当前温度状态
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
uint8_t temperature_get_data(int32_t ad_value, e_ad_ch_type *cur_temp)
{
    uint8_t ret = 0;

	switch(*cur_temp)
	{
	case CH_AMBIENT_TEMP_ADS:
		/* 获取环境温度 */
		g_run_sys_data.ambient_temp = get_ambient_temerature(ad_value);
        *cur_temp = CH_POWER_TEMP_ADS;
		break;
	case CH_POWER_TEMP_ADS:
		/* 获取功率温度 */
		g_run_sys_data.power_temp = get_power_temerature(ad_value);
        *cur_temp = CH_AMBIENT_TEMP_ADS;
        ret = 1;
		break;
	default:
        *cur_temp = CH_AMBIENT_TEMP_ADS; 
		break;
	}

    return ret;
}

/*=============================================================
 * 函数名称：CellTempDeal
 * 函数功能：电芯温度标志处理，归一化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人        修改类型
 * 2021-11-03        戴辉发        创建
==============================================================*/
static void CellTempDeal(void)
{
	uint8_t i;
    uint8_t dchFlag;
    uint8_t chFlag;
	uint16_t t_timer;

	/* 电芯温度充电逻辑判断 */
	for (i = 0; i < MAX_CELL_TEMP_NUM; i ++)
    {
		/* 电芯温度充电逻辑判断 */
		t_timer = t_cht_timer[i];
		temperature_alarm_process(g_run_sys_data.cell_temp[i], &t_timer, &g_cht_status[i], &g_cht_child[i], 0);
		t_cht_timer[i] = t_timer;
		/* 电芯温度放电逻辑判断 */
		t_timer = t_dcht_timer[i];
		temperature_alarm_process(g_run_sys_data.cell_temp[i], &t_timer, &g_dcht_status[i], &g_dcht_child[i], 1);
		t_dcht_timer[i] = t_timer;
	}

    g_ch_temp_flag = E_TEMP_IDLE;
    g_dch_temp_flag = E_TEMP_IDLE;
    dchFlag = 0;
    chFlag = 0;
    for(i = 0; i < MAX_CELL_TEMP_NUM; i ++)
    {
        /* 电芯温度禁充判决 */
        if (g_cht_child[i] == E_CHILD_IDLE)
        {
            if ((E_TEMP_LOW_PROTECT == g_cht_status[i]) || (E_TEMP_HIGH_PROTECT == g_cht_status[i]))
            {
                chFlag = 1;
                g_ch_temp_flag = g_cht_status[i];
            }
            /* 电芯温度禁充告警判决 */
            else if ((E_TEMP_LOW_ALARM == g_cht_status[i]) || (E_TEMP_HIGH_ALARM == g_cht_status[i]))
            {
                /* 电芯温度禁充告警首先排除已经存在电芯温度禁充状态 */
                if (0 == chFlag)
                {
                    g_ch_temp_flag = g_cht_status[i];
                }
            }
        }
        else
        {
            if (E_TEMP_LOW_PROTECT == g_cht_status[i])
            {
                /* 电芯温度禁充告警首先排除已经存在电芯温度禁充状态 */
                if (0 == chFlag)
                {
                    g_ch_temp_flag = E_TEMP_LOW_ALARM;
                }
            }
            if (E_TEMP_HIGH_PROTECT == g_cht_status[i])
            {
                /* 电芯温度禁充告警首先排除已经存在电芯温度禁充状态 */
                if (0 == chFlag)
                {
                    g_ch_temp_flag = E_TEMP_HIGH_ALARM;
                }
            }
        }

        /* 电芯温度禁放判决 */
        if (g_dcht_child[i] == E_CHILD_IDLE)
        {
            if ((E_TEMP_LOW_PROTECT == g_dcht_status[i]) || (E_TEMP_HIGH_PROTECT == g_dcht_status[i]))
            {
                dchFlag = 1;
                g_dch_temp_flag = g_dcht_status[i];
            }
            /* 电芯温度禁放告警判决 */
            else if ((E_TEMP_LOW_ALARM == g_dcht_status[i]) || (E_TEMP_HIGH_ALARM == g_dcht_status[i]))
            {
                /* 电芯温度禁放告警首先排除已经存在电芯温度禁放状态 */
                if (0 == dchFlag)
                {
                    g_dch_temp_flag = g_dcht_status[i];
                }
            }
        }
        else
        {
            if (E_TEMP_LOW_PROTECT == g_dcht_status[i])
            {
                /* 电芯温度禁放告警首先排除已经存在电芯温度禁放状态 */
                if (0 == dchFlag)
                {
                    g_dch_temp_flag = E_TEMP_LOW_ALARM;
                }
            }
            if (E_TEMP_HIGH_PROTECT == g_dcht_status[i])
            {
                if (0 == dchFlag)
                {
                    g_dch_temp_flag = E_TEMP_HIGH_ALARM;
                }
            }
        }
    }
}

/*=============================================================
 * 函数名称：PowerTempDeal
 * 函数功能：功率温度标志处理，归一化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人        修改类型
 * 2021-11-03        戴辉发        创建
==============================================================*/
static void PowerTempDeal(void)
{
	uint16_t t_timer;
   
	/* 功率温度逻辑判断 */
	t_timer = t_powt_timer;
	temperature_alarm_process(g_run_sys_data.power_temp, &t_timer, &g_powt_status, &g_powt_child, 2);
	t_powt_timer = t_timer;

    if (E_CHILD_IDLE == g_powt_child)
    {
        if ((E_TEMP_LOW_PROTECT == g_powt_status) || (E_TEMP_HIGH_PROTECT == g_powt_status))
        {
            powtTempFlag = g_powt_status;
        }
        else if ((E_TEMP_LOW_ALARM == g_powt_status) || (E_TEMP_HIGH_ALARM == g_powt_status))
        {
            powtTempFlag = g_powt_status;
        }
        else
        {
            powtTempFlag = E_TEMP_IDLE;
        }
    }
    else
    {
        if (E_TEMP_LOW_PROTECT == g_powt_status)
        {
            powtTempFlag = E_TEMP_LOW_ALARM;
        }
        else if (E_TEMP_HIGH_PROTECT == g_powt_status)
        {
            powtTempFlag = E_TEMP_HIGH_ALARM;
        }
        else
        {
            powtTempFlag = E_TEMP_IDLE;
        }
    }
}

/*=============================================================
 * 函数名称：EnviTempDeal
 * 函数功能：环境温度标志处理，归一化
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人        修改类型
 * 2021-11-03        戴辉发        创建
==============================================================*/
static void EnviTempDeal(void)
{
	uint16_t t_timer;
 
	/* 环境温度逻辑判断 */
	t_timer = t_envt_timer;
	temperature_alarm_process(g_run_sys_data.ambient_temp, &t_timer, &g_envt_status, &g_evnt_child, 3);
	t_envt_timer = t_timer;

    /*  不是中间状态*/
    if (E_CHILD_IDLE == g_evnt_child)
    {
        if ((E_TEMP_LOW_PROTECT == g_envt_status) || (E_TEMP_HIGH_PROTECT == g_envt_status))
        {
            envtTempFlag = g_envt_status;
        }
        else if ((E_TEMP_LOW_ALARM == g_envt_status) || (E_TEMP_HIGH_ALARM == g_envt_status))
        {
            envtTempFlag = g_envt_status;
        }
        else
        {
            envtTempFlag = E_TEMP_IDLE;
        }
    }
    else
    {
        if (E_TEMP_LOW_PROTECT == g_envt_status)
        {
            envtTempFlag = E_TEMP_LOW_ALARM;
        }
        else if (E_TEMP_HIGH_PROTECT == g_envt_status)
        {
            envtTempFlag = E_TEMP_HIGH_ALARM;
        }
        else
        {
            envtTempFlag = E_TEMP_IDLE;
        }
    }
}

/*=============================================================
 * 函数名称：temperature_mange
 * 函数功能：温度逻辑处理模块温度处理主流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *=============================================================
 * 日    期          修改人         修改类型
 * 2018-06-14        戴辉发     	创建
 * 2021-11-03        戴辉发         温度处理归一化
==============================================================*/
static void temp_alarm_process(void)
{
    uint8_t flag;

    /* 设定温度最值 */
    set_max_celltemp(get_max_cell_temp());
    set_min_celltemp(get_min_cell_temp());
    set_max_powtemp(g_run_sys_data.power_temp);
    set_min_powtemp(g_run_sys_data.power_temp);
    set_max_envtemp(g_run_sys_data.ambient_temp);
    set_min_envtemp(g_run_sys_data.ambient_temp);

    /* 获取上报温度值 */
    g_report_cell_temp = get_report_temp();

    /***********************************************************/
    /*******************统一判别温度标识************************/
    /* 电芯温度归一化处理 */
    CellTempDeal();
    /* 功率温度归一化处理 */
    PowerTempDeal();
    /* 环境温度归一化处理 */
    EnviTempDeal();
    
    flag = 0;
    /* 判决禁充禁放逻辑 */
    /* 环境温度影响逻辑 */
    if((E_TEMP_LOW_PROTECT == envtTempFlag) || (E_TEMP_HIGH_PROTECT == envtTempFlag))
    {
        flag = 1;
        if(g_temp_flag != E_TEMP_DISCHDCH_STATUS)
        {
            g_temp_flag = E_TEMP_DISCHDCH_STATUS;
            set_temp_nocharge();
            set_temp_nodischarge();
            record_protect = 1;
        }
    }
    /* 功率温度影响逻辑 */
    if ((E_TEMP_LOW_PROTECT == powtTempFlag) || (E_TEMP_HIGH_PROTECT == powtTempFlag))
    {
        flag = 1;
        if(g_temp_flag != E_TEMP_DISCHDCH_STATUS)
        {
            g_temp_flag = E_TEMP_DISCHDCH_STATUS;
            set_temp_nocharge();
            set_temp_nodischarge();
            record_protect = 1;
        }
    }
    /* 电芯温度影响逻辑 */
    if(((E_TEMP_LOW_PROTECT == g_dch_temp_flag) || (E_TEMP_HIGH_PROTECT == g_dch_temp_flag)) && 
       ((E_TEMP_LOW_PROTECT == g_ch_temp_flag) || (E_TEMP_HIGH_PROTECT == g_ch_temp_flag)))
    {
        flag = 1;
        if(g_temp_flag != E_TEMP_DISCHDCH_STATUS)
        {
            g_temp_flag = E_TEMP_DISCHDCH_STATUS;
            set_temp_nocharge();
            set_temp_nodischarge();
            record_protect = 1;
        }
    }

    /* 系统不存在禁充又禁放逻辑 */
    if(0 == flag)
    {
        /* 系统温度禁放逻辑 */
        if((E_TEMP_LOW_PROTECT == g_dch_temp_flag) || (E_TEMP_HIGH_PROTECT == g_dch_temp_flag))
        {
            flag = 1;
            if(g_temp_flag != E_TEMP_DISDCH_STATUS)
            {
                g_temp_flag = E_TEMP_DISDCH_STATUS;
                set_temp_nodischarge();
                record_protect = 1;
            }
        }
        /* 系统温度禁充逻辑 */
        if ((E_TEMP_LOW_PROTECT == g_ch_temp_flag) || (E_TEMP_HIGH_PROTECT == g_ch_temp_flag))
        {
            flag = 1;
            if(g_temp_flag != E_TEMP_DISCH_STATUS)
            {
                g_temp_flag = E_TEMP_DISCH_STATUS;
                set_temp_nocharge();
                record_protect = 1;
            }
        }
    }

    /* 系统不存在禁充或禁放现象 */
    if (0 == flag)
	{
        if ((E_TEMP_LOW_ALARM == envtTempFlag) || (E_TEMP_HIGH_ALARM == envtTempFlag))
        {
            flag = 1;
            g_temp_flag = E_TEMP_ALARM_STATUS;
        }
        if ((E_TEMP_LOW_ALARM == powtTempFlag) || (E_TEMP_HIGH_ALARM == powtTempFlag))
        {
            flag = 1;
            g_temp_flag = E_TEMP_ALARM_STATUS;
        }
        if (E_CHARGE_STATUS != get_system_status())
        {
            if((E_TEMP_LOW_ALARM == g_dch_temp_flag) || (E_TEMP_HIGH_ALARM == g_dch_temp_flag))
            {
                flag = 1;
                g_temp_flag = E_TEMP_ALARM_STATUS;
            }
        }
        else
        {
            if((E_TEMP_LOW_ALARM == g_ch_temp_flag) || (E_TEMP_HIGH_ALARM == g_ch_temp_flag))
            {
                flag = 1;
                g_temp_flag = E_TEMP_ALARM_STATUS;
            }
        }
    }

    /* 系统也不存在温度告警现象 */
    if (0 == flag)
    {
        g_temp_flag = E_TEMP_IDLE_STATUS;
    }

    if(g_dch_temp_flag  ==  E_TEMP_LOW_PROTECT)
    {
        protect_code[2] |= 0x01;
    }
    else
    {
        protect_code[2] &= ~0x01;
    }

    if(g_dch_temp_flag  ==  E_TEMP_HIGH_PROTECT)
    {
        protect_code[2] |= 0x02;
    }
    else
    {
        protect_code[2] &= ~0x02;
    }

    if(g_ch_temp_flag  ==  E_TEMP_LOW_PROTECT)
    {
        protect_code[2] |= 0x04;
    }
    else
    {
        protect_code[2] &= ~0x04;
    }

    if(g_ch_temp_flag  ==  E_TEMP_HIGH_PROTECT)
    {
        protect_code[2] |= 0x08;
    }
    else
    {
        protect_code[2] &= ~0x08;
    }

    if(powtTempFlag == E_TEMP_LOW_PROTECT)
    {
        protect_code[2] |= 0x10;
    }
    else
    {
        protect_code[2] &= ~0x10;
    }

    if(powtTempFlag == E_TEMP_HIGH_PROTECT)
    {
        protect_code[2] |= 0x20;
    }
    else
    {
        protect_code[2] &= ~0x20;
    }

    if(envtTempFlag == E_TEMP_LOW_PROTECT)
    {
        protect_code[2] |= 0x40;
    }
    else
    {
        protect_code[2] &= ~0x40;
    }

    if(envtTempFlag == E_TEMP_HIGH_PROTECT)
    {
        protect_code[2] |= 0x80;
    }
    else
    {
        protect_code[2] &= ~0x80;
    }

    control_temp_indication();
}

/*=============================================================
 * 函数名称：temperature_mange
 * 函数功能：温度逻辑处理模块温度处理主流程
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
void temperature_mange(void)
{
    if (judge_vol_sample_finished())
    {
        temp_alarm_process();
    }
}

/*=============================================================
 * 函数名称：get_ch_temp_status
 * 函数功能：获取指定电芯充电温度状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           电芯充电温度状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-09-09        戴辉发     	创建
==============================================================*/
e_item_status get_ch_temp_status(void)
{
	return g_ch_temp_flag;
}

/*=============================================================
 * 函数名称：get_dch_temp_status
 * 函数功能：获取指定电芯放电温度状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           电芯放电温度状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-09-09        戴辉发     	创建
==============================================================*/
e_item_status get_dch_temp_status(void)
{
	return g_dch_temp_flag;
}

/*=============================================================
 * 函数名称：get_environ_temp_status
 * 函数功能：获取指定环境温度状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           环境温度状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-09-09        戴辉发     	创建
==============================================================*/
e_item_status get_environ_temp_status(void)
{
	return g_envt_status;
}

/*=============================================================
 * 函数名称：get_power_temp_status
 * 函数功能：获取指定功率温度状态
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           功率温度状态
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-09-09        戴辉发     	创建
==============================================================*/
e_item_status get_power_temp_status(void)
{
	return powtTempFlag;
}

/*=============================================================
 * 函数名称：get_cell_temp
 * 函数功能：获取指定序号的电芯温度
 * 参数个数：1
 * 参数描述：
 *           [IN]    index       电芯温度序号
 * 返 回 值：
 *           电芯温度
 * 修改记录：
 *===============================================================
 * 日    期          修改人      修改类型
 * 2018-06-14        戴辉发     	创建
==============================================================*/
int16_t get_cell_temp(uint8_t index)
{
	return g_run_sys_data.cell_temp[index];
}

/*=============================================================
 * 函数名称：get_max_cell_temp
 * 函数功能：获取当前最高电芯温度
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最高电芯温度
 * 修改记录：
 *==============================================================
 * 日    期             修改人      修改类型
 * 2019-08-22       	戴辉发     	创建
==============================================================*/
int16_t get_max_cell_temp(void)
{
    uint8_t i;
    int16_t temp, temp1;

    temp = g_run_sys_data.cell_temp[0];
    for (i = 1; i < MAX_CELL_TEMP_NUM; i ++)
    {
        temp1 = g_run_sys_data.cell_temp[i];
        if (temp < temp1)
        {
            temp = temp1;
        }
    }
    return temp;
}

/*=============================================================
 * 函数名称：get_max_cell_temp_no
 * 函数功能：获取当前最高电芯温度号
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最高电芯温度号
 * 修改记录：
 *==============================================================
 * 日    期             修改人      修改类型
 * 2020-6-3      	    李勇     	创建
==============================================================*/
uint8_t get_max_cell_temp_no(void)
{
    uint8_t i,max;
    int16_t temp, temp1;

    max = 0;
    temp = g_run_sys_data.cell_temp[0];
    for (i = 1; i < MAX_CELL_TEMP_NUM; i ++)
    {
        temp1 = g_run_sys_data.cell_temp[i];
        if (temp < temp1)
        {
            temp = temp1;
            max = i;
        }
    }
    return max;
}

/*=============================================================
 * 函数名称：get_min_cell_temp
 * 函数功能：获取当前最低电芯温度
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最高电芯温度
 * 修改记录：
 *==============================================================
 * 日    期             修改人      修改类型
 * 2020-6-3      	    李勇     	创建
==============================================================*/
int16_t get_min_cell_temp(void)
{
    uint8_t i;
    int16_t temp, temp1;

    temp = g_run_sys_data.cell_temp[0];
    for (i = 1; i < MAX_CELL_TEMP_NUM; i ++)
    {
        temp1 = g_run_sys_data.cell_temp[i];
        if (temp > temp1)
        {
            temp = temp1;
        }
    }
    return temp;
}

/*=============================================================
 * 函数名称：get_max_cell_temp_no
 * 函数功能：获取当前最高电芯温度号
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *          最高电芯温度号
 * 修改记录：
 *==============================================================
 * 日    期             修改人      修改类型
 * 2020-6-3      	    李勇     	创建
==============================================================*/
uint8_t get_min_cell_temp_no(void)
{
    uint8_t i,min;
    int16_t temp, temp1;

    min = 0;
    temp = g_run_sys_data.cell_temp[0];
    for (i = 1; i < MAX_CELL_TEMP_NUM; i ++)
    {
        temp1 = g_run_sys_data.cell_temp[i];
        if (temp > temp1)
        {
            temp = temp1;
            min = i;
        }
    }
    return min;
}
/*=============================================================
 * 函数名称：get_average_cell_temp
 * 函数功能：获取平均电芯温度
 * 参数个数：0
 * 参数描述：
 * 
 * 返 回 值：无
 * 修改记录：
 *==============================================================
 * 日    期          修改人      修改类型
 * 2018-07-06        戴辉发     	创建
==============================================================*/
int16_t get_average_cell_temp(void)
{
	uint8_t i = 0;
	long tmp = 0;

	for(i = 0; i < MAX_CELL_TEMP_NUM; i ++)
	{
		tmp += g_run_sys_data.cell_temp[i];
	}
	tmp = tmp / MAX_CELL_TEMP_NUM;

	return (int16_t)tmp;
}

/*=============================================================
 * 函数名称：get_power_temp
 * 函数功能：获取功率温度
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           功率温度
 * 修改记录：
 *===============================================================
 * 日    期             修改人      修改类型
 * 2018-06-14       	戴辉发     	创建
==============================================================*/
int16_t get_power_temp(void)
{
	return g_run_sys_data.power_temp;
}

/*=============================================================
 * 函数名称：get_enviror_temp
 * 函数功能：获取环境温度
 * 参数个数：0
 * 参数描述：
 * 返 回 值：
 *           环境温度
 * 修改记录：
 *===============================================================
 * 日    期             修改人      修改类型
 * 2018-06-14       	戴辉发     	创建
==============================================================*/
int16_t get_enviror_temp(void)
{
	return g_run_sys_data.ambient_temp;
}