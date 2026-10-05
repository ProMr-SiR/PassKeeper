# PassKeeper ESP 🔐

![PassKeeper ESP Banner](https://img.shields.io/badge/Security-AirGapped-success?style=for-the-badge&logo=security)
![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue?style=for-the-badge&logo=espressif)
![C++](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B)
![PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-orange?style=for-the-badge&logo=platformio)

**PassKeeper ESP** (formerly Mr.PassKeeper) is an advanced, isolated hardware password manager powered by the **ESP32** microcontroller. It combines an OLED display, BLE (Bluetooth Low Energy) keyboard emulation, and an isolated Wi-Fi web server to provide a secure, physical vault for password management. 

By isolating passwords from potentially compromised operating systems and eliminating cloud dependency, the system ensures maximum security, storing all data encrypted in the ESP32's NVS memory. With its plug-and-play capability and dynamic web dashboard, users can easily generate, store, and seamlessly type complex passwords into any BLE-supported device.

---

## 🎓 Academic Project Details

This project was developed as a university project at **Alrazi University** (Summer 2026).

**Team Members:**
- Mohammed Fouad Mohammed Al-Yousefi (138)
- Ezzedin Mueen Mohammed Aqeel (103)
- Muneer Abdullah Hamoud Ahmed Al-Najm (140)
- Mohammed Abdo Mohammed Haj Muhaim (132)
- Hadi Abdullah Nasser Saleh Futaih (149)

**Instructors:**
- Dr. Mahmoud E. Hodeish
- Eng. Majd Alsurihi

---

## 🎯 Project Objectives

1. **Physical Security (Air-Gapped):** Develop an ESP32-based hardware password manager isolated from OS vulnerabilities (like malware and keyloggers).
2. **Cloud Independence:** Zero reliance on cloud storage. All credentials are saved securely and locally in the NVS flash memory.
3. **Cross-Platform Plug-and-Play:** Emulate a Bluetooth Keyboard for seamless cross-platform typing without requiring external software.
4. **Intuitive Local Web Dashboard:** Implement an easy-to-use local Web Dashboard (Captive Portal) for adding and managing credentials.
5. **Hardware RNG:** Provide a hardware-based Random Password Generator.
6. **Physical UI Navigation:** Display a UI on a 1.3" OLED screen using physical push-button navigation.

---

## 🏗️ System Overview & Architecture

PassKeeper ESP operates as a self-contained vault managed by the ESP32 CPU. 

### High-Level Description
- **Configuration Mode:** When interacting with the web server via the ESP32's Wi-Fi Access Point, users can add, edit, or generate accounts.
- **Injection Mode:** For usage, the user navigates the OLED UI using physical buttons to select an account, which triggers the BLE module to send the saved credentials as emulated keystrokes to the connected host device.

### Functional Flow
- **Sensing / Input:** 3 Hardware push buttons trigger UI navigation and injection events.
- **Processing:** C++ OOP logic manages data, interfaces with the OLED, and controls Wi-Fi/BLE stacks. Data is saved in NVS memory.
- **Visualization:** An I2C-enabled SH1106 1.3" OLED provides a user-friendly interface.
- **Alerting / Output:** BLE Keyboard emulator sends automated keystrokes.

```mermaid
graph TD
    subgraph Hardware Layer
        BTN[Push Buttons]
        OLED[1.3" OLED SH1106]
        BLE[Bluetooth - BLE Keyboard]
        FLASH[(Internal NVS Flash)]
    end

    subgraph Core Classes
        PM[clsPasswordManager]
        RNG[clsRandomPasswordGenerator]
        MS[clsMainScreen]
        SC[clsScreen - Base]
    end

    subgraph Feature Classes
        ACC[clsAccountsScreen]
        SRV[clsServiceAccountScreen]
        SND[clsSendAs]
        SET[clsSettingScreen]
        WEB[clsWebDashboard]
    end

    %% Connections
    BTN --> MS
    MS --> ACC
    MS --> SET
    
    ACC --> SRV
    SRV --> SND
    SND --> BLE
    
    SET --> WEB
    WEB --> PM
    SRV --> PM
    ACC --> PM
    
    PM --> FLASH
    WEB --> RNG
    SET --> RNG
```

---

## 🚀 Getting Started (Walkthrough)

### 1️⃣ Boot and OLED Interface
- Power the ESP32 via USB (5V DC) or a battery module. Total consumption is ~150-250mA (Wi-Fi) and much lower in BLE mode.
- The welcome screen appears, followed by the main menu:
  1. `Accounts` (View saved accounts).
  2. `Settings` (Launch the web dashboard).

### 2️⃣ Adding Accounts (Web Dashboard)
1. Navigate to **Settings** on the OLED by long-pressing the green button (Confirm/Select).
2. The device will broadcast a Wi-Fi Access Point and display a **Random Password** on the screen.
3. Connect to this Wi-Fi network using your phone or PC.
4. Once connected, a Captive Portal will automatically open the **PassKeeper ESP Dashboard**.
5. Create a new service (e.g., GitHub), add your email/username and password. Use the `Regenerate` button for a secure random password.
6. Click **Save** to store credentials instantly in the ESP32 flash memory.
7. Long-press the green button on the ESP32 to stop the Wi-Fi AP and return to the main menu.

### 3️⃣ Using Your Passwords (BLE Injection)
1. Go to **Accounts** on the OLED menu.
2. Select the service you added (e.g., GitHub).
3. Select your account and choose the injection method:
   - `Username`
   - `Password`
   - `User&Pass` (Types Username -> presses 'Tab' -> types Password -> presses 'Enter').
4. Ensure your host device (PC/Phone) keyboard language is set to **English**.
5. Long-press the injection button. The ESP32 will automatically type the data flawlessly!

---

## ⚙️ Hardware Components

| Component Name | Specification / Model | Quantity |
| --- | --- | --- |
| Microcontroller | ESP32 (DOIT DEVKIT V1) | 1 |
| Display | 1.3" OLED SH1106 (I2C) | 1 |
| Input | Push Buttons | 3 |

*The ESP32 was selected for its integrated Wi-Fi and BLE capabilities, plus adequate flash memory for the Web Server. The SH1106 OLED offers a crisp interface over just two I2C pins, saving GPIO.*

---

## 💻 Development Prerequisites

To compile and modify this project, set up **PlatformIO** with the following requirements:

1. **Board:** ESP32 (DOIT DEVKIT V1 or equivalent)
2. **Partition Scheme:** Due to the large size of Wi-Fi and BLE stacks, you must modify `platformio.ini`:
   ```ini
   board_build.partitions = huge_app.csv
   ```
3. **Required Libraries:**
   - `Adafruit SH110X` (Display)
   - `Adafruit GFX Library` (Graphics)
   - `ESP32 BLE Keyboard` (Keystroke Injection)
   - `Preferences` (Built-in for NVS)
   - `WiFi`, `WebServer`, `DNSServer` (Built-in)

---

## 🛠️ Testing & Challenges

- **High-Speed Typing Drops:** Fast BLE injection caused dropped characters. *Solution:* Implemented `bleKeyboard.setDelay(30)` to introduce a micro-pause between keystrokes.
- **Code Size Limitations:** Default partitions were too small. *Solution:* Switched to `huge_app.csv`.
- **Keyboard Layout:** Wrong characters are typed if the host PC keyboard is Arabic. *Solution:* Known limitation; host keyboard must be set to English.

## 🚀 Future Work
- **Security:** Biometric fingerprint sensor integration for unlocking.
- **Data Backup:** SD Card module for encrypted backups and restoration.
- **Localization:** Algorithms to support sending ALT-codes to bypass keyboard language layout limitations.

---
*Developed as an isolated, OOP-structured, and highly efficient password management prototype.*
