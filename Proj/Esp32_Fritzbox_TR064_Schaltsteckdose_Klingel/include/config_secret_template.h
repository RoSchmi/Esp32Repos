#ifndef _CONFIG_SECRET_H
#define _CONFIG_SECRET_H

// Rename config_secret_template.h into config_secret.h to activate the content

// Wifi
#define IOT_CONFIG_WIFI_SSID            "MySSID"
#define IOT_CONFIG_WIFI_PASSWORD        "MyPassword"

// FritzBox
#define FRITZ_IP_ADDRESS "fritz.box"       // URL of FritzBox (needed for https)
//#define FRITZ_IP_ADDRESS "192.168.1.1"   // (example) IP Address of FritzBox 
//#define FRITZ_IP_ADDRESS "192.168.179.1" // IP Address of FritzBox (not for https transmission)
                                           // Change for your needs
#define FRITZ_USER "thisUser"              // FritzBox User (may be empty)
#define FRITZ_PASSWORD "MySecretName"      // FritzBox Password

#define FRITZ_DEVICE_AIN_01 "12345 0123456" // AIN = Actor ID Numberof Fritz!Dect (12 digit)
#define FRITZ_DEVICE_AIN_02 ""              // must be entered exactly in this format
#define FRITZ_DEVICE_AIN_03 ""
#define FRITZ_DEVICE_AIN_04 ""

#endif // _CONFIG_SECRET_H