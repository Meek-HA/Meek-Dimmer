/*
 * ***************************************************************************************************************************\
 * Arduino project "ESP Easy" � Copyright www.esp8266.nu
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://sourceforge.net/projects/espeasy/
 * Support         : http://www.esp8266.nu
 * Discussion      : http://www.esp8266.nu/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
**************************************************************************************************************************/

// This file is to be loaded onto an Arduino Pro Mini so it will act as a simple IO extender to the ESP module.
// Communication between ESP and Arduino is using the I2C bus, so only two wires needed.
// It best to run the Pro Mini on 3V3, although the 16MHz versions do not officially support this voltage level on this frequency.
// That way, you can skip levelconverters on I2C.
// Arduino Mini Pro uses A4 and A5 for I2C bus. ESP I2C can be configured but they are on GPIO-4 and GPIO-5 by default.



#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define DETECT 2  //zero cross detect
#define GATE 3    //TRIAC gate
#define PULSE 4   //trigger pulse width (counts)

int Meek;
int StepDown=1;
int StepUp=1;
int InputDelay=10;
int Percent;
int Percent1;
int Difference ; 
int UpperLimit = 605;
int LowerLimit = 150; 
int TouchUp = 6;
int TouchDown = 7;
int DimLevel = 400;
int Mistik;

#define I2C_MSG_IN_SIZE    4
#define I2C_MSG_OUT_SIZE   4

#define CMD_DIGITAL_WRITE  1
#define CMD_DIGITAL_READ   2
#define CMD_ANALOG_WRITE   3
#define CMD_ANALOG_READ    4


volatile uint8_t sendBuffer[I2C_MSG_OUT_SIZE];

void setup()
{
  Wire.begin(0x7f);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
 // Serial.begin(9600);
  pinMode(TouchUp, INPUT); 
  pinMode(TouchDown, INPUT); 

// set up pins
  pinMode(DETECT, INPUT);     //zero cross detect
  digitalWrite(DETECT, HIGH); //enable pull-up resistor
  pinMode(GATE, OUTPUT);      //TRIAC gate control

  // set up Timer1 
  //(see ATMEGA 328 data sheet pg 134 for more details)
  OCR1A = 100;      //initialize the comparator
  TIMSK1 = 0x03;    //enable comparator A and overflow interrupts
  TCCR1A = 0x00;    //timer control registers set for
  TCCR1B = 0x00;    //normal operation, timer disabled

  attachInterrupt(0,zeroCrossingInterrupt, RISING);    
 
}

void zeroCrossingInterrupt(){ //zero cross detect   
  TCCR1B=0x04; //start timer with divide by 256 input
  TCNT1 = 0;   //reset timer - count from zero
}

ISR(TIMER1_COMPA_vect){ //comparator match
  digitalWrite(GATE,HIGH);  //set TRIAC gate to high
  TCNT1 = 65536-PULSE;      //trigger pulse width
}

ISR(TIMER1_OVF_vect){ //timer1 overflow
  digitalWrite(GATE,LOW); //turn off TRIAC gate
  TCCR1B = 0x00;          //disable timer stopd unintended triggers
}


void loop() {

//  ------------------- Up & Down Button Begin ---------------------
digitalWrite(TouchUp, LOW);
digitalWrite(TouchDown, LOW);
int Down = digitalRead(TouchDown);
int Up = digitalRead(TouchUp);

if (Up == HIGH && Meek >=LowerLimit && Meek<=UpperLimit){
Meek=Meek +1 ;}
if (Up == HIGH && OCR1A==0 ){
Meek=UpperLimit;}
if ( Down == HIGH && Meek >=LowerLimit && Meek<=UpperLimit){
Meek=Meek -1 ;}
if ((Down == HIGH) && (OCR1A==0 )){
Meek=LowerLimit;}

//  ------------------- Up & Down Button End ---------------------


//  ------------------- DimLevel Start ---------------------
if (Meek >= LowerLimit && Meek <=UpperLimit){
  DimLevel=Meek;
}
//  ------------------- DimLevel End ---------------------
//  ------------------- Maintain Upper & LowerLimit Start ---------------------
if (OCR1A>UpperLimit) {
OCR1A=UpperLimit;}
if (OCR1A>0 && OCR1A<=LowerLimit) {
OCR1A=LowerLimit;}
if (Meek ==0){
  OCR1A=0;}
//  ------------------- Maintain Upper & LowerLimit End ---------------------
//  ------------------- Synchronize Meek with OCR1A Start ---------------------
if (Meek>=LowerLimit && Meek<=UpperLimit) {  
OCR1A = Meek;     }
//  ------------------- Synchronize Meek with OCR1A Start ---------------------
//  ------------------- Synchronize Percentage with Home AUtomation system 0-100% Start ---------------------
Difference = round(UpperLimit / LowerLimit);
if (Meek>=LowerLimit && Meek<=UpperLimit) {
  Percent = map(OCR1A , LowerLimit, UpperLimit, 100 , 0);}
//  ------------------- Synchronize Percentage with Home AUtomation system 0-100% End ---------------------
//  ------------------- Touch Button On Off Start ---------------------
if (Meek==6000 && OCR1A==0 ){
  OCR1A=UpperLimit;} 
if (Meek==6000 && OCR1A>=DimLevel){
  OCR1A=OCR1A-1;} 
if (Meek==6000 && OCR1A<=DimLevel){
  OCR1A=OCR1A+1;} 
   
if (Meek==6001 && OCR1A<UpperLimit ){
  OCR1A=OCR1A+1;}
if (Meek==6001 && OCR1A==UpperLimit ){
  OCR1A=0;
  Meek=0;
  }
  if (OCR1A==DimLevel){
  Meek=DimLevel;} 
//  ------------------- Touch Button On Off End ---------------------
//  ------------------- Store DimLevel Start ---------------------
//if (Meek >=LowerLimit && Meek <=UpperLimit && OCR1A>=LowerLimit && OCR1A<=UpperLimit){
//  DimLevel=Meek;}
//  ------------------- Store DimLevel End ---------------------

//if (Meek>=500 && Meek<=600) {
//OCR1A =map(Meek,1,100, LowerLimit,UpperLimit);}
//if (Meek>=500 && Meek<=600) {
//  OCR1A =map(Meek,500,600, LowerLimit,UpperLimit);}
//if (Meek>=LowerLimit && Meek<=UpperLimit && Difference > (Percent-Percent1)) {
//  Percent = Percent1;}
//-------------------------------------------------------------------------------
//if (Meek>=1300 && Meek<=1399){
//  LowerLimit=(Meek-1300);} 
//if (Meek>=2000 && Meek<=2999){
//  UpperLimit=(Meek-2000);} 
//if (Meek>=1500 && Meek<=1599){
//  StepDown=(Meek-1500);} 
//if (Meek>=1500 && Meek<=1599){
//  StepDown=(Meek-1500);} 
//if (Meek>=1600 && Meek<=1699){
//  StepUp=(Meek-1600);} 
//if (Meek>=1700 && Meek<=1799){
//  InputDelay=(Meek-1700);}   
//if (Meek>=1800 && Meek<=1899 && OCR1A>LowerLimit && OCR1A<=(UpperLimit-2)){
//  OCR1A=(OCR1A-(Meek-1800));} 
//if (Meek>=1900 && Meek<=1999 && OCR1A>=LowerLimit && OCR1A<(UpperLimit-2) ){
//  OCR1A=(OCR1A+(Meek-1900));} 
//if (Meek==2000 && OCR1A>=LowerLimit && OCR1A<=(UpperLimit-2)){
//  OCR1A=(OCR1A+1);} 
//if (Meek==5000 && OCR1A==(UpperLimit-1)){
//  OCR1A=(0);} 
//if (Meek==3001 && OCR1A==0){
//  OCR1A=(UpperLimit);} 
//if (Meek==4001 && OCR1A<=(UpperLimit-2) && OCR1A>=LowerLimit){
//  OCR1A=(OCR1A-1);} 



//-------------------------------------------------------------------------------


Serial.print("Meek = ");  
Serial.print(Meek);
Serial.print("OCR1A = ");  
Serial.print(OCR1A);
//Serial.print("Percent =");
//Serial.print(Percent); 
//Serial.print("Difference =");
//Serial.print(Difference); 
//Serial.print("Percent11111 =");
//Serial.print(Percent1); 
Serial.print("Up =");
Serial.print(Up); 
Serial.print("DimLevel =");
Serial.print(DimLevel); 
//Serial.print("Upp =");
//Serial.print(Up); 
Serial.print("Down =");
Serial.print(Down); 
//Serial.print("Mistik =");
//Serial.print(Mistik); 
//delay(1000);
Serial.print('\n');

        
delay(InputDelay);}


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
          //analogWrite(port,value);
          Meek=(value);
          break;
        case CMD_ANALOG_READ:
          clearSendBuffer();
          int valueRead = Percent;
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
