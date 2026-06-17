#ifndef _CH_HEART_DEFINE_
#define _CH_HEART_DEFINE_

#include <stdint.h>

void ChHeartMemInit(void);
uint8_t GetChHeartStatus(void);
void SetChHeartStatus(uint8_t flag);
void ChHeartTimer(void);
void ChHeartProc(void);
void ChHeartData(uint8_t data);

#endif
