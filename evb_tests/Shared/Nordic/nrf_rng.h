#ifndef __NRF_RNG_H
#define __NRF_RNG_H

#include "global.h"
#include "nrf.h"

void nrf_rng_init(void);
uint8_t nrf_rng_get_8(void);
uint16_t nrf_rng_get_16(void);
uint32_t nrf_rng_get_32(void);

#endif // __NRF_RNG_H
