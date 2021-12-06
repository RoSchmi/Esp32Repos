// Copyright RoSchmi 2021, License MIT
// https://github.com/RoSchmi/Teensy/tree/master/Proj/Teens41_FritzBox_FritzDect
//
// This is an adaption/modification of:
// https://github.com/planetk/ArduinoFritzApi 

// For the MD5 hash calculation the used way was taken from
// https://github.com/schuppeste/Arduino-Fritzbox-Login

#ifndef RS_HTTP_FRITZ_API_H
#define RS_HTTP_FRITZ_API_H

#include <Arduino.h>
#include <MD5.h>

#define FRITZ_ERR_HTTP_COMMUNICATION  -1001
#define FRITZ_ERR_NO_CHALLENGE        -1002
#define FRITZ_ERR_NO_SID              -1003
#define FRITZ_ERR_EMPTY_SID           -1004
#define FRITZ_ERR_INVALID_SID         -1005
#define FRITZ_ERR_VALUE_NOT_AVAILABLE -1006

#define FRITZ_ON_TEMPERATURE          100.0f
#define FRITZ_OFF_TEMPERATURE         0.0f

typedef enum {
      useHttp,
      useHttps
  } Protocol;

  typedef const char* X509Certificate;

class FritzApi {
  public:
    
    //Constructor
    FritzApi(const char* user, const char* password, const char* ip, Protocol protocol, WiFiClient client, HTTPClient * httpClient, X509Certificate pCertificate);
   
    ~FritzApi();

    bool init();
    
    // Test if actually used SID is valid, returns actual SID 
    String testSID();	
	// Switch actor on, return new switch state (true=on, false=off)
    boolean setSwitchOn(String ain);
	// Switch actor off, return new switch state (true=on, false=off)
    boolean setSwitchOff(String ain);
	// Toggle switch state, return new switch state (true=on, false=off)
    boolean setSwitchToggle(String ain);
	// Retrieve current switch state (true=on, false=off)
    boolean getSwitchState(String ain);
	// Check for presence of a switch with the given ain (true=present, false=offline)
    boolean getSwitchPresent(String ain);
	// Get current power consumption in Watt
	double getSwitchPower(String ain);
	// Get total energy since last reset in Wh
	int getSwitchEnergy(String ain);
	// Get temperature in °C
    double getTemperature(String ain);
	// Get name of an actor
	String getSwitchName(String ain);
    // Start ringing on a phone (Alarm)
    bool startRingTest(int phoneNo);
    // Stop ringing on a phone 
    bool stopRingTest(int phoneNo);

    // The Thermostat functions are actually not implemented
    /*
	// Get nominal temperature of thermostat (8 = <= 8°C, 28 = >= 28°C, 100 = max, 0 = off)
	double getThermostatNominalTemperature(String ain);
	// Get comfort temperature of thermostat (8 = <= 8°C, 28 = >= 28°C, 100 = max, 0 = off)
	double getThermostatComfortTemperature(String ain);
	// Get reduced temperature of thermostat (8 = <= 8°C, 28 = >= 28°C, 100 = max, 0 = off)
	double getThermostatReducedTemperature(String ain);
	// Set nominal temperature of thermostat (8 = <= 8°C, 28 = >= 28°C, 100 = max, 0 = off)
	double setThermostatNominalTemperature(String ain, double temp);
    */

  private:
    Protocol _protocol;
    int _port;
    const char* _user;
    const char* _pwd;
    const char* _ip;
    String _sid;

    X509Certificate _certificate;
    
    const char * fon_devices_EditDectRingToneService = "/fon_devices/edit_dect_ring_tone.lua?";
    const char * homeautoswitchService = "/webservices/homeautoswitch.lua?";
    const char * login_sidService = "/login_sid.lua?";

    byte mynewbytes[100];
    
    HTTPClient * instHttp;

    WiFiClient client;

    String getChallengeResponse();
    String getSID(String response);
    String executeRequest(String service, String request);

    // not implemented
	//double convertTemperature(String result); 
};

#include "RsHttpFritzApi_Impl.h"

#endif // RS_HTTP_FRITZ_API_H