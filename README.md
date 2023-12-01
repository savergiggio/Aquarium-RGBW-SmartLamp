<p align="center">
    <img width="30%" src="/welcome.png">
</p>

# Aquarium-RGBW-SmartLamp
An OpenSource project about a RGBW Lamp for Aquariums, with all the main, and more features that an aquarium lamp needs! **So let's go in deep in the project**

# BUT FIRST....
A very big thank you to the company  **JLCPCB** one of my partners that contrubuted to realise the PCBs of the Controller and Led Boards. 
**JLCPCB** is a global PCB manufacturer. It has a cost of $2 for 1-8 layer PCB prototyping; PCB Assembly from $0 ,3D Printing starts at $0.3. 
You can register from this link to GET an instant quote for your projects : https://jlcpcb.com/quote 

<p align="center">
    <img width="30%" src="/sponsor.png" (https://jlcpcb.com/quote )>
</p>

## The Two Main Components:
Controller Board + LED Board

### Overview of the Controller Board:
- ESP32-WROOM-32 as main controller.
- 3 On-Board Temperature sensors:
  - Controller Board Temeprature.
  - Led Board Controller.
  - Aquarium Water Temperature.
- 4 LED-Driver, 1 for each LED Color with PWM Control to adjust the brightness (RED,GREEN,BLUE,WHITE).
- MAX VCC input 26V
- **BLUETOOTH** Connection for controll with Smartphone APP.
- **WIFI** OTA Updates (OPTIONAL).

### Overview of the LED Board:
- 4 LED of 12W each (RED, GREEN, BLUE, WHITE) , 48W Total Power XXX LUMEN
- PCB in Aluminium for Heat Dissipation
- Temperature Built-in Sensor
- FAN for Heat Dissipation with Alluminium HeatSink
  
## Android App to Controll the Lamp
The Smart RGBW Lamp can be controller through his Android APP. These are the main features:
- Adjustble Brightness for each Color in PWM with a 13 bit resolution, up to 8192 steps.
- Set the time to turn ON/OFF the light.
- Adjust the fading for the rising UP/DOWN to turn ON/OFF the lamp.
- Update the firmware via OTA.

## 3D Printed Case or Alluminium Case
The main idea is to make as much as possible the project easy to realize at home , when possible. For this reason, i opted for 2 solutions.
1. Case in Alluminium. I found this alluminium case in a chinese store for 10$.
2. 3D Printed Case. This solution is aesthetically worse respect the alluminium case, but this permit to have a custom case with a more power fan for heat dissipation, permitting to have lower temperatures of the lamp.

# SUPPORT ME
If you would like to support my project, and continue to develop it you can subscribe as **PATREON** at the following link: https://patreon.com/RGBWSmartAquariumLamp or make a donation to my **PAYPAL** account: pasqualetripodi14@gmail.com.
All the materials will be available for free to download, for this reason I only inserted a symbolic price, of 3€, when downloading the app from the Play Store. If you already made a donation or you are a patreon please write to me , i will give you the app for FREE.


# Next Steps
-
-
-
-





