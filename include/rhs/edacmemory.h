/**
 * @file rhs/edacmemory.h
 * @author Conlan Wesson
 * @copyright (c) 2022, Conlan Wesson All rights reserved.
 */

#ifndef _RHS_EDACMEMORY_H_
#define _RHS_EDACMEMORY_H_

#include "error.h"
extern "C" {
#include "fec.h"
}
#include <cstdint>
#include <iostream>
#include <functional>
#include <cstring>

namespace rhs {

/**
 * Reed-Solomon
 * @tparam T Type to correct over.
 */
template<typename T, typename B>
class reedsolomon {
	public:
		enum {
			BLOCK_SIZE = 255, ///< Encoded block length in bytes.
			DATA_SIZE = 223,  ///< Message data length in bytes.
			_remainder = sizeof(T) % DATA_SIZE,
			PAD_SIZE = (_remainder == 0) ? 0 : (DATA_SIZE - _remainder),        ///< Padding needed to round to BLOCK_SIZE.
			PADDED_SIZE = sizeof(T) + PAD_SIZE,                                 ///< Total object size with padding in bytes.
			PARITY_SIZE = (PADDED_SIZE / DATA_SIZE) * (BLOCK_SIZE - DATA_SIZE), ///< Size of additional parity data in bytes.
		};

		explicit reedsolomon(const B& data) {
			calculate(data);
		}

		/**
		 * Calculate and store checksum.
		 * @param data Object to checksum.
		 */
		void calculate(const B& data) {
			const uint8_t* dptr = reinterpret_cast<const uint8_t*>(&data);
			uint8_t* pptr = parity;
			for(size_t rem = PADDED_SIZE; rem > 0; rem -= DATA_SIZE){
				encode_rs_ccsds(const_cast<uint8_t*>(dptr), pptr, 0);
				dptr += DATA_SIZE;
				pptr += (BLOCK_SIZE-DATA_SIZE);
			}
		}
		
		/**
		 * Verify stored checksum.
		 * @param data Object to checksum.
		 * @return Error code.
		 * @retval RHS_EOK if checksum verifies.
		 * @retval RHS_ENOTVERIFIED if checksum does not verify.
		 */
		rhs_error_t verify(const B& data) {
			rhs_error_t ret = RHS_EOK;
			const uint8_t* dptr = reinterpret_cast<const uint8_t*>(&data);
			uint8_t* pptr = parity;
			for(size_t rem = PADDED_SIZE; rem > 0; rem -= DATA_SIZE){
				uint8_t block[BLOCK_SIZE];
				memcpy(block, dptr, DATA_SIZE);
				memcpy(&block[DATA_SIZE], pptr, PARITY_SIZE);
				int r = decode_rs_ccsds(block, NULL, 0, 0);
				if(r != 0){
					// An error was found
					ret = RHS_ENOTVERIFIED;
				}
				dptr += DATA_SIZE;
				pptr += (BLOCK_SIZE-DATA_SIZE);
			}
			return ret;
		}
		
		/**
		 * Correct errors.
		 * @param data Object to correct.
		 * @return Error code.
		 * @retval RHS_EOK if checksum verifies.
		 * @retval RHS_ENOTVERIFIED if checksum does not verify.
		 * @retval RHS_ENOTCORRECTED if correction fails.
		 */
		rhs_error_t correct(B& data) {
			rhs_error_t ret = RHS_EOK;
			uint8_t* dptr = reinterpret_cast<uint8_t*>(&data);
			uint8_t* pptr = parity;
			for(size_t rem = PADDED_SIZE; rem > 0; rem -= DATA_SIZE){
				uint8_t block[BLOCK_SIZE];
				memcpy(block, dptr, DATA_SIZE);
				memcpy(&block[DATA_SIZE], pptr, PARITY_SIZE);
				int r = decode_rs_ccsds(block, NULL, 0, 0);
				if(r != 0){
					// An error was found
					memcpy(dptr, &block, DATA_SIZE);
					if(ret != RHS_ENOTCORRECTED){
						ret = RHS_ENOTVERIFIED;
					}
				}
				if(r < 0){
					// An uncorrectable error was found
					ret = RHS_ENOTCORRECTED;
				}
				dptr += DATA_SIZE;
				pptr += (BLOCK_SIZE-DATA_SIZE);
			}
			return ret;
		}
	
	private:
		uint8_t parity[PARITY_SIZE];
};

/**
 * ECC object wrapper.
 * @tparam T Type of wrapped object.
 */
template<typename T>
class ecc_obj {
	private:
		struct data_t; // forward declaration
		typedef reedsolomon<T, data_t> ECC; ///< ECC type
		
#pragma pack(push,1)
		/**
		 * Struct to keep object and padding together.
		 */
		struct data_t {
			T obj;                          ///< Object being protected.
			uint8_t padding[ECC::PAD_SIZE]; ///< Padding needed for message data.
		} data;
#pragma pack(pop)
		static_assert(sizeof(data_t) % ECC::DATA_SIZE == 0, "Object is not padded correctly");
		
		ECC ecc; ///< ECC state.
		static_assert((sizeof(data_t) + sizeof(ECC)) % ECC::BLOCK_SIZE == 0, "Encoded size is not correct");
	
	public:
		/**
		 * Constructor.
		 */
		ecc_obj() :
			data{{}, {0}},
			ecc(data)
		{}
		
		/**
		 * Move constructor.
		 * @param p Object to move.
		 */
		ecc_obj(const T&& p) : // cppcheck-suppress noExplicitConstructor
			data{p, {0}},
			ecc(data)
		{}
		
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
			return data.obj;
		}
		
		/**
		 * Dereference operator.
		 * @return Reference to wrapped object.
		 */
		T& operator*() {
			verifyAndCorrect();
			return data.obj;
		}
		
		/**
		 * Arrow operator.
		 * @return Pointer to wrapped object.
		 */
		const T* operator->() const {
			verify();
			return &data.obj;
		}
		
		/**
		 * Arrow operator.
		 * @return Pointer to wrapped object.
		 */
		T* operator->() {
			verifyAndCorrect();
			return &data.obj;
		}
		
		/**
		 * Update the ECC for the wrapped object.
		 * @note This must be called after the object is intentionally modified.
		 */
		void update() {
			std::memset(data.padding, 0, ECC::PAD_SIZE);
			ecc.calculate(data);
		}
		
		/**
		 * Verify the integrity of the wrapped object.
		 * @return Error code.
		 * @retval RHS_EOK if checksum verifies.
		 * @retval RHS_ENOTVERIFIED if checksum does not verify.
		 */
		rhs_error_t verify() {
			rhs_error_t ret = ecc.verify(data);
			if(ret == RHS_ENOTVERIFIED){
				std::cout << "Verification failed" << std::endl;
			}
			return ret;
		}
		
		/**
		 * Correct errors in the wrapped object.
		 * @return Error code.
		 * @retval RHS_EOK if checksum verifies.
		 * @retval RHS_ENOTVERIFIED if checksum does not verify.
		 * @retval RHS_ENOTCORRECTED if correction fails.
		 */
		rhs_error_t correct() {
			rhs_error_t ret = ecc.correct(data);
			if(ret == RHS_ENOTCORRECTED){
				std::cout << "Correction failed" << std::endl;
			}
			return ret;
		}
		
		/**
		 * Verify the integrity of the wrapped object and correct errors.
		 * @return Error code.
		 * @retval RHS_EOK if checksum verifies.
		 * @retval RHS_ENOTVERIFIED if checksum does not verify.
		 * @retval RHS_ENOTCORRECTED if correction fails.
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
		tmr_obj(const T& p) {  // cppcheck-suppress noExplicitConstructor
			for(unsigned int i = 0; i < N; ++i){
				obj[i] = p;
			}
		}
		
		/**
		 * Copy constructor.
		 * @param p Original object
		 */
		tmr_obj(const tmr_obj<T>& p) {
			for(unsigned int i = 0; i < N; ++i){
				obj[i] = p;
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
			return obj[i];
		}
		
		/**
		 * Index operator.
		 * @param i Index.
		 * @return Reference to redundant copy i.
		 * @note For testing only.
		 */
		T& operator[](unsigned int i) {
			return obj[i];
		}
		
		operator T() {
			verifyAndCorrect();
			return obj[0];
		}
		
		tmr_obj<T, N>& operator+=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				obj[i] += b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator-=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				obj[i] -= b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator*=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				obj[i] *= b;
			}
			return *this;
		}
		
		tmr_obj<T, N>& operator/=(tmr_obj<T> b){
			verifyAndCorrect();
			for(unsigned int i = 0; i < N; ++i){
				obj[i] /= b;
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
			bool ret = (obj[0] == obj[1]);
			for(unsigned int i = 1; i < N; ++i){
				ret = (obj[i-1] == obj[i]) && ret;
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
					if(obj[c] == obj[i]){
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
					obj[i] = obj[corrected];
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
		T obj[N]; ///< N redundant copies.
};

} // namespace rhs

#endif // _RHS_EDACMEMORY_H_
