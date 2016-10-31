#ifndef __SOFT_I2C_H__
#define __SOFT_I2C_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSYS.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvSPI.h"
#include "Packet.h"
#include "gpio_rw.h"

/************************** Type Prototypes **************************/

typedef struct _SoftI2C
{
    GPIO_T *data_port;
    uint32_t data_mask;
    GPIO_T *clock_port;
    uint32_t clock_mask;
} SoftI2C;

/********************** Function Prototypes **************************/

bool nonstatic_i2c_send_byte(SoftI2C * inst, uint8_t byte);

SoftI2C i2c_init(GPIO_T * data_port, uint32_t data_pin, GPIO_T * clock_port,
                 uint32_t clock_pin);

bool i2c_send(SoftI2C * inst, uint8_t address, uint8_t * data, uint8_t count);
bool i2c_send_packet(SoftI2C * inst, PACKETData * data);
bool i2c_received_stop_condition(SoftI2C * inst);

bool i2c_recv(SoftI2C * inst, uint8_t address, uint8_t * data, uint8_t count);

bool slave_i2c_recv(SoftI2C * inst, uint8_t * data, uint8_t count);
bool slave_i2c_recv_packet(SoftI2C * inst, PACKETData * data);

bool i2c_send_recv(SoftI2C * inst, uint8_t address, uint8_t * data_out,
                   uint8_t count_out, uint8_t * data_in, uint8_t count_in);

void i2c_data_high(SoftI2C * inst);
void i2c_data_low(SoftI2C * inst);

bool i2c_clock_read(SoftI2C * inst);
bool i2c_data_read(SoftI2C * inst);

#endif /* __SOFT_I2C_H__ */
