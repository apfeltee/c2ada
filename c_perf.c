/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: gperf -t -p c.prf  */
/* Computed positions: -k'1,3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "c.prf"

/* $Source: /home/CVSROOT/c2ada/c.prf,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $  */
#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "host.h"
#include "files.h"
#include "hash.h"
#include "il.h"
#include "stmt.h"
#include "y.tab.h"
#line 17 "c.prf"
struct resword {char *name; short token;};

#define TOTAL_KEYWORDS 33
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 8
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 51
/* maximum key range = 49, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 30, 10, 10,
      25, 20,  0, 10, 52,  5, 52, 52, 40, 52,
       5, 20,  5, 52,  0,  5,  0,  0,  0,  5,
      52, 52, 25, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
      52, 52, 52, 52, 52, 52
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct resword *
in_word_set (register const char *str, register size_t len)
{
  static struct resword wordlist[] =
    {
      {""}, {""}, {""},
#line 32 "c.prf"
      {"for",            FOR},
      {""}, {""},
#line 39 "c.prf"
      {"return",         RETURN},
#line 34 "c.prf"
      {"if",             IF},
#line 36 "c.prf"
      {"int",		INT},
#line 49 "c.prf"
      {"void",		VOID},
#line 47 "c.prf"
      {"union",          UNION},
#line 44 "c.prf"
      {"struct",         STRUCT},
#line 46 "c.prf"
      {"typedef",        TYPEDEF},
#line 48 "c.prf"
      {"unsigned",       UNSIGNED},
#line 33 "c.prf"
      {"goto",           GOTO},
#line 51 "c.prf"
      {"while",          WHILE},
#line 45 "c.prf"
      {"switch",         SWITCH},
      {""},
#line 38 "c.prf"
      {"register",       REGISTER},
#line 21 "c.prf"
      {"case",           CASE},
#line 23 "c.prf"
      {"const",          CONST},
#line 41 "c.prf"
      {"signed",         SIGNED},
      {""},
#line 24 "c.prf"
      {"continue",       CONTINUE},
#line 29 "c.prf"
      {"enum",           ENUM},
#line 31 "c.prf"
      {"float",		FLOAT},
#line 30 "c.prf"
      {"extern",         EXTERN},
#line 26 "c.prf"
      {"do",             DO},
      {""},
#line 28 "c.prf"
      {"else",           ELSE},
#line 40 "c.prf"
      {"short",          SHORT},
#line 27 "c.prf"
      {"double",		DOUBLE},
#line 25 "c.prf"
      {"default",        DEFAULT},
      {""},
#line 19 "c.prf"
      {"auto",           AUTO},
#line 20 "c.prf"
      {"break",          BREAK},
#line 42 "c.prf"
      {"sizeof",         SIZEOF},
      {""}, {""}, {""}, {""},
#line 43 "c.prf"
      {"static",         STATIC},
      {""}, {""},
#line 22 "c.prf"
      {"char",		CHAR},
      {""}, {""}, {""},
#line 50 "c.prf"
      {"volatile",       VOLATILE},
#line 37 "c.prf"
      {"long",           LONG},
      {""},
#line 35 "c.prf"
      {"inline",		INLINE}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 52 "c.prf"

