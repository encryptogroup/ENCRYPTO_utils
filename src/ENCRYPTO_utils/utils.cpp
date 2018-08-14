#include "utils.h"

#include <cstdint>
#include <fcntl.h>
#include <gmp.h>
#include <iostream>
#include <unistd.h>


//TODO: this is bad, fix occurrences of ceil_log2 and replace by ceil_log2_min1 where log(1) = 1 is necessary. For all else use ceil_log2_real
uint32_t ceil_log2(int bits) {
	if (bits == 1)
		return 1;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

uint32_t ceil_log2_min1(int bits) {
	if (bits <= 1)
		return 1;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

uint32_t ceil_log2_real(int bits) {
	if (bits == 1)
		return 0;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

uint32_t floor_log2(int bits) {
	if (bits == 1)
		return 1;
	int targetlevel = 0;
	while (bits >>= 1)
		++targetlevel;
	return targetlevel;
}

/**
 * returns a 4-byte value from dev/random
 */
uint32_t aby_rand() {
	int frandom = open("/dev/random", O_RDONLY);
	if (frandom < 0) {
		std::cerr << "Error in opening /dev/random: utils.h:aby_rand()" << std::endl;
		exit(1);
	} else {
		char data[4];
		size_t len = 0;
		while (len < sizeof data) {
			ssize_t result = read(frandom, data + len, (sizeof data) - len);
			if (result < 0) {
				std::cerr << "Error in generating random number: utils.h:aby_rand()" << std::endl;
				exit(1);
			}
			len += result;
		}
		close(frandom);
		return *((uint32_t*) data);
	}
	return 0;
}

/**
 * returns a random mpz_t with bitlen len generated from dev/urandom
 */
void aby_prng(mpz_t rnd, mp_bitcnt_t len) {
	size_t byte_count = ceil_divide(len, 8);
	char * data;

	int furandom = open("/dev/urandom", O_RDONLY);
	if (furandom < 0) {
		std::cerr << "Error in opening /dev/urandom: utils.h:aby_prng()" << std::endl;
		exit(1);
	} else {
		data = (char*) malloc(sizeof(*data) * byte_count);
		size_t len = 0;
		while (len < byte_count) {
			ssize_t result = read(furandom, data + len, byte_count - len);
			if (result < 0) {
				std::cerr << "Error in generating random number: utils.h:aby_prng()" << std::endl;
				exit(1);
			}
			len += result;
		}
		close(furandom);
	}

	mpz_import(rnd, byte_count, 1, sizeof(*data), 0, 0, data);
	free(data);
}
