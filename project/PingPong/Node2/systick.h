#ifndef SYSTICK_H__
#define SYSTICK_H__

#include <stdint.h>
#include <stdbool.h>

typedef void (*systick_cb_t)(void);

typedef struct
{
    uint32_t period_10ms;
} systick_init_t;

void systick_init(const systick_init_t * params);
void systick_enable(void);
uint32_t systick_period_10ms_get(void);
bool systick_cb_register(systick_cb_t cb);

#endif // SYSTICK_H__