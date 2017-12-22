#include "serial_port.h"

/*
 * Inicializaçao da comunicaçao via USART
 * para o modo de configuração
 */
void init_USART(void) {

	UBRR0 = baudgen; /*definir baudrate*/

	/*enable do receptor e do emissor*/
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	/*2 stop bits + 8 bits de dados + sem paridade*/
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);

	return;
}

/*
 * Envia um char pela porta serie
 */
void print_char(char c) {

	/*espera que UDR0 esteja vazio;*/
	while (!(UCSR0A & (1 << UDRE0))){
		;
	}

	UDR0 = c; /*envia para a porta série*/

	return;
}

/*
 * Recebe um char pela porta serie
 */
char get_char(void) {

	/*espera que receba alguma coisa*/
	while (!(UCSR0A & (1 << RXC0))){
		;
	}

	return UDR0;
}

/*
 * Envia uma sequencia de chars pela porta serie
 * utilizando a funçao print_char
 */
void write_string(char *str) {

	while (*str != 0) {
		print_char(*str);
		++str;
	}

	return;
}

/*
 * Recebe uma sequencia de chars pela porta serie
 * utilizando a funçao get_char
 */
uint8_t read_string(char *c) {

	char p = 0;
	static int pos = 0;

	if (UCSR0A & (1 << RXC0)) {
		p = get_char();
		if ((p == '\r') || (p == '\n')) {
			p = 0;
			c[pos++] = '\n';
			c[pos++] = p;
			pos = 0;
			return 1;
		} else {
			c[pos++] = p;
			return 0;
		}
	} else {
		return 0;
	}
}
