#include <avr/io.h>

#define	baud 57600  // baud rate
#define baudgen ((F_CPU/(8*baud))-1)  //baud divider
#define UPCOUNT 1
#define DOWNCOUNT 2
#define ERROCOUNT 9999

/*
 * Estados para a maquina de estados dos LED's
 */
#define STATEINITLED 1
#define STATEGREEN 2
#define STATERED 3


int state_led;	//Estados dos LED's
int state_comms;
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
void send_buffer(uint8_t buffer) {
	// Espera que UDR0 esteja vazio
	while ((UCSR0A & (1 << UDRE0)) == 0);

	//ativar 9 bit para endereco
	UCSR0B = (1 << RXB80);

	UDR0 = buffer; // Envia para a porta serie
}

void init_leds(){

}

void init_

void maquina_estados_comunicacao(){

}

void maquina_estados_led(){
	switch(state_led){
	case STATEINITLED:
		if(change_to_green)
			state_led = STATEGREEN;
		else if (change_to_red)
			state_led = STATERED;
		else
			state_led = STATEINITLED;
		break;
	case STATEGREEN:
		if(change_to_red)
			state_led = STATERED;
		else
			state_led = STATEGREEN;
		break;
	case STATERED:
		if(change_to_green)
			state_led = STATEGREEN;
		else
			state_led = STATERED;
			break;
	}

	if(STATEINITLED){
		//Todos os leds apagados
	}

	if(STATEGREEN){
		//Acende o Led Verde
	}

	if(STATERED){
		//Acende o led Vermelho
	}


}

int contador(int updown){
	if(UPCOUNT)
		return cont++;
	else if(DOWNCOUNT)
		return cont--;
	else
		return ERROCOUNT; //erro
}

int main(int argc, char **argv) {
	//inteiro de 8bits
	uint8_t buffer = 0;

	init_usart();


	//Maquina de estados comunicação


	return 0;
}


