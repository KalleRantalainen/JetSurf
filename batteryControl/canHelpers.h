#ifndef CAN_HELPERS_H_
#define CAN_HELPERS_H_

#include <stdbool.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"

// Default ESP32 DevKitC pins connected to the external CAN transceiver.
#define BATTERY_CAN_TX_GPIO GPIO_NUM_5
#define BATTERY_CAN_RX_GPIO GPIO_NUM_4

// Initialize and start the TWAI (CAN) controller at the Daly bus speed.
bool canHelpers_init(gpio_num_t txPin, gpio_num_t rxPin);

// Stop and uninstall the TWAI controller.
void canHelpers_deinit(void);

// CAN frame contains id (29-bit in this case), flag if the frame
// is extended (it is in this case), dlc byte and 8 bytes of payload.
typedef struct {
	uint32_t id;
	bool extended;
	uint8_t dataLength;
	uint8_t data[TWAI_FRAME_MAX_LEN];
} canHelpers_frame_t;

// Send one CAN frame
bool canHelpers_send(uint32_t id, const uint8_t *data, uint8_t dataLength,
					TickType_t timeoutTicks);
// Receive one CAN frame or timeout
bool canHelpers_receive(canHelpers_frame_t *frame, TickType_t timeoutTicks);

#endif // CAN_HELPERS_H_