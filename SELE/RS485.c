#include "RS485.h"
#include "serial_port.h"
#include <stdlib.h>

/*
 * Watchdog timer e a flag
 */
volatile uint8_t watchdog_flag = 0;
volatile uint16_t watchdog = 0;
uint8_t EEMEM address;

void print_value(uint8_t value){

	char aux = '0';

	if(9 >= value){

		print_char(value + 48);
		return;

	}
	else if(9 < value && 99 >= value){

		aux = value / 10;
		print_char(aux + 48);
		aux = value % 10;
		print_char(aux + 48);
		return;

	}
	else if(99 < value){

		aux = value / 100;
		print_char(aux + 48);
		aux = (value % 100) / 10;
		print_char(aux + 48);
		aux = value % 10;
		print_char(aux + 48);
		return;

	}
	else {
		;
	}

	return;
}

void setup_address(void) {
	/* se for feito reset e o botão de entrada estiver precionado entra em modo configuração */
	uint8_t addr = 0;
	char nAddr[5], *ptr;

	if(!(PIND & (1 << PD2))){

		init_USART();

		addr = eeprom_read_byte(&address);

		write_string("Address: ");
		print_value(addr);
		write_string("\r\n");

		write_string("New Address [1, 255]: ");

		while (0 == (read_string(nAddr))) {
			;
		}

		addr = strtol(nAddr, &ptr, 10);

		eeprom_update_byte(&address, addr);

		addr = eeprom_read_byte(&address);
		write_string("Address saved: ");
		print_value(addr);
		write_string("\r\n");

	}

	return;
}
/*
 * Inicializa o timer do Watchdog
 * Utiliza o timer1
 */
void init_timer1(void) {

	OCR1A = 16000;

	TCCR1B |= (1 << WGM12);
	/* Mode 4, CTC on OCR1A */

	TIMSK1 |= (1 << OCIE1A);
	/* Set interrupt on compare match */

	TCCR1B |= (1 << CS10);
	/* start the timer */

	sei();
	/* enable interrupts */
	return;
}

/*
 * Interrupção por comparação para o timer1
 * Faz um timer que o menor tick é 1ms
 * Ativa a flag do watchdog caso o Watchdog >= COMMDETH
 */
ISR(TIMER1_COMPA_vect) {
	watchdog++;

	if (watchdog == 65530) {
		reset_watchdog();
	}

	if (COMMDEATH <= watchdog) {
		/* reset_watchdog(); */
		watchdog_flag = 1;
	}

	return;
}

/*
 * Retorna o vaor do watchdog
 */
uint16_t get_timer_time(void) {
	return watchdog;
}

/*
 * Retorna o valor daflag do watchdog
 * A flag do watchdog e colocada a 1 quando o watch dog passa o valor em COMMDEATH
 */
uint8_t get_watchdog_flag(void) {
	return watchdog_flag;
}

/*
 * Faz reset ao contador watchdog das comunicações
 */
void reset_watchdog(void) {
	watchdog = 0;

	return;
}

/*
 * Inicializa a comunicação assincrona RS485
 * Trama:
 * - 1 start bit
 * - 9 bits data
 * - 1 stop bit
 * Rx/Tx
 */
void init_RS485(void) {

	/*Definir endereço*/
	setup_address();

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

	return;
}

/*
 * Envia um byte atravez da comunicação assincrona
 */
void send_byte(uint8_t byte) {

	set_driver(WRITE);
	_delay_us(30);

	/* Espera que UDR0 esteja vazio */
	while (0 == (UCSR0A & (1 << UDRE0))) {
		if (0  != watchdog_flag) {
			set_driver(READ);
			return;
		}
	}

	/* Envia para a porta serie */
	UDR0 = byte;

	/* Reset flag */
	UCSR0A |= (1 << TXC0);

	/* Utilizar TXC0 para esperar que seja tudo enviado */
	while (0 == (UCSR0A & (1 << TXC0))) {
		if (0 != watchdog_flag) {
			set_driver(READ);
			return;
		}
	}

	set_driver(READ);

	return;
}

/*
 * Retorna um byte recebido atravez do RS485
 */
uint8_t get_byte(void) {

	set_driver(READ);

	/* Espera que RXC0 tenha la alguma coisa */
	while (0 == (UCSR0A & (1 << RXC0))) {
		if (0 != watchdog_flag) {
			return 0x00;
		}
	}

	return UDR0;
}

/*
 * Verifica se o endereço recebido é o endereço do SLAVE
 * Se sim retorna 1 senão retorna 0
 */
uint8_t check_addr(uint8_t byte) {
	/* (verifica se e um addr) and (corresponde ao slave) */

	if (eeprom_read_byte(&address) == byte) {
		/*Desativar modo multiprocessador*/
		clear_multiprocessor_bit();
		return 1;
	}

	return 0;
}

/*
 * Coloca a 1 o bit do modo multiprocessador
 */
void set_multiprocessor_bit(void) {
	UCSR0A |= (1 << MPCM0);
	return;
}

/*
 * Coloca a 0 o bit do modo multiprocessador para o desativar
 */
void clear_multiprocessor_bit(void) {
	UCSR0A &= ~(1 << MPCM0);
	return;
}

/*
 * Coloca o driver do RS485 em modo de escrita (WRITE) ou em modo de leitura (READ)
 */
void set_driver(int operation) {
	if (READ == operation) {
		PORTB = PORTB & ~(1 << 2);
	} else if (WRITE == operation) {
		PORTB = PORTB | (1 << 2);
	}
	return;
}
