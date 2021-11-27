/**
 * @file rhs/edacmemory.h
 * @author Conlan Wesson
 * @copyright (c) 2021, Conlan Wesson All rights reserved.
 */

#ifndef _RHS_ECCMEMORY_H_
#define _RHS_ECCMEMORY_H_

#include "error.h"
#include <cstdint>
#include <iostream>

namespace rhs {

/**
 * Simple zero-sum checksum
 * @tparam T Type to checksum over.
 */
template<typename T>
class zerosum {
	public:
		/**
		 * Calculate and store checksum.
		 * @param data Object to checksum.
		 */
		void calculate(const T& data) {
			diff = 0 - _calculate(data);
		}
		
		/**
		 * Verify stored checksum.
		 * @param data Object to checksum.
		 * @return RHS_EOK if checksum verifies, RHS_ENOTVERIFIED otherwise.
		 */
		rhs_error_t verify(const T& data) {
			uint8_t sum = _calculate(data) + diff;
			return (sum == 0) ? RHS_EOK : RHS_ENOTVERIFIED;
		}
		
		/**
		 * Correct errors.
		 * @param data Objec to correct.
		 * @return RHS_EOK if successfully corrected, RHS_ENOTCORRECTED otherwise.
		 */
		rhs_error_t correct(const T& data) {
			(void)data;
			return RHS_ENOTSUP;
		}
	
	private:
		uint8_t _calculate(const T& data) {
			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&data);
			uint8_t sum = 0;
			for(std::size_t i = 0; i < sizeof(T); ++i){
				sum += ptr[i];
			}
			return sum;
		}
		
		uint8_t diff;
};

/**
 * ECC object wrapper.
 * @tparam T Type of wrapped object.
 * @tparam ECC ECC to use.
 */
template<typename T, typename ECC=zerosum<T>>
class ecc_obj {
	public:
		/**
		 * Constructor.
		 */
		ecc_obj() :
			_obj(),
			_ecc()
		{
			update();
		}
		
		/**
		 * Move constructor.
		 * @param p Object to move.
		 */
		ecc_obj(T&& p) :
			_obj(p)
		{
			update();
		}
		
		/**
		 * Destructor.
		 */
		virtual ~ecc_obj() = default;
		
		/**
		 * Dereference operator.
		 * @return Reference to wrapped object.
		 */
		const T& operator*() const {
			verify();
			return _obj;
		}
		
		/**
		 * Dereference operator.
		 * @return Reference to wrapped object.
		 */
		T& operator*() {
			verifyAndCorrect();
			return _obj;
		}
		
		/**
		 * Arrow operator.
		 * @return Pointer to wrapped object.
		 */
		const T* operator->() const {
			verify();
			return &_obj;
		}
		
		/**
		 * Arrow operator.
		 * @return Pointer to wrapped object.
		 */
		T* operator->() {
			verifyAndCorrect();
			return &_obj;
		}
		
		/**
		 * Update the ECC for the wrapped object.
		 * @note This must be called after the object is intentionally modified.
		 */
		void update() {
			_ecc.calculate(_obj);
		}
		
		/**
		 * Verify the integrity of the wrapped object.
		 * @return RHS_EOK if object is verified.
		 */
		rhs_error_t verify() {
			rhs_error_t ret = _ecc.verify(_obj);
			if(ret == RHS_ENOTVERIFIED){
				std::cout << "Verification failed" << std::endl;
			}
			return ret;
		}
		
		/**
		 * Correct errors in the wrapped object.
		 * @return RHS_EOK if object is corrected.
		 */
		rhs_error_t correct() {
			rhs_error_t ret = _ecc.correct(_obj);
			if(ret == RHS_ENOTCORRECTED){
				std::cout << "Correction failed" << std::endl;
			}
			return ret;
		}
		
		/**
		 * Verify the integrity of the wrapped object and correct errors.
		 * @return RHS_EOK if object is verified or corrected.
		 */
		rhs_error_t verifyAndCorrect() {
			rhs_error_t ret = verify();
			if(ret == RHS_ENOTVERIFIED){
				rhs_error_t corr = correct();
				if(corr != RHS_ENOTSUP){
					ret = corr;
				}
			}
			return ret;
		}
	
	private:
		T _obj;
		ECC _ecc;
};

/**
 * Make an ecc_obj.
 * @tparam T Type of wrapped object.
 * @tparam Args Types of arguments for constructor.
 * @param args Arguments for constructor
 * @return New ecc_obj.
 */
template<typename T, typename... Args>
ecc_obj<T> make_ecc(Args&&... args) {
	return ecc_obj<T>(T(args...));
}

/**
 * Redundant object wrapper.
 * @tparam T Type of wrapped object.
 * @tparam N Number of redundant copies.
 */
template<typename T, unsigned int N=3>
class tmr_obj {
	public:
		/**
		 * Constructor.
		 */
		tmr_obj() {}
		
		/**
		 * Constructor.
		 * @param p Original object
		 */
		tmr_obj(T& p) {
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] = p;
			}
		}
		
		/**
		 * Constructor.
		 * @param p Original object
		 */
		tmr_obj(T p) {
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] = p;
			}
		}
		
		/**
		 * Copy constructor.
		 * @param p Original object
		 */
		tmr_obj(const tmr_obj<T>& p) {
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] = p;
			}
		}
		
		/**
		 * Destructor.
		 */
		virtual ~tmr_obj() = default;
		
		/**
		 * Index operator.
		 * @param i Index.
		 * @return Reference to redundant copy i.
		 */
		const T& operator[](unsigned int i) const {
			return _obj[i];
		}
		
		/**
		 * Index operator.
		 * @param i Index.
		 * @return Reference to redundant copy i.
		 * @note For testing only.
		 */
		T& operator[](unsigned int i) {
			return _obj[i];
		}
		
		operator T() {
			verifyAndCorrect();
			return _obj[0];
		}
		
		tmr_obj<T, N>& operator+=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] += b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator-=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] -= b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator*=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] *= b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator/=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				_obj[i] /= b;
			}
			return *this;
		}
		
		tmr_obj<T, N> operator+(tmr_obj<T> b){
			tmr_obj<T, N> ret(*this);
			return ret += b;
		}
		
		tmr_obj<T, N> operator-(tmr_obj<T> b){
			tmr_obj<T, N> ret(*this);
			return ret -= b;
		}
		
		tmr_obj<T, N> operator*(tmr_obj<T> b){
			tmr_obj<T, N> ret(*this);
			return ret *= b;
		}
		
		tmr_obj<T, N> operator/(tmr_obj<T> b){
			tmr_obj<T, N> ret(*this);
			return ret /= b;
		}
		
		/**
		 * Verify the integrity of the wrapped object.
		 * @return RHS_EOK if object is verified.
		 */
		rhs_error_t verify() {
			bool ret = (_obj[0] == _obj[1]);
			for(unsigned int i = 1; i < N; ++i){
				ret = (_obj[i-1] == _obj[i]) && ret;
			}
			if(!ret){
				std::cout << "Verification failed" << std::endl;
			}
			return ret ? RHS_EOK : RHS_ENOTVERIFIED;
		}
		
		/**
		 * Correct errors in the wrapped object.
		 * @return RHS_EOK if object is corrected.
		 */
		rhs_error_t correct() {
			unsigned int corrected = 0;
			unsigned int most = 0;
			
			for(unsigned int c = 0; c < N; ++c){
				unsigned int counter = 0;
				for(unsigned int i = c+1; i < N; ++i){
					if(_obj[c] == _obj[i]){
						++counter;
					}
				}
				if(counter > most){
					most = counter;
					corrected = c;
				}
			}
			
			if(most > 0){
				for(unsigned int i = 0; i < N; ++i){
					_obj[i] = _obj[corrected];
				}
				
				return RHS_EOK;
			}else{
				std::cout << "Correction failed" << std::endl;
				return RHS_ENOTCORRECTED;
			}
		}
		
		/**
		 * Verify the integrity of the wrapped object and correct errors.
		 * @return RHS_EOK if object is verified or corrected.
		 */
		rhs_error_t verifyAndCorrect() {
			rhs_error_t ret = verify();
			if(ret == RHS_ENOTVERIFIED){
				rhs_error_t corr = correct();
				if(corr != RHS_ENOTSUP){
					ret = corr;
				}
			}
			return ret;
		}
	
	private:
		T _obj[N];
};

} // namespace rhs

#endif // _RHS_ECCMEMORY_H_
