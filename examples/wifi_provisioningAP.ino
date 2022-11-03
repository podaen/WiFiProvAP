/*
 Name:		wifi_provisioningAP.ino
 Created:	11/3/2022 10:12:01 AM
 Author:	podaen
*/

//download the "esp prov" app for android or ios

#include <WiFiProvAP.h>
#include <WiFi.h>

#include "nvs_flash.h"
#include "esp_wifi.h"

int cntNetworks = 0;

void SysProvWiFiEvent(arduino_event_t* sys_event)
{
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        printNetwork();
        WiFiProvAP.beginProvisionAP(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, "abcd1234", "Prov_123");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        //periodicConnectionCheckTicker.attach_ms(TryWiFiConnectDelay, ConnectWifiSequense);
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("\nDisconnected. Connecting to the AP again... ");
        //periodicConnectionCheckTicker.attach_ms(TryWiFiConnectDelay, ConnectWifiSequense);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("WiFi sta authmode change");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        WiFi.setSleep(false);//full power
        printConnectivity();
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
            //provBegin();
            ESP.restart();
        }
        else {
            Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
            eraseNVS();
            ESP.restart();
            //provBegin();
        }
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        //WiFi.onEvent(WiFiEvent);
        //WiFi.removeEvent(SysProvEvent);
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        break;
    default:
        break;
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    printFreeGap();
    //eraseNVS();//erase credentials and etcetera
    WiFi.mode(WIFI_OFF);//WIFI_OFF//WIFI_STA
    WiFi.disconnect();//folowing example https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino
    delay(5000);
    WiFi.onEvent(SysProvWiFiEvent);
    scanNetwork(6);//my channel for 2.4GHz (only for testing)
    WiFi.setAutoReconnect(true);
    Serial.println("setup done");
}
void loop() {
    delay(50);
}

void eraseNVS() {
    esp_err_t ret;
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    Serial.println("nvs flash erased");
    ESP_ERROR_CHECK(ret);
}
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

void printConnectivity() {
    esp_err_t err;
    uint8_t getprotocol;
    err = esp_wifi_get_protocol(WIFI_IF_STA, &getprotocol);
    if (err != ESP_OK) {
        Serial.println("Could not get protocol!");
        //log_e("Could not get protocol! %d", err);
    }
    if (getprotocol & WIFI_PROTOCOL_11N) {
        Serial.println("WiFi_Protocol_11n");
    }
    if (getprotocol & WIFI_PROTOCOL_11G) {
        Serial.println("WiFi_Protocol_11g");
    }
    if (getprotocol & WIFI_PROTOCOL_11B) {//worst, but dificuld to set else
        Serial.println("WiFi_Protocol_11b");
    }

    wifi_bandwidth_t wifi_bandwidth;
    err = esp_wifi_get_bandwidth(WIFI_IF_STA, &wifi_bandwidth);
    if (err != ESP_OK) {
        Serial.println("Could not get bandwide!");
        //log_e("Could not get protocol! %d", err);
    }
    if (wifi_bandwidth & WIFI_BW_HT20) {
        Serial.println("WiFi_Bandwide_WIFI_BW_HT20");
    }
    if (wifi_bandwidth & WIFI_BW_HT40) {
        //WIFI_BW_HT40 is supported only when the interface support 11N
        Serial.println("WiFi_Bandwide_WIFI_BW_HT40");
    }

    //Serial.print("Wifi channel: ");
    //Serial.println(WiFi.channel());

    //uint8_t primary;
    //wifi_second_chan_t second;
    //err = esp_wifi_get_channel(&primary, &second);
    //if (err != ESP_OK) {
    //    Serial.println("Could not get channel!");
    //    //log_e("Could not get protocol! %d", err);
    //}
    //else
    //{
    //    Serial.print("primairy channel: ");
    //    Serial.println(primary);
    //}
    //if (second & WIFI_SECOND_CHAN_NONE) {//Doesn't print???
    //    Serial.println("WiFi_WIFI_SECOND_CHAN_NONE");
    //}
    //if (second & WIFI_SECOND_CHAN_ABOVE) {
    //    Serial.println("WiFi_WIFI_SECOND_CHAN_ABOVE");
    //}
    //if (second & WIFI_SECOND_CHAN_BELOW) {
    //    Serial.println("WiFi_WIFI_SECOND_CHAN_BELOW");
    //}

    //Serial.print("RSSI: ");
    //Serial.println(WiFi.RSSI());

    Serial.println("");
}
bool setProtocol() {
    esp_err_t err;
    const uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;
    err = esp_wifi_set_protocol(WIFI_IF_STA, protocol);
    if (err != ESP_OK) {
        //258 = ESP_ERR_INVALID_ARG
        Serial.println(err);
        Serial.println("Could not set protocol!");
        return false;
    }
    return true;
}
void printNetwork() {
    Serial.println(cntNetworks);
    //#define WIFI_SCAN_RUNNING (-1)
    //#define WIFI_SCAN_FAILED (-2);
    if (cntNetworks == WIFI_SCAN_FAILED) {
        Serial.println("Scan Failed!");//returs -2
    }
    if (!cntNetworks) {
        Serial.println("no networks found");
    }
    else
    {
        for (int i = 0; i < cntNetworks; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" ch:");
            Serial.print(WiFi.channel(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
        Serial.println("");
    }
}
void scanNetwork(uint8_t channel) {
    cntNetworks = WiFi.scanNetworks(false, false, false, 300UL, channel);
}