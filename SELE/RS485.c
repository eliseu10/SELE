/*
 * RS485.c
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#include "RS485.h"


void init_RS485(void) {
	// Definir baudrate
	UBRR0H = (uint8_t) (baudgen >> 8);
	UBRR0L = (uint8_t) baudgen;
	//UCSR0A = (1 << U2X0); // Double speed

	// Definir formato da trama
	UCSR0C = (7 << UCSZ00) // 9 data bits
			| (0 << UPM00) // no parity
			| (0 << USBS0) // 1 stop bit
			| (0 << UMSEL00)
			| (0 << UMSEL01); //comunicacao assincrona

	// Ativar emissao e rececao
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

void send_byte(uint8_t byte) {
	// Espera que UDR0 esteja vazio
	while ((UCSR0A & (1 << UDRE0)) == 0);

	UDR0 = byte; // Envia para a porta serie
}

uint8_t get_byte(){
	// Espera que RXC0 tenha la alguma coisa
	while ((UCSR0A & (1 << RXC0)) == 0);

	return UDR0;
}

int check_addr(uint8_t byte){
	//(verifica se e um addr) and (corresponde ao slave)
	if((UCSR0B & (1 << RXB80)) && (byte == SLAVEADDR))
		return 1;
	else
		return 0;
}



void set_driver(int operation){
	if(READ == operation)
		PORTB = PORTB & ~ ( 1 << 2);
	else if(WRITE == operation)
		PORTB = PORTB | ( 1 << 2);

}
