#include <avr/io.h>
#include "RS485.h"

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

/*
 *
 */
#define RED 1
#define GREEN 2
#define OFF 0
#define ON 1
#define OUT 0
#define IN 1
#define READ 0
#define WRITE 1
#define PARKFULL 1


int state_led = STATEINITLED;	//Estados dos LED's
int state_comms = INIT;
int state_counter = INIT;
int master_state = RED;
int cont = 0;


void maquina_estados_comunicacao(){
	//integer 8 bits
	uint8_t byte = 0;

	switch (state_comms) {
	case INIT:
		set_driver(READ);
		byte = get_byte();
		if (check_addr(byte))
			state_comms = SENDCOUNT;
		break;

	case SENDCOUNT:
		set_driver(WRITE);
		send_byte(cont);
		state_comms = RECEIVESTATE;
		break;

	case RECEIVESTATE:
		set_driver(READ);
		byte = get_byte();
		check_master_byte(byte);
		state_comms = INIT;
		break;
	}



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
	PORTB = PORTB | (1<<3);
	PORTB = PORTB | (1<<4);
}

//tested and working
void set_led(int color, int set){
	if((RED == color) && (ON == set)) {
		PORTB = PORTB | ( 1 << 1);
	}
	else if ((RED == color) && (OFF == set)) {
		PORTB = PORTB & ~(1 << 1);
	}
	else if ((GREEN == color) && (ON == set)) {
		PORTB = PORTB | ( 1 << 0);
	}
	else if ((GREEN == color) && (OFF == set)) {
		PORTB = PORTB & ~(1 << 0);
	}
	else{
		return;
	}
}

/*
 * Verificada e a funicionar
 */
int check_button(int direction){
	if(IN == direction){
		if(!(PINB & (1<<3)))
			return ON;
	}else if (OUT == direction) {
		if(!(PINB & (1<<4)))
			return ON;
	}
	return OFF;
}

void maquina_estados_led(){
	switch(state_led){
	case STATEINITLED:
		if(GREEN == master_state)
			state_led = STATEGREEN;
		else if (RED == master_state)
			state_led = STATERED;
		else
			state_led = STATEINITLED;
		break;
	case STATEGREEN:
		if(RED == master_state)
			state_led = STATERED;
		else
			state_led = STATEGREEN;
		break;
	case STATERED:
		if(GREEN == master_state)
			state_led = STATEGREEN;
		else
			state_led = STATERED;
			break;
	}

	if(STATEINITLED == state_led){
		//Todos os leds apagados
		set_led(GREEN,OFF);
		set_led(RED,OFF);
	}

	if(STATEGREEN == state_led){
		//Acende o Led Verde
		set_led(GREEN,ON);
	}

	if(STATERED == state_led){
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

void maquina_estados_contador(){
	switch (state_counter) {
		case INIT:
			if(check_button(IN))
				contador(UPCOUNT);
			if(check_button(OUT))
				contador(DOWNCOUNT);
			if(RED == master_state)
				state_counter = PARKFULL;
			break;
		case PARKFULL:
			if(GREEN == master_state)
				state_counter = INIT;
			break;
	}
}

int main(int argc, char **argv) {
	init_RS485();
	init_io();

	while(1){
		/*
		if(check_button(OUT) == ON){
			set_led(GREEN,ON);
		}
		if(check_button(IN) == ON){
			set_led(GREEN,OFF);
		}*/
		maquina_estados_comunicacao();
		maquina_estados_led();
		maquina_estados_contador();
	}
	return 0;
}
