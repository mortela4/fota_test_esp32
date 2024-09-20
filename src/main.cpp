/**
 * @file main.cpp
 * 
 * @brief Demo of 'Esp-GitHub-OTA' library.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <GitHubOTA.h>
#include <GitHubFsOTA.h>
#include "LittleFS.h"


// NOTE: important - this string should correspond to github tag used for Releasing (via. Github Actions)!!
#define VERSION "0.0.1"


// Replace your_username/your_repo with your values (ex. axcap/Esp-GitHub-OTA)
// This is a link to repo where your firmware updates will be pulled from
// #define RELEASE_URL "https://api.github.com/repos/your_username/your_repo/releases/latest"

// Use this version of the URL together with init_ota(VERSION, true) under debugging
// to spare yourself from getting timeout from GitHub API:
#define RELEASE_URL "https://github.com/mortela4/fota_test_esp32/releases/latest"

#define DELAY_MS 1000

#define SSID        "7Sense_Guest"
#define PASSWORD    "moloveien2018"

#define LED_BUILTIN 13

GitHubOTA OsOta(VERSION, RELEASE_URL, "firmware.bin", true);
GitHubFsOTA FsOta(VERSION, RELEASE_URL, "filesystem.bin", true);

void setup_wifi();
void listRoot();

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while(!Serial);

  Serial.printf("Current firmware version: %s\n", VERSION); 
  Serial.printf("Current filesystem version: %s\n", VERSION); 

  LittleFS.begin();
  listRoot();

  setup_wifi();
  
  // Chech for updates
  FsOta.handle();
  OsOta.handle();
}

void loop()
{
  // Your code goes here
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(DELAY_MS);                  // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(DELAY_MS);                  // wait for a second
}

void listRoot(){
  Serial.printf("Listing root directory\r\n");

  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();

  while(file){
    Serial.printf("  FILE: %s\r\n", file.name());
    file = root.openNextFile();
  }
}

void setup_wifi(){
  Serial.println("Initialize WiFi");
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

