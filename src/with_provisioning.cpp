/*
Please read README.md file in this folder, or on the web:
https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiProv/examples/WiFiProv

Note: This sketch takes up a lot of space for the app and may not be able to flash with default setting on some chips.
  If you see Error like this: "Sketch too big"
  In Arduino IDE go to: Tools > Partition scheme > chose anything that has more than 1.4MB APP
   - for example "No OTA (2MB APP/2MB SPIFFS)"
*/
#if 0

#include <Arduino.h>

#include "WiFiProv.h"
#include "WiFi.h"

#include "network_config.h"
#include "manager.h"


// #define USE_SOFT_AP // Uncomment if you want to enforce using the Soft AP method instead of BLE
const char *pop = "abcd1234";           // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
const char *service_name = "PROV_123";  // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
const char *service_key = NULL;         // Password used for SofAP method (NULL = no password needed)
bool reset_provisioned = true;          // When true the library will automatically delete previously provisioned data.

static volatile bool wifi_connected = false;


// WARNING: SysProvEvent is called from a separate FreeRTOS task (thread)!
void SysProvEvent(arduino_event_t *sys_event) 
{
  switch (sys_event->event_id) 
  {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial0.print("\nConnected to IP address : ");
      Serial0.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      wifi_connected = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: Serial0.println("\nDisconnected. Connecting to the AP again... "); break;
    case ARDUINO_EVENT_PROV_START:            Serial0.println("\nProvisioning started\nGive Credentials of your access point using smartphone app"); break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
    {
      Serial0.println("\nReceived Wi-Fi credentials");
      Serial0.print("\tSSID : ");
      Serial0.println((const char *)sys_event->event_info.prov_cred_recv.ssid);
      Serial0.print("\tPassword : ");
      Serial0.println((char const *)sys_event->event_info.prov_cred_recv.password);
      //
      break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL:
    {
      #if defined(USE_SOFT_AP)
      Serial0.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
      if (sys_event->event_info.prov_fail_reason == NETWORK_PROV_WIFI_STA_AUTH_ERROR) 
      {
        Serial0.println("\nWi-Fi AP password incorrect");
      } 
      else 
      {
        Serial0.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
      }
      #else
      // PASS ... (when NOT using WiFi-AP as provisioning mechanism, this clause should NEVER be taken!)
      #endif
      break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS: Serial0.println("\nProvisioning Successful"); break;
    case ARDUINO_EVENT_PROV_END:          Serial0.println("\nProvisioning Ends"); break;
    default:                              break;
  }
}


void setup() 
{
  Serial0.begin(115200);

  WiFi.begin();  // no SSID/PWD - get it from the Provisioning APP or from NVS (last successful connection)
  WiFi.onEvent(SysProvEvent);

// BLE Provisioning using the ESP SoftAP Prov works fine for any BLE SoC, including ESP32, ESP32S3 and ESP32C3.
#if CONFIG_BLUEDROID_ENABLED && !defined(USE_SOFT_AP)
  Serial0.println("Begin Provisioning using BLE ...");
  // Sample uuid that user can pass during provisioning using BLE
  uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf, 0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02};
  WiFiProv.beginProvision(
    WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BLE, WIFI_PROV_SECURITY_1, pop, service_name, service_key, uuid, reset_provisioned
  );
  log_d("ble qr");
  WiFiProv.printQR(service_name, pop, "ble");
#else
    // NOTE: below code assumes SoftAP (WiFi) provisioning is used!!
  Serial0.println("Begin Provisioning using Soft AP");
  WiFiProv.beginProvision(NETWORK_PROV_SCHEME_SOFTAP, NETWORK_PROV_SCHEME_HANDLER_NONE, NETWORK_PROV_SECURITY_1, pop, service_name, service_key);
  log_d("wifi qr");
  WiFiProv.printQR(service_name, pop, "softap");
#endif

    Serial0.println("WiFi provisioning set up ...");
}


#include "HTTPClient.h"

const char *fileServer = "https://www2.cs.uic.edu/~i101/SoundFiles/";


static String httpGETRequest(const char* serverName) 
{
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "--"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}


void loop() 
{
    static bool hasDownloaded = false;

    vTaskDelay(10000);
    if (wifi_connected)
    {
        Serial0.println("WiFi connected ...\r\n");

        if (!hasDownloaded)
        {
            String content = httpGETRequest(fileServer);

            Serial0.println(content);

            hasDownloaded = true;
        }
    }
    else 
    {
        Serial0.println("Alive ...\r\n");
    }
}


#endif  // IF 0 ...
