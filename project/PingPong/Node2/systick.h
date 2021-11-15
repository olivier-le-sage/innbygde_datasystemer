#ifndef SYSTICK_H__
#define SYSTICK_H__

#include <stdint.h>

typedef void (*systick_cb_t)(void);

typedef struct
{
    systick_cb_t cb;
    uint32_t period_10ms;
} systick_init_t;

void systick_init(const systick_init_t * params);

#endif // SYSTICK_H__