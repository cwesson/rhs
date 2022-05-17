/**
 * @file rhs/error.h
 * @author Conlan Wesson
 * @copyright (c) 2022, Conlan Wesson All rights reserved.
 * RHS error codes.
 */

#ifndef _RHS_ERROR_H_
#define _RHS_ERROR_H_

/**
 * RHS error codes.
 */
typedef enum {
	RHS_EOK = 0,        ///< No error
	RHS_ENOTSUP,        ///< Not supported
	RHS_ENOTVERIFIED,   ///< Verification failed
	RHS_ENOTCORRECTED,  ///< Correction failed
} rhs_error_t;

#endif
