#ifndef MEMORY_TEST_H_
#define MEMORY_TEST_H_

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define SIGNATURE 54321

#define UP 1
#define DOWN 2

#define ONE 0xFF
#define ZERO 0x00

/*******************************************************************************************************************************
 * Testes de memória ClassB
 * Adaptação da biblioteca da AVR
 ***************************************************************************************************************************/
/*
 Brief Number of sections the SRAM is divided into for testing.
 It is advisable that \c INTERNAL_SRAM_SIZE is divisible by the number of
 sections and, therefore, recommended values are 2, 4, 8, 16, etc. Otherwise an extra
 section will be added with the remainder of the division as size. Note that the higher
 the number of sections the smaller the size of \ref classb_buffer, i.e. the section of memory
 that is reserved for the test) and the faster each partial test is completed.
 */
#define INTERNAL_SRAM_SIZE 2048
#define INTERNAL_SRAM_START 0x0100
#define CLASSB_NSECS 8

//internal The size of each segment in bytes
#define CLASSB_SEC_SIZE (INTERNAL_SRAM_SIZE / CLASSB_NSECS)

int memory_sram_test( void );

int marchCminus(register volatile uint8_t * p_sram, register volatile uint8_t * p_buffer, register uint16_t size);

/******************************************************************************************
 * Minhas funções
 *********************************************************************************************/
uint8_t memory_test_flash_online(void);

uint8_t memory_test_offline(void);

uint8_t memory_test_SRAM(void);

uint8_t read_one_write_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

uint8_t read_zero_write_one(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

uint8_t read_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

void write_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);


#endif /* MEMORY_TEST_H_ */
