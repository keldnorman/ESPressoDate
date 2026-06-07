# ESPressoDate
A captive portal on an ESP32-C6 that turns a "Truth or Dare" Wi-Fi network
into an invitation for a date.

This code was written for a Seeed Studio XIAO ESP32-C6

The code is designed to make coding fun by giving learners a clear purpose where hardware and software work together in a practical technology project.

## Hardware: 

ESP32 Seeed Studio XIAO **ESP32C6** WiFi 6+Bluetooth-compatible Ble 5 Support Zigbee Matter Wireless Development Board

I bought it here: https://www.aliexpress.com/item/1005006946443492.html
<p align="left">
<img src="https://github.com/keldnorman/ESPressoDate/blob/main/images/xiao-esp32-c6.png" width="200" alt="esp32c6"/>
</p>

There is a guide here: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/ 

## How to install/build the code: 

To compile it just start arduino IDE ( i use version 2.3.9 ) then go to:

*File -> Preferences -> Additional Boards Manager URL*

And add this: https://espressif.github.io/arduino-esp32/package_esp32_index.json

Then select the menu Tools -> Boards -> Boards managers ( or just press CTRL + SHIFT + B if it works ? )

Search for "esp32 by Espressif Systems" and install it. 

<sub>It was in version 3.3.10 when i wrote this code<sub>

## Settings

Change the contact info, the secret admin code, and the SSID by altering the .ino file 

You will find the following variables related to WiFi:

```
#define AP_SSID "Truth or Dare? 😏"
#define AP_CHANNEL 6
#define AP_MAX_CLIENTS 4
#define ACTIVE_CLIENT_LIMIT 2
```

Contact info: 
```
static const char date_name[] = "Your Name";
static const char date_phone[] = "+xx xxxxxxxx";
static const char date_instagram[] = "@your.instagram";
static const char date_mail[] = "your@email.com";
```
The code needed to access the saved answers:
```
#define ADMIN_KEY "1337"
```

And a setting to activate the external antenna if you choose to add one:
```
static const bool external_antenna = true;
```

## 3D-Print a case

If you want to 3D-print a nice case for the esp32 then have a look at this: 

https://makerworld.com/en/models/1723550-seeed-studio-xiao-esp32-c6-snap-fit-wall-mount#profileId-1829704

<img src="https://github.com/keldnorman/ESPressoDate/blob/main/images/case1.jpg" width="200">  <img src="https://github.com/keldnorman/ESPressoDate/blob/main/images/case2.jpg" width="400">

## How it looks

The wifi name:

<img src="https://github.com/keldnorman/ESPressoDate/blob/main/images/connect-to-wifi.png" width="300"> 

<style="text/css">
.top_aligned_image {vertical-align: top; /* or text-top, I can't remember for sure which works better */}
</style>

<img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare1.jpg" width="300"><img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare2.jpg" width="300"><img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare3.jpg" width="300">
<img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare4.jpg" width="300"><img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare5.jpg" width="300"><img src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare6.jpg" width="300">
<img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare7.jpg" width="300"><img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare8.jpg" width="300"><img align="top" src="https://github.com/keldnorman/ESPressoDate/blob/main/images/dare9.jpg" width="300">
## Buy me a cup of coffee ?

If you like this project and wants to give me a cup of coffee:
https://buymeacoffee.com/keldnormanh


(C)opyleft Keld Norman, 2025
