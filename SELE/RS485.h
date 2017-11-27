/*
 * RS485.h
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#ifndef RS485_H_
#define RS485_H_

#include <avr/io.h>

#define	baud 57600  // baud rate
#define baudgen ((F_CPU/(8*baud))-1)  //baud divider

#define RED 1
#define GREEN 2
#define READ 0
#define WRITE 1

#define SLAVEADDR 0xAA
#define GREENCODE 0x01
#define REDCODE 0x02


void init_RS485(void);

void send_byte(uint8_t byte);

uint8_t get_byte();

int check_addr(uint8_t byte);

void check_master_byte(uint8_t byte);

void set_driver(int operation);

#endif /* RS485_H_ */

