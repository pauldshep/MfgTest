/*******************************************************************************
*   File Name: muxPCA9546.c
*  
* Description: Implementation of driver for TI PCA9546A I2C switch with 
*              reset function. 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include "twi_utils.h"
#include "muxPCA9546.h"
#include "timers.h"
 
 
//-----------------------------------------------------------------------------
// Public Global Variables
//-----------------------------------------------------------------------------
 
 
//-----------------------------------------------------------------------------
// External Global Variables
//-----------------------------------------------------------------------------
 
 
//-----------------------------------------------------------------------------
// Private Function Definitions
//-----------------------------------------------------------------------------
 
 
//-----------------------------------------------------------------------------
// Private Data and Definitions
//-----------------------------------------------------------------------------

#define STRINGS_MATCH    0



/*******************************************************************************
*                                   RESET MUX                                  *
********************************************************************************
* Description: Reset the MUX.  Pulling RESET low resets the I2C state machine
*              and all the channels to be deselected.  Pulse duration, /RESET
*              low, is greater than 6 nsec.  Processor port attached to reset
*              is: PD4.
*
*      Global: None
*
*   Arguments: None
*
*      Return: None
*******************************************************************************/
void resetMux(void)
{
    printf("Resetting MUX\r\n");
    
    // reset MUX to power on state
    PORTD &= ~0x10;     // set output low for 5msec
    ms_sleep(5);
    PORTD |= 0x10;      // set output high
    ms_sleep(5);
    
    getMuxConfiguration();
}


/*******************************************************************************
*                                INITIALIZE MUX                                *
********************************************************************************
* Description: Configures MUX
*
*      Global: None
*
*   Arguments: None
*
*      Return: None
*******************************************************************************/
void initMux(void)
{
    // MUX Reset Pin: PD4, Active Low
    DDRD  |= 0x10;      // port direction is output
    PORTD |= 0x10;      // set output high
    
    resetMux();         // reset MUX to power on state
    
    // enable MUX channel for humidity sensor: either 2 or 3
    enableMuxOutputChannel(2);      // enable channel 2
    enableMuxOutputChannel(3);      // enable channel 3
}


/*******************************************************************************
*                             GET MUX CONFIGURATION                            *
********************************************************************************
* Description: Sets MUX output channel state: either on or off.  Each channel
*              consists of a data and clock line.  Uses I2C to
*              control MUX.
*              Note: multiple channels can be enabled
*
*      Global: None
*
*   Arguments: channelID
*
*      Return: None
*******************************************************************************/
uint8_t getMuxConfiguration(void)
{
    uint8_t data_buf[5];
    uint8_t data_len = 1;
    int     ret_code = 0;

    ret_code = twi_read_bytes(MUX_PCA9546_I2C_ADDR, data_len, data_buf);
    printf("  mux config = 0x%X, status = %d\r\n", data_buf[0], ret_code);

    return data_buf[0];
}

 
/*******************************************************************************
*                                 SET MUX OUTPUT                               *
********************************************************************************
* Description: Sets MUX output channel state: either on or off.  Each channel
*              consists of a data and clock line.  Uses I2C to
*              control MUX.
*              Note: multiple channels can be enabled
*  
*      Global: None
*  
*   Arguments: channelID
*  
*      Return: None
*******************************************************************************/
int8_t enableMuxOutputChannel(uint8_t channelID)
{
    uint8_t data_buf[2];
    uint8_t data_len     = 1;
    int     ret_code     = 0;
    uint8_t channel_bit  = 1 << channelID;

    printf("enableMuxOutputChannel(chan = %d)\r\n", channelID);

    data_buf[0]  = 0;
    data_buf[0]  = getMuxConfiguration();
    data_buf[0] |= channel_bit;

    ret_code = twi_write_bytes(MUX_PCA9546_I2C_ADDR, data_len, data_buf);
    printf("  enable mux channel %d, cmd = 0x%X, status = %d\r\n", channelID, data_buf[0], ret_code);
    
    getMuxConfiguration();
    return ret_code;
}


/*******************************************************************************
*                           DISABLE MUX OUTPUT CHANNEL                         *
********************************************************************************
* Description: Disable MUX output Channel.
*  
*      Global: None
*  
*   Arguments: channelID - channel to disable
*  
*      Return: None
*******************************************************************************/
int8_t disableMuxOutputChannel(uint8_t channelID)
{
    uint8_t data_buf[5];
    uint8_t data_len     = 1;
    int     ret_code     = 0;
    uint8_t channel_bit  = 1 << channelID;
    
    printf("disableMuxOutputChannel(chan = %d)\r\n", channelID);

    data_buf[0]   = getMuxConfiguration();
    data_buf[0]  &= ~channel_bit; 

    ret_code = twi_write_bytes(MUX_PCA9546_I2C_ADDR, data_len, data_buf);
    printf("disable mux channel %d, cmd = 0x%X, status = %d\r\n", channelID, data_buf[0], ret_code);

    getMuxConfiguration();
    return ret_code;
}


/*******************************************************************************
*                      CONFIGURE MUX FOR PRESSURE MEASUREMENT                  *
********************************************************************************
* Description: Configures the MUX for pressure measurements with the specified
*              transducer.  This is necessary because both pressure transducers
*              (NPA700) are on I2C address 0x28.  So, to avoid address
*              conflicts the differential pressure transducer is attached to
*              MUX channel 0 and the absolute pressure transducer is attached
*              to channel1.
*
*      Global: None
*
*   Arguments: channelID - pressure channel to enable
*
*      Return: None
*******************************************************************************/
void setMuxPressureMeasurement(uint8_t pressureMeas)
{
    if(pressureMeas == MUX_DIFF_PRESSURE)
    {
       disableMuxOutputChannel(MUX_ABS_PRESSURE);
       enableMuxOutputChannel(MUX_DIFF_PRESSURE);
    }
    else if(pressureMeas == MUX_ABS_PRESSURE) 
    {
        disableMuxOutputChannel(MUX_DIFF_PRESSURE);
        enableMuxOutputChannel(MUX_ABS_PRESSURE);  
    }
    else
    {
        printf("ERROR - invalid pressure meas spec = %d\r\n", pressureMeas);
    }
}


/*******************************************************************************
*                             DISPLAY SERIAL COMMANDS                          *
********************************************************************************
* Description: Display MUX serial command help
*  
*      Global: None
*  
*   Arguments: None
*  
*      Return: None
*******************************************************************************/
void displayMuxSerialCmdHelp(void)
{
    printf("MUX Serial Commands:\r\n");
    printf("  mux cfg   - display mux configuration\r\n");
    printf("  mux ena n - enable channel n\r\n");
    printf("  mux dis n - disable channel n\r\n");
    printf("  mux reset - reset mux\r\n");
    printf("  mux pres n - set mux pressure measurement\r\n");
    return;
}


/*******************************************************************************
*                             PROCESS SERIAL COMMANDS                          *
********************************************************************************
* Description: Process Serial commands.  If we are here the first, mux, part
*              of the command has been processed
*  
*      Global: None
*  
*   Arguments: serCmd
*  
*      Return: None
*******************************************************************************/
void processMuxSerialCmd(char *serCmd)
{
    char *ptr_cmd;
    int   int_val;

    ptr_cmd = strtok(NULL, TOKEN_DELIMINATORS);

    if(strcmp(ptr_cmd, "cfg")        == STRINGS_MATCH)
    {
        getMuxConfiguration();
    }
    else if(strcmp(ptr_cmd, "ena")   == STRINGS_MATCH)
    {
        ptr_cmd = strtok(NULL, TOKEN_DELIMINATORS);
        int_val = atoi(ptr_cmd);
        enableMuxOutputChannel(int_val);
    }
    else if(strcmp(ptr_cmd, "dis")   == STRINGS_MATCH)
    {
        ptr_cmd = strtok(NULL, TOKEN_DELIMINATORS);
        int_val = atoi(ptr_cmd);
        disableMuxOutputChannel(int_val);
    }
    else if(strcmp(ptr_cmd, "reset") == STRINGS_MATCH)
    {
        resetMux();
    }
    else if(strcmp(ptr_cmd, "pres") == STRINGS_MATCH)
    {
        ptr_cmd = strtok(NULL, TOKEN_DELIMINATORS);
        int_val = atoi(ptr_cmd);
        setMuxPressureMeasurement(int_val);
    }
    else
    {
        printf("ERROR - unknown serial command = %s\r\n", serCmd);
    }
}
