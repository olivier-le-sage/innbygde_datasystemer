#ifndef RTC_H__
#define RTC_H__

#include <stdint.h>


#define MCK_8_FACTOR_MS (10500)
#define MCK_8_FACTOR_FOR_TICK 105

void timer_init(void);
void timer_start(void);
void timer_stop(void);
uint32_t timer_ms_get(void);

#endif // RTC_H__