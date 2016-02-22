/*******************************************************************************
*   File Name: serialPortCmd.c
*  
* Description: Process serial port command from the user 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "serialPortCmd.h"
#include "HumiditySensor.h"
#include "muxPCA9546.h"
#include "i2c.h"



/*******************************************************************************
*                        PROCESS SERIAL COMMAND STRING                         *
********************************************************************************
* Description: Process a serial port command string from the user.
*  
*   Arguments: ptrCmd - points to serial port command string
*  
*      Return: None
*******************************************************************************/
void processSerialCommand(char *ptrCmd)
{
    char *ptr_cmd = strtok(ptrCmd, TOKEN_DELIM);

    if((strcmp(ptr_cmd, "humid") == STRINGS_MATCH) ||
      ( strcmp(ptr_cmd, "hu")    == STRINGS_MATCH))
    {
        humidityCmds();
    }
    else if(strcmp(ptr_cmd, "mux") == STRINGS_MATCH)

    {
        processMuxSerialCmd(ptrCmd);
    }
    else if(strcmp(ptr_cmd, "i2c") == STRINGS_MATCH)

    {
        processI2cSerialCmd(ptrCmd);
    }
    else
    {
        displaySerialCmdHelp();
    }
 
    printf(">"); 
    return;
}



/*******************************************************************************
*                        DISPLAY SERIAL PORT COMMAND HELP                      *
********************************************************************************
* Description: Displays serial port command help 
*  
*   Arguments: None
*  
*      Return: None
*******************************************************************************/
void displaySerialCmdHelp(void)
{
    displayHumidityMenu();
    displayMuxSerialCmdHelp();
    displayI2cSerialCmdHelp();
}

