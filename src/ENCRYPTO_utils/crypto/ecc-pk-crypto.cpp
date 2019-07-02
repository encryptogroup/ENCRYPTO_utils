/**
 \file 		ecc-pk-crypto.cpp
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
 */

#include "ecc-pk-crypto.h"


void ecc_field::init(seclvl sp, uint8_t* seed) {
	relic_mutex.lock();
	core_set(NULL);
	
	core_init(); //initialize the relic library
	rand_seed(seed, sp.symbits >> 3); //set the seed of the relic's internal random generator
	eb_param_set_any_plain();

	fe_bytelen = ceil_divide(ECCLVL, 8) + 1;

	context = core_get();
	relic_mutex.unlock();

	generator = (ecc_fe*) get_generator();
	order = get_order();
}

ecc_field::~ecc_field() {
	delete generator;
	delete order;

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);
	core_clean();
}

num* ecc_field::get_num() {
	return new ecc_num(this);
}

num* ecc_field::get_order() {
	relic_mutex.lock();
	core_set(context);

	bn_t bn_order;
	bn_null(bn_order);
	bn_new(bn_order);
	eb_curve_get_ord(bn_order);

	relic_mutex.unlock();
	return new ecc_num(this, bn_order);
}

num* ecc_field::get_rnd_num(uint32_t bitlen) {
	if (bitlen == 0) {
		bitlen = ECCLVL;
	}

	relic_mutex.lock();
	core_set(context);

	bn_t rnd;
	bn_null(rnd);
	bn_new(rnd);

	bn_rand(rnd, RLC_POS, bitlen);

	relic_mutex.unlock();

	num* res = new ecc_num(this, rnd);
	return res;
}
fe* ecc_field::get_fe() {
	return new ecc_fe(this);
}

fe* ecc_field::get_rnd_fe() {
	return sample_random_point();
}

fe* ecc_field::get_generator() {
	relic_mutex.lock();
	core_set(context);

	eb_t eb_generator;
	eb_null(eb_generator);
	eb_new(eb_generator);
	eb_curve_get_gen(eb_generator);

	relic_mutex.unlock();

	return new ecc_fe(this, eb_generator);
}
fe* ecc_field::get_rnd_generator() {
	//TODO not sure how this method is supposed to work
	return sample_random_point();
}
uint32_t ecc_field::num_byte_size() {
		return ceil_divide(ECCLVL, 8);
	}
uint32_t ecc_field::get_field_size() {
	return ECCLVL;
}
brickexp* ecc_field::get_brick(fe* gen) {
	return new ecc_brickexp(gen, this);
}
uint32_t ecc_field::get_size() {
	return ECCLVL;
}

fe* ecc_field::sample_random_point() {
	relic_mutex.lock();
	core_set(context);

	eb_t tmp_eb;
	eb_null(tmp_eb);
	eb_new(tmp_eb);
	eb_rand(tmp_eb);

	relic_mutex.unlock();

	fe* tmp_fe = new ecc_fe(this, tmp_eb);
	return tmp_fe;
}

ctx_t* ecc_field::get_context() {
	return context;
}

ecc_fe::ecc_fe(ecc_field* fld) {
	field = fld;
	init();
}

ecc_fe::ecc_fe(ecc_field* fld, eb_t src) {
	field = fld;
	init();

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_copy(val, src);
}
ecc_fe::~ecc_fe() {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_free(val);
}

void ecc_fe::set(fe* src) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t tmp_val;
	((ecc_fe*) src)->get_val(tmp_val);
	eb_copy(val, tmp_val);
}

void ecc_fe::get_val(eb_t res) {
	shallow_copy(res, val);
}

//Note: if the same value is processed, a has to be this value
void ecc_fe::set_mul(fe* a, fe* b) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t eb_a;
	eb_t eb_b;
	((ecc_fe*) a)->get_val(eb_a);
	((ecc_fe*) b)->get_val(eb_b);

	eb_add(val, eb_a, eb_b);
}

void ecc_fe::set_pow(fe* a, num* e) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t eb_a;
	bn_t bn_e;
	((ecc_fe*) a)->get_val(eb_a);
	((ecc_num*) e)->get_val(bn_e);

	eb_mul(val, eb_a, bn_e);
}

//Note: if the same value is processed, a has to be this value
void ecc_fe::set_div(fe* a, fe* b) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t eb_a;
	eb_t eb_b;
	((ecc_fe*) a)->get_val(eb_a);
	((ecc_fe*) b)->get_val(eb_b);

	eb_sub(val, eb_a, eb_b);
}

//TODO not sure what double here means, please check
void ecc_fe::set_double_pow_mul(fe* b1, num* e1, fe* b2, num* e2) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t eb_b1;
	eb_t eb_b2;
	((ecc_fe*) b1)->get_val(eb_b1);
	((ecc_fe*) b2)->get_val(eb_b2);
	bn_t bn_e1;
	bn_t bn_e2;
	((ecc_num*) e1)->get_val(bn_e1);
	((ecc_num*) e2)->get_val(bn_e2);

	eb_mul_sim(val, eb_b1, bn_e1, eb_b2, bn_e2);
}

void ecc_fe::import_from_bytes(uint8_t* buf) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_read_bin(val, buf, field->fe_byte_size());
}

void ecc_fe::export_to_bytes(uint8_t* buf) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_write_bin(buf, field->fe_byte_size(), val, true);
}

//TODO not sure what the difference to import_from_bytes is yet
void ecc_fe::sample_fe_from_bytes(uint8_t* buf, uint32_t bytelen) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_read_bin(val, buf, bytelen);
	//TODO normalizing or something like that
}
bool ecc_fe::eq(fe* a) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t to_cmp;
	((ecc_fe*) a)->get_val(to_cmp);
	return eb_cmp(val, to_cmp) == RLC_EQ;
}

void ecc_fe::init() {
	context = field->get_context();

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_null(val);
	eb_new(val);
};
void ecc_fe::print() {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_print(val);
};

void ecc_fe::shallow_copy(eb_t to, eb_t from) {
	*to = *from;
}

ecc_num::ecc_num(ecc_field* fld) {
	field = fld;
	context = field->get_context();

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_null(val);
	bn_new(val);
}
ecc_num::ecc_num(ecc_field* fld, bn_t src) {
	field = fld;
	context = field->get_context();

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_null(val);
	bn_new(val);
	bn_copy(val, src);
}

ecc_num::~ecc_num() {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_free(val);
}

void ecc_num::get_val(bn_t res) {
	shallow_copy(res, val);
}

void ecc_num::set(num* src) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_t tmp_val;
	((ecc_num*) src)->get_val(tmp_val);
	bn_copy(val, tmp_val);
}
void ecc_num::set_si(int32_t src) {
	//TODO implement this method
}
void ecc_num::set_add(num* a, num* b) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_t bn_a;
	bn_t bn_b;
	((ecc_num*) a)->get_val(bn_a);
	((ecc_num*) b)->get_val(bn_b);
	bn_add(val, bn_a, bn_b);
}
void ecc_num::set_sub(num* a, num* b) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_t bn_a;
	bn_t bn_b;
	((ecc_num*) a)->get_val(bn_a);
	((ecc_num*) b)->get_val(bn_b);
	bn_sub(val, bn_a, bn_b);
}
void ecc_num::set_mul(num* a, num* b) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_t bn_a;
	bn_t bn_b;
	((ecc_num*) a)->get_val(bn_a);
	((ecc_num*) b)->get_val(bn_b);
	bn_mul(val, bn_a, bn_b);
}

void ecc_num::mod(num* modulus) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_t bn_mod;
	((ecc_num*) modulus)->get_val(bn_mod);
	bn_mod(val, val, bn_mod);
}

void ecc_num::set_mul_mod(num* a, num* b, num* modulus) {
	//TODO is normalizing meant instead?
	set_mul(a, b);
	mod(modulus);
}

void ecc_num::import_from_bytes(uint8_t* buf, uint32_t field_size_bytes) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_read_bin(val, buf, field_size_bytes);
}

//export and pad all leading zeros
void ecc_num::export_to_bytes(uint8_t* buf, uint32_t field_size_bytes) {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_write_bin(buf, field_size_bytes, val);
}

void ecc_num::print() {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	bn_print(val);
};

void ecc_num::shallow_copy(bn_t to, bn_t from) {
	*to = *from;
}

ecc_brickexp::ecc_brickexp(fe* generator, ecc_field* field) {
	this->field = field;
	context = field->get_context();

	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	eb_t tmp_val;
	((ecc_fe*) generator)->get_val(tmp_val);
	eb_table = new eb_t[RLC_EB_TABLE_MAX];
	for(uint32_t i = 0; i < RLC_EB_TABLE_MAX; i++) {
		eb_null(eb_table[i]);
	}
	for(uint32_t i = 0; i < RLC_EB_TABLE; i++) {
		eb_new(eb_table[i]);
	}
	eb_mul_pre(eb_table, tmp_val);
}

ecc_brickexp::~ecc_brickexp() {
	std::lock_guard<std::mutex> lock(relic_mutex);
	core_set(context);

	for(uint32_t i = 0; i < RLC_EB_TABLE; ++i) {
		eb_free(eb_table[i]);
	}
	delete eb_table;
}


void ecc_brickexp::pow(fe* result, num* e) {
	relic_mutex.lock();
	core_set(context);

	eb_t eb_res;
	eb_null(eb_res);
	eb_new(eb_res);
	bn_t bn_e;
	((ecc_num*) e)->get_val(bn_e);
	eb_mul_fix(eb_res, eb_table, bn_e);

	relic_mutex.unlock();

	ecc_fe tmp_fe(field, eb_res);
	result->set(&tmp_fe);

}
