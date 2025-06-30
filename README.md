# Firmware Over-The-Air (FOTA) for Embedded Devices

This project implements a lightweight **Firmware Over-The-Air (FOTA)** update system designed for resource-constrained embedded systems. It enables devices to download and apply firmware updates remotely via serial communication interfaces.

## Overview

- Designed for microcontrollers such as **STM32F103**
- Supports firmware update via UART or custom data protocol
- Uses a **bootloader–application** split architecture
- Verifies firmware integrity using checksum
- Reduces the need for physical access to reprogram devices

## Workflow

1. Device boots into **bootloader**.
2. Bootloader checks if new firmware is available:
   - Either via flag in flash or command from host
3. If available:
   - Downloads firmware chunk-by-chunk
   - Verifies checksum
   - Writes new firmware to application area
4. Transfers control to application on success.


## Future work
- Add support for encrypted firmware
- Add rollback mechanism in case of update failure
- Support wireless protocols (BLE, LoRa, etc.)
