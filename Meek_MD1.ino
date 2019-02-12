/*Meek MD1 v0.7.1 12-Feb-2019, Atmega328 Source Code for Zero Cross-, I2C, Interface controller for
Meek MD1 Wi-Fi enabled dimmer ( http://www.meek-ha.com/ )
Predefined commands:
6000 - Turn the Dimmer On and fade up to the last known DimLevel ( http://192.168.2.39/control?cmd=EXTPWM,5,6000 )
6001 - Turn the Dimmer Off by fading down ( http://192.168.2.39/control?cmd=EXTPWM,5,6001 )
6002 - Instant Max. brightness ( http://192.168.2.39/control?cmd=EXTPWM,5,6002 )
6003 – Instant Off ( http://192.168.2.39/control?cmd=EXTPWM,5,6003 )
5000,5999 - Fade Delay (Set delay for each dimlevel transition , Default=10 : http://192.168.2.39/control?cmd=EXTPWM,5,5010 )
4000,4999 - UpperLimit ( UpperLimit range 4000-4999 ,  Default UpperLimit=990: http://192.168.2.39/control?cmd=EXTPWM,5,4990 )
3000,3999 - LowerLimit (LowerLimit range 3000-3999 ,  Default LowerLimit=0: http://192.168.2.39/control?cmd=EXTPWM,5,3000 )
8000,8100 - Input Home Automation System (script_device_Meek_MD1.lua for Domoticz , http://192.168.2.39/control?cmd=EXTPWM,5,8050 )
Meek – Input from ESPEasy ( e.g. http://<ESP IP>/control?cmd=EXTPWM,5,<Value> )

Zero Cross:
Updated by Robert Twomey
Adapted from sketch by Ryan McLaughlin
http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1230333861/30

I2C:
https://github.com/letscontrolit/ESPEasySlaves/tree/master/MiniProExtender

*/

#include <Wire.h>

#define I2C_MSG_IN_SIZE    4
#define I2C_MSG_OUT_SIZE   4

#define CMD_DIGITAL_WRITE  1
#define CMD_DIGITAL_READ   2
#define CMD_ANALOG_WRITE   3
#define CMD_ANALOG_READ    4

volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

int StepDown=1;
int StepUp=1;
int InputDelay=10;
uint32_t Percent;
uint32_t Percent1;
int UpperLimit = 990;
int LowerLimit = 0; 
int TouchUp = 6;
int TouchDown = 7;
int DimLevel = 50;
int Meek=UpperLimit;
int HASystem = UpperLimit;
int On = 0;

#include  <TimerOne.h>        // Avaiable from http://www.arduino.cc/playground/Code/Timer1
volatile int i=0;             // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross=0;// Boolean to store a "switch" to tell us if we have crossed zero
int AC_pin = 3;               // Output to Opto Triac
int dim = UpperLimit;         // Dimming level (0-128)  0 = on, 128 = 0ff
int inc=1;                    // counting up or down, 1=up, -1=down
int freqStep = 10;            // This is the delay-per-brightness step in microseconds.
                      

void setup() {                                      // Begin setup
  pinMode(AC_pin, OUTPUT);                          // Set the Triac pin as output
  attachInterrupt(0, zero_cross_detect, RISING);    // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  Timer1.initialize(freqStep);                      // Initialize TimerOne library for the freq we need
  Timer1.attachInterrupt(dim_check, freqStep);      // Use the TimerOne Library to attach an interrupt This function will now run every freqStep in microseconds.     
  Wire.begin(0x7f);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
// Serial.begin(9600);                              
}

void zero_cross_detect() {    
  zero_cross = true;               // set the boolean to true to tell our dimming function that a zero cross has occured
  i=0;
  digitalWrite(AC_pin, LOW);       // turn off TRIAC (and AC)
}                                 

//  ------------------- Turn on the TRIAC at the appropriate time Begin ---------------------
void dim_check() {                   
  if(zero_cross == true) {              
    if(i>=dim) {                     
      digitalWrite(AC_pin, HIGH);   // turn on light       
      i=0;                          // reset time step counter                         
      zero_cross = false;           //reset zero cross detection
    } 
    else {
      i++;                          // increment time step counter                     
    }                                
  }                                  
}                                   
//  ------------------- Turn on the TRIAC at the appropriate time End ---------------------


void loop() {                        

//  ------------------- Up & Down Button Begin ---------------------
digitalWrite(TouchUp, LOW);
digitalWrite(TouchDown, LOW);
int Down = digitalRead(TouchDown);
int Up = digitalRead(TouchUp);

if (Down == HIGH && dim>(LowerLimit) && dim<=(UpperLimit) && On==(1) && Meek!=6001){   
dim=dim-1;
DimLevel=dim;
Meek=DimLevel;}

if (Down == HIGH && Meek==6000){   
dim=dim-1;
DimLevel=dim;
Meek=DimLevel;}

if (Up == HIGH && Meek==6000){   
dim=dim+1;
DimLevel=dim;
Meek=DimLevel;}

if ( Up == HIGH && dim>=(LowerLimit) && dim<(UpperLimit) && On==(1) && Meek!=6001){
dim=dim+1;
DimLevel=dim;
Meek=DimLevel;}
//  ------------------- Up & Down Button End ---------------------


//  ------------------- Maintain Upper & LowerLimit Start ---------------------
if (dim>UpperLimit){
  dim=UpperLimit;}

if (dim<LowerLimit && dim!=0){
  dim=LowerLimit;}
//  ------------------- Maintain Upper & LowerLimit End ---------------------


//  ------------------- Synchronize Percentage with Home Automation system 0-100% Start ---------------------
Percent1= map(dim , LowerLimit, UpperLimit, 1000 , 0);
Percent= round(Percent1/10);
//  ------------------- Synchronize Percentage with Home Automation system 0-100% End ---------------------


//  ------------------- Touch Button On Off Start ---------------------
//6000=On
if (Meek==6000 && dim!=DimLevel && On==1){
  dim=dim-1;}
if (Meek==6000 && On==0){
On=1;}
  
//6001=Off
if (Meek==6001 && dim<UpperLimit){
  dim=dim+1;}
if (Meek==6001 && dim==UpperLimit){
On=0;    }
//  ------------------- Touch Button On Off End ---------------------


//  ------------------- Input from Meek Start ---------------------
if (Meek>LowerLimit && Meek<=UpperLimit && dim!=Meek && dim>=Meek ){
  dim=dim-1;    }
  
if (Meek>=LowerLimit && Meek<UpperLimit && dim!=Meek && dim<=Meek ){
  dim=dim+1;    }
//  ------------------- Input from Meek End ---------------------


//  ------------------- Input Home Automation System HASystem Start ---------------------
if (Meek>=8000 && Meek<=8100){
  HASystem=(Meek-8000);
Meek = map(HASystem , 100 , 0 , (LowerLimit), (UpperLimit));
DimLevel=Meek;
On=1;}
//  ------------------- Input Home Automation System HASystem End ---------------------


//  ------------------- 6002 Full Brightness Command Start ---------------------
if (Meek==6002){
  dim=LowerLimit;
  DimLevel=dim;
  On=1;
  Meek=6000;}
//  ------------------- 6002 Full Brightness Command End ---------------------


//  ------------------- 5000,5999 - Fade Delay Start ---------------------
if (Meek>=5000 && Meek<=5999){
  InputDelay=Meek-5000;}
//  ------------------- 5000,5999 - Fade Delay End ---------------------


//  ------------------- Set UpperLimit(4000,4999) & LowerLimit(3000,3999) Start ---------------------
if (Meek>=4000 && Meek<=4999){
  UpperLimit=Meek-4000;}
if (Meek>=3000 && Meek<=3999){
  LowerLimit=Meek-3000;}
//  ------------------- Set UpperLimit(4000,4999) & LowerLimit(3000,3999) End ---------------------



/*
Serial.print("Dim = ");  
Serial.print(dim);
Serial.print("Inc = ");  
Serial.print(inc);
Serial.print("i = ");  
Serial.print(i);
Serial.print(" Meek = ");  
Serial.print(Meek);
Serial.print(" Up =");
Serial.print(Up); 
Serial.print(" Down =");
Serial.print(Down); 
Serial.print(" Percent =");
Serial.print(Percent); 
Serial.print(" Percent1 =");
Serial.print(Percent1); 
Serial.print(" Difference =");
Serial.print(Difference); 
Serial.print(" DimLevel =");
Serial.print(DimLevel); 
Serial.print(" HASystem =");
Serial.print(HASystem); 
Serial.print(" On =");
Serial.print(On); 

Serial.print('\n');
//*/
delay(InputDelay);
  
}


void receiveEvent(int count)
{
  if (count == I2C_MSG_IN_SIZE)
  {
    byte cmd = Wire.read();
    byte port = Wire.read();
    int value = Wire.read();
    value += Wire.read()*256;
    switch(cmd)
      {
        case CMD_DIGITAL_WRITE:
          pinMode(port,OUTPUT);
          digitalWrite(port,value);
          break;
        case CMD_DIGITAL_READ:
          pinMode(port,INPUT_PULLUP);
         clearSendBuffer();
         sendBuffer[0] = digitalRead(port);
         break;
        case CMD_ANALOG_WRITE:
          analogWrite(port,value);
          Meek=(value);
          break;
       case CMD_ANALOG_READ:
         clearSendBuffer();
         uint32_t valueRead = Percent;
          //int valueRead = analogRead(port);
          sendBuffer[0] = valueRead & 0xff;
         sendBuffer[1] = valueRead >> 8;
          break;
      }
  }
}

void clearSendBuffer()
{
  for(byte x=0; x < sizeof(sendBuffer); x++)
    sendBuffer[x]=0;
}

void requestEvent()
{
  Wire.write((const uint8_t*)sendBuffer,sizeof(sendBuffer));
}
