
// Program 'Esp32_Sinric_Ring_Doorbell_Alexa_Fritzbox_MyHome'
// Cpoyright RoSchmi 2021, License Apache 2.0
// 
// Esp32 application to ring Dect phones of a Fritzbox (Router/DECT-Phone combination) 
// when the 'Ring Video Doorbell Wired' button is pressed 
// (works in cooperation with Ring- and Sinric Pro-Alexa Skills and Alexa Routines).
// Additionally switches Fritz!Dect 200 power socket and monitors energy consumption/generation on the socket.
//
// What is 
// The 'Ring Video Doorbell Wired' is a doorbell consisting of ring-button, 
// video camera with motion detector, microphone and speaker and a WiFi module 
// to transfere videos to a cloud server of the producer. 
// There are Apps for iOS and Android phones which connect to the 
// cloud server and produce a 'ring-sound' when the button is pressed, 
// can show a video taken from the doorbell camera and provide a talk-back 
// functionality to talk with the visitor at the door remotely
// 
// What is Sinric Pro?
// Sinric Pro is an internet based ecosystem mainly consisting of a cloud 
// service, Phone Apps and SDKs to run connected applications on several
// microcontrollers and mini computers (like Esp32, Arduino, Raspberry Pi)
// Smart home systems like Alexa or Google Home can be integrated.
//
// What is FritzBox and Fritz!Dect 200 switchable power socket?
// Fritz!Dect 200 is a switchable power socket, which can be switched
// remotely through radio transmission of the DECT telefone system 
// integrated in FritzBox devices. FritzBox devices, produced by the
// german company AVM, are mostly combinations of Internet Router (WLAN and
// Ehternet) with a DECT telefone system. 'FritzBox'es are very popular in
// Germany.
// 
// 
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
//
// This example is made to work with one power socket
// If more than one power socket shall be used
// make changes in the code where you find the comment:
// 'Change_here_for_more_power_sockets'

#include <Arduino.h>
#include "defines.h"
#include "config_secret.h"
#include "config.h"
#include "SinricPro_Generic.h"
#include "SinricProSwitch.h"
#include "SinricProPowerSensor.h"

#include "FreeRTOS.h"
#include "Esp.h"
#include "esp_task_wdt.h"
#include <rom/rtc.h>

#include "HTTPClient.h"
#include "WiFiClientSecure.h"

#include "WiFiClient.h"
#include "RsHttpFritzApi.h"
#include "RsPowerMeasureMgr.h"

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

// if a queue should be used to store commands (not used in this App) see:
//https://techtutorialsx.com/2017/08/20/esp32-arduino-freertos-queues/

// Represents the On/Off states of switchable sockets
// Change_here_for_more_power_sockets
bool powerState1 = false;
bool powerState2 = false;

// Class to accomplish reading of power consumption of sockets (here 2 sockets)
// Change_here_for_more_power_sockets
RsPowerMeasureMgr PowerMeasureMgr_1(POWERSENSOR_ID_1, FRITZ_DEVICE_AIN_01, SWITCH_READ_REPEAT_COUNT, POWER_MEASURE_READINTERVAL_MS);
//RsPowerMeasureMgr PowerMeasureMgr_2(POWERSENSOR_ID_2, FRITZ_DEVICE_AIN_02, SWITCH_READ_REPEAT_COUNT, POWER_MEASURE_READINTERVAL_MS);

typedef struct
{
   bool actState = false;
   bool lastState = true;
}ButtonState;

int debug_level = SELECTED_DEBUG_LEVEL; ///< Available levels are `DEBUG_NONE`, `DEBUG_ERROR`, `DEBUG_WARNING`, `DEBUG_INFO`, and `DEBUG_VERBOSE`.

//RoSchmi
#define GPIOPin_16 16    // only used for debugging
#define GPIOPin 0
#define BUTTON_1 GPIOPin
#define BUTTON_2 GPIOPin

#define FlashButton GPIOPin

bool GPIO_16_State = false;

bool buttonPressed = false;

volatile ButtonState flashButtonState;

#define LED_BUILTIN 2

int LED1_PIN = LED_BUILTIN;

typedef struct 
{      
  // struct for the std::map below
  int relayPIN;
  int flipSwitchPIN;
  bool hasPowerMeasure;
  int index;
} deviceConfig_t;

// Sinric Pro
// this is the main configuration of switches
// please put in your deviceId, the PIN for Relay the PIN for flipSwitch, a flag if powerMeasure available
// and an index to address the entry
// this can be up to N devices...depending on available power sockets and pins on the Esp32)
// right now we have 2 devicesIds going to 1 LED and 2 flip switches (set to the same button)
// set the LED to -1 for none

// Change_here_for_more_power_sockets
std::map<String, deviceConfig_t> devices =
{
  //{deviceId, {relayPIN,  flipSwitchPIN, measure flag, index}}
  // You have to set the pins correctly.
  // In this App we used -1 when the relay pin shall be ignored 

  { SWITCH_ID_1, {  (int)LED1_PIN,  (int)BUTTON_1, true, 0}},
  { SWITCH_ID_2, {  (int)-1, (int)BUTTON_2, false, 1}}
};

uint32_t millisAtLastFritzConnectTest;
uint32_t millisBetwFritzConnectTests = 10000;

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

// forward declaration
bool sendThisPowerSensorData(RsPowerMeasureMgr &powerMeasureMgr);

bool sendPowerSensorData() {
  bool returnResult = false;
  // Change_here_for_more_power_sockets
  returnResult = returnResult || sendThisPowerSensorData(PowerMeasureMgr_1);
  //returnResult = returnResult || sendThisPowerSensorData(PowerMeasureMgr_2);
  return returnResult;
}

// forward declarations
void deb_print(String message, int level);
void deb_println(String message, int level);
void print_reset_reason(RESET_REASON reason);
bool onPowerState(String deviceId, bool &state);
bool onPowerState1(const String &deviceId, bool state);
bool onPowerState2(const String &deviceId, bool state);
bool onPowerState3(const String &deviceId, bool state);
void setupSinricPro(bool restoreStates);
void handleButtonPress();

void setup() 
{
  Serial.begin(BAUD_RATE);
  //while(!Serial);

  pinMode(FlashButton, INPUT_PULLUP);
  //attachInterrupt(FlashButton, GPIOPinISR, FALLING);  // not used

  pinMode(LED1_PIN, OUTPUT);
  pinMode(GPIOPin_16, OUTPUT);

  // Wait some time (3000 ms)
  uint32_t start = millis();
  while ((millis() - start) < 3000)
  {
    delay(10);
  }

  deb_println(F("\r\nStarting"), DEBUG_NONE);

  //RoSchmi
  if (debug_level >= DEBUG_INFO)
  {
    resetReason_0 = rtc_get_reset_reason(0);
    resetReason_1 = rtc_get_reset_reason(1);
    lastResetCause = resetReason_1;
    Serial.printf("Last Reset Reason: CPU_0 = %u, CPU_1 = %u\r\n", resetReason_0, resetReason_1);
    Serial.print("Reason CPU_0: ");
    print_reset_reason(resetReason_0);
    Serial.print("Reason CPU_1: ");
    print_reset_reason(resetReason_1);
  }

  delay(3000);
  deb_print(F("\nStarting ConnectWPA on "), DEBUG_NONE); 
  deb_print(BOARD_NAME, DEBUG_NONE); 
  deb_print(F(" with "), DEBUG_NONE);
  deb_println(SHIELD_TYPE, DEBUG_NONE);
  
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
    deb_println(F("Watchdog enabled with interval of 20 sec"), DEBUG_NONE);
  }
  else
  {
    deb_println(F("Failed to enable watchdog"), DEBUG_ERROR);
  }
  esp_task_wdt_add(NULL);

  //https://www.az-delivery.de/blogs/azdelivery-blog-fur-arduino-und-raspberry-pi/watchdog-und-heartbeat

  //https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/wdts.html
  #endif 

  //Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  deb_println(F("First disconnecting, then\r\nConnecting to WiFi-Network"), DEBUG_INFO);
  
  while (WiFi.status() != WL_DISCONNECTED)
  {
    WiFi.disconnect();
    delay(200); 
  }
  WiFi.begin(ssid, password);

  if (!WiFi.enableSTA(true))
  { 
    deb_println(F("Connect failed. Rebooting..."), DEBUG_ERROR);   
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
    deb_print((tryConnectCtr++ % 40 == 0) ? "\r\n" : ".", DEBUG_NONE);
  }
  deb_print(F("\r\nGot Ip-Address: "), DEBUG_NONE);
  if (debug_level >= DEBUG_INFO)
  {
    Serial.print(WiFi.localIP()); 
  }
  deb_print("\r\n", DEBUG_NONE);
  
  if (fritz.init())
  {
    deb_println(F("Initialization for FritzBox succeeded"), DEBUG_NONE); 
  }
  else
  {
    deb_println(F("Initialization for FritzBox failed, rebooting"), DEBUG_NONE);
    delay(10 * 1000); // 10 seconds
    ESP.restart();  
  }
  fritz_SID = fritz.testSID();
  //If wanted, printout SID
  if (debug_level >= DEBUG_INFO)
  {
      Serial.printf("Actual SID is: %s\r\n", fritz_SID.c_str());
  }

  // if true: when the device connects to the server the sever will
  // send the last states of the switch to the device
  // if false: the device will actualize the server to the state just beeing present on the device
  bool restoreStatesFromServer = false;
  setupSinricPro(restoreStatesFromServer);

  delay(1000);

  if (!restoreStatesFromServer)
  {
    // Change_here_for_more_power_sockets 
    bool actualSocketState = fritz.getSwitchState(FRITZ_DEVICE_AIN_01);
    
    onPowerState1(SWITCH_ID_1, actualSocketState);  // once more switch Fritz!Dect socket to actual state  

    // get Switch device back
    SinricProSwitch& mySwitch = SinricPro[SWITCH_ID_1];
    // send powerstate event      
    mySwitch.sendPowerStateEvent(actualSocketState); // send the actual powerState to SinricPro server
    deb_println(F("PowerState of Fritz!Dect transmitted to server"), DEBUG_INFO);
  }
  
  // Set time interval for repeating commands
  millisAtLastFritzConnectTest = millis();
  millisBetwFritzConnectTests = 60 * 1000;  //to refresh fritz_SID every minute
  deb_println(F("Setup completed, application is working..."), DEBUG_NONE);
}

void loop() 
{ 
  if (++loopCounter % 100000 == 0)   // Reset watchdog every 100000 th round
  {
         
    #if WORK_WITH_WATCHDOG == 1
      deb_println(F("Reset WDG"), DEBUG_INFO); 
      esp_task_wdt_reset();
    #endif
  }
  if ((millis() - millisAtLastFritzConnectTest) > millisBetwFritzConnectTests) // time interval expired?
  {
     deb_println(F("Testing SID"), DEBUG_INFO);     
     millisAtLastFritzConnectTest = millis();
     if (fritz_SID != fritz.testSID())
     {
        if (fritz.init())
        {
          fritz_SID = fritz.testSID();
          deb_println(F("Re-init for FritzBox succeeded"), DEBUG_INFO);          
        }
        else
        {
          deb_println(F("Re-init for FritzBox failed, rebooting"), DEBUG_ERROR);         
          delay(10 * 1000); // 10 seconds
          ESP.restart();  
        }
     }     
  }
  SinricPro.handle();
  sendPowerSensorData();
  handleButtonPress();
}

// not used
void GPIOPinISR()
{
  buttonPressed = true;
}

// this is where you read from power sensor
bool doPowerMeasure(RsPowerMeasureMgr &actPowerMeasure) { 
  if (actPowerMeasure.isActive())
  {
    actPowerMeasure.SetPower((float)fritz.getSwitchPower(actPowerMeasure.GetFritzDevice_AIN()));
  }
  else
  {
    actPowerMeasure.SetPower(nanf(""));
  }
  return true;
}

bool sendThisPowerSensorData(RsPowerMeasureMgr &powerMeasureMgr) 
{ 
  bool powerMeasureStateToggled = false;
  if (!powerMeasureMgr.isSendForced())  // send if forced in any case
  {
    // dont send data if device is turned off or if powerMeasure is off
    if (!(powerMeasureMgr.isActive() && powerMeasureMgr.GetPowerMeasureState()))
    {
      return false; 
    }
    // only send if sendinterval has elapsed
    if ((millis() - powerMeasureMgr.GetLastSendTimeMs()) < powerMeasureMgr.GetSendIntervalMs()) 
    {
      return false; 
    }
  }
  // Will be sent to Sinric Pro server
  powerMeasureMgr.SetSendForced(false);
  if (powerMeasureMgr.isAutoRepeatEnabled())
  {    
    powerMeasureMgr.DecrementAutoRepeatCounter(1000);
    char outStr[50] {0};
    snprintf(outStr,sizeof(outStr), "RepeatCounter: %d", powerMeasureMgr.GetAutoRepeatCounter());
    deb_println(outStr, DEBUG_INFO);
    
    if (powerMeasureMgr.GetAutoRepeatCounter() <= 0)
    {
      powerMeasureMgr.SetPowerMeasureState(false);
      powerMeasureStateToggled = true;
      powerMeasureMgr.SetAutoRepeatCounter(SWITCH_READ_REPEAT_COUNT);
      deb_println(F("RepeatCounter was 0, stopping to repeat"), DEBUG_INFO);          
    }
  }
  else
  {
    powerMeasureMgr.SetSendForced(false);
    powerMeasureMgr.SetAutoRepeatCounter(SWITCH_READ_REPEAT_COUNT);
  }
  
  powerMeasureMgr.SetLastSendTimeMs(millis());
  
  // send measured data
  SinricProPowerSensor &myPowerSensor = SinricPro[powerMeasureMgr.GetPowerSensor_ID()];
  powerMeasure myPowerMeasure = powerMeasureMgr.GetPowerValues();
 
  deb_println(F("sendPowerSensorEvent"), DEBUG_INFO);          
  
  // RoSchmi: toggle GPIO_16 (for debugging)
  GPIO_16_State = !GPIO_16_State;
  digitalWrite(GPIOPin_16, GPIO_16_State);
  
  char outStr[50] {0};
  snprintf(outStr, sizeof(outStr), "Measured Power: %f", myPowerMeasure.power);
  deb_println(outStr, DEBUG_VERBOSE);
  
  if (powerMeasureStateToggled)
  {
    myPowerSensor.sendPowerStateEvent(false);
  }
  
  bool success = myPowerSensor.sendPowerSensorEvent(myPowerMeasure.voltage, myPowerMeasure.current, myPowerMeasure.power);
  if (success) { 
    doPowerMeasure(powerMeasureMgr); }
  return success;
}

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
        deb_println(F("(Switched manually via flashbutton)"), DEBUG_INFO);             
      }    
    }     
  }
  else
  {
    flashButtonState.actState = false;
  }
}

bool onPowerState(String deviceId, bool &state)
{
  bool returnResult = false;
  char outStr[50] {0};
  snprintf(outStr, sizeof(outStr), "Received: %s State: %s", deviceId.c_str(), state ? " on" : " off");
  deb_println(outStr, DEBUG_VERBOSE);
  
  if (deviceId == POWERSENSOR_ID_1)
  {
    PowerMeasureMgr_1.SetPowerMeasureState(state);
    if (state)
    {
      PowerMeasureMgr_1.SetAutoRepeatCounter(MEASURE_READ_REPEAT_COUNT);
    }
    PowerMeasureMgr_1.SetSendForced(true);
    returnResult = doPowerMeasure(PowerMeasureMgr_1);
  }
  else
  {
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
  }
  return returnResult;
}

bool onPowerState1(const String &deviceId, bool state)
{
  bool switchResult = false;
  
  if (state == true)
  {
    switchResult = fritz.setSwitchOn(FRITZ_DEVICE_AIN_01);
  }
  else
  {
    switchResult = !fritz.setSwitchOff(FRITZ_DEVICE_AIN_01);
  }
  if (switchResult == true)  // Set LED to on or off
  {
    int relayPIN = devices[deviceId].relayPIN; // get the relay pin for corresponding device
    if (relayPIN != -1)
    {
      digitalWrite(relayPIN, state);      // set the new relay state
    }
    char outStr[50] {0};
    snprintf(outStr, sizeof(outStr), "Device 1 turned %s\r\n", state ? "on" : "off");
    deb_println(outStr, DEBUG_INFO);
    
    powerState1 = state;
    flashButtonState.actState = state;    
  }
  else
  {
    char outStr[50] {0};
    snprintf(outStr, sizeof(outStr), "Failed to turn Device 1 %s\r\n", state ? "on" : "off");
    deb_println(outStr, DEBUG_ERROR);
    
  }
  bool readPowerResult = true;
  
  if (devices[deviceId].hasPowerMeasure)
  { 
     readPowerResult = doPowerMeasure(PowerMeasureMgr_1);
     PowerMeasureMgr_1.SetSendForced(true);
     PowerMeasureMgr_1.SetPowerMeasureState(true);
     PowerMeasureMgr_1.SetAutoRepeatCounter(SWITCH_READ_REPEAT_COUNT);
  }
     
  return switchResult && readPowerResult; // request handled properly
}

bool onPowerState2(const String &deviceId, bool state)
{
  char outStr[50] {0};
  snprintf(outStr, sizeof(outStr), "Device 2 turned %s\r\n", state ? "on" : "off");
  deb_println(outStr, DEBUG_INFO); 
  powerState2 = state;
  
  bool atLeastOneSuccess = false;
  if (state == true)
  {
    deb_println(F("\r\nStarting Ring Test\r\n"), DEBUG_INFO);     
    if (RingPhone_1 == 1)
    {
      if(fritz.startRingTest(1))
      {
        atLeastOneSuccess = true;
      }
    }
    if (RingPhone_2 == 1)
    {
      if(fritz.startRingTest(2))
      {
        atLeastOneSuccess = true;
      }
    }
    if (RingPhone_3 == 1)
    {
      if(fritz.startRingTest(3))
      {
        atLeastOneSuccess = true;
      }
    } 
    if (atLeastOneSuccess)
    {
      deb_println(F("Successfully started Ringtest"), DEBUG_INFO);
    }
    else
    {
      deb_println(F("Failed to start Ringtest"), DEBUG_ERROR);  
    }    
  }
  else
  {
    if (RingPhone_1 == 1)
    {
      if(fritz.stopRingTest(1))
      {
        atLeastOneSuccess = true;
      }
    }
    if (RingPhone_2 == 1)
    {
      if(fritz.stopRingTest(2))
      {
        atLeastOneSuccess = true;
      }
    }
    if (RingPhone_3 == 1)
    {
      if(fritz.stopRingTest(3))
      {
        atLeastOneSuccess = true;
      }
    } 
    if (atLeastOneSuccess)
    {
      deb_println(F("Successfully stopped Ringtest"), DEBUG_INFO);    
    }
    else
    {
      deb_println(F("Failed to stop Ringtest"), DEBUG_ERROR);      
    }  
  }
  return atLeastOneSuccess; // request handled properly ?

}


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
  // Change_here_for_more_power_sockets
  SinricProPowerSensor &myPowerSensor = SinricPro[POWERSENSOR_ID_1];

  // set callback function to device
  myPowerSensor.onPowerState(onPowerState);

  SinricPro.restoreDeviceStates(restoreStates);
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET); 
  // if true, restore the last states from the Sinric Server to this local device
  
}

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

/**************************************************************************/
/*!
    @brief  Debug-print. Only prints the message if the debug level is high enough.
    @param    message
                The message to be conditionally printed.
    @param    level
                The minimally required debug level.
*/
/**************************************************************************/
void deb_print(String message, int level) {
    if (Serial) {
        if (debug_level >= level) {
            Serial.print(message);
            //Serial.flush();
        }
    }
}

/**************************************************************************/
/*!
    @brief  Same as deb_print, but with a new line at the end.
            Only prints the message if the debug level is high enough.
    @param    message
                The message to be conditionally printed.
    @param    level
                The minimally required debug level.
*/
/**************************************************************************/
void deb_println(String message, int level) {
    if (Serial) {
        if (debug_level >= level) {
            Serial.println(message);
            //Serial.flush();
        }
    }
}




