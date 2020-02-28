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
int counter_rpm = 10000;
int present_rpm = 0;
int previous_rpm = 0;
float rpmai = 1;
unsigned long duration_rpm = 10000;
unsigned long elapsedt_rpm = 1;
unsigned long elapsed_prev_rpm = 1;
unsigned long last_show=0;
unsigned long cur = 0;
unsigned long del= 0;
unsigned long del2= 0;
bool rpmflag=false;


int wheelpin = D1;
int wheel=156; //your wheel circumference in cm
int speed1=0;
int speed3=1;
int speed_a=1;
int counter = 10000;
int present = 0;
int previous = 0;
float speedo=1;
float odo=0;
float odo2=0;
float dispodo = 0;
float dispodo2 = 0;
float allodo = 0;
unsigned long duration = 10000;
unsigned long elapsed_wheel = 1;
unsigned long elapsed_prev = 1;
unsigned long del3= 0;
bool wheelflag=false;



void setup()   {   
  WiFi.forceSleepBegin();
  delay(1500);


  tft.init();
  tft.setRotation(2);         // rotate screen 180 degrees
  tft.setTextDatum(BR_DATUM); 

drawStatic();

  pinMode(button, INPUT_PULLUP);  // change to pinMode(button, INPUT);  if capacitive button is used
  pinMode(rpmpin,INPUT);
  pinMode(wheelpin,INPUT);
 
  EEPROM.begin(10);
  EEPROM.get(3,dispodo);  
  odo = dispodo * 1000;
  dispodo2=dispodo;
  EEPROM.get(7,allodo); 
                        // uncomment below two lines during first time programming to store some values to memory. change allodo value to your mileage
//  dispodo=0.1;  
//  allodo=5660.2;


    elapsedt_rpm = millis();
    elapsed_wheel = millis();

}

void loop() 
 {

 int sensorValue = analogRead(A0);
 if (sensorValue < 700)               //if ignition was switched off, record odo and trip to memory
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

if (digitalRead(button) == LOW)
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
   dispodo2=0;
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
      
      if (statebutton)                 // delete if you don't need black on white color scheme
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
   if (del < 5)                  // here we do debounce for rpm input as pickup coil can be noisy
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
  if (del2>1000)                   //if no sigal for 1 second, add flag which makes rpm to show "0"
   {
   rpmflag=true;  
   }
  }
 
 if (digitalRead(wheelpin) == 1 && previous == 0)
  {
  previous = 1;
  duration = elapsed_wheel - elapsed_prev;
  elapsed_prev  = millis();
  duration=duration*2;    //we have two impulses per revolution if adding hall inside original speedometer
  odo += wheel/200;        // as above
  }

 if (digitalRead(wheelpin) == 1 && previous == 1)
  {
  previous = 1;    
  elapsed_wheel = millis();   
  del3=elapsed_wheel - elapsed_prev;
  if (del3>1000)              //if no sigal for 1 second, add flag which makes speed to show "0"
   {
   wheelflag=true;
   }
  }

 if (digitalRead(wheelpin) == 0 && previous == 1)
  {
  previous = 0;     
  }

 if (digitalRead(wheelpin) == 0 && previous == 0)
  {
  previous = 0;
  elapsed_wheel = millis();   
  del3=elapsed_wheel - elapsed_prev;
  if (del3>1000)              //if no sigal for 1 second, add flag which makes speed to show "0"
   {
   wheelflag=true;
   }
  }

 rpmai = 60000/duration_rpm;   //math to get rpm
 rpm = round (rpmai);

 speedo = (36*wheel)/duration;  //math to get speed
 speed1 = round(speedo);
 odo2 = odo/1000;
 dispodo = roundf (odo2*10)/10;
 allodo = allodo + dispodo - dispodo2;
 dispodo2=dispodo;

 if ( (rpm_a-10) < rpm  &&  rpm < (rpm_a+10))   //some error correction to remove unexpected deviations
  {
  rpm_a = rpm;
  rpm3 = rpm3 + rpm;
  counter_rpm = counter_rpm + 1;
  }
  else
   {
   rpm_a=rpm;
   }

 if ( (speed_a-2) < speed1  &&  speed1 < (speed_a+2)) //some error correction to remove unexpected deviations
  {
  speed_a = speed1;
  speed3 = speed3 + speed1;
  counter = counter + 1;
  }
  else
   {
   speed_a=speed1;
   }

 if ((millis()-last_show) >500)  //refresh rate 0.5 seconds
  {
  rpm2=rpm3/counter_rpm;
  rpm2=((rpm2+5)/10)*10;  //round to tens
  counter_rpm=0;
  rpm3=0;

  speed1 =speed3 / counter;
  counter =0;
  speed3 = 0;

  if (rpmflag)
   {
   rpm2=0;
   rpmflag=false;
   tft.fillRect(0, 80, 240, 99, backroundcolor);
   }

  if (wheelflag)
   {
   speed1=0;
   wheelflag=false;
   }

  tft.setFreeFont(&Open_Sans_Condensed_Bold_137);
  if (speed1<10)
   { 
   tft.fillRect(52, 190, 64, 100, backroundcolor);
   }
  if(speed1<55)                                       //if speed is less than 55 km/h, draw it green and below sets color red for above 55 speeds
   { 
   tft.setTextColor(speedcolor, backroundcolor); 
   tft.drawNumber(speed1,180,320);
   }
   else
    {
    tft.setTextColor(TFT_RED, backroundcolor);
    tft.drawNumber(speed1,180,320);
    }
           
  tft.setFreeFont(&Roboto_Mono_Medium_96);
  tft.setTextColor(rpmcolor, backroundcolor);
  tft.drawNumber(rpm2,236,172); 

  drawOdo();

  last_show= millis();
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
