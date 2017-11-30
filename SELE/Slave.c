#include "RS485.h"
#include <util/delay.h>
#include <avr/interrupt.h>

#define UPCOUNT 1
#define DOWNCOUNT 2
#define ERRORCOUNT 9999

#define RED 1
#define GREEN 2

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
 * Liga ou desliga led define
 */
#define OFF 0
#define ON 1

/*
 * Entrou carro ou saiu
 */
#define OUT 0
#define IN 1

int state_led = STATEINITLED; /* Estados dos LED's */
int state_comms = STATEINITCOMM;
int master_state = RED;
char cont = 0x05;

/*
 * PB0 - Led Green
 * PB1 - Led Red
 * PB3 - write/read selector
 * PB4 - button in
 * PB5 - button out
 */
void init_io(void)
{

	DDRB = 0b00000111; //colocar como saidas os pinos para o max e led's


	/* set pull-up resistors */
//	PORTB = PORTB | (1 << 3);
//	PORTB = PORTB | (1 << 4);

	DDRD &= ~(1 << PD2);     // PD2 e PD3 inputs
	DDRD &= ~(1 << PD3);


	PORTD |= (1 << PD3);		//
	PORTD |= (1 << PD2);		// turn On the Pull-up
	// PD2 is now an input with pull-up enabled
}

void init_interrupts_buttons(void)
{

	EICRA |= (1 << ISC11) | (1 << ISC10) | (1 << ISC01) | (1 << ISC00);    // set INT0 to trigger on RE
	EIMSK |= (1 << INT0);     // Turns on INT0
	EIMSK |= (1 << INT1);     // Turns on INT1
	// turn on interrupts

	sei();


}

/* Liga e desliga os led's */
void set_led(int color, int set)
{
	if ((RED == color) && (ON == set))
	{
		PORTB = PORTB | (1 << 1);
	} else if ((RED == color) && (OFF == set))
	{
		PORTB = PORTB & ~(1 << 1);
	} else if ((GREEN == color) && (ON == set))
	{
		PORTB = PORTB | (1 << 0);
	} else if ((GREEN == color) && (OFF == set))
	{
		PORTB = PORTB & ~(1 << 0);
	}
}

/*
 * Verificar se o mestre diz que pode entrar carros ou não e muda o semafero de acordo.
 */
void check_master_state(char byte)
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
	}
}

void maquina_estados_comunicacao(void)
{
	/* integer 8 bits */
	char byte = 0;

	switch (state_comms)
	{

	case STATEINITCOMM:

		set_driver(READ);
		byte = get_byte();

		if (check_addr(byte))
		{
			state_comms = STATESENDCOUNT;
		}
		break;

	case STATESENDCOUNT:

		set_driver(WRITE);
		_delay_us(5);
		send_byte(cont);

		state_comms = STATERECEIVESTATE;
		break;

	case STATERECEIVESTATE:

		set_driver(READ);
		byte = get_byte();

		check_master_state(byte);

		state_comms = STATESENDACK;
		break;

	case STATESENDACK:
		set_driver(WRITE);
		_delay_us(5);
		send_byte(byte);
		state_comms = STATEINITCOMM;
		break;

	default:

		state_comms = STATEINITCOMM;

		break;
	}

}



/*
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
 //Todos os leds apagados
 set_led(GREEN,OFF);
 set_led(RED,OFF);
 }

 if(STATEGREEN == state_led)
 {
 // Acende o Led Verde
 set_led(GREEN,ON);
 }

 if(STATERED == state_led)
 {
 // Acende o led Vermelho
 set_led(RED,ON);
 }

 }
 */

/*int check_button(int direction)
 {
 if(IN == direction)
 {
 if(!(PINB & (1<<3)))
 {
 return ON;
 }
 }
 else if (OUT == direction)
 {
 if(!(PINB & (1<<4)))
 {
 return ON;
 }

 }
 return OFF;
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
 return ERRORCOUNT;  erro
 }
 }

 void maquina_estados_contador(void)
 {

 if (check_button(IN))
 contador(UPCOUNT);
 if (check_button(OUT))
 contador(DOWNCOUNT);

 }*/

/*
 * Contador de carros
 */
/*
ISR (INT0_vect)
{
	cont++;
	set_led(GREEN, ON);
	_delay_ms(500);
	set_led(GREEN, OFF);
	return;
}

ISR (INT1_vect)
{
	cont--;
	set_led(RED, ON);
	_delay_ms(500);
	set_led(RED, OFF);
	return;
}
*/



 int main(int argc, char **argv)
{
	init_RS485();
	init_io();
	//init_interrupts_buttons();

	while (1)
	{
		/*
		 if(check_button(OUT) == ON){
		 set_led(GREEN,ON);
		 }
		 if(check_button(IN) == ON){
		 set_led(GREEN,OFF);
		 }
		 send_byte(0x12);
		 set_led(GREEN,ON);*/

//		set_led(GREEN,ON);
//		_delay_ms(500);
//		set_led(RED,ON);
//		_delay_ms(500);
//		set_led(GREEN,OFF);
//		_delay_ms(500);
//		set_led(RED,OFF);
//		_delay_ms(500);
		maquina_estados_comunicacao();
		//maquina_estados_led();
		//maquina_estados_contador();
	}
	return 0;
}
