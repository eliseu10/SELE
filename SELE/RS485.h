/*
 * RS485.h
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#ifndef RS485_H_
#define RS485_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define COMMDEATH 5000

#define F_CPU 16000000UL
#define	baud 57600  /* baud rate */
#define baudgen ((F_CPU/(16*baud))-1)  /* baud divider */

#define READ 0
#define WRITE 1

#define SLAVEADDR 0x02   /*Endereço do slave*/

#define GREENCODE 0xAA
#define REDCODE 0xFF

void reset_watchdog(void);

void clear_multiprocessor_bit(void);

void set_multiprocessor_bit(void);

uint8_t get_watchdog_flag(void);

int get_timer_time(void);

void init_timer(void);

void init_RS485(void);

void send_byte(char byte);

char get_byte(void);

int check_addr(uint8_t byte);

void set_driver(int operation);

uint8_t is_addr(void);

#endif /* RS485_H_ */

