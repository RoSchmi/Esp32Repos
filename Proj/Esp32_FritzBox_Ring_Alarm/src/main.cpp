// Program 'Esp32_FritzBox_Ring_Alarm'
// Copyright RoSchmi 2021, License Apache 2.0

// adapted from:
// https://www.open4me.de/index.php/2016/10/fritzbox-esp8266-laesst-telefon-klingeln/#more-719

// Before you can start:
// Define WiFi-Credentials, FritzBox-Credentials and Sinric Pro Credentials
// in file 'config_secrete.h' (take 'config_secret_template.h' as a template)

// Define 'TRANSPORT_PROTOCOL' (http or https) in file config.h
// When you begin to work with this App set TRANSPORT_PROTOCOL = 0 (http)
// The https secured connection will not work before you include the specific
// certificate of your personal FritzBox in 'config.h'
// Instructions how to get the certificate are given in the file 'config.h'
// When you have included the correct certificate, set TRANSPORT_PROTOCOL = 1

// The FRITZ_DEVICE_AIN can be found on your Fritz!Dect 200 powersocket
// To get the Sinric Pro Credentials have a look at:
// https://sinric.pro/de-index.html
// https://sinricpro.github.io/esp8266-esp32-sdk/index.html
// https://github.com/sinricpro

#include <Arduino.h>
//#include "ESPAsyncWebServer.h"
#include "defines.h"
#include "config_secret.h"
#include "config.h"
//#include "SinricPro_Generic.h"
//#include "SinricProSwitch.h"

#include "FreeRTOS.h"
#include "Esp.h"
#include "esp_task_wdt.h"
#include <rom/rtc.h>

#include "HTTPClient.h"
#include "WiFiClientSecure.h"

#include "WiFiClient.h"
#include "RsHttpFritzApi.h"

// Default Esp32 stack size of 8192 byte is not enough for some applications.
// --> configure stack size dynamically from code to 16384
// https://community.platformio.org/t/esp32-stack-configuration-reloaded/20994/4
// Patch: Replace C:\Users\thisUser\.platformio\packages\framework-arduinoespressif32\cores\esp32\main.cpp
// with the file 'main.cpp' from folder 'patches' of this repository, then use the following code to configure stack size
#if !(USING_DEFAULT_ARDUINO_LOOP_STACK_SIZE)
  uint16_t USER_CONFIG_ARDUINO_LOOP_STACK_SIZE = 16384;
#endif

RESET_REASON resetReason_0;
RESET_REASON resetReason_1;

uint8_t lastResetCause = 0;

const char *ssid = IOT_CONFIG_WIFI_SSID;
const char *password = IOT_CONFIG_WIFI_PASSWORD;

// if a queue should be used to store commands see:
//https://techtutorialsx.com/2017/08/20/esp32-arduino-freertos-queues/

bool powerState1 = false;
bool powerState2 = false;

typedef struct
{
   bool actState = false;
   bool lastState = true;
}ButtonState;

//RoSchmi
#define GPIOPin 0
#define BUTTON_1 GPIOPin
#define BUTTON_2 GPIOPin

#define FlashButton GPIOPin

bool buttonPressed = false;

volatile ButtonState flashButtonState;

#define LED_BUILTIN 2

int LED1_PIN = LED_BUILTIN;

typedef struct 
{      
  // struct for the std::map below
  int relayPIN;
  int flipSwitchPIN;
  int index;
} deviceConfig_t;

// Sinric Pro
// this is the main configuration
// please put in your deviceId, the PIN for Relay and PIN for flipSwitch and an index to address the entry
// this can be up to N devices...depending on how much pin's available on your device ;)
// right now we have 2 devicesIds going to 1 LED and 2 flip switches (set to the same button)

/*
std::map<String, deviceConfig_t> devices =
{
  //{deviceId, {relayPIN,  flipSwitchPIN, index}}
  // You have to set the pins correctly.
  // In this App we used -1 when the relay pin shall be ignored 

  { SWITCH_ID_1, {  (int)LED1_PIN,  (int)BUTTON_1, 0}},
  { SWITCH_ID_2, {  (int)-1, (int)BUTTON_2, 1}} 
};
*/

uint32_t millisAtLastAction;
uint32_t millisBetweenActions = 10000;

X509Certificate myX509Certificate = myfritzbox_root_ca;

#if TRANSPORT_PROTOCOL == 1
    static WiFiClientSecure wifi_client;
    Protocol protocol = Protocol::useHttps;
  #else
    static WiFiClient wifi_client;
    Protocol protocol = Protocol::useHttp;
  #endif

HTTPClient http;
static HTTPClient * httpPtr = &http;

uint64_t loopCounter = 0;
String fritz_SID = "";
FritzApi fritz((char *)FRITZ_USER, (char *)FRITZ_PASSWORD, FRITZ_IP_ADDRESS, protocol, wifi_client, httpPtr, myX509Certificate);

// not used
void GPIOPinISR()
{
  buttonPressed = true;
}

// forward declarations
void print_reset_reason(RESET_REASON reason);
//bool onPowerState(String deviceId, bool &state);
//bool onPowerState2(const String &deviceId, bool state);
//bool onPowerState1(const String &deviceId, bool state);
//void setupSinricPro(bool restoreStates);
void handleButtonPress();

void setup() {
  Serial.begin(BAUD_RATE);
  //while(!Serial);

  pinMode(FlashButton, INPUT_PULLUP);
  //attachInterrupt(FlashButton, GPIOPinISR, FALLING);  // not used

  pinMode(LED1_PIN, OUTPUT);

  // Wait some time (3000 ms)
  uint32_t start = millis();
  while ((millis() - start) < 3000)
  {
    delay(10);
  }
  //RoSchmi
  Serial.println("\r\nStarting");
  
  resetReason_0 = rtc_get_reset_reason(0);
  resetReason_1 = rtc_get_reset_reason(1);
  lastResetCause = resetReason_1;
  Serial.printf("Last Reset Reason: CPU_0 = %u, CPU_1 = %u\r\n", resetReason_0, resetReason_1);
  Serial.println("Reason CPU_0: ");
  print_reset_reason(resetReason_0);
  Serial.println("Reason CPU_1: ");
  print_reset_reason(resetReason_1);

  delay(3000);

  Serial.print(F("\nStarting ConnectWPA on "));
  Serial.print(BOARD_NAME);
  Serial.print(F(" with "));
  Serial.println(SHIELD_TYPE); 
  
  // Wait some time (3000 ms)
  start = millis();
  while ((millis() - start) < 3000)
  {
    delay(10);
  }

  #if WORK_WITH_WATCHDOG == 1
  // Start watchdog with 20 seconds
  if (esp_task_wdt_init(20, true) == ESP_OK)
  {
    Serial.println(F("Watchdog enabled with interval of 20 sec"));
  }
  else
  {
    Serial.println(F("Failed to enable watchdog"));
  }
  esp_task_wdt_add(NULL);

  //https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/watchdog-und-heartbeat

  //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html
  #endif 



  //Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  Serial.println(F("First disconnecting, then\r\nConnecting to WiFi-Network"));
  
  while (WiFi.status() != WL_DISCONNECTED)
  {
    WiFi.disconnect();
    delay(200); 
  }
  WiFi.begin(ssid, password);

if (!WiFi.enableSTA(true))
{     
    Serial.println("Connect failed.");
    delay(10 * 1000);  // Reboot after 10 seconds
    ESP.restart(); 
}

#if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
#endif 

// not tested
#if USE_STATIC_IP == 1
  if (!WiFi.config(presetIp, presetGateWay, presetSubnet, presetDnsServer1, presetDnsServer2))
  {
    while (true)
    {
      // Stay in endless loop
    lcd_log_line((char *)"WiFi-Config failed");
      delay(3000);
    }
  }
  else
  {
    lcd_log_line((char *)"WiFi-Config successful");
    delay(1000);
  }
  #endif

  uint32_t tryConnectCtr = 0;
  while (WiFi.status() != WL_CONNECTED)
  {  
    delay(100);
    Serial.print((tryConnectCtr++ % 40 == 0) ? "\r\n" : "." );  
  }

  Serial.print(F("\r\nGot Ip-Address: "));
  Serial.println(WiFi.localIP());

  if (fritz.init())
  {
    Serial.println(F("Initialization for FritzBox succeeded"));
  }
  else
  {
    Serial.println(F("Initialization for FritzBox failed, rebooting"));
    delay(10 * 1000); // 10 seconds
    ESP.restart();  
  }
  fritz_SID = fritz.testSID();
  //If wanted, printout SID
  Serial.printf("Actual SID is: %s\r\n", fritz_SID.c_str());

  // if true: when the device connects to the server the sever will
  // send the last states of the switch to the device
  // if false: the device will actualize the server to the state just beeing present on the device
  //bool restoreStatesFromServer = false;
  //setupSinricPro(restoreStatesFromServer);

  delay(1000);
  
  /*
  if (!restoreStatesFromServer)
  { 
    bool actualSocketState = fritz.getSwitchState(FRITZ_DEVICE_AIN_01);
    
    onPowerState1(SWITCH_ID_1, actualSocketState);  // once more switch Fritz!Dect socket to actual state  

    // get Switch device back
    SinricProSwitch& mySwitch = SinricPro[SWITCH_ID_1];
    // send powerstate event      
    mySwitch.sendPowerStateEvent(actualSocketState); // send the actual powerState to SinricPro server
    Serial.println("PowerState from Fritz!Dect was transmitted to server"); 
  }
  */
  
  // Set time interval for repeating commands
  millisAtLastAction = millis();
  millisBetweenActions = 60 * 1000;  //to refresh fritz_SID every minute
  
  Serial.println("\r\nStarting Ring Test\r\n");
  if (fritz.startRingTest(1))
  {
    Serial.println("Starting Ringtest was successful");
  }
  else
  {
    Serial.println("Starting Ringtest failed");
  }
  delay (5000);
  Serial.println("\r\nStopping Ring Test\r\n");
  if (fritz.stopRingTest(1))
  {
    Serial.println("Stopping Ringtest was successful");
  }
  else
  {
    Serial.println("Stopping Ringtest failed");
  }

  while (true)   // Stay in loop
  {
    digitalWrite(LED1_PIN, LOW);
    delay(1000);
    digitalWrite(LED1_PIN, HIGH);
    delay(1000);
  }
}

void loop() 
{ 
  if (++loopCounter % 100000 == 0)   // Reset watchdog every 100000 th round
  {
    //Serial.println("Reset WDG");   
    #if WORK_WITH_WATCHDOG == 1
      esp_task_wdt_reset();
    #endif
  }
  if ((millis() - millisAtLastAction) > millisBetweenActions) // time interval expired?
  {
    
     //Serial.println(F("Testing SID"));
     millisAtLastAction = millis();
     if (fritz_SID != fritz.testSID())
     {
        if (fritz.init())
        {
          fritz_SID = fritz.testSID();
          Serial.println(F("Re-init for FritzBox succeeded"));
        }
        else
        {
          Serial.println(F("Re-init for FritzBox failed, rebooting"));
          delay(10 * 1000); // 10 seconds
          ESP.restart();  
        }
     }  
  }
  //SinricPro.handle();
  //handleButtonPress();
}

/*
void handleButtonPress()
{
  if (digitalRead(FlashButton) == LOW)
  {   
    flashButtonState.lastState = flashButtonState.actState;    
    flashButtonState.actState = true;
    if (flashButtonState.actState != flashButtonState.lastState)  // if has toggled
    {     
      if (onPowerState1(SWITCH_ID_1, !powerState1))             // switch Fritz!Dect socket
      {       
        // get Switch device back
        SinricProSwitch& mySwitch = SinricPro[SWITCH_ID_1];
        // send powerstate event      
        mySwitch.sendPowerStateEvent(powerState1); // send the new powerState to SinricPro server
        Serial.println("(Switched manually via flashbutton)");        
      }    
    }     
  }
  else
  {
    flashButtonState.actState = false;
  }
}
*/

/*
bool onPowerState(String deviceId, bool &state)
{
  bool returnResult = false;
  //Serial.println( String(deviceId) + String(state ? " on" : " off")); 
  switch (devices[deviceId].index)
  {
    case 0:
    {
      returnResult = onPowerState1(deviceId, state);
    }
    break;
    case 1:
    {
      returnResult = onPowerState2(deviceId, state);
    }
    break; 
    default:
    {}
  }
  return returnResult;
}
*/

/*
bool onPowerState1(const String &deviceId, bool state)
{
  bool returnResult = false;
  if (state == true)
  {
    returnResult = fritz.setSwitchOn(FRITZ_DEVICE_AIN_01);
  }
  else
  {
    returnResult = !fritz.setSwitchOff(FRITZ_DEVICE_AIN_01);
  }
  if (returnResult == true)
  {
    int relayPIN = devices[deviceId].relayPIN; // get the relay pin for corresponding device
    if (relayPIN != -1)
    {
      digitalWrite(relayPIN, state);      // set the new relay state
    }
    Serial.printf("Device 1 turned %s\r\n", state ? "on" : "off");
    powerState1 = state;
    flashButtonState.actState = state;    
  }
  else
  {
    Serial.printf("Failed to turn Device 1 %s\r\n", state ? "on" : "off");
    //powerState1 = state;
    //flashButtonState.actState = state;  
  }    
  return returnResult; // request handled properly
}
*/

/*
bool onPowerState2(const String &deviceId, bool state)
{
  Serial.printf("Device 2 turned %s\r\n", state ? "on" : "off");
  powerState2 = state;
  return true; // request handled properly
}
*/

/*
// Create devices in Sinric Pro
// restoreStates: true means:
// restore the last states from the Sinric Server to this local device
void setupSinricPro(bool restoreStates)
{
  for (auto &device : devices)
  {
    // for each switch device defined in the map devices
    // create a SinricProSwitch instance with its deviceId
    const char *deviceId = device.first.c_str();
    // doesn't matter that the name is the same for all
    SinricProSwitch& mySwitch = SinricPro[deviceId];
    // we take the same callback for all and distinguish according to the index in the map    
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET); 
  // if true, restore the last states from the Sinric Server to this local device
  
  SinricPro.restoreDeviceStates(restoreStates);
}
*/

void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET");break;          /**<1, Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET");break;               /**<3, Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        /**<5, Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}