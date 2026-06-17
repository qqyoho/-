#ifndef CHARGER_PROTOCOL_H
#define CHARGER_PROTOCOL_H

#include <stdint.h>
#include "can.h"

typedef struct
{
    uint16_t voltage;
    uint16_t current;
    uint16_t fault1;
}CHARGER_MESSAGE_SEND;

#pragma pack(1)
typedef struct
{
    uint16_t voltage;
    uint16_t current;
    uint16_t soc;
    uint8_t fault;
    uint8_t flow;
    uint8_t bTemp;
    uint8_t cTemp;
    uint16_t voltageAc;
    uint8_t acHz;
    uint16_t type;
    uint8_t chargerStatus;
    uint16_t chargerCurve;
    uint8_t hVersion;
    uint8_t cSoc;
}CHARGER_MESSAGE_DETAIL;
#pragma pack()

extern CHARGER_MESSAGE_SEND chargerRequest;
extern CHARGER_MESSAGE_SEND chargerMessage;
extern CHARGER_MESSAGE_DETAIL chargerDetail;

void ChargerProtocolMemoryInit(void);
void ChargeFlowProcess(uint16_t voltage, uint16_t current, uint8_t flag);
void SetChargerInvalidFlag(uint8_t flag);
void SendChargeMessage(uint16_t voltage, uint16_t current, uint8_t flag);
void ChargerMessageDelay(void);
void ChargeFlowSendStopCharge(void);
void send_rystate_to_charger(uint16_t voltage, uint16_t current, uint8_t flag);
void RecivedChargerMessageProcess(CanRxMsg rx_msg);

#endif
