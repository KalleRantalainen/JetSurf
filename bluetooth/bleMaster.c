#include "bleMaster.h"

#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "logger.h"

static const char *TAG = "bleMaster";

// Create BLE service and charcteristic for the throttle value.
// Use random ids to avoid collisions with other devices.
static const ble_uuid128_t throttleServiceUuid = BLE_UUID128_INIT(
    0x2d, 0x55, 0xb1, 0x61, 0x4b, 0xf4, 0xef, 0x3f,
    0x22, 0x67, 0x6f, 0x41, 0x09, 0x57, 0x91, 0x8e);
static const ble_uuid128_t throttleCharacteristicUuid = BLE_UUID128_INIT(
    0x43, 0x55, 0xb1, 0x61, 0x4b, 0xf4, 0xef, 0x3f,
    0x22, 0x67, 0x6f, 0x41, 0x09, 0x57, 0x91, 0x87);

// Latest throttle value receicved over BLE
static volatile uint8_t latestThrottle;
// Time of the most recent valid throttle notification, in milliseconds.
static volatile uint32_t latestThrottleTimestampMs;
// Connection status of the link, true if connected
static volatile bool connected;
// BLE connection handle
static uint16_t connectionHandle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t characteristicValueHandle;
static uint16_t cccdHandle;
static uint16_t throttleServiceEndHandle;

static int gapEvent(struct ble_gap_event *event, void *arg);
static void startScan(void);

/**
 * Terminate the current connection, or restart scanning when no connection
 * is active.
 */
static void disconnectAndRetry(void)
{
    if (connectionHandle != BLE_HS_CONN_HANDLE_NONE) {
        ble_gap_terminate(connectionHandle, BLE_ERR_REM_USER_CONN_TERM);
    } else {
        startScan();
    }
}

/**
 * Handle completion of the CCCD write that enables throttle notifications.
 * Mark the link usable only after the subscription succeeds.
 */
static int subscribeComplete(uint16_t connHandle,
                             const struct ble_gatt_error *error,
                             struct ble_gatt_attr *attr,
                             void *arg)
{
    (void)connHandle;
    (void)attr;
    (void)arg;

    if (error->status != 0) {
        ESP_LOGE(TAG, "Failed to subscribe to throttle notifications: %d", error->status);
        disconnectAndRetry();
        return 0;
    }

    connected = true;
    ESP_LOGI(TAG, "Connected and subscribed to throttle notifications");
    return 0;
}

/**
 * Discover the characteristic's descriptors and enable its CCCD.
 * The callback is invoked once for each descriptor and once with a NULL
 * descriptor when discovery is complete.
 */
static int descriptorDiscoveryComplete(uint16_t connHandle,
                                       const struct ble_gatt_error *error,
                                       uint16_t chrDefHandle,
                                       const struct ble_gatt_dsc *descriptor,
                                       void *arg)
{
    (void)chrDefHandle;
    (void)arg;

    if (error->status != 0 && error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "Failed to discover throttle descriptors: %d", error->status);
        disconnectAndRetry();
        return 0;
    }

    if (descriptor != NULL) {
        if (ble_uuid_cmp(&descriptor->uuid.u, BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16)) == 0) {
            cccdHandle = descriptor->handle;
        }
        return 0;
    }

    if (cccdHandle == 0) {
        ESP_LOGE(TAG, "Throttle characteristic has no CCCD");
        disconnectAndRetry();
        return 0;
    }

    const uint8_t enableNotifications[] = {1, 0};
    const int rc = ble_gattc_write_flat(connHandle, cccdHandle,
                                        enableNotifications,
                                        sizeof(enableNotifications),
                                        subscribeComplete, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to enable throttle notifications: %d", rc);
        disconnectAndRetry();
    }
    return 0;
}

/**
 * Find the throttle characteristic in the discovered service and begin
 * discovery of its descriptors.
 */
static int characteristicDiscoveryComplete(uint16_t connHandle,
                                            const struct ble_gatt_error *error,
                                            const struct ble_gatt_chr *characteristic,
                                            void *arg)
{
    (void)arg;

    if (error->status != 0 && error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "Failed to discover throttle characteristic: %d", error->status);
        disconnectAndRetry();
        return 0;
    }

    if (characteristic != NULL) {
        characteristicValueHandle = characteristic->val_handle;
        cccdHandle = 0;
        const int rc = ble_gattc_disc_all_dscs(connHandle,
                                               characteristic->val_handle,
                                               throttleServiceEndHandle,
                                               descriptorDiscoveryComplete, NULL);
        if (rc != 0) {
            ESP_LOGE(TAG, "Failed to discover throttle descriptors: %d", rc);
            disconnectAndRetry();
        }
    }
    return 0;
}

/**
 * Find the throttle service and begin discovery of its characteristic.
 */
static int serviceDiscoveryComplete(uint16_t connHandle,
                                    const struct ble_gatt_error *error,
                                    const struct ble_gatt_svc *service,
                                    void *arg)
{
    (void)arg;

    if (error->status != 0 && error->status != BLE_HS_EDONE) {
        ESP_LOGE(TAG, "Failed to discover throttle service: %d", error->status);
        disconnectAndRetry();
        return 0;
    }

    if (service != NULL) {
        throttleServiceEndHandle = service->end_handle;
        const int rc = ble_gattc_disc_chrs_by_uuid(connHandle,
                                                   service->start_handle,
                                                   service->end_handle,
                                                   &throttleCharacteristicUuid.u,
                                                   characteristicDiscoveryComplete,
                                                   NULL);
        if (rc != 0) {
            ESP_LOGE(TAG, "Failed to discover throttle characteristic: %d", rc);
            disconnectAndRetry();
        }
    }
    return 0;
}

/**
 * Request the minimum BLE connection interval supported by this application.
 * BLE intervals are measured in 1.25 ms units, so six units equals 7.5 ms.
 */
static void requestConnectionInterval(uint16_t connHandle)
{
    const struct ble_gap_upd_params params = {
        .itvl_min = 6,
        .itvl_max = 6,
        .latency = 0,
        .supervision_timeout = 400,
        .min_ce_len = 0,
        .max_ce_len = 0,
    };
    const int rc = ble_gap_update_params(connHandle, &params);
    if (rc != 0) {
        ESP_LOGW(TAG, "7.5 ms connection interval request failed: %d", rc);
    }
}

/**
 * Return true when an advertisement belongs to the expected radio controller.
 * Matching the service UUID is preferred, while the device name is retained
 * as a useful fallback for the initial radio implementation.
 */
static bool isTargetAdvertisement(const struct ble_gap_disc_desc *advertisement)
{
    struct ble_hs_adv_fields fields;
    if (ble_hs_adv_parse_fields(&fields, advertisement->data,
                                advertisement->length_data) != 0) {
        return false;
    }

    if (fields.name != NULL && fields.name_len == strlen(BLE_MASTER_DEVICE_NAME) &&
        memcmp(fields.name, BLE_MASTER_DEVICE_NAME, fields.name_len) == 0) {
        return true;
    }

    for (int index = 0; index < fields.num_uuids128; ++index) {
        if (ble_uuid_cmp(&fields.uuids128[index].u, &throttleServiceUuid.u) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Process scan, connection, notification, and disconnection events from
 * the NimBLE host.
 */
static int gapEvent(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_DISC:
        if (isTargetAdvertisement(&event->disc)) {
            ble_gap_disc_cancel();
            uint8_t ownAddressType;
            if (ble_hs_id_infer_auto(0, &ownAddressType) == 0) {
                const int rc = ble_gap_connect(ownAddressType, &event->disc.addr,
                                               30000, NULL, gapEvent, NULL);
                if (rc != 0) {
                    ESP_LOGW(TAG, "Failed to connect to radio: %d", rc);
                    startScan();
                }
            }
        }
        break;

    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status != 0) {
            ESP_LOGW(TAG, "Radio connection failed: %d", event->connect.status);
            connectionHandle = BLE_HS_CONN_HANDLE_NONE;
            startScan();
            break;
        }
        connectionHandle = event->connect.conn_handle;
        requestConnectionInterval(connectionHandle);
        ble_gattc_disc_svc_by_uuid(connectionHandle, &throttleServiceUuid.u,
                                    serviceDiscoveryComplete, NULL);
        break;

    case BLE_GAP_EVENT_NOTIFY_RX:
        if (event->notify_rx.conn_handle == connectionHandle &&
            event->notify_rx.attr_handle == characteristicValueHandle &&
            event->notify_rx.om != NULL &&
            OS_MBUF_PKTLEN(event->notify_rx.om) == 1) {
            os_mbuf_copydata(event->notify_rx.om, 0, 1, (void *)&latestThrottle);
            latestThrottleTimestampMs = (uint32_t)(esp_timer_get_time() / 1000ULL);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        connected = false;
        connectionHandle = BLE_HS_CONN_HANDLE_NONE;
        characteristicValueHandle = 0;
        cccdHandle = 0;
        ESP_LOGW(TAG, "Radio disconnected; scanning again");
        startScan();
        break;

    default:
        break;
    }
    return 0;
}

/**
 * Scan continuously for the configured radio controller advertisement.
 */
static void startScan(void)
{
    const struct ble_gap_disc_params scanParameters = {
        .itvl = 0x0010,
        .window = 0x0010,
        .filter_duplicates = 1,
        .passive = 0,
    };
    const int rc = ble_gap_disc(BLE_OWN_ADDR_RPA_PUBLIC_DEFAULT,
                                BLE_HS_FOREVER, &scanParameters,
                                gapEvent, NULL);
    if (rc != 0 && rc != BLE_HS_EALREADY) {
        ESP_LOGE(TAG, "Failed to start radio scan: %d", rc);
    }
}

/**
 * Initialize the remote-controller client state. The shared NimBLE host is
 * initialized by bleMobileInterface so the board can be both central and
 * peripheral without starting NimBLE twice.
 */
void bleMaster_init(void)
{
    latestThrottle = 0;
    latestThrottleTimestampMs = 0;
    connected = false;
}

void bleMaster_startScan(void)
{
    startScan();
}

/**
 * Return the most recently received one-byte throttle value.
 */
uint8_t bleMaster_getThrottle(void)
{
    if (!connected) {
        return 0;
    }

    const uint32_t nowMs = (uint32_t)(esp_timer_get_time() / 1000ULL);
    const uint32_t ageMs = nowMs - latestThrottleTimestampMs;
    if (ageMs > BLE_MASTER_THROTTLE_TIMEOUT_MS) {
        LOG_WARN("BLE throttle value is %d ms old. Setting throttle to 0.\n", ageMs);
        return 0;
    }
    LOG_INFO("BLE Throttle age: %d ms\n", ageMs);
    return latestThrottle;
}

/**
 * Return whether the BLE link has completed notification subscription.
 */
bool bleMaster_isConnected(void)
{
    return connected;
}