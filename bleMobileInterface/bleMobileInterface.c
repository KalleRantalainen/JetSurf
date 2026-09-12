#include "bleMobileInterface.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "logger.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdCardModule.h"
#include "inputSignals.h"

static const char *TAG = "bleMobileInterface";

static const ble_uuid128_t mobileServiceUuid = BLE_UUID128_INIT(
    0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x2d, 0x73,
    0x75, 0x72, 0x66, 0x2d, 0x61, 0x70, 0x70, 0x01);
static const ble_uuid128_t telemetryCharacteristicUuid = BLE_UUID128_INIT(
    0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x2d, 0x73,
    0x75, 0x72, 0x66, 0x2d, 0x61, 0x70, 0x70, 0x02);
static const ble_uuid128_t commandCharacteristicUuid = BLE_UUID128_INIT(
    0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x2d, 0x73,
    0x75, 0x72, 0x66, 0x2d, 0x61, 0x70, 0x70, 0x03);
static const ble_uuid128_t logCharacteristicUuid = BLE_UUID128_INIT(
    0x6d, 0x6f, 0x62, 0x69, 0x6c, 0x65, 0x2d, 0x73,
    0x75, 0x72, 0x66, 0x2d, 0x61, 0x70, 0x70, 0x04);

#define MOBILE_DEVICE_NAME "jetSurfBoard"
#define MOBILE_COMMAND_DOWNLOAD_LATEST 0x01U
#define MOBILE_PACKET_TELEMETRY 0x01U
#define MOBILE_PACKET_FILE_START 0x10U
#define MOBILE_PACKET_FILE_DATA 0x11U
#define MOBILE_PACKET_FILE_END 0x12U
#define MOBILE_PACKET_TRANSFER_END 0x13U
#define MOBILE_PACKET_ERROR 0x7fU
#define MOBILE_LOG_CHUNK_SIZE 180U
#define MOBILE_MAX_SESSION_FILES 32U
#define MOBILE_TELEMETRY_PERIOD_MS 500U

static uint16_t telemetryValueHandle;
static uint16_t commandValueHandle;
static uint16_t logValueHandle;
static uint16_t mobileConnectionHandle = BLE_HS_CONN_HANDLE_NONE;
static volatile bool telemetryNotificationsEnabled;
static volatile bool logNotificationsEnabled;
static QueueHandle_t downloadQueue;
static bleMobileInterface_scan_start_t remoteScanStart;
static uint32_t telemetrySequence;
static uint32_t logSequence;

typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t bleThrottle;
    uint32_t sequence;
    uint32_t timestampMs;
    float velocityMetSec;
    float latitudeDeg;
    float longitudeDeg;
    float courseDeg;
    uint32_t gpsTimestampMs;
    float battery1Current;
    float battery1Voltage;
    float battery1Soc;
    float battery1HighestTemp;
    uint8_t battery1HighestTempSensor;
    float battery1CellVoltageDiff;
    float battery2Current;
    float battery2Voltage;
    float battery2Soc;
    float battery2HighestTemp;
    uint8_t battery2HighestTempSensor;
    float battery2CellVoltageDiff;
} telemetryPacket_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint32_t sequence;
    uint16_t fileIndex;
    uint32_t offset;
    uint16_t payloadLength;
    uint8_t payload[MOBILE_LOG_CHUNK_SIZE];
} logPacket_t;

static void startAdvertising(void);

static bool mobileConnected(void)
{
    return mobileConnectionHandle != BLE_HS_CONN_HANDLE_NONE;
}

static void notifyCharacteristic(uint16_t valueHandle, const void *data, size_t length)
{
    if (!mobileConnected()) {
        return;
    }

    struct os_mbuf *buffer = ble_hs_mbuf_from_flat(data, length);
    if (buffer == NULL) {
        ESP_LOGW(TAG, "Could not allocate BLE notification buffer");
        return;
    }

    const int result = ble_gatts_notify_custom(mobileConnectionHandle, valueHandle, buffer);
    if (result != 0) {
        os_mbuf_free_chain(buffer);
    }
}

static telemetryPacket_t makeTelemetryPacket(void)
{
    telemetryPacket_t packet = {
        .version = 1,
        .bleThrottle = inputSignal_bleThrottle,
        .sequence = telemetrySequence++,
        .timestampMs = (uint32_t)(esp_timer_get_time() / 1000ULL),
        .velocityMetSec = (float)inputSignal_velocityMetSec,
        .latitudeDeg = (float)inputSignal_latitudeDeg,
        .longitudeDeg = (float)inputSignal_longitudeDeg,
        .courseDeg = (float)inputSignal_courseDeg,
        .gpsTimestampMs = inputSignal_gpsTimestampMs,
        .battery1Current = inputSignal_battery1_current,
        .battery1Voltage = inputSignal_battery1_voltage,
        .battery1Soc = inputSignal_battery1_soc,
        .battery1HighestTemp = inputSignal_battery1_highestTemp,
        .battery1HighestTempSensor = inputSignal_battery1_highestTempSensor,
        .battery1CellVoltageDiff = inputSignal_battery1_cellVoltageDiff,
        .battery2Current = inputSignal_battery2_current,
        .battery2Voltage = inputSignal_battery2_voltage,
        .battery2Soc = inputSignal_battery2_soc,
        .battery2HighestTemp = inputSignal_battery2_highestTemp,
        .battery2HighestTempSensor = inputSignal_battery2_highestTempSensor,
        .battery2CellVoltageDiff = inputSignal_battery2_cellVoltageDiff,
    };
    return packet;
}

static void telemetryTask(void *arg)
{
    (void)arg;
    while (true) {
        if (telemetryNotificationsEnabled) {
            const telemetryPacket_t packet = makeTelemetryPacket();
            notifyCharacteristic(telemetryValueHandle, &packet, sizeof(packet));
        }
        vTaskDelay(pdMS_TO_TICKS(MOBILE_TELEMETRY_PERIOD_MS));
    }
}

static void sendLogPacket(logPacket_t *packet)
{
    packet->sequence = logSequence++;
    notifyCharacteristic(logValueHandle, packet,
                         offsetof(logPacket_t, payload) + packet->payloadLength);
}

static void sendTransferMarker(uint8_t type)
{
    logPacket_t packet = { .type = type };
    sendLogPacket(&packet);
}

static int notifyCharacteristicAccess(uint16_t connHandle, uint16_t attrHandle,
                                      struct ble_gatt_access_ctxt *context, void *arg)
{
    (void)connHandle;
    (void)attrHandle;
    (void)context;
    (void)arg;
    return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

static void transferLatestSession(void)
{
    if (!logNotificationsEnabled || !sdCardModule_isReady()) {
        sendTransferMarker(MOBILE_PACKET_ERROR);
        return;
    }

    char filenames[MOBILE_MAX_SESSION_FILES][32];
    size_t fileCount = 0;
    if (!sdCardModule_getLatestSessionFiles(filenames, MOBILE_MAX_SESSION_FILES, &fileCount)) {
        sendTransferMarker(MOBILE_PACKET_ERROR);
        return;
    }

    uint8_t data[MOBILE_LOG_CHUNK_SIZE];
    for (size_t fileIndex = 0; fileIndex < fileCount; ++fileIndex) {
        logPacket_t packet = { .type = MOBILE_PACKET_FILE_START,
                               .fileIndex = (uint16_t)fileIndex };
        packet.payloadLength = (uint16_t)(strlen(filenames[fileIndex]) + 1U);
        memcpy(packet.payload, filenames[fileIndex], packet.payloadLength);
        sendLogPacket(&packet);
        vTaskDelay(pdMS_TO_TICKS(20));

        uint32_t offset = 0;
        while (logNotificationsEnabled) {
            size_t bytesRead = 0;
            if (!sdCardModule_readLatestSessionFile(filenames[fileIndex], offset,
                                                    data, sizeof(data), &bytesRead)) {
                sendTransferMarker(MOBILE_PACKET_ERROR);
                return;
            }
            if (bytesRead == 0) {
                break;
            }

            packet = (logPacket_t){ .type = MOBILE_PACKET_FILE_DATA,
                                    .fileIndex = (uint16_t)fileIndex,
                                    .offset = offset,
                                    .payloadLength = (uint16_t)bytesRead };
            memcpy(packet.payload, data, bytesRead);
            sendLogPacket(&packet);
            offset += (uint32_t)bytesRead;
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        packet = (logPacket_t){ .type = MOBILE_PACKET_FILE_END,
                                .fileIndex = (uint16_t)fileIndex,
                                .offset = offset };
        sendLogPacket(&packet);
    }

    sendTransferMarker(MOBILE_PACKET_TRANSFER_END);
}

static void downloadTask(void *arg)
{
    (void)arg;
    uint8_t command;
    while (true) {
        if (xQueueReceive(downloadQueue, &command, portMAX_DELAY) == pdTRUE &&
            command == MOBILE_COMMAND_DOWNLOAD_LATEST && mobileConnected()) {
            transferLatestSession();
        }
    }
}

static int commandAccess(uint16_t connHandle, uint16_t attrHandle,
                         struct ble_gatt_access_ctxt *context, void *arg)
{
    (void)attrHandle;
    (void)arg;
    if (context->op != BLE_GATT_ACCESS_OP_WRITE_CHR || context->om == NULL ||
        OS_MBUF_PKTLEN(context->om) != 1) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    uint8_t command;
    os_mbuf_copydata(context->om, 0, 1, &command);
    if (command == MOBILE_COMMAND_DOWNLOAD_LATEST) {
        mobileConnectionHandle = connHandle;
        if (xQueueSend(downloadQueue, &command, 0) != pdTRUE) {
            return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static const struct ble_gatt_svc_def mobileServices[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &mobileServiceUuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &telemetryCharacteristicUuid.u,
                .val_handle = &telemetryValueHandle,
                .access_cb = notifyCharacteristicAccess,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            {
                .uuid = &commandCharacteristicUuid.u,
                .val_handle = &commandValueHandle,
                .access_cb = commandAccess,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {
                .uuid = &logCharacteristicUuid.u,
                .val_handle = &logValueHandle,
                .access_cb = notifyCharacteristicAccess,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            { 0 },
        },
    },
    { 0 },
};

static int gapEvent(struct ble_gap_event *event, void *arg)
{
    (void)arg;
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            mobileConnectionHandle = event->connect.conn_handle;
            telemetryNotificationsEnabled = false;
            logNotificationsEnabled = false;
            ESP_LOGI(TAG, "Mobile phone connected");
        } else {
            startAdvertising();
        }
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.conn_handle == mobileConnectionHandle) {
            if (event->subscribe.attr_handle == telemetryValueHandle) {
                telemetryNotificationsEnabled = event->subscribe.cur_notify != 0;
            } else if (event->subscribe.attr_handle == logValueHandle) {
                logNotificationsEnabled = event->subscribe.cur_notify != 0;
            }
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        if (event->disconnect.conn.conn_handle == mobileConnectionHandle) {
            mobileConnectionHandle = BLE_HS_CONN_HANDLE_NONE;
            telemetryNotificationsEnabled = false;
            logNotificationsEnabled = false;
            startAdvertising();
        }
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        startAdvertising();
        break;
    default:
        break;
    }
    return 0;
}

static void startAdvertising(void)
{
    uint8_t ownAddressType;
    if (ble_hs_id_infer_auto(0, &ownAddressType) != 0) {
        ESP_LOGE(TAG, "Could not determine local BLE address type");
        return;
    }

    struct ble_hs_adv_fields fields = { 0 };
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.uuids128 = (ble_uuid128_t *)&mobileServiceUuid;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;
    if (ble_gap_adv_set_fields(&fields) != 0) {
        ESP_LOGE(TAG, "Could not set BLE advertisement fields");
        return;
    }

    struct ble_hs_adv_fields responseFields = { 0 };
    responseFields.name = (uint8_t *)MOBILE_DEVICE_NAME;
    responseFields.name_len = strlen(MOBILE_DEVICE_NAME);
    responseFields.name_is_complete = 1;
    if (ble_gap_adv_rsp_set_fields(&responseFields) != 0) {
        ESP_LOGE(TAG, "Could not set BLE scan response fields");
        return;
    }

    const struct ble_gap_adv_params parameters = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };
    const int result = ble_gap_adv_start(ownAddressType, NULL, BLE_HS_FOREVER,
                                         &parameters, gapEvent, NULL);
    if (result != 0 && result != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "Could not start BLE advertising: %d", result);
    }
}

static void onHostSync(void)
{
    startAdvertising();
    if (remoteScanStart != NULL) {
        remoteScanStart();
    }
}

static void nimbleHostTask(void *arg)
{
    (void)arg;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void bleMobileInterface_init(bleMobileInterface_scan_start_t scanStart)
{
    remoteScanStart = scanStart;
    downloadQueue = xQueueCreate(1, sizeof(uint8_t));
    if (downloadQueue == NULL) {
        ESP_LOGE(TAG, "Could not create mobile download queue");
        return;
    }

    esp_err_t nvsResult = nvs_flash_init();
    if (nvsResult == ESP_ERR_NVS_NO_FREE_PAGES || nvsResult == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvsResult = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvsResult);

    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(MOBILE_DEVICE_NAME));
    ESP_ERROR_CHECK(ble_gatts_count_cfg(mobileServices));
    ESP_ERROR_CHECK(ble_gatts_add_svcs(mobileServices));
    ble_hs_cfg.sync_cb = onHostSync;
    nimble_port_freertos_init(nimbleHostTask);

    xTaskCreate(telemetryTask, "mobileTelemetry", 3072, NULL, 4, NULL);
    xTaskCreate(downloadTask, "mobileDownload", 4096, NULL, 3, NULL);
}
