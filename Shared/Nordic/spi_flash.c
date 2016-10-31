#include <string.h>
#include "global.h"
#include "nrf_gpio.h"
#include "spi_flash.h"

// Open the SPI flash peripheral.
void spi_flash_open(SPIModuleNumber module, uint8_t spi_mode, uint8_t lsb_first, uint8_t spi_freq, SPIFlash *handler)
{
	// Configure the SPI device.
	spi_configure(module, spi_mode, lsb_first, spi_freq);

	// Reset the size and flags.
	handler->module = module;
	handler->flash_size = 0;
	handler->flags = 0;
}

// Close the SPI flash peripheral.
void spi_flash_close(SPIFlash *handler)
{
	// Clear the handler structure.
	memset(handler, 0, sizeof(SPIFlash));
}

// Read the SPI flash status register.
uint8_t spi_flash_read_status(SPIFlash *handler)
{
	uint8_t status;

	nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
	status = (uint8_t) spi_write_read_slave_16(handler->module, ((uint16_t) SPIFLASH_READ_STATUS << 8));
	nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

	return status;
}

// Wait until the SPI flash is not writing.
void spi_flash_wait_not_writing(SPIFlash *handler)
{
	// Wait until there is no writing activity on the SPI flash.
	while (spi_flash_read_status(handler) & (SPIFLASH_WEL | SPIFLASH_WIP)) osDelay(1);
}

// Send the write enable/disable instruction.
void spi_flash_write_enable(SPIFlash *handler, bool enable)
{
	uint8_t instruction = enable ? SPIFLASH_WRITE_ENABLE : SPIFLASH_WRITE_DISABLE;

	// Write chip enable/disable.
  nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
	spi_write_read_slave_8(handler->module, instruction);
  nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);
}

// Fill in the SPI flash chip information.
void spi_flash_get_chip_info(SPIFlash *handler)
{
	uint8_t device_id;
	uint8_t manufacture_id;
	uint8_t mem_type;
	uint32_t value;
	uint8_t flags = 0;
	bool is_w24p_flash = false;

	// Send the DEVICE_ID command.
  nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
	spi_write_read_slave_32(handler->module, ((uint32_t) SPIFLASH_DEVICE_ID << 24));
	value = (uint32_t) spi_write_read_slave_16(handler->module, 0);
  nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

	// Set the device ID and manufacture ID.
	device_id = (uint8_t) value;
	manufacture_id = (uint8_t) (value >> 8);
	
	// Get JEDEC ID command to detect W25X,W25Q MXIC and ATmel series
	// Only W25P series does not support the JEDEC ID command
  nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
	value = spi_write_read_slave_32(handler->module, ((uint32_t) SPIFLASH_JEDEC_ID << 24));
  nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

	// Check for WINBOND and MXIC SPI flash.
	if (((value >> 16) & 0xff) == manufacture_id)
	{
		device_id = ((uint8_t) value) - 1;
		mem_type  = (uint8_t) (value >> 8);

		if(((value >> 16) & 0xff) == 0xef)		
		{
			flags = SPIFLASH_FLAG_WINBOND;			
			device_id += mem_type;

			// W25P80,W25P32,W25P16
			if (mem_type == 0x20) is_w24p_flash = true;
		}
		else
		{
			flags = SPIFLASH_FLAG_MXIC;
			device_id += 0x30;
		}
	}
	// Check for ATMEL SPI flash.
	else if (((value >> 16) & 0xff) == 0x1f)
	{		
		device_id = (((value >> 8) & 0x1f) - 2);
		flags = SPIFLASH_FLAG_ATMEL;		
	}

	// Check for unsupported devices.
	if ((device_id == 0) || ((device_id & 0xf) > 7))
	{
		handler->flash_size = 0;
	}
	else
	{
		// Get the size from the device ID.
		handler->flash_size = 1024 * 1024 / 8 << (device_id & 0x0f);

		// Remove the size from the device ID.
		device_id &= 0xf0;

		// Special case the W25P series.
		if (is_w24p_flash)
		{
			flags |= SPIFLASH_FLAG_ERASE_64K;	
		}
		else
		{
			// Set the flags to indicate what types of erases are supported.
			if (device_id == SPIFLASH_W25X)
				flags |= SPIFLASH_FLAG_ERASE_64K | SPIFLASH_FLAG_ERASE_4K;
			else if (device_id == SPIFLASH_W25Q)
				flags |= SPIFLASH_FLAG_ERASE_64K | SPIFLASH_FLAG_ERASE_4K | SPIFLASH_FLAG_ERASE_32K;
			else if (device_id == SPIFLASH_W25P)
				flags |= SPIFLASH_FLAG_ERASE_64K;
			else if (((value >> 16) & 0xff) == 0x1f) // Atmel SPIFlash
				flags |= SPIFLASH_FLAG_ERASE_64K | SPIFLASH_FLAG_ERASE_4K | SPIFLASH_FLAG_ERASE_32K;
			else
			{
				handler->flash_size = 0;
			}
		}
	}

	handler->flags = flags;
}

// Erase the entire SPI flash peripheral.
void spi_flash_erase_chip(SPIFlash *handler)
{
	// Write chip enable.
	spi_flash_write_enable(handler, true);

	// Erase the chip.
  nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
	spi_write_read_slave_8(handler->module, SPIFLASH_CHIP_ERASE);
  nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

	// Wait until the chip is erased.
	spi_flash_wait_not_writing(handler);
}

// Erase the indicated SPI flash pages.
void spi_flash_erase(SPIFlash *handler, uint32_t cmd_addr, uint32_t add_incr, uint16_t erase_count)
{
	// Loop until the count of pages are erased.
	while (erase_count--)
	{
		// Write chip enable.
		spi_flash_write_enable(handler, true);

		// Write the combined command and address.
		nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);
		spi_write_read_slave_32(handler->module, cmd_addr);
		nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

		// Add the increment to the address.
		cmd_addr += add_incr;

		// Wait for erase write to complete.
		spi_flash_wait_not_writing(handler);
	}
}

// Read from the SPI flash peripheral at the indicated address.
void spi_flash_read(SPIFlash *handler, uint32_t byte_addr, uint8_t *data, uint32_t length)
{
	uint32_t i;

	// Sanity check the length.
	if (!length) return;

	// Select the flash device.
  nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);

	// Send the READ command with the address.
	spi_write_read_slave_32(handler->module, ((uint32_t) SPIFLASH_READ_DATA << 24) | (byte_addr & 0x00ffffff));

	// Prepare for the read.
	NRF_SPI0->EVENTS_READY = 0U;

	// Write dummy data to initate the reads.
  NRF_SPI0->TXD = (uint32_t) 0;

	// Read the data bytes.
	for (i = 1; i < length; ++i)
	{
		// Write dummy data to initate the next read.  We take 
		// advantage of double buffering to keep data flowing.
		NRF_SPI0->TXD = (uint32_t) 0;

		// Wait for the transaction to complete.
		while (NRF_SPI0->EVENTS_READY == 0U);

		// Get ready for the next event.
		NRF_SPI0->EVENTS_READY = 0U;

		// Read the next data byte.
		*(data++) = (uint8_t) NRF_SPI0->RXD;
	}

	// Wait for the final transaction to complete.
	while (NRF_SPI0->EVENTS_READY == 0U);

	// Read the final data byte.
	*(data++) = (uint8_t) NRF_SPI0->RXD;
	
	// Release the flash device.
  nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);
}

// Write to the SPI flash peripheral at the indicated address.
void spi_flash_write(SPIFlash *handler, uint32_t byte_addr, const uint8_t *data, uint32_t length)
{
	uint8_t d;
	uint32_t i;
	uint32_t count;
	uint32_t left_on_page;
	uint32_t left_to_write;

	// Silence unused warning.
	UNUSED(d);
	
	// Sanity check the length.
	if (!length) return;

	// Set the number of bytes left to write.
	left_to_write = length;

	// Keep going while there are bytes left to write.
	while (left_to_write)
	{
		// Determine how much left on this page.
		left_on_page = 256 - (byte_addr % 256);

		// Determine how much to write on this page.  We can never write
		// more than what is left on this page.
		count = left_to_write > left_on_page ? left_on_page : left_to_write;
		
		// Write chip enable.
		spi_flash_write_enable(handler, true);

		// Select the flash device.
		nrf_gpio_pin_clear(SPI0_FLASH_SS_PIN);

		// Send the PAGE PROGRAM command with the address.
		spi_write_read_slave_32(handler->module, ((uint32_t) SPIFLASH_PAGE_PROGRAM << 24) | (byte_addr & 0x00ffffff));

		// Get ready for the next event.
		NRF_SPI0->EVENTS_READY = 0U;

		// Write the first byte.
		NRF_SPI0->TXD = (uint32_t) data[0];

		// Read the data bytes.
		for (i = 1; i < count; ++i)
		{
			// Take advantage of double buffer to keep data flowing.
			NRF_SPI0->TXD = (uint32_t) data[i];

			// Wait for the write to complete.
			while (NRF_SPI0->EVENTS_READY == 0U);

			// Get ready for the next event.
			NRF_SPI0->EVENTS_READY = 0U;

			// Read the next data byte.
			d = (uint8_t) NRF_SPI0->RXD;
		}

		// Wait for the final write to complete.
		while (NRF_SPI0->EVENTS_READY == 0U);

		// Read the final data byte.
		d = (uint8_t) NRF_SPI0->RXD;

		// Release the flash device.
		nrf_gpio_pin_set(SPI0_FLASH_SS_PIN);

		// Wait until the page write to complete.
		spi_flash_wait_not_writing(handler);
		
		// Increment the pointers, addresses and counts.
		data += count;
		byte_addr += count;
		left_to_write -= count;
	}
}

