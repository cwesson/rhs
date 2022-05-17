/**
 * @file rhs/rhsbool.h
 * @author Conlan Wesson
 * @copyright (c) 2022, Conlan Wesson All rights reserved.
 * RHS Booleans.
 */

#ifndef _RHS_RHSBOOL_H_
#define _RHS_RHSBOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RHS Boolean type.
 */
typedef enum rhs_bool {
	rhs_true  = -1, ///< RHS true
	rhs_false =  0, ///< RHS false
} rhs_bool_t;

/**
 * Check if RHS Boolean is true.
 * @param _b RHS Boolean.
 * @return true if RHS Boolean is true, false otherwise.
 */
#define rhs_is_true(_b) (__builtin_popcount(_b) > ((sizeof(rhs_bool_t)*8)/2))

/**
 * Check if RHS Boolean is false.
 * @param _b RHS Boolean.
 * @return true if RHS Boolean is false, false otherwise.
 */
#define rhs_is_false(_b) (__builtin_popcount(_b) < ((sizeof(rhs_bool_t)*8)/2))

#ifdef __cplusplus
}; // extern "C"

namespace rhs {

/**
 * RHS Boolean
 */
class boolean {
	public:
		/**
		 * Constructor.
		 * @tparam T Type to construct from.
		 * @param v Convert anything to a RHS Boolean.
		 */
		template<typename T>
		boolean(const T& v) {
			*this = v;
		}
		
		/**
		 * Assignment operator.
		 * @tparam T Type to construct from.
		 * @param v Convert anything to a RHS Boolean.
		 * @return Reference to this.
		 */
		template<typename T>
		boolean& operator=(const T& v) {
			if(v){
				b = rhs_true;
			}else{
				b = rhs_false;
			}
			
			return *this;
		}
		
		/**
		 * Cast to bool.
		 * @return true if rhs_is_true, false otherwise.
		 * @deprecated Explicit comparisons are preferred.
		 */
		[[deprecated("Use operator== instead.")]]
		operator bool() const {
			return rhs_is_true(b);
		}
		
		/**
		 * Logical NOT operator.
		 * @return NOT RHS Boolean.
		 */
		boolean operator!() const {
			return boolean(~b);
		}
		
		/**
		 * Equality operator.
		 * @param other Other RHS Boolean to compare to
		 * @return true if this and other are the same RHS Boolean, false otherwise.
		 */
		bool operator==(rhs_bool_t other) const {
			return (rhs_is_true(b) && rhs_is_true(other)) || (rhs_is_false(b) && rhs_is_false(other));
		}
		
		/**
		 * Equality operator.
		 * @param other Other RHS Boolean to compare to
		 * @return true if this and other are not the same RHS Boolean, false otherwise.
		 */
		bool operator!=(rhs_bool_t other) const {
			return !(*this == other);
		}
	
	private:
		rhs_bool_t b; ///< Underlying rhs_bool_t.
};

/**
 * Assignment operator.
 * Template specialization for converting from rhs_bool_t.
 * @param v Convert anything to a RHS Boolean.
 * @return Reference to this.
 */
template<>
boolean& boolean::operator=(const rhs_bool_t& v) {
	if(rhs_is_true(v)){
		b = rhs_true;
	}else{
		b = rhs_false;
	}
	
	return *this;
}

} // namespace rhs
#endif // __cplusplus

#endif // _RHS_RHSBOOL_H_
