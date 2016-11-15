#include "soft_i2c.h"

// Implements software-based I2C communication protocol.

// I2C Bus Timing - uS
#define I2C_BASE_TIME	  17
#define I2C_START_DELAY   I2C_BASE_TIME
#define I2C_STOP_DELAY    I2C_BASE_TIME
#define I2C_DATA_SETTLE   I2C_BASE_TIME/2
#define I2C_CLOCK_HIGH    I2C_BASE_TIME * 2
#define I2C_CLOCK_LOW     I2C_BASE_TIME * 2
#define I2C_HALF_CLOCK    I2C_BASE_TIME
#define I2C_STRETCH_MAX   500

//**********************************************************************//
//							Private Variables							//
//**********************************************************************//
static SoftI2C i2c = {
	(GPIO_T*)0, // data_port
    0,			// data_mask
    (GPIO_T*)0,	// clock_port
    0,			// clock_mask
	HIGH,		// sck_pin
	HIGH,		// sda_pin
	NO_STATE	// state
};

//**********************************************************************//
//							Private Functions							//
//**********************************************************************//

static __INLINE void
i2c_delay(uint32_t delay) {
    while (delay--);
}

// TODO make sure clock_pin is set up as a mask.
static __INLINE void i2c_clock_high(void) {
    DrvGPIO_SetOutputBit(i2c.clock_port, i2c.clock_mask);
}

// TODO make sure clock_pin is set up as a mask.
static __INLINE void i2c_clock_low(void) {
    DrvGPIO_ClearOutputBit(i2c.clock_port, i2c.clock_mask);
}

__INLINE bool
i2c_clock_read(void) {
    return DrvGPIO_GetInputPinValue(i2c.clock_port, i2c.clock_mask) & i2c.clock_mask ? true : false;
}

__INLINE void
i2c_data_high(void) {
	DrvGPIO_SetOutputBit(i2c.data_port, i2c.data_mask);
}

__INLINE void
i2c_data_low(void) {
    DrvGPIO_ClearOutputBit(i2c.data_port, i2c.data_mask);
}

__INLINE bool
i2c_data_read(void) {
    return DrvGPIO_GetInputPinValue(i2c.data_port, i2c.data_mask) & i2c.data_mask ? true : false;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_clock_high(void) {
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && !i2c_clock_read());
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_clock_low(void) {
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && i2c_clock_read());
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_data_high(void) {
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && !i2c_data_read());
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_data_low(void) {
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && i2c_data_read());
    if (wait == 0)
        return false;
    return true;
}

static void
i2c_stretch(void) {
    uint32_t wait = I2C_STRETCH_MAX;

    // Clock stretching is where the I2C slave pulls the SCL line low to
    // prevent the master from clocking.  If the slave is holding the 
    // clock low, we need to wait until the clock is released by the 
    // slave before continuing.
    while (wait-- && !i2c_clock_read());
}

static void
i2c_clock(void) {
    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    i2c_clock_high();

    // Handle clock stretching by the slave.
    i2c_stretch();

    // Minimum clock high time.
    i2c_delay(I2C_CLOCK_HIGH);

    // Lower the clock.
    i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
}

static void
inverted_i2c_clock(void) {
    // Minimum clock low time.
//    i2c_delay(I2C_DATA_SETTLE);

	// TODO check that clock is low?
    // Lower the clock.
    i2c_clock_low();
	
	// Handle clock stretching by the slave.
    i2c_stretch();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
	
    // Raise the clock.
    i2c_clock_high();

    // Handle clock stretching by the slave.
    i2c_stretch();

    // Minimum clock high time.
    i2c_delay(I2C_CLOCK_HIGH);
}

// NOTE (brandon) : Added to support slave read.
static void
wait_for_i2c_clock(void) {
    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    wait_for_i2c_clock_high();

    // Minimum clock high time.
    i2c_delay(I2C_CLOCK_HIGH);

    // Lower the clock.
    wait_for_i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
}

static void
i2c_start(void) {
    // Hold bus idle for a bit.
    i2c_clock_high();
    i2c_data_high();
    i2c_delay(I2C_START_DELAY);

    // Generate the start condition.
    i2c_data_low();
    i2c_delay(I2C_START_DELAY);

    // Minimum clock low time.
    i2c_clock_low();
    i2c_delay(I2C_CLOCK_LOW);
}

static void
i2c_stop(void) {
    // Generate stop condition.
    i2c_clock_high();
    i2c_data_low();
    i2c_stretch();
    i2c_delay(I2C_STOP_DELAY);
    i2c_data_high();
}

static uint8_t
i2c_read_bit(void) {
    uint8_t data;

    // Make sure the data pin is released to receive a bit.
    i2c_data_high();

    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    i2c_clock_high();

    // Handle clock stretching by the slave.
    i2c_stretch();

    // Wait have a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Read in the data bit.
    data = i2c_data_read() ? 1 : 0;

    // Wait for the rest of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Lower the clock.
    i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return data;
}

// NOTE (brandon) : Added to support slave read.
static uint8_t
slave_i2c_read_bit(void) {
    uint8_t data;

    // Make sure the data pin is released to receive a bit.
    i2c_data_high();

    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    wait_for_i2c_clock_high();

    // Wait have a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Read in the data bit.
    data = i2c_data_read() ? 1 : 0;

    // Lower the clock.
    wait_for_i2c_clock_low();

    // Read in the data bit.
    // XXX data = i2c_data_read() ? 1 : 0;

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return data;
}

static bool
i2c_get_ack(void) {
    bool received_ack;

    // Ensure clock is low.
    i2c_clock_low();

    // Release the Data pin so slave can ACK.
    i2c_data_high();

    // Raise the clock pin.
    i2c_clock_high();

    // Handle clock stretching.
    i2c_stretch();

    // Wait half a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Sample the ACK signal.
    received_ack = i2c_data_read() ? false : true;

    // Wait for the rest of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Finish the clock pulse.
    i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return received_ack;
}

static void
i2c_send_ack(void) {
    // Send Ack to slave.
    i2c_data_low();

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    i2c_clock();

    // Release Ack.
    i2c_data_low();

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

// NOTE (brandon) : Added to support slave read.
static void
slave_i2c_send_ack(void) {
    // Send Ack to slave.
    i2c_data_low();

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    wait_for_i2c_clock();

    // Release Ack.
    // TODO (brandon) : should this be i2c_data_high() ??
    // XXX i2c_data_low();
    i2c_data_high();

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

static void
i2c_send_nak(void) {
    // Send Nak to slave except for last byte.
    i2c_data_high();

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    i2c_clock();

    // Release Nak.
    i2c_data_low();

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

// NOTE (brandon) : Added to support slave read.
static void
slave_i2c_send_nak(void) {
    // Send Nak to slave except for last byte.
    i2c_data_high();

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    wait_for_i2c_clock();

    // Release Nak.
    // TODO (brandon) : should this be i2c_data_high() ??
    // XXX i2c_data_low();
    i2c_data_high();

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

static bool i2c_send_byte(uint8_t byte) {
    uint8_t i;

    // Always start with clock and data low.
    i2c_data_low();
    i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
	
    // Send 8 bits of data.
    for (i = 0; i < 8; ++i)
    {
        // Set the bit.
        if (byte & 0x80)
            i2c_data_high();
        else
            i2c_data_low();

        // Shift next bit into position.
        byte = byte << 1;

        // Pulse the clock.
        i2c_clock();
//		inverted_i2c_clock();
    }

    // Release data pin for ACK.
    i2c_data_low();

    return true;
}

bool nonstatic_i2c_send_byte(uint8_t byte) {
	i2c_send_byte(byte);
	return TRUE;
}

static uint8_t i2c_get_byte(void) {
    uint8_t i;
    uint8_t byte = 0;

    // Make sure the data and clock lines are pulled low.
    i2c_data_low();
    i2c_clock_low();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    // Read 8 bits of data.
    for (i = 0; i < 8; ++i) {
        // Shift bits into position.
        byte = byte << 1;

        // Set the next bit.
        byte |= i2c_read_bit();
    }

    return byte;
}

// NOTE (brandon) : Added to support slave read.
static uint8_t
slave_i2c_get_byte(void) {
    uint8_t i;
    uint8_t byte = 0;

    // Make sure the data and clock lines are released
    i2c_data_high();
    i2c_clock_high();

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    // Read 8 bits of data.
    for (i = 0; i < 8; ++i) {
        // Shift bits into position.
        byte = byte << 1;

        // Set the next bit.
        byte |= slave_i2c_read_bit();
    }

    return byte;
}

//**********************************************************************//
//							Public Functions							//
//**********************************************************************//

// Handle the GPIO interrupt. Print a message and clear the int flags.
void GPAB_IRQHandler(void) {
	uint16_t int_flags = DrvGPIO_GetIntFlag(I2C_SCK_PORT, 0xFFFF);
	
	// Did interrupt fire from a clock transition?
	if(int_flags & I2C_SCK_MASK) {
		DrvGPIO_ClearIntFlag(I2C_SCK_PORT, I2C_SCK_MASK);
		
		// Did clock rise from low to high?
		if(i2c_clock_read()) {
			// The buttons aren't debounced, sometimes we get two of the same transition. Guard for that.
			if(i2c.sck_pin == LOW) {
				i2c_update_slave_state(SCK_ROSE);
			}
		} else {
			// The buttons aren't debounced, sometimes we get two of the same transition. Guard for that.
			if(i2c.sck_pin == HIGH) {
				i2c_update_slave_state(SCK_FELL);
			}
		}
	}
	
	// Did interrupt fire from a data transition?
	if(int_flags & I2C_SDA_MASK) {
		DrvGPIO_ClearIntFlag(I2C_SDA_PORT, I2C_SDA_MASK);
		
		// Did clock rise from low to high?
		if(i2c_data_read()) {
			// The buttons aren't debounced, sometimes we get two of the same transition. Guard for that.
			if(i2c.sda_pin == LOW) {
				i2c_update_slave_state(SDA_ROSE);
			}
		} else {
			// The buttons aren't debounced, sometimes we get two of the same transition. Guard for that.
			if(i2c.sda_pin == HIGH) {
				i2c_update_slave_state(SDA_FELL);
			}
		}
	}
}



bool i2c_update_slave_state(transition_t transition) {
	uint8_t write_bit;
	if(transition == SCK_ROSE) {
		PRINTD("sck /\\\n");
		i2c.sck_pin = HIGH;
		switch(i2c.state) {
			case NO_STATE:
				i2c_data_high();
				break;
			// Should we read in the address?
			case ADDRESS:
				// Read in the address bits.
				if(i2c.bit <= 6) {
					i2c.address |= i2c.sda_pin << (6 - i2c.bit);
					PRINTD("addr = 0x%x\n", i2c.address);
				// Does master want to read or write?
				} else if(i2c.bit == 7) {
					if((i2c.sda_pin == HIGH) && (i2c.address == 0x0d)) {
//					if(i2c.sda_pin == HIGH) {
						i2c.data_state = WRITE;
						i2c.state = SEND_ADDRESS_ACK;
					} else {
						i2c_data_high();
						i2c.state = NO_STATE;
						PRINTD("reading from address 0x%x\n", i2c.address);
					}
				} else {
					PRINTD("ERROR: address bit is too high (%d)\n", i2c.bit);
					for(;;);
				}
				
				// This is how we keep track of how many bits we have read in. Increment it.
				i2c.bit++;
				break;
			
			// Is this the address ack bit?
			case ADDRESS_ACK_SENT:
				// Double check that our ack is working.
				if(i2c.sda_pin == HIGH) {
					PRINTD("ERROR we failed to send an ack.\n");
					for(;;);
				} else {
					PRINTD("sending an ack correctly!\n");
				}
				break;

			// Are we in the data portion of the message?
			case DATA:
				// Should we read in the data?
				if(i2c.data_state == READ) {
					if(i2c.bit <= 7) {
						i2c.data |= i2c.sda_pin << (7 - i2c.bit);
						PRINTD("read data = 0x%x\n", i2c.data);
						i2c.bit++;
						if(i2c.bit >= 8) {
							i2c.state = SEND_DATA_ACK;
						}
					} else {
						PRINTD("ERROR: data bit is too high (%d)\n", i2c.bit);
						for(;;);
					}
					// Else do nothing
				}
				break;
	
			// Is this the data ack bit?
			case DATA_ACK_SENT:
				// Double check that our ack is working.
				if(i2c.sda_pin == HIGH) {
					PRINTD("ERROR we failed to send an ack.\n");
					for(;;);
				} else {
					PRINTD("sending an ack correctly!\n");
				}
				break;
			default:
				break;
		}
	} else if(transition == SCK_FELL) {
		PRINTD("sck \\/\n");
		i2c.sck_pin = LOW;
		if(i2c.state == SEND_ADDRESS_ACK) {
			PRINTD("trying to keep sda low\n");
			i2c_data_low();
			if(i2c.data_state == READ) {
				i2c.state = ADDRESS_ACK_SENT;
			} else {
				i2c.state = DATA;
				i2c.bit = 0;
			}
		}
		else if(i2c.state == SEND_DATA_ACK) {
			PRINTD("trying to keep sda low\n");
			i2c_data_low();
			i2c.state = DATA_ACK_SENT;
		}
		else if(i2c.state == ADDRESS_ACK_SENT) {
			i2c_data_high();
			i2c.state = DATA;
			i2c.bit = 0;
			PRINTD("going to data\n");
		}
		else if(i2c.state == DATA_ACK_SENT) {
			i2c_data_high();
			i2c.state = NO_STATE;
			i2c.bit = 0;
			PRINTD("%x %x\n", i2c.address, i2c.data);
//			printf("%x %x\n", i2c.address, i2c.data);
		} else if(i2c.state == DATA) {
			if(i2c.data_state == WRITE) {
				if(i2c.bit <= 7) {
					i2c.data = i2c.address;
					write_bit = i2c.data >> (7 - i2c.bit);
					write_bit = write_bit & 0x01;
					PRINTD("wb %u\n", write_bit);
					if(write_bit) {
						i2c_data_high();
					} else {
						i2c_data_low();
					}
					i2c.bit++;
					if(i2c.bit >= 8) {
						i2c.state = SEND_DATA_ACK;
					}
				} else {
					PRINTD("ERROR: data bit is too high (%d)\n", i2c.bit);
					for(;;);
				}
			} // Else do nothing.
		} else if(i2c.state == NO_STATE) {
			i2c_data_high();
		}// Else do nothing.
	} else if(transition == SDA_ROSE) {
		PRINTD("sda /\\\n");
		i2c.sda_pin = HIGH;
		if(i2c.sck_pin == HIGH) {
			PRINTD("stop condition\n");
			i2c.state = NO_STATE;
		}
	} else if(transition == SDA_FELL) {
		PRINTD("sda \\/\n");
		i2c.sda_pin = LOW;
		if(i2c.sck_pin == HIGH) {
			PRINTD("start condition\n");
			i2c.state = ADDRESS;
			i2c.bit = i2c.data = i2c.address = 0;
		}
	} else {
		PRINTD("error illegal transition");
		return 0;
	}
	return 1;
}

SoftI2C i2c_init(GPIO_T * data_port, uint32_t data_mask, GPIO_T * clock_port, uint32_t clock_mask) {
	uint8_t data = 0x6A;

    i2c.data_port = data_port;
    i2c.clock_port = clock_port;
    i2c.data_mask = data_mask;
    i2c.clock_mask = clock_mask;

    // Set the modes to open drain so we can sink current.
#if I2C_SDA_PIN != 14
	#ERROR
#else
//	DrvGPIO_SetIOMode(i2c.data_port, DRVGPIO_IOMODE_PIN14_OPEN_DRAIN);
#endif

#if I2C_SCK_PIN != 15
	#ERROR
#else
//	DrvGPIO_SetIOMode(i2c.clock_port, DRVGPIO_IOMODE_PIN15_OPEN_DRAIN);
#endif

    // Make sure SDA and SCL are set high.
    i2c_clock_high();
	i2c_data_high();

#ifdef MASTER
	for(;;) {
//		nonstatic_i2c_send_byte(&i2c_port, 0x0A);
//		i2c_send(0x56, &data, 1);
		i2c_recv(0x0d, &data, 1);
		PRINTD("received data: 0x%x\n", data);

	}
#endif

    return i2c;
}

// This is only called by i2c interrupt, so it's a stop condition if the clock is high.
bool i2c_received_stop_condition(void) {
	// Is SCK high?
	return (DrvGPIO_GetInputPinValue(i2c.clock_port, i2c.clock_mask) != 0);
}

// TODO (brandon) : Renable the ACK checks.
bool
i2c_send(uint8_t address, uint8_t * data, uint8_t count) {
    __disable_irq();

    // Send the start condition.
    i2c_start();

    // Send the address with the read/write bit reset (write).
    i2c_send_byte((address << 1) & 0xFE);

    // Was the address aknowledged?
    if (!i2c_get_ack())
    {
        // No. Stop the transaction.
        /*
         * i2c_stop();
         * __enable_irq();
         * return false;
         */
    }

    // Send each data byte.
    while (count--)
    {
        // Send the next byte.
        i2c_send_byte(*(data++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack())
        {
            // Stop the transaction.
            /*
             * i2c_stop();
             * __enable_irq();
             * return false;
             */
        }
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}

// TODO (brandon) : Renable the ACK checks.
bool
i2c_send_packet(PACKETData * data) {
    // NOTE (brandon) : Need to add 2 to data->length
    // to add the length and checksum fields.
    uint8_t count = data->length + 2;
    uint8_t *send_data = (uint8_t *) data;

    __disable_irq();

    // Send the start condition.
    i2c_start();

    // Send each data byte.
    while (count--)
    {
        // Send the next byte.
        i2c_send_byte(*(send_data++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack())
        {
            // Stop the transaction.
            /*
             * i2c_stop();
             * __enable_irq();
             * return false;
             */
        }
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}

bool
i2c_recv(uint8_t address, uint8_t * data, uint8_t count) {
    __disable_irq();

    // Send the start condition.
    i2c_start();

    // Send the address with the read/write bit set (read).
    i2c_send_byte((address << 1) | 0x01);

    // Was the address aknowledged?
    if (!i2c_get_ack())
    {
        // No. Stop the transaction.
        i2c_stop();
        __enable_irq();
        return false;
    }

    // Receive each data byte.
    while (count--)
    {
        // Get the next byte.
        *(data++) = i2c_get_byte();

        // Send Ack unless this is the last byte.
        if (count > 0)
            i2c_send_ack();
        else
            i2c_send_nak();
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}

// NOTE (brandon) : Added slave function.
bool
slave_i2c_recv(uint8_t * data, uint8_t count) {
    __disable_irq();

    // Release the sda and clock lines
    i2c_data_high();
    i2c_clock_high();

    // TODO (brandon) : Wait for start signal.
    // Wait for both SDA and CLK to go low.
    if (wait_for_i2c_data_low() == false)
        return false;
    if (wait_for_i2c_clock_low() == false)
        return false;

    // increment count to get the first
    // byte which is the address.
    count++;

    // Receive each data byte.
    while (count--)
    {
        // Get the next byte.
        *(data++) = slave_i2c_get_byte();

        // Send Ack unless this is the last byte.
        if (count > 0)
            slave_i2c_send_ack();
        else
            slave_i2c_send_nak();
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}

// NOTE (brandon) : Added slave function.
bool
slave_i2c_recv_packet(PACKETData * data) {
    uint8_t count = 0;
    uint8_t *recv_data = (uint8_t *) data;
    __disable_irq();

    // Release the sda and clock lines
    i2c_data_high();
    i2c_clock_high();

    // TODO (brandon) : Wait for start signal.
    // Wait for both SDA and CLK to go low.
    if (wait_for_i2c_data_low() == false)
        return false;
    if (wait_for_i2c_clock_low() == false)
        return false;

    // Receive each data byte.
    // Assumes there is at least one byte the packet.
    // The first byte is always the length of the packet.
    // data->length actually is the length of packet->buffer
    // so we need to add 2 for the extra length and checksum
    // fields.
    while (1)
    {
        // Get the next byte.
        *(recv_data++) = slave_i2c_get_byte();

        // Send Ack unless this is the last byte.
        if (count > 0)
            slave_i2c_send_ack();
        else
            slave_i2c_send_nak();

        count++;
        if (count >= data->length + 2)
            break;
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}

bool
i2c_send_recv(uint8_t address, uint8_t * data_out,
              uint8_t count_out, uint8_t * data_in, uint8_t count_in) {
    __disable_irq();

    // Send the start condition.
    i2c_start();

    // Send the address with the read/write bit reset (write).
    i2c_send_byte((address << 1) & 0xFE);

    // Was the address aknowledged?
    if (!i2c_get_ack())
    {
        i2c_stop();
        __enable_irq();
        return false;
    }

    // Send each of the outgoing bytes.
    while (count_out--)
    {
        // Send the next byte.
        i2c_send_byte(*(data_out++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack())
        {
            i2c_stop();
            __enable_irq();
            return false;
        }
    }

    // Repeat the start.
    i2c_start();

    // Send the address with the read/write bit set (read).
    i2c_send_byte((address << 1) | 0x01);

    // Was the address aknowledged?
    if (!i2c_get_ack())
    {
        i2c_stop();
        __enable_irq();
        return false;
    }

    // Receive each of the incoming bytes.
    while (count_in--)
    {
        // Get the next byte.
        *(data_in++) = i2c_get_byte();

        // Send Ack unless this is the last byte.
        if (count_in > 0)
            i2c_send_ack();
        else
            i2c_send_nak();
    }

    // Send the stop condition.
    i2c_stop();

    __enable_irq();

    return true;
}
