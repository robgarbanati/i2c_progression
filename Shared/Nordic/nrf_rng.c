#include "global.h"
#include "nrf_soc.h"
#include "nrf_rng.h"


// Get an 8-bit random number.
uint8_t nrf_rng_get_8(void)
{
	uint8_t retval;

	// Call the SoftDevice to get a random number.
	sd_rand_application_vector_get(&retval, sizeof(retval));

	return retval;
}


// Get a 16-bit random number.
uint16_t nrf_rng_get_16(void)
{
	uint16_t retval;

	// Call the SoftDevice to get a random number.
	sd_rand_application_vector_get((uint8_t *) &retval, sizeof(retval));

	return retval;
}


// Get a 32-bit random number.
uint32_t nrf_rng_get_32(void)
{
	uint32_t retval;

	// Call the SoftDevice to get a random number.
	sd_rand_application_vector_get((uint8_t *) &retval, sizeof(retval));

	return retval;
}

