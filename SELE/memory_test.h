#ifndef MEMORY_TEST_H_
#define MEMORY_TEST_H_

#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define SIGNATURE 55550

#define ONE 0xFF
#define ZERO 0x00

/*******************************************************************************************************************************
 * Testes de memória
 * Adaptação da biblioteca da AVR
 ***************************************************************************************************************************/

#define INTERNAL_SRAM_SIZE 2048
#define INTERNAL_SRAM_START 0x0100
#define NSECS 8

/* The size of each segment in bytes */
#define SEC_SIZE (INTERNAL_SRAM_SIZE / NSECS)

uint8_t memory_sram_test(void);

uint8_t marchCminus(register volatile uint8_t * p_sram, register volatile uint8_t * p_buffer, register uint16_t size);

uint8_t memory_test_flash_online(void);

uint8_t memory_test_offline(void);

#endif /* MEMORY_TEST_H_ */
