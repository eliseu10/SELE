#include "RS485.h"
#include <util/delay.h>


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
#define STATEINITCOMM 0
#define SENDCOUNT 1
#define RECEIVESTATE 2

/*
 * Liga ou desliga led define
 */
#define OFF 0
#define ON 1

/*
 * Entrou carro ou saiu
 */
#define OUT 0
#define IN 1


int state_led = STATEINITLED;	/* Estados dos LED's */
int state_comms = STATEINITCOMM;
int master_state = RED;
uint8_t cont = 0x05;


/*
 * PB0 - Led Green
 * PB1 - Led Red
 * PB3 - write/read selector
 * PB4 - button in
 * PB5 - button out
 */
void init_io(void)
{
	DDRB = 0b00000111;

	/* set pull-up resistors */
	PORTB = PORTB | (1<<3);
	PORTB = PORTB | (1<<4);
}

void check_master_byte(uint8_t byte)
{
	if(GREENCODE == byte)
	{
		master_state = GREEN;
	}
	else if(REDCODE == byte)
	{
		master_state = RED;
	}
}

/* tested and working */
void set_led(int color, int set)
{
	if((RED == color) && (ON == set))
	{
		PORTB = PORTB | ( 1 << 1);
	}
	else if ((RED == color) && (OFF == set))
	{
		PORTB = PORTB & ~(1 << 1);
	}
	else if ((GREEN == color) && (ON == set))
	{
		PORTB = PORTB | ( 1 << 0);
	}
	else if ((GREEN == color) && (OFF == set))
	{
		PORTB = PORTB & ~(1 << 0);
	}
}

void maquina_estados_comunicacao(void)
{
	/* integer 8 bits */
	uint8_t byte = 0;

	switch (state_comms)
	{
	case STATEINITCOMM:
		set_driver(READ);

		byte = get_byte();

		if (check_addr(byte))
		{
			set_led(GREEN,ON);
			state_comms = SENDCOUNT;
		}
		break;

	case SENDCOUNT:
		set_driver(WRITE);
		_delay_us(5);
		send_byte(cont);
		state_comms = RECEIVESTATE;
		break;

	case RECEIVESTATE:
		set_driver(READ);
		byte = get_byte();
		check_master_byte(byte);
		set_led(RED,ON);
		state_comms = STATEINITCOMM;
		break;

	default:
		state_led = STATEINITCOMM;
		break;
	}
}


/*
 * Verificada e a funicionar
 */
int check_button(int direction)
{
	if(IN == direction)
	{
		if(!(PINB & (1<<3)))
		{
			return ON;
		}
	}else if (OUT == direction)
	{
		if(!(PINB & (1<<4)))
		{
			return ON;
		}

	}
	return OFF;
}

void maquina_estados_led(void)
{
	switch (state_led)
	{
	case STATEINITLED:
		if (GREEN == master_state)
		{
			state_led = STATEGREEN;
		}
		else if (RED == master_state)
		{
			state_led = STATERED;
		}
		else
		{
			state_led = STATEINITLED;
		}
		break;
	case STATEGREEN:
		if (RED == master_state)
		{
			state_led = STATERED;
		}
		else
		{
			state_led = STATEGREEN;
		}
		break;

	case STATERED:
		if (GREEN == master_state)
		{
			state_led = STATEGREEN;
		}
		else
		{
			state_led = STATERED;
		}
		break;

	default:
		state_led = STATEINITLED;
		break;
	}

	if(STATEINITLED == state_led)
	{
		/* Todos os leds apagados */
		set_led(GREEN,OFF);
		set_led(RED,OFF);
	}

	if(STATEGREEN == state_led)
	{
		/* Acende o Led Verde */
		set_led(GREEN,ON);
	}

	if(STATERED == state_led)
	{
		/* Acende o led Vermelho */
		set_led(RED,ON);
	}

}

int contador(int updown)
{
	if(UPCOUNT)
	{
		return cont++;
	}
	else if(DOWNCOUNT)
	{
		return cont--;
	}
	else
	{
		return ERRORCOUNT; /* erro */
	}
}

void maquina_estados_contador(void)
{

	if (check_button(IN))
		contador(UPCOUNT);
	if (check_button(OUT))
		contador(DOWNCOUNT);

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
		}
		send_byte(0x12);
		set_led(GREEN,ON);*/
		maquina_estados_comunicacao();
		//maquina_estados_led();
		//maquina_estados_contador();
	}
	return 0;
}
