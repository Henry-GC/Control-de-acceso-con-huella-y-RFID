# 🔐 Control de Cerradura Electrónica con ESP32, Sensor R308, RFID y Node-RED

Este proyecto de **código abierto** te permite implementar un **sistema de control de acceso inteligente** utilizando una placa **ESP32**, un lector de huellas dactilares **R308**, un lector **RFID RC522**, y una interfaz web construida con **Node-RED**. La comunicación entre el microcontrolador y la interfaz se realiza mediante el **protocolo MQTT** utilizando un broker público o local (como Mosquitto).

> ✅ Este repositorio incluye el código `.ino` del ESP32 y el archivo `.json` con el flujo completo de Node-RED.

---

## 📦 Materiales Requeridos

- ESP32 (cualquier modelo con UART y SPI)
- Sensor de huellas digitales R308
- Lector RFID RC522
- Protoboard
- Cables Dupont
- Actuador (LED, relé o pistón)
- Computadora con Node.js y Node-RED
- Broker MQTT (público o local)
- Imagen del esquema de conexión [📷 _Aquí irá tu imagen_]

---

## 🛠️ Diagrama de Conexiones

Aquí se debe adjuntar el diagrama de conexiones en formato imagen:

📌 **[Inserta aquí tu imagen de diagrama de conexiones]**

---

## 🔌 Instalación del Entorno ESP32

1. **Instala Arduino IDE**:  
   Descárgalo desde [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

2. **Agrega soporte para placas ESP32**:
   - Ve a `Archivo > Preferencias`
   - Añade la siguiente URL en "Gestor de URLs adicionales de tarjetas":  
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Luego ve a `Herramientas > Placa > Gestor de tarjetas`, busca `esp32` e instala.

3. **Instala el driver USB de la placa ESP32**:  
   - Driver CP210x: [https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers)

4. **Librerías necesarias** (desde el gestor de librerías):
   - `WiFi.h`
   - `PubSubClient`
   - `Adafruit_Fingerprint`
   - `SPI`
   - `MFRC522`

---

## 📡 Configuración del ESP32

Este sistema utiliza:

- **WiFi** para conectarse a la red local.
- **MQTT** para enviar mensajes hacia Node-RED.
- **Serial2** para la lectura del sensor R308.
- **SPI** para el lector RFID RC522.

### Configura las credenciales WiFi y MQTT en el archivo `.ino`:

```cpp
const char* ssid = "Tu_SSID";
const char* password = "Tu_PASSWORD";

const char* mqtt_server = "test.mosquitto.org"; // Puedes usar un servidor local también
