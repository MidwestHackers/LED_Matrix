#ifndef __MAX_7219_H__
#define __MAX_7219_H__

/*
    Matrix Layout
        Dx -> Digit x
        Bx -> Bit x

    D0:B7   D1:B7   ...     ...     D7:B7
    D0:B6   D1:B6   ...     ...     D7:B6
    ...     ...     ...     ...     ...
    ...     ...     ...     ...     ...
    D0:B0   D1:B0   ...     ...     D7:B0
*/

#include "periph.h"

#define MAX_7219_DISPLAY_LEN 4

typedef enum {
    status_ok = 0x00,
    status_busy = 0x01,
    error_unknown = 0xff,
} max_7219_status_t;

typedef enum {
    bus_ready = 0x00,
    bus_busy = 0x01,
} max_7219_bus_status_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_gpio_bank;
    uint16_t cs_gpio_pin;
    volatile max_7219_bus_status_t bus_status;
    void (*tx_done_handler)(void);
    uint8_t tx_data[MAX_7219_DISPLAY_LEN * 2];
} max_7219_handle_t;

typedef enum {
     no_op          = 0x00,
     digit_0        = 0x01,
     digit_1        = 0x02,
     digit_2        = 0x03,
     digit_3        = 0x04,
     digit_4        = 0x05,
     digit_5        = 0x06,
     digit_6        = 0x07,
     digit_7        = 0x08,
     decode_mode    = 0x09,
     intensity      = 0x0A,
     scan_limit     = 0x0B,
     shutdown       = 0x0C,
     display_test   = 0x0F,
} max_7219_reg_addr_t;

void max_7219_init (max_7219_handle_t *max);
max_7219_status_t max_7219_write_all (max_7219_handle_t *max, uint8_t reg, uint8_t data);
max_7219_status_t max_7219_test_mode (max_7219_handle_t *max, uint8_t enable);
max_7219_status_t max_7219_display_data (max_7219_handle_t *max, uint8_t enable);

#endif // __MAX_7219_H__
