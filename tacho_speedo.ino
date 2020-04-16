#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

unsigned int backroundcolor = TFT_BLACK;
unsigned int speedcolor = TFT_GREEN;
unsigned int rpmcolor = TFT_YELLOW;
unsigned int odocolor = TFT_WHITE;

int button = D6;
int stateb = 0;
long buttonTimer = 0;
long longPressTime = 3000;
boolean statebutton = false;
boolean buttonActive = false;
boolean longPressActive = false;

int eepromsize = 21;
int odoaddr = 13;
int allodoaddr = 17;

int rpmpin = D2;
int rpm = 1;
int rpmprev = 9999;
int rpm_a = 1;
int rpm1 = 1;
int rpm2 = 1;
int rpm3 = 1;
int counter_rpm = 0;
int present_rpm = 0;
int previous_rpm = 0;
float rpmai = 1;
unsigned long duration_rpm = 1;
unsigned long elapsedt_rpm = 1;
unsigned long elapsed_prev_rpm = 1;
unsigned long last_show_rpm=0;
unsigned long cur = 0;
unsigned long del= 0;
unsigned long del2= 0;
bool rpmflag=true;
bool rpmupdated=false;


int speedpin = D1;
int speedprev=99;
volatile byte speedcount=0;
int speedcountTmp=0;
float ratas=1.56; 
float distance=0;
int speed1=0;
float speedo=0;
float odo=0;
float odo2=0;
float odo3=0;
float odo4=0;
float dispodo = 0;
float dispodoprev = 0;
float allodo;
unsigned long last_show_speed=0;
unsigned long last_update_speed=0;
unsigned long duration_speedTmp=1;
volatile unsigned long duration_speed=1;
volatile unsigned long last_speed=0;
bool speedupdated=false;
bool speedupdated2=false;


bool flag50 = false;
unsigned long time50start = 0;
float time50 = 0;
int speedprev50 = 0;


void setup()   {  
 
  WiFi.forceSleepBegin();
  delay(300);


  tft.init();
  tft.setRotation(2);
  tft.setTextDatum(BR_DATUM);

drawStatic();

  pinMode(button, INPUT_PULLUP);
  pinMode(rpmpin,INPUT);
  pinMode(speedpin,INPUT);

  attachInterrupt(digitalPinToInterrupt(speedpin), speed_counter, FALLING);
 
  EEPROM.begin(eepromsize);
  EEPROM.get(odoaddr,odo);  
  EEPROM.get(allodoaddr,allodo); 
  EEPROM.end(); 

  odo2 = odo/1000;
  dispodo = roundf(odo2*10)/10;
  dispodoprev = dispodo;
 //  odo=0.1;  
 //  allodo=5660.2;
 

}

void loop() 
 {

 int sensorValue = analogRead(A0);
 if (sensorValue < 100) 
  {
  EEPROM.begin(eepromsize);
  EEPROM.put(odoaddr,odo);  
  EEPROM.put(allodoaddr,allodo); 
  EEPROM.commit();

  tft.fillScreen(backroundcolor);
  tft.setTextColor(odocolor, backroundcolor); 
  tft.setFreeFont(&Roboto_Mono_Thin_24);
  tft.drawString("ODO",2,32);
  tft.drawString("TRIP",2,62);

  drawOdo();  
  
  ESP.deepSleep(0);      
  }

if (digitalRead(button) == HIGH)
 {
 if (buttonActive == false) 
  {
  buttonActive = true;
  buttonTimer = millis();
  }
  if ((millis() - buttonTimer > longPressTime) && (longPressActive == false))
   {
   longPressActive = true;
   odo=0;
   dispodo=0;
   dispodoprev = 0;
   tft.fillRect(150, 32, 90, 30, backroundcolor);
   }
  } 
  else 
   {
   if (buttonActive == true) 
    {
    if (longPressActive == true) 
     {
     longPressActive = false;
     }
     else
      {
      statebutton = !statebutton;
/*
      stateb +=1;
      if (stateb>3){stateb=0;}
  */    
  
   if (statebutton)
       {
       backroundcolor = TFT_WHITE;
       speedcolor = TFT_BLACK;
       rpmcolor = TFT_BLUE;
       odocolor = TFT_BLACK;
       }

      if (!statebutton)
       {
       backroundcolor = TFT_BLACK;
       speedcolor = TFT_GREEN;
       rpmcolor = TFT_YELLOW;
       odocolor = TFT_WHITE;
       }

      drawStatic();

    }
   buttonActive = false;
   }
 }


 if (digitalRead(rpmpin) == 1 && previous_rpm == 0)
  {
  previous_rpm = 1;
  duration_rpm = elapsedt_rpm - elapsed_prev_rpm;
  rpmupdated=true;
  elapsed_prev_rpm  = millis();    
  }

 if (digitalRead(rpmpin) == 1 && previous_rpm == 1)
  {
  previous_rpm = 1;       
  }

 if (digitalRead(rpmpin) == 0 && previous_rpm == 1)
  {
  cur=millis();
  del=cur - elapsed_prev_rpm;
   if (del < 5) 
    {
    previous_rpm=1;
    }
    else 
     {
     previous_rpm = 0;     
     }
  }

 if (digitalRead(rpmpin) == 0 && previous_rpm == 0)
  {
  previous_rpm = 0;
  elapsedt_rpm = millis(); 
  del2=elapsedt_rpm - elapsed_prev_rpm;
  if (del2>2000)
   {
   rpmflag=true;
   }
  }
 
 

if (rpmupdated){
 rpmai = 60000/duration_rpm;
 rpm = round (rpmai);
 rpmupdated=false;
 
 if ( ((rpm_a-50) < rpm) && (rpm < (rpm_a+50)))
  {
  rpm_a = rpm;
  rpm3 = rpm3 + rpm;
  counter_rpm = counter_rpm + 1;
  }
  else
   {
   rpm_a=rpm;
   }
}




 if ((millis()-last_show_rpm) >800)  //refresh rate
  {
 if (counter_rpm>0){   
  rpm2=rpm3/counter_rpm;
  rpm2=((rpm2+5)/10)*10;  //round to tens
  rpm2 = min(9999, rpm2); 
  counter_rpm=0;
  rpm3=0;
 }
  if (rpmflag)
   {
   rpm2=0;
   rpmflag=false;
   tft.fillRect(0, 80, 181, 99, backroundcolor);
   }

if (rpmprev!=rpm2){           
  tft.setFreeFont(&Roboto_Mono_Medium_96);
  tft.setTextColor(rpmcolor, backroundcolor);
  tft.drawNumber(rpm2,236,172); 
 
rpmprev=rpm2;
}
  last_show_rpm= millis();
  drawOdo();
 }

speedupdated2=speedupdated;
 
if ((millis()-last_show_speed) >100)
{

if ((millis()-last_update_speed) >2000)
{
speed1=0;
}

if (speedupdated)
{
  noInterrupts();
  speedupdated=false;
  speedcountTmp=speedcount;
  speedcount = 0;
  duration_speedTmp=duration_speed;
  interrupts();

speedo = ratas*3600.0/duration_speedTmp;  
speed1 = int(speedo+0.5);   
speed1 = min(99,speed1);

distance =speedcountTmp*ratas;
odo += distance;
odo2=odo/1000;
dispodo = (roundf(odo2*10))/10;
allodo += distance/1000;

if ((dispodo-dispodoprev)>1)
{
  dispodoprev=dispodo;
  EEPROM.begin(eepromsize);
  EEPROM.put(odoaddr,odo);  
  EEPROM.put(allodoaddr,allodo); 
  EEPROM.commit();
}

last_update_speed= millis();

}

if (speed1!=speedprev){ 
  tft.setFreeFont(&Open_Sans_Condensed_Bold_137);
  if (speed1<10)
   { 
   tft.fillRect(0, 180, 180, 140, backroundcolor);
   }
 
   tft.setTextColor(speedcolor, backroundcolor); 
   tft.drawNumber(speed1,180,320);
 speedprev=speed1;   
}
    
last_show_speed= millis();
}


if (speed1 == 0)
{
flag50 = false;
speedprev50 = 0;
}

if ((speedupdated2) && (speedprev50 == 0) && (!flag50))
{
time50start = millis();
if (!flag50)
{
  tft.fillRect(70, 32, 80, 30, backroundcolor);
  time50 = 0;
}
flag50 = true;
speedupdated2=false;
speedprev50 = speed1;
}

if ((speed1>50) && (flag50))
{
time50 = (millis()-time50start)/1000.0;
time50 = roundf(time50*100)/100;
flag50 = false;
}

 
}

ICACHE_RAM_ATTR void speed_counter()
{
speedcount++;
duration_speed = millis()-last_speed;
last_speed = millis();
speedupdated=true;
}

 



void drawStatic()
 {
 tft.fillScreen(backroundcolor);
 tft.setFreeFont(&Roboto_Mono_Thin_24);
 tft.setTextColor(odocolor, backroundcolor); 
 tft.drawString("ODO",2,32);
 tft.drawString("TRIP",2,62);
 tft.setFreeFont(&Roboto_Mono_Medium_24);
 tft.setTextColor(speedcolor, backroundcolor); 
 tft.drawString("km/h",236,290);
 }


void drawOdo()
 {
 tft.setFreeFont(&Roboto_Mono_Medium_24);
 tft.setTextColor(odocolor, backroundcolor);
 tft.drawFloat(allodo,1,230,32);    
 tft.drawFloat(dispodo,1,230,62); 

  tft.setTextColor(TFT_RED, backroundcolor);
  tft.drawFloat(time50,2,230,179);             //display 0-50 km/h time
 }
