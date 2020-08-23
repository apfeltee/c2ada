/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -N cpp_keyword -t -p cpp.prf  */
/* Computed positions: -k'2,4' */

#if !(                                                                          \
(' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37)         \
&& ('&' == 38) && ('\'' == 39) && ('(' == 40) && (')' == 41) && ('*' == 42)     \
&& ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) && ('/' == 47)      \
&& ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52)      \
&& ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && ('9' == 57)      \
&& (':' == 58) && (';' == 59) && ('<' == 60) && ('=' == 61) && ('>' == 62)      \
&& ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && ('D' == 68)      \
&& ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73)      \
&& ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && ('N' == 78)      \
&& ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) && ('S' == 83)      \
&& ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && ('X' == 88)      \
&& ('Y' == 89) && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93)     \
&& ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) && ('c' == 99)      \
&& ('d' == 100) && ('e' == 101) && ('f' == 102) && ('g' == 103)                 \
&& ('h' == 104) && ('i' == 105) && ('j' == 106) && ('k' == 107)                 \
&& ('l' == 108) && ('m' == 109) && ('n' == 110) && ('o' == 111) && ('p' == 112) \
&& ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) && ('u' == 117) \
&& ('v' == 118) && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
&& ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
    /* The character set is not based on ISO-646.  */
    #error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "cpp.prf"

/* $Source: /home/CVSROOT/c2ada/cpp.prf,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $  */

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "cpp_hide.h"
#line 17 "cpp.prf"
struct resword
{
    char* name;
    int token;
};

#define TOTAL_KEYWORDS 13
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 7
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 24
/* maximum key range = 20, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
    #ifdef __cplusplus
inline
    #endif
#endif
static unsigned int
hash(register const char* str, register size_t len)
{
    static unsigned char asso_values[]
    = { 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 13, 0,  5,  0,  25, 15, 25, 25,
        5,  25, 0,  5,  25, 25, 5,  25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
        25, 25, 25, 25 };
    register unsigned int hval = len;

    switch(hval)
    {
        default:
            hval += asso_values[(unsigned char)str[3]];
        /*FALLTHROUGH*/
        case 3:
        case 2:
            hval += asso_values[(unsigned char)str[1]];
            break;
    }
    return hval;
}

struct resword* cpp_keyword(register const char* str, register size_t len)
{
    static struct resword wordlist[] = { { "" },
                                         { "" },
                                         { "" },
                                         { "" },
                                         { "" },
#line 31 "cpp.prf"
                                         { "undef", Undef },
                                         { "" },
#line 25 "cpp.prf"
                                         { "if", If },
                                         { "" },
#line 21 "cpp.prf"
                                         { "else", Else },
#line 26 "cpp.prf"
                                         { "ifdef", Ifdef },
#line 30 "cpp.prf"
                                         { "pragma", Pragma },
#line 28 "cpp.prf"
                                         { "include", Include },
                                         { "" },
#line 20 "cpp.prf"
                                         { "elif", Elif },
#line 23 "cpp.prf"
                                         { "error", Error },
                                         { "" },
                                         { "" },
#line 24 "cpp.prf"
                                         { "ident", Ident },
#line 29 "cpp.prf"
                                         { "line", Line },
#line 22 "cpp.prf"
                                         { "endif", Endif },
#line 19 "cpp.prf"
                                         { "define", Define },
                                         { "" },
                                         { "" },
#line 27 "cpp.prf"
                                         { "ifndef", Ifndef } };

    if(len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
        register unsigned int key = hash(str, len);

        if(key <= MAX_HASH_VALUE)
        {
            register const char* s = wordlist[key].name;

            if(*str == *s && !strcmp(str + 1, s + 1))
                return &wordlist[key];
        }
    }
    return 0;
}
#line 32 "cpp.prf"
