#include <string.h>
#include "cmsis_os.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSPI.h"
#include "Driver/DrvSYS.h"
#include "Spi.h"
#include "Led.h"
#include "Command.h"
#include "Debug.h"

//
// Global Variables
//

// Control thread.
osThreadId spiThreadID = NULL;

//
// Local Defines
//

// SPI master mode for communication with IO Expander.
#define SPI_IOEXPANDER_HANDLER		DRVSPI_SPI0_HANDLER
#define SPI_IOEXPANDER_DEVICE  		eDRVSPI_SLAVE_1
#define SPI_IOEXPANDER_DIVIDER    (_DRVSPI_DIVIDER(DrvCLK_GetHclk(), 10000000))
#define SPI_IOEXPANDER_OPEN_FLAGS (DRVSPI_ENDIAN_BIG | \
																	 DRVSPI_IDEL_CLK_LOW | \
																	 DRVSPI_MSB_FIRST | \
																	 DRVSPI_TX_1DATA | \
																	 DRVSPI_TX_NEGEDGE | \
																	 DRVSPI_RX_POSEDGE | \
																	 _DRVSPI_SLEEP(2) | \
																	 _DRVSPI_DATA_BITS(8))

// SPI slave mode for communication with robot body.
#define SPI_BODY_HANDLER					DRVSPI_SPI1_HANDLER
#define SPI_BODY_DIVIDER     			(_DRVSPI_DIVIDER(DrvCLK_GetHclk(), 8000000))
#define SPI_BODY_OPEN_FLAGS 			(DRVSPI_ENDIAN_BIG | \
																	 DRVSPI_MSB_FIRST | \
																	 DRVSPI_TX_1DATA | \
																	 DRVSPI_TX_NEGEDGE | \
																	 DRVSPI_RX_POSEDGE | \
																	 _DRVSPI_DATA_BITS(8))

//
// Mutex Declarations
//

// SPI port 0 mutex.
osMutexId spiPort0Mutex;
osMutexDef(spiPort0Mutex);

// SPI send queue mutex.
osMutexId spiSendQueueMutex;
osMutexDef(spiSendQueueMutex);

//
// Send/Receive Queues
//

// Must be a power of two.
#define SPI_SEND_QUEUE_SIZE 8
#define SPI_RECV_QUEUE_SIZE 8
#define SPI_SEND_QUEUE_MASK (SPI_SEND_QUEUE_SIZE - 1)
#define SPI_RECV_QUEUE_MASK (SPI_RECV_QUEUE_SIZE - 1)

// Send packet queue state variables.
static UINT16 spiSendQueueTail = 0;
static UINT16 spiSendQueueHead = 0;
static spiHeadData spiSendQueue[SPI_SEND_QUEUE_SIZE];

// Receive packet queue state variables.
static UINT16 spiRecvQueueTail = 0;
static UINT16 spiRecvQueueHead = 0;
static spiHeadData spiRecvQueue[SPI_RECV_QUEUE_SIZE];

int button1, button2, button3, button4;

//
// Local Functions
//

// Handles the sending/recieving of the SPI packets at interrupt time.
// This interrupt handler will normally be on the order of 10 to 100
// microseconds depending on the packet length.
void spiHeadHandler(void)
{
	UINT8 index = 0;
	UINT8 *sendData;
	UINT8 *recvData;

	// Increment the receive queue head.
	spiRecvQueueHead = (spiRecvQueueHead + 1) & SPI_RECV_QUEUE_MASK;

	// Prevent collision of the receive queue head by discarding old packets.
	if (spiRecvQueueHead == spiRecvQueueTail) spiRecvQueueTail = (spiRecvQueueTail + 1) & SPI_RECV_QUEUE_MASK;

	// Increment the send queue tail.
	spiSendQueueTail = (spiSendQueueTail + 1) & SPI_SEND_QUEUE_MASK;

	// Point to the send/recv data buffers.
	sendData = spiSendQueue[spiSendQueueTail].buffer;
	recvData = spiRecvQueue[spiRecvQueueHead].buffer;

	// Assume we receive a zero length packet.
	recvData[0] = 0;

	// Send/receive bytes until the SPI CS pin is raised.
	while (!DrvGPIO_GetInputPinValue(&SPI_HEAD_GPIO, SPI_HEAD_CS_PIN))
	{
		// Wait while the SPI port is busy and the SPI CS line is low.
		while (DrvSPI_GetBusy(SPI_BODY_HANDLER) && !DrvGPIO_GetInputPinValue(&SPI_HEAD_GPIO, SPI_HEAD_CS_PIN));

		// Process the next byte if the SPI CS line is still low.
		if (!DrvGPIO_GetInputPinValue(&SPI_HEAD_GPIO, SPI_HEAD_CS_PIN))
		{
			// Read the value shifted in.
			recvData[index] = (UINT8) DrvSPI_SingleReadData0(SPI_BODY_HANDLER);
		
			// Set the data to shift out.
			DrvSPI_SingleWriteData0(SPI_BODY_HANDLER, (UINT32) (button1 | button2 | button3 | button4) >> 12); //(UINT32) recvData[index]);//sendData[index]);

			// Initiate the next SPI transaction.
			DrvSPI_SetGo(SPI_BODY_HANDLER);

			// Increment the index, but prevent overflow.
			if (index < SPI_HEAD_BUF_LENGTH) ++index;
		}
	}

	// Zero the length of the send packet to indicate it has been sent.
	sendData[0] = 0;

	// Initialize the first zero status byte to shift out on the next packet.
	DrvSPI_SingleWriteData0(SPI_BODY_HANDLER, (UINT32) (button1 | button2 | button3 | button4) >> 12);

	// Initiate the next SPI transaction.
	DrvSPI_SetGo(SPI_BODY_HANDLER);

}



//
// Global Functions
//
void robSpiInit(void)
{
	// Open the SPI driver.
	DrvSPI_Open(SPI_BODY_HANDLER, SPI_BODY_OPEN_FLAGS, SPI_BODY_DIVIDER);

	// Configure for slave mode.
	DrvSPI_SPI1_SetSlaveMode(TRUE);

	// Level trigger for slave mode.
	DrvSPI_SPI1_LevelTriggerInSlave(TRUE);

	// Set the zero status byte to shift out.
	DrvSPI_SingleWriteData0(SPI_BODY_HANDLER, (UINT32) 0x00);

	// Initiate the SPI transaction.
	DrvSPI_SetGo(SPI_BODY_HANDLER);

	// Enable interupt on SPI CS falling.
	DrvGPIO_SetFallingInt(&SPI_HEAD_GPIO, SPI_HEAD_CS_PIN, TRUE);
}

