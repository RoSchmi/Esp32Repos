// Program Esp32_Fritzbox_TR064_Schaltsteckdose_Klingel
// by RoSchmi
// modifiziert nach Projekt
// https://github.com/christophreuter/TR-064
// von Dr. Christoph Reuter
// Modifiziert am 5.7.2021

// Verwendete library:
// https://github.com/Aypac/Arduino-TR-064-SOAP-Library

// Auf der Fritzbox vorzunehmende Einstellungen findet man hier:
// https://www.schlaue-huette.de/apis-co/fritz-tr064/
//
// Standardmäßig ist die TR-064 Schnittstelle nicht aktiviert.
// -->FritzBox Weboberfläche-->Expertenansicht
// Heimnetz » Heimnetzübersicht » Netzwerkeinstellungen
// --> Zugriff für Anwendungen zulassen --> neu starten
// --> Usernamen/Passwort angeben (System » FritzBox Benutzer)
// Benutzereinstellungen: „FRITZBox Einstellungen“ und „Sprachnachrichten, Faxnachrichten, FRITZApp Fon und Anrufliste“ aktivieren.
// wenn alles richtig, öffne: http://fritz.box:49000/tr64desc.xml

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <tr064.h>
#include "config_secret.h"
#include "config.h"

// Default Esp32 stack size of 8192 byte is not enough for some applications.
// --> configure stack size dynamically from code to 16384
// https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/4
// Patch: Replace C:\Users\thisUser\.platformio\packages\framework-arduinoespressif32\cores\esp32\main.cpp
// with the file 'main.cpp' from folder 'patches' of this repository, then use the following code to configure stack size
#if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
  uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
#endif


WiFiMulti wiFiMulti;
 
//-------------------------------------------------------------------------------------
// Put your router settings here
//-------------------------------------------------------------------------------------
// Wifi network name (SSID)
const char* wifi_ssid = IOT_CONFIG_WIFI_SSID; 
// Wifi network password
const char* wifi_password = IOT_CONFIG_WIFI_PASSWORD;
// The username if you created an account, "admin" otherwise
const char* fuser = FRITZ_USER;
// The password for the aforementioned account.
const char* fpass = FRITZ_PASSWORD;
//IP address of your router. This should be "192.168.179.1" for most FRITZ!Boxes
//const char* IP = "192.168.179.1";
const char* IP = FRITZ_IP_ADDRESS;
// Port of the API of your router. This should be 49000 for all TR-064 devices.
const int PORT = 49000;
// -------------------------------------------------------------------------------------
 
// TR-064 connection
TR064 connection(PORT, IP, fuser, fpass);
 
// Die AIN der DECT!200 Steckdose findet sich im FritzBox Webinterface
//const String Steckdose1 = "123456 123456";
const String Steckdose1 = FRITZ_DEVICE_AIN_01;

// forward declarations
void ensureWIFIConnection();
void SetSwitch(String AIN, String state);
void GetDeviceInfo(String AIN);
void GetGenericDeviceInfo(String AIN);
void WahlRundruf();
void serialEvent();


void setup() {
    Serial.begin(115200);
    while(!Serial);
    Serial.println("boot...");
 
    // Connect to wifi
    ensureWIFIConnection();
    Serial.println("WIFI connected...");

    delay(5000);

    // Bei Problemen kann hier die Debug Ausgabe aktiviert werden
    connection.debug_level = DEBUG_VERBOSE;
    
    connection.init();
    
    /*
while (true)
    {
        delay(1000);
    }
    */
}
 
void loop() {
    delay(200);
    GetDeviceInfo(Steckdose1);
    delay(500);
    //GetGenericDeviceInfo(Steckdose1);
    delay(500);
    Serial.println("Making Wahlrundruf");
    WahlRundruf();

    GetDeviceInfo(Steckdose1);
    delay(500);
    SetSwitch(Steckdose1, "ON");
    Serial.println("Switched on");
    delay(20000);
    SetSwitch(Steckdose1, "OFF");
     Serial.println("Switched off");
    delay(20000);


    while (true)
    {
        delay(200);
    }
}
/*
    if(Serial.available()) 
    {
        Serial.println("Available");
        delay(500);
        serialEvent();
    }
    else
    {
        Serial.println("Not available");
        delay(500);
    }
    
    delay(20);
    }
    */

 
void serialEvent(){
  String inData;
  char inChar;
    while(Serial.available() > 0) {
        inChar = Serial.read();
        if(inChar != '\n' && inChar != '\r') {
            inData += inChar;
        }
    }
    if(inData == "on") {
        SetSwitch(Steckdose1, "ON");
    }else if(inData == "off") {
        SetSwitch(Steckdose1, "OFF");
    }else if(inData == "toggle") {
        SetSwitch(Steckdose1, "TOGGLE");
    }else if(inData == "info") {
      GetDeviceInfo(Steckdose1);
    }else if(inData == "ring") {
     // WahlRundruf();  
    }
    Serial.println(inData);
}
 
void SetSwitch(String AIN, String state) {
    ensureWIFIConnection();
    String paramsb[][2] = {{"NewAIN", AIN},{"NewSwitchState", state}};
    connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "SetSwitch", paramsb, 2);
}
 
void GetDeviceInfo(String AIN) {
    ensureWIFIConnection();
    String paramsb[][2] = {{"NewAIN", AIN}};
    String reqb[][2] = {{"NewMultimeterPower", ""}, {"NewTemperatureCelsius", ""}};
    connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetSpecificDeviceInfos", paramsb, 1, reqb, 2);
    float power = reqb[0][1].toInt() / 100.0;
    float temp = reqb[1][1].toInt() / 10.0;
    Serial.print("Stromverbrauch: ");
    Serial.print(power, 1);
    Serial.println("W");
    Serial.print("Temperatur: ");
    Serial.print(temp, 1);
    Serial.println("*C");
}

void GetGenericDeviceInfo(String AIN) {
    ensureWIFIConnection();
    String paramsb[][2] = {{"NewAIN", AIN}};
    String reqb[][2] = {{"NewMultimeterPower", ""}, {"NewTemperatureCelsius", ""}};
    connection.action("urn:dslforum-org:service:X_AVM-DE_Homeauto:1", "GetGenericDeviceInfos", paramsb, 1, reqb, 2);
    float power = reqb[0][1].toInt() / 100.0;
    float temp = reqb[1][1].toInt() / 10.0;
    Serial.print("Stromverbrauch: ");
    Serial.print(power, 1);
    Serial.println("W");
    Serial.print("Temperatur: ");
    Serial.print(temp, 1);
    Serial.println("*C");
}


  //  Rundruffunktion über TR064 an der Fritzbox auslösen
void WahlRundruf() {
      String service = "urn:dslforum-org:service:X_VoIP:1";

  // Die Telefonnummer **9 ist der Fritzbox-Rundruf.
  String call_params[][2] = {{"NewX_AVM-DE_PhoneNumber", "**9"}};
  connection.action(service, "X_AVM-DE_DialNumber", call_params, 1);

  // Warte vier Sekunden bis zum auflegen
  delay(4000);
  connection.action(service, "X_AVM-DE_DialHangup");
  
}
 
void ensureWIFIConnection() {
    if((wiFiMulti.run() != WL_CONNECTED)) {
        wiFiMulti.addAP(wifi_ssid, wifi_password);
        while ((wiFiMulti.run() != WL_CONNECTED)) {
            delay(100);
        }
    }
}