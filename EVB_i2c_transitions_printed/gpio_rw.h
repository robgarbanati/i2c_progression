/**
 *
 * @file KubitsBlock.h
 *
 * This header file defines a library
 * of functions that are used by the 
 * Kubits Blocks.
 *
 * Author: brandon@ologicinc.com
 * Create date: 3/19/2016
 */

#ifndef __GPIO_RW_H__
#define __GPIO_RW_H__

#include <stdbool.h>


#define I2C_SDA_PORT				&GPIOB 
#define I2C_SDA_PIN					14
#define I2C_SDA_MASK				1 << I2C_SDA_PIN
#define I2C_SCK_PORT				&GPIOB 
#define I2C_SCK_PIN					15
#define I2C_SCK_MASK				1 << I2C_SCK_PIN


/************************** Inline Functions **********************/

__INLINE void
pin_high(GPIO_T *port, uint32_t mask)
{
    DrvGPIO_SetOutputBit(port, mask);
}

__INLINE void
pin_low(GPIO_T *port, uint32_t mask)
{
    DrvGPIO_ClearOutputBit(port, mask);
}

__INLINE bool
pin_read(GPIO_T *port, uint32_t mask)
{
    return DrvGPIO_GetOutputPinValue(port, mask) & mask ? true : false;
}

#endif // __GPIO_RW_H__
