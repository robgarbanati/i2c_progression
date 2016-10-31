#include "soft_i2c.h"

// Implements software-based I2C communication protocol.

// I2C Bus Timing - uS
#define I2C_BASE_TIME	  12
#define I2C_START_DELAY   I2C_BASE_TIME
#define I2C_STOP_DELAY    I2C_BASE_TIME
#define I2C_DATA_SETTLE   1
#define I2C_CLOCK_HIGH    I2C_BASE_TIME * 2
#define I2C_CLOCK_LOW     I2C_BASE_TIME * 2
#define I2C_HALF_CLOCK    I2C_BASE_TIME
#define I2C_STRETCH_MAX   500

#define GPIO_SDA_PIN      GPIO_Pin_3
#define GPIO_SCL_PIN      GPIO_Pin_0

#define GPIO_SDA_PORT     GPIOA
#define GPIO_SCL_PORT     GPIOA

// Clock and data pins.
/*
static GPIO_T *m_data_port;
static GPIO_T *m_clock_port;
static uint32_t m_data_pin;
static uint32_t m_clock_pin;
*/

static __INLINE void
i2c_delay(uint32_t delay)
{
    while (delay--);
}

// TODO make sure clock_pin is set up as a mask.
static __INLINE void i2c_clock_high(SoftI2C * inst) {
    DrvGPIO_SetOutputBit(inst->clock_port, inst->clock_pin);
}

// TODO make sure clock_pin is set up as a mask.
static __INLINE void i2c_clock_low(SoftI2C * inst) {
    DrvGPIO_ClearOutputBit(inst->clock_port, inst->clock_pin);
}

static __INLINE bool
i2c_clock_read(SoftI2C * inst)
{
    return DrvGPIO_GetOutputPinValue(inst->clock_port, inst->clock_pin) & inst->data_pin ? true : false;
}

__INLINE void
i2c_data_high(SoftI2C * inst)
{
	DrvGPIO_SetOutputBit(inst->data_port, inst->data_pin);
}

__INLINE void
i2c_data_low(SoftI2C * inst)
{
    DrvGPIO_ClearOutputBit(inst->data_port, inst->data_pin);
}

static __INLINE bool
i2c_data_read(SoftI2C * inst)
{
    return DrvGPIO_GetOutputPinValue(inst->data_port, inst->data_pin) & inst->data_pin ? true : false;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_clock_high(SoftI2C * inst)
{
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && !i2c_clock_read(inst));
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_clock_low(SoftI2C * inst)
{
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && i2c_clock_read(inst));
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_data_high(SoftI2C * inst)
{
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && !i2c_data_read(inst));
    if (wait == 0)
        return false;
    return true;
}

// NOTE (brandon) : Added to support slave read.
static bool
wait_for_i2c_data_low(SoftI2C * inst)
{
    uint32_t wait = I2C_STRETCH_MAX;
    while (wait-- && i2c_data_read(inst));
    if (wait == 0)
        return false;
    return true;
}

static void
i2c_stretch(SoftI2C * inst)
{
    uint32_t wait = I2C_STRETCH_MAX;

    // Clock stretching is where the I2C slave pulls the SCL line low to
    // prevent the master from clocking.  If the slave is holding the 
    // clock low, we need to wait until the clock is released by the 
    // slave before continuing.
    while (wait-- && !i2c_clock_read(inst));
}

static void
i2c_clock(SoftI2C * inst)
{
    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    i2c_clock_high(inst);

    // Handle clock stretching by the slave.
    i2c_stretch(inst);

    // Minimum clock high time.
    i2c_delay(I2C_CLOCK_HIGH);

    // Lower the clock.
    i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
}

// NOTE (brandon) : Added to support slave read.
static void
wait_for_i2c_clock(SoftI2C * inst)
{
    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    wait_for_i2c_clock_high(inst);

    // Minimum clock high time.
    i2c_delay(I2C_CLOCK_HIGH);

    // Lower the clock.
    wait_for_i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);
}

static void
i2c_start(SoftI2C * inst)
{
    // Hold bus idle for a bit.
    i2c_clock_high(inst);
    i2c_data_high(inst);
    i2c_delay(I2C_START_DELAY);

    // Generate the start condition.
    i2c_data_low(inst);
    i2c_delay(I2C_START_DELAY);

    // Minimum clock low time.
    i2c_clock_low(inst);
    i2c_delay(I2C_CLOCK_LOW);
}

static void
i2c_stop(SoftI2C * inst)
{
    // Generate stop condition.
    i2c_clock_high(inst);
    i2c_data_low(inst);
    i2c_stretch(inst);
    i2c_delay(I2C_STOP_DELAY);
    i2c_data_high(inst);
}

static uint8_t
i2c_read_bit(SoftI2C * inst)
{
    uint8_t data;

    // Make sure the data pin is released to receive a bit.
    i2c_data_high(inst);

    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    i2c_clock_high(inst);

    // Handle clock stretching by the slave.
    i2c_stretch(inst);

    // Wait have a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Read in the data bit.
    data = i2c_data_read(inst) ? 1 : 0;

    // Wait for the rest of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Lower the clock.
    i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return data;
}

// NOTE (brandon) : Added to support slave read.
static uint8_t
slave_i2c_read_bit(SoftI2C * inst)
{
    uint8_t data;

    // Make sure the data pin is released to receive a bit.
    i2c_data_high(inst);

    // Minimum clock low time.
    i2c_delay(I2C_DATA_SETTLE);

    // Raise the clock.
    wait_for_i2c_clock_high(inst);

    // Wait have a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Read in the data bit.
    data = i2c_data_read(inst) ? 1 : 0;

    // Lower the clock.
    wait_for_i2c_clock_low(inst);

    // Read in the data bit.
    // XXX data = i2c_data_read(inst) ? 1 : 0;

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return data;
}

static bool
i2c_get_ack(SoftI2C * inst)
{
    bool received_ack;

    // Ensure clock is low.
    i2c_clock_low(inst);

    // Release the Data pin so slave can ACK.
    i2c_data_high(inst);

    // Raise the clock pin.
    i2c_clock_high(inst);

    // Handle clock stretching.
    i2c_stretch(inst);

    // Wait half a clock to sample in the middle of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Sample the ACK signal.
    received_ack = i2c_data_read(inst) ? false : true;

    // Wait for the rest of the high clock.
    i2c_delay(I2C_HALF_CLOCK);

    // Finish the clock pulse.
    i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    return received_ack;
}

static void
i2c_send_ack(SoftI2C * inst)
{
    // Send Ack to slave.
    i2c_data_low(inst);

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    i2c_clock(inst);

    // Release Ack.
    i2c_data_low(inst);

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

// NOTE (brandon) : Added to support slave read.
static void
slave_i2c_send_ack(SoftI2C * inst)
{
    // Send Ack to slave.
    i2c_data_low(inst);

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    wait_for_i2c_clock(inst);

    // Release Ack.
    // TODO (brandon) : should this be i2c_data_high(inst) ??
    // XXX i2c_data_low(inst);
    i2c_data_high(inst);

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

static void
i2c_send_nak(SoftI2C * inst)
{
    // Send Nak to slave except for last byte.
    i2c_data_high(inst);

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    i2c_clock(inst);

    // Release Nak.
    i2c_data_low(inst);

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

// NOTE (brandon) : Added to support slave read.
static void
slave_i2c_send_nak(SoftI2C * inst)
{
    // Send Nak to slave except for last byte.
    i2c_data_high(inst);

    // Give it time to settle.
    i2c_delay(I2C_DATA_SETTLE);

    // Pulse the clock.
    wait_for_i2c_clock(inst);

    // Release Nak.
    // TODO (brandon) : should this be i2c_data_high(inst) ??
    // XXX i2c_data_low(inst);
    i2c_data_high(inst);

    // Gap between next byte.
    i2c_delay(I2C_DATA_SETTLE);
}

static bool i2c_send_byte(SoftI2C * inst, uint8_t byte) {
    uint8_t i;

    // Always start with clock and data low.
    i2c_data_low(inst);
    i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    // Send 8 bits of data.
    for (i = 0; i < 8; ++i)
    {
        // Set the bit.
        if (byte & 0x80)
            i2c_data_high(inst);
        else
            i2c_data_low(inst);

        // Shift next bit into position.
        byte = byte << 1;

        // Pulse the clock.
        i2c_clock(inst);
    }

    // Release data pin for ACK.
    i2c_data_low(inst);

    return true;
}

bool nonstatic_i2c_send_byte(SoftI2C * inst, uint8_t byte) {
	i2c_send_byte(inst, byte);
	return TRUE;
}

static uint8_t i2c_get_byte(SoftI2C * inst) {
    uint8_t i;
    uint8_t byte = 0;

    // Make sure the data and clock lines are pulled low.
    i2c_data_low(inst);
    i2c_clock_low(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    // Read 8 bits of data.
    for (i = 0; i < 8; ++i)
    {
        // Shift bits into position.
        byte = byte << 1;

        // Set the next bit.
        byte |= i2c_read_bit(inst);
    }

    return byte;
}

// NOTE (brandon) : Added to support slave read.
static uint8_t
slave_i2c_get_byte(SoftI2C * inst)
{
    uint8_t i;
    uint8_t byte = 0;

    // Make sure the data and clock lines are released
    i2c_data_high(inst);
    i2c_clock_high(inst);

    // Minimum clock low time.
    i2c_delay(I2C_CLOCK_LOW);

    // Read 8 bits of data.
    for (i = 0; i < 8; ++i)
    {
        // Shift bits into position.
        byte = byte << 1;

        // Set the next bit.
        byte |= slave_i2c_read_bit(inst);
    }

    return byte;
}

SoftI2C i2c_init(GPIO_T * data_port, uint32_t data_pin, GPIO_T * clock_port, uint32_t clock_pin) {
    SoftI2C inst;
    // Validate the pin values.
    // XXX if ((data_pin > 15) || (clock_pin > 15)) return;

    // Save the ports and pin numbers.
    /*
     * m_data_port = data_port;
     * m_clock_port = clock_port;
     * m_data_pin = 1 << data_pin;
     * m_clock_pin = 1 << clock_pin;
     */
    inst.data_port = data_port;
    inst.clock_port = clock_port;
    inst.data_pin = 1 << data_pin;
    inst.clock_pin = 1 << clock_pin;

    // Set the modes to open drain so we can sink current.
#if I2C_SDA_PIN != 7
	#ERROR
#else
	DrvGPIO_SetIOMode(inst.data_port, DRVGPIO_IOMODE_PIN7_OPEN_DRAIN);
#endif

#if I2C_SCK_PIN != 6
	#ERROR
#else
	DrvGPIO_SetIOMode(inst.clock_port, DRVGPIO_IOMODE_PIN6_OPEN_DRAIN);
#endif

    // Make sure SDA and SCL are set high.
    i2c_clock_high(&inst);
	i2c_data_high(&inst);

    return inst;
}

// TODO (brandon) : Renable the ACK checks.
bool
i2c_send(SoftI2C * inst, uint8_t address, uint8_t * data, uint8_t count)
{
    __disable_irq();

    // Send the start condition.
    i2c_start(inst);

    // Send the address with the read/write bit reset (write).
    i2c_send_byte(inst, (address << 1) & 0xFE);

    // Was the address aknowledged?
    if (!i2c_get_ack(inst))
    {
        // No. Stop the transaction.
        /*
         * i2c_stop(inst);
         * __enable_irq(inst);
         * return false;
         */
    }

    // Send each data byte.
    while (count--)
    {
        // Send the next byte.
        i2c_send_byte(inst, *(data++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack(inst))
        {
            // Stop the transaction.
            /*
             * i2c_stop(inst);
             * __enable_irq(inst);
             * return false;
             */
        }
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}

// TODO (brandon) : Renable the ACK checks.
bool
i2c_send_packet(SoftI2C * inst, PACKETData * data)
{
    // NOTE (brandon) : Need to add 2 to data->length
    // to add the length and checksum fields.
    uint8_t count = data->length + 2;
    uint8_t *send_data = (uint8_t *) data;

    __disable_irq();

    // Send the start condition.
    i2c_start(inst);

    // Send each data byte.
    while (count--)
    {
        // Send the next byte.
        i2c_send_byte(inst, *(send_data++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack(inst))
        {
            // Stop the transaction.
            /*
             * i2c_stop(inst);
             * __enable_irq(inst);
             * return false;
             */
        }
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}

bool
i2c_recv(SoftI2C * inst, uint8_t address, uint8_t * data, uint8_t count)
{
    __disable_irq();

    // Send the start condition.
    i2c_start(inst);

    // Send the address with the read/write bit set (read).
    i2c_send_byte(inst, (address << 1) | 0x01);

    // Was the address aknowledged?
    if (!i2c_get_ack(inst))
    {
        // No. Stop the transaction.
        i2c_stop(inst);
        __enable_irq();
        return false;
    }

    // Receive each data byte.
    while (count--)
    {
        // Get the next byte.
        *(data++) = i2c_get_byte(inst);

        // Send Ack unless this is the last byte.
        if (count > 0)
            i2c_send_ack(inst);
        else
            i2c_send_nak(inst);
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}

// NOTE (brandon) : Added slave function.
bool
slave_i2c_recv(SoftI2C * inst, uint8_t * data, uint8_t count)
{
    __disable_irq();

    // Release the sda and clock lines
    i2c_data_high(inst);
    i2c_clock_high(inst);

    // TODO (brandon) : Wait for start signal.
    // Wait for both SDA and CLK to go low.
    if (wait_for_i2c_data_low(inst) == false)
        return false;
    if (wait_for_i2c_clock_low(inst) == false)
        return false;

    // increment count to get the first
    // byte which is the address.
    count++;

    // Receive each data byte.
    while (count--)
    {
        // Get the next byte.
        *(data++) = slave_i2c_get_byte(inst);

        // Send Ack unless this is the last byte.
        if (count > 0)
            slave_i2c_send_ack(inst);
        else
            slave_i2c_send_nak(inst);
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}

// NOTE (brandon) : Added slave function.
bool
slave_i2c_recv_packet(SoftI2C * inst, PACKETData * data)
{
    uint8_t count = 0;
    uint8_t *recv_data = (uint8_t *) data;
    __disable_irq();

    // Release the sda and clock lines
    i2c_data_high(inst);
    i2c_clock_high(inst);

    // TODO (brandon) : Wait for start signal.
    // Wait for both SDA and CLK to go low.
    if (wait_for_i2c_data_low(inst) == false)
        return false;
    if (wait_for_i2c_clock_low(inst) == false)
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
        *(recv_data++) = slave_i2c_get_byte(inst);

        // Send Ack unless this is the last byte.
        if (count > 0)
            slave_i2c_send_ack(inst);
        else
            slave_i2c_send_nak(inst);

        count++;
        if (count >= data->length + 2)
            break;
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}

bool
i2c_send_recv(SoftI2C * inst, uint8_t address, uint8_t * data_out,
              uint8_t count_out, uint8_t * data_in, uint8_t count_in)
{
    __disable_irq();

    // Send the start condition.
    i2c_start(inst);

    // Send the address with the read/write bit reset (write).
    i2c_send_byte(inst, (address << 1) & 0xFE);

    // Was the address aknowledged?
    if (!i2c_get_ack(inst))
    {
        i2c_stop(inst);
        __enable_irq();
        return false;
    }

    // Send each of the outgoing bytes.
    while (count_out--)
    {
        // Send the next byte.
        i2c_send_byte(inst, *(data_out++));

        // Keep going unless we receive a Nak.
        if (!i2c_get_ack(inst))
        {
            i2c_stop(inst);
            __enable_irq();
            return false;
        }
    }

    // Repeat the start.
    i2c_start(inst);

    // Send the address with the read/write bit set (read).
    i2c_send_byte(inst, (address << 1) | 0x01);

    // Was the address aknowledged?
    if (!i2c_get_ack(inst))
    {
        i2c_stop(inst);
        __enable_irq();
        return false;
    }

    // Receive each of the incoming bytes.
    while (count_in--)
    {
        // Get the next byte.
        *(data_in++) = i2c_get_byte(inst);

        // Send Ack unless this is the last byte.
        if (count_in > 0)
            i2c_send_ack(inst);
        else
            i2c_send_nak(inst);
    }

    // Send the stop condition.
    i2c_stop(inst);

    __enable_irq();

    return true;
}
