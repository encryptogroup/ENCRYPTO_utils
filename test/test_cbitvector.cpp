#include <gtest/gtest.h>
#include "ENCRYPTO_utils/cbitvector.h"


TEST(TestCBitVector, Create){
	
	// check padding to AES bit size bis in Create()
	CBitVector v2;
	v2.Create(129);
	ASSERT_EQ(v2.GetSize(), 32);
}

TEST(TestCBitVector, SetBitsPosOffset) {

	auto read_bits = [] (const CBitVector& v) {
		uint16_t bits;
		v.GetBits(reinterpret_cast<uint8_t*>(&bits), 0, 16);
		return bits;
	};

	// creating zero vector
	CBitVector v;
	v.CreateZeros(16);

	ASSERT_EQ(read_bits(v), 0b0000000000000000);

	// Setting first two bits
	v.SetBitNoMask(0, 1);
	v.SetBitNoMask(1, 1);

	ASSERT_EQ(read_bits(v), 0b0000000000000011);

	// Setting bits 6-9 by other all-one byte array, accessing from bit 3
	uint8_t all_one[1] = {0xff};
	v.SetBitsPosOffset(all_one, 3, 6, 4);

	ASSERT_EQ(read_bits(v), 0b0000001111000011);

	// Setting bits 0-5 by other alternating byte array, accessing from bit 2
	uint8_t alternating[1] = {0b01010101};
	v.SetBitsPosOffset(alternating, 2, 0, 6);

	ASSERT_EQ(read_bits(v), 0b0000001111010101);
}

