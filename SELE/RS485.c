/*
 * RS485.c
 *
 *  Created on: 27/11/2017
 *      Author: helio
 */

#include "RS485.h"

volatile uint8_t watchdog_flag = 0;
volatile int watchdog = 0;

/*
 * Configuracao TIMER1
 */
void init_timer(void) {

	/* habilita a interrupção do TIMER1 */
	TIMSK1 |= (1 << OCIE1A);

	TCCR1A = 0; /* Normal mode */
	TCCR1B = 0; /* inicializa, Stop TC1 */

	/* forma de contar clock/1, 1/(16000000/1)= 62,5 ns */

	TCCR1B |= (1 << CS10); /* sem per-divisao */
	OCR1A = 16000; /* temporizador (62,5 ns) * 16000 = 1ms */

}

/*
 * Configurar watch dog timer
 * Caso passem mais de 6 segundos sem comunicar
 * liga watch dog e passa para rescue_mode
 */

ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
	watchdog++;

	if (watchdog == INTMAX_MAX)
		watchdog = 0;

	if (COMMDEATH <= watchdog) {
		reset_watchdog();
		watchdog_flag = 1;
	}

	return;
}

int get_timer_time(void){
	return watchdog;
}

uint8_t get_watchdog_flag(void){
	return watchdog_flag;
}

void reset_watchdog(void){
	watchdog = 0;
}


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
void send_byte(char byte) {

	set_driver(WRITE);
	_delay_us(30);

	/* Espera que UDR0 esteja vazio */
	while ((UCSR0A & (1 << UDRE0)) == 0){
		if (watchdog_flag){
			return;
		}
	}

	UDR0 = byte; /* Envia para a porta serie */

	/* Reset flag */
	UCSR0A |= (1 << TXC0);

	/* Utilizar TXC0 para esperar que seja tudo enviado */
	while ((UCSR0A & (1 << TXC0)) == 0){
		if (watchdog_flag){
			return;
		}
	}

	set_driver(READ);

	return;
}

char get_byte(void) {

	set_driver(READ);

	/* Espera que RXC0 tenha la alguma coisa */
	while ((UCSR0A & (1 << RXC0)) == 0){
		if (watchdog_flag){
			return 0x00;
		}
	}

	return UDR0;
}

/*
 * Verifica se endereco corresponde ao meu
 */

void set_multiprocessor_bit(void){

	UCSR0A |= (1 << MPCM0);
	return;

}

void clear_multiprocessor_bit(void){

	UCSR0A &= ~(1 << MPCM0);
	return;

}

/*Não utiliar*/
uint8_t is_addr(void){

	/*Ativar modo multiprocessador*/
	UCSR0A |= (1 << MPCM0);

	while ( !(UCSR0A & (1<<RXC0))){
		if (watchdog_flag){
			return 0x00;
		}
	}

	return (UCSR0B & (1 << RXB80));

}

int check_addr(uint8_t byte) {
	/* (verifica se e um addr) and (corresponde ao slave) */

	if (byte == SLAVEADDR) {
		/*Desativar modo multiprocessador*/
		clear_multiprocessor_bit();
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
