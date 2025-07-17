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
- Imagen del esquema de conexión [📷 *Aquí irá tu imagen*]

---

## 🛠️ Diagrama de Conexiones

Aquí se debe adjuntar el diagrama de conexiones en formato imagen:

📌 **[Inserta aquí tu imagen de diagrama de conexiones]**

---

## 🔌 Instalación del Entorno ESP32

1. **Instala Arduino IDE**:\
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
```

Puedes cambiar los topics MQTT si deseas personalizar los canales de comunicación:

```cpp
const char* topic_acceso = "utelvt/sensor/huella/acceso";
const char* topic_registro = "utelvt/sensor/huella/registro";
const char* topic_general = "utelvt/sensor/huella/#";
```

Una vez configurado todo, **compila y sube el código a la ESP32** desde Arduino IDE.

---

## 🌐 Configuración de Node-RED

### 1. **Instalación**

Instala Node.js desde su sitio oficial:\
🔗 [https://nodejs.org/](https://nodejs.org/)

Luego instala Node-RED usando:

```bash
npm install -g --unsafe-perm node-red
```

Ejecuta Node-RED con:

```bash
node-red
```

Accede a la interfaz desde tu navegador en:\
📍 [http://localhost:1880](http://localhost:1880)

---

### 2. **Importar el Flow**

- Abre Node-RED.
- Haz clic en el menú (esquina superior derecha) > `Import`.
- Copia y pega el contenido del archivo `.json` o súbelo desde tu PC.
- Asegúrate de modificar los **topics MQTT** para que coincidan con los de tu ESP32.

---

### 3. **Instalar Pallets Requeridos**

Desde Node-RED:

- Ve al menú > `Manage palette` > `Install`.
- Instala los siguientes nodos:
  - `node-red-dashboard`
  - `node-red-node-ui-led`
  - `node-red-node-ui-table`
  - `node-red-node-ui-switch`

Esto te permitirá crear un dashboard visual para:

- Visualizar accesos
- Encender LEDs virtuales
- Activar o desactivar sensores
- Mostrar tarjetas RFID registradas, etc.

---

## 🧪 Prueba del Sistema

1. Enciende tu ESP32 y asegúrate que esté conectado al WiFi.
2. Node-RED debe estar en ejecución y mostrar mensajes entrantes.
3. Prueba pasar una huella registrada o una tarjeta RFID válida.
4. Observa cómo se activa el actuador y cómo el sistema reacciona en el dashboard.

---

## 🔐 Seguridad Recomendaciones

- Este proyecto utiliza el broker público `test.mosquitto.org`, por lo tanto:
  - **Recomendado:** Cambia los topics por otros únicos.
  - **Ideal:** Ejecuta tu propio broker MQTT local con Mosquitto.

---

## ☁️ ¿Cómo usar Mosquitto en local?

### Instalación en Windows/Linux/Mac

#### En Windows:

1. Descarga Mosquitto desde:\
   [https://mosquitto.org/download/](https://mosquitto.org/download/)
2. Instala y ejecuta el servicio Mosquitto.

#### En Linux:

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

#### En Mac (con Homebrew):

```bash
brew install mosquitto
brew services start mosquitto
```

### Configuración

Puedes usar la configuración por defecto, pero para mejor seguridad se recomienda configurar:

- Autenticación (usuario/contraseña)
- TLS/SSL si accedes de forma remota

---

## 📁 Archivos del Repositorio

| Archivo                 | Descripción                            |
| ----------------------- | -------------------------------------- |
| `control_acceso.ino`    | Código principal para el ESP32         |
| `flow_nodered.json`     | Flujo completo del panel en Node-RED   |
| `diagrama_conexion.png` | Imagen del circuito (agrega tu imagen) |

---

## 🤝 Contribuciones

¡Este es un proyecto de código abierto! Si deseas aportar mejoras, nuevas funcionalidades o correcciones, ¡eres bienvenido a contribuir mediante *pull requests* o *issues*!

---

## 🧑‍💻 Autor

Desarrollado por [**HenryGC**](https://github.com/Henry-GC)\
📍 Esmeraldas, Ecuador\
💬 Contacto: [contacto@henrygc.com](mailto:contacto@henrygc.com)

---

## 📜 Licencia

Este proyecto se distribuye bajo la licencia **MIT**.\
Libre para usar, modificar y distribuir. ¡Hazlo tuyo!

---

## 🌟 Agradecimientos

- Comunidad Arduino
- Comunidad Node-RED
- Proyecto Mosquitto
- Adafruit y otros proveedores de librerías

---

¡No dudes en clonar este repositorio y construir tu propio sistema de acceso inteligente basado en ESP32!

