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
* Proper Embedded CI/CD and management of a private and public repo via GitHub Actions

---

## Project Architecture

* **Bootloader**: Standard ESP-IDF bootloader with OTA support
* **Partition Table**:

  * Factory application
  * OTA Slot 0
  * OTA Slot 1
* **Update Strategy**:

  * Device checks a manifest.json on a public repo for the latest software version for its hardware.
  * Compares the running version with the latest version
  * Downloads and installs the update if a newer version exists
  * Safely switches boot partition
* **CI/CD**:

  * New commit is pushed with a tag starting with "v".
  * the [workflow](.github/workflows/main.yml) begins and starts building the ESP-IDF firmware.
  * Extracts the hardware from [CMakeLists.txt](CMakeLists.txt) and the firmware binary and software version from the build/project_description.json file created from the build.
  * The sha256 hash is generated of the bin file and the manifest.json file of the public repo is updated with the new sha256 and version for the specific hardware.
  * the bin file is renamed according to the hardware and is placed in the firmwares/ directory of the public repo.
  * the changes are commited and in case of failure the tag and release are deleted.

---

## Hardware & Software Requirements

### Hardware

* ESP32 development board
* On-board or external LED

### Software

* ESP-IDF **v5.5.1** or newer
* Git
* A [GitHub public repository](https://github.com/SulaimanNiazi/ESP32-Git-OTA-Public) with the manifest json file and firmware binaries.
* A Github private repository with the actual project and a secret variable named "ACCESS_TOKEN" storing an access token for writing to the public repo.

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

* Each Git tag corresponds to a firmware release for a specific hardware version
* A hardware versions are supported up to 32 characters in length.
* OTA logic checks the latest version for its hardware available on GitHub public repo from the manifest json file and compares it with the current version and hardware stored in its own [CMakeLists.txt](CMakeLists.txt).
* If a newer version exists, the device:

  1. Downloads the firmware from the raw GitHub link of the public repo
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
