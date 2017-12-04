#include "memory_test.h"

uint16_t EEMEM sign_eeprom, hash_flash;

uint8_t memory_test_FLASH(void){

	uint16_t hash = 0;
	int16_t i = 0;
	int16_t n_words = 256 * 64;

	for (i = 0; i < n_words; i++) {
		hash ^= pgm_read_word_near(i);
	}

	if (eeprom_read_word(&sign_eeprom) != SIGNATURE) {
		eeprom_update_word(&hash_flash, hash);
		eeprom_update_word(&sign_eeprom, SIGNATURE);
	}

	return (hash == eeprom_read_word(&hash_flash));

}

uint8_t memory_test_SRAM(void){
	//uint8_t* base_addrs = (uint8_t*) 0x0100;

	uint8_t base_addrs[2100];

	/*
	 * Implementation of MARCH C-
	 */

	write_all_zero(base_addrs, DOWN, MEMORY_SIZE);

	if (read_zero_write_one(base_addrs, UP, MEMORY_SIZE)){
		return 1;
	}

	if (read_one_write_zero(base_addrs, UP, MEMORY_SIZE)){
		return 1;
	}

	if (read_zero_write_one(base_addrs, DOWN, MEMORY_SIZE)){
		return 1;
	}

	if (read_one_write_zero(base_addrs, DOWN, MEMORY_SIZE)){
		return 1;
	}

	if (read_all_zero(base_addrs, DOWN, MEMORY_SIZE)){
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
	//uint8_t march[2048];
	int16_t i;

	for (i = 0; i < 2048; i++){
		base_addr[i] = ZERO;
	}

//	int16_t i;
//
//	switch (up_or_down) {
//		case UP:
//			for (i = 0; i <= (nBytes - 1); i++) {
//				base_addr[i] = ZERO;
//			}
//			break;
//
//		case DOWN:
//			for (i = (nBytes - 1); i >= 0; i--) {
//				if((base_addr + 8*i) == &i){
//					i = i+2;
//					continue;
//				}
//				base_addr[i] = ZERO;
//			}
//			break;
//	}

	return;
}

//void write_all_zero(/*uint8_t* base_addr, */uint8_t up_or_down/*, uint16_t nBytes*/) {
//	uint8_t* base_addr;
//
//	switch (up_or_down) {
//		case UP:
//			for (base_addr = (uint8_t*) 0x0100; base_addr < (uint8_t*) 0x08FF; base_addr++) {
//				*base_addr = ZERO;
//			}
//			break;
//
//		case DOWN:
//			for (base_addr = 0x0800; base_addr >= (uint8_t*) 0x010A; base_addr--) {
//				if(base_addr == &base_addr){
//					continue;
//				}
//				*base_addr = ZERO;
//			}
//			break;
//	}
//
//	return;
//}
