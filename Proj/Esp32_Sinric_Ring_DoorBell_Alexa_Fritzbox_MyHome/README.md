# Esp32_Sinric_Ring_Doorbell_Alexa_Fritzbox_MyHome

This is a special application with different functions.

1) Controlling and monitoring of a switchable Fritz!Dect 200 power socket ( for FritzBox ) via Sinric Pro
2) Ringing Dect phones of a Fritzbox (Router/DECT-Phone combination) when a Ring Video Doorbell Wired is pressed (works together with a cooperation of Ring- and Sinric Pro-Alexa Skills and Alexa Routines)

#### To function 1:

This is a quite complex application which can be used to switch a power socket and read the power consumption or the power production on the socket when the socket is e.g. connected to the micro converter of a solar panel. The code of this example is good to control one power socket. To use more than one power socket, the code has to be changed at different places (explanations are given in the file main.cpp)

A simple and easier to understand application which can only switch the Fritz!Dect 200 power socket is provided here:

https://github.com/RoSchmi/Esp32_SinricPro_FritzDect_Controller

What is Sinric Pro?
Sinric Pro is an internet based ecosystem mainly consisting of a of an internet portal, a cloud service, mobile Apps and SDKs to run connected applications on different microcontrollers and mini computers (like Esp32, Arduino, Raspberry Pi) Smart home systems like Alexa or Google Home can be integrated.

What is FritzBox and Fritz!Dect 200 switchable power socket?
Fritz!Dect 200 is a switchable power socket, which can be switched remotely through radio transmission of the DECT telefone system integrated in FritzBox devices. FritzBox devices, produced by the german company AVM, are mostly combinations of Internet Router (WLAN and Ehternet) with a DECT telefone system. 'FritzBox'es are very popular in Germany.

This App accomplishes a way to switch the power socket remotely over the internet from a Sinric Pro Phone App via the Sinric Pro cloud service and the FritzBox (Router/DECT-Phone combination). The power consumption at the socket can be measured remotely as well.

#### To function 2:

The 'Ring Video Doorbell Wired' is a doorbell consisting of ring-button, video camera with motion detector, microphone and speaker and a WiFi module to transfere videos to a cloud server of the producer. There are Apps for iOS and Android phones which connect to the cloud server and produce a 'ring-sound' when the button is pressed, can show a video taken from the doorbell camera and provide a talk-back functionality to talk with the visitor at the door remotely.

https://ring.com/products/video-doorbell-wired

Before you can start:
Define WiFi-Credentials, FritzBox-Credentials and Sinric Pro Credentials in the file include/config_secrete.h (take 'config_secret_template.h' as a template)

Define 'TRANSPORT_PROTOCOL' (http or https) in file config.h When you begin to work with this App set TRANSPORT_PROTOCOL = 0 (http) The https secured connection will not work before you include the specific certificate of your personal FritzBox in include/config.h. Instructions how to get the certificate are given in the file 'config.h' When you have included the correct certificate, set TRANSPORT_PROTOCOL = 1 More configurations can be made in config.h (details are explained in config.h).
When you start using the application set 'WORK_WITH_WATCHDOG' to 0. When everthing is working, set 'WORK_WITH_WATCHDOG' to 1.

The FRITZ_DEVICE_AIN can be found on your Fritz!Dect 200 powersocket To get the Sinric Pro Credentials have a look at:

https://sinric.pro/de-index.html

https://sinricpro.github.io/esp8266-esp32-sdk/index.html

https://github.com/sinricpro

#### Getting hands on:
In the Fritzbox Web UI in 'System' -> FRITZ!Box-Benutzer' create a user and give rights to 'FRITZ!Box Einstellungen'.

In tab 'Anmeldung im Heimnetz' select 'Anmeldung mit FRITZ!Box-Benutznamen und Kennwort' Note the user name and the password, they must be entered in the App 'Esp32_RingDoorBell_Alexa_Sinric_FritzPhone'

Create a Sinric Pro Account and create a 'Switch' device with e.g. the following settings:

Name: FritzPhoneRing

Timer: Auto Aus (7sec)

Note the Device_ID, the APP_Key and the App_Secret, they must be entered in the App 'Esp32_RingDoorBell_Alexa_Sinric_FritzPhone'

Open the application 'Esp32_RingDoorBell_Alexa_Sinric_FritzPhone' in PlatformIO, apply the needed settings and passwords in the files include/config_secret.h (use config_secret_template.h as a template) and in include/config.h. Build the application and upload it to the Esp32 board. In the serial monitor you should see if the application runs properly.

In the Sinric Pro Portal select 'Dashboard' on the left menue bar and switch the device (e.g.) FritzPhoneRing to 'On'. When everthing worked fine, the selected Dect-Phones should ring for a short time.

Download the Sinric Pro mobile App from App Store or Google Play and test if everything works from the smartphone App as well.

In the Alexa App on your smartphone on the menue bar select the right icon (three lines), then select 'Skills und Spiele'. Search for 'Sinric Pro' and add the Sinric Pro skill following the given advices.

When everthing worked fine, you should now have a new switch (socket) device in the Alexa App. Switch it to 'On' in the Alexa App to seeif the Dect-phone ring.

Now we come to the 'Ring Video Doorbell' part. To continue, the doorbell should be installed and setted up properly and should work properly with the 'Ring' smartphone App.

Now in the Alexa App on your smartphone you should add the 'Ring Video Doorbell' Alexa skill and create an Alexa routine. In this routine you can add an action: set (e.g.) 'FritzPhoneRing' to 'On'

Now hopefully the DECT-phones should ring when you press the doorbell.

I hope that I didn't forget any important steps, if I did, please let me know.
