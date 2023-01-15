/**
 * @file rhs/test.cpp
 * @author Conlan Wesson
 * @copyright (c) 2021, Conlan Wesson All rights reserved.
 * RHS test program.
 */

#include "rhs/edacmemory.h"
#include "rhs/rhsbool.h"
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
		
		int inc() {
			std::cout << "inc" << std::endl;
			return _a += 1;
		}
	
		int _a;
		int _b;
};

#define TEST(_x) (std::cout << ((_x) ? "PASS" : "FAIL") << " " << #_x << std::endl)

int main(){
	rhs::ecc_obj<test> a = rhs::make_ecc<test>(12, 30);
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	TEST((*a)._a == 12);
	TEST((*a)._b == 30);
	TEST(a->sum() == 42);
	
	a->_a = 13; // inject bit error
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	TEST((*a)._a == 12);
	TEST((*a)._b == 30);
	TEST(a->sum() == 42);
	
	a->_a = 13;
	a.update();
	TEST((*a)._a == 13);
	
	std::cout << (*a)._a << "+" << (*a)._b << "=" << a->sum() << std::endl;
	TEST((*a)._a == 13);
	TEST((*a)._b == 30);
	TEST(a->sum() == 43);
	
	rhs::boolean b(rhs_true);
	if(b == rhs_true){
		std::cout << "true" << std::endl;
	}else if(b == rhs_false){
		std::cout << "false" << std::endl;
	}else{
		std::cout << "undef" << std::endl;
	}
	
	rhs::tmr_obj<int> c(3);
	TEST(c == 3);
	
	c += 2;
	
	std::cout << "3+2=" << c << std::endl;
	TEST(c == 5);
	
	c[1] = 7; // inject bit error
	
	c *= 4;
	std::cout << "5*4=" << c << std::endl;
	TEST(c == 20);
	
	c /= 5;
	std::cout << "20/5=" << c << std::endl;
	TEST(c == 4);
	
	return 0;
}
