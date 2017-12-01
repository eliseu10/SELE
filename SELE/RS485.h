/*
 * RS485.h
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#ifndef RS485_H_
#define RS485_H_

#include <avr/io.h>
#include <avr/delay.h>

#define F_CPU 16000000UL
#define	baud 57600  /* baud rate */
#define baudgen ((F_CPU/(16*baud))-1)  /* baud divider */

#define READ 0
#define WRITE 1

#define SLAVEADDR 0x01
#define GREENCODE 0xAA
#define REDCODE 0xFF


void init_RS485(void);

void send_byte(uint8_t byte);

uint8_t get_byte(void);

int check_addr(uint8_t byte);

void set_driver(int operation);

uint8_t is_addr(void);

#endif /* RS485_H_ */

