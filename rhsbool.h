/**
 * @file rhs/rhsbool.h
 * @author Conlan Wesson
 * @copyright (c) 2021, Conlan Wesson All rights reserved.
 * RHS Booleans.
 */

#ifndef _RHS_RHSBOOL_H_
#define _RHS_RHSBOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * RHS Boolean type.
 */
typedef enum rhs_bool {
	rhs_true  = -1, ///< RHS true
	rhs_false = 0,  ///< RHS false
} rhs_bool_t;

/**
 * Check if RHS Boolean is true.
 * @param b RHS Boolean.
 * @return true if RHS Boolean is true, false otherwise.
 */
bool rhs_is_true(rhs_bool_t b);

/**
 * Check if RHS Boolean is false.
 * @param b RHS Boolean.
 * @return true if RHS Boolean is false, false otherwise.
 */
bool rhs_is_false(rhs_bool_t b);

#ifdef __cplusplus
};
#endif

#endif // _RHS_RHSBOOL_H_
