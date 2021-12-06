# Esp32_FritzBox_Ring_Alarm

Quick and dirty example: Let Fritzbox telefone ring controlled by Esp32 program

### What is FritzBox 
FritzBox devices, produced by the german company AVM, are mostly combinations of Internet Router (WLAN and Ethernet) with a DECT telefone system. 'FritzBox'es are very popular in Germany.


### Before you can start:
Define WiFi-Credentials and FritzBox-Credentials in the file include/config_secrete.h (take 'config_secret_template.h' as a template)

Define 'TRANSPORT_PROTOCOL' (http or https) in file config.h When you begin to work with this App set TRANSPORT_PROTOCOL = 0 (http) The https secured connection will not work before you include the specific certificate of your personal FritzBox in include/config.h. Instructions how to get the certificate are given in the file 'config.h' When you have included the correct certificate, set TRANSPORT_PROTOCOL = 1 More configurations can be made in config.h (details are explained in config.h).








