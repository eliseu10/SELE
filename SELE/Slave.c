#include "RS485.h"
#include "memory_test.h"

/*
 * Identificação dos LED's
 */
#define RED 0xFF
#define GREEN 0xAA
#define YELLOW 0x00

/*
 * Identificação dos estados do Mestre
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
 * Estado dos LED's
 */
#define OFF 0
#define ON 1

/*
 * Periodo da leitura dos butões 10 ms
 */
#define READ_BUTTONS_PERIOD 10

/*
 * Declaração e inicialização de variaveis globais
 */
uint8_t state_comms = STATEINITCOMM;
uint8_t master_state = STATEINITLED;
volatile int8_t cont = 0;
volatile uint16_t timer = 0;
volatile uint8_t last_button_out = 1;
volatile uint8_t last_button_in = 1;
/**********************************************************************
 *               Declaração de funções utilizadas                     *
 **********************************************************************/

/*
 * Inicializa os inputs e os outputs
 */
void init_io(void);

/*
 * Liga e desliga os LEDs
 * color - Cor do led (RED, GREEN ou YELLOW)
 * set - ligado ou desligado (ON ou OFF)
 */
void set_led(int color, int set);

/*
 * Verifica o byte que o mestre mandou, ou seja se o parque esta livre ou está cheio.
 * Liga o LED verde ou Vermelho de acordo com o que o mestre mandou
 */
void check_master_state(uint8_t byte);

/*
 * Maquina de estados responsavel pelo funcionamento das comunicações entre o mestre e o slave
 */
void state_machine_comunications(void);

/*
 * Função que permite o Mestre testar se o Slave esta vivo e responde
 */
void test_communication_master(void);

/*
 * Função que teste se os LED vermelho e verde estão a funcionar
 * (Só testa circuitos abertos. Não consegue detetar curto circuitos)
 */
void test_led(void);

/*
 * Obtem o o estado do LED vermelho (RED) e do LED verde (GREEN)
 * led - GREEN ou RED
 * Retorno - 1 se ligado 0 se não
 */
uint8_t get_led_state(uint8_t led);

/*
 * Liga o LED verde ou vermelho atravez do les para fazer teste.
 */
void set_mosfet_led(uint8_t led, uint8_t set);

/*
 *
 */
void memory_test(void);

/*
 *
 */
void init_timer0(void);

/**************************************************
 * MAIN
 **************************************************/

int main(int argc, char **argv) {

	init_RS485();
	init_io();
	init_timer1();
	init_timer0();

	memory_test();

	test_led();

	test_communication_master();

	while (1) {
		state_machine_comunications();
	}

	return 0;
}

/*
 * Inicializa o timer0
 * Utiliza o timer0
 */
void init_timer0(void) {

	/* Set the Timer Mode to CTC */
	TCCR0A |= (1 << WGM01);

	/* Set the value that you want to count to */
	OCR0A = 250;

	TIMSK0 |= (1 << OCIE0A); /* Set the ISR COMPA vect */

	TCCR0B |= (1 << CS01) | (1 << CS00);
	/* set prescaler to 64 and start the timer */

	sei();
	/* enable interrupts */

	/*
	 * Interrupção a cada 1ms
	 */
	return;
}

/*
 * Interrupção por comparação para o timer0
 * a cada 1ms
 */
ISR(TIMER0_COMPA_vect) {
	timer++;

	if (timer == UINT16_MAX) {
		timer = 0;
	}

	if (timer >= READ_BUTTONS_PERIOD) {
		if (!(PIND & (1 << PD2))) {
			/* entrou */
			if (last_button_in != 0) {
				cont++;
				last_button_in = 0;
			}
		} else {
			last_button_in = 1;
		}

		if (!(PIND & (1 << PD3))) {
			/* saiu */
			if (last_button_out != 0) {
				cont--;
				last_button_out = 0;
			}
		} else {
			last_button_out = 1;
		}

		timer = 0;
	}

	return;
}

/*
 *Função de teste de memoria flash
 */
void memory_test(void) {
	uint8_t err = 0;
	cli();
	if (0 == memory_test_flash_online()) {
		set_led(YELLOW, ON);
		err = 1;
	}
	if (0 == memory_sram_test()) {
		set_led(RED, ON);
		err = 1;
	}
	if (err != 0) {
		sei();
		while (1) {
			; /* Bolqueia o programa e não faz mais nada */
		}
	}
	sei();
	return;
}

/*
 * Obtem o o estado do LED vermelho (RED) e do LED verde (GREEN)
 * Nota_ só usar em teste de led
 * led - GREEN ou RED
 * Retorno - 1 se ligado 0 se não
 */
uint8_t get_led_state(uint8_t led) {

	if (GREEN == led) {
		if (PINB & (1 << PB0)) {
			return ON;
		} else {
			return OFF;
		}
	} else if (RED == led) {
		if (PINB & (1 << PB1)) {
			return ON;
		} else {
			return OFF;
		}
	}
	return 0xFF;
}

/*
 * Liga o LED verde ou vermelho atravez do les para fazer teste.
 */
void set_mosfet_led(uint8_t led, uint8_t set) {
	if ((RED == led) && (ON == set)) {
		PORTD = PORTD | (1 << PD5);
	} else if ((RED == led) && (OFF == set)) {
		PORTD = PORTD & ~(1 << PD5);
	} else if ((GREEN == led) && (ON == set)) {
		PORTD = PORTD | (1 << PD4);
	} else if ((GREEN == led) && (OFF == set)) {
		PORTD = PORTD & ~(1 << PD4);
	}
	return;
}

/*
 * Função que teste se os LED vermelho e verde estão a funcionar
 * (Só testa circuitos abertos. Não consegue detetar curto circuitos)
 */
void test_led(void) {

	uint8_t led_yellow_state = OFF;
	uint16_t rapido_lento = 500;

	/* Forçar a saida a 0*/
	PORTB &= ~(1 << PB0) & ~(1 << PB1);

	/* colocar como entradas os pinos para o led's */
	DDRB &= ~(1 << PB0) & ~(1 << PB1);

	/* Desativar pull-up resgister para os led*/
	PORTB &= ~(1 << PB0) & ~(1 << PB1);

	set_mosfet_led(GREEN, ON);
	set_mosfet_led(RED, ON);

	while (1) {
		if ((0 == get_led_state(GREEN)) && ( 0 != get_led_state(RED))) {
			if ((500 == get_timer_time())) {
				if (ON == led_yellow_state) {
					set_led(YELLOW, OFF);
					led_yellow_state = OFF;
					reset_watchdog();
				} else {
					set_led(YELLOW, ON);
					led_yellow_state = ON;
					reset_watchdog();
				}
			}
		} else if ((0 == get_led_state(RED)) && ( 0 != get_led_state(GREEN))) {
			if ((1000 == get_timer_time())) {
				if (ON == led_yellow_state) {
					set_led(YELLOW, OFF);
					led_yellow_state = OFF;
					reset_watchdog();
				} else {
					set_led(YELLOW, ON);
					led_yellow_state = ON;
					reset_watchdog();
				}
			}
		} else if ((0 == get_led_state(RED)) && (0 == get_led_state(GREEN))) {
			if (get_timer_time() == rapido_lento) {
				if (ON == led_yellow_state) {
					set_led(YELLOW, OFF);
					led_yellow_state = OFF;
					reset_watchdog();
				} else {
					set_led(YELLOW, ON);
					led_yellow_state = ON;
					reset_watchdog();
					if (500 == rapido_lento) {
						rapido_lento = 1000;
					} else {
						rapido_lento = 500;
					}
				}
			}
		} else {
			;
		}

		if ((0 != get_led_state(RED)) && (0 != get_led_state(GREEN))) {
			init_io();
			return;
		}
	}
	init_io();
	return;
}

/*
 * Maquina de estados responsavel pelo funcionamento das comunicações entre o mestre e o slave
 */
void state_machine_comunications(void) {

	uint8_t led_red_state = OFF;

	/* integer 8 bits */
	uint8_t byte = 0;

	switch (state_comms) {

	case STATEINITCOMM:

		set_multiprocessor_bit();

		byte = get_byte();

		if (0 != get_watchdog_flag()) {

			state_comms = STATESAFE;

		} else if (0 != check_addr(byte)) {

			reset_watchdog();
			state_comms = STATESENDCOUNT;

		} else {

			state_comms = STATEINITCOMM;

		}

		break;

	case STATESENDCOUNT:

		send_byte(cont);

		if (0 != get_watchdog_flag()) {

			state_comms = STATESAFE;

		} else {

			state_comms = STATERECEIVESTATE;

		}

		break;

	case STATERECEIVESTATE:

		byte = get_byte();

		if (0 != get_watchdog_flag()) {

			state_comms = STATESAFE;

		} else {

			reset_watchdog();

			check_master_state(byte);

			state_comms = STATESENDACK;
		}

		break;

	case STATESENDACK:

		send_byte(master_state);

		if (0 != get_watchdog_flag()) {

			state_comms = STATESAFE;

		} else {

			state_comms = STATEINITCOMM;
		}

		break;

	case STATESAFE:

		set_led(GREEN, OFF);

		reset_watchdog();

		while (1) {
			if ((500 == get_timer_time())) {
				if (ON == led_red_state) {
					set_led(RED, OFF);
					led_red_state = OFF;
					reset_watchdog();
				} else {
					set_led(RED, ON);
					led_red_state = ON;
					reset_watchdog();
				}
			}
		}

		break;

	default:

		state_comms = STATESAFE;

		break;
	}
	return;
}

/*
 * Função que permite o Mestre testar se o Slave esta vivo e responde
 */
void test_communication_master(void) {

	uint8_t byte;

	do {
		set_multiprocessor_bit();

		byte = get_byte();

		if (0 != get_watchdog_flag()) {
			state_comms = STATESAFE;
			break;
		}

	} while (!check_addr(byte));

	reset_watchdog();

	send_byte(byte);

	return;

}

/*
 * Inicializa os inputs e os outputs
 */
void init_io(void) {

	/* colocar como saidas os pinos para o max e led's */
	DDRB = 0b00000111;
	DDRD |= (1 << PD6);

	/* Colocar pinos para led's a 1 */
	PORTB |= (1 << PB0) | (1 << PB1);
	PORTD |= (1 << PD6);

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

	return;
}

/*
 * Liga e desliga os LEDs
 * color - Cor do led (RED, GREEN ou YELLOW)
 * set - ligado ou desligado (ON ou OFF)
 */
void set_led(int color, int set) {
	if ((RED == color) && (OFF == set)) {
		PORTB = PORTB | (1 << 1);
	} else if ((RED == color) && (ON == set)) {
		PORTB = PORTB & ~(1 << 1);
	} else if ((GREEN == color) && (OFF == set)) {
		PORTB = PORTB | (1 << 0);
	} else if ((GREEN == color) && (ON == set)) {
		PORTB = PORTB & ~(1 << 0);
	} else if ((YELLOW == color) && (OFF == set)) {
		PORTD = PORTD | (1 << PD6);
	} else if ((YELLOW == color) && (ON == set)) {
		PORTD = PORTD & ~(1 << PD6);
	}
	return;
}

/*
 * Verifica o byte que o mestre mandou, ou seja se o parque esta livre ou está cheio.
 * Liga o LED verde ou Vermelho de acordo com o que o mestre mandou
 */
void check_master_state(uint8_t byte) {
	if (GREEN == byte) {
		master_state = GREEN;
		set_led(GREEN, ON);
		set_led(RED, OFF);
	} else if (RED == byte) {
		master_state = RED;
		set_led(RED, ON);
		set_led(GREEN, OFF);
	} else {
		master_state = 0xA0;
	}
	return;
}

