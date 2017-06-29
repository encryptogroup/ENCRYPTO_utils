#ifndef _UTILS_H__
#define _UTILS_H__

#include "typedefs.h"

#define two_pow(e) (((uint64_t) 1) << (e))

#define pad_to_power_of_two(e) ( ((uint64_t) 1) << (ceil_log2(e)) )

/*compute (a-b) mod (m+1) as: b > a ? (m) - (b-1) + a : a - b	*/
#define MOD_SUB(a, b, m) (( ((b) > (a))? (m) - ((b) -1 ) + a : a - b))

#define ceil_divide(x, y)			(( ((x) + (y)-1)/(y)))
#define bits_in_bytes(bits) (ceil_divide((bits), 8))
#define pad_to_multiple(x, y) 		( ceil_divide(x, y) * (y))

#define PadToRegisterSize(x) 		(PadToMultiple(x, OTEXT_BLOCK_SIZE_BITS))
#define PadToMultiple(x, y) 		( ceil_divide(x, y) * (y))

//TODO: this is bad, fix occurrences of ceil_log2 and replace by ceil_log2_min1 where log(1) = 1 is necessary. For all else use ceil_log2_real
static uint32_t ceil_log2(int bits) {
	if (bits == 1)
		return 1;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

static uint32_t ceil_log2_min1(int bits) {
	if (bits <= 1)
		return 1;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

static uint32_t ceil_log2_real(int bits) {
	if (bits == 1)
		return 0;
	int targetlevel = 0, bitstemp = bits;
	while (bitstemp >>= 1)
		++targetlevel;
	return targetlevel + ((1 << targetlevel) < bits);
}

static uint32_t floor_log2(int bits) {
	if (bits == 1)
		return 1;
	int targetlevel = 0;
	while (bits >>= 1)
		++targetlevel;
	return targetlevel;
}

#endif // _UTILS_H__
