#ifndef _CONFIG_SECRET_H
#define _CONFIG_SECRET_H

// Rename config_secret_template.h into config_secret.h to activate the content

// Wifi
#define IOT_CONFIG_WIFI_SSID            "MySSID"
#define IOT_CONFIG_WIFI_PASSWORD        "MyPassword"

// FritzBox
#define FRITZ_IP_ADDRESS "fritz.box"    // IP Address of FritzBox
                                        // Change for your needs
#define FRITZ_USER ""                   // FritzBox User (may be empty)
#define FRITZ_PASSWORD "MySecretName"   // FritzBox Password

#define FRITZ_DEVICE_AIN_01 "111122223333" // AIN = Actor ID Numberof Fritz!Dect (12 digit)
#define FRITZ_DEVICE_AIN_02 ""             
#define FRITZ_DEVICE_AIN_03 ""
#define FRITZ_DEVICE_AIN_04 ""

// Sinric Pro
#define APP_KEY           "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define SWITCH_ID_1       "5dc1564130xxxxxxxxxxxxxx"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define SWITCH_ID_2       "5dc1564130xxxxxxxxxxxxxx"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
//#define SWITCH_ID_3     ""                            // Should look like "5dc1564130xxxxxxxxxxxxxx"
//#define SWITCH_ID_4     ""                            // Should look like "5dc1564130xxxxxxxxxxxxxx"    

#endif // _CONFIG_SECRET_H