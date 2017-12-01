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
uint8_t volatile state_comms = STATEINITCOMM;
uint8_t volatile master_state = STATEINITLED;
uint8_t volatile cont = 0;

volatile int watchdog = 0;

void init_io(void);
void init_interrupts_buttons(void);
void init_timer(void);
void set_led(int color, int set);
void check_master_state(char byte);
void state_machine_comunications(void);

/*
 * Configurar watch dog timer
 * Caso passem mais de 5 segundos sem comunicar
 * liga watch dog e passa para rescue_mode
 */
ISR(TIMER1_COMPA_vect) {
	TCNT1 = 0;
	watchdog++;

	if (6000 == watchdog) {
		watchdog = 0;
		state_comms = STATESAFE;

	}
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

/*
 * Configuracao TIMER1
 * 
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

void state_machine_comunications(void) {

	/* integer 8 bits */
	uint8_t byte = 0;

	switch (state_comms) {

	case STATEINITCOMM:

		if (is_addr()) {
			byte = get_byte();
			if (check_addr(byte)) {
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

	case STATESAFE:

		if (((watchdog > 0) && (watchdog < 1000))
				|| ((watchdog > 2000) && (watchdog < 3000))
				|| ((watchdog > 4000) && (watchdog < 5000))) {
			set_led(RED, ON);
		}
		if (((watchdog > 1000) && (watchdog < 2000))
				|| ((watchdog > 3000) && (watchdog < 4000))
				|| ((watchdog > 5000) && (watchdog < 6000))) {
			set_led(RED, OFF);
		}

		break;

	default:

		state_comms = STATESAFE;

		break;
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

/*
 * Contador de carros
 */

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

int main(int argc, char **argv) {

	init_RS485();
	init_io();
	init_timer();
	init_interrupts_buttons();

	while (1) {

		state_machine_comunications();
	}

}

