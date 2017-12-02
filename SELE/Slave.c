#include "RS485.h"
#include <util/delay.h>
#include <avr/interrupt.h>

#define UPCOUNT 1
#define DOWNCOUNT 2
#define ERRORCOUNT 9999

/*
 * Led RED ou Green
 */
#define RED 0xFF
#define GREEN 0xAA

/*
 * Estados para a maquina de estados dos LED's
 */
#define STATEINITLED 1
#define STATEGREEN 2
#define STATERED 3

/*
 * Estados para a maquina de estados para comunicações
 */
#define STATEINITCOMM 0
#define STATESENDCOUNT 1
#define STATERECEIVESTATE 2
#define STATESENDACK 5
#define STATESAFE 3

/*
 * Liga ou desliga led
 */
#define OFF 0
#define ON 1

/*
 * Entrou carro ou saiu
 */
#define OUT 0
#define IN 1

//int state_led = STATEINITLED; /* Estados dos LED's */
volatile uint8_t state_comms = STATEINITCOMM;
volatile char master_state = STATEINITLED;
volatile char cont = 0;

void init_io(void);

void init_interrupts_buttons(void);

void set_led(int color, int set);

void check_master_state(uint8_t byte);

void state_machine_comunications(void);

void test_communication_master(void);

int main(int argc, char **argv) {

	init_RS485();
	init_io();
	init_timer();
	init_interrupts_buttons();

	test_communication_master();

	while (1) {

		state_machine_comunications();

	}
}

void state_machine_comunications(void) {

	/* integer 8 bits */
	char byte = 0;

	switch (state_comms) {

	case STATEINITCOMM:

		set_multiprocessor_bit();

		byte = get_byte();

		if (get_watchdog_flag()){

			state_comms = STATESAFE;

		} else if (check_addr(byte)) {

			reset_watchdog();
			state_comms = STATESENDCOUNT;

		} else {

			state_comms = STATEINITCOMM;

		}

		break;

	case STATESENDCOUNT:

		send_byte(cont);

		if(get_watchdog_flag()){

			state_comms = STATESAFE;

		} else {

			state_comms = STATERECEIVESTATE;

		}

		break;

	case STATERECEIVESTATE:

		byte = get_byte();

		if(get_watchdog_flag()){

			state_comms = STATESAFE;

		} else {

			reset_watchdog();

			check_master_state(byte);

			state_comms = STATESENDACK;
		}

		break;

	case STATESENDACK:

		send_byte(master_state);

		if(get_watchdog_flag()){

			state_comms = STATESAFE;

		} else {

			state_comms = STATEINITCOMM;
		}

		break;

	case STATESAFE:

		set_led(GREEN, OFF);

		while (1) {

			set_led(RED, ON);
			while (!(500 < get_timer_time()));
			reset_watchdog();

			set_led(RED, OFF);
			while (!(500 < get_timer_time()));
			reset_watchdog();

		}

		break;

	default:

		state_comms = STATESAFE;

		break;
	}

}


void test_communication_master(void){

	char byte;

	do {
		set_multiprocessor_bit();

		byte = get_byte();

		if (get_watchdog_flag()){

			state_comms = STATESAFE;
			break;
		}

	} while (!check_addr(byte));

	reset_watchdog();

	send_byte(byte);

	return;

}



/*
 * PB0 - Led Green
 * PB1 - Led Red
 * PB3 - write/read selector
 * PB4 - button in
 * PB5 - button out
 */
void init_io(void) {

	/* colocar como saidas os pinos para o max e led's */
	DDRB = 0b00000111;

	/* Colocar pinos para led's a 1 */
	PORTB |= (1 << PB0) | (1 << PB1);

	/* Pinos como saidas PD4 e PD5 para os mosfet */
	DDRD |= (1 << PD4) | (1 << PD5);

	/* Inicializar as gates do mosfet a 0*/
	PORTD &= ~(1 << PD4) & ~(1 << PD5);

	/* PD2 e PD3 inputs para os botões*/
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD3);

	/* turn On the Pull-up dos botões*/
	PORTD |= (1 << PD3);
	PORTD |= (1 << PD2);
}

/* Liga e desliga os led's */
void set_led(int color, int set) {
	if ((RED == color) && (OFF == set)) {
		PORTB = PORTB | (1 << 1);
	} else if ((RED == color) && (ON == set)) {
		PORTB = PORTB & ~(1 << 1);
	} else if ((GREEN == color) && (OFF == set)) {
		PORTB = PORTB | (1 << 0);
	} else if ((GREEN == color) && (ON == set)) {
		PORTB = PORTB & ~(1 << 0);
	}
}

/*
 * Verificar se o mestre diz que pode entrar carros ou não e muda o semafero de acordo.
 */
void check_master_state(uint8_t byte) {
	if (GREENCODE == byte) {
		master_state = GREEN;
		set_led(GREEN, ON);
		set_led(RED, OFF);
	} else if (REDCODE == byte) {
		master_state = RED;
		set_led(RED, ON);
		set_led(GREEN, OFF);
	} else {
		master_state = 0xA0;
	}
}

void init_interrupts_buttons(void) {

	/* set INT0 and INT1 to trigger on FE */
	EICRA |= (0b10 << ISC00);
	EICRA |= (0b10 << ISC10);

	EIMSK |= (1 << INT0); /* Turns on INT0 */
	EIMSK |= (1 << INT1); /* Turns on INT1 */

	sei();
}

/* Entrada */
ISR (INT0_vect) {
	cont++;
	return;
}

/* Saidas */
ISR (INT1_vect) {
	cont--;
	return;
}

