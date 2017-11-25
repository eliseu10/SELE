#include <avr/io.h>

#define	baud 57600  // baud rate
#define baudgen ((F_CPU/(8*baud))-1)  //baud divider
#define UPCOUNT 1
#define DOWNCOUNT 2
#define ERRORCOUNT 9999

/*
 * Estados para a maquina de estados dos LED's
 */
#define STATEINITLED 1
#define STATEGREEN 2
#define STATERED 3

/*
 * Estados para a maquina de estados para comunicações
 */
#define INIT 0
#define SENDCOUNT 1
#define RECEIVESTATE 2

#define RED 1
#define GREEN 2
#define OFF 0
#define ON 1
#define OUT 0
#define IN 1

int state_led;	//Estados dos LED's
int state_comms;
int master_state;
int cont;

void init_usart(void) {
	// Definir baudrate
	UBRR0H = (uint8_t) (baudgen >> 8);
	UBRR0L = (uint8_t) baudgen;
	UCSR0A = (1 << U2X0); // Double speed

	// Definir formato da trama
	UCSR0C = (5 << UCSZ00) // 9 data bits
			| (0 << UPM00) // no parity
			| (0 << USBS0) // 1 stop bit
			| (0 << UMSEL00)
			| (0 << UMSEL01); //comunicacao assincrona

	// Ativar emissao e rececao
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

/*
 * Para enviar um byte basta escreve-lo
 * no registo UDR0, verificando antes se
 * este está disponível (bit UDRE0 de UCSR0A)
 *
 */
void send_byte(uint8_t byte) {
	// Espera que UDR0 esteja vazio
	while ((UCSR0A & (1 << UDRE0)) == 0);

	UDR0 = byte; // Envia para a porta serie
}

void maquina_estados_comunicacao(){
	//integer 8 bits
	uint8_t buffer = 0;

	//ativar 9 bit para endereco
	UCSR0B = (1 << RXB80);

	switch (state_comms) {
		case INIT:

			break;
		case SENDCOUNT:

			break;
		case RECEIVESTATE:

			break;
	}

	init_usart();
}

/*
 * PB0 - Led Green
 * PB1 - Led Red
 * PB3 - write/read selector
 * PB4 - button in
 * PB5 - button out
 */
void init_io(){
	DDRB = DDRB=0b00000111;

	//set pull-up resistors
	PORTB = PORTB | (1<<4);
	PORTB = PORTB | (1<<5);
}

//tested and working
void set_led(int color, int set){
	if((RED == color) && (ON == set)){
		PORTB = PORTB | ( 1 << 1);
	}else if ((RED == color) && (OFF == set)) {
		PORTB = PORTB & ~ ( 1 << 1);
	}else if ((GREEN == color) && (ON == set)) {
		PORTB = PORTB | ( 1 << 0);
	}else if ((GREEN == color) && (OFF == set)) {
		PORTB = PORTB & ~ ( 1 << 0);
	}
}

/*
 * Botoes ainda nao estao a funcionar
 * acho que devemos por resistencia de pull-up
 */
int check_button(int direction){
	if(IN == direction){
		if(!(PINB & (1<<4)))
			return ON;
	}else if (OUT == direction) {
		if(!(PINB & (1<<5)))
			return ON;
	}
	return OFF;
}


void maquina_estados_led(){
	switch(state_led){
	case STATEINITLED:
		if(master_state == GREEN)
			state_led = STATEGREEN;
		else if (master_state == RED)
			state_led = STATERED;
		else
			state_led = STATEINITLED;
		break;
	case STATEGREEN:
		if(master_state == RED)
			state_led = STATERED;
		else
			state_led = STATEGREEN;
		break;
	case STATERED:
		if(master_state == GREEN)
			state_led = STATEGREEN;
		else
			state_led = STATERED;
			break;
	}

	if(STATEINITLED){
		//Todos os leds apagados
		set_led(GREEN,OFF);
		set_led(RED,OFF);
	}

	if(STATEGREEN){
		//Acende o Led Verde
		set_led(GREEN,ON);
	}

	if(STATERED){
		//Acende o led Vermelho
		set_led(RED,ON);
	}


}

int contador(int updown){
	if(UPCOUNT)
		return cont++;
	else if(DOWNCOUNT)
		return cont--;
	else
		return ERRORCOUNT; //erro
}

int main(int argc, char **argv) {
	init_io();
	set_led(RED,OFF);
	set_led(GREEN,OFF);
	while(1){



	}
	return 0;
}


