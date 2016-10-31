#ifndef BLE_SPI_H__
#define BLE_SPI_H__

#include <stdint.h>
#include <stdbool.h>
#include "global.h"
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_UUID_SPI_SERVICE 0x0001
#define BLE_UUID_SPI_TX_CHARACTERISTIC 0x0002
#define BLE_UUID_SPI_RX_CHARACTERISTIC 0x0003

#define SPI_MAX_DATA_LENGTH 20

// BLE thread ID.
extern osThreadId bleThreadId;

uint32_t ble_spi_init(void);
void ble_spi_on_ble_evt(ble_evt_t *ble_event);
void ble_spi_get_adv_uuid(ble_uuid_t *p_uuid_type);

void ble_thread(void const *argument);

#endif // BLE_SPI_H__

/** @} */
