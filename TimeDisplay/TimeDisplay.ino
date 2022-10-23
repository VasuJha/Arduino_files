
// Adapted from https://github.com/martinius96/ESP32-eduroam to work at UCL
//

#include <WiFi.h>
#include "esp_wpa2.h"   // wpa2 for connection to enterprise networks
#include "time.h"
#include "sntp.h"
#include <TFT_eSPI.h>
#include <SPI.h>

#define EAP_IDENTITY "zcabvjh@ucl.ac.uk"                
#define EAP_PASSWORD "V@nd2004!@#$"

const char* essid = "eduroam";
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI();

#define TFT_BLACK 0x0000 //black

void printLocalTimeTTGO() {//function that displays time on the TTGO

tft.fillScreen(TFT_BLACK);
struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    tft.println("No time available (yet)");
    return;
  }
  
  tft.setCursor(0, 0, 2);  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.println(&timeinfo, "%A");

  tft.setCursor(0, 40, 2);  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.println(&timeinfo, "%B %d %Y");

  tft.setCursor(0, 80, 2);  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.println(&timeinfo, "%H:%M:%S");
}


void printLocalTime()//function that displays time on the serial monitor
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}

void setup() {
  bool eduroamFound = false;
  tft.init();
  tft.setRotation(1);
  

  Serial.begin(115200);

  sntp_set_time_sync_notification_cb( timeavailable );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  delay(10);

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Scan available WiFi networks until eduroam is seen
  //
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Repeatedly scan until we see eduroam
  //
  while (!eduroamFound) {
    Serial.println("scan start");
    int n = WiFi.scanNetworks(); // WiFi.scanNetworks returns the number of networks found
    Serial.println("scan done");
    
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            int    rssi = WiFi.RSSI(i);
          
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(ssid);
            Serial.print(" (");
            Serial.print(rssi);
            Serial.print(")");
            Serial.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
            
            ssid.trim();
            if (ssid == essid) {
              Serial.print(" <==== eduroam found");
              eduroamFound = true;
            }
            Serial.println("");
        }
    }
    Serial.println("");

    // Wait a bit before scanning again
    if (!eduroamFound)
      delay(5000);
  }

  ////////////////////////////////////////////////////////////////////////////////
  //
  // If we come here, we've successfully found eduroam. Try to connect to it.
  //
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(essid);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // This is where the wpa2 magic happens to allow us to connect to eduroam
  //
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  WiFi.begin(essid);       //connect to eduroam
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi is connected to ");
  Serial.println(essid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //print LAN IP


}


void loop() {
  delay(1000);
  printLocalTime();     // it will take some time to sync time :)
  printLocalTimeTTGO();//function calls inside the loop so that time is updated regularly
}



