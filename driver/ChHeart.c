#include "ChHeart.h"

#define CH_HEART_TIMER_TIMES      10
#define CH_HEART_MAX_TIME         (3000 / CH_HEART_TIMER_TIMES)

static uint8_t chHeartStatus;
static volatile uint16_t chHeartTimer;

void ChHeartMemInit(void)
{
    chHeartStatus = 0;
    chHeartTimer = 0;
}

uint8_t GetChHeartStatus(void)
{
    return chHeartStatus;
}

void SetChHeartStatus(uint8_t flag)
{
    if(flag == 0)
    {
        chHeartStatus = 0;
        chHeartTimer = 0;
    }
    else
    {
        chHeartStatus = 1;
        chHeartTimer = CH_HEART_MAX_TIME;
    }
}

void ChHeartTimer(void)
{
    if(chHeartTimer != 0)
    {
        chHeartTimer--;
    }
}

void ChHeartProc(void)
{
    if(chHeartTimer == 0)
    {
        chHeartStatus = 0;
    }
}

void ChHeartData(uint8_t data)
{
    SetChHeartStatus(data);
}
