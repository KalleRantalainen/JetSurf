# Mobile BLE interface

The jetSurf ESP32 uses one NimBLE host for both BLE links:

- The existing `bleMaster` central connection receives throttle notifications from `jetSurfController`.
- This component advertises as `jetSurfBoard` and exposes the phone analytics service.

The mobile service UUID is `6d6f6269-6c65-2d73-7572-662d61707001`.

## Characteristics

| Characteristic | UUID suffix | Direction | Purpose |
|---|---:|---|---|
| Telemetry | `02` | ESP32 -> phone, notify | Live input-signal snapshot every 500 ms |
| Command | `03` | phone -> ESP32, write | Write `0x01` to download the current session |
| Log data | `04` | ESP32 -> phone, notify | Framed log-file transfer |

The phone should request an ATT MTU of at least 196 bytes before enabling notifications. The firmware's preferred MTU is 256. The log packet has a 13-byte header and up to 180 bytes of file data.

## Telemetry packet

All values are little-endian and the packet is packed. The packet contains:

```text
uint8   version                 // 1
uint8   bleThrottle
uint32  sequence
uint32  timestampMs
float   velocityMetSec
float   latitudeDeg
float   longitudeDeg
float   courseDeg
uint32  gpsTimestampMs
float   battery1Current
float   battery1Voltage
float   battery1Soc
float   battery1HighestTemp
uint8   battery1HighestTempSensor
float   battery1CellVoltageDiff
float   battery2Current
float   battery2Voltage
float   battery2Soc
float   battery2HighestTemp
uint8   battery2HighestTempSensor
float   battery2CellVoltageDiff
```

## Log packets

All log notifications use this packed header followed by `payloadLength` bytes:

```text
uint8   type
uint32  sequence
uint16  fileIndex
uint32  offset
uint16  payloadLength
uint8[] payload
```

Packet types are:

- `0x10`: file start; payload is a null-terminated filename such as `log1.log`.
- `0x11`: file data; payload is a chunk at `offset`.
- `0x12`: file end; `offset` is the total file size sent.
- `0x13`: all files have been sent.
- `0x7f`: transfer error.

The transfer includes every `logN.log` file in the session created by the currently running firmware session. A transfer is intentionally low priority and does not run when the phone is not subscribed to the log characteristic.
