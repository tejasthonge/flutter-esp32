#include "WiFi.h"
#include "WiFiProv.h"
#include <Preferences.h>

// Define variables for provisioning
#define SERVICE_NAME "PROV_123"  // Name of your device for provisioning
#define POP "abcd1234"          // Proof of possession (PIN code)

const char *service_key = NULL;  // Optional password for BLE method (NULL = no password)
bool reset_provisioned = true;   // Automatically reset previous provisioning data

Preferences preferences;  // To store Wi-Fi credentials

// Wi-Fi event handler
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Connected IP address: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("Disconnected. Reconnecting...");
      WiFi.begin();  // Reconnect to Wi-Fi
      break;
    default:
      break;
  }
}

// Wi-Fi provisioning event handler
void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nConnected to Wi-Fi with IP: ");
      Serial.println(WiFi.localIP());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("\nDisconnected. Trying to reconnect...");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
      // Received Wi-Fi credentials
      String ssid = (const char*)sys_event->event_info.prov_cred_recv.ssid;
      String password = (char const*)sys_event->event_info.prov_cred_recv.password;
      Serial.print("\nReceived Wi-Fi credentials:\nSSID: ");
      Serial.println(ssid);
      Serial.print("Password: ");
      Serial.println(password);

      // Save credentials in non-volatile storage (Preferences)
      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.end();

      // Connect to the Wi-Fi
      WiFi.begin(ssid.c_str(), password.c_str());

      break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL:
      Serial.println("\nProvisioning failed!");
      break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("\nProvisioning successful!");
      break;
    case ARDUINO_EVENT_PROV_END:
      Serial.println("\nProvisioning ended.");
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);  // Setup Wi-Fi event handler
  WiFi.mode(WIFI_MODE_STA);  // Start in STA mode

  // Check if Wi-Fi credentials are already stored
  preferences.begin("wifi", true);  // Open Preferences in read mode
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");

  if (ssid != "" && password != "") {
    // If credentials are found, connect to the Wi-Fi network
    Serial.println("Found saved credentials. Connecting to Wi-Fi...");
    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(1000);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to Wi-Fi!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect to Wi-Fi, starting BLE provisioning...");
      // Start BLE provisioning if the connection failed or no credentials exist
      WiFiProv.beginProvision(NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BLE, NETWORK_PROV_SECURITY_1, POP, SERVICE_NAME, service_key, NULL, reset_provisioned);
    }
  } else {
    // If no saved credentials, start BLE provisioning
    Serial.println("No saved Wi-Fi credentials. Starting BLE provisioning...");
    WiFiProv.beginProvision(NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BLE, NETWORK_PROV_SECURITY_1, POP, SERVICE_NAME, service_key, NULL, reset_provisioned);
  }
}

void loop() {
  // Nothing to do in the loop as provisioning is handled in the event handler
}
