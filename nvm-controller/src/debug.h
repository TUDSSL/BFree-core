/*
 * debug.h
 *
 *  Created on: Jun 25, 2019
 *      Author: abubakar
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include<msp430.h>
#include<string.h>
#include<stdint.h>


void uartConfig(void);
int io_putchar(int);
int io_puts_no_newline(const char *);
void printf(char *, ...);                                   // tiny printf function


#endif /* DEBUG_H_ */
