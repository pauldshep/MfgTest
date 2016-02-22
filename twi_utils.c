
/*******************************************************************************
*   File Name: twi_utils.c 
*  
* Description: Two Wire Interface, I2C, utilities. 
*******************************************************************************/
#include <util/twi.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <inttypes.h>
#include <string.h> 
#include "twi_utils.h"
#include "timers.h"

const char s_twi_start_error[]   PROGMEM = "TWI START CONDITION ERROR";
const char s_twi_sla_w_error[]   PROGMEM = "TWI SLAVE ADDRESS ERROR";
const char s_twi_data_tx_error[] PROGMEM = "TWI DATA TX ERROR";
const char s_twi_data_rx_error[] PROGMEM = "TWI DATA RX ERROR";
const char s_twi_timeout[]       PROGMEM = "TWI TIMEOUT";
const char s_twi_error[]         PROGMEM = "TWI ERROR\n";
const char s_fmt_twi_error[]     PROGMEM = " TWCR=%02x STATUS=%02x\n";

static uint8_t verbose;


/******************************************************************************* 
*                         INITIALIZE TWO WIRE INTERFACE                        *
******************************************************************************** 
* Description: Initialize the I2C (two wire) interface:
*              prescaller value = 16, bit rate = 15
*              SCL = 16MHz / (16 + 2 * 15 * (4^16) = 32,258
* 
*              Ports:   PD0 - SCL
*                       PD1 - SDA 
* 
*              Registers:
*              TWSR - two wire status register, p206
*              TWBR - two wire bit rate register, p205
*              TWCR - two wire control register, p205
* 
*      Global: verbose
* 
*   Arguments: None
* 
*      Return: None
*******************************************************************************/ 
void init_twi(void)
{
    /* set the I2C bit rate generator to 100 kb/s */  
    /* TWSR &= ~0x03;   // set prescaller to zero
    TWBR  = 28;
    TWCR |= _BV(TWEN);*/

    // GSL, well no its not 100kb/s -- isn't it CPU / (16 + 28 * 4^0)? which is 360 kb/s
    // to get 100kbs with 16 Mhz clock we need TWBR = 9 and prescaller = 16
    // or 16 MHZ / ( 16 + 9*4^2)
    TWSR &= ~0x03;        // clear prescaller bits 0 and 1, ps value = 1
    TWSR &=  0x02;        // set prescaller to 2, ps value = 16
    TWBR  =    15;        // 0000,1111 
    TWCR |= _BV(TWEN);    // enable twi
    verbose = 0;
}


/******************************************************************************* 
*                                   START TWI                                  *
********************************************************************************
* Description: START--signal an TWI start condition in preparation for an TWI
*              bus transfer sequence (polled)
*              Registers:
*              TWCR - two wire control register, p205
*              TWSR - two wire status register
* 
*   Arguments: expected_status
* 
*      Return: 0 if no errors are detected, error code otherwise
*******************************************************************************/
int8_t twi_start(uint8_t expected_status)
{
    uint8_t status;

    ms_twiCount = 0;

    // send start condition to take control of the bus: TWINT, TWSTA, TWEN
    TWCR = TWCR_START;

    while(!(TWCR & _BV(TWINT)) && (ms_twiCount < TWI_TIMEOUT))
    ;

    if(ms_twiCount >= TWI_TIMEOUT) 
    {
   	  twi_error(s_twi_start_error, TWCR, TWSR);
	  twi_error(s_twi_timeout, TWCR, TWSR);
      return -1;
    }

    // verify start condition
    status = TWSR;
    if(status != expected_status) 
    {
	   printf_P(PSTR("expected  "));
       twi_error(s_twi_start_error, TWCR, status);
	   return -1;
    }
    return 0;
}

 
/******************************************************************************* 
*                                    STOP TWI                                  *
********************************************************************************
* Description: STOP--signal the end of an TWI bus transfer
* 
*  Arguments: None
* 
*     Return: 0 if no error is detected
*******************************************************************************/
int8_t twi_stop()
{
    TWCR        = _BV(TWINT)|_BV(TWEN)|_BV(TWSTO);
    ms_twiCount = 0;

    while (TWCR & _BV(TWSTO) && ms_twiCount < TWI_TIMEOUT)
    ;

    if(ms_twiCount >= TWI_TIMEOUT) 
    {
       twi_error(s_twi_timeout, TWCR, TWSR);
    }
    return 0;
}


/*******************************************************************************
*                                   TWI ERROR                                  *
********************************************************************************
* Description: display the TWI status and error message and release the TWI bus
*  
*   Arguments: *message
*               cr
*               status
*  
*      Return: None
*******************************************************************************/
void twi_error(const char * message, uint8_t cr, uint8_t status)
{
    if (verbose)
    {
  	    printf_P(message);
  	    printf_P(s_fmt_twi_error, cr, status);
    }
    twi_stop(1);
}


/*******************************************************************************
*                                TWI WRITE BYTES                               *
********************************************************************************
* Description: Write to twi without address pointer (or address pointer is
*              part of buffer)
*  
*   Arguments: twi_addr - two wire interface address
*              line
*              *buf
*  
*      Return: 0 if no error is detected
*******************************************************************************/
int twi_write_bytes(uint8_t twi_addr, int len, uint8_t *buf)
{
	return twi_write_bytes_with_WP(twi_addr, -2, len, buf);
}


/*******************************************************************************
*                      TWI WRITE BYTES WITH ADDRESS POINTER                    *
********************************************************************************
* Description: Write to twi with two byte address pointer.  If the write
*              pointer address is less then zero start then it is not sent.
*  
*   Arguments: twi_addr
*              write Pointer
*              len
*              *buf
*  
*      Return: None
*******************************************************************************/
int twi_write_bytes_with_WP(uint8_t twi_addr, int16_t writePointer,  int len, uint8_t *buf)
{
    uint8_t n     = 0;
    uint8_t wpCnt = 0;  //write pointer count.
    uint8_t twst;
    int     rv    = 0;
    //uint16_t ms_twiCount;
  
  
  if (writePointer <0)
  	wpCnt = 2;					// don't send  write Pointer.

  restart:

  if(n++ >= TWI_MAX_ITER)
    return -TWI_MAX_ITER;


 begin:
 
  ms_twiCount = 0;

  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }



  /* send twi_addr + W */
  ms_twiCount = 0;
  TWDR = (twi_addr <<1) | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  ms_sleep(1);  // don't know why we need to wait but we do!

  

  for (; len > 0; len--)
  {
    ms_twiCount = 0;
    if (wpCnt>1)
    	TWDR = *buf++;
    else                       // first two bytes are the address of the date to be written.
    { 
    	len++;
			TWDR = (wpCnt==0) ? writePointer>> 8 : writePointer & 0xFF ;    
			wpCnt++;
		}
    	
    TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
    while ( !(TWCR & _BV(TWINT)) &&  ms_twiCount < TWI_TIMEOUT) ; /* wait for transmission */
    if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_data_tx_error, TWCR, TWSR);
	  goto quit;  // should this be go to error?
    }
    switch ((twst = TW_STATUS))
    {
  	  case TW_MT_DATA_NACK:
	    twi_error( s_twi_data_tx_error ,TWCR, twst);
        goto error;	   // why go to error here but go to quit	

      case TW_MT_DATA_ACK:
        rv++;
	    break;

	  default:
      goto error;
     
	  }
  	ms_sleep(1);  // don't know why we need to wait but we do!
  }
  quit:
  twi_stop(1);  

  return rv;

  error:
  rv = -1;
  goto quit;
}


/*******************************************************************************
*                                  READ BYTES                                  *
********************************************************************************
* Description: Data Read---NO pointer is sent
* 
*   Arguments: twi_addr - device address
*              len      - number of bytes to read
*              *buf     - put read data here
*  
*      Return: number of bytes read
*******************************************************************************/
int twi_read_bytes(uint8_t twi_addr, int len, uint8_t *buf)
{  

  uint8_t n = 0;
  uint8_t twst, twcr;
  int rv = 0;



  restart:

  if (n++ >= TWI_MAX_ITER)
  {
    return -TWI_MAX_ITER;
  }
  begin:
 
  ms_twiCount =0;
 //printf_P(PSTR("TWI-Start Cond\n"));			//GSL
  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }


  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
//	  printf_P(PSTR("TWI-Started\n"));			//GSL
      break;

    case TW_MT_ARB_LOST:
//	  printf_P(PSTR("TWI_ARB_LOST\n"));			//GSL
      goto begin;

    default:
//	  printf_P(PSTR("TWI_DEFAULT1\n"));			//GSL	
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }



  /* send twi_addr + R */
  ms_twiCount =0;
  TWDR = (twi_addr <<1) | TW_READ;
//  printf_P(PSTR("TWI-starting Trans\n"));			//GSL	

  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
//	  printf_P(PSTR("TWI-timedOut\n"));			//GSL
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
//	   printf_P(PSTR("TWI-ACK\n"));			//GSL

      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
//	   printf_P(PSTR("TWI-NotACK\n"));			//GSL
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
//	   printf_P(PSTR("TWI-ARB-2\n"));			//GSL
      goto begin;
  }


  for (; len > 0; len--)
  {
//     printf_P(PSTR("len-%d\n"),len);			//GSL
	ms_twiCount =0;
    twcr = _BV(TWINT) | _BV(TWEN) |  _BV(TWEA);  // NAK on last byte
    if (len == 1)
      twcr = _BV(TWINT) | _BV(TWEN); // send NAK this time 
    TWCR = twcr;		/* clear int to start transmission */
    while ((TWCR & _BV(TWINT)) == 0  && (ms_twiCount < TWI_TIMEOUT)) ; /* wait for transmission */
    if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
   }
    switch ((twst = TW_STATUS))
	{
	  case TW_MR_DATA_NACK:
//	    printf_P(PSTR("TWI-NACK-2\n"));			//GSL
	    len = 0;		/* force end of loop */
	    /* FALLTHROUGH */
	  case TW_MR_DATA_ACK:
//	    printf_P(PSTR("ACK\n"));			//GSL
	    *buf++ = TWDR;
	    rv++;
	    break;

	default:
//	  printf_P(PSTR("TWI-defaultError\n"));			//GSL
	  goto error;
	}
  }
  quit:
  /* Note [14] */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;
}



/*******************************************************************************
*                                  READ BYTES                                  *
********************************************************************************
* Description: Data Read ONE BYTE data ADDRESS!! 
*              ---Write Pointer Then Read   
*              USE: twi_read_bytes_wP2() for 2 byte address
*  
*   Arguments: twi_addr
*              write_pointer_addr
*              len
*              *buf
*  
*      Return: None
*******************************************************************************/
int twi_read_bytes_wP(uint8_t twi_addr, uint8_t write_pointer_addr, int len, uint8_t *buf)
{
  uint8_t n = 0;
  uint8_t twst, twcr;
  int rv = 0;
//  uint16_t ms_twiCount; 


  restart:

  if (n++ >= TWI_MAX_ITER)
    return -TWI_MAX_ITER;

  begin:
 
  ms_twiCount =0;
  ///////////////////////////
  // send start condition 
  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }

  ///////////////////////////
  // send twi addr + w
  ms_twiCount =0;
  TWDR = (twi_addr <<1) | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  ms_sleep(1);  // don't know why we need to wait but we do!



  ///////////////////////////
  // send data pointer 
  ms_twiCount =0;
  TWDR = write_pointer_addr;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  ms_sleep(1);  // don't know why we need to wait but we do!


  ///////////////////////////
  //  resend start condition 
  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }


  ///////////////////////////
  // send twi addr + R
  ms_twiCount =0;
  TWDR = (twi_addr <<1) | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  //////////////////////////////////////
  // Now Read Data
  for (; len > 0; len--)
  {
    twcr = _BV(TWINT) | _BV(TWEN) |  _BV(TWEA);  // NAK on last byte
    if (len == 1)
      twcr = _BV(TWINT) | _BV(TWEN); // send NAK this time 
    TWCR = twcr;		/* clear int to start transmission */
    while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
    switch ((twst = TW_STATUS))
	{
	  case TW_MR_DATA_NACK:
	    len = 0;		/* force end of loop */
	    /* FALLTHROUGH */
	  case TW_MR_DATA_ACK:
	    *buf++ = TWDR;
	    rv++;
	    break;

	default:
	  goto error;
	}
  }

 quit:
  /* Note [14] */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;

}




/*******************************************************************************
*                                  READ BYTES                                  *
********************************************************************************
* Description: Data Read TWO BYTE data ADDRESS!!
*              ---Write Pointer Then Read   
*              USE: twi_read_bytes_wP() for 1 byte address
*  
*   Arguments: twi_addr
*              write_pointer_addr
*              len
*              *buf
*  
*      Return: None
*******************************************************************************/
int twi_read_bytes_wP2(uint8_t twi_addr, uint16_t write_pointer_addr, int len, uint8_t *buf)
{
  uint8_t n = 0, write_pointer_addr_H, write_pointer_addr_L;
  uint8_t twst, twcr;
  int rv = 0;
//  uint16_t ms_twiCount;

  write_pointer_addr_H = (write_pointer_addr >> 8);
  write_pointer_addr_L = (write_pointer_addr & 0xFF);


  restart:

  if (n++ >= TWI_MAX_ITER)
    return -TWI_MAX_ITER;

  begin:
 
  ms_twiCount =0;
  ///////////////////////////
  // send start condition 
  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }

  ///////////////////////////
  // send twi addr + w
  ms_twiCount =0;
  TWDR = (twi_addr <<1) | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  ms_sleep(1);  // don't know why we need to wait but we do!



  ///////////////////////////
  // send MSB data pointer  
  ms_twiCount =0;
  TWDR = write_pointer_addr_H ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
//  ms_sleep(1);  // don't know why we need to wait but we do!


///////////////////////////
  // send LSB data pointer  
  ms_twiCount =0;
  TWDR = write_pointer_addr_L ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  ms_sleep(1);  // don't know why we need to wait but we do!



  ///////////////////////////
  //  resend start condition 
  TWCR = TWCR_START ; // send start condition 
  while ( !(TWCR & _BV(TWINT))  && (ms_twiCount < TWI_TIMEOUT) ) ; // wait for transmission 
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_start_error, TWCR, TWSR);
    return -2;
   }
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		// OK, but should not happen 
    case TW_START:
      break;

    case TW_MT_ARB_LOST:
      goto begin;

    default:
      return -3;		// error: not in start condition 
				// NB: do /not/ send stop condition 
    }


  ///////////////////////////
  // send twi addr + R
  ms_twiCount =0;
  TWDR = (twi_addr <<1) | TW_READ;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ( !(TWCR & _BV(TWINT)) && ms_twiCount < TWI_TIMEOUT ) 
  {
  			; // wait for transmission 
  }
  if (ms_twiCount >= TWI_TIMEOUT) {
      twi_error(s_twi_sla_w_error, TWCR, TWSR);
      goto error;	  
  }

  switch ((twst = TW_STATUS))
  {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;
  }
  //////////////////////////////////////
  // Now Read Data
  for (; len > 0; len--)
  {
    twcr = _BV(TWINT) | _BV(TWEN) |  _BV(TWEA);  // NAK on last byte
    if (len == 1)
      twcr = _BV(TWINT) | _BV(TWEN); // send NAK this time 
    TWCR = twcr;		/* clear int to start transmission */
    while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
    switch ((twst = TW_STATUS))
	{
	  case TW_MR_DATA_NACK:
	    len = 0;		/* force end of loop */
	    /* FALLTHROUGH */
	  case TW_MR_DATA_ACK:
	    *buf++ = TWDR;
	    rv++;
	    break;

	default:
	  goto error;
	}
  }

 quit:
  /* Note [14] */
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

  error:
  rv = -1;
  goto quit;

}



/*******************************************************************************
*                              PARSE I2C COMMANDS                              *
********************************************************************************
* Description: Parse I2C command 
*  
*   Arguments: *cmd
*  
*      Return: None
*******************************************************************************/
void parseI2C_commands(char *cmd)
{
	if (!strncmp(cmd,"tem",3 ))
	{
		verbose ^= 1;
		verbose &= 1;
		if (verbose)
		{
			printf_P(PSTR("error messages on\n"));
		}
		else 			
		{
			printf_P(PSTR("error messages off\n"));
		}
	}
	else if (cmd[0] == '?')
	{
		printf_P(PSTR("terr -- toggle TWI Error display of Messages on and off\n"));
	}
	else
	{
		printf_P(PSTR("\ni2c command unknown.\n"));
	}

}
