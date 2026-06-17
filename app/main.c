#include "hardware.h"
#include "switch_status.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "vol_manage.h"
#include "ch_detect.h"
#include "key_status.h"
#include "ch_addition.h"
#include "balance.h"
#include "low_power.h"
#include "parameter.h"
#include "run_record.h"
#include "serial_no.h"
#include "soc.h"
#include "afe_app.h"
#include "system_control.h"
#include "idog.h"
#include "storage_manage.h"
#include "can_app.h"
#include "bsp_nor_flash.h"
#include "power.h"
#include "clear_swd.h"
#include "fault_manage.h"
#include "led_show.h"
#include "adc_sampling.h"
#include "mode_manage.h"
#include "final_protect.h"
#include "peak_record.h"
#include "soc_update.h"
#include "vol_curr_addi_deal.h"
#include "protect_record.h"
#include "iap_app.h"
#include "system_adjust.h"
#include "bluetooth.h"
#include "short.h"
#include "Appcommunication.h"
#include "ChHeart.h"

void HardFault_Handler()
{
    static uint8_t testt;
     while(1)
     {
         testt++;
     }
     
}

//NEGT_DEBUG
int main(void) 
{ 
	/* 低功耗管理模块内存初始化 */
    init_low_power_status();  
    /*短路故障解除*/
    short_mem_init();
	/* AFE模块内存初始化 */
    afe_app_mem_init();
	/* 系统参数管理控制模块内存初始化 */
	system_parameter_mem_init();
	/* 运行记录管理控制模块内存初始化 */
	run_recorde_mem_init();
	/* 产品序列号管理控制模块内存初始化 */
	serial_no_mem_init();
	/* SOC管理控制模块内存初始化 */
	soc_manage_mem_init();
	/* MOS管理控制模块内存初始化 */
	switch_status_mem_init();
	/* 系统校准模块内存初始化 */
	system_adjust_mem_init();
	/* 控制模块内存初始化 */
	control_mem_init();
	/* 温度管理内存初始化 */
	temperature_init();
	/* 电压管理内存初始化 */
	vol_manage_init();
	/* 电流管理模块内存初始化 */
	current_mem_init();
	/* 均衡管理模块内存初始化 */
	balance_mem_init();
	/* 充电器检测内存初始化 */
	ch_detect_mem_init();
	/* 开关机按键检测内存初始化 */
	key_mem_init();
    /* 充放电开关故障管理模块内存初始化 */
    fault_manage_mem_init();
    /* 显示模块内存初始化 */
    led_show_mem_init();
    /* 采样数据内存初始化 */
    adc_mem_init();
    /* 模式管理内存初始化 */
    mode_manage_mem_init();
    /* CANOPEN内存初始化 */
    canopen_init();
    /* 存储流程初始化 */
    storage_manage_mem_init();
#if defined (BATTARY_LFP)
    /* 充电附加模块 */
    ch_addition_mem_init();
#endif 
    ChHeartMemInit();
    /* 极值管理模块内存初始化 */
    peak_record_mem_init();
    /* 保护次数模块内存初始化 */
    protect_record_mem_init();
    /* 异常记录 */
    fault_record_mem_init();
    /* 均衡记录 */
    balance_no_mem_init();
    /* 电压电流辅助判断模块内存初始化 */
    vol_curr_addi_deal_mem_init();
    /* 升级 */
    update_record_mem_init();
	/* 硬件初始化 */
	hardware_init();
    /* 需要延时 */
	/* 序列号存储硬件初始化 */
	serial_no_hard_init();
    /* 硬件版本号初始化 */
    HardwareVersionNoInit();
	/* 系统校准存储硬件初始化 */
	system_adjust_hard_init();
	/* 系统参数存储硬件初始化 */
	system_parameter_hard_init();
    /* 终极保护内存初始化 */
    final_protect_mem_init();
	/* 运行记录存储硬件初始化 */
	run_record_hard_init();
    /* 升级记录 */
    update_record_hard_init();
	/* SOC估算模块初始化 */
	soc_manage_init(); 
    /* 极值硬件初始化 */
    peak_record_hard_init();
    /* 保护次数存储读取初始化 */
    protect_record_hard_init();
    /* 异常记录 */
    fault_record_hard_init();
    /* 均衡记录 */
    balance_record_hard_init();
    /* 需要下载系统软件 */
    app_jump_success();
    /* 写复位次数 */
    set_mcu_reset_fault();
    /* 上电计数初始化 */
    PowerCountInit();
    AppCommonMemInit();
    /* 设置读保护 */
    Set_readProtect();

    /* 系统主流程 */
	while (1)
	{
        __enable_irq();
        /* 数据采样处理流程 */
        adc_data_get_process();
        /* 均衡处理流程 */
        balance_mange();
        /* 主电源检测处理流程 */
        main_power_detect_process();
        ChHeartProc();
        /* 充电器检测处理流程 */
        ch_detect_process();
        /* 开关机键处理流程 */
        key_detect_process();
        /* 开关失效判决流程 */
        switch_fault_process();
        /* SOC处理流程  为了防止第一个数据被新数据 替换掉，所以把函数放在 这里，尽量先计算soc，再计算电流 */
        soc_manage();
        /* AFE数据采集处理流程 */
        get_data_process();
        /* 容量校准处理流程 */
        bat_capacity_adjust();
        /* 电压检测处理流程 */
        vol_manage_process();
        /* 电流检测处理流程 */
        current_managment_process();
        /* 温度检测处理流程 */
        temperature_mange();
        AppCommonProcess();
        /* 电压电流辅助判断 */
        vol_curr_deal_process();
        /* 电压判决AFE异常 */
        vol_afe_fault_process();
        /* 终极保护处理流程 */
        system_final_protect();
        /* MOS故障处理流程 */
        fet_fault_deal_process();
        /* 显示模块处理流程 */
        led_show_process();
        /* 低功耗处理流程 */
        low_power_process();
        /* 充电附加模块 */
        ch_addition_process();
        /* 磷酸铁锂充电末端SOC校准 */
        soc_update_process();
        /* 功率开关处理流程 */
        switch_operate_process();
        /* 存储管理处理流程 */
        storage_manage_process();
        /* 模式管理处理流程 */
        mode_sleep_status_process();
        /* CANOPEN协议栈处理流程 */
        canopen_main();
        /* 蓝牙流程 */
#if defined( BLUETOOTH ) 
        BluetoothDataProcess();
#endif
        /* 喂狗 */
        feed_iwdg();
	}
}
