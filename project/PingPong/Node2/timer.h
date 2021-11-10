#ifndef RTC_H__
#define RTC_H__

#include <stdint.h>

void timer_init(void);
void timer_start(void);
void timer_stop(void);
uint32_t timer_ms_get(void);

#endif // RTC_H__