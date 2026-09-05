#include "canHelpers.h"

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "freertos/queue.h"
#include "logger.h"

// Flag, tells if CAN controller is initialized
static bool s_canStarted = false;
// ESP CAN node
static twai_node_handle_t s_canNode = NULL;
// All frames received through controller interrupt are 
// added to this queue
static QueueHandle_t s_receiveQueue = NULL;
// Temporary receive buffer for the received payload
static uint8_t s_receiveBuffer[TWAI_FRAME_MAX_LEN];
// Temporary storage for a received frame
static twai_frame_t s_receiveFrame;

/**
 * Called upon CAN controller receive interrupt
 *
 * @param node Handle to the TWAI node (CAN controller) 
 *             that received the frame.
 * @param eventData Information about the RX event.
 *        Currently unused by this callback.
 * @param userContext User-provided context pointer passed
 *                    when registering the callback. Currently unused.
 * @return true if a higher-priority FreeRTOS task was woken 
 *         by this callback, false otherwise.
 */
static bool canHelpers_onReceive(twai_node_handle_t node,
							 const twai_rx_done_event_data_t *eventData,
							 void *userContext)
{
	(void)eventData;
	(void)userContext;

	if (twai_node_receive_from_isr(node, &s_receiveFrame) == ESP_OK) {
		canHelpers_frame_t frame = {0};
		frame.id = s_receiveFrame.header.id;
		frame.extended = s_receiveFrame.header.ide;
		frame.dataLength = (uint8_t)twaifd_dlc2len(s_receiveFrame.header.dlc);
		if (frame.dataLength <= TWAI_FRAME_MAX_LEN) {
			memcpy(frame.data, s_receiveFrame.buffer, frame.dataLength);
			BaseType_t higherPriorityTaskWoken = pdFALSE;
			xQueueSendFromISR(s_receiveQueue, &frame, &higherPriorityTaskWoken);
			return higherPriorityTaskWoken == pdTRUE;
		}
	}

	return false;
}

/**
 * Initialize the ESP32 CAN controller.
 * @param txPin the physical TX pin the transceiver TX is connected to
 * @param rxPin the physical RX pin the transceiver RX is connected t0
 * @return true if initialization is succesfull or already initialized
 *         false if something went wrong in the initialization
 */
bool canHelpers_init(gpio_num_t txPin, gpio_num_t rxPin)
{
    // If already initialized, return true and do nothing
	if (s_canStarted) {
		return true;
	}

    // Create RX queue fitting 16 CAN frames
	s_receiveQueue = xQueueCreate(16, sizeof(canHelpers_frame_t));
	if (s_receiveQueue == NULL) {
		return false;
	}

    // Configure the CAN controller to work with 250
    // kbaud CAN bus.
	twai_onchip_node_config_t nodeConfig = {0};
	nodeConfig.io_cfg.tx = txPin;
	nodeConfig.io_cfg.rx = rxPin;
	nodeConfig.io_cfg.quanta_clk_out = GPIO_NUM_NC;
	nodeConfig.io_cfg.bus_off_indicator = GPIO_NUM_NC;
	nodeConfig.bit_timing.bitrate = 250000;
	nodeConfig.tx_queue_depth = 8;

	esp_err_t result = twai_new_node_onchip(&nodeConfig, &s_canNode);
	if (result != ESP_OK) {
		printf("[CAN] Failed to create TWAI node: %s\n", esp_err_to_name(result));
		vQueueDelete(s_receiveQueue);
		s_receiveQueue = NULL;
		return false;
	}

    // Set up the temporary storages for the RX frames
	s_receiveFrame.buffer = s_receiveBuffer;
	s_receiveFrame.buffer_len = sizeof(s_receiveBuffer);
	twai_event_callbacks_t callbacks = {0};
	callbacks.on_rx_done = canHelpers_onReceive;
	result = twai_node_register_event_callbacks(s_canNode, &callbacks, NULL);
	if (result == ESP_OK) {
		result = twai_node_enable(s_canNode);
	}
	if (result != ESP_OK) {
		printf("[CAN] Failed to configure or enable TWAI node: %s\n",
		       esp_err_to_name(result));
		twai_node_delete(s_canNode);
		s_canNode = NULL;
		vQueueDelete(s_receiveQueue);
		s_receiveQueue = NULL;
		return false;
	}

    // Init success, return true
	printf("[CAN] TWAI started: TX GPIO=%d, RX GPIO=%d, bitrate=%lu\n",
	       txPin, rxPin, (unsigned long)nodeConfig.bit_timing.bitrate);
	s_canStarted = true;
	return true;
}

/**
 * Destruct everything related to CAN
*/
void canHelpers_deinit(void)
{
	if (!s_canStarted) {
		return;
	}

	twai_node_disable(s_canNode);
	twai_node_delete(s_canNode);
	s_canNode = NULL;
	vQueueDelete(s_receiveQueue);
	s_receiveQueue = NULL;
	s_canStarted = false;
}

/**
 * Send a single CAN frame to the bus
 * @param id CAN frame's id
 * @param data frame's payload, maximum of 8 bytes
 * @param dataLenght DLC of the frame, length of the payload in bytes, max 8
 * @param timeoutTicks maximum waittime to wait for the frame to be accepted
 * @return true if the frame was sent, false otherwise
 */
bool canHelpers_send(uint32_t id, const uint8_t *data, uint8_t dataLength,
					 TickType_t timeoutTicks)
{
	if (!s_canStarted || data == NULL || dataLength > 8U || id > 0x1FFFFFFFU) {
		return false;
	}

    // Construct the message
	twai_frame_t message = {0};
	message.header.id = id;
	message.header.ide = true;
	message.header.dlc = dataLength;
	message.buffer = (uint8_t *)data;
	message.buffer_len = dataLength;

    // Send the message and return the status
	esp_err_t result = twai_node_transmit(
		s_canNode, &message, (int)pdTICKS_TO_MS(timeoutTicks));
	if (result != ESP_OK) {
		LOG_ERR("canHelpers", "TWAI transmit failed for id=0x%08lx: %s",
			(unsigned long)id, esp_err_to_name(result));
	}

	return result == ESP_OK;
}

/**
 * Receive a frame from the local software queue
 * @param frame the frame to which to store the RX data
 * @param timeoutTicks maximum time to wait for a frame
 * @return true if received a frame, false otherwise.
 */
bool canHelpers_receive(canHelpers_frame_t *frame, TickType_t timeoutTicks)
{
	if (!s_canStarted || frame == NULL || s_receiveQueue == NULL) {
		return false;
	}

	return xQueueReceive(s_receiveQueue, frame, timeoutTicks) == pdTRUE;
}
// Should implement can functions to read bms data.

// BMS's do not send anything by dafault the data has to be queried.

// The bus is 250kbit bus with 29-bit ids. 

// The messages follow this kind of structure:

// id: 18900140, where:
//     - 18 is the priority, should maybe be different for different batteries to avoid collisions?
//     - 90 is id for the data asked from the bms, 90 here stands for total volts, current, and soc
//     - 01 is the id of the bms (01 by default, I have two bms, so I set the other to 02)
//     - 40 is the id of the one snding the request, should be 40
// dlc: 8 for every query
// paylaod: 0's for every query.

// So the example message sent would be
// 18900140 8 00 00 00 00 00 00 00 00
// And the response to this would be
// 18904001 8 02 15 00 00 75 30 03 D8
// In the response, the sender and target id are flipped.

// This however, should onlyu act as a CAN interface, so this file does
// not have to know about the bms too much, this only has to be able to send
// and receive CAN frames from the ESP32's CAN controller. So this also has
// to define the correct pins to connect to and probably intialize the controller
// so that it has RX and TX fifos or buffers. 