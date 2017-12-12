#ifndef RS485_H_
#define RS485_H_

#include <util/delay.h>
#include <avr/interrupt.h>

/*
 * Tempo a partir do qual que se considera que foi perdida a conecção com o mestre
 */
#define COMMDEATH 5000

/*
 *  Endereço do slave a ser programado (Mudar isto para cada slave)
 */
#define SLAVEADDR 0x02

#define F_CPU 16000000UL
#define	baud 57600  /* baud rate */
#define baudgen ((F_CPU/(16*baud))-1)  /* baud divider */

/*
 * Modos de funcionamento para a driver do RS485
 */
#define READ 0
#define WRITE 1

/*
 * Faz reset ao contador watchdog das comunicações
 */
void reset_watchdog(void);

/*
 * Coloca a 0 o bit do modo multiprocessador para o desativar
 */
void clear_multiprocessor_bit(void);

/*
 * Coloca a 1 o bit do modo multiprocessador
 */
void set_multiprocessor_bit(void);

/*
 * Retorna o valor daflag do watchdog
 * A flag do watchgod e colocada a 1 quando o watch dog passa o valor em COMMDEATH
 */
uint8_t get_watchdog_flag(void);

/*
 * Retorna o vaor do watchdog
 */
uint16_t get_timer_time(void);

/*
 * Inicializa o timer do Watchdog
 */
void init_timer1(void);

/*
 * Inicializa a comunicação assimcrona RS485
 */
void init_RS485(void);

/*
 * Envia um byte atravez da comunicação assincrona
 */
void send_byte(uint8_t byte);

/*
 * Retorna um byte recebido atravez do RS485
 */
uint8_t get_byte(void);

/*
 * Verifica se o endereço recebido é o endereço do SLAVE
 * Se sim retorna 1 senã retorna 0
 */
uint8_t check_addr(uint8_t byte);

/*
 * Coloca o driver do RS485 em modo de escrita (WRITE) ou em modo de leitura (READ)
 */
void set_driver(int operation);

#endif /* RS485_H_ */

