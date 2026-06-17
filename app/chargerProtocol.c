#include <string.h>
#include "chargerProtocol.h"
#include "can.h"
#include "ch_addition.h"
#include "system_control.h"
#include "vol_manage.h"
#include "current_manage.h"
#include "temp_manage.h"
#include "soc.h"
#include "system_adjust.h"
#include "parameter.h"
#include "switch_status.h"
#include "short.h"

#define CHARGE_RECIVED_DELAY     (20)
#define MAX_PGN_DATA_LEN         (30)

typedef enum
{
    CONTRONL_REQUEST,
    CONFIGN_PARAMETER,
    CHARGE_STOP_CHARGE
}CHARGE_FLOW_STATUS;

typedef struct
{
    uint32_t pgn;
    uint16_t dataNum;
    uint8_t packNum;
    uint8_t status;
    uint16_t delay;
    uint8_t data[MAX_PGN_DATA_LEN];
}PGN_MANAGE_FLOW;

static CHARGE_FLOW_STATUS chargeFlowStatus;
static uint8_t chargerInvalidFlag;
static PGN_MANAGE_FLOW pgnManageFlow;

CHARGER_MESSAGE_SEND chargerRequest;
CHARGER_MESSAGE_SEND chargerMessage;
CHARGER_MESSAGE_DETAIL chargerDetail;

static uint32_t GetRxExtId(CanRxMsg rxMsg)
{
    uint32_t id;

    id = rxMsg.rxcanId.canid_bits.StdId;
    id <<= 18;
    id |= rxMsg.rxcanId.canid_bits.extId;

    return id & 0x1fffffff;
}

void ChargerProtocolMemoryInit(void)
{
    chargeFlowStatus = CONTRONL_REQUEST;
    chargerInvalidFlag = 0;
    memset(&chargerRequest, 0, sizeof(chargerRequest));
    memset(&chargerMessage, 0, sizeof(chargerMessage));
    memset(&chargerDetail, 0, sizeof(chargerDetail));
    memset(&pgnManageFlow, 0, sizeof(pgnManageFlow));
}

void ChargeFlowSendStopCharge(void)
{
    uint8_t data[8] = {0};

    data[0] = 0x20;
    data[1] = 0x05;
    data[2] = 0x00;
    data[3] = 0x01;
    data[4] = 0xff;
    data[5] = 0x02;
    data[6] = 0xf1;
    data[7] = 0x00;
    can_ext_transmit(0x1CECFF28, data, 8);

    data[0] = 0x01;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    data[5] = 0x01;
    data[6] = 0xff;
    data[7] = 0xff;
    can_ext_transmit(0x1CEBFF28, data, 8);
}

void ChargeFlowProcess(uint16_t voltage, uint16_t current, uint8_t flag)
{
    switch(chargeFlowStatus)
    {
    case CONTRONL_REQUEST:
        if((chargerInvalidFlag == 1) && (flag == 0))
        {
            chargeFlowStatus = CONFIGN_PARAMETER;
        }
        break;
    case CONFIGN_PARAMETER:
        if(flag == 1)
        {
            chargeFlowStatus = CHARGE_STOP_CHARGE;
        }
        break;
    default:
        if(flag == 0)
        {
            chargeFlowStatus = CONTRONL_REQUEST;
        }
        break;
    }

    (void)voltage;
    (void)current;
}

void SetChargerInvalidFlag(uint8_t flag)
{
    chargerInvalidFlag = flag;
}

void SendChargeMessage(uint16_t voltage, uint16_t current, uint8_t flag)
{
    uint8_t buf[8] = {0};

    buf[0] = (uint8_t)((voltage >> 8) & 0xff);
    buf[1] = (uint8_t)(voltage & 0xff);
    buf[2] = (uint8_t)((current >> 8) & 0xff);
    buf[3] = (uint8_t)(current & 0xff);
    buf[4] = flag;
    can_std_transmit(0x1f4, buf, 0, 8);
}

void RecivedChargerMessageProcess(CanRxMsg rx_msg)
{
    uint32_t id = GetRxExtId(rx_msg);

    if((id == 0x1CECFF3C) ||
       (id == 0x1CEBFF3C) ||
       (id == 0x1826F456))
    {
        SetRecivedChargerMessFlag(1);
    }

    switch(pgnManageFlow.status)
    {
    case 0:
        if(id == 0x1CECFF3C)
        {
            if(rx_msg.Data[0] == 0x20)
            {
                pgnManageFlow.pgn = rx_msg.Data[7];
                pgnManageFlow.pgn <<= 8;
                pgnManageFlow.pgn += rx_msg.Data[6];
                pgnManageFlow.pgn <<= 8;
                pgnManageFlow.pgn += rx_msg.Data[5];

                pgnManageFlow.dataNum = rx_msg.Data[2];
                pgnManageFlow.dataNum <<= 8;
                pgnManageFlow.dataNum += rx_msg.Data[1];

                pgnManageFlow.packNum = rx_msg.Data[3];

                if(pgnManageFlow.dataNum <= MAX_PGN_DATA_LEN)
                {
                    pgnManageFlow.status = 1;
                    pgnManageFlow.delay = CHARGE_RECIVED_DELAY;
                    memset(pgnManageFlow.data, 0, pgnManageFlow.dataNum);
                }
            }
        }
        break;
    case 1:
        if(pgnManageFlow.delay == 0)
        {
            pgnManageFlow.status = 0;
        }
        else if(id == 0x1CEBFF3C)
        {
            uint8_t offset;
            uint8_t num;

            offset = rx_msg.Data[0] - 1;
            offset *= 7;
            if(rx_msg.Data[0] < pgnManageFlow.packNum)
            {
                num = 7;
            }
            else
            {
                num = (uint8_t)(pgnManageFlow.dataNum - offset);
            }

            if(offset + num <= MAX_PGN_DATA_LEN)
            {
                memcpy(pgnManageFlow.data + offset, rx_msg.Data + 1, num);
                if(rx_msg.Data[0] >= pgnManageFlow.packNum)
                {
                    pgnManageFlow.status = 2;
                }
            }
        }
        break;
    case 2:
        if(pgnManageFlow.pgn == 0x01F100)
        {
            SetChargerInvalidFlag(1);
        }
        else if(pgnManageFlow.pgn == 0x02F100)
        {
            uint16_t temp;

            temp = pgnManageFlow.data[1];
            temp <<= 8;
            temp += pgnManageFlow.data[0];
            chargerMessage.voltage = temp;

            temp = pgnManageFlow.data[3];
            temp <<= 8;
            temp += pgnManageFlow.data[2];
            chargerMessage.current = temp;
            chargerMessage.fault1 = pgnManageFlow.data[4];
        }
        else if(pgnManageFlow.pgn == 0x04F100)
        {
            memcpy((uint8_t *)&chargerDetail, pgnManageFlow.data, sizeof(chargerDetail));
        }
        pgnManageFlow.status = 0;
        break;
    default:
        pgnManageFlow.status = 0;
        break;
    }
}

void ChargerMessageDelay(void)
{
    if(pgnManageFlow.delay != 0)
    {
        pgnManageFlow.delay--;
    }
}
