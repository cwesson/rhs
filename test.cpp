/**
 * @file rhs/test.cpp
 * @author Conlan Wesson
 * @copyright (c) 2021, Conlan Wesson All rights reserved.
 * RHS test program.
 */

#include "edacmemory.h"
#include "rhsbool.h"
#include <iostream>

class test {
	public:
		test(int a, int b) :
			_a(a),
			_b(b)
		{}
		
		int sum() {
			return _a + _b;
		}
	
		int _a;
		int _b;
};

int main(){
	rhs::ecc_obj<test> a = rhs::make_ecc<test>(12, 30);
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	
	a->_a = 13;
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	
	a->_a = 13;
	a.update();
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	
	rhs_bool_t b = rhs_true;
	if(rhs_is_true(b)){
		std::cout << "true" << std::endl;
	}else if(rhs_is_false(b)){
		std::cout << "false" << std::endl;
	}else{
		std::cout << "undef" << std::endl;
	}
	
	rhs::tmr_obj<int> c(3);
	
	c += 2;
	
	std::cout << "3+2=" << c << std::endl;
	
	c[1] = 7;
	
	c *= 4;
	std::cout << "5*4=" << c << std::endl;
	
	c /= 5;
	std::cout << "20/5=" << c << std::endl;
	
	return 0;
}
