#ifndef EXT_PERIPHERALS_H__
#define EXT_PERIPHERALS_H__

#include <stdint.h>

#define EXT_OLED_MEM_START 0x1000
#define EXT_OLED_MEM_SIZE 1024

#define EXT_OLED_CMD_MEM_START 0x1000
#define EXT_OLED_CMD_MEM_SIZE 512

#define EXT_OLED_DATA_MEM_START 0x1200
#define EXT_OLED_DATA_MEM_SIZE 512

#define EXT_ADC_MEM_START 0x1400
#define EXT_ADC_MEM_SIZE 1024

#define EXT_SRAM_MEM_START 0x1800
#define EXT_SRAM_MEM_SIZE 2048

typedef struct __attribute__((packed,aligned(1))) {
  uint8_t CMD;
  uint8_t _unused_cmd[EXT_OLED_CMD_MEM_SIZE - sizeof(uint8_t)];
  uint8_t DATA;
  uint8_t _unused_data[EXT_OLED_DATA_MEM_SIZE - sizeof(uint8_t)];
} ext_oled_t;

static volatile ext_oled_t *const EXT_OLED = (volatile ext_oled_t *)EXT_OLED_MEM_START;
static volatile uint8_t *const EXT_ADC = (volatile uint8_t *)EXT_ADC_MEM_START;
static volatile uint8_t *const EXT_SRAM = (volatile uint8_t *)EXT_SRAM_MEM_START;

#endif /* EXT_PERIPHERALS_H__ */