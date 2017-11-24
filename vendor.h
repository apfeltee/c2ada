#ifndef _H_VENDOR_
#define _H_VENDOR_

typedef enum {
	unspecified_vendor,
	Rational,
	VADS,
	ICC,					/* Irvine Compiler */
	GNAT
} vendor_t;

extern vendor_t ada_compiler;

#endif /* _H_VENDOR_ */
