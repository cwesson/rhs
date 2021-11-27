/**
 * @file rhs/rhsbool.h
 * @author Conlan Wesson
 * @copyright (c) 2021, Conlan Wesson All rights reserved.
 * RHS Booleans.
 */

#include "rhsbool.h"

static unsigned int cnt(rhs_bool_t b){
	unsigned int sum = 0;
	for(unsigned int i = b; i > 0; i >>= 1){
		sum += i & 1;
	}
	return sum;
}

bool rhs_is_true(rhs_bool_t b) {
	return (cnt(b) > ((sizeof(rhs_bool_t)*8)/2));
}

bool rhs_is_false(rhs_bool_t b) {
	return (cnt(b) < ((sizeof(rhs_bool_t)*8)/2));
}
