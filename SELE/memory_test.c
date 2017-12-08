#include "memory_test.h"

uint16_t EEMEM sign_eeprom;
uint16_t EEMEM hash_flash_offline;
uint8_t EEMEM hash_flash_online[16];
uint16_t EEMEM bytes;

static uint8_t classb_buffer[CLASSB_SEC_SIZE]/* __attribute__ ((section (".classb_sram_buffer")))*/;

int memory_sram_test()
{
	/* This variable keeps track of the section to test. */
	static uint8_t current_section = 0;

	for(current_section = 0; current_section < CLASSB_NSECS; current_section++) {
		if (marchCminus( (uint8_t *) INTERNAL_SRAM_START + current_section * CLASSB_SEC_SIZE, classb_buffer, CLASSB_SEC_SIZE))
			return 0;
	}

	return 1;

}

/*__attribute__ ((section (".classb_sram_buffer")))
 * SRAM memory.
 *
 *  \param p_sram    Pointer to first byte in memory area to be tested
 *  \param p_buffer  Pointer to first byte in the buffer
 *  \param size      Size of area to be tested in bytes.
 *
 */
int marchCminus(register volatile uint8_t * p_sram, register volatile uint8_t * p_buffer, register uint16_t size) {

	register uint16_t i = 0;

	/* Save content of the section: copy to buffer unless we test the buffer */
	if (p_buffer != p_sram)
		for (uint16_t i = 0; i < size; i++)
			*(p_buffer + i) = *(p_sram + i);

	/*
	 * Implemetação do MARCH C-
	 */

	/* Test phase 1: Write zeros UP */
	for (i = 0; i < size; i++) {
		*(p_sram + i) = ZERO;

		/* induzir erro */
		/*
		if((p_sram +i) == (uint8_t *) 0x08FF)
			*(p_sram+i) = 0x22;
		*/
	}

	/* Test phase 2: read ZERO, write ONE. UP */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ZERO)
			return 1;

		*(p_sram + i) = ONE;
	}

	/* Test phase 3: read ONE, write ZERO. UP. */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ONE)
			return 1;

		*(p_sram + i) = ZERO;
	}

	/* Test phase 4: read ZERO, write ONE. DOWN */
	for (i = size; i > 0; i--) {
		if (*(p_sram + i - 1) != ZERO)
			return 1;

		*(p_sram + i - 1) = ONE;
	}

	/* Test phase 5: read ONE, write ZERO. DOWN */
	for (i = size; i > 0; i--) {
		if (*(p_sram + i - 1) != ONE)
			return 1;

		*(p_sram + i - 1) = ZERO;
	}

	/* Test phase 6: read ZERO. UP */
	for (i = 0; i < size; i++) {
		if (*(p_sram + i) != ZERO)
			return 1;
	}

	/* Restore content of the section: copy from buffer, unless buffer is tested */
	if (p_buffer != p_sram)
		for (i = 0; i < size; i++)
			*(p_sram + i) = *(p_buffer + i);

	return 0;
}


/*****************************************************************************************************
 * Funções implementadas por mim
 *****************************************************************************************************/

uint8_t memory_test_flash_online(void){

	uint16_t hash = 0;
	uint16_t n_words = 256*64;
	uint16_t i;

	for (i = 0; i < n_words; i++) {
		hash ^= pgm_read_word(i);
	}

	if(SIGNATURE != eeprom_read_word(&sign_eeprom)){
		eeprom_update_word(&hash_flash_offline, hash);
		eeprom_update_word(&sign_eeprom, SIGNATURE);
		return 0;
	}

	if(hash != eeprom_read_word(&hash_flash_offline)){
		return 1;
	}

	return 0;

}

uint8_t memory_test_flash_offline(void){

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
		if (hash[i] != eeprom_read_byte(hash_flash_online + i)) {
			return 0;
		}
	}

	return 1;
}

/****************************************************************************
 * Não usadas
 ****************************************************************************/
uint8_t memory_test_SRAM(void){
	/* uint8_t* base_addrs = (uint8_t*) 0x0100; */

	uint8_t base_addrs[2100];

	/*
	 * Implementation of MARCH C-
	 */

	write_all_zero(base_addrs, DOWN, INTERNAL_SRAM_SIZE);

	if (read_zero_write_one(base_addrs, UP, INTERNAL_SRAM_SIZE)){
		return 1;
	}

	if (read_one_write_zero(base_addrs, UP, INTERNAL_SRAM_SIZE)){
		return 1;
	}

	if (read_zero_write_one(base_addrs, DOWN, INTERNAL_SRAM_SIZE)){
		return 1;
	}

	if (read_one_write_zero(base_addrs, DOWN, INTERNAL_SRAM_SIZE)){
		return 1;
	}

	if (read_all_zero(base_addrs, DOWN, INTERNAL_SRAM_SIZE)){
		return 1;
	}

	return 0;
}

uint8_t read_one_write_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes){
	int16_t i;

	switch (up_or_down) {
	case UP:
		for (i = 0; i <= (nBytes - 1); i++) {
			if (ONE == base_addr[i]) {
				return base_addr[i];
			}
			base_addr[i] = ZERO;
		}
		break;

	case DOWN:
		for (i = nBytes - 1; i >= 0; i--) {
			if (ONE == base_addr[i]) {
				return base_addr[i];
			}
			base_addr[i] = ZERO;
		}

		break;
	}

	return 0;
}

uint8_t read_zero_write_one(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes){
	int16_t i;

	switch (up_or_down) {
	case UP:
		for (i = 0; i <= (nBytes - 1); i++) {
			if (base_addr[i]) {
				return base_addr[i];
			}
			base_addr[i] = ONE;
		}
		break;

	case DOWN:
		for (i = nBytes - 1; i >= 0; i--) {
			if (base_addr[i]) {
				return base_addr[i];
			}
			base_addr[i] = ONE;
		}

		break;
	}

	return 0;
}

uint8_t read_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes) {
	int16_t i;

	switch (up_or_down) {
	case UP:
		for (i = 0; i <= (nBytes - 1); i++) {
			if (base_addr[i]) {
				return base_addr[i];
			}
		}
		break;

	case DOWN:
		for (i = nBytes - 1; i >= 0; i--) {
			if (base_addr[i]) {
				return base_addr[i];
			}
		}
		break;
	}

	return 0;
}

void write_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes) {
	/* uint8_t march[2048]; */
	int16_t i;

	for (i = 0; i < 2048; i++){
		base_addr[i] = ZERO;
	}

/*
	int16_t i;

	switch (up_or_down) {
		case UP:
			for (i = 0; i <= (nBytes - 1); i++) {
				base_addr[i] = ZERO;
			}
			break;

		case DOWN:
			for (i = (nBytes - 1); i >= 0; i--) {
				if((base_addr + 8*i) == &i){
					i = i+2;
					continue;
				}
				base_addr[i] = ZERO;
			}
			break;
	}
*/

	return;
}


/*
void write_all_zero(uint8_t* base_addr, uint8_t up_or_down, uint16_t nBytes) {
	uint8_t* base_addr;

	switch (up_or_down) {
		case UP:
			for (base_addr = (uint8_t*) 0x0100; base_addr < (uint8_t*) 0x08FF; base_addr++) {
				*base_addr = ZERO;
			}
			break;

		case DOWN:
			for (base_addr = 0x0800; base_addr >= (uint8_t*) 0x010A; base_addr--) {
				if(base_addr == &base_addr){
					continue;
				}
				*base_addr = ZERO;
			}
			break;
	}

	return;
}*/

