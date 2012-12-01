//Author: Michiel van der Coelen
//contact: author with "." for spaces @gmail.com
//copyright: Do whatever you want with it, this is not rocket science
//date: 2010-10-27
//tabsize: 2
//avr-gcc compile options: -DF_CPU=$(F_CPU) -std=c99 -Wall -Os -mmcu=atmega88

//www.pengurobotics.com
//HP linear encoder C6487-80014-A implementation
/* 
Phase A
 __      __      __
/  \    /  \    /  \
    \__/    \__/    \__/

Phase B (+90 degrees)
   __      __      __
  /  \    /  \    /  \
_/    \__/    \__/    \_
    

state = 0x##
          ||
          ||_ Phase A positive
          |__ Phase B positive

State loop (forward):
  0x00
  0x01
  0x11
  0x10          

MCU = atmega88
Lfuse = 74
Hfuse = D9
connections:
  LED at PB0
  0.5*VCC(encoder) at AIN0 (PD6)
  encoder Phase A at ADC0 (PC0)
  encoder Phase B at ADC1 (PC1)
*/

#include <avr/io.h>

#define SET(x,y) (x|=(1<<y))
#define CLR(x,y) (x&=(~(1<<y)))
#define CHK(x,y) (x&(1<<y)) 
#define TOG(x,y) (x^=(1<<y))
#define Delay(x) (_delay_us(x))

//-----------------------------------------------------------------------------
//Global Variables
//-----------------------------------------------------------------------------
volatile uint8_t state = 0x00;
volatile uint8_t previousState=0x00;
volatile int16_t pos=0;
//-----------------------------------------------------------------------------
//FUNCTIONS
//-----------------------------------------------------------------------------

//setup the registers for the analog comparator:
//(ACME)   the ADC multiplexer is turned on to allow the comparator to use one 
//        of the ADC ports for the negative input.
void initializeAC(){
  ADCSRB    |=  (1<<ACME);
  ACSR      |=  (0<<ACIE) |(1<<ACIS1);
  DIDR1     |=  (1<<AIN1D)|(1<<AIN0D);
}

//this function sets the ADC/AC multiplexer to index
void setMux(uint8_t index){
  //index = 0..7, 15, 14 (ADC0..7; 1,1V; 0V) 
  ADMUX &= 0xF0; //clear mux
  if(index<0x07 || index== 0x0E || index==0x0F) ADMUX |= index;
  //else leave mux at 0
}

//-----------------------------------------------------------------------------
//MAIN
//-----------------------------------------------------------------------------
int main() {
//AC
  initializeAC();
  setMux(0);
//-----------------------------------------------------------------------------
//MAIN LOOP
//-----------------------------------------------------------------------------
while(1){
  //First we fill "state" by checking the comparator with ADC0 and ADC1
  //select ADC0 (Phase A) as input for the comparator
  //we have to wait 2 clockcycles before the AC uses the newly selected PORT
  setMux(0);
  
  previousState = state;
  CLR(state,0); 
  
  //if Phase A is positive:
  if(CHK(ACSR,ACO)) {
    //select ADC1 (Phase B)
    setMux(1);
    //we might as well update a LED while we wait
    CLR(state,0); 
    SET(state,0); //==> 0x#1
    
    if(CHK(ACSR,ACO))  SET(state,4); //==> 0x11
    else CLR(state,4); //==> 0x01
  }  else {
    setMux(1);
    CLR(state,0);
    CLR(state,0); //==> 0x#0
    if(CHK(ACSR,ACO))  SET(state,4); //==> 0x10
    else CLR(state,4); //==> 0x00
  }
  /*this next part determines the direction by comparing previousState and
    state. In this form I tried to keep it readable.
    It should be optimized for speed by using if-else statements and 
    changing the way state and previousState are stored.
  */
  if (state != previousState){
    //forward = 01 -> 11 -> 10 -> 00
    switch(state){
      case 0x00:
        if (previousState == 0x10) pos++;
        if (previousState == 0x01) pos--;
        break;
      case 0x11:
        if (previousState == 0x01) pos++;
        if (previousState == 0x10) pos--;
        break;
      case 0x10:
        if (previousState == 0x11) pos++;
        if (previousState == 0x00) pos--;
        break;
      case 0x01:
        if (previousState == 0x00) pos++;
        if (previousState == 0x11) pos--;
        break;
    }
  }
}//main loop end
}//main end

