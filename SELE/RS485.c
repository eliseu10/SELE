/*
 * RS485.c
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#include "RS485.h"

/*
 * Trama:
 * - 1 start bit
 * - 8 bits data
 * - 1 address bit
 * - 1 stop bit
 * Rx/Tx
 */
void init_RS485(void) {
	/* Definir baudrate */
	UBRR0H = (uint8_t) (baudgen >> 8);
	UBRR0L = (uint8_t) baudgen;
	/* UCSR0A = (1 << U2X0);  Double speed */

	/* Definir formato da trama */
	UCSR0C = (7 << UCSZ00) /* 9 data bits */
			| (0 << UPM00) /* no parity */
			| (0 << USBS0) /* 1 stop bit */
			| (0 << UMSEL00) | (0 << UMSEL01); /* comunicacao assincrona */

	/* Ativar emissao e rececao */
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << UCSZ02);
}

/*
 * Para enviar um byte basta escreve-lo
 * no registo UDR0, verificando antes se
 * este está disponível (bit UDRE0 de UCSR0A)
 */
void send_byte(uint8_t byte) {

	set_driver(WRITE);
	_delay_us(30);

	/* Espera que UDR0 esteja vazio */
	while ((UCSR0A & (1 << UDRE0)) == 0);

	UDR0 = byte; /* Envia para a porta serie */

	/* Reset flag */
	UCSR0A |= (1 << TXC0);

	/* Utilizar TXC0 para esperar que seja tudo enviado */
	while ((UCSR0A & (1 << TXC0)) == 0);

	set_driver(READ);

	return;
}

uint8_t get_byte(void) {

	set_driver(READ);

	/* Espera que RXC0 tenha la alguma coisa */
	while ((UCSR0A & (1 << RXC0)) == 0);

	return UDR0;
}

/*
 * Verifica se endereco corresponde ao meu
 */

uint8_t is_addr(void){

	while ( !(UCSR0A & (1<<RXC0)));

	return (UCSR0B & (1 << RXB80));

}

int check_addr(uint8_t byte) {
	/* (verifica se e um addr) and (corresponde ao slave) */

	if (byte == SLAVEADDR) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * Controla pino responsável por indicar a drive(MAX485)
 * se vai receber ou enviar
 */
void set_driver(int operation) {
	if (READ == operation) {
		PORTB = PORTB & ~(1 << 2);
	} else if (WRITE == operation) {
		PORTB = PORTB | (1 << 2);
	}
}
