/*******************************************************************************
*   File Name: serialPortCmd.h 
*  
* Description: Data and definitions for serialPortCommand.c
*******************************************************************************/
#ifndef __SERIAL_PORT_COMMAND_H__
#define __SERIAL_PORT_COMMAND_H__
  
#define STRINGS_MATCH    0
  
// Global Function Prototypes
void processSerialCommand(char *ptrCmd);
void displaySerialCmdHelp(void);
  
  
#endif  // end __SERIAL_PORT_COMMAND_H__

