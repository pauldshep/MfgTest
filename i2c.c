#include <util/twi.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <inttypes.h>
#include "led.h"
#include "serialPortCmd.h"
#include "i2c.h"

uint8_t i2cscan(void);

#define TOKEN_DELIMINATORS (" ")

const char s_i2c_start_error[]   PROGMEM = "I2C START CONDITION ERROR";
const char s_i2c_sla_w_error[]   PROGMEM = "I2C SLAVE ADDRESS ERROR";
const char s_i2c_data_tx_error[] PROGMEM = "I2C DATA TX ERROR";
const char s_i2c_data_rx_error[] PROGMEM = "I2C DATA RX ERROR";
const char s_i2c_timeout[]       PROGMEM = "I2C TIMEOUT";
const char s_i2c_error[]         PROGMEM = "I2C ERROR\n";
const char s_fmt_i2c_error[]     PROGMEM = " TWCR=%02x STATUS=%02x\n";

extern volatile uint16_t ms_count;


/*
 * signal the end of an I2C bus transfer
 */
int8_t i2c_stop(void)
{
  TWCR = _BV(TWINT)|_BV(TWEN)|_BV(TWSTO);
  while (TWCR & _BV(TWSTO))
    ;
  return 0;
}


/*
 * display the I2C status and error message and release the I2C bus
 */
void i2c_error(const char * message, uint8_t cr, uint8_t status)
{
  i2c_stop();
  printf_P(message);
  printf_P(s_fmt_i2c_error, cr, status);
}


/*
 * signal an I2C start condition in preparation for an I2C bus
 * transfer sequence (polled)
 */
int8_t i2c_start(uint8_t expected_status, uint8_t verbose)
{
  uint8_t status;

  ms_count = 0;

  /* send start condition to take control of the bus */
  TWCR = I2C_START;
  while (!(TWCR & _BV(TWINT)) && (ms_count < I2C_TIMEOUT))
    ;

  if (ms_count >= I2C_TIMEOUT) {
    if (verbose) {
      i2c_error(s_i2c_start_error, TWCR, TWSR);
      i2c_error(s_i2c_timeout, TWCR, TWSR);
    }
    return -1;
  }

  /* verify start condition */
  status = TWSR;
  if (status != expected_status) {
    if (verbose) {
      i2c_error(s_i2c_start_error, TWCR, status);
    }
    return -1;
  }

  return 0;
}


/*
 * initiate a slave read or write I2C operation (polled)
 */
int8_t i2c_sla_rw(uint8_t device, uint8_t op, uint8_t expected_status, 
                  uint8_t verbose)
{
  uint8_t sla_w;
  uint8_t status;

  ms_count = 0;

  /* slave address + read/write operation */
  sla_w = (device << 1) | op;
  TWDR  = sla_w;
  TWCR  = I2C_MASTER_TX;
  while (!(TWCR & _BV(TWINT)) && (ms_count < I2C_TIMEOUT))
    ;

  if (ms_count >= I2C_TIMEOUT) {
    if (verbose) {
      i2c_error(s_i2c_sla_w_error, TWCR, TWSR);
      i2c_error(s_i2c_timeout, TWCR, TWSR);
    }
    return -1;
  }

  status = TWSR;
  if ((status & 0xf8) != expected_status) {
    if (verbose) {
      i2c_error(s_i2c_sla_w_error, TWCR, status);
    }
    return -1;
  }

  return 0;
}


/*
 * transmit a data byte onto the I2C bus (polled)
 */
int8_t i2c_data_tx(uint8_t data, uint8_t expected_status, uint8_t verbose)
{
  uint8_t status;

  ms_count = 0;

  /* send data byte */
  TWDR = data;
  TWCR = I2C_MASTER_TX;
  while (!(TWCR & _BV(TWINT)) && (ms_count < I2C_TIMEOUT))
    ;

  if (ms_count >= I2C_TIMEOUT) {
    if (verbose) {
      i2c_error(s_i2c_data_tx_error, TWCR, TWSR);
      i2c_error(s_i2c_timeout, TWCR, TWSR);
    }
    return -1;
  }

  status = TWSR;
  if ((status & 0xf8) != expected_status) {
    if (verbose) {
      i2c_error(s_i2c_data_tx_error, TWCR, status);
    }
    return -1;
  }

  return 0;
}


/*
 * receive a data byte from the I2C bus (polled)
 */
int8_t i2c_data_rx(uint8_t * data, uint8_t ack, uint8_t expected_status, 
                   uint8_t verbose)
{
  uint8_t status;
  uint8_t b;

  ms_count = 0;

  if (ack) {
    TWCR = _BV(TWINT)|_BV(TWEN)|_BV(TWEA);
  }
  else {
    TWCR = _BV(TWINT)|_BV(TWEN);
  }
  while (!(TWCR & _BV(TWINT)) && (ms_count < I2C_TIMEOUT))
    ;

  if (ms_count >= I2C_TIMEOUT) {
    if (verbose) {
      i2c_error(s_i2c_data_rx_error, TWCR, TWSR);
      i2c_error(s_i2c_timeout, TWCR, TWSR);
    }
    return -1;
  }

  status = TWSR;
  if ((status & 0xf8) != expected_status) {
    if (verbose) {
      i2c_error(s_i2c_data_rx_error, TWCR, status);
    }
    return -1;
  }

  b = TWDR;

  *data = b;

  return 0;
}


uint8_t i2cscan(void)
{
    uint8_t i;
    uint8_t n;

    n = 0;
    
    for (i = 1; i < 127; i++)
    {
        setLED(1);
        ms_sleep(10);
        
        /* start condition */
        if (i2c_start(0x08, 1))
        {
            break;
        }
        
        printf(" %.2X", i);
        if(i % 0x20 == 0)
        printf("\r\n");
        
        /* address slave device, write */
        if (!i2c_sla_rw(i, 0, TW_MT_SLA_ACK, 0)) {
            printf("\r\nfound device at address 0x%02x\r\n", i);
            n++;
        }
        
        i2c_stop();
        setLED(0);
        ms_sleep(10);
    }

    return n;
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
void displayI2cSerialCmdHelp(void)
{
    printf("I2C Serial Commands:\r\n");
    printf("  i2c scan - scan all addresses and report active ones\r\n");
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
void processI2cSerialCmd(char *serCmd)
{
    char *ptr_cmd;
    int   int_val;
    int   n;

    ptr_cmd = strtok(NULL, TOKEN_DELIMINATORS);

    if(strcmp(ptr_cmd, "scan") == STRINGS_MATCH)
    {
          printf("\r\n\r\nI2C bus scanner starting ...\r\n\r\n");
          n = i2cscan();
          printf("\r\n%u devices found\r\n", n);
          printf("scan complete\r\n");
    }
    else
    {
        printf("ERROR - unknown serial command = %s\r\n", serCmd);
    }
}


