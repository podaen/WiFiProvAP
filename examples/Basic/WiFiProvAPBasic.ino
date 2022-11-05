/*
 Name:		WiFiProvAPBasic.ino
 Created:	11/3/2022 10:12:01 AM
 Author:	podaen
*/

/*download the "esp prov" app for android or ios
to list the networks in the app, be sure that the flash options are:
Arduino Runs On : Core 1
Events Run On : Core 0*/

/*# additional Wi-Fi config in sdkconfig (not needed)
CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM=16
CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM=64
CONFIG_ESP32_WIFI_DYNAMIC_TX_BUFFER_NUM=64
CONFIG_ESP32_WIFI_TX_BA_WIN=32
CONFIG_ESP32_WIFI_RX_BA_WIN=32
CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED=y
CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED=y*/

#include <WiFiProvAP.h>

#include "nvs_flash.h"

const char* service_name = "my_device";
//const char* service_key = "password";
wifi_prov_security_t security = WIFI_PROV_SECURITY_1;//WIFI_PROV_SECURITY_0
scheme_handler_t provHandle = WIFI_PROV_SCHEME_HANDLER_NONE;//WIFI_PROV_SCHEME_HANDLER_MAX
const char* pop = "abcd1234";

int cntNetworks = 0;//WiFiScan

/*ERASE FLASH SETTINGS*/
void eraseNVS() {
    esp_err_t ret;
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    Serial.println("nvs flash erased");
    ESP_ERROR_CHECK(ret);
}
/*Print freeGap*/
void printFreeGap() {
    //getFreeHeap: 251940 from 290116
    Serial.println("");
    Serial.print("xPortGetFreeHeapSize: ");
    Serial.println(xPortGetFreeHeapSize());
    Serial.print("getFreeHeap: ");
    Serial.println(ESP.getFreeHeap());//290116//Since ESP32 has 328 KiB of data RAM
    Serial.print("getFreePsram: ");
    Serial.println(ESP.getFreePsram());
    Serial.println("");
}
/*EVENTS*/
void SysProvWiFiEvent(arduino_event_t* sys_event)
{
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        //auto reconnect is enabled
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("\nDisconnected. Connecting to the AP again... ");
        //auto reconnect is enabled
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("WiFi sta authmode change");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        Serial.println("");
        WiFi.setSleep(false);//full power
        printFreeGap();
        //connect to clients
        break;
    case ARDUINO_EVENT_PROV_START:
        Serial.println("\nProvisioning started\nGive Credentials of your access point using \" Android app \"");
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char*)sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const*)sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL: {
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) {
            Serial.println("\nWi-Fi AP password incorrect");
            eraseNVS();
            ESP.restart();
        }
        else {
            Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
            eraseNVS();
            ESP.restart();
        }
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        break;
    default:
        break;
    }
}
/*MAIN*/
void setup() {
    Serial.begin(115200);
    while (!Serial);
    //eraseNVS();//erase credentials and etcetera
    WiFi.disconnect(true);
    delay(100);
    WiFi.onEvent(SysProvWiFiEvent);
    WiFi.setAutoReconnect(true);
    WiFiProvAP.beginProvisionAP(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, security, pop, service_name);
    Serial.println("setup done");
    Serial.println("");
}
void loop() {
    delay(50);
}