/**
 \file 		cbitvector.cpp
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 ENCRYPTO Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.
            ABY is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU Lesser General Public License for more details.
            You should have received a copy of the GNU Lesser General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		CBitVector Implementation
 */

#include "cbitvector.h"
#include "crypto/crypto.h"
#include "utils.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <cstring>


namespace {

/** Array which stores the bytes which are reversed. For example, the hexadecimal 0x01 is when reversed becomes 0x80.  */
constexpr BYTE REVERSE_BYTE_ORDER[256] = { 0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8,
		0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C,
		0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2,
		0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96,
		0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1,
		0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85,
		0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD,
		0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B,
		0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF,
		0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF };

/**
	This array is used by \link XORBits(BYTE* p, int pos, int len) \endlink and \link SetBits(BYTE* p, uint64_t pos, uint64_t len) \endlink
    method for lower bit mask.
*/
constexpr BYTE RESET_BIT_POSITIONS[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };
/**
	This array is used by \link XORBits(BYTE* p, int pos, int len) \endlink and \link SetBits(BYTE* p, uint64_t pos, uint64_t len) \endlink
    method for upper bit mask.
*/
constexpr BYTE RESET_BIT_POSITIONS_INV[9] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };

/** This array is used by \link GetBits(BYTE* p, int pos, int len) \endlink method for lower bit mask. */
constexpr BYTE GET_BIT_POSITIONS[9] = { 0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00 };

/** This array is used by \link GetBits(BYTE* p, int pos, int len) \endlink method for upper bit mask. */
constexpr BYTE GET_BIT_POSITIONS_INV[9] = { 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

/**
	This array is used for masking bits and extracting a particular positional bit from the provided byte array.
	This array is used by \link GetBit(int idx) \endlink method.
*/
constexpr BYTE MASK_BIT[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 };

/**
	This array is used for extracting a particular positional bit from the provided byte array without masking.
	This array is used by \link GetBitNoMask(int idx) \endlink method.
*/
static constexpr BYTE BIT[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };

/**
	This array is used for masking bits and setting a particular positional bit from the provided byte array in the CBitVector.
	This array is used by \link SetBit(int idx, BYTE b) \endlink and \link ANDBit(int idx, BYTE b) \endlink methods.
*/
constexpr BYTE CMASK_BIT[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

/**
	This array is used for setting a particular positional bit from the provided byte array without masking in the CBitVector.
	This array is used by \link SetBitNoMask(int idx, BYTE b) \endlink and \link ANDBitNoMask(int idx, BYTE b) \endlink methods.
*/
constexpr BYTE C_BIT[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

/**
	This array is used for masking bits and setting a particular positional bit from the provided byte array in the CBitVector.
	This array is used by \link SetBit(int idx, BYTE b) \endlink and \link XORBit(int idx, BYTE b) \endlink methods.
*/
constexpr BYTE MASK_SET_BIT_C[2][8] = { { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1 }, { 0, 0, 0, 0, 0, 0, 0, 0 } };

/**
	This array is used for setting a particular positional bit from the provided byte array without masking in the CBitVector.
	This array is used by \link SetBitNoMask(int idx, BYTE b) \endlink and \link XORBitNoMask(int idx, BYTE b) \endlink methods.
*/
constexpr BYTE SET_BIT_C[2][8] = { { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 }, { 0, 0, 0, 0, 0, 0, 0, 0 } };

const BYTE SELECT_BIT_POSITIONS[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

#if (__WORDSIZE==32)
constexpr REGISTER_SIZE TRANSPOSITION_MASKS[6] =
{	0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};
constexpr REGISTER_SIZE TRANSPOSITION_MASKS_INV[6] =
{	0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
#else
#if (__WORDSIZE==64)
/** Transposition mask used for Eklund Bit Matrix Transposition.*/
constexpr REGISTER_SIZE TRANSPOSITION_MASKS[6] = { 0x5555555555555555, 0x3333333333333333, 0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF, 0x00000000FFFFFFFF };
constexpr REGISTER_SIZE TRANSPOSITION_MASKS_INV[6] = { 0xAAAAAAAAAAAAAAAA, 0xCCCCCCCCCCCCCCCC, 0xF0F0F0F0F0F0F0F0, 0xFF00FF00FF00FF00, 0xFFFF0000FFFF0000, 0xFFFFFFFF00000000 };
#else
#endif
#endif

constexpr size_t SHIFTVAL = 3;


template<class T> void GetBytes(T* dst, const T* src, const T* lim) {
	while (dst != lim) {
		*dst++ = *src++;
	}
}

template<class T> void SetBytes(T* dst, const T* src, const T* lim) {
	while (dst < lim) {
		*dst++ = *src++;
	}
}

//Generic bytewise XOR operation
template<class T> void XORBytes(T* dst, const T* src, const T* lim) {
	while (dst != lim) {
		*dst++ ^= *src++;
	}
}

template<class T> void ANDBytes(T* dst, const T* src, const T* lim) {
	while (dst != lim) {
		*dst++ &= *src++;
	}
}

constexpr BYTE GetArrayBit(const BYTE* p, size_t idx) {
	return 0 != (p[idx >> 3] & BIT[idx & 0x7]);
}

} // namespace


CBitVector::CBitVector() {
	Init();
}

CBitVector::CBitVector(std::size_t bits) {
	Init();
	Create(bits);
}

CBitVector::CBitVector(std::size_t bits, crypto* crypt) {
	Init();
	Create(bits, crypt);
}

void CBitVector::Init() {
	m_pBits = NULL;
	m_nByteSize = 0;
}

CBitVector::~CBitVector(){
	delCBitVector();
};

void CBitVector::delCBitVector() {
	if (( m_nByteSize > 0 )&& (m_pBits != NULL)) {
		free(m_pBits);
	}
	m_nByteSize = 0;
	m_pBits = NULL;
}

/* Fill random values using the pre-defined AES key */
void CBitVector::FillRand(std::size_t bits, crypto* crypt) {
	if (bits > m_nByteSize << 3)
		Create(bits);
	crypt->gen_rnd(m_pBits, ceil_divide(bits, 8));
}

void CBitVector::CreateExact(std::size_t bits) {
	if (bits == 0){
		bits = AES_BITS;
	}

	// if memory was previously allocated: free it
	if (m_nByteSize > 0) {
		free(m_pBits);
	}

	m_nByteSize = ceil_divide(bits, 8);
	m_pBits = (BYTE*) calloc(m_nByteSize, sizeof(BYTE));
	assert(m_pBits != NULL);

	m_nElementLength = 1;
	m_nNumElements = m_nByteSize;
	m_nNumElementsDimB = 1;
}

void CBitVector::Create(std::size_t bits) {
	//TODO: check if padding to AES_BITS is still necessary as default
	CreateExact(ceil_divide(bits, AES_BITS) * AES_BITS);
}

void CBitVector::CreateBytes(std::size_t bytes) {
	Create(bytes << 3);
}

void CBitVector::CreateBytes(std::size_t bytes, crypto* crypt) {
	Create(bytes << 3, crypt);
}

void CBitVector::CreateZeros(std::size_t bits) {
	Create(bits);
	memset(m_pBits, 0, m_nByteSize);
}

void CBitVector::Create(std::size_t bits, crypto* crypt) {
	Create(bits);
	FillRand(bits, crypt);
}

void CBitVector::Create(std::size_t numelements, std::size_t elementlength) {
	Create(numelements * elementlength);
	m_nElementLength = elementlength;
	m_nNumElements = numelements;
	m_nNumElementsDimB = 1;
}

void CBitVector::Create(std::size_t numelements, std::size_t elementlength, crypto* crypt) {
	Create(numelements * elementlength, crypt);
	m_nElementLength = elementlength;
	m_nNumElements = numelements;
	m_nNumElementsDimB = 1;
}

void CBitVector::Create(std::size_t numelementsDimA, std::size_t numelementsDimB, std::size_t elementlength) {
	Create(numelementsDimA * numelementsDimB * elementlength);
	m_nElementLength = elementlength;
	m_nNumElements = numelementsDimA;
	m_nNumElementsDimB = numelementsDimB;
}
void CBitVector::Create(std::size_t numelementsDimA, std::size_t numelementsDimB, std::size_t elementlength, crypto* crypt) {
	Create(numelementsDimA * numelementsDimB * elementlength, crypt);
	m_nElementLength = elementlength;
	m_nNumElements = numelementsDimA;
	m_nNumElementsDimB = numelementsDimB;
}

void CBitVector::ResizeinBytes(std::size_t newSizeBytes) {
	BYTE* tBits = m_pBits;
	uint64_t tSize = (m_nByteSize<newSizeBytes)? m_nByteSize:newSizeBytes; //fix for overflow condition in memcpy.

	m_nByteSize = newSizeBytes;
	m_pBits = (uint8_t*) calloc(m_nByteSize, sizeof(uint8_t));

	memcpy(m_pBits, tBits, tSize);

	free(tBits);
}

void CBitVector::Reset() {
	memset(m_pBits, 0, m_nByteSize);
}

void CBitVector::ResetFromTo(std::size_t frombyte, std::size_t tobyte) {
	assert(frombyte <= tobyte);
	assert(tobyte < m_nByteSize);
	memset(m_pBits + frombyte, 0, tobyte - frombyte);
}

void CBitVector::SetToOne() {
	memset(m_pBits, 0xFF, m_nByteSize);
}

void CBitVector::Invert() {
	for(std::size_t i = 0; i < m_nByteSize; i++) {
		m_pBits[i] = ~m_pBits[i];
	}
}

std::size_t CBitVector::GetSize() const {
	return m_nByteSize;
}

BOOL CBitVector::IsEqual(const CBitVector& vec) const {
	if (vec.GetSize() != m_nByteSize) {
		return false;
	}

	const BYTE* ptr = vec.GetArr();
	for (std::size_t i = 0; i < m_nByteSize; i++) {
		if (ptr[i] != m_pBits[i]) {
			return false;
		}
	}
	return true;
}

BOOL CBitVector::IsEqual(const CBitVector& vec, std::size_t from, std::size_t to) const {
	if (vec.GetSize() * 8 < to || m_nByteSize * 8 < to || from > to) {
		return false;
	}

	for (std::size_t i = from; i < to; i++) {
		if (vec.GetBit(i) != GetBit(i)) {
			return false;
		}
	}
	return true;
}

void CBitVector::SetElementLength(std::size_t elelen) {
	m_nElementLength = elelen;
}

std::size_t CBitVector::GetElementLength() const {
	return m_nElementLength;
}

void CBitVector::Copy(const CBitVector& vec) {
	Copy(vec.GetArr(), 0, vec.GetSize());
}

void CBitVector::Copy(const CBitVector& vec, std::size_t pos, std::size_t len) {
	Copy(vec.GetArr(), pos, len);
}

void CBitVector::Copy(const BYTE* p, std::size_t pos, std::size_t len) {
	if (pos + len > m_nByteSize) {
		if (m_pBits)
			ResizeinBytes(pos + len);
		else {
			CreateBytes(pos + len);
		}
	}
	memcpy(m_pBits + pos, p, len);
}

void CBitVector::ORByte(std::size_t pos, BYTE p) {
	assert(pos <= m_nByteSize);
	m_pBits[pos] |= p;
}

BYTE CBitVector::GetBit(std::size_t idx) const {
	assert(idx < (m_nByteSize << 3));
	return !!(m_pBits[idx >> 3] & MASK_BIT[idx & 0x7]);
}

void CBitVector::SetBit(std::size_t idx, BYTE b) {
	assert(idx < (m_nByteSize << 3));
	m_pBits[idx >> 3] = (m_pBits[idx >> 3] & CMASK_BIT[idx & 0x7]) | MASK_SET_BIT_C[!(b & 0x01)][idx & 0x7];
}

BYTE CBitVector::GetBitNoMask(std::size_t idx) const {
	assert(idx < (m_nByteSize << 3));
	return GetArrayBit(m_pBits, idx);
}

void CBitVector::SetBitNoMask(std::size_t idx, BYTE b) {
	assert(idx < (m_nByteSize << 3));
	m_pBits[idx >> 3] = (m_pBits[idx >> 3] & C_BIT[idx & 0x7]) | SET_BIT_C[!(b & 0x01)][idx & 0x7];
}

void CBitVector::XORBitNoMask(std::size_t idx, BYTE b) {
	assert(idx < (m_nByteSize << 3));
	m_pBits[idx >> 3] ^= SET_BIT_C[!(b & 0x01)][idx & 0x7];
}

void CBitVector::SetByte(std::size_t idx, BYTE p) {
	assert(idx < m_nByteSize);
	m_pBits[idx] = p;
}

BYTE CBitVector::GetByte(std::size_t idx) const {
	assert(idx < m_nByteSize);
	return m_pBits[idx];
}

void CBitVector::XORByte(std::size_t idx, BYTE b) {
	assert(idx < m_nByteSize);
	m_pBits[idx] ^= b;
}

void CBitVector::ANDByte(std::size_t idx, BYTE b) {
	assert(idx < m_nByteSize);
	m_pBits[idx] &= b;
}

void CBitVector::GetBits(BYTE* p, std::size_t pos, std::size_t len) const {
	if (len < 1 || (pos + len) > (m_nByteSize << 3)) {
		return;
	}
	if (len == 1) {
		*p = GetBitNoMask(pos);
		return;
	}

	if (!((pos & 0x07) || (len & 0x07))) {
		GetBytes(p, pos >> 3, len >> 3);
		return;
	}

	int posctr = pos >> 3;
	int lowermask = pos & 7;
	int uppermask = 8 - lowermask;

	std::size_t i;
	for (i = 0; i < len / (sizeof(BYTE) * 8); i++, posctr++) {
		p[i] = ((m_pBits[posctr] & GET_BIT_POSITIONS[lowermask]) >> lowermask) & 0xFF;
		p[i] |= (m_pBits[posctr + 1] & GET_BIT_POSITIONS_INV[uppermask]) << uppermask;
	}
	int remlen = len & 0x07;
	if (remlen) {
		if (remlen <= uppermask) {
			p[i] = ((m_pBits[posctr] & ((((1 << remlen) - 1) << lowermask))) >> lowermask) & 0xFF;
		} else {
			p[i] = ((m_pBits[posctr] & GET_BIT_POSITIONS[lowermask]) >> lowermask) & 0xFF;
			p[i] |= (m_pBits[posctr + 1] & (((1 << (remlen - uppermask)) - 1))) << uppermask;
		}
	}
}


//optimized bytewise for set operation
void CBitVector::GetBytes(BYTE* p, std::size_t pos, std::size_t len) const {
	assert(pos+len <= m_nByteSize);
	BYTE* src = m_pBits + pos;
	BYTE* dst = p;
	//Do many operations on REGSIZE types first and then (if necessary) use bytewise operations
	::GetBytes((REGSIZE*) dst, (REGSIZE*) src, ((REGSIZE*) dst) + (len >> SHIFTVAL));
	dst += ((len >> SHIFTVAL) << SHIFTVAL);
	src += ((len >> SHIFTVAL) << SHIFTVAL);
	::GetBytes(dst, src, dst + (len & ((1 << SHIFTVAL) - 1)));
}
//
//pos and len in bits
void CBitVector::SetBits(const BYTE* p, std::size_t pos, std::size_t len) {
	if (len < 1 || (pos + len) > (m_nByteSize << 3)){
		return;
	}

	if (len == 1) {
		SetBitNoMask(pos, *p);
		return;
	}
	if (!((pos & 0x07) || (len & 0x07))) {

		SetBytes(p, pos >> 3, len >> 3);
		return;
	}
	std::size_t posctr = pos >> 3;
	int lowermask = pos & 7;
	int uppermask = 8 - lowermask;

	std::size_t i;
	BYTE temp;
	for (i = 0; i < len / (sizeof(BYTE) * 8); i++, posctr++) {
		temp = p[i];
		m_pBits[posctr] = (m_pBits[posctr] & RESET_BIT_POSITIONS[lowermask]) | ((temp << lowermask) & 0xFF);
		m_pBits[posctr + 1] = (m_pBits[posctr + 1] & RESET_BIT_POSITIONS_INV[uppermask]) | (temp >> uppermask);
	}
	int remlen = len & 0x07;
	if (remlen) {
		temp = p[i] & RESET_BIT_POSITIONS[remlen];
		if (remlen <= uppermask) {
			m_pBits[posctr] = (m_pBits[posctr] & (~(((1 << remlen) - 1) << lowermask))) | ((temp << lowermask) & 0xFF);
		} else {
			m_pBits[posctr] = (m_pBits[posctr] & RESET_BIT_POSITIONS[lowermask]) | ((temp << lowermask) & 0xFF);
			m_pBits[posctr + 1] = (m_pBits[posctr + 1] & (~(((1 << (remlen - uppermask)) - 1)))) | (temp >> uppermask);
		}
	}
}


//Set bits given an offset on the bits for p which is not necessarily divisible by 8
void CBitVector::SetBitsPosOffset(const BYTE* p, std::size_t ppos, std::size_t pos, std::size_t len) {
	for (auto i = pos, j = ppos; j < ppos + len; i++, j++) {
		BYTE source_bit = GetArrayBit(p, j);
		SetBitNoMask(i, source_bit);

	}
}

//optimized bytewise for set operation
void CBitVector::SetBytes(const BYTE *src, std::size_t pos, std::size_t len) {
	assert(pos + len <= m_nByteSize);

	BYTE *dst = m_pBits + pos;

	//Do many operations on REGSIZE types first and then (if necessary) use bytewise operations
	::SetBytes((REGSIZE*) dst, (REGSIZE*) src, ((REGSIZE*) dst) + (len >> SHIFTVAL));
	dst += ((len >> SHIFTVAL) << SHIFTVAL);
	src += ((len >> SHIFTVAL) << SHIFTVAL);
	::SetBytes(dst, src, dst + (len & ((1 << SHIFTVAL) - 1)));
}

void CBitVector::SetBytesToZero(std::size_t bytepos, std::size_t bytelen) {
	assert(bytepos + bytelen <= m_nByteSize);
	memset(m_pBits + bytepos, 0x00, bytelen);
}


void CBitVector::SetBitsToZero(std::size_t bitpos, std::size_t bitlen) {
	int firstlim = ceil_divide(bitpos, 8);
	int firstlen = ceil_divide(bitlen - (bitpos % 8), 8);
	for (int i = bitpos; i < firstlim; i++) {
		SetBitNoMask(i, 0);
	}
	if (bitlen > 7) {
		memset(m_pBits + firstlim, 0, firstlen);
	}
	for (std::size_t i = (firstlim + firstlen) << 8; i < bitpos + bitlen; i++) {
		SetBitNoMask(i, 0);
	}
}

//optimized bytewise XOR operation
void CBitVector::XORBytes(const BYTE* p, std::size_t pos, std::size_t len) {
	if(pos + len > m_nByteSize)
	std::cout << "pos = " << pos << ", len = " << len << ", bytesize = " << m_nByteSize << std::endl;
	assert(pos + len <= m_nByteSize);

	BYTE* dst = m_pBits + pos;
	const BYTE* src = p;
	//Do many operations on REGSIZE types first and then (if necessary) use bytewise operations
	::XORBytes((REGSIZE*) dst, (REGSIZE*) src, ((REGSIZE*) dst) + (len >> SHIFTVAL));
	dst += ((len >> SHIFTVAL) << SHIFTVAL);
	src += ((len >> SHIFTVAL) << SHIFTVAL);
	::XORBytes(dst, src, dst + (len & ((1 << SHIFTVAL) - 1)));
}

void CBitVector::XORBytes(const BYTE* p, std::size_t len) {
	XORBytes(p, 0, len);
}

void CBitVector::XORVector(const CBitVector &vec, std::size_t pos, std::size_t len) {
	XORBytes(vec.GetArr(), pos, len);
}

void CBitVector::XORBits(const BYTE* p, std::size_t pos, std::size_t len) {
	if (len < 1 || (pos + len) > m_nByteSize << 3) {
		return;
	}
	if (len == 1) {
		XORBitNoMask(pos, *p);
		return;
	}
	if (!((pos & 0x07) || (len & 0x07))) {
		XORBytes(p, pos >> 3, len >> 3);
		return;
	}
	int posctr = pos >> 3;
	int lowermask = pos & 7;
	int uppermask = 8 - lowermask;

	std::size_t i;
	BYTE temp;
	for (i = 0; i < len / (sizeof(BYTE) * 8); i++, posctr++) {
		temp = p[i];
		m_pBits[posctr] ^= ((temp << lowermask) & 0xFF);
		m_pBits[posctr + 1] ^= (temp >> uppermask);
	}
	int remlen = len & 0x07;
	if (remlen) {
		temp = p[i] & RESET_BIT_POSITIONS[remlen];
		if (remlen <= uppermask) {
			m_pBits[posctr] ^= ((temp << lowermask) & 0xFF);
		} else {
			m_pBits[posctr] ^= ((temp << lowermask) & 0xFF);
			m_pBits[posctr + 1] ^= (temp >> uppermask);
		}
	}
}

//XOR bits given an offset on the bits for p which is not necessarily divisible by 8
void CBitVector::XORBitsPosOffset(const BYTE* p, std::size_t ppos, std::size_t pos, std::size_t len) {
	assert((pos + len) <= (m_nByteSize<<3));
	for (auto i = pos, j = ppos; j < ppos + len; i++, j++) {
		m_pBits[i / 8] ^= (((p[j / 8] & (1 << (j % 8))) >> j % 8) << i % 8);
	}
}

//Method for directly XORing CBitVectors
void CBitVector::XOR(const CBitVector* b) {
	assert(b->GetSize() == m_nByteSize);
	XORBytes(b->GetArr(), 0, m_nByteSize);
}

void CBitVector::XORBytesReverse(const BYTE* p, std::size_t pos, std::size_t len) {
	assert((pos + len) <= m_nByteSize);
	const BYTE* src = p;
	BYTE* dst = m_pBits + pos;
	BYTE* lim = dst + len;
	while (dst != lim) {
		*dst++ ^= REVERSE_BYTE_ORDER[*src++];
	}
}


//optimized bytewise for AND operation
void CBitVector::ANDBytes(const BYTE* p, std::size_t pos, std::size_t len) {
	assert(pos+len <= m_nByteSize);
	BYTE* dst = m_pBits + pos;
	const BYTE* src = p;
	//Do many operations on REGSIZE types first and then (if necessary) use bytewise operations
	::ANDBytes((REGSIZE*) dst, (REGSIZE*) src, ((REGSIZE*) dst) + (len >> SHIFTVAL));
	dst += ((len >> SHIFTVAL) << SHIFTVAL);
	src += ((len >> SHIFTVAL) << SHIFTVAL);
	::ANDBytes(dst, src, dst + (len & ((1 << SHIFTVAL) - 1)));
}

void CBitVector::SetXOR(const BYTE* p, const BYTE* q, std::size_t pos, std::size_t len) {
	Copy(p, pos, len);
	XORBytes(q, pos, len);
}

void CBitVector::SetAND(const BYTE* p, const BYTE* q, std::size_t pos, std::size_t len) {
	Copy(p, pos, len);
	ANDBytes(q, pos, len);
}

//Method for directly ANDing CBitVectors
void CBitVector::AND(const CBitVector* b) {
	assert(b->GetSize() == m_nByteSize);
	ANDBytes(b->GetArr(), 0, m_nByteSize);
}

//Cyclic left shift by pos bits
void CBitVector::CLShift(std::size_t pos) {
	uint8_t* tmpbuf = (uint8_t*) malloc(m_nByteSize);
	for(std::size_t i = 0; i < m_nByteSize; i++) {
		tmpbuf[i+pos] = m_pBits[i];
	}
	free(m_pBits);
	m_pBits = tmpbuf;
}

BYTE* CBitVector::GetArr() {
	return m_pBits;
}

const BYTE* CBitVector::GetArr() const {
	return m_pBits;
}

void CBitVector::AttachBuf(BYTE* p, std::size_t size) {
	m_pBits = p;
	m_nByteSize = size;
}


/**
	This method is used to detach the buffer from the CBitVector. */
void CBitVector::DetachBuf() {
	m_pBits = NULL;
	m_nByteSize = 0;
}


void CBitVector::Print(std::size_t fromBit, std::size_t toBit) {
	std::size_t to = toBit > (m_nByteSize << 3) ? (m_nByteSize << 3) : toBit;
	for (std::size_t i = fromBit; i < to; i++) {
		std::cout << (unsigned int) GetBitNoMask(i);
	}
	std::cout << std::endl;
}

void CBitVector::PrintHex(bool linebreak) {
	for (std::size_t i = 0; i < m_nByteSize; i++) {
		std::cout << std::setw(2) << std::setfill('0') << (std::hex) << ((unsigned int) m_pBits[i]);
	}
	if(linebreak){
		std::cout << (std::dec) << std::endl;
	}
}

void CBitVector::PrintHex(std::size_t fromByte, std::size_t toByte, bool linebreak) {
	std::size_t to = toByte > (m_nByteSize) ? (m_nByteSize) : toByte;

	for (std::size_t i = fromByte; i < to; i++) {
		std::cout << std::setw(2) << std::setfill('0') << (std::hex) << ((unsigned int) m_pBits[i]);
	}
	if(linebreak){
		std::cout << (std::dec) << std::endl;
	}
}

void CBitVector::PrintBinary() {
	Print(0, m_nByteSize << 3);
}

void CBitVector::PrintContent() {
	if (m_nElementLength == 1) {
		PrintHex();
		return;
	}
	if (m_nNumElementsDimB == 1) {
		for (std::size_t i = 0; i < m_nNumElements; i++) {
			std::cout << Get<int>(i) << ", ";
		}
		std::cout << std::endl;
	} else {
		for (std::size_t i = 0; i < m_nNumElements; i++) {
			std::cout << "(";
			for (std::size_t j = 0; j < m_nNumElementsDimB - 1; j++) {
				std::cout << Get2D<int>(i, j) << ", ";
			}
			std::cout << Get2D<int>(i, m_nNumElementsDimB - 1);
			std::cout << "), ";
		}
		std::cout << std::endl;
	}
}

void CBitVector::PrintBinaryMasked(std::size_t from, std::size_t to) {
	std::size_t new_to = to > (m_nByteSize<<3) ? (m_nByteSize<<3) : to;

	for (std::size_t i = from; i < new_to; i++) {
		std::cout << (unsigned int) GetBit(i);
	}
	std::cout << std::endl;
}

void CBitVector::Transpose(std::size_t rows, std::size_t columns) {
#ifdef SIMPLE_TRANSPOSE
	SimpleTranspose(rows, columns);
#else
	EklundhBitTranspose(rows, columns);
#endif
}

void CBitVector::SimpleTranspose(std::size_t rows, std::size_t columns) {
	CBitVector temp(rows * columns);
	temp.Copy(m_pBits, 0, rows * columns / 8);
	for (std::size_t i = 0; i < rows; i++) {
		for (std::size_t j = 0; j < columns; j++) {
			SetBit(j * rows + i, temp.GetBit(i * columns + j));
		}
	}
}

//A transposition algorithm for bit-matrices of size 2^i x 2^i
void CBitVector::EklundhBitTranspose(std::size_t rows, std::size_t columns) {
	REGISTER_SIZE* rowaptr;	//ptr;
	REGISTER_SIZE* rowbptr;
	REGISTER_SIZE temp_row;
	REGISTER_SIZE mask;
	REGISTER_SIZE invmask;
	REGISTER_SIZE* lim;

	lim = (REGISTER_SIZE*) m_pBits + ceil_divide(rows * columns, 8);

	std::size_t offset = (columns >> 3) / sizeof(REGISTER_SIZE);
	std::size_t numiters = ceil_log2(std::min(rows, columns));
	std::size_t srcidx = 1, destidx;
	std::size_t rounds;

	//If swapping is performed on bit-level
	for (std::size_t i = 0; i < LOG2_REGISTER_SIZE; i++, srcidx *= 2) {
		destidx = offset * srcidx;
		rowaptr = (REGISTER_SIZE*) m_pBits;
		rowbptr = rowaptr + destidx;

		//Preset the masks that are required for bit-level swapping operations
		mask = TRANSPOSITION_MASKS[i];
		invmask = ~mask;

		//If swapping is performed on byte-level reverse operations due to little-endian format.
		rounds = rows / (srcidx * 2);
		if (i > 2) {
			for (std::size_t j = 0; j < rounds; j++) {
				for (lim = rowbptr + destidx; rowbptr < lim; rowaptr++, rowbptr++) {
					temp_row = *rowaptr;
					*rowaptr = ((*rowaptr & mask) ^ ((*rowbptr & mask) << srcidx));
					*rowbptr = ((*rowbptr & invmask) ^ ((temp_row & invmask) >> srcidx));
				}
				rowaptr += destidx;
				rowbptr += destidx;
			}
		} else {
			for (std::size_t j = 0; j < rounds; j++) {
				for (lim = rowbptr + destidx; rowbptr < lim; rowaptr++, rowbptr++) {
					temp_row = *rowaptr;
					*rowaptr = ((*rowaptr & invmask) ^ ((*rowbptr & invmask) >> srcidx));
					*rowbptr = ((*rowbptr & mask) ^ ((temp_row & mask) << srcidx));
				}
				rowaptr += destidx;
				rowbptr += destidx;
			}
		}
	}

	for (std::size_t i = LOG2_REGISTER_SIZE, swapoffset = 1, dswapoffset; i < numiters; i++, srcidx *= 2, swapoffset = swapoffset << 1) {
		destidx = offset * srcidx;
		dswapoffset = swapoffset << 1;
		rowaptr = (REGISTER_SIZE*) m_pBits;
		rowbptr = rowaptr + destidx - swapoffset;

		rounds = rows / (srcidx * 2);
		for (std::size_t j = 0; j < rows / (srcidx * 2); j++) {
			std::size_t p;
			for (p = 0, lim = rowbptr + destidx; p < destidx && rowbptr < lim; p++, rowaptr++, rowbptr++) {
				if ((p % dswapoffset >= swapoffset)) {
					temp_row = *rowaptr;
					*rowaptr = *rowbptr;
					*rowbptr = temp_row;
				}
			}
			rowaptr += destidx;
			rowbptr += destidx;
		}
	}

	if (columns > rows) {
		BYTE* tempvec = (BYTE*) malloc((rows * columns) / 8);
		memcpy(tempvec, m_pBits, ((rows / 8) * columns));

		rowaptr = (REGISTER_SIZE*) m_pBits;
		std::size_t rowbytesize = rows / 8;
		std::size_t rowregsize = rows / (sizeof(REGISTER_SIZE) * 8);
		for (std::size_t i = 0; i < columns / rows; i++) {
			rowbptr = (REGISTER_SIZE*) tempvec;
			rowbptr += (i * rowregsize);
			for (std::size_t j = 0; j < rows; j++, rowaptr += rowregsize, rowbptr += offset) {
				memcpy(rowaptr, rowbptr, rowbytesize);
			}
		}
		free(tempvec);
	}

	if (rows > columns) {
		BYTE* tempvec = (BYTE*) malloc((rows * columns) / 8);
		memcpy(tempvec, m_pBits, ((rows / 8) * columns));

		REGISTER_SIZE* rowaptr = (REGISTER_SIZE*) m_pBits;
		std::size_t colbytesize = columns / 8;
		std::size_t colregsize = columns / (sizeof(REGISTER_SIZE) * 8);
		std::size_t offset_cols = (columns * columns) / (sizeof(REGISTER_SIZE) * 8);

		for (std::size_t i = 0; i < columns; i++) {
			rowbptr = (REGISTER_SIZE*) tempvec;
			rowbptr += (i * colregsize);
			for (std::size_t j = 0; j < rows / columns; j++, rowaptr += colregsize, rowbptr += offset_cols) {
				memcpy(rowaptr, rowbptr, colbytesize);
			}
		}
		free(tempvec);
	}
}
