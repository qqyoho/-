#include "system_adjust.h"
#include "vol_manage.h"
#include "adc_sampling.h"
#include "hardware.h"
#include "storage_manage.h"
#include "string.h"
#include "system_adjust.h"
#include "idog.h"
#include "parameter.h"
#include "iap_app.h"
#include "protect_record.h"
#include "peak_record.h"
#include "balance.h"
#include "soc_update.h"
#include "afe_app.h"
#include "switch_status.h"
#include "run_record.h"
#include "low_power.h"
/* 静态SOC校准 */
typedef struct BAT_CAPACITY_TYP
{
  uint16_t bat_vol_value;
  uint8_t cap_percent;
}bat_capacity_typ;

#define MAX_VOLTAGE_SOC_NUM           (21)
#define TEST_CURRENT_CMD_BASE         30000
#define TEST_CURRENT_CMD_RANGE        1000
#define TEST_CURRENT_CMD_DISABLE      110
#define TEST_CURRENT_A_TO_01A         10
#define TEST_POWER_ON_COUNT_MAX       20000

/* 记录静态SOC校准 */
#if defined (HUAFU) && defined (BAT_8S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3341, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3342, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3342, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3342, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3342, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3342, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (HUAFU) && defined (BAT_15S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3341, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3342, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3342, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3342, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3342, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3342, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (FUDEER) && defined (BAT_8S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3323, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3326, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3329, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3332, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3335, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3338, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (YUHENG) && defined (BAT_8S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3323, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3326, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3329, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3332, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3335, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3338, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (FUDEER) && defined (BAT_15S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3323, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3326, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3329, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3332, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3335, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3338, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (TIANHONG) && defined (BAT_8S) && defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3341, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3342, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3342, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3342, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3342, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3342, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#elif defined (NEWNOB) && defined (BAT_8S) && defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3341, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3342, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3342, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3342, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3342, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3342, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

#elif defined (TIANFENG) && defined (BAT_8S)&& defined (BATTARY_LFP)
static const bat_capacity_typ g_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3201, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3209, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3233, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3251, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3265, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3278, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3285, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3287, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3288, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3289, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3291, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3297, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3311, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3324, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3327, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3328, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3328, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3329, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3330, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3332, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};

static const bat_capacity_typ g_ch_bat_capacity[MAX_VOLTAGE_SOC_NUM] =
{
	{3215, (uint8_t)(0.00 * MAX_SOC_VALUE)}, /* 00 */
	{3234, (uint8_t)(0.05 * MAX_SOC_VALUE)}, /* 05 */
	{3267, (uint8_t)(0.10 * MAX_SOC_VALUE)}, /* 10 */
	{3290, (uint8_t)(0.15 * MAX_SOC_VALUE)}, /* 15 */
	{3303, (uint8_t)(0.20 * MAX_SOC_VALUE)}, /* 20 */
	{3306, (uint8_t)(0.25 * MAX_SOC_VALUE)}, /* 25 */
	{3307, (uint8_t)(0.30 * MAX_SOC_VALUE)}, /* 30 */
	{3308, (uint8_t)(0.35 * MAX_SOC_VALUE)}, /* 35 */
	{3310, (uint8_t)(0.40 * MAX_SOC_VALUE)}, /* 40 */
	{3312, (uint8_t)(0.45 * MAX_SOC_VALUE)}, /* 45 */
	{3320, (uint8_t)(0.50 * MAX_SOC_VALUE)}, /* 50 */
	{3323, (uint8_t)(0.55 * MAX_SOC_VALUE)}, /* 55 */
	{3326, (uint8_t)(0.60 * MAX_SOC_VALUE)}, /* 60 */
	{3329, (uint8_t)(0.65 * MAX_SOC_VALUE)}, /* 65 */
	{3332, (uint8_t)(0.70 * MAX_SOC_VALUE)}, /* 70 */
	{3335, (uint8_t)(0.75 * MAX_SOC_VALUE)}, /* 75 */
	{3338, (uint8_t)(0.80 * MAX_SOC_VALUE)}, /* 80 */
	{3342, (uint8_t)(0.85 * MAX_SOC_VALUE)}, /* 85 */
	{3358, (uint8_t)(0.90 * MAX_SOC_VALUE)}, /* 90 */
	{3419, (uint8_t)(0.95 * MAX_SOC_VALUE)}, /* 95 */
	{3425, (uint8_t)(1.00 * MAX_SOC_VALUE)}, /*100 */
};
#endif

/* 电压校准参数结构定义 */
typedef struct _ADJUST_VOLTAGE_PARA_STRUCT_
{
	float k_value[MAX_BAT_NUM]; /* 电压校准值K值 */
	float b_value[MAX_BAT_NUM]; /* 电压校准值B值 */
    float t_k_value;
    float t_b_value;
	uint8_t end_id;
}t_adjust_voltage_para;

typedef struct _T_SOC_CORRECT_STRUCT_
{
	uint16_t voltage[MAX_VOLTAGE_SOC_NUM]; /* 对应电压值 */
	uint8_t soc[MAX_VOLTAGE_SOC_NUM]; /* 对应SOC值 */
	uint8_t soc_num; /* SOC个数 */
	uint8_t bat_type; /* 电芯类型 */
	uint8_t end_id;
}t_soc_correct_st;

static uint8_t g_curr_main_flag;
static uint8_t g_curr_end_id;
t_adjust_para_st g_adjust_curr_para;

static uint8_t g_vol_main_flag;
static uint8_t g_vol_end_id;
static t_adjust_voltage_para g_adjust_voltage_para;

uint16_t g_balance_test;

static uint16_t cvol1_advlu[MAX_BAT_NUM];
static uint16_t cvol2_advlu[MAX_BAT_NUM];
static uint16_t tvol1_adv1u;
static uint16_t tvol2_adv2u;

/* 校准电流，单位10mA */
short correct_curr;
/* 校准电压1，单位毫伏 */
uint16_t correct_vol1;
/* 校准电压2，单位毫伏 */
uint16_t correct_vol2;

/*==============================================================
 * 函数名称：system_adjust_mem_init
 * 函数功能：系统参数内存初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
void system_adjust_mem_init(void)
{
	g_curr_main_flag = 0;
	g_curr_end_id = MAX_STORAGE_NUM;
	memset(&g_adjust_curr_para, 0, sizeof(g_adjust_curr_para));

	g_vol_main_flag = 0;
	g_vol_end_id = MAX_STORAGE_NUM;
	memset(&g_adjust_voltage_para, 0, sizeof(g_adjust_voltage_para));

	correct_curr = -1000;
	correct_vol1 = 3000;
	correct_vol2 = 4000;
    g_balance_test = 0x0000;
}

/*==============================================================
 * 函数名称：current_adjust_data_hard_init
 * 函数功能：电流校准参数内存读取存储区后初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
static void current_adjust_data_hard_init(void)
{
	short flag = 0;
	t_adjust_para_st current_data;

	flag = read_data_from_storage(ADJUST_CURRENT_START, (uint8_t *)&g_adjust_curr_para, sizeof(g_adjust_curr_para));
    if( flag == sizeof(g_adjust_curr_para) )
    {
       flag = read_data_from_storage(ADJUST_CURRENT_BACK, (uint8_t *)&current_data, sizeof(current_data));
       if( flag == sizeof(g_adjust_curr_para) )
       {
          flag = judge_new_sector(g_adjust_curr_para.end_id, current_data.end_id);
       }
    }
	
	if (flag == -1)
	{
		g_curr_main_flag = 1;
		g_curr_end_id = current_data.end_id;
		memcpy(&g_adjust_curr_para, &current_data, sizeof(current_data));
        protect_code[3] |= 0x08;
	}
	else if (flag == 0)
	{
		g_curr_main_flag = 1;
		g_curr_end_id = MAX_STORAGE_NUM;
		g_adjust_curr_para.k_value = CURRENT_RVALUE;
		g_adjust_curr_para.b_value = 0.0;
	}
	else
	{
        protect_code[3] |= 0x08;
		g_curr_main_flag = 0;
		g_curr_end_id = g_adjust_curr_para.end_id;
	}
}

/*==============================================================
 * 函数名称：vol_adjust_data_hard_init
 * 函数功能：电压校准参数内存读取存储区后初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
static void vol_adjust_data_hard_init(void)
{
	short flag = 0;
	t_adjust_voltage_para vol_data;
    
	flag = read_data_from_storage(ADJUST_VOLTAGE_START, (uint8_t *)&g_adjust_voltage_para, sizeof(g_adjust_voltage_para));
    if( flag == sizeof(g_adjust_voltage_para) )
    {
       flag = read_data_from_storage(ADJUST_VOLTAGE_BACK, (uint8_t *)&vol_data, sizeof(vol_data));
       if( flag == sizeof(g_adjust_voltage_para) )
       {
          flag = judge_new_sector(g_adjust_voltage_para.end_id, vol_data.end_id);
       }
    }

	if (flag == -1)
	{
		g_vol_main_flag = 1;
		g_vol_end_id = vol_data.end_id;
		memcpy(&g_adjust_voltage_para, &vol_data, sizeof(vol_data));
        protect_code[3] |= 0x04;
	}
	else if (flag == 0)
	{
		uint8_t i;

		g_vol_main_flag = 1;
		g_vol_end_id = MAX_STORAGE_NUM;

		for (i = 0; i < MAX_BAT_NUM; i ++)
		{
			g_adjust_voltage_para.k_value[i] = 1.0;
			g_adjust_voltage_para.b_value[i] = 0;
		}      
        g_adjust_voltage_para.t_k_value =  1.0;
        g_adjust_voltage_para.t_b_value = 0;
	}
	else
	{
		g_vol_main_flag = 0;
		g_vol_end_id = g_adjust_voltage_para.end_id;
        protect_code[3] |= 0x04;
	}
}

/*==============================================================
 * 函数名称：system_adjust_hard_init
 * 函数功能：系统校准参数内存读取存储区后初始化
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
void system_adjust_hard_init(void)
{
	current_adjust_data_hard_init();
	vol_adjust_data_hard_init();
}

/*==============================================================
 * 函数名称：save_adjust_current_parameter
 * 函数功能：保存校准参数
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
void save_adjust_current_parameter(void)
{
	t_write_data_st data;

	g_curr_end_id = sector_num_next(g_curr_end_id);

	g_adjust_curr_para.end_id = g_curr_end_id;
	
	if ( g_curr_main_flag ){
		g_curr_main_flag = 0;
		data.write_storge_address = ADJUST_CURRENT_START;
	}
	else{
		g_curr_main_flag = 1;
		data.write_storge_address = ADJUST_CURRENT_BACK;
	}
	data.msg_type = E_ADJUST_PARA_MSG;
	data.write_len = sizeof(g_adjust_curr_para);
	data.write_memery_address = (uint8_t *)&g_adjust_curr_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
 * 函数名称：get_current_kb_value
 * 函数功能：获取电流KB值
 * 参数个数：4
 * 函数参数：
 *           [IN/OUT]  k_value            斜率
 *           [IN/OUT]  b_value            起始点AD值
 *           [IN/OUT]  vref               基准电压对应的AD值
 *           [IN/OUT]  amplify            出厂时放大倍数
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-22          戴辉发             创建
==============================================================*/
void get_current_kb_value(float *k_value, float *b_value, float *vref, float *amplify)
{
	*k_value = g_adjust_curr_para.k_value;
	*b_value = g_adjust_curr_para.b_value;
}

/*==============================================================
 * 函数名称：save_adjust_voltage_parameter
 * 函数功能：保存电压校准参数
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-23          戴辉发             创建
==============================================================*/
void save_adjust_voltage_parameter(void)
{
	t_write_data_st data;

	g_vol_end_id = sector_num_next(g_vol_end_id);
	g_adjust_voltage_para.end_id = g_vol_end_id;

	if ( g_vol_main_flag )
    {
		g_vol_main_flag = 0;
		data.write_storge_address = ADJUST_VOLTAGE_START;
	}
	else
    {
		g_vol_main_flag = 1;
		data.write_storge_address = ADJUST_VOLTAGE_BACK;
	}
	data.msg_type = E_ADJUST_PARA_MSG;
	data.write_len = sizeof(g_adjust_voltage_para);
	data.write_memery_address = (uint8_t *)&g_adjust_voltage_para;

	set_write_data_to_queue(&data);
}

/*==============================================================
 * 函数名称：set_voltage_kb_value
 * 函数功能：设定电压KB值
 * 参数个数：3
 * 函数参数：
 *           [IN]      k_value            电流K值缓冲
 *           [IN]      b_value            电流B值缓冲
 *           [IN]      num                电芯个数
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-23          戴辉发             创建
==============================================================*/
void set_voltage_kb_value(float *k_value, float *b_value, uint8_t num)
{
	uint8_t i, flag = 0;

	if ( num == 0 || num > MAX_BAT_NUM ) return;

	for (i = 0; i < num; i ++){
		if ((g_adjust_voltage_para.k_value[i] != k_value[i]) || (g_adjust_voltage_para.b_value[i] != b_value[i])){
			g_adjust_voltage_para.k_value[i] = k_value[i];
			g_adjust_voltage_para.b_value[i] = b_value[i];
			flag = 1;
		}
	}
	if (flag){
		save_adjust_voltage_parameter();
	}
}

/*==============================================================
 * 函数名称：get_voltage_kb_value
 * 函数功能：获取指定电芯KB值
 * 参数个数：3
 * 函数参数：
 *           [IN/OUT]  k_value            零点电流值
 *           [IN/OUT]  b_value            零点电流对应的adc值
 *           [IN]      index              电芯位置索引
 * 返 回 值：
 *           无
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-06-23          戴辉发             创建
==============================================================*/
void get_voltage_kb_value(float *k_value, float *b_value, uint8_t index)
{
	if (index < MAX_BAT_NUM){
		*k_value = g_adjust_voltage_para.k_value[index];
		*b_value = g_adjust_voltage_para.b_value[index];
	}
}

/*==============================================================
 * 函数名称：soc_static_adjust
 * 函数功能：系统静态开路校准
 * 参数个数：1
 * 函数参数：
 *           [IN]      voltage            输入电压值
 *           [IN]      flag               校准标识
 * 返 回 值：
 *           SOC
 * 修改记录:
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
uint16_t soc_static_adjust(uint16_t voltage, uint8_t flag)
{
    const bat_capacity_typ *p_soc_value;
	float k_value = 0.0;
	uint16_t cur_soc_percent = 0;
	uint8_t i;

    if (flag)
    {
        p_soc_value = &g_ch_bat_capacity[0];
    }
    else
    {
        p_soc_value = &g_bat_capacity[0];
    }
	if (voltage < MIN_VOLTAGE_VALUE)
	{
		cur_soc_percent = 0;
	}
	else
	{
	    if (voltage >= p_soc_value[0].bat_vol_value)
	    {
	        /* 查找校准点位 */
	        for(i = 0; i < (MAX_VOLTAGE_SOC_NUM - 1); i++)
	        {
	            /* 平均电压与相邻节点数值比较 */
	            if ((voltage >= p_soc_value[i].bat_vol_value) && (voltage < p_soc_value[i + 1].bat_vol_value))
	            {
	                /* 找到对应区间 */
	                if (p_soc_value[i].bat_vol_value != p_soc_value[i + 1].bat_vol_value)
	                {
	                    k_value = 1.0 * (p_soc_value[i + 1].cap_percent - p_soc_value[i].cap_percent) / (p_soc_value[i + 1].bat_vol_value - p_soc_value[i].bat_vol_value);
	                }
	                break;
	            }
	        }
	        if (i == (MAX_VOLTAGE_SOC_NUM - 1))
	        {
	            cur_soc_percent = MAX_SOC_VALUE;
	        }
	        else
	        {
	            /* 计算当前SOC */
	            cur_soc_percent = (uint16_t)(p_soc_value[i].cap_percent + k_value * (voltage - p_soc_value[i].bat_vol_value));
	            if (cur_soc_percent > MAX_SOC_VALUE)
	            {
	                cur_soc_percent = MAX_SOC_VALUE;
	            }
	        }
	    }
	}
    return cur_soc_percent;
}

/*==============================================================
 * 函数名称：current_adjust
 * 函数功能：电流校准过程
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
void current_adjust(void)
{
    if (correct_curr != 0)
    {
        uint8_t num;
        uint8_t flag;
        float vol, vol0,ftemp;

        num = 0;
        flag = 0;
        vol = 0;
        /* correct_curr电流 */
        while (1)
        {
            uint16_t temp,temp1;
            uint8_t ret;

            /* 读取电流数值 */
            /* 连续读取两次数据 */
            ret = get_curr_vol_data(&temp);
            ret = get_curr_vol_data(&temp1);           
            if (ret && (temp1 == temp))
            {
                ftemp = temp;
                ftemp = -(ftemp * 20000.0 / 0x8000 - 20000.0) ; /* 0.1A为单位 */
                vol += ftemp;
                flag++;
                if (flag >= 3)
                {
                    vol /= 3;
                    break;
                }
            }
            delay_ms(132);
            /* 喂狗 */
            feed_iwdg(); 
            num += 1;
            if (num >= 10)
            {
                break;
            }
        }

        cutoff_all_switch();
        delay_ms(500);
        /* correct_curr电流 */
        num = 0;
        vol0 = 0;
        flag = 0;
        while (1)
        {
            uint16_t temp, temp1;
            uint8_t ret;

            /* 读取电流数值 */
            //连续读取两次数据
            ret = get_curr_vol_data(&temp);
            ret = get_curr_vol_data(&temp1);           
            if (ret && (temp1 == temp))
            {      
                  ftemp = temp;
                  ftemp = -(ftemp * 20000.0 / 0x8000 - 20000.0) ; /* 0.1A为单位 */
                  vol0 += ftemp;
                  flag++;
                  if (flag >= 3)
                  {
                      vol0 /= 3;
                      break;
                  }
            }
            delay_ms(132);
            /* 喂狗 */
            feed_iwdg(); 
            num += 1;
            if (num >= 10)
            {
                break;
            }
        }

        /* 计算电流校准K值 放大10倍*/
        g_adjust_curr_para.k_value = (vol - vol0) / correct_curr/10;
        /* 电流B值 */
        g_adjust_curr_para.b_value = vol0;
        protect_code[3] |= 0x08;
        /* 保存电流校准数据到EEP */
        save_adjust_current_parameter();
    }
}

/*==============================================================
 * 函数名称：voltage1_adjust
 * 函数功能：电压1校准
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
void voltage1_adjust(void)
{
    uint16_t vol_buf[MAX_BAT_NUM];
    uint8_t num;
    uint8_t flag;
    uint8_t i;

    cell_balance_off(0);
    flag = 0;
    num = 0;
    for (i = 0; i < MAX_BAT_NUM; i ++)
    {
        cvol1_advlu[i] = 0;
    }
    while (1)
    {
        delay_ms(100);

        if (1 == get_adjust_cell_vol(vol_buf))
        {
            for (i = 0; i < BAT_NUM; i ++)
            {
                cvol1_advlu[i] += vol_buf[i];
            }
            flag += 1;
        }

        if (flag >= 3)
        {
            break;
        }
        num += 1;
        if (10 <= num)
        {
            break;
        }
    }
    if (flag > 0)
    {
        for (i = 0; i < BAT_NUM; i ++)
        {
            cvol1_advlu[i] /= flag;
        }
    }
    tvol1_adv1u = 0;
    for (i = 0; i < 10; i ++)
    {
        //tvol1_adv1u += get_voltage_process(adc_get_sampling_value(TOTAL_VOLTAGE_ADS));
    }
    tvol1_adv1u /= 10;
}

/*==============================================================
 * 函数名称：voltage2_adjust
 * 函数功能：电压2校准
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
void voltage2_adjust(void)
{
    uint16_t vol_buf[MAX_BAT_NUM];
    uint8_t num;
    uint8_t flag;
    uint8_t i;

    cell_balance_off(0);
    flag = 0;
    num = 0;
    for (i = 0; i < MAX_BAT_NUM; i ++)
    {
        cvol2_advlu[i] = 0;
    }
    while (1)
    {
        delay_ms(100);

        if (1 == get_adjust_cell_vol(vol_buf))
        {
            for (i = 0; i < BAT_NUM; i ++)
            {
                cvol2_advlu[i] += vol_buf[i];
            }
            flag += 1;
        }

        if (flag >= 3)
        {
            break;
        }
        num += 1;
        if (10 <= num)
        {
            break;
        }
    }
    if (flag > 0)
    {
        for (i = 0; i < BAT_NUM; i ++)
        {
            cvol2_advlu[i] /= flag;
        }
    }
    tvol2_adv2u = 0;
    for (i = 0; i < 10; i ++)
    {
        //tvol2_adv2u += get_voltage_process(adc_get_sampling_value(TOTAL_VOLTAGE_ADS));
    }
    tvol2_adv2u /= 10;
	/* 计算完成后将计算出来的数值写入存储空间 */
	voltage_kb_adjust();
	save_adjust_voltage_parameter();
}

/*==============================================================
 * 函数名称：balance_adjustt
 * 函数功能：均衡测试
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-10-19          戴辉发             创建
==============================================================*/
void balance_adjust(void)
{
    if (g_balance_test == 0x965A)
    {
        uint8_t i;

        for (i = 0; i < BAT_NUM; i ++)
        {
            cell_balance_on(i);
            delay_ms(500);
            cell_balance_off(i);
            feed_iwdg();         
        }
    }
}

/*==============================================================
 * 函数名称：voltage_kb_adjust
 * 函数功能：电压斜率和基准计算
 * 参数个数：0
 * 函数参数：
 * 返 回 值：
 *           无
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
void voltage_kb_adjust(void)
{
	uint8_t i = 0;
	float advlu_diff = 0;
	float k_value;

	/* 计算电压校准的K值和B值 */
	for (i = 0; i < BAT_NUM; i++)
	{
		advlu_diff = (float)(cvol2_advlu[i] - cvol1_advlu[i]);
		if (!advlu_diff)
			advlu_diff = 1;
		k_value = (correct_vol2 - correct_vol1) / (advlu_diff);
		g_adjust_voltage_para.k_value[i] = k_value;
		g_adjust_voltage_para.b_value[i] = (uint16_t)(correct_vol1 - k_value * cvol1_advlu[i]);
	}

    advlu_diff = (float)(tvol2_adv2u - tvol1_adv1u);
    if (!advlu_diff)
    {
        advlu_diff = 1;
    }
    k_value = (correct_vol2 * BAT_NUM - correct_vol1 * BAT_NUM)/10 / (advlu_diff);
    g_adjust_voltage_para.t_k_value = k_value;
    g_adjust_voltage_para.t_b_value = (uint16_t)(correct_vol1 * BAT_NUM/10 - k_value * tvol1_adv1u);
    protect_code[3] |= 0x04;
}

/*==============================================================
 * 函数名称：calaclute_voltage
 * 函数功能：根据ADC计算单体电压
 * 参数个数：2
 * 函数参数：
 *           [IN]      cell_index         对应单体电压下限
 *           [IN]      adc_value          采样换算后的电压值
 * 返 回 值：
 *           校准后电压值
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
uint16_t calaclute_voltage(uint8_t cell_index, float adc_value)
{
	uint16_t tmp;

	/* 计算单压值 */
	tmp = (uint16_t)(adc_value * g_adjust_voltage_para.k_value[cell_index] + g_adjust_voltage_para.b_value[cell_index]);

	return tmp;
}

/*==============================================================
 * 函数名称：calaclute_voltage
 * 函数功能：根据ADC计算单体电压
 * 参数个数：1
 * 函数参数：
 *           [IN]      adc_value          采样换算后的电压值
 * 返 回 值：
 *           校准后电压值
 * 修改记录：
 *===============================================================
 * 日期                修改人             修改内容
 * 2018-07-06          戴辉发             创建
==============================================================*/
uint16_t calaclute_total_voltage(uint16_t adc_value)
{
	uint16_t tmp;

	/* 计算单压值 */
	tmp =  (uint16_t)(adc_value * g_adjust_voltage_para.t_k_value + g_adjust_voltage_para.t_b_value);

	return tmp;
}

uint16_t g_break_test;
uint16_t g_software_reset;
uint16_t g_clear_protect;
uint16_t g_clear_fault;
uint16_t g_clear_voltage_Adjust;
uint16_t g_clear_current_Adjust;
uint16_t g_clear_limit;
uint32_t g_test_cmd;
extern int16_t g_test_current;
extern uint8_t g_test_current_valid;
/*==============================================================
 * 函数名称：Break_detection_test
 * 函数功能：
 * 参数个数：1
 * 函数参数：
 *          
 * 返 回 值：
 *           
 * 修改记录：  
 *===============================================================
 * 日期                修改人             修改内容
 * 2020-06-19          李勇              创建
==============================================================*/
void Break_detection_test(void)
{
	if ((g_break_test & 0xFF00) == 0xab00)
	{     
		float temp = (MAX_SOC_VALUE / 100.00) * (g_break_test & 0x00ff);
        long rated_capcity;
         /* 电池额定电量, 单位10毫安秒 */
        rated_capcity = get_rated_capcity();
        rated_capcity *= AH_VALUE; /* 换算为10毫安S */
		if (temp > MAX_SOC_VALUE) 
			temp = MAX_SOC_VALUE;
		g_run_sys_data.soc = (uint16_t)temp;
		g_dch_circle.residue =  (uint32_t)(temp / MAX_SOC_VALUE * rated_capcity);
	}
	else if( g_break_test== 0xacac )
	{
        FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
		//f_DisenableReadProtection();
        GPIO_InitStruct.pin = FL_GPIO_PIN_7|FL_GPIO_PIN_8;
        GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;  
        GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.pull = FL_ENABLE;
        GPIO_InitStruct.analogSwitch = FL_DISABLE;
        GPIO_InitStruct.remapPin =  FL_DISABLE;
        FL_GPIO_Init(GPIOD, &GPIO_InitStruct);   
	}
}

void software_reset(void)
{
	if (g_software_reset == 0x111)
	{
		NVIC_SystemReset();
		g_software_reset = 0;
	}
}

void clear_protect_numb(void)
{
	if (g_clear_protect == 0x222)
	{
		protect_data_clear();
		g_clear_protect = 0;
	}
}

void clear_fault_numb(void)
{
	if (g_clear_fault == 0x333)
	{
		fault_data_clear();
		g_clear_fault = 0;
	}
}

void clear_voltage_Adjust(void)
{
	if (g_clear_voltage_Adjust == 0x1111)
	{
		uint8_t i;
		for (i = 0; i < MAX_BAT_NUM; i ++)
		{
			g_adjust_voltage_para.k_value[i] = 1.0;
			g_adjust_voltage_para.b_value[i] = 0;
		}
		g_adjust_voltage_para.t_k_value =  1.0;
		g_adjust_voltage_para.t_b_value = 0;
        protect_code[3] &= ~0x04;  
		save_adjust_voltage_parameter();
		g_clear_voltage_Adjust = 0;
	}
}

void clear_current_Adjust(void)
{
	if (g_clear_current_Adjust == 0x1122)
	{
		g_adjust_curr_para.k_value = CURRENT_RVALUE;
		g_adjust_curr_para.b_value = 0;  //0点重新上电会自动校准的，不需要清
		/* 保存电流校准数据到EEP */
        protect_code[3] &= ~0x08;  
		save_adjust_current_parameter();
		g_clear_current_Adjust = 0;
	}
}

void clear_limit_vaule(void)
{
	if (g_clear_limit == 0x444)
	{
		reset_peak_record();
		g_clear_limit = 0;
	}
    else if(g_clear_limit == 0x111)
    {
        run_record_clear();
        g_clear_limit = 0;
    }
    else if(g_clear_limit == 0x222)
    {
        protect_record_clear();
        g_clear_limit = 0;
    }
}

void test_bms_cmd(void)
{
    int32_t testCurrent;

	if (g_test_cmd == 0x111)
	{
		reset_blance_num();
	}
	else  if (g_test_cmd == 0x222)
	{
		capacity_data_clear();
	}
    else  if ( g_test_cmd == 54321 )
	{
		seeBackupVoltageFlag = 6000;
	}
    else  if ( g_test_cmd == TEST_CURRENT_CMD_DISABLE )
	{
        g_test_current = 0;
        g_test_current_valid = 0;
        g_run_sys_data.current = 0;
	}
    else  if (g_test_cmd <= TEST_POWER_ON_COUNT_MAX)
	{
        PowerOnCount = g_test_cmd;
	}
    else  if ((g_test_cmd >= (TEST_CURRENT_CMD_BASE - TEST_CURRENT_CMD_RANGE)) &&
              (g_test_cmd <= (TEST_CURRENT_CMD_BASE + TEST_CURRENT_CMD_RANGE)))
	{
        testCurrent = (int32_t)g_test_cmd - TEST_CURRENT_CMD_BASE;
        testCurrent *= TEST_CURRENT_A_TO_01A;
        g_test_current = (int16_t)testCurrent;
        g_test_current_valid = 1;
        g_run_sys_data.current = g_test_current;
	}
	g_test_cmd = 0;
}
