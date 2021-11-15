#ifndef UTILS_H__
#define UTILS_H__

#include <stdint.h>

#define _FORCE_UINT8(v) ((uint8_t) ((v) & UINT8_MAX))
#define _STRINGIFY(v) #v
#define STRINGIFY(v) _STRINGIFY(v)

#endif