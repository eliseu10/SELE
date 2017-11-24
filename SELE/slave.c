#include <avr/io.h>

#define	baud 57600  // baud rate
#define baudgen ((F_CPU/(8*baud))-1)  //baud divider

void init_usart(void) {
	// Definir baudrate
	UBRR0H = (uint8_t) (baudgen >> 8);
	UBRR0L = (uint8_t) baudgen;
	UCSR0A = (1 << U2X0); // Double speed

	// Definir formato da trama
	UCSR0C = (3 << UCSZ00) // 8 data bits
	| (0 << UPM00) // no parity
			| (0 << USBS0); // 1 stop bit

	// Ativar apenas o emissor
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

int main(int argc, char **argv) {
	return 0;
}

