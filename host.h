#ifndef _H_HOST_
#define _H_HOST_

/*
 * Internal types for all integer and floating point constants.
 * If you decide to change these types you better check the
 * print_value() and print_fp_value() routines in gen.c.  They
 * will most likely have to be updated.
 */

typedef long int host_int_t;
typedef double host_float_t;

/*
 * External var symbol name prefix by C compiler for this host OS.
 */

#ifdef aix
#define C_VAR_PREFIX	"."
#else
#define C_VAR_PREFIX	"_"
#endif

/*
 * External function symbol name prefix by C compiler for this host OS.
 */

#ifdef aix
#define C_SUBP_PREFIX	"."
#else
#define C_SUBP_PREFIX	"_"
#endif

/*
 * Assume enum == int.  If this is not true you should
 * ifdef for your system and define it as in hostinfo.h.
 */
#define SIZEOF_ENUM		SIZEOF_INT
#define ALIGNOF_ENUM	ALIGNOF_INT

#endif /* _H_HOST_ */
