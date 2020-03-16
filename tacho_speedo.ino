#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#define TFT_RST  PIN_D4

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

int rpmpin = D2;
int rpm = 1;
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


int wheelpin = D1;
int wheel=1.56; //wheel circumference in meters
int speed1=0;
int speed3=1;
int speed_a=1;
int counter = 0;
int present = 0;
int previous = 0;
float speedo=1;
float odo=0;
float odo2=0;
float dispodo = 0;
float allodo = 0;
unsigned long duration = 1;
unsigned long elapsed_wheel = 1;
unsigned long elapsed_prev = 1;
unsigned long last_show_speed=0;
unsigned long cur_speed = 0; 
unsigned long del3 = 0;
unsigned long del4 = 0;
bool wheelflag=true;
bool speedupdated=false;



void setup()   {   
 
  WiFi.forceSleepBegin();
  delay(300);


  tft.init();
  tft.setRotation(2);
  tft.setTextDatum(BR_DATUM);

drawStatic();

  pinMode(button, INPUT_PULLUP);
  pinMode(rpmpin,INPUT);
  pinMode(wheelpin,INPUT);
 
  EEPROM.begin(10);
  EEPROM.get(3,dispodo);  
  odo = dispodo * 1000;
//  dispodo=0.1;
  EEPROM.get(7,allodo); 
//  allodo=5660.2;


}

void loop() 
 {

 int sensorValue = analogRead(A0);
 if (sensorValue < 100)             //check if ignition is off, save dispodo and allodo
  {
  EEPROM.begin(10);
  EEPROM.put(3,dispodo);  
  EEPROM.put(7,allodo); 
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
   if (del < 5)                  //debounce 
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
  if (del2>1000)             //if no updates for 1 second, set rpm to 0
   {
   rpmflag=true;
   }
  }
 
 if (digitalRead(wheelpin) == 1 && previous == 0)
  {
   previous = 1;
   duration = elapsed_wheel - elapsed_prev;
   elapsed_prev  = millis();
   duration=duration;    
   odo += wheel/6;          // i have 6 signal changes per rotation
   allodo += wheel/6000;    // add traveled distance in kilometers
   speedupdated=true;
  }

 if (digitalRead(wheelpin) == 1 && previous == 1)
  {
  previous = 1;    
  elapsed_wheel = millis();   
  del3=elapsed_wheel - elapsed_prev;
  if (del3>1000)            //if no updates for 1 second, set speed to 0    
   {
   wheelflag=true;
   }
  }

 if (digitalRead(wheelpin) == 0 && previous == 1)
  {
   previous = 0; 
   duration = elapsed_wheel - elapsed_prev;
   elapsed_prev  = millis();
   duration=duration;    
   odo += wheel/6;         // i have 6 signal changes per rotation
   allodo += 0.00026;      // add traveled distance in kilometers
   speedupdated=true;    
  }

 if (digitalRead(wheelpin) == 0 && previous == 0)
  {
  previous = 0;
  elapsed_wheel = millis();   
  del3=elapsed_wheel - elapsed_prev;
  if (del3>1000)            //if no updates for 1 second, set speed to 0 
   {
   wheelflag=true;
   }
  }

if (rpmupdated){
 rpmai = 60000/duration_rpm;
 rpm = round (rpmai);
 rpm = min(9999, rpm);    // show 9999 if rpm is higher than 9999
 rpmupdated=false;
 
 if ( (rpm_a-10) < rpm  &&  rpm < (rpm_a+10))
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

if (speedupdated){
 speedo = (wheel/6)*3600/duration;  
 speed1 = int(speedo+0.5);  // rounding
 odo2 = odo/1000;
 dispodo = roundf(odo2*10)/10;  // round to tens
 speed1 = min(99,speed1);       // show 99 if speed higher than 99. Due to font issue cannot show higher speed
 speedupdated=false;

 if ( (speed_a-1) < speed1  &&  speed1 < (speed_a+1))
  {
  speed_a = speed1;
  speed3 = speed3 + speed1;
  counter = counter + 1;
  }
  else
   {
   speed_a=speed1;
   }

}


 if ((millis()-last_show_rpm) >800)  //refresh rate
  {
 if (counter_rpm>0){       //calculate only if updated
  rpm2=rpm3/counter_rpm;
  rpm2=((rpm2+5)/10)*10;  //round to tens
  counter_rpm=0;
  rpm3=0;
 }
  if (rpmflag)
   {
   rpm2=0;
   rpmflag=false;
   tft.fillRect(0, 80, 181, 99, backroundcolor);
   }

           
  tft.setFreeFont(&Roboto_Mono_Medium_96);
  tft.setTextColor(rpmcolor, backroundcolor);
  tft.drawNumber(rpm2,236,172); 
  last_show_rpm= millis();

    drawOdo();
 }


  if ((millis()-last_show_speed) >300)  //refresh rate
  {
if (counter>0){       //calculate only if updated
  speed1 =speed3/counter;
  counter =0;
  speed3 = 0;
}
  if (wheelflag)
   {
   speed1=0;
   wheelflag=false;
   }

  tft.setFreeFont(&Open_Sans_Condensed_Bold_137);
  if (speed1<10)              // clear display if speed less than 10
   { 
      tft.fillRect(0, 180, 180, 140, backroundcolor);
   }
  if(speed1<55)
   { 
   tft.setTextColor(speedcolor, backroundcolor); 
   tft.drawNumber(speed1,180,320);
   }
   else                        //if speed is above 55 km/h, display it in RED
    {
    tft.setTextColor(TFT_RED, backroundcolor);
    tft.drawNumber(speed1,180,320);
    }
    
  last_show_speed= millis();
  }
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
 }
