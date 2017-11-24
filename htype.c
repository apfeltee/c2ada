#include <stdio.h>
#include <sys/types.h>

#include <sys/param.h>  /* for NBBY */

#define CHAR_SIZE				sizeof(char)
#define SHORT_SIZE				sizeof(short int)
#define INT_SIZE				sizeof(int)
#define LONG_SIZE				sizeof(long int)
#define ADDRESS_SIZE		   	        sizeof(char*)
#define FLOAT_SIZE				sizeof(float)
#define DOUBLE_SIZE				sizeof(double)
#define LONG_DOUBLE_SIZE		        sizeof(long double)

/*
 * Do not make static
 */
short host_byte_order = 0x0102;

static int
alignof_address()
{
	struct s {
		char x;
		char *y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + ADDRESS_SIZE));
}

static int
alignof_short()
{
	struct s {
		char x;
		short int y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + SHORT_SIZE));
}

static int
alignof_int()
{
	struct s {
		char x;
		int y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + INT_SIZE));
}

static int
alignof_long()
{
	struct s {
		char x;
		long int y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + LONG_SIZE));
}

static int
alignof_float()
{
	struct s {
		char x;
		float y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + FLOAT_SIZE));
}

static int
alignof_double()
{
	struct s {
		char x;
		double y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + DOUBLE_SIZE));
}

static int
alignof_long_double()
{
	struct s {
		char x;
		long double y;
	};

	return CHAR_SIZE + (sizeof(struct s) - (CHAR_SIZE + LONG_DOUBLE_SIZE));
}


int
main()
{

  /* format strings for printing *SIZE values */
#ifdef __APPLE__
#define SIZE_FLAG "%lu"
#elif defined linux
#define SIZE_FLAG "%u"
#elif defined sun
#define SIZE_FLAG "%u"
#else
#define SIZE_FLAG "%u"
#endif

	printf("#define BITS_PER_BYTE\t\t\t%d\n\n", NBBY);
	printf("#define SIZEOF_CHAR\t\t\t" SIZE_FLAG "\n", CHAR_SIZE);
	printf("#define SIZEOF_SHORT\t\t\t" SIZE_FLAG "\n", SHORT_SIZE);
	printf("#define SIZEOF_INT\t\t\t" SIZE_FLAG "\n", INT_SIZE);
	printf("#define SIZEOF_LONG\t\t\t" SIZE_FLAG "\n", LONG_SIZE);
	printf("#define SIZEOF_FLOAT\t\t\t" SIZE_FLAG "\n", FLOAT_SIZE);
	printf("#define SIZEOF_DOUBLE\t\t\t" SIZE_FLAG "\n", DOUBLE_SIZE);
	printf("#define SIZEOF_LONG_DOUBLE\t\t" SIZE_FLAG "\n", LONG_DOUBLE_SIZE);
	printf("#define SIZEOF_ADDRESS\t\t\t" SIZE_FLAG "\n", ADDRESS_SIZE);

	printf("\n#define ALIGNOF_CHAR\t\t\t" SIZE_FLAG "\n", CHAR_SIZE);
	printf("#define ALIGNOF_SHORT\t\t\t%d\n", alignof_short());
	printf("#define ALIGNOF_INT\t\t\t%d\n", alignof_int());
	printf("#define ALIGNOF_LONG\t\t\t%d\n", alignof_long());
	printf("#define ALIGNOF_FLOAT\t\t\t%d\n", alignof_float());
	printf("#define ALIGNOF_DOUBLE\t\t\t%d\n", alignof_double());
	printf("#define ALIGNOF_LONG_DOUBLE\t\t%d\n", alignof_long_double());
	printf("#define ALIGNOF_ADDRESS\t\t\t%d\n", alignof_address());

	printf("\n#define CHARS_ARE_%sSIGNED\n", (((char)-1) < 0) ? "" : "UN");

#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
	if (*p == 1) {
                puts("#if !defined(BIG_ENDIAN)");
		puts("#define BIG_ENDIAN");
                puts("#endif");
	}
	else {
                puts("#if !defined(LITTLE_ENDIAN)");
		puts("#define LITTLE_ENDIAN");
                puts("#endif");
	}
#endif

	return 0;
}
