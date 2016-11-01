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


//**********************************************************************//
//							Module Definitions							//
//**********************************************************************//
#define I2C_SDA_PORT				(&GPIOB)
#define I2C_SDA_PIN					14
#define I2C_SDA_MASK				(1 << I2C_SDA_PIN)
#define I2C_SCK_PORT				(&GPIOB)
#define I2C_SCK_PIN					15
#define I2C_SCK_MASK				(1 << I2C_SCK_PIN)

/************************** Type Prototypes **************************/
typedef enum pin_enum {
	LOW,
	HIGH
} pin_t;

typedef enum i2c_state_enum {
	NO_STATE,
//	START_CONDITION,
//	REPEATED_START_CONDITION,
//	STOP_CONDITION,
	ADDRESS,
	READ_DATA,
	SEND_DATA,
	SEND_ADDRESS_ACK,
	SEND_DATA_ACK,
	SEND_NAK,
	ADDRESS_ACK_SENT,
	DATA_ACK_SENT
} i2c_state_t;

typedef struct _SoftI2C {
    GPIO_T *data_port;
    uint32_t data_mask;
    GPIO_T *clock_port;
    uint32_t clock_mask;
	pin_t sck_pin;
	pin_t sda_pin;
	i2c_state_t state;
	uint8_t bit;
	uint8_t address;
	uint8_t data;
} SoftI2C;

typedef enum transition_enum {
	SCK_ROSE,
	SCK_FELL,
	SDA_ROSE,
	SDA_FELL
} transition_t;

/********************** Function Prototypes **************************/

bool nonstatic_i2c_send_byte(uint8_t byte);

SoftI2C i2c_init(GPIO_T * data_port, uint32_t data_pin, GPIO_T * clock_port,
                 uint32_t clock_pin);

bool i2c_send(uint8_t address, uint8_t * data, uint8_t count);
bool i2c_send_packet(PACKETData * data);
bool i2c_received_stop_condition(void);

bool i2c_recv(uint8_t address, uint8_t * data, uint8_t count);

bool slave_i2c_recv(uint8_t * data, uint8_t count);
bool slave_i2c_recv_packet(PACKETData * data);

bool i2c_send_recv(uint8_t address, uint8_t * data_out,
                   uint8_t count_out, uint8_t * data_in, uint8_t count_in);

void i2c_data_high(void);
void i2c_data_low(void);

bool i2c_clock_read(void);
bool i2c_data_read(void);

bool i2c_update_slave_state(transition_t transition);

#endif /* __SOFT_I2C_H__ */
