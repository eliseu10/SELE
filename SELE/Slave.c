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
uint8_t volatile state_comms = STATEINITCOMM;
uint8_t volatile master_state = STATEINITLED;
uint8_t volatile cont = 0;


/*
 * PB0 - Led Green
 * PB1 - Led Red
 * PB3 - write/read selector
 * PB4 - button in
 * PB5 - button out
 */
void init_io(void)
{

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
void set_led(int color, int set)
{
	if ((RED == color) && (OFF == set))
	{
		PORTB = PORTB | (1 << 1);
	} else if ((RED == color) && (ON == set))
	{
		PORTB = PORTB & ~(1 << 1);
	} else if ((GREEN == color) && (OFF == set))
	{
		PORTB = PORTB | (1 << 0);
	} else if ((GREEN == color) && (ON == set))
	{
		PORTB = PORTB & ~(1 << 0);
	}
}

/*
 * Verificar se o mestre diz que pode entrar carros ou não e muda o semafero de acordo.
 */
void check_master_state(uint8_t byte)
{
	if (GREENCODE == byte)
	{
		master_state = GREEN;
		set_led(GREEN, ON);
		set_led(RED, OFF);
	} else if (REDCODE == byte)
	{
		master_state = RED;
		set_led(RED, ON);
		set_led(GREEN, OFF);
	} else {
		master_state = 0xA0;
	}
}

void state_machine_comunications(void) {

	/* integer 8 bits */
	uint8_t byte = 0;

	switch (state_comms) {

	case STATEINITCOMM:

		if(is_addr()){
			byte = get_byte();
			if (check_addr(byte))
			{
				state_comms = STATESENDCOUNT;
			}
		}

		break;

	case STATESENDCOUNT:

		send_byte(cont);

		state_comms = STATERECEIVESTATE;
		break;

	case STATERECEIVESTATE:

		byte = get_byte();

		check_master_state(byte);

		state_comms = STATESENDACK;
		break;

	case STATESENDACK:

		send_byte(master_state);

		state_comms = STATEINITCOMM;

		break;

	default:

		state_comms = STATEINITCOMM;

		break;
	}

}

/*
void state_machine_led(void) {
	switch (state_led) {
	case STATEINITLED:

			if (GREEN == master_state) {
				state_led = STATEGREEN;
			} else if (RED == master_state) {
				state_led = STATERED;
			} else {
				state_led = STATEINITLED;
			}
			break;

		case STATEGREEN:

			if (RED == master_state) {
				state_led = STATERED;
			} else {
				state_led = STATEGREEN;
			}
			break;

		case STATERED:
			if (GREEN == master_state) {
				state_led = STATEGREEN;
			} else {
				state_led = STATERED;
			}
			break;

		default:

			state_led = STATEINITLED;
			break;
	}

	if (STATEINITLED == state_led) {
		//Todos os leds apagados
		set_led(GREEN, OFF);
		set_led(RED, OFF);
	}

	if (STATEGREEN == state_led) {
		// Acende o Led Verde
		set_led(GREEN, ON);
		set_led(RED, OFF);
	}

	if (STATERED == state_led) {
		// Acende o led Vermelho
		set_led(RED, ON);
		set_led(GREEN, OFF);
	}
}
*/

/*int check_button(int direction) {
	if (IN == direction) {
		if (!(PINB & (1 << 3))) {
			return ON;
		}
	} else if (OUT == direction) {
		if (!(PINB & (1 << 4))) {
			return ON;
		}

	}
	return OFF;
}

int contador(int updown) {
	if (UPCOUNT) {
		return cont++;
	} else if (DOWNCOUNT) {
		return cont--;
	} else {
		return ERRORCOUNT;
		erro
	}
}

void maquina_estados_contador(void) {

	if (check_button(IN))
		contador(UPCOUNT);
	if (check_button(OUT))
		contador(DOWNCOUNT);

}*/


void test_led(void){

	/*Trocar Pinos de controlo de leds de saidas para entradas*/
	/* PD2 e PD3 entradas para teste led's*/
	DDRB &= ~(1 << PB0);
	DDRB &= ~(1 << PB1);

	/* turn OFF the Pull-up para os led's*/
	PORTB &= ~(1 << PB0);
	PORTB &= ~(1 << PB1);


	PORTD &= ~(1 << PD4) & ~(1 << PD5);
	_delay_ms(500);
	PORTD |= (1 << PD4) | (1 << PD5);
	_delay_ms(500);

}

void init_interrupts_buttons(void)
{

	/* set INT0 and INT1 to trigger on FE */
	EICRA |= (0b10 << ISC00);
	EICRA |= (0b10 << ISC10);

	EIMSK |= (1 << INT0);     /* Turns on INT0 */
	EIMSK |= (1 << INT1);     /* Turns on INT1 */

	sei();

}

/*
 * Contador de carros
 */

/* Entrada */
ISR (INT0_vect) {
	cont++;

/*	set_led(GREEN, ON);
	_delay_ms(500);
	set_led(GREEN, OFF);*/

	return;
}

/* Saidas */
ISR (INT1_vect) {
	cont--;

/*	set_led(RED, ON);
	_delay_ms(500);
	set_led(RED, OFF);*/

	return;
}


 int main(int argc, char **argv){

	init_RS485();
	init_io();
	init_interrupts_buttons();



	while (1) {


		state_machine_comunications();
//		state_machine_led();

	}
	return 0;
}
