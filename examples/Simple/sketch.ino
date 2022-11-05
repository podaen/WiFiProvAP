/*
 Name:		sketch.ino
 Created:	11/3/2022 10:12:01 AM
 Author:	podaen
*/

#include <esp32-WiFi.h>

/*MAIN*/
void setup() {
    Serial.begin(115200);
    while (!Serial);
    startWifi();
    Serial.println("setup done");
    Serial.println("");
}
void loop() {
    delay(50);
}