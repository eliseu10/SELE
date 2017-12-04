#ifndef MEMORY_TEST_H_
#define MEMORY_TEST_H_

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define MEMORY_SIZE 2048
#define SIGNATURE 54322

#define UP 1
#define DOWN 2

#define ONE 0xFF
#define ZERO 0x00

uint8_t memory_test_FLASH(void);

uint8_t memory_test_SRAM(void);

uint8_t read_one_write_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

uint8_t read_zero_write_one(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

uint8_t read_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

void write_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes);

#endif /* MEMORY_TEST_H_ */
