#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>

// *******************
// CONFIGURACIÓN WIFI
// *******************
const char* ssid = "****";         // Reemplaza con tu SSID
const char* password = "********"; // Reemplaza con tu contraseña

// ********************
// CONFIGURACIÓN MQTT
// ********************
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;  // Puerto MQTT (generalmente 1883)
const char* mqtt_client_id = "ESP32_Acceso"; // Identificador único del cliente ESP32
const char* topic_acceso = "utelvt/sensor/huella/acceso";
const char* topic_registro = "utelvt/sensor/huella/registro";
const char* topic_general = "utelvt/sensor/huella/#";

WiFiClient espClient;
PubSubClient client(espClient);

#define SS_PIN 21
#define RST_PIN 5
#define LED_GREEN 26
#define LED_BLUE 25
#define BUTTON_FINGERPRINT 32

#define MAX_FINGERPRINTS 5
#define MAX_RFID_CARDS 5

#define FINGER_RX_PIN 16
#define FINGER_TX_PIN 17

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
MFRC522 mfrc522(SS_PIN, RST_PIN);

int storedFingerprints[MAX_FINGERPRINTS];
int fingerprintCount = 0;
String storedRFIDs[MAX_RFID_CARDS];
int rfidCount = 0;

byte knownCards[][4] = {
  {0xD3, 0x8A, 0x3E, 0x1C}, // Usuario 1
  {0x53, 0x66, 0x87, 0xFA}  // Usuario 2
};

bool buttonPressed = false;
unsigned long lastButtonTime = 0;
const unsigned long DEBOUNCE_DELAY = 200;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial2.begin(57600, SERIAL_8N1, FINGER_RX_PIN, FINGER_TX_PIN);
  initializeFingerprintSensor();

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Lector RFID inicializado");

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_FINGERPRINT, INPUT_PULLUP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  // Inicialización WiFi
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Inicialización MQTT
  client.setServer(mqtt_server, mqtt_port);
  connectMQTT();
  client.setCallback(mqttCallback);

  Serial.println("\nSistema de control de acceso listo");
  printMenu();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Mensaje recibido en el topic ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);

  if (String(topic) == topic_registro) {
    int huella = message.toInt();
    if (huella > 0 && huella < 128) {
      Serial.println("LECTURA DE REGISTRO DE HUELLA");
      publishMessage("utelvt/sensor/huella/registro/led", "ON");
      digitalWrite(LED_BLUE, HIGH);
      startFingerprintEnrollment((uint8_t)huella);
      publishMessage("utelvt/sensor/huella/registro/led", "OFF");
      digitalWrite(LED_BLUE, LOW);
    }
  }

  if (String(topic) == "utelvt/sensor/huella/manual" && message == "ON") {
    grantAccess("BOTON", "INTERFAZ");
  }
}

void loop() {
  // Mantener la conexión MQTT
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  checkRFID();
  checkFingerprint();

  delay(100);
}

// **********************
// FUNCIONES WIFI/MQTT
// **********************

void connectMQTT() {
  Serial.print("Conectando a MQTT broker...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("Conectado a MQTT broker");
  client.subscribe(topic_general);
}

void reconnectMQTT() {
  Serial.println("Reconectando a MQTT...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("Reconectado a MQTT");
  client.subscribe(topic_general);
}

void publishMessage(const char* topic, const char* payload) {
  client.publish(topic, payload);
  Serial.print("Publicado en ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);
}

// **********************
// FUNCIONES PRINCIPALES
// **********************

void checkRFID() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String cardUID = getCardUID();
    Serial.print("Tarjeta detectada. UID: ");
    Serial.println(cardUID);

    if (isAuthorizedCard(cardUID)) {
      grantAccess("RFID", cardUID.c_str()); // Pasar tipo y UID
    } else {
      denyAccess("RFID", cardUID.c_str()); // Pasar tipo y UID
    }
    mfrc522.PICC_HaltA();
  }
}

void checkFingerprint() {
  int fingerID = getFingerprintIDez();
  if (fingerID > 0) {
    Serial.print("Huella reconocida. ID: ");
    Serial.println(fingerID);
    grantAccess("Huella", String(fingerID).c_str()); // Pasar tipo e ID
  }
}

// **********************
// FUNCIONES DE HUELLAS
// **********************

void initializeFingerprintSensor() {
  Serial.println("\nInicializando sensor de huellas...");
  if (finger.verifyPassword()) {
    Serial.println("Sensor de huellas conectado correctamente");
    finger.getTemplateCount();
    Serial.print("Huellas registradas: ");
    Serial.println(finger.templateCount);
  } else {
    Serial.println("Error: No se pudo comunicar con el sensor de huellas");
    while (1) { delay(1); }
  }
}

void startFingerprintEnrollment(uint8_t message) {
  if (fingerprintCount >= MAX_FINGERPRINTS) {
    Serial.println("Error: Límite de huellas alcanzado");
    publishMessage(topic_registro, "error:limite_huellas");
    return;
  }

  Serial.println("\nModo registro de huellas");
  Serial.println("Ingrese el ID para la nueva huella (1-127):");

  int id = message;
  if (id <= 0 || id > 127) {
    Serial.println("ID inválido");
    publishMessage(topic_registro, "error:id_invalido");
    return;
  }

  Serial.print("Iniciando registro para ID: ");
  Serial.println(id);
  publishMessage(topic_registro, String("iniciado:").c_str());

  if (enrollFingerprint(id)) {
    storedFingerprints[fingerprintCount++] = id;
    Serial.println("Huella registrada exitosamente con ID: " + String(id));
    publishMessage(topic_registro, (String("Huella almacenada con Exito!! ID: ") + String(id)).c_str());
    delay(2000);
  } else {
    publishMessage(topic_registro, "Fallo en el registro, intente nuevamente");
  }
}

bool enrollFingerprint(uint8_t id) {
  int p = -1;

  Serial.println("Coloca tu dedo en el sensor...");
  publishMessage(topic_registro, "Coloque la huella dactilar");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagen capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      default:
        Serial.println("Error en captura");
        publishMessage(topic_registro, "error:captura_1");
        return false;
    }
    delay(50);
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error en procesamiento");
    publishMessage(topic_registro, "error:procesamiento_1");
    return false;
  }

  Serial.println("Retira tu dedo");
  publishMessage(topic_registro, "Retire el dedo");
  delay(2000);

  Serial.println("Coloca el mismo dedo nuevamente");
  publishMessage(topic_registro, "Coloque nuevamente la huella dactilar");
  while (finger.getImage() != FINGERPRINT_NOFINGER) delay(100);

  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Imagen capturada");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      default:
        Serial.println("Error en captura");
        publishMessage(topic_registro, "error:captura_2");
        return false;
    }
    delay(50);
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error en procesamiento");
    publishMessage(topic_registro, "error:procesamiento_2");
    return false;
  }

  Serial.println("Creando modelo...");
  publishMessage(topic_registro, "paso:creando_modelo");
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error: Las huellas no coinciden");
    publishMessage(topic_registro, "error:huellas_no_coinciden");
    return false;
  }

  Serial.println("Almacenando huella...");
  publishMessage(topic_registro, "Almacenando huella...");
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error al almacenar");
    publishMessage(topic_registro, "error:almacenamiento");
    return false;
  }

  delay(1000);
  return true;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

// **********************
// FUNCIONES DE RFID
// **********************

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  return uid;
}

bool isAuthorizedCard(String cardUID) {
  for (byte i = 0; i < sizeof(knownCards)/sizeof(knownCards[0]); i++) {
    String knownUID = "";
    for (byte j = 0; j < 4; j++) {
      knownUID += String(knownCards[i][j] < 0x10 ? "0" : "");
      knownUID += String(knownCards[i][j], HEX);
    }
    if (cardUID.equalsIgnoreCase(knownUID)) {
      return true;
    }
  }
  return false;
}

// **********************
// FUNCIONES DE ACCESO
// **********************

void grantAccess(const char* type, const char* id) {
  Serial.println("Acceso concedido por " + String(type) + ": " + String(id));
  publishMessage("utelvt/sensor/huella/acceso/led", "ON");
  publishMessage("utelvt/sensor/huella/acceso/ON", "ON");
  digitalWrite(LED_GREEN, HIGH);
  delay(3000);
  publishMessage("utelvt/sensor/huella/acceso/led", "OFF");
  digitalWrite(LED_GREEN, LOW);
}

void denyAccess(const char* type, const char* id) {
  Serial.println("Acceso denegado por " + String(type) + ": " + String(id));
  digitalWrite(LED_GREEN, LOW);
  publishMessage("utelvt/sensor/huella/acceso/led", "DENY");
  delay(3000);
  publishMessage("utelvt/sensor/huella/acceso/led", "OFF");
}

// **********************
// FUNCIONES AUXILIARES
// **********************

void printMenu() {
  Serial.println("\n-----------------------------");
  Serial.println("1. Presione el botón para registrar nueva huella");
  Serial.println("2. Acerca una tarjeta RFID autorizada");
  Serial.println("3. Coloque su dedo para verificación");
  Serial.println("-----------------------------\n");
}