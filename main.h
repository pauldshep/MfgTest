/******************************************************************************
* main.h
*
* Created: 1/25/2016 4:38:15 PM
*  Author: Paul Shepherd
******************************************************************************/ 
#ifndef MAIN_H_
#define MAIN_H_

#define F_CPU           16000000                 // 16 MHz clock frequency
#define UART_BAUD       9600                     // serial port baud rate   
#define UBRR            F_CPU/16/UART_BAUD-1     // baud rate register value  
#define BAUD_PRE        (((F_CPU / (UART_BAUD * 16UL))) - 1  

#define RX_BUFFER_SIZE   32
#define CMD_BUFFER_SIZE 128
          


// function definitions
//void    init_timer(void);
void    init_usart0(void);
void    transmit_usart0(uint8_t byte);
uint8_t usart0_has_rx_data(void);
uint8_t receive_usart0(void);
void    getCommandData(void);
//void    processCommand(char *cmdBuf);
void    writeToSerialPort(char *msg);
void    toggleLED(void);
int     UartPutChar(char c, FILE* stream);
int     UartGetChar(FILE* stream);

#endif /* MAIN_H_ */