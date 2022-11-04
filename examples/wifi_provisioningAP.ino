/*
 Name:		wifi_provisioningAP.ino
 Created:	11/3/2022 10:12:01 AM
 Author:	podaen
*/

/*download the "esp prov" app for android or ios
to list the networks in the app, be sure that the flash options are:
Arduino Runs On : Core 1
Events Run On : Core 0*/

/*# Wi-Fi config in sdkconfig
CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM=16
CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM=64
CONFIG_ESP32_WIFI_DYNAMIC_TX_BUFFER_NUM=64
CONFIG_ESP32_WIFI_TX_BA_WIN=32
CONFIG_ESP32_WIFI_RX_BA_WIN=32
CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED=y
CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED=y*/

#include <WiFiProvAP.h>
#include <WiFi.h>

#include "nvs_flash.h"
#include "esp_wifi.h"

const char* service_name = "my_device";
//const char* service_key = "password";
wifi_prov_security_t security = WIFI_PROV_SECURITY_1;//WIFI_PROV_SECURITY_0
scheme_handler_t provHandle = WIFI_PROV_SCHEME_HANDLER_NONE;//WIFI_PROV_SCHEME_HANDLER_MAX
const char* pop = "abcd1234";

int cntNetworks = 0;//WiFiScan

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
        Connectivity();
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
/*MAIN*/
void setup() {
    Serial.begin(115200);
    while (!Serial);
    //eraseNVS();//erase credentials and etcetera
    printInfo();
    WiFi.disconnect(true);//folowing example https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiScan/WiFiScan.ino
    delay(100);
    WiFi.onEvent(SysProvWiFiEvent);
    scanNetwork();
    WiFi.setAutoReconnect(true);
    WiFiProvAP.beginProvisionAP(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, security, pop, service_name);
    Serial.println("setup done");
    Serial.println("");
}
void loop() {
    delay(50);
}
/*ERASE FLASH SETTINGS*/
void eraseNVS() {
    esp_err_t ret;
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    Serial.println("nvs flash erased");
    ESP_ERROR_CHECK(ret);
}
/*deINI*/
void deiniWifi() {
    Serial.println("Deintialize WiFi");
    WiFi.removeEvent(SysProvWiFiEvent);
    WiFi.disconnect(true);// delete old config
    WiFi.mode(WIFI_OFF);//espWiFiStop
}
/*NETWORK DETAILS*/
void Connectivity() {
    esp_err_t err;
    getProtocol();
    getBandwide();
    Serial.println("");
}
void getProtocol() {
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
    else
    {
        Serial.println("try to set WiFi_Protocol_11n");
        setProtocol();
    }
    if (getprotocol & WIFI_PROTOCOL_11G) {
        Serial.println("WiFi_Protocol_11g");
    }
    if (getprotocol & WIFI_PROTOCOL_11B) {
        Serial.println("WiFi_Protocol_11b");
    }
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
void getBandwide() {
    esp_err_t err;
    wifi_bandwidth_t wifi_bandwidth;
    err = esp_wifi_get_bandwidth(WIFI_IF_STA, &wifi_bandwidth);
    if (err != ESP_OK) {
        Serial.println("Could not get bandwide!");
        //log_e("Could not get protocol! %d", err);
    }
    if (wifi_bandwidth & WIFI_BW_HT20) {
        Serial.println("WiFi_Bandwide_WIFI_BW_HT20");
        setBandwide();
    }
    if (wifi_bandwidth & WIFI_BW_HT40) {
        //WIFI_BW_HT40 is supported only when the interface support 11N
        Serial.println("WiFi_Bandwide_WIFI_BW_HT40");
    }
}
bool setBandwide() {
    esp_err_t err;
    wifi_bandwidth_t wifi_bandwidth = WIFI_BW_HT40;//WIFI_BW_HT20
    err = esp_wifi_set_bandwidth(WIFI_IF_STA, wifi_bandwidth);//only in ap mode
    if (err != ESP_OK) {
        wifi_bandwidth = WIFI_BW_HT20;
        err = esp_wifi_set_bandwidth(WIFI_IF_STA, wifi_bandwidth);//only in ap mode
        if (err != ESP_OK) {
            Serial.println("failed to set bandwide");
            return false;
        }
        else
        {
            Serial.println("bandwide set to WIFI_BW_HT20");
            return true;
        }
    }
    else
    {
        Serial.println("bandwide set to WIFI_BW_HT40");
        return true;
    }
}
void getChannel() {
    //Serial.print("Wifi channel: ");
//Serial.println(WiFi.channel());
    esp_err_t err;
    uint8_t primary;
    wifi_second_chan_t second;
    err = esp_wifi_get_channel(&primary, &second);
    if (err != ESP_OK) {
        Serial.println("Could not get channel!");
        //log_e("Could not get protocol! %d", err);
    }
    else
    {
        Serial.print("primairy channel: ");
        Serial.println(primary);
    }
    if (second & WIFI_SECOND_CHAN_NONE) {//Doesn't print???
        Serial.println("WiFi_WIFI_SECOND_CHAN_NONE");
    }
    if (second & WIFI_SECOND_CHAN_ABOVE) {
        Serial.println("WiFi_WIFI_SECOND_CHAN_ABOVE");
    }
    if (second & WIFI_SECOND_CHAN_BELOW) {
        Serial.println("WiFi_WIFI_SECOND_CHAN_BELOW");
    }
}
/*SCAN NETWORK*/
void scanNetwork() {
    cntNetworks = WiFi.scanNetworks();
    printNetwork();
}
void printNetwork() {
    //#define WIFI_SCAN_RUNNING (-1)
    //#define WIFI_SCAN_FAILED (-2);
    if (cntNetworks == WIFI_SCAN_FAILED) {
        Serial.println("Scan Failed!");//returs -2
        return;
    }
    if (!cntNetworks) {
        Serial.println("no networks found");
        return;
    }
    if (cntNetworks > 0) {
        Serial.print(cntNetworks);
        Serial.println(" networks found");
        for (int i = 0; i < cntNetworks; ++i) {
            // Print SSID and RSSI for each network found
            //Serial.print(i + 1);
            //Serial.print(": ");
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
/*CHIP INFO*/
void printInfo() {
    Serial.println("");
    Serial.println("");
    Serial.println(F("START " __FILE__));
    Serial.println(F("FROM " __DATE__));
    Serial.println("");
    chipInfo();
    getCPUspeed();
    printFreeGap();
}
void chipInfo() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores\nWiFi%s%s\n",
        chip_info.cores,
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d\n", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
    chipId.toUpperCase();

    Serial.printf("Chip id: %s\n", chipId.c_str());

    String Modelno = ESP.getChipModel();

    Serial.printf("Model number: %s\n", Modelno.c_str());
}
void getCPUspeed() {
    Serial.print("CPU Freq: ");
    Serial.println(getCpuFrequencyMhz());
}
void setCPUspeed(uint32_t MHz) {
    setCpuFrequencyMhz(MHz);
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