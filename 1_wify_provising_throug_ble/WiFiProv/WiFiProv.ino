#include <WiFi.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Use UUIDs matching Flutter app's expectations
#define SERVICE_UUID        "0000ff00-0000-1000-8000-00805f9b34fb"
#define CREDENTIALS_UUID    "0000fff1-0000-1000-8000-00805f9b34fb"  // Changed to FFF1
#define STATUS_UUID         "0000fff2-0000-1000-8000-00805f9b34fb"  // Changed to FFF2

Preferences preferences;
BLECharacteristic *statusCharacteristic;

class CredentialsCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    // Use Arduino String instead of std::string
    String value = pCharacteristic->getValue().c_str(); // Convert to Arduino String

    Serial.println("Received credentials: " + value);  // Log the received credentials

    if (value.length() == 0) {
      Serial.println("Empty credentials received");
      statusCharacteristic->setValue("Empty credentials");
      statusCharacteristic->notify();
      return;
    }

    // Use Arduino String methods
    int colonIndex = value.indexOf(':');
    if (colonIndex == -1) {
      Serial.println("Invalid format for credentials");
      statusCharacteristic->setValue("Invalid format");
      statusCharacteristic->notify();
      return;
    }

    String ssid = value.substring(0, colonIndex);
    String password = value.substring(colonIndex + 1);

    Serial.println("Attempting to connect to SSID: " + ssid);  // Log SSID being used

    // Rest of your connection logic...
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
      delay(500);
      Serial.print(".");  // Show dots while trying to connect
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected, IP address: " + WiFi.localIP().toString());
      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.end();
      
      String connectionStatus = "Connected: " + WiFi.localIP().toString();
      statusCharacteristic->setValue(connectionStatus.c_str());
      statusCharacteristic->notify();  // Notify Flutter app about the Wi-Fi connection
      
    } else {
      Serial.println("WiFi connection failed");
      statusCharacteristic->setValue("Connection failed");
      statusCharacteristic->notify();  // Notify Flutter app about the failure
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting setup...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);  // Disconnect and reset auto-connect
  if(!WiFi.isConnected()){
    preferences.begin("wifi", false);
    preferences.clear();  // Clear saved credentials
    preferences.end();
  }{
    delay(5000); 
  }
  // Try saved credentials first
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  if (ssid != "" && password != "") {
    Serial.println("Trying saved WiFi credentials...");
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(5000);  // Wait for connection
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Already connected to WiFi: " + WiFi.localIP().toString());
      return;
    }
  }

  // BLE Setup
  Serial.println("Setting up BLE...");
  BLEDevice::init("PROV_ESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Single credentials characteristic
  BLECharacteristic *credentialsChar = pService->createCharacteristic(
    CREDENTIALS_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  credentialsChar->setCallbacks(new CredentialsCallback());

  // Status characteristic
  statusCharacteristic = pService->createCharacteristic(
    STATUS_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE Ready and Advertising");
}

void loop() {
  // Keep empty if not needed
}
