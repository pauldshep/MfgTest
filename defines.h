/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * General stdiodemo defines
 *
 * $Id: defines.h,v 1.2 2006/10/08 21:47:36 joerg_wunsch Exp $
 */

/* CPU frequency */
#define F_CPU 16000000UL

/* Baud Rate */
#define UART_BAUD  9600

/* UART baud rate */
#define BAUD 9600
#define BAUD_RR(f,b) ((f % (16L*b) >= (16L*b)/2) ? (f / (16L*b)) : (f / (16L*b)) - 1)

