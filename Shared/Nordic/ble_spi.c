#include <string.h>
#include "global.h"
#include "packet.h"
#include "ble_spi.h"
#include "app_util.h"
#include "btle_queue.h"
#include "spi_master.h"
#include "vuart.h"

//
// Global Variables
//

// BLE service thread ID.
osThreadId bleThreadId;

//
// Local Variables
//

// Service UUID.
static uint8_t uuid_type;										

// Count of available transmission buffers.
static uint8_t available_buffer_count;

// Handle of SPI Service (as provided by the BLE stack).
static uint16_t service_handle;

// Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).
static uint16_t conn_handle;

// Handles related to the TX and RX characteristics.
static ble_gatts_char_handles_t tx_handles;
static ble_gatts_char_handles_t rx_handles;

// Notification enabled flag.
static bool is_notification_enabled;

// GUUID of the Nordic UART.  XXX This needs to be changed.
const static ble_uuid128_t spi_base_uuid = {0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5, 0x00, 0x00, 0x40, 0x6E};

//
// Local Functions
//

// Connect event handler.
static void ble_on_connect(ble_evt_t *p_ble_evt)
{
	// Save the connection handle.
	conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

	// XXX Reset the available buffer count.
}


// Disconnect event handler.
static void ble_on_disconnect(ble_evt_t *p_ble_evt)
{
	UNUSED_PARAMETER(p_ble_evt);

	// Discard the connection handle.
	conn_handle = BLE_CONN_HANDLE_INVALID;
}


// Write event handler.
static void ble_on_write(ble_evt_t *p_ble_evt)
{
	ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

	// Is this a service notification?
	if ((p_evt_write->handle == rx_handles.cccd_handle) && (p_evt_write->len == 2))
	{
		// Set whether notification is being enabled or disabled.
		is_notification_enabled = ble_srv_is_notification_enabled(p_evt_write->data) ? true : false;
	}
	else if (p_evt_write->handle == tx_handles.value_handle)
	{
		// This is data intended for this service.
		PACKETData packet_data;

		// Packets must be no less than 1 byte and no greater than 20 bytes.
		if ((p_evt_write->len >= 1) || (p_evt_write->len <= 20))
		{
			// Initialize a new packet with the data we just received.
			packet_init(&packet_data, (uint8_t) p_evt_write->len, p_evt_write->data);

			// Determine where to route this packet to.
			if (packet_get_dest(&packet_data) < PACKET_LOC_MCU0)
			{
				// Ignore BTLE packets from BTLE to prevent endless loops.
			}
			else if (packet_get_dest(&packet_data) > PACKET_LOC_MCU0)
			{
				// Direct this packet to the SPI send queue.
				// XXX We will drop the packet if the send queue is filled.
				spi_put_send_queue(&packet_data);
			}
			else
			{
				// Direct this packet to the local receive queue.
				// XXX We will drop the packet if the receive queue is filled.
				vuart_put_recv_queue(&packet_data);
			}
		}
	}
	else
	{
		// Do Nothing. This event is not relevant to this service.
	}
}

// Transfer complete event handler.
static void ble_on_tx_complete(ble_evt_t *p_ble_evt)
{
	// Add to the buffer count the number of complete transmissions.
	available_buffer_count += p_ble_evt->evt.common_evt.params.tx_complete.count;

	// Signal the ble thread that more transmissions can take place.
	osSignalSet(bleThreadId, 0x01);
}

// Add RX characteristic.
static uint32_t rx_char_add(void)
{
	ble_uuid_t ble_uuid;
	ble_gatts_attr_t attr_char_value;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_attr_md_t cccd_md;
	ble_gatts_char_md_t char_md;

	// Fill in the RX uuid.
	ble_uuid.type = uuid_type;
	ble_uuid.uuid = BLE_UUID_SPI_RX_CHARACTERISTIC;

	// Add Battery Level characteristic
	memset(&cccd_md, 0, sizeof(cccd_md));
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
	cccd_md.vloc = BLE_GATTS_VLOC_STACK;

	// GATT characteristic metadata.
	memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.notify = 1;
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = &cccd_md;
	char_md.p_sccd_md = NULL;

	// Attribute metadata.
	memset(&attr_md, 0, sizeof(attr_md));
	attr_md.vloc = BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 1;
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

	// GATT attribute.
	memset(&attr_char_value, 0, sizeof(attr_char_value));
	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = sizeof(uint8_t);
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = 20;

	// Add a characteristic declaration, a characteristic value declaration and optional 
	// characteristic descriptor declarations to the local server ATT table.
	return sd_ble_gatts_characteristic_add(service_handle, &char_md, &attr_char_value, &rx_handles);
}

// Add TX characteristic.
static uint32_t tx_char_add(void)
{
	ble_uuid_t ble_uuid;
	ble_gatts_attr_t attr_char_value;
	ble_gatts_attr_md_t attr_md;
	ble_gatts_char_md_t char_md;

	// Fill in the RX uuid.
	ble_uuid.type = uuid_type;
	ble_uuid.uuid = BLE_UUID_SPI_TX_CHARACTERISTIC;

	// GATT characteristic metadata.
	memset(&char_md, 0, sizeof(char_md));
	char_md.char_props.write = 1;
	char_md.p_char_user_desc = NULL;
	char_md.p_char_pf = NULL;
	char_md.p_user_desc_md = NULL;
	char_md.p_cccd_md = NULL;
	char_md.p_sccd_md = NULL;

	// Attribute metadata.
	memset(&attr_md, 0, sizeof(attr_md));
	attr_md.vloc = BLE_GATTS_VLOC_STACK;
	attr_md.rd_auth = 0;
	attr_md.wr_auth = 0;
	attr_md.vlen = 1;
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

	// GATT attribute.
	memset(&attr_char_value, 0, sizeof(attr_char_value));
	attr_char_value.p_uuid = &ble_uuid;
	attr_char_value.p_attr_md = &attr_md;
	attr_char_value.init_len = 1;
	attr_char_value.init_offs = 0;
	attr_char_value.max_len = SPI_MAX_DATA_LENGTH;

	// Add a characteristic declaration, a characteristic value declaration and optional 
	// characteristic descriptor declarations to the local server ATT table.
	return sd_ble_gatts_characteristic_add(service_handle, &char_md, &attr_char_value, &tx_handles);
}

// Return the count of transmission buffers available for application data.
static uint32_t ble_spi_buffer_count(void)
{
	// Always return one less than the actual count.  There is some 
	// strangeness in the S110 stack that requires this.
	return available_buffer_count > 0 ? available_buffer_count - 1 : 0;
}

uint32_t ble_spi_init(void)
{
	uint32_t err_code;
	ble_uuid_t ble_uuid;

	// Initialize service structure
	conn_handle = BLE_CONN_HANDLE_INVALID;
	is_notification_enabled = false;

	// Get the buffer count.
	err_code = sd_ble_tx_buffer_count_get(&available_buffer_count);
	if (err_code != NRF_SUCCESS) return err_code;

	// Add custom base UUID.
	err_code = sd_ble_uuid_vs_add(&spi_base_uuid, &uuid_type);
	if (err_code != NRF_SUCCESS) return err_code;

	// Service UUID.
	ble_uuid.type = uuid_type;
	ble_uuid.uuid = BLE_UUID_SPI_SERVICE;

	// Add a service declaration to the local server ATT table.
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &service_handle);
	if (err_code != NRF_SUCCESS) return err_code;

	// Add the receive characteristic.
	err_code = rx_char_add();
	if (err_code != NRF_SUCCESS) return err_code;

	// Add the transmit characteristic.
	err_code = tx_char_add();
	if (err_code != NRF_SUCCESS) return err_code;

	return NRF_SUCCESS;
}

// Handle the different BLE event.
void ble_spi_on_ble_evt(ble_evt_t *ble_event)
{
	switch (ble_event->header.evt_id)
	{
		case BLE_GAP_EVT_CONNECTED:
			ble_on_connect(ble_event);
			break;

		case BLE_GAP_EVT_DISCONNECTED:
			ble_on_disconnect(ble_event);
			break;

		case BLE_GATTS_EVT_WRITE:
			ble_on_write(ble_event);
			break;

		case BLE_EVT_TX_COMPLETE:
			ble_on_tx_complete(ble_event);
			break;

		default:
			break;
	}
}

void ble_spi_get_adv_uuid(ble_uuid_t *p_adv_uuid)
{
	// Fill in the service uuid type.
	p_adv_uuid->uuid = BLE_UUID_SPI_SERVICE;
	p_adv_uuid->type = uuid_type;
}

// Send packet data as a notification.
static uint32_t ble_spi_send_packet(PACKETData *packet)
{
	uint16_t length;
	uint32_t err_code;
	ble_gatts_hvx_params_t hvx_params;

	// Fill in the packet length.
	length = (uint16_t) packet->length;
	
	// Sanity check arguments.
	if (!available_buffer_count) return NRF_ERROR_INVALID_STATE;
	if (!is_notification_enabled) return NRF_ERROR_INVALID_STATE;
	if (length < 1) return NRF_ERROR_INVALID_PARAM;
	if (length > SPI_MAX_DATA_LENGTH) return NRF_ERROR_INVALID_PARAM;

	// Fill the the HVx parameters.
	memset(&hvx_params, 0, sizeof(hvx_params));
	hvx_params.handle = rx_handles.value_handle;
	hvx_params.p_data = packet->buffer;
	hvx_params.p_len = &length;
	hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

	// Send the notification.
	err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);

	// Subtract from the transmision buffers available.
	if (err_code == NRF_SUCCESS) --available_buffer_count;

	return err_code;
}

// BLE processing thread.
void ble_thread(void const *argument)
{
	bool something_to_do = true;
	
	// We loop forever waiting for signals that indicate packets
	// are ready to be sent or that there is space on the BTLE
	// stack buffer.  Notice that the something_to_do flag is
	// manipulated in such a way that we always check twice 
	// to send data before sleeping on a signal.
	for (;;)
	{
		PACKETData send_data;
		
		// If no space on the BTLE stack means there is definately nothing to do.
		if (ble_spi_buffer_count() == 0) something_to_do = false;

		// Sleep if nothing to do.
		while (!something_to_do)
		{
			// Wait for a signal indicating BTLE activity.
			osEvent evt = osSignalWait(0, osWaitForever);

			// Make sure a signal event was returned.
			if (evt.status != osEventSignal) continue;

			// Make sure there is space on the BTLE stack buffer.
			if (ble_spi_buffer_count() == 0) continue;

			// Looks like we have something to do.
			something_to_do = true;
		}

		// Assume we have nothing to send.
		something_to_do = false;
		
		// Note that we give packets from other MCUs that are on the BTLE 
		// send queue a higher priority than the local serial stream queue.
	
		// First check the btle send queue for send data.
		if (!something_to_do) something_to_do = btle_get_send_queue(&send_data);

		// Then check the vuart serial stream send queue for send data.
		if (!something_to_do) something_to_do = vuart_get_send_queue(&send_data);

		// Prepare to send the packet.
		if (something_to_do)
		{
			// Packets with an invalid destination are silently discarded.
			if (packet_get_dest(&send_data) < PACKET_LOC_MCU0)
			{
				// Send the data to the attached client.
				uint32_t err_code = ble_spi_send_packet(&send_data);

				// Check for errors.
				if ((err_code != NRF_ERROR_INVALID_STATE) && (err_code != NRF_SUCCESS))
				{
					// Call the application error handler.
					app_error_handler(err_code, __LINE__, (uint8_t*) __FILE__);
				}
			}
		}
	}
}
