/*******************************************************************************
*   File Name: humiditySensor.h
*  
* Description: Data and definitions for humiditySensor.c
*******************************************************************************/
#ifndef __HUMIDITY_SENSOR_H__
#define __HUMIDITY_SENSOR_H__
  

#define HUMID_SENSOR_ADDR       0x1E    // 7-bit sensor address
#define HUMID_READ_CMD          0x01    // read bit  = 1
#define HUMID_WRITE_CMD         0x00    // write bit = 0

#define TWI_START_CONDITION_TRANSMITTED     0x08
#define TWI_HUMIDITY_SENSOR_ADDR            0x28
#define TOKEN_DELIM                         " "

#define NUM_SENSOR_READ_RETRIES             10

// sensor data states
#define HUMIDITY_SENSOR_VALID_DATA      0   // measurement data has not been read
#define HUMIDITY_SENSOR_STALE_DATA      1   // measurement data has been read
  
// public function definitions   
void    displayHumidityMenu(void);
uint8_t humidityCmds(void);
uint8_t measurementRequest(void);
uint8_t readSensor(uint8_t *ptrStatus); 
uint8_t scanTWI(void);
uint8_t measurementUpdate(void);


  
#endif  // end __HUMIDITY_SENSOR_H__

