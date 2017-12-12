#include "memory_test.h"

#define SRAM_ERROR 0

uint16_t EEMEM sign_eeprom;
uint16_t EEMEM hash_flash_online;
uint8_t EEMEM hash_flash_offline[16];
uint16_t EEMEM bytes;

static uint8_t buffer[SEC_SIZE]/* __attribute__ ((section (".sram_buffer")))*/;

/*
 * Return 0 se falha 1 se passa
 */
uint8_t memory_sram_test() {
	/* This variable keeps track of the section to test. */
	static uint8_t current_section = 0;

	for (current_section = 0; current_section < NSECS; current_section++) {
		if (0 == marchCminus((uint8_t *) INTERNAL_SRAM_START + current_section * SEC_SIZE, buffer, SEC_SIZE)) {
			return 0;
		}
	}

	return 1;

}

/*
 * SRAM memory.
 *
 *  p_sram    Pointer to first byte in memory area to be tested
 *  p_buffer  Pointer to first byte in the buffer
 *  size      Size of area to be tested in bytes.
 *
 *	Retorna 0 se falhar 1 se não
 */
uint8_t marchCminus(register volatile uint8_t * p_sram, register volatile uint8_t * p_buffer, register uint16_t size) {

	register uint16_t i = 0;
	register uint8_t err = 0;

	/* Save content of the section: copy to buffer unless we test the buffer */
	if (p_buffer != p_sram) {
		for (uint16_t i = 0; i < size; i++) {
			*(p_buffer + i) = *(p_sram + i);
		}
	}

	/*
	 * Implemetação do MARCH C-
	 */

	/* Test phase 1: Write zeros UP */
	for (i = 0; i < size; i++) {
		*(p_sram + i) = ZERO;

		/* induzir erro */

#if SRAM_ERROR
		 if ((p_sram + i) == (uint8_t *)0x0800) {
			*(p_sram + i) = 0x22;
		}
#endif
	}

	/* Test phase 2: read ZERO, write ONE. UP */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ZERO) {
			err = 1;
		}

		*(p_sram + i) = ONE;
	}

	/* Test phase 3: read ONE, write ZERO. UP. */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ONE) {
			err = 1;
		}

		*(p_sram + i) = ZERO;
	}

	/* Test phase 4: read ZERO, write ONE. DOWN */
	for (i = size; i > 0; i--) {
		if (*(p_sram + i - 1) != ZERO) {
			err = 1;
		}

		*(p_sram + i - 1) = ONE;
	}

	/* Test phase 5: read ONE, write ZERO. DOWN */
	for (i = size; i > 0; i--) {
		if (*(p_sram + i - 1) != ONE) {
			err = 1;
		}

		*(p_sram + i - 1) = ZERO;
	}

	/* Test phase 6: read ZERO. UP */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ZERO) {
			err = 1;
		}
	}

	/* Restore content of the section: copy from buffer, unless buffer is tested */
	if (p_buffer != p_sram) {
		for (i = 0; i < size; i++) {
			*(p_sram + i) = *(p_buffer + i);
		}
	}
	if (err == 1) {
		return 0;
	}
	return 1;
}

/*
 * Retorna 1 se passa 0 se falha
 */
uint8_t memory_test_flash_online(void) {

	uint16_t hash = 0;
	uint16_t n_words = 256 * 64;
	uint16_t i;

	for (i = 0; i < n_words; i++) {
		hash ^= pgm_read_word(i);
	}

	if (SIGNATURE != eeprom_read_word(&sign_eeprom)) {
		eeprom_update_word(&hash_flash_online, hash);
		eeprom_update_word(&sign_eeprom, SIGNATURE);
	}

	if (hash != eeprom_read_word(&hash_flash_online)) {
		return 0;
	}

	return 1;

}

/*
 * retorna 1 se passa 0 se falha
 */
uint8_t memory_test_flash_offline(void) {

	uint8_t hash[16];

	uint16_t i = 0;
	uint16_t x = 0;
	uint16_t n_bytes = eeprom_read_word(&bytes);

	for (i = 0; i < 16; i++) {
		hash[i] = 0;
	}

	for (i = 0; i < n_bytes; i++) {
		hash[x] ^= pgm_read_byte(i);
		x++;
		if (x == 16) {
			x = 0;
		}
	}

	for (i = 0; i < 16; i++) {
		if (hash[i] != eeprom_read_byte(hash_flash_offline + i)) {
			return 0;
		}
	}

	return 1;
}
