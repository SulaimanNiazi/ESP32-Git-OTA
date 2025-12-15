# ESP32 Git OTA

**ESP32 Git OTA** is an ESP-IDF–based project that demonstrates a robust Over-The-Air (OTA) firmware update mechanism using **Git tags as firmware versions**. The application connects to Wi-Fi, queries a GitHub repository for the latest tagged release, and automatically updates the device if a newer version is available.

To clearly indicate the active firmware version, each release toggles an LED on a **different GPIO pin** (e.g., GPIO 2 in one version, GPIO 14 in another).

---

## Key Features

* Wi-Fi connectivity using ESP-IDF
* OTA firmware updates driven by GitHub tags
* Version identification via GPIO-based LED blinking
* Dual OTA slots with factory fallback
* GPIO-based triggers for factory reset and test application (optional)
* ESP-IDF–compliant partition table configuration

---

## Project Architecture

* **Bootloader**: Standard ESP-IDF bootloader with OTA support
* **Partition Table**:

  * Factory application
  * OTA Slot 0
  * OTA Slot 1
* **Update Strategy**:

  * Device checks GitHub for the latest Git tag
  * Compares the running version with the remote tag
  * Downloads and installs the update if a newer version exists
  * Safely switches boot partition

---

## Hardware & Software Requirements

### Hardware

* ESP32 development board
* On-board or external LED

### Software

* ESP-IDF **v5.5.1** or newer
* Git
* GitHub repository with tagged firmware releases

---

## Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/SulaimanNiazi/ESP32-Git-OTA.git
cd ESP32-Git-OTA
```

### 2. Set Up ESP-IDF

Follow the official ESP-IDF installation guide:
[https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)

Ensure the environment is correctly exported:

```bash
idf.py --version
```

---

## Project Configuration

### 1. Open Menuconfig

```bash
idf.py menuconfig
```

### 2. Required Settings

#### Partition Table

* **Partition Table Type**:
  `Factory app, two OTA definitions`

This enables:

* Safe OTA updates
* Factory firmware fallback
* Two OTA slots for seamless upgrades

#### Wi-Fi Configuration

* Upon Restart you will be prompted to add Wifi Credentials.
* The credentials will be stored in NVS.
* Press Boot button (Pin 0) to clear NVS.

#### Optional Bootloader Features

* GPIO triggers for:

  * Factory reset
  * Test application selection

---

## Build & Flash

```bash
idf.py build
idf.py flash monitor
```

---

## Versioning & OTA Updates

This project uses **Git tags as firmware versions**.

* Each Git tag corresponds to a firmware release
* OTA logic checks the latest version available on GitHub from the [CMakeLists.txt](CMakeLists.txt) and compares it with the current version stored in its own [CMakeLists.txt](CMakeLists.txt).
* If a newer version exists, the device:

  1. Downloads the firmware from [build/Git_OTA.bin](build/Git_OTA.bin)
  2. Verifies integrity
  3. Switches to the updated OTA partition

### Visual Version Indicator

Each firmware version **switches** the blinking LED between 2 and 14, allowing easy identification of the running version without serial logs.

Example:

* Version `v1.0.0` → GPIO 2
* Version `v1.0.1` → GPIO 14
* Version `v1.1.0` → GPIO 2
* And so on...

---

## Runtime Behavior

* Device boots and connects to Wi-Fi
* Performs OTA version check
* Applies update if available
* Blinks LED continuously to indicate active firmware
* Falls back to factory app if OTA validation fails
