/*******************************************************************************
*   File Name: twi_utils.h
*  
* Description: data and definitions for twi_utils.c
*******************************************************************************/
#ifndef __twi_h__
#define __twi_h__

#include <inttypes.h>
//#include "timers.h"

/*
 * define some handy TWI constants
 */
#define TWCR_START    (_BV(TWINT)|_BV(TWSTA)|_BV(TWEN))
#define TWI_MASTER_TX (_BV(TWINT)|_BV(TWEN))
#define TWI_TIMEOUT   180  // CHANGED BY gl TO 180 FROM 1000
#define TWI_ACK       1
#define TWI_NACK      0
#define TWI_VERBOSE   1   // GSL
#define TWI_QUIET     0   // GSL
#define TWI_MAX_ITER  250


void    init_twi(void);
int8_t  twi_stop();
void    twi_error(const char * message, uint8_t cr, uint8_t status);
int8_t  twi_start(uint8_t expected_status);
int     twi_write_bytes(uint8_t twi_addr, int len, uint8_t *buf);
int 	twi_write_bytes_with_WP(uint8_t twi_addr, int16_t writePointer,  int len, uint8_t *buf);
int     twi_read_bytes(uint8_t twi_addr, int len, uint8_t *buf);
int     twi_read_bytes_wP(uint8_t twi_addr, uint8_t data_addr, int len, uint8_t *buff);
int     twi_read_bytes_wP2(uint8_t twi_addr, uint16_t write_pointer_addr, int len, uint8_t *buf);
void    parseI2C_commands(char *cmd);


#endif
