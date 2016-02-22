
/*******************************************************************************
*   File Name: timers.c
*  
* Description: Timer initialization and control
*******************************************************************************/
#include "defines.h"
#include <avr/io.h>
#include <avr/interrupt.h> 


volatile int32_t  ms_motorStepCount;
volatile uint16_t ms_count=0; 
volatile uint16_t ms_MotorCount=0; 
volatile uint16_t ms_SleepCount=0; 
volatile uint16_t ms_ADCcount=0; 
volatile uint16_t ms_switchCount=0; 
volatile uint16_t ms_switchReleasedCount=0; 
volatile uint16_t ms_twiCount=0; 
volatile uint16_t ms_PressureCount=0;
volatile uint32_t ms_log_count;
volatile uint32_t ms_injectionCount;
volatile uint32_t ms_led_count;

// local functions
void init_timer0(void);
void init_timer1_FastPWM(char a, char b, char c);
void init_timer3_FastPWM(char a, char b, char c);


//// millisecond counter interrupt vector 
//SIGNAL(TIMER0_COMP_vect)
//{
  //ms_count+=8;
  //ms_MotorCount+=8;
  //ms_SleepCount+=8;
  //ms_ADCcount+=8; 
  //ms_motorStepCount+=8;
  //ms_log_count+=8;
  //ms_injectionCount +=8;
  //ms_switchCount += 8;
  //ms_twiCount +=8;
  //ms_switchReleasedCount +=8;
  //ms_PressureCount += 8;
  //ms_led_count++;
//}

/******************************************************************************
*                     TIMER 0 INTERRUPT VECTOR ATMEGA2561                     *
*******************************************************************************
* Description: Timer 0 interrupt vector.  Timer 0 interrupts every 8 msecs.
******************************************************************************/
SIGNAL(TIMER0_COMPA_vect)
{
    ms_count               +=8;
    ms_MotorCount          +=8;
    ms_SleepCount          +=8;
    ms_ADCcount            +=8;
    ms_motorStepCount      +=8;
    ms_log_count           +=8;
    ms_injectionCount      +=8;
    ms_switchCount         +=8;
    ms_twiCount            +=8;
    ms_switchReleasedCount +=8;
    ms_PressureCount       +=8;
    ms_led_count           +=8;
    //ms_flow_count          +=8;
    //ms_flow_ctrl_count     +=8;
}


void init_timers()
{
	ms_count = ms_SleepCount = ms_ADCcount = ms_switchCount = 0;
	ms_switchReleasedCount = ms_injectionCount = 0;
	init_timer0();
	//	ms_MotorCount = 0;
	init_timer1_FastPWM('a', 'b', 'c');
	init_timer3_FastPWM('a', 'b', 'c');
}

// ms_sleep() - delay for specified number of milliseconds
void ms_sleep(uint16_t ms)  //---NOTE RESOLUTIION is 8 milliseconds!
{
	TCNT0  = 0;
	ms_SleepCount = 0;
	while (ms_SleepCount < ms)
	;
}



// initialize timer 0 to generate an interrupt every eight milliseconds.
// Used for timing of motor control, loging, & for  ADC
//void init_timer0(void)
//{
  ///* 
   //*   CTC mode -- Clear Timer on Compare match
   //* Initialize timer0 to generate an output compare interrupt, and
   //* set the output compare register so that we get that interrupt
   //* every 8 millisecond.
   //*/
  //TIFR  |= _BV(OCIE0);
  //TCCR0  = _BV(WGM01)|_BV(CS02)| _BV(CS01)| _BV(CS00); /* CTC, prescale = 1024-- frequency of 15.625 KHz, i.e period of 64 micro seconds */  
  //TCNT0  = 0;
  //TIMSK |= _BV(OCIE0);    /* enable output compare interrupt */
  //OCR0   =  (int8_t) ((F_CPU / 1000UL) / 128UL) ;          /* match in 8 ms count to 125 at 15.625 KHz */
//
//}


/******************************************************************************
*                              INITIALIZE TIMER 0                             *
*******************************************************************************
* Description: Initialize ATmega2561 Timer 0 to generate an interrupt every
*              eight milliseconds. Used for timing of motor control, logging, 
*              and for ADC.
*
*              CTC mode -- Clear Timer on Compare match
*              Initialize timer0 to generate an output compare interrupt, and
*              set the output compare register so that we get that interrupt
*              every 8 millisecond.
*
*   ATmega2561
*   Registers:
*
*   Arguments: None
*
*      Return: None
******************************************************************************/
void init_timer0(void)
{    
    TIFR0  |= _BV(OCF0A);     // output compare flag timer 0 A match
    TCCR0B  = _BV(WGM02) |    // waveform generation mode
    _BV(CS02)  |    // Clock select - CLKtos
    _BV(CS00)  ;    // Clock select - CLKtos - CTC, prescale = 1024-- frequency of 15.625 KHz, i.e period of 64 micro seconds
    TCNT0   = 0;              // Remove Compare Match
    TIMSK0 |= _BV(OCIE0A);    // Enable Output Compare Match A Interrupt
    OCR0A   =  (int8_t) ((F_CPU / 1000UL) / 128UL) ;       // match in 8 ms count to 125 at 15.625 KHz
    }


/* 8 bit fast PWM mode at 16MHz/256/64 = 976 Hz . No inturrupts. 
   Wave form output on pin OCR1A, and/or OCR1B, 
   and/or OCR1AC i.e. PB5, PB6, PB7 
   PB7 is shared with OCR1AC, and OCR2 */
void init_timer1_FastPWM(char a, char b, char c)
{
	
	if (a == 'a' || a == 'A')
	{
		TCCR1A |= _BV(COM1A1) ;
		DDRB |= _BV(PB5);
		OCR1A = 0x0080;   // only using 8 bits
	}
	if (b == 'b' || b == 'B')
	{
		TCCR1A |=  _BV(COM1B1) ;
		DDRB |= _BV(PB6);
		OCR1BL = 0x80;   // only using 8 bits

	}
	if  (c == 'c' || c == 'C') 
	{
		TCCR1A |= _BV(COM1C1);
		DDRB |= _BV(PB7);                  //Conflicts with OCR2  !
		OCR1CL = 0x20;   // only using 8 bits     
	} 
	TCCR1A |=  _BV(WGM10) ;
	TCCR1B = _BV(WGM12)  | _BV(CS11) | _BV(CS10) ;    /* 8 bit fast PWM  prescale 64   --- 976.625 Hz = 16Mz/ (256 *64)*/
//	TCCR1B = _BV(WGM12)  | _BV(CS11) ;    /* 8 bit fast PWM  prescale 8   ---> 7812.5 KHz = 16Mz/ (256 * 8)*/
}



/*******************************************************************************
*                         INITIALIZE TIMER 3 FAST PWM                          *
********************************************************************************
* Description: Initialize Timer/Counter 3.  8 bit fast PWM mode at
*              16MHz/256/64 = 977 Hz.  No interrupts.  Wave form output on pins:
*                E3(OC3A) - PWM Output A - Initiator temperature
*                E4(OC3B) - PWM Output B - Conditioner temperature
*                E5(OC3C) - PWM Output C - Sample Pump
*
*              Registers:
*                TCCR3A - Timer/Counter3 Control Register A, p132
*                DDRE   - Data Direction Register E, p87
*                OCR1A  - Output Compare Register 1 A
*                OCR3BL - Output Compare Register 3 B Low
*                OCR3CL - Output Compare Register 3 C Low
*                TCCR3B - Timer/Counter3 Control Register B, p135
*                TCCR3C - Timer/Counter3 Control Register C, p136
*  
*   Arguments: a - enable channel a initialization
*              b - enable channel b initialization
*              c - enable channel c initialization
*  
*      Return: None
*******************************************************************************/
void init_timer3_FastPWM(char a, char b, char c)
{
	if(a == 'a' || a == 'A')        // initialize output A
	{
		TCCR3A |= _BV(COM3A1);      // Compare Output Mode for Channel A
		DDRE   |= _BV(PE3);         // PE3 data direction is output
		OCR1A   = 0x0080;           // compared with counter value to generate an interrupt 
	}

	if(b == 'b' || b == 'B')        // initialize output B
	{
		TCCR3A |= _BV(COM3B1);      // Compare Output Mode for Channel B
		DDRE   |= _BV(PE4);         // PE4 data direction is output
		OCR3BL  = 0x80;             // compared with counter value to generate an interrupt 
	}

	if(c == 'c' || c == 'C')        // initialize output C
	{
		TCCR3A |= _BV(COM3C1);      // Compare Output Mode for Channel C
		DDRE   |= _BV(PE5);         // PE4 data direction is output
		OCR3CL  = 0x20;             // compared with counter value to generate 
                                    // an interrupt or to generate a wave form 
                                    // output on the OCnx pin: PE5 (OC3C).
                                    // Multiply this with the prescaler to get 
                                    // freq divisor: 16Mhz/(64 * 32) = 7813 Hz.
                                    // DC motor requires frequency from 6 to 20 KHz
                                    // todo: Conflicts with OCR2?    
	} 

	TCCR3A |= _BV(WGM10);   // PWM, Phase Correct, 8-bit, todo: should this be WGM30?
                            
    // 8 bit fast PWM  prescale 64   --- 976.625 Hz = 16Mz / (256 * 64)
	TCCR3B  = _BV(WGM32) |  // waveform gen mode - 8 bit fast PWM
              _BV(CS31)  |  // clock select
              _BV(CS30);    // clock select - clkIO/64 (From prescaler)
                            
//	TCCR3B = _BV(WGM32)  | _BV(CS31) ;    /* 8 bit fast PWM  prescale 8   ---> 7812.5 KHz = 16Mz/ (256 * 8)*/
}
