#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <SPI.h>



TFT_eSPI tft = TFT_eSPI();

unsigned int backroundcolor = TFT_WHITE;
unsigned int speedcolor = TFT_BLACK;
unsigned int rpmcolor = TFT_BLUE;
unsigned int odocolor = TFT_BLACK;


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

unsigned long last_serial_print=0;
unsigned long last_show_odo=0;
unsigned long last_show_50=0;

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

const int numReadings = 5;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;


int speedpin = D1;
int speedprev=99;
volatile byte speedcount=0;
int speedcountTmp=0;
float ratas=0.312;    //wheel circumstance divided by hall sensor pulses per rotation
float distance=0;
int speed1=0;
float speedo=0;
float odo=0;
float odo2=0;
float odo3=0;
float odo4=0;
float dispodo = 0;
float dispodo_last;
float dispodoprev = 0;
float allodo;
float allodo_last;
unsigned long last_show_speed=0;
unsigned long last_update_speed=0;
unsigned long duration_speedTmp=1;
volatile unsigned long duration_speed=1;
volatile unsigned long last_speed=0;
volatile bool speedupdated=false;

bool flag50 = false;
unsigned long time50start = 0;
float time50 = 0;
float time50_prev=0;
int speedprev50 = 0;


void setup()   {   

 Serial.begin(115200);


  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(300);


  tft.init();
  tft.setRotation(2);
  tft.setTextDatum(BR_DATUM);

drawStatic();


  pinMode(button, INPUT);
  pinMode(rpmpin,INPUT);
  pinMode(speedpin,INPUT);

  attachInterrupt(digitalPinToInterrupt(speedpin), speed_counter, FALLING);
 
  Serial.println("Reading memory.");

  EEPROM.begin(eepromsize);
  EEPROM.get(odoaddr,odo);  
  EEPROM.get(allodoaddr,allodo); 
  EEPROM.end(); 

  odo2 = odo/1000;
  odo4=odo;
  dispodo = roundf(odo2*10)/10;
  dispodoprev = dispodo;

  Serial.print("ODOm: ");
  Serial.println(odo);
  Serial.print("Dispodo: ");
  Serial.println(dispodo);

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }


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

  Serial.println("Going to sleep.");
  tft.fillScreen(backroundcolor);
  tft.setTextColor(odocolor, backroundcolor); 
  tft.setFreeFont(&Roboto_Mono_Thin_24);
  tft.drawString("ODO",2,28);
  tft.drawString("TRIP",2,52);

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
  Serial.println("Button long press ");
   longPressActive = true;
   odo=0;
   odo4=0;
   dispodo=0;
   dispodoprev = 0;
   tft.fillRect(150, 28, 90, 26, backroundcolor);
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
  Serial.println("Button short press");
  
   if (!statebutton)
       {
       backroundcolor = TFT_WHITE;
       speedcolor = TFT_BLACK;
       rpmcolor = TFT_BLUE;
       odocolor = TFT_BLACK;
       }

      if (statebutton)
       {
       backroundcolor = TFT_BLACK;
       speedcolor = TFT_GREEN;
       rpmcolor = TFT_YELLOW;
       odocolor = TFT_WHITE;
       }

      drawStatic();
 
 tft.setFreeFont(&Roboto_Mono_Medium_24);  // digits max widthXheight  15x17
 tft.setTextColor(odocolor, backroundcolor);
 tft.drawFloat(allodo,1,230,28);    
 tft.drawFloat(dispodo,1,230,52);

  }
   buttonActive = false;
   }
 }



 if (digitalRead(rpmpin) == 1 && previous_rpm == 0)
  {
  previous_rpm = 1;
  duration_rpm = elapsedt_rpm - elapsed_prev_rpm;
  rpmupdated=true;
  elapsed_prev_rpm  = micros();   
  }

 if (digitalRead(rpmpin) == 1 && previous_rpm == 1)
  {
  previous_rpm = 1;       
  }

 if (digitalRead(rpmpin) == 0 && previous_rpm == 1)
  {
  cur=micros();
  del=cur - elapsed_prev_rpm;
   if (del < 5900) 
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
  elapsedt_rpm = micros(); 
  del2=elapsedt_rpm - elapsed_prev_rpm;
  if (del2>2000000)
   {
   rpmflag=true;
   }
  }
 
 

if (rpmupdated){
 rpmai = 60000000/duration_rpm;
 rpm = round (rpmai);
 rpm=((rpm+5)/10)*10;  //round to tens
 rpmupdated=false;

 printData();

 if ( ((rpm_a-100) > rpm) && (rpm > (rpm_a+100))&&(rpm>1000))
  {
     rpm = rpm_a;
  }

 if ( ((rpm_a-50) < rpm) && (rpm < (rpm_a+50)))
  {
  rpm_a = rpm;

  total = total - readings[readIndex];
  readings[readIndex] = rpm;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings)
      {
       readIndex = 0;
       }
  }
  else
     {
     rpm_a=rpm;
     }
}

 if ((millis()-last_show_rpm) >300)  //refresh rate
  {

rpm2 = total / numReadings;
rpm2=((rpm2+5)/10)*10;

  if (rpmflag)
   {
   rpm2=0;
   rpmflag=false;
   tft.fillRect(0, 62, 177, 96, backroundcolor);
   }

if (rpmprev!=rpm2){          

  if (rpm2<1000)
   { 
   tft.fillRect(0, 62, 177, 96, backroundcolor);
   }
   
  tft.setFreeFont(&Roboto_Mono_Medium_96);  // digits max widthXheight  59x71
  tft.setTextColor(rpmcolor, backroundcolor);
  tft.drawNumber(rpm2,236,162); 
 
rpmprev=rpm2;
}
  last_show_rpm= millis();
 }



if ((millis()-last_update_speed) >700)
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

speedo = ratas*3600000.0/duration_speedTmp;  
speed1 = int(speedo+0.5);  
speed1 = speed1 % 100;

distance =speedcountTmp*ratas;
odo += distance;
odo2=odo/1000;
dispodo = (roundf(odo2*10))/10;
allodo += distance/1000;

last_update_speed= millis();

}

if ((millis()-last_show_speed) >300)
{

if (speed1!=speedprev){ 
  tft.setFreeFont(&&Open_Sans_Condensed_Bold_137);
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


if ((millis()-last_show_odo) >1000)
{
    drawOdo();
    last_show_odo=millis();
}


if ((millis()-last_show_50) >100)
{
    draw50();
    last_show_50=millis();
}



if (speed1 == 0)
{
flag50 = false;
speedprev50 = 0;
}

if ((speed1>0) && (!flag50) && (speedprev50 == 0))
{
time50start = millis();
time50 = 0;
flag50 = true;
speedprev50 = speed1;
}

if ((speed1<50) && (flag50))
{
time50 = (millis()-time50start)/1000.0;
time50 = roundf(time50*100)/100;

   if ((millis()-time50start)>9000)
   {
     time50=9.9;
     flag50 = false;
   }

}

if ((speed1>=50) && (flag50))
{
time50 = (millis()-time50start)/1000.0;
time50 = roundf(time50*100)/100 + 0.3;  //adding 0.3 seconds to compensate first signal not accounted for wheel rotation
flag50 = false;
}


yield(); 
}

ICACHE_RAM_ATTR void speed_counter()
{
speedcount++;
duration_speed = micros()-last_speed;
last_speed = micros();
speedupdated=true;
}



void drawStatic()
 {
 tft.fillScreen(backroundcolor);
 tft.setFreeFont(&Roboto_Mono_Thin_24);
 tft.setTextColor(odocolor, backroundcolor); 
 tft.drawString("ODO",2,28);
 tft.drawString("TRIP",2,52);
 }

void printData()
{

  Serial.print(micros());
  Serial.print(",");
  Serial.print(rpmai);
  Serial.print(",");
  Serial.print(rpm2);
  Serial.print(",");
  Serial.println(speed1);

}

void drawOdo()
 {
 tft.setFreeFont(&Roboto_Mono_Medium_24);  
 tft.setTextColor(odocolor, backroundcolor);

if(allodo!=allodo_last)
{
 tft.setFreeFont(&Roboto_Mono_Medium_24);  
 tft.setTextColor(odocolor, backroundcolor);
 tft.drawFloat(allodo,1,230,28);    
allodo_last=allodo;
}

if(dispodo!=dispodo_last)
{
 tft.setFreeFont(&Roboto_Mono_Medium_24);
 tft.setTextColor(odocolor, backroundcolor);
 tft.drawFloat(dispodo,1,230,52); 
dispodo_last=dispodo;
}


 void draw50()
 {
 if(time50!=time50_prev)
 {
  tft.setFreeFont(&Roboto_Mono_Medium_24);
  tft.setTextColor(TFT_RED, backroundcolor);
  tft.drawFloat(time50,2,120,52);
  time50_prev=time50;
  }
 }
