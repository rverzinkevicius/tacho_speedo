# tacho_speedo
ESP8266 based tachometer and speedometer

started in http://modernvespa.com/forum/post2384534#2384534




![img](https://i.postimg.cc/6tdJM6Yx/IMG-20200206-070351.jpg)

It has ressetable trip function, shows rpm, exact speed and has two colour schemes: one like in picture and another high contrast black on white for sunny days. It hooks up to pickup coil via optocoupler for rpm and i have added hall sensor into original speedo for speed. Based on esp8266 chip and 2.2 inch TFT display.

PARTLIST

Main processing unit is ESP8266 in form of "D1 mini" with ProtoBoard shield. Both can be found at ebay for cheap

![img](https://www.picclickimg.com/d/l400/pict/163566262128_/Mini-ESP8266-ESP-12-WeMos-D1-NodeMcu-WIFI-Development.jpg)

Screen is also from ebay, 2.2 inch tft ili9341 320x240, there are several variations of them, mine has 3.3v logic level (no logic level shifter chip). As i use 3.3v to power it, J1 should be shorted by solder.
![img](https://hackster.imgix.net/uploads/image/file/60743/LCD22.jpg)

Step down converter 12V to 5V.
![img](https://ram-e-shop.com/wp-content/uploads/2018/09/dc_dc_301.jpg)

4N35 optocoupler to get RPM from pickup coil.

A3144 hall sensor to get speed. Placed inside original speedometer near spinning magnetic disk.
![img](https://www.howacarworks.com/illustration/1655/inside-a-mechanical-speedometer.png)

1M, 10k, 4.7k resistors

2x 1N4007 diodes. I think one would do the job, but i'm too lazy to remove it and do tests.

25V 10000uF electrolytic capacitor or several in parallel, i have used 6 capacitors 2200uF each. This will provide enough power for D1 mini to save odometer and trip to memory after switching ignition off.

Pushbutton. Short press switches between color schemes, long press (over 3s)- resets trip. 
It is possible to use capacitive sensor placed behind plastic trim, so no hole drilling etc.
![img](https://pg-cdn-a2.datacaciques.com/wm/NDAy/4174414326/2681266511.jpg)



Hand drawn schematic

![img](https://i.postimg.cc/B6wL8GG2/IMG-20200212-193810.jpg)


Pins GND, 5V, 3.3V, RST, A0, D1-D7 are on D1 mini board

Screen connection:
Screen -> D1 mini
Vcc to 3.3V
GND to GND
CS to GND
Reset to RST
RS to D3
SCK to D5
SDI to D7
LED to Vcc
SDO not connected

Different screens have different names for pins.

![img](https://i.postimg.cc/52K8YGW5/IMG-20200213-104950-631.png)



main code is in tacho_speedo.ino

You will nedd to extract TFT_eSPI-master-20200213T100157Z-001.zip to Documents\Arduino\libraries on your PC


There are few things to mention. First you will need to change wheel circumference to reflect your wheel size in centimeters  in line 41
int wheel=156;

Next you will need to uncomment (remove //) two lines 83 and 84 in code

//  dispodo=0.1;  
//  allodo=5660.2;

Change allodo to mileage you have on your scooter, upload code to D1 mini, then comment/delete those two lines and again upload code.


To change km to miles, i think it should be enough to multiply your wheel size by 0.621371,  for example line 41
int wheel=156;
should be
int wheel=97;
and change line 328
 tft.drawString("km/h",236,290);
to
 tft.drawString("m/h",236,290);
