/*
 * RS485.h
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#ifndef RS485_H_
#define RS485_H_

#include <avr/io.h>

#define F_CPU 16000000UL
#define	baud 57600  // baud rate
#define baudgen ((F_CPU/(16*baud))-1)  //baud divider

#define RED 1
#define GREEN 2
#define READ 0
#define WRITE 1

#define SLAVEADDR 0x01
#define GREENCODE 0xAA
#define REDCODE 0xFF


/*
 * Trama:
 * - 1 start bit
 * - 8 bits data
 * - 1 address bit
 * - 1 stop bit
 * Rx/Tx
 */
void init_RS485(void);

/*
 * Para enviar um byte basta escreve-lo
 * no registo UDR0, verificando antes se
 * este está disponível (bit UDRE0 de UCSR0A)
 */
void send_byte(uint8_t byte);

/*
 * TXB80 - configure that data or addr
 * RXB80 - read 9 bit of data
 */
uint8_t get_byte();

/*
 * Verifica se endereco corresponde ao meu
 */
int check_addr(uint8_t byte);

/*
 * Controla pino responsável por indicar a drive(MAX485)
 * se vai receber ou enviar
 */
void set_driver(int operation);

#endif /* RS485_H_ */

