# Provisioning for Arduino

This sketch implements provisioning using various IDF components

# Description

This example allows Arduino user to choose either SOFTAP as a mode of transport, over which the provisioning related communication is to take place, between the device (to be provisioned) and the client (owner of the device).

# APIs introduced for provisioning

## WiFi.onEvent()

Using this API user can register to receive WiFi Events and Provisioning Events

## WiFi.beginProvisionAP()

WiFi.beginProvisionAP(void ( * scheme_cb)(), wifi_prov_scheme_event_handler_t scheme_event_handler, wifi_prov_security_t security, char * pop, char * service_name, char * service_key, uint8_t * uuid);

#### Parameters passed

*  function pointer : choose the mode of transfer
    * provSchemeSoftAP - Using SoftAP
        
* security : choose security type
    * WIFI_PROV_SECURITY_1 - It allows secure communication which consists of secure handshake using key exchange and proof of possession (pop) and encryption/decryption of messages.

    * WIFI_PROV_SECURITY_0 - It do not provide application level security, it involve simply plain text communication.

* scheme_event_handler : specify the handlers according to the mode chosen
    * SoftAp :
        - WIFI_PROV_EVENT_HANDLER_NONE

* pop : It is the string that is used to provide the authentication.

* service_name : Specify service name for the device, if it is not specified then default chosen name is PROV_XXX where XXX are the last 3 bytes of the MAC address.  

* service_key : Specify service key, if chosen mode of provisioning is BLE then service_key is always NULL

# NOTE

* If none of the parameters are specified in beginProvision then default provisioning takes place using SoftAP with
    * scheme = WIFI_PROV_SCHEME_SOFTAP
    * scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    * security = WIFI_PROV_SECURITY_1
    * pop = "abcd1234"
    * service_name = "PROV_XXX"
    * service_key = NULL
    * uuid = NULL

# Log Output
* Enable debuger : [ Tools -> Core Debug Level -> Info ] 

# Provisioning Tools
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/provisioning/wifi_provisioning.html#provisioning-tools

# Example output

## Provisioning using SoftAP

```
[I][WiFiProvAP.cpp] beginProvisionAP(): Starting AP using SOFTAP
 service_name : PROV_XXX
 password : 123456789
 pop : abcd1234

Provisioning started
Give Credentials of your access point using " Android app "

Received Wi-Fi credentials
	SSID : GIONEE M2
	Password : 123456789

Connected IP address : 192.168.43.120
Provisioning Successful
Provisioning Ends

```

## Credentials are available on device

```
[I][WiFiProvAP.cpp] beginProvisionAP(): Aleardy Provisioned, starting Wi-Fi STA
[I][WiFiProv.cpp] beginProvisionAP(): SSID : Wce*****
[I][WiFiProv.cpp] beginProvisionAP(): CONNECTING TO THE ACCESS POINT : 
Connected IP address : 192.168.43.120
```