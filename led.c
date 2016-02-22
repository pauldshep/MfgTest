/*******************************************************************************
*   File Name: led.c 
*  
* Description: Mavrick controller LED functions.  The Mavric board has one 
*              orange LED at PB0. 
*******************************************************************************/
#include "led.h"
#include <avr/io.h>


/******************************************************************************
*                                  TOGGLE LED                                 *
*******************************************************************************
* Description: Toggles LED on Mavric board on and off
*
*      Global: None
*
*   Arguments: None
*
*      Return: None
******************************************************************************/
void toggleLED(void)
{
    PORTB ^= 0x01;
}


/******************************************************************************
*                                  TOGGLE LED                                 *
*******************************************************************************
* Description: Sets LED on Mavric controll to on or off
*
*      Global: None
*
*   Arguments: ledState - either on or off
*
*      Return: None
******************************************************************************/
void setLED(uint8_t ledState)
{
    if(ledState)
    {
        PORTB |= 0x01;
    }
    else
    {
        PORTB &= ~0x01;
    } 
}


