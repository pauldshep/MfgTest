/*******************************************************************************
*   File Name: humiditySensor.c
*  
* Description: Support for I2C humidity sensor: TELAIRE ChipCap2 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include "twi_utils.h"
#include "humiditySensor.h"
#include "serialPortCmd.h"
#include "main.h"
#include "led.h"


// local functions
uint8_t decodeStatusBits(uint8_t *ptrBuf);
void    decodeHumidityData(uint8_t *ptrBuf, uint8_t numBytes);
void    decodeTemperatureData(uint8_t *ptrBuf, uint8_t numBytes);


/*******************************************************************************
*                             HUMIDITY SERIAL PORT MENU                        *
********************************************************************************
* Description: Read humidity from sensor
*
*   Arguments: None
*
*      Return: None
*******************************************************************************/
void displayHumidityMenu(void)
{
    printf("Humidity Sensor Menu:\r\n");
    printf("  humid read  - read sensor humidity\r\n");
    printf("  humid scan  - scans the available I2c addresses for a sensor\r\n");
    printf("  humid mr    - issues a sleep mode measurement request\r\n");
    printf("  humid update - issues a meas request then reads the sensor\r\n");
    
    return;
}


/*******************************************************************************
*                             HUMIDITY SERIAL COMMANDS                         *
********************************************************************************
* Description: Parse and execute humidity sensor serial port commands
*
*   Arguments: None
*
*      Return: 0 if no error is detected, error code otherwise
*******************************************************************************/
uint8_t humidityCmds(void)
{
    uint8_t sensor_status;
    char *cmd_token;
    
    cmd_token = strtok(NULL, " ");
    
    if((strcmp(cmd_token, "read") == STRINGS_MATCH) ||
    (strcmp(cmd_token, "rd")   == STRINGS_MATCH))
    {
        readSensor(&sensor_status);
    }
    else if(strcmp(cmd_token, "scan") == STRINGS_MATCH)
    {
        scanTWI();
    }
    else if(strcmp(cmd_token, "mr") == STRINGS_MATCH)
    {
        measurementRequest();
    }
    else if((strcmp(cmd_token, "update") == STRINGS_MATCH) ||
           ( strcmp(cmd_token, "up")     == STRINGS_MATCH))
    {
        measurementUpdate();
    }
    else
    {
        // unknown humidity command
        printf("ERROR - unknown humidity command = %s\r\n", cmd_token);
        return 1;
    }
    
    return 0;
}


/*******************************************************************************
*                              MEASUREMENT REQUEST                             *
********************************************************************************
* Description: Sends measurement request to humidity sensor.  This is needed
*              because this is sleep mode sensor.  The measurement request
*              wakes the sensor up causes it to start a measurement.  When the
*              measurement is complete data read from the sensor will have a
*              status of 0.  After the data is read the status changes to 1.
*
*   Arguments: None
*
*      Return: 0 if no error is detected
*******************************************************************************/
uint8_t measurementRequest(void)
{
    uint8_t data_buf[5];
    uint8_t data_len = 0;
    int     ret_code = 0;
    
    printf("Sending Measurement Request: addr = 0x%X\r\n", TWI_HUMIDITY_SENSOR_ADDR);
    
    // send measurement request
    ret_code = twi_write_bytes(TWI_HUMIDITY_SENSOR_ADDR, data_len, data_buf);
    
    return ret_code;
}


/*******************************************************************************
*                        READ TEMPERATURE/HUMIDITY SENSOR                      *
********************************************************************************
* Description: Read ChipCap2 sensor humidity and temperature values.  These are
*              decoded and displayed.
*  
*   Arguments: ptrStatus - save status of the read operation here
*  
*      Return: 0 if no error is detected
*******************************************************************************/
uint8_t readSensor(uint8_t *ptrStatus)
{
    uint8_t i;
    uint8_t data_buf[5];
    uint8_t data_len = 4;
    int     ret_code = 0;
    
    data_buf[0] = 0;
    
    setLED(1);
    printf("Reading Humidity Sensor, addr = 0x%X\r\n", TWI_HUMIDITY_SENSOR_ADDR);
    
    // read humidity sensor
    ret_code = twi_read_bytes(TWI_HUMIDITY_SENSOR_ADDR, data_len, data_buf);
    
    printf("  return code = %d\r\n", ret_code);
    printf("  data: ");
    for(i = 0; i <data_len; i++)
    {
        printf(" 0x%X(%d)", data_buf[i], data_buf[i]);
    }
    printf("\r\n");
    setLED(0);
    *ptrStatus = decodeStatusBits(data_buf);
    decodeHumidityData(data_buf, data_len);
    decodeTemperatureData(data_buf, data_len);
    
    return ret_code;
}


/*******************************************************************************
*                              MEASUREMENT UPDATE                              *
********************************************************************************
* Description: Combines a measurement request with a read data.  
*
*   Arguments: None
*
*      Return: 0 if no error is detected
*******************************************************************************/
uint8_t measurementUpdate(void)
{
    uint8_t ret_code     = 0;
    uint8_t read_retries = 0;
    uint8_t read_status  = HUMIDITY_SENSOR_STALE_DATA;
    
    printf("\r\nUpdating and Reading Sensor\r\n");
    
    // send measurement request
    measurementRequest();
    
    // read until we get good data or time out
    while((read_status == HUMIDITY_SENSOR_STALE_DATA) && 
         (read_retries < NUM_SENSOR_READ_RETRIES))
    {
        printf("\r\n");
        readSensor(&read_status);
        read_retries++;
    }
    
        
    return ret_code;
}


/*******************************************************************************
*                            SCAN TWO WIRE INTERFACE                           *
********************************************************************************
* Description: Scan the two wire interface for the presence of a humidity 
*              sensor.  Use this function to determine the TWI address of the
*              sensor.  Addresses are 7-bits between 1 and 0x7F(127) with
*              the following special addresses:
*              0000000 - general call
*              0000001 - CBUS Addresses
*  
*
*   Arguments: None
*
*      Return: 0 if no error is detected
*******************************************************************************/
uint8_t scanTWI(void)
{
    uint8_t data_buf[10];
    uint8_t twi_addr  = 0x22;
    uint8_t data_len  = 1;
    uint8_t dev_addr  = 0;
    uint8_t num_found = 0;
    int     ret_code  = 0;

    data_buf[0] = 0;
    
    setLED(1);
    printf("Scanning for I2C Devices on the Bus:\r\n");
    
    for(twi_addr = 0; twi_addr <= 0x7F; twi_addr++)
    {
        printf("  reading device at address = 0x%X\r\n", twi_addr);
        
        ret_code = twi_read_bytes(twi_addr, data_len, data_buf);

        if(ret_code >= 0)
        {
           dev_addr = twi_addr;
           num_found++; 
           printf("  num found = %d\r\n", num_found);
           printf("  ret_code = %d\r\n", ret_code);
           printf("  addr = 0x%X\r\n", twi_addr);
        }
    }
    
    printf("TWI Address Found = 0x%X\r\n", dev_addr);
    printf("Number Found = %d\r\n", num_found);
    setLED(0);
    
    return 0;
}


/******************************************************************************
*                             DECODE STATUS BITS                              *
*******************************************************************************
* Description: Decodes and displays status bits from the ChipCap2 sensor read
*              data.  Possible values:
*                00b - valid data, data has not been read since last update
*                01b - stale data, data has been read and not updated
*                10b - ChipCap2 is in command mode
*                11b - not used
*
*   Arguments: ptrBuf - pointer to buffer containing the data to decode
*
*      Return: decoded status bits
******************************************************************************/
uint8_t decodeStatusBits(uint8_t *ptrBuf)
{
    uint8_t status_bits = 0;
    
    printf("Decoding Status Information:\r\n");
    
    // get status information from the first byte
    status_bits = (ptrBuf[0] & 0xC0) >> 6;
    
    if(status_bits == HUMIDITY_SENSOR_VALID_DATA)
        printf("  status = 0x%X (VALID DATA)\r\n", status_bits);
    else if(status_bits == HUMIDITY_SENSOR_STALE_DATA)
        printf("  status = 0x%X (STALE DATA)\r\n", status_bits);
    else
       printf("  status = 0x%X\r\n", status_bits); 
        
    return status_bits;
}


/******************************************************************************
*                            DECODE HUMIDITY DATA                             *
*******************************************************************************
* Description: Decodes and displays humidity data from the ChipCap2 sensor
*
*   Arguments: ptrBuf   - pointer to buffer containing the data to decode
*              numBytes - number of bytes of data to decode
*
*      Return: None
******************************************************************************/
void decodeHumidityData(uint8_t *ptrBuf, uint8_t numBytes)
{
    int32_t rh_low;
    int32_t rh_high;
    int32_t humidity_value = 0;
    
    printf("Decoding Humidity Data:\r\n");
    
    // get humidity value from the first two bytes
    rh_high = (ptrBuf[0] & 0x3F) * 256;
    rh_low  =  ptrBuf[1];
    
    printf("  rh high = %ld\r\n", rh_high);
    printf("  rh low  = %ld\r\n", rh_low);
    
    humidity_value  = ((rh_high + rh_low) * 100) / 16384;
    printf("  humidity = %ld PCT RH\r\n", humidity_value);
    
    return;    
}  


/******************************************************************************
*                           DECODE TEMPERATURE DATA                           *
*******************************************************************************
* Description: Decodes and displays temperature data from the ChipCap2 sensor
*
*   Arguments: ptrBuf   - pointer to buffer containing the data to decode
*              numBytes - number of bytes of data to decode
*
*      Return:
******************************************************************************/
void decodeTemperatureData(uint8_t *ptrBuf, uint8_t numBytes)
{
    int32_t temp_high = 0;
    int32_t temp_low  = 0;
    int32_t buf_value;
    int32_t temperature_value = 0;
    
    
    printf("Decoding Temperature Data:\r\n");
    
    // return if there is no temperature data
    if(numBytes < 3)
    {
        printf("  no temperature data to decode\r\n");
        return; 
    }        
    
    // get the first byte of temperature data
    buf_value = ptrBuf[2];
    temp_high = (((buf_value * 64) * 165) / 16384) - 40;
    printf("  temp_high = %ld\r\n", temp_high);
    
    
    // get the 2nd byte of temperature data if there is any
    if(numBytes == 4)
    {
        buf_value = (ptrBuf[3] & 0xFC) >> 2;
        temp_low = ((buf_value / 4) * 165) / 16384;
        printf("  temp_low = %ld\r\n", temp_low);
    }
    
    temperature_value = temp_high + temp_low;
    printf("  temperature = %ld C\r\n", temperature_value);
 
    return;   
}    


