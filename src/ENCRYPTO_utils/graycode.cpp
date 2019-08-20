/**
 \file 		graycode.cpp
 \author 	Martin Kromm<martin.kromm@stud.tu-darmstadt.de>
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
 \brief		Gray-Code implementation
 */

#include "./graycode.h"
#include <stdlib.h>

uint32_t* BuildGrayCode(uint32_t length) {
	uint32_t* gray_code = (uint32_t*) malloc(sizeof(uint32_t) * length);
	for(uint32_t i = 0; i < length; ++i) {
		gray_code[i] = i ^ (i >> 1);
	}
	return gray_code;
}

uint32_t* BuildGrayCodeIncrement(uint32_t length) {
	uint32_t* gray_code_increment = (uint32_t*) malloc(sizeof(uint32_t) * length);
	for(uint32_t i = 0; i < length; ++i) {
		gray_code_increment[i] = 0;
	}
	uint32_t length_inc = 2;
	while(length_inc < length) {
		uint32_t length_count = length_inc - 1;
		while(length_count <= length) {
			(gray_code_increment[length_count])++;
			length_count += length_inc;
		}
		length_inc = length_inc << 1;
	}
	return gray_code_increment;
}
