/******************************************************************************
*   File Name: main.c
*
* Description: Tests Telaire(Amphenol) ChipCap2 Humidity/Temperature Sensor.
*              This is a sleep mode sensor.
*
* Created: 1/25/2016 4:33:53 PM
* Author : Paul Shepherd
******************************************************************************/ 
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include "main.h"
#include <util/delay.h>
#include "HumiditySensor.h"
#include "serialPortCmd.h"
#include "led.h"
#include "twi_utils.h"
#include "timers.h"
#include "muxPCA9546.h"


// global data
volatile uint16_t ms_count;

// serial port input circular buffer
uint8_t           data;
uint8_t           rxBuf[RX_BUFFER_SIZE];
uint8_t           ptrRxBufStart;
uint8_t           ptrRxBufEnd;

// serial port command
uint8_t           cmdBuf[CMD_BUFFER_SIZE];
uint8_t           ptrCmdBuf;

// printf support
FILE uartstr = FDEV_SETUP_STREAM(UartPutChar, UartGetChar, _FDEV_SETUP_RW);



/******************************************************************************
*                                    MAIN                                     *
*******************************************************************************
* Description: Firmware Project start.  This project:
*              (1) blinks the Mavric LED every .5 seconds
*
*   Arguments: None
*
*      Return: None
******************************************************************************/
int main(void)
{    
    ptrRxBufStart  = 0;
    ptrRxBufEnd    = 0;
    ptrCmdBuf      = 0;
    rxBuf[0]       = '\0';
        
    init_timers();
    init_usart0();
    init_twi();
    
    // enable printf
    stdout=stdin=&uartstr;
      
    // enable interrupts
    sei();
    
    initMux();

    DDRB = 0x01;    // enable PORTB 1 as an output (LED)
    
    printf("\r\n\r\nAerosole Devices Manufacturing Test Program\r\n");
    displaySerialCmdHelp();
    printf(">");

    while (1)
    {
        // blink LED if time has elapsed
        if(ms_count > 250)
        {
            ms_count = 0;
            toggleLED(); 
        } 
        
        getCommandData();           
    }
    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
////////////////////////////// INTERRUPT HANDLERS /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
*                               USART0 RXC ISR                                *
*******************************************************************************
* Description: UART 0 receive complete interrupt service routine
*
*      Global: uint8_t rx_buf[RX_BUFFER_SIZE] - receive character buffer
******************************************************************************/
ISR(USART0_RX_vect)
{
    // read received character
    data = UDR0;
    
    // echo the received character back
    UDR0 = data;
    
    // copy received character to circular buffer
    rxBuf[ptrRxBufEnd++] = data;
    if(ptrRxBufEnd == RX_BUFFER_SIZE)
    {
        ptrRxBufEnd = 0;
    }
    rxBuf[ptrRxBufEnd] = '\0';
}



///////////////////////////////////////////////////////////////////////////////
//////////////////////////// PUBLIC MEMBER FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
*                            GET SERIAL COMMAND DATA                          *
*******************************************************************************
* Description: Moves any new serial command data from serial data buffer to 
*              command buffer then checks for end of command ("\n").  If end
*              of command is found then the command is processed.
*
*              Supported special characters:
*                '\n'   10(0x0A) line feed
*                '\r'   13(0x0D) carriage return - end of command
*                
*
*      Global: cmdBuf[CMD_BUFFER_SIZE];
*              ptrCmdBuf
*              rxBuf[RX_BUFFER_SIZE] - circular buffer for RX data
*              ptrRxBufStart         - data extraction point in RX buffer
*              ptrRxBufEnd           - next insert point in RX buffer
*
*   Arguments: None
*
*      Return: None
******************************************************************************/
void getCommandData(void)
{   
    uint8_t ser_data;
    
    while(ptrRxBufStart != ptrRxBufEnd)
    {
        // get data byte
        ser_data = rxBuf[ptrRxBufStart++];
        if(ptrRxBufStart == RX_BUFFER_SIZE)
            ptrRxBufStart = 0;
            
        // process special characters
        if(ser_data == '\r')    // carriage return
        {
            // end of command
            printf("\r\n>");
            cmdBuf[ptrCmdBuf] = '\0';
            processSerialCommand((char *)cmdBuf);
            ptrCmdBuf = 0;
            cmdBuf[0] = '\0';
        }
        else
        { 
        // move data to command buffer
        cmdBuf[ptrCmdBuf++] = ser_data;
        if(ptrCmdBuf == CMD_BUFFER_SIZE)
            ptrCmdBuf = 0;
            
        // terminate command string
        cmdBuf[ptrCmdBuf] = '\0';
        }               
        toggleLED();
    }     
}


/******************************************************************************
*                             WRITE TO SERIAL PORT                            *
*******************************************************************************
* Description: Writes specified string to serial port
*
*      Global: None
*
*   Arguments: msg - string to write to serial port
*
*      Return: None
******************************************************************************/
void writeToSerialPort(char *msg)
{
    for(uint8_t i = 0; i < strlen(msg); i++)
    {
       transmit_usart0(msg[i]);  
    }
} 
   


/******************************************************************************
*                              INITIALIZE UART                                *
*******************************************************************************
* Description: Initialize USART0: 9600, 8, N, 1 with no flow control.

*              UBRR0L - USART0 baud rate register low
*              UBRR0H - USART0 baud rate register high
*              UCSR0A - USART0 control and status register A. p188
*              UCSR0B - USART0 control and status register B, p189
*              UCSR0C - USART0 control and status register C
*              DDRE   - Data Direction Register Port E
*
*              Processor Parameters:
*              clock - 16MHz
*              RXD0/(PDI)  - PE0 - Pen  2
*              TXD0/(PDO)  - PE1 - Pen  3 - 
*              XCK0/(AIN0) - PE2 - Pen  4
*              OC0         - PB4 - Pen 14
*
*   Arguments: None
*
*      Return: None
******************************************************************************/
void init_usart0(void)
{
    // Initialize the baud rate registers for 9600
    UBRR0H = (UBRR) >> 8; 
    UBRR0L = (UBRR) &  0xFF;
    
    // Enable the receiver, transmitter, and receive complete interrupt
    UCSR0B = (1 << RXEN0)  | (1 << TXEN0) | (1 << RXCIE0);
     
    // Set the USART data size to 8b, no parity, one stop bit
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
    
    // Set the TX line (PE1) to be an output.      
    DDRE  |= (1 << 1);                      
}


/******************************************************************************
*                               TRANSMIT UART                                 *
*******************************************************************************
* Description: Blocking single byte transmit on USART0
*              UCSR0A - UART0 Control and Status Register A, p188
*              UDR0   - UART0 I/O Data Register, p188
*
*   Arguments: data - data to transmit
*
*      Return: None
******************************************************************************/
void transmit_usart0(uint8_t data)
{   
    // wait until the UDRE0 flag (data register empty) in the UCSRA0 register 
    // is set.
    while(!(UCSR0A & (1 << UDRE0)))
    ; 
    
    // Start next transmission by writing to UDR0
    UDR0 = data;
    //_delay_ms(2);                    
}


/******************************************************************************
*                              UART HAS RX DATA                               *
*******************************************************************************
* Description: Checks the USART0 Rx complete status flag 
*
*   Arguments: None
*
*      Return: true if there is RX data in the buffer
******************************************************************************/
uint8_t usart0_has_rx_data(void)
{
    return (UCSR0A & (1 << RXC0)) != 0;
}


/******************************************************************************
*                               RECEIVE UART                                  *
*******************************************************************************
* Description: Reads data from the USART receive buffer 
*              UDR0 - UART0 Data Register
*
*   Arguments: None
*
*      Return: character received (8bits)
******************************************************************************/
uint8_t receive_usart0(void)
{
    uint8_t ret_val = UDR0; // Read data register
    return ret_val;         // Return read value
}


/******************************************************************************
*                            UART PUT CHARACTER                               *
*******************************************************************************
* Description: Implementation of putc so that printf with work.  UART0 is used 
*              for output.
*
*   Arguments: c      - char to send to UART0
*              stream - 
*
*      Return: 0
******************************************************************************/
int UartPutChar(char c, FILE* stream) 
{
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
    return 0;
}


/******************************************************************************
*                            UART GET CHARACTER                               *
*******************************************************************************
* Description: Implementation of getc so that printf with work.  UART0 is used
*              for input.
*
*   Arguments: stream -
*
*      Return: character read from UART0
******************************************************************************/
int UartGetChar(FILE* stream) 
{
    char c;
    while(!(UCSR0A & (1 << RXC0)));
    c = UDR0;
    return c;
}


///////////////////////////////////////////////////////////////////////////////
/////////////////////////// PRIVATE MEMBER FUNCTIONS //////////////////////////
///////////////////////////////////////////////////////////////////////////////
