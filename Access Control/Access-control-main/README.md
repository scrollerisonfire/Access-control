# Access Control Plamen


# ESP32 Access Control System

![ESP32](./images/esp32.png)  <!-- Optional: Add an image of your device -->

## Project Overview

This project is an **Access Control System** implemented on the **ESP32** using **C++**.  
It demonstrates a modular design for embedded devices, featuring:

- WiFi connectivity (station and access point modes)
- MQTT communication
- Time synchronization (SNTP)
- NFC and QR code readers
- SPI and UART interfaces
- Access Control management with roles and permissions
- FreeRTOS task management for concurrent operations
- Logging and event callbacks

This project is intended for **educational purposes and portfolio demonstration** and does **not contain sensitive company data**.

---

## Features

- **WiFi Management**
  - Connects to WiFi in STA mode
  - Can start an AP mode for configuration
  - Handles reconnects and timeouts
  - Supports NTP time synchronization

- **MQTT Communication**
  - Connects to MQTT broker
  - Publishes and subscribes to topics
  - Can be used for remote monitoring or integration with cloud services

- **Access Control**
  - Role-based permissions (e.g., admin, user)
  - Event-driven callbacks for access events
  - NFC and QR reader support

- **Peripheral Interfaces**
  - SPI bus communication for external modules
  - UART serial communication
  - Arduino-style API (`digitalWrite`, `digitalRead`, `millis`, `delay`)

- **Multithreading / FreeRTOS**
  - Concurrent tasks for NFC and QR readers
  - Event callbacks for asynchronous processing

---

## Technologies & Tools

- **Programming Language:** C++  
- **Microcontroller:** ESP32 DevKit / ESP32-WROOM  
- **Frameworks:** ESP-IDF / FreeRTOS  
- **Protocols:** MQTT, SNTP  
- **Interfaces:** SPI, UART, GPIO  
- **Development Environments:** PlatformIO (VSCode), ESP-IDF  
- **Version Control:** Git + GitHub  

---

