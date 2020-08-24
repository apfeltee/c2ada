/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "grammar.y" /* yacc.c:339  */

/*
** This is a bare-bones prototype for an ANSI C parser.
**
** It is based on _The C Programming Language,
** Second Edition_, Kernighan and Ritchie, Printice Hall, 1988.
*/
#include <sys/types.h>

#include "c2ada.h"

/* from scan.c */
extern file_pos_t yypos;  /* in scan.c */
extern comment_block_pt fetch_comment_block(void);
extern void yield_typedef(bool);
extern void td(void);
extern void yyerror(char* msg);


#line 86 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    BAD_TOKEN = 258,
    MARKER = 259,
    INTEGER_CONSTANT = 260,
    CHARACTER_CONSTANT = 261,
    FLOATING_CONSTANT = 262,
    IDENTIFIER = 263,
    STRING = 264,
    TYPEDEF_NAME = 265,
    SIZEOF = 266,
    PTR_OP = 267,
    INC_OP = 268,
    DEC_OP = 269,
    LEFT_OP = 270,
    RIGHT_OP = 271,
    LE_OP = 272,
    GE_OP = 273,
    EQ_OP = 274,
    NE_OP = 275,
    AND_OP = 276,
    OR_OP = 277,
    MUL_ASSIGN = 278,
    DIV_ASSIGN = 279,
    MOD_ASSIGN = 280,
    ADD_ASSIGN = 281,
    SUB_ASSIGN = 282,
    LEFT_ASSIGN = 283,
    RIGHT_ASSIGN = 284,
    AND_ASSIGN = 285,
    XOR_ASSIGN = 286,
    OR_ASSIGN = 287,
    TYPEDEF = 288,
    EXTERN = 289,
    STATIC = 290,
    AUTO = 291,
    REGISTER = 292,
    INLINE = 293,
    CHAR = 294,
    SHORT = 295,
    INT = 296,
    LONG = 297,
    SIGNED = 298,
    UNSIGNED = 299,
    FLOAT = 300,
    DOUBLE = 301,
    CONST = 302,
    VOLATILE = 303,
    VOID = 304,
    STRUCT = 305,
    UNION = 306,
    ENUM = 307,
    ELLIPSIS = 308,
    DOTDOT = 309,
    CASE = 310,
    DEFAULT = 311,
    IF = 312,
    SWITCH = 313,
    WHILE = 314,
    DO = 315,
    FOR = 316,
    GOTO = 317,
    CONTINUE = 318,
    BREAK = 319,
    RETURN = 320,
    THEN = 321,
    ELSE = 322
  };
#endif
/* Tokens.  */
#define BAD_TOKEN 258
#define MARKER 259
#define INTEGER_CONSTANT 260
#define CHARACTER_CONSTANT 261
#define FLOATING_CONSTANT 262
#define IDENTIFIER 263
#define STRING 264
#define TYPEDEF_NAME 265
#define SIZEOF 266
#define PTR_OP 267
#define INC_OP 268
#define DEC_OP 269
#define LEFT_OP 270
#define RIGHT_OP 271
#define LE_OP 272
#define GE_OP 273
#define EQ_OP 274
#define NE_OP 275
#define AND_OP 276
#define OR_OP 277
#define MUL_ASSIGN 278
#define DIV_ASSIGN 279
#define MOD_ASSIGN 280
#define ADD_ASSIGN 281
#define SUB_ASSIGN 282
#define LEFT_ASSIGN 283
#define RIGHT_ASSIGN 284
#define AND_ASSIGN 285
#define XOR_ASSIGN 286
#define OR_ASSIGN 287
#define TYPEDEF 288
#define EXTERN 289
#define STATIC 290
#define AUTO 291
#define REGISTER 292
#define INLINE 293
#define CHAR 294
#define SHORT 295
#define INT 296
#define LONG 297
#define SIGNED 298
#define UNSIGNED 299
#define FLOAT 300
#define DOUBLE 301
#define CONST 302
#define VOLATILE 303
#define VOID 304
#define STRUCT 305
#define UNION 306
#define ENUM 307
#define ELLIPSIS 308
#define DOTDOT 309
#define CASE 310
#define DEFAULT 311
#define IF 312
#define SWITCH 313
#define WHILE 314
#define DO 315
#define FOR 316
#define GOTO 317
#define CONTINUE 318
#define BREAK 319
#define RETURN 320
#define THEN 321
#define ELSE 322

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 21 "grammar.y" /* yacc.c:355  */

    int 	val;
    node_t *	nod;
    symbol_t *	sym;
    typeinfo_t *typ;
    stmt_t * 	stmt;
    file_pos_t  pos;
    comment_block_pt com;

#line 270 "y.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 287 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1057

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  92
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  95
/* YYNRULES -- Number of rules.  */
#define YYNRULES  251
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  443

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   322

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    90,     2,     2,     2,    88,    82,     2,
      76,    74,    75,    85,    71,    86,    91,    87,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    73,    68,
      83,    72,    84,    79,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    77,     2,    78,    81,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,    80,    70,    89,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   151,   151,   160,   173,   180,   186,   196,   202,   208,
     215,   222,   229,   236,   252,   256,   274,   276,   284,   285,
     286,   290,   291,   296,   297,   302,   306,   309,   314,   318,
     319,   323,   324,   326,   327,   328,   329,   333,   334,   335,
     336,   337,   338,   345,   346,   350,   351,   352,   353,   354,
     355,   356,   357,   361,   362,   363,   364,   368,   369,   373,
     375,   377,   381,   382,   386,   387,   391,   392,   396,   397,
     402,   407,   408,   409,   410,   414,   415,   419,   420,   421,
     425,   426,   428,   432,   433,   437,   438,   442,   443,   447,
     448,   456,   457,   459,   461,   463,   465,   467,   473,   474,
     489,   491,   493,   498,   499,   500,   501,   505,   506,   510,
     511,   515,   516,   520,   522,   524,   529,   530,   534,   535,
     537,   542,   543,   547,   548,   553,   554,   555,   559,   561,
     563,   565,   567,   569,   571,   573,   575,   580,   584,   588,
     592,   596,   597,   598,   599,   600,   601,   605,   607,   609,
     615,   616,   620,   624,   627,   630,   633,   648,   651,   652,
     656,   659,   662,   668,   670,   672,   676,   677,   682,   683,
     684,   685,   686,   690,   691,   697,   698,   703,   704,   705,
     706,   707,   708,   709,   710,   711,   712,   713,   718,   719,
     725,   730,   731,   735,   736,   740,   741,   745,   746,   750,
     751,   755,   756,   757,   761,   762,   763,   764,   765,   769,
     770,   771,   775,   776,   777,   781,   782,   783,   784,   788,
     789,   794,   795,   796,   797,   798,   799,   804,   805,   806,
     807,   808,   809,   814,   816,   819,   822,   825,   828,   831,
     834,   839,   840,   841,   842,   846,   847,   851,   852,   853,
     861,   868
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "BAD_TOKEN", "MARKER",
  "INTEGER_CONSTANT", "CHARACTER_CONSTANT", "FLOATING_CONSTANT",
  "IDENTIFIER", "STRING", "TYPEDEF_NAME", "SIZEOF", "PTR_OP", "INC_OP",
  "DEC_OP", "LEFT_OP", "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP",
  "AND_OP", "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN",
  "ADD_ASSIGN", "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
  "XOR_ASSIGN", "OR_ASSIGN", "TYPEDEF", "EXTERN", "STATIC", "AUTO",
  "REGISTER", "INLINE", "CHAR", "SHORT", "INT", "LONG", "SIGNED",
  "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "STRUCT",
  "UNION", "ENUM", "ELLIPSIS", "DOTDOT", "CASE", "DEFAULT", "IF", "SWITCH",
  "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN", "THEN",
  "ELSE", "';'", "'{'", "'}'", "','", "'='", "':'", "')'", "'*'", "'('",
  "'['", "']'", "'?'", "'|'", "'^'", "'&'", "'<'", "'>'", "'+'", "'-'",
  "'/'", "'%'", "'~'", "'!'", "'.'", "$accept", "NS_ntd", "NS_td",
  "NS_scope_push", "NS_scope_pop", "NS_block_scope_push", "NS_struct_push",
  "NS_struct_pop", "NS_id", "NS_new_parm", "NS_is_typedef",
  "NS_direct_decl", "NS_ptr_decl", "identifier", "translation_unit",
  "external_declaration", "function_definition", "function_head",
  "function_body", "declaration", "untyped_declaration",
  "declaration_list", "declaration_specifiers", "storage_class_specifier",
  "type_specifier", "actual_type_specifier", "type_adjective",
  "type_qualifier", "struct_or_union_specifier", "struct_or_union",
  "struct_declaration_list", "init_declarator_list", "init_declarator",
  "struct_declaration", "specifier_qualifier_list",
  "struct_declarator_list", "struct_declarator", "enum_specifier",
  "enumerator_list", "enumerator", "opt_comma", "declarator",
  "direct_declarator", "function_declarator", "direct_function_declarator",
  "pointer", "type_qualifier_list", "parameter_type_list",
  "parameter_list", "parameter_declaration", "identifier_list",
  "initializer", "initializer_list", "type_name", "abstract_declarator",
  "direct_abstract_declarator", "lpar", "rpar", "lbra", "rbra",
  "statement", "labeled_statement", "expression_statement",
  "begin_compound", "compound_statement", "nested_declaration_list",
  "statement_list", "selection_statement", "iteration_statement",
  "opt_expr", "jump_statement", "expression", "assignment_expression",
  "assignment_operator", "conditional_expression", "constant_expression",
  "logical_or_expression", "logical_and_expression",
  "inclusive_or_expression", "exclusive_or_expression", "and_expression",
  "equality_expression", "relational_expression", "shift_expression",
  "additive_expression", "multiplicative_expression", "cast_expression",
  "unary_expression", "unary_operator", "postfix_expression",
  "primary_expression", "argument_expression_list", "constant", ".pos",
  ".com", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,    59,   123,
     125,    44,    61,    58,    41,    42,    40,    91,    93,    63,
     124,    94,    38,    60,    62,    43,    45,    47,    37,   126,
      33,    46
};
# endif

#define YYPACT_NINF -357

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-357)))

#define YYTABLE_NINF -126

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -357,    51,  -357,  -357,  -357,   937,  -357,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,    95,   947,
     -41,     7,  -357,   967,  -357,  -357,    37,   381,   876,  -357,
     892,    11,  -357,     0,    48,  -357,  -357,    75,    40,  -357,
    -357,    95,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
      20,  -357,  -357,    49,  -357,  -357,  -357,    -6,  -357,  -357,
     967,    37,  -357,  -357,    24,    27,  -357,  -357,  -357,  -357,
    -357,    40,    39,    59,  -357,    33,   793,  -357,  -357,    48,
    -357,    77,    48,    75,  -357,  -357,  -357,   150,   157,   102,
     108,   120,  -357,  -357,  -357,   431,  -357,  -357,   133,  -357,
    -357,   683,  -357,  -357,   144,   997,   132,   158,  -357,    55,
    -357,  -357,  -357,  -357,  -357,   805,   817,   817,  -357,   639,
    -357,  -357,  -357,  -357,  -357,   167,  -357,  -357,  -357,     8,
     227,   175,   198,   184,   214,   135,   221,   155,   -21,  -357,
    -357,   793,    63,  -357,  -357,  -357,  -357,  -357,    35,    48,
     212,   223,  -357,  -357,   150,   331,   226,   219,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
     977,   248,   246,   499,  -357,  -357,  -357,  -357,   517,   517,
    -357,  -357,  -357,    73,  -357,  -357,   376,  -357,   683,  -357,
    -357,  -357,    37,   144,   268,   314,   144,   639,  -357,   793,
    -357,  -357,   583,   661,  -357,   249,    90,  -357,  -357,   793,
     793,   793,   793,   793,   793,   793,   793,   793,   793,   793,
     793,   793,   793,   793,   793,   793,   793,   793,  -357,   319,
    -357,  -357,   769,   793,   319,  -357,  -357,   254,   104,   793,
     150,   262,  -357,   223,   320,  -357,    18,   331,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,   319,   265,   267,   781,  -357,
     263,  -357,  -357,   271,   517,   273,  -357,  -357,   793,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
     793,  -357,   173,  -357,    59,  -357,    19,  -357,    48,    57,
     793,  -357,  -357,  -357,  -357,  -357,   264,  -357,  -357,   107,
     793,  -357,   227,    76,   175,   198,   184,   214,   135,   135,
     221,   221,   221,   221,   155,   155,   -21,   -21,  -357,  -357,
    -357,  -357,  -357,  -357,   134,   -25,  -357,  -357,  -357,  -357,
    -357,  -357,   274,   275,  -357,   793,   266,  -357,   277,   320,
     793,   279,   272,   278,   280,   553,   281,   285,  -357,  -357,
    -357,   142,   553,  -357,   296,  -357,  -357,  -357,  -357,   657,
    -357,    48,  -357,   295,   793,   302,  -357,  -357,  -357,  -357,
    -357,  -357,   -18,  -357,   185,  -357,   793,   793,  -357,  -357,
    -357,  -357,  -357,    18,   287,   793,   312,   310,   553,   793,
     793,   793,   288,   793,  -357,  -357,  -357,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,  -357,
    -357,  -357,  -357,  -357,   553,  -357,   140,   151,   152,   308,
     317,   315,  -357,  -357,  -357,   553,   553,   553,   793,   793,
     321,  -357,  -357,   156,   323,   553,   324,   793,  -357,  -357,
     322,   553,  -357
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      16,     9,     1,   251,    17,     2,    91,    38,    39,    40,
      41,    42,    53,    54,    55,    56,    57,    58,   103,     0,
       0,     0,    18,     2,    19,    20,     3,     2,     2,    44,
       2,     0,    66,    68,    12,     3,    12,     4,     4,   107,
     105,   104,    50,    46,    47,    48,    49,    45,    62,    63,
       2,    43,    51,     2,    52,   137,    37,     0,    21,    29,
       2,     3,   250,     5,     0,     3,     3,    32,    34,    36,
      28,     4,     0,     0,    89,    10,     2,    23,    98,    13,
      13,     0,    12,     4,   108,   106,    15,     0,     0,    82,
       0,    61,   152,    22,    30,     2,    25,    26,     0,    24,
      67,     0,   139,   116,   101,     2,     0,   109,   111,     0,
     247,   248,   249,   241,   243,     0,     0,     0,   228,     2,
     227,   229,   230,   231,   232,     0,    93,   190,     2,   188,
     191,   193,   195,   197,   199,   201,   204,   209,   212,   215,
     219,     0,   221,   233,   242,    90,    99,     5,    10,    13,
      85,    88,    83,     3,     0,     2,     0,   241,   250,   250,
     250,   250,   250,   250,   250,   250,   250,   250,   250,   250,
       0,     0,     0,   157,   158,   141,   143,   142,     2,     2,
     144,   145,   146,     0,   173,   175,   219,    27,     0,     2,
     118,    96,     3,   100,    10,     0,   102,     2,   225,     0,
     222,   223,     2,     2,     3,     0,     0,   140,    94,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     2,
     239,   240,     0,     0,     2,    92,     5,     0,     0,     0,
      87,     0,    14,    88,     2,    64,     4,     2,   251,   251,
     251,   251,   251,   251,   251,     2,     0,     0,     0,   150,
      50,   153,   251,     0,     2,     0,   159,   151,     0,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,   177,
       0,   121,     0,    69,   114,     3,   125,     3,   126,    10,
       2,    95,   110,   112,   117,    97,     0,    72,    74,     3,
       0,   244,   192,     0,   194,   196,   198,   200,   202,   203,
     207,   208,   205,   206,   210,   211,   213,   214,   216,   217,
     218,   238,   235,   245,     0,     0,   237,     5,     5,    86,
      84,    80,     0,     0,    65,     0,     3,    75,    77,     2,
       0,     0,     0,     0,     0,     2,     0,     0,   169,   170,
     171,     0,     2,   155,     0,   154,   174,   176,   119,     0,
     113,   127,   115,    10,     2,     0,     5,     5,   133,   129,
       2,   226,     3,   124,    10,   220,     0,     0,   236,   234,
      81,    59,    78,     4,     0,     0,     0,     0,     2,     0,
       0,     0,     0,   167,   168,   172,   147,   156,   120,   122,
       5,   135,   131,     2,   138,   134,   128,   130,   189,   246,
      76,    70,    79,    60,     2,   149,     0,     0,     0,     0,
       0,   166,   136,   132,   148,     2,     2,     2,     0,   167,
     160,   162,   163,     0,     0,     2,     0,   167,   161,   164,
       0,     2,   165
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -357,    -5,     3,  -357,    53,  -357,   304,    58,  -357,  -357,
    -357,   357,   329,   -38,  -357,  -357,  -357,  -357,   350,    14,
    -357,   316,    32,  -357,   -75,  -357,  -357,    -9,  -357,  -357,
     165,   408,   355,  -231,   -98,  -357,    47,  -357,   289,   194,
     192,   -37,     5,   420,   413,    -2,  -357,   -70,  -357,   259,
     306,  -180,  -357,   283,  -178,  -266,   -32,  -201,   -30,  -117,
       6,  -357,  -357,  -357,    10,  -357,   282,  -357,  -357,  -356,
    -357,  -113,   -94,  -357,   -15,  -205,  -357,   250,   251,   258,
     269,   247,    66,    54,    29,    45,  -124,    87,  -357,  -357,
    -357,  -357,  -357,   888,   584
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    88,    73,    20,   365,    57,    90,   333,     3,   105,
      21,    74,   145,   172,     1,     4,    22,    23,    58,    59,
      25,    60,    61,    27,    28,    51,    29,    30,    52,    53,
     244,    65,    32,   245,   246,   336,   337,    54,   151,   152,
     241,    33,    82,    35,    36,    83,    41,   366,   107,   108,
     109,   189,   282,   205,   367,   288,    38,   368,   290,   126,
     174,   175,   176,    62,   177,   178,   179,   180,   181,   420,
     182,   183,   184,   280,   185,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,   139,   186,   141,   142,
     143,   324,   144,    95,     5
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      19,    81,    75,    37,    76,   106,   206,   190,   281,    39,
      34,   208,    89,   334,   287,    91,    40,   228,    19,    24,
     361,   204,    19,    19,    37,    19,     6,     6,    86,    64,
     209,    34,    84,    63,   329,    55,    72,    26,    77,    85,
      56,   103,    79,   103,   202,     6,   268,    75,     6,    76,
     148,     2,    76,   379,   225,    19,  -125,    86,    -4,    67,
      68,   127,    69,    92,    64,     6,   226,   227,    98,    99,
      63,   125,    -3,   434,    94,   229,   230,   231,   237,    70,
     202,   440,    71,     6,   206,   370,   206,   210,   149,    87,
     170,   335,    97,    18,   190,    -4,    -3,   303,    71,   204,
      19,   318,   319,   320,   297,   298,   361,   104,   334,   236,
     203,   101,    18,    -4,    19,    18,    96,   148,    -7,    76,
     325,   373,   202,   125,    -4,    -3,   195,   202,   202,   196,
     382,    -5,    18,    -4,    -3,   387,   102,   192,   323,   232,
     233,   267,    16,    17,   268,   351,   203,   268,   171,   376,
      19,   147,   217,   218,   234,   285,   242,   191,   150,   403,
     289,   268,   401,   140,   301,   405,   406,   153,    19,   202,
      18,   154,   202,   369,   356,   195,   375,   155,   328,   399,
     412,  -123,    18,    -4,   283,   266,   357,    94,   203,    -7,
     286,   321,    19,   203,   203,   284,   326,    19,    19,   422,
     235,   187,   198,   200,   201,   377,   193,   299,   378,   338,
     395,   268,    -5,   268,   425,    -5,    -5,   347,   219,   220,
      -5,    -5,   268,   268,   127,   426,   427,   268,   140,   194,
     436,   263,   265,   215,   216,   203,   221,   222,   203,    19,
     223,   224,    19,   358,   359,   207,   291,   402,   211,   295,
     314,   315,    81,   407,   289,   212,   363,   289,   364,    -5,
      18,    -4,    -3,   242,   202,   190,   214,   374,   316,   317,
     266,   310,   311,   312,   313,   127,   416,   417,   418,   213,
     421,   308,   309,   409,   239,   125,   423,   286,   360,   191,
     362,   149,   -15,   400,   240,   247,   140,   372,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   433,   421,   354,   261,   262,
     127,   292,   294,   300,   421,   127,   140,    86,   327,   363,
     203,   364,   331,   348,    19,   349,    -3,   383,   371,   384,
     374,   353,   374,   355,   380,   381,   338,   419,   389,   127,
     385,   392,   388,   394,   390,   411,   391,   393,   396,   125,
      12,   408,    13,    14,    15,   125,   397,    16,    17,    -5,
     127,    12,   372,    13,    14,    15,   404,   140,    16,    17,
     291,   295,   413,   414,   428,   429,   268,   140,   435,   -31,
      -8,   437,   439,    78,   415,   156,   441,   386,   125,   269,
     270,   271,   272,   273,   274,   275,   276,   277,   278,   146,
      93,   173,   339,    31,   -11,     7,     8,     9,    10,    11,
     424,    12,   140,    13,    14,    15,   100,   140,    16,    17,
     410,   430,   431,   432,   330,   332,   110,   111,   112,   157,
     114,   438,   115,   243,   116,   117,    66,   442,   279,   -31,
      80,   140,   -31,   293,   238,   -31,   -31,   -31,   -31,   302,
     264,   307,   304,   140,   -11,     7,     8,     9,    10,    11,
     305,    12,   140,    13,    14,    15,     0,     0,    16,    17,
     296,     0,   306,     0,     0,     0,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,     0,     0,   169,
      -6,    -5,     0,     0,     0,     0,   118,   119,     0,    -2,
       0,     0,     0,   120,     0,     0,   121,   122,     0,     0,
     123,   124,   110,   111,   112,   157,   114,     0,   115,     0,
     116,   117,   -11,     7,     8,     9,    10,    11,    -2,    12,
      -2,    13,    14,    15,    -2,    -2,    16,    17,    -2,    -2,
      -2,    -2,     0,     0,     0,     0,     0,     0,   110,   111,
     112,   157,   114,     0,   115,     0,   116,   117,     0,     0,
       0,     0,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,     0,     0,   169,    -6,    -5,     0,     0,
       0,   -71,   118,   119,     0,     0,     0,     0,     0,   120,
       0,     0,   121,   122,     0,     0,   123,   124,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,     0,
       0,   169,    -6,    12,     0,    13,    14,    15,   118,   119,
      16,    17,     0,     0,     0,   120,     0,     0,   121,   122,
       0,     0,   123,   124,   110,   111,   112,   113,   114,     0,
     115,     0,   116,   117,     0,     0,   -71,   -71,   -71,   -71,
     -71,     0,   110,   111,   112,   113,   114,     0,   115,   -73,
     116,   117,     0,     0,     0,     0,     0,     0,     0,    12,
       0,    13,    14,    15,     0,     0,    16,    17,   110,   111,
     112,   113,   114,     0,   115,     0,   116,   117,     0,     0,
       0,    12,     0,    13,    14,    15,     0,     0,    16,    17,
       0,     0,     0,     0,   118,   119,     0,     0,     0,     0,
       0,   120,     0,     0,   121,   122,   188,   398,   123,   124,
       0,     0,   118,   119,   -73,   -73,   -73,   -73,   -73,   120,
       0,     0,   121,   122,     0,     0,   123,   124,     0,     0,
       0,     0,   188,     0,     0,     0,     0,     0,   118,   119,
       0,     0,     0,     0,     0,   120,     0,     0,   121,   122,
       0,     0,   123,   124,   110,   111,   112,   113,   114,     0,
     115,     0,   116,   117,     0,     0,   110,   111,   112,   113,
     114,     0,   115,     0,   116,   117,     0,     0,   110,   111,
     112,   113,   114,     0,   115,     0,   116,   117,     0,     0,
     110,   111,   112,   113,   114,     0,   115,     0,   116,   117,
       0,     0,   110,   111,   112,   113,   114,     0,   115,     0,
     116,   117,   340,   341,   342,   343,   344,   345,   346,     0,
       0,     0,     0,   322,   118,   119,   352,     0,     0,   350,
       0,   120,     0,     0,   121,   122,   118,   119,   123,   124,
       0,     0,     0,   120,     0,     0,   121,   122,   118,   119,
     123,   124,     0,     0,     0,   120,     0,     0,   121,   122,
     118,   197,   123,   124,   -33,     0,     0,   120,     0,     0,
     121,   122,   118,   199,   123,   124,     0,     0,     0,   120,
     -35,     0,   121,   122,     0,     0,   123,   124,     0,   -11,
       7,     8,     9,    10,    11,     0,    12,     0,    13,    14,
      15,     0,     0,    16,    17,   -11,     7,     8,     9,    10,
      11,     0,    12,     0,    13,    14,    15,     0,     0,    16,
      17,     0,     0,     0,   -33,     6,     0,   -33,     0,     0,
     -33,   -33,   -33,   -33,     0,     0,     0,    42,     0,     0,
     -35,     0,     0,   -35,     0,     0,   -35,   -35,   -35,   -35,
     -11,     7,     8,     9,    10,    11,     0,    12,     0,    13,
      14,    15,     0,     0,    16,    17,    43,   260,    44,     0,
       0,     0,    45,    46,     0,     0,    47,    48,    49,    50,
     -11,     7,     8,     9,    10,    11,     0,    12,     0,    13,
      14,    15,    18,    -4,    16,    17,    43,     0,    44,     0,
       0,     0,    45,    46,     0,     0,    47,    48,    49,    50,
     -11,     7,     8,     9,    10,    11,    -6,    12,     0,    13,
      14,    15,     0,     0,    16,    17,   248,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,   259
};

static const yytype_int16 yycheck[] =
{
       5,    38,    34,     5,    34,    75,   119,   101,   188,    18,
       5,   128,    50,   244,   192,    53,    18,   141,    23,     5,
     286,   119,    27,    28,    26,    30,     8,     8,     8,    26,
      22,    26,    41,    23,   239,    76,    33,     5,    35,    41,
      33,     8,    37,     8,   119,     8,    71,    79,     8,    79,
      82,     0,    82,    78,    75,    60,    74,     8,    76,    27,
      28,    76,    30,    69,    61,     8,    87,    88,    65,    66,
      60,    76,    72,   429,    60,    12,    13,    14,   148,    68,
     155,   437,    71,     8,   197,   290,   199,    79,    83,    69,
      95,    73,    68,    75,   188,    76,    77,   210,    71,   197,
     105,   225,   226,   227,   202,   203,   372,    74,   339,    74,
     119,    72,    75,    76,   119,    75,    63,   149,    69,   149,
     233,   299,   197,   128,    76,    77,    71,   202,   203,    74,
     335,    74,    75,    76,    77,   340,    77,   105,   232,    76,
      77,    68,    47,    48,    71,   258,   155,    71,    95,    73,
     155,    74,    17,    18,    91,   192,   153,   104,     8,   364,
     192,    71,   363,    76,    74,   366,   367,    10,   173,   244,
      75,    69,   247,   290,   268,    71,   300,    69,    74,   359,
     385,    74,    75,    76,   189,   179,   280,   173,   197,    69,
     192,   229,   197,   202,   203,   192,   234,   202,   203,   400,
     147,    68,   115,   116,   117,    71,    74,   204,    74,   246,
      68,    71,    68,    71,    74,    71,    72,   255,    83,    84,
      76,    77,    71,    71,   239,    74,    74,    71,   141,    71,
      74,   178,   179,    19,    20,   244,    15,    16,   247,   244,
      85,    86,   247,    70,    71,    78,   193,   364,    21,   196,
     221,   222,   289,   370,   286,    80,   288,   289,   288,    74,
      75,    76,    77,   260,   339,   359,    82,   299,   223,   224,
     264,   217,   218,   219,   220,   290,   389,   390,   391,    81,
     393,   215,   216,   377,    72,   290,   403,   289,   285,   236,
     287,   286,    73,   363,    71,    69,   209,   299,   211,   212,
     213,   214,   215,   216,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   428,   429,   264,    70,    73,
     335,    53,     8,    74,   437,   340,   239,     8,    74,   361,
     339,   361,    70,    68,   339,    68,    73,    71,    74,   336,
     372,    70,   374,    70,    70,    70,   383,    59,    76,   364,
      73,   345,    73,    68,    76,    68,    76,    76,   352,   364,
      40,   376,    42,    43,    44,   370,    70,    47,    48,    74,
     385,    40,   374,    42,    43,    44,    74,   290,    47,    48,
     327,   328,    70,    73,    76,    68,    71,   300,    67,     8,
      70,    68,    68,    36,   388,    91,    74,   339,   403,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    80,
      60,    95,   247,     5,    33,    34,    35,    36,    37,    38,
     414,    40,   335,    42,    43,    44,    71,   340,    47,    48,
     383,   425,   426,   427,   240,   243,     5,     6,     7,     8,
       9,   435,    11,   154,    13,    14,    26,   441,    72,    68,
      37,   364,    71,   194,   148,    74,    75,    76,    77,   209,
     178,   214,   211,   376,    33,    34,    35,    36,    37,    38,
     212,    40,   385,    42,    43,    44,    -1,    -1,    47,    48,
     197,    -1,   213,    -1,    -1,    -1,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    -1,    -1,    68,
      69,    70,    -1,    -1,    -1,    -1,    75,    76,    -1,    10,
      -1,    -1,    -1,    82,    -1,    -1,    85,    86,    -1,    -1,
      89,    90,     5,     6,     7,     8,     9,    -1,    11,    -1,
      13,    14,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    -1,    -1,    -1,    -1,    -1,    -1,     5,     6,
       7,     8,     9,    -1,    11,    -1,    13,    14,    -1,    -1,
      -1,    -1,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    68,    69,    70,    -1,    -1,
      -1,     8,    75,    76,    -1,    -1,    -1,    -1,    -1,    82,
      -1,    -1,    85,    86,    -1,    -1,    89,    90,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    68,    69,    40,    -1,    42,    43,    44,    75,    76,
      47,    48,    -1,    -1,    -1,    82,    -1,    -1,    85,    86,
      -1,    -1,    89,    90,     5,     6,     7,     8,     9,    -1,
      11,    -1,    13,    14,    -1,    -1,    73,    74,    75,    76,
      77,    -1,     5,     6,     7,     8,     9,    -1,    11,     8,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    42,    43,    44,    -1,    -1,    47,    48,     5,     6,
       7,     8,     9,    -1,    11,    -1,    13,    14,    -1,    -1,
      -1,    40,    -1,    42,    43,    44,    -1,    -1,    47,    48,
      -1,    -1,    -1,    -1,    75,    76,    -1,    -1,    -1,    -1,
      -1,    82,    -1,    -1,    85,    86,    69,    70,    89,    90,
      -1,    -1,    75,    76,    73,    74,    75,    76,    77,    82,
      -1,    -1,    85,    86,    -1,    -1,    89,    90,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    75,    76,
      -1,    -1,    -1,    -1,    -1,    82,    -1,    -1,    85,    86,
      -1,    -1,    89,    90,     5,     6,     7,     8,     9,    -1,
      11,    -1,    13,    14,    -1,    -1,     5,     6,     7,     8,
       9,    -1,    11,    -1,    13,    14,    -1,    -1,     5,     6,
       7,     8,     9,    -1,    11,    -1,    13,    14,    -1,    -1,
       5,     6,     7,     8,     9,    -1,    11,    -1,    13,    14,
      -1,    -1,     5,     6,     7,     8,     9,    -1,    11,    -1,
      13,    14,   248,   249,   250,   251,   252,   253,   254,    -1,
      -1,    -1,    -1,    74,    75,    76,   262,    -1,    -1,    68,
      -1,    82,    -1,    -1,    85,    86,    75,    76,    89,    90,
      -1,    -1,    -1,    82,    -1,    -1,    85,    86,    75,    76,
      89,    90,    -1,    -1,    -1,    82,    -1,    -1,    85,    86,
      75,    76,    89,    90,     8,    -1,    -1,    82,    -1,    -1,
      85,    86,    75,    76,    89,    90,    -1,    -1,    -1,    82,
       8,    -1,    85,    86,    -1,    -1,    89,    90,    -1,    33,
      34,    35,    36,    37,    38,    -1,    40,    -1,    42,    43,
      44,    -1,    -1,    47,    48,    33,    34,    35,    36,    37,
      38,    -1,    40,    -1,    42,    43,    44,    -1,    -1,    47,
      48,    -1,    -1,    -1,    68,     8,    -1,    71,    -1,    -1,
      74,    75,    76,    77,    -1,    -1,    -1,    10,    -1,    -1,
      68,    -1,    -1,    71,    -1,    -1,    74,    75,    76,    77,
      33,    34,    35,    36,    37,    38,    -1,    40,    -1,    42,
      43,    44,    -1,    -1,    47,    48,    39,    10,    41,    -1,
      -1,    -1,    45,    46,    -1,    -1,    49,    50,    51,    52,
      33,    34,    35,    36,    37,    38,    -1,    40,    -1,    42,
      43,    44,    75,    76,    47,    48,    39,    -1,    41,    -1,
      -1,    -1,    45,    46,    -1,    -1,    49,    50,    51,    52,
      33,    34,    35,    36,    37,    38,    69,    40,    -1,    42,
      43,    44,    -1,    -1,    47,    48,   158,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   106,     0,   100,   107,   186,     8,    34,    35,    36,
      37,    38,    40,    42,    43,    44,    47,    48,    75,    93,
      95,   102,   108,   109,   111,   112,   114,   115,   116,   118,
     119,   123,   124,   133,   134,   135,   136,   137,   148,   119,
     137,   138,    10,    39,    41,    45,    46,    49,    50,    51,
      52,   117,   120,   121,   129,    76,    33,    97,   110,   111,
     113,   114,   155,   156,    94,   123,   135,   114,   114,   114,
      68,    71,    94,    94,   103,   148,   150,    94,   103,   134,
     136,   133,   134,   137,   119,   137,     8,    69,    93,   105,
      98,   105,    69,   110,   111,   185,    96,    68,    94,    94,
     124,    72,    77,     8,    74,   101,   139,   140,   141,   142,
       5,     6,     7,     8,     9,    11,    13,    14,    75,    76,
      82,    85,    86,    89,    90,    93,   151,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   184,   104,   104,    74,   148,   134,
       8,   130,   131,    10,    69,    69,    98,     8,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    68,
      93,    96,   105,   113,   152,   153,   154,   156,   157,   158,
     159,   160,   162,   163,   164,   166,   179,    68,    69,   143,
     164,    96,   114,    74,    71,    71,    74,    76,   179,    76,
     179,   179,   116,   119,   126,   145,   163,    78,   151,    22,
      79,    21,    80,    81,    82,    19,    20,    17,    18,    83,
      84,    15,    16,    85,    86,    75,    87,    88,   178,    12,
      13,    14,    76,    77,    91,    96,    74,   139,   142,    72,
      71,   132,    94,   130,   122,   125,   126,    69,   185,   185,
     185,   185,   185,   185,   185,   185,   185,   185,   185,   185,
      10,    70,    73,    96,   158,    96,   152,    68,    71,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    72,
     165,   143,   144,    93,    94,   133,   137,   146,   147,   148,
     150,    96,    53,   141,     8,    96,   145,   126,   126,    94,
      74,    74,   169,   163,   170,   171,   172,   173,   174,   174,
     175,   175,   175,   175,   176,   176,   177,   177,   178,   178,
     178,   105,    74,   164,   183,   163,   105,    74,    74,   167,
     131,    70,   132,    99,   125,    73,   127,   128,   133,   122,
     186,   186,   186,   186,   186,   186,   186,   105,    68,    68,
      68,   163,   186,    70,    96,    70,   164,   164,    70,    71,
      94,   147,    94,   148,   150,    96,   139,   146,   149,   151,
     167,    74,   137,   146,   148,   178,    73,    71,    74,    78,
      70,    70,   167,    71,    94,    73,    99,   167,    73,    76,
      76,    76,   152,    76,    68,    68,   152,    70,    70,   143,
     139,   149,   151,   167,    74,   149,   149,   151,   166,   164,
     128,    68,   167,    70,    73,   152,   163,   163,   163,    59,
     161,   163,   149,   151,   152,    74,    74,    74,    76,    68,
     152,   152,   152,   163,   161,    67,    74,    68,   152,    68,
     161,    74,   152
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   105,   106,   106,   107,   107,
     107,   108,   108,   109,   109,   110,   111,   111,   112,   113,
     113,   114,   114,   114,   114,   114,   114,   115,   115,   115,
     115,   115,   115,   116,   116,   117,   117,   117,   117,   117,
     117,   117,   117,   118,   118,   118,   118,   119,   119,   120,
     120,   120,   121,   121,   122,   122,   123,   123,   124,   124,
     125,   126,   126,   126,   126,   127,   127,   128,   128,   128,
     129,   129,   129,   130,   130,   131,   131,   132,   132,   133,
     133,   134,   134,   134,   134,   134,   134,   134,   135,   135,
     136,   136,   136,   137,   137,   137,   137,   138,   138,   139,
     139,   140,   140,   141,   141,   141,   142,   142,   143,   143,
     143,   144,   144,   145,   145,   146,   146,   146,   147,   147,
     147,   147,   147,   147,   147,   147,   147,   148,   149,   150,
     151,   152,   152,   152,   152,   152,   152,   153,   153,   153,
     154,   154,   155,   156,   156,   156,   156,   157,   158,   158,
     159,   159,   159,   160,   160,   160,   161,   161,   162,   162,
     162,   162,   162,   163,   163,   164,   164,   165,   165,   165,
     165,   165,   165,   165,   165,   165,   165,   165,   166,   166,
     167,   168,   168,   169,   169,   170,   170,   171,   171,   172,
     172,   173,   173,   173,   174,   174,   174,   174,   174,   175,
     175,   175,   176,   176,   176,   177,   177,   177,   177,   178,
     178,   179,   179,   179,   179,   179,   179,   180,   180,   180,
     180,   180,   180,   181,   181,   181,   181,   181,   181,   181,
     181,   182,   182,   182,   182,   183,   183,   184,   184,   184,
     185,   186
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     3,     1,     0,     2,     3,     3,
       3,     2,     3,     2,     3,     2,     3,     4,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     2,     1,     1,
       1,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       7,     2,     1,     1,     1,     2,     1,     3,     1,     5,
       4,     1,     2,     1,     2,     1,     3,     1,     2,     3,
       5,     6,     2,     1,     3,     1,     3,     1,     0,     2,
       3,     1,     4,     3,     4,     5,     4,     5,     2,     3,
       4,     3,     4,     1,     2,     2,     3,     1,     2,     1,
       3,     1,     3,     4,     3,     4,     1,     3,     1,     3,
       4,     1,     3,     2,     3,     1,     1,     2,     3,     2,
       3,     3,     4,     2,     3,     3,     4,     2,     2,     2,
       2,     1,     1,     1,     1,     1,     1,     4,     6,     5,
       2,     2,     2,     4,     5,     5,     6,     1,     1,     2,
       7,     9,     7,     7,     9,    11,     1,     0,     4,     3,
       3,     3,     4,     1,     3,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     5,
       1,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     3,     1,     3,     3,     3,     3,     1,
       3,     3,     1,     3,     3,     1,     3,     3,     3,     1,
       4,     1,     2,     2,     2,     2,     4,     1,     1,     1,
       1,     1,     1,     1,     4,     3,     4,     3,     3,     2,
       2,     1,     1,     1,     3,     1,     3,     1,     1,     1,
       0,     0
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 151 "grammar.y" /* yacc.c:1646  */
    {
        yield_typedef(false);
    }
#line 1846 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 160 "grammar.y" /* yacc.c:1646  */
    {
        yield_typedef(true);
    }
#line 1854 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 173 "grammar.y" /* yacc.c:1646  */
    {
        scope_push(Unspecified_scope);
        td();
    }
#line 1863 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 180 "grammar.y" /* yacc.c:1646  */
    {
        scope_pop();
    }
#line 1871 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 186 "grammar.y" /* yacc.c:1646  */
    {
        scope_push(Block_scope);
        td();
    }
#line 1880 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 196 "grammar.y" /* yacc.c:1646  */
    {
        td();
    }
#line 1888 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 202 "grammar.y" /* yacc.c:1646  */
    {
        /* struct_pop(); */
    }
#line 1896 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 208 "grammar.y" /* yacc.c:1646  */
    {
        /* new_declaration(name_space_decl); */
    }
#line 1904 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 215 "grammar.y" /* yacc.c:1646  */
    {
        td();
    }
#line 1912 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 222 "grammar.y" /* yacc.c:1646  */
    {
        /* set_typedef(); */
    }
#line 1920 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 229 "grammar.y" /* yacc.c:1646  */
    {
        /* direct_declarator(); */
    }
#line 1928 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 236 "grammar.y" /* yacc.c:1646  */
    {
        /* pointer_declarator(); */
    }
#line 1936 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 253 "grammar.y" /* yacc.c:1646  */
    {
            (yyval.nod) = id_from_typedef((yyvsp[-1].typ));
        }
#line 1944 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 284 "grammar.y" /* yacc.c:1646  */
    {define_func((yyvsp[0].sym),(yyvsp[-1].com));}
#line 1950 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 285 "grammar.y" /* yacc.c:1646  */
    {typed_external_decl((yyvsp[0].sym),(yyvsp[-1].com));}
#line 1956 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 286 "grammar.y" /* yacc.c:1646  */
    {(yyvsp[0].nod)->comment = (yyvsp[-1].com);}
#line 1962 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 290 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = new_func((yyvsp[-1].sym),(yyvsp[0].stmt));}
#line 1968 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 292 "grammar.y" /* yacc.c:1646  */
    { KnR_params((yyvsp[-2].sym), (yyvsp[-1].sym)); (yyval.sym) = new_func((yyvsp[-2].sym),(yyvsp[0].stmt));}
#line 1974 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 296 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = function_spec(0, (yyvsp[-1].nod), 0);}
#line 1980 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 298 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = function_spec((yyvsp[-2].typ), (yyvsp[-1].nod), 0);}
#line 1986 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 302 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = (yyvsp[-1].stmt); }
#line 1992 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 307 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = novar_declaration((yyvsp[-2].typ));
	 if ((yyval.sym)) (yyval.sym)->comment = fetch_comment_block();}
#line 1999 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 310 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = var_declaration((yyvsp[-3].typ), (yyvsp[-2].nod)); (yyval.sym)->comment = fetch_comment_block();}
#line 2005 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 319 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = concat_symbols((yyvsp[-1].sym),(yyvsp[0].sym));}
#line 2011 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 323 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typemod((yyvsp[0].val));}
#line 2017 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 325 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = concat_types(typeof_typemod((yyvsp[-1].val)), (yyvsp[0].typ));}
#line 2023 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 327 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = concat_types((yyvsp[-1].typ), (yyvsp[0].typ));}
#line 2029 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 328 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typemod((yyvsp[0].val));}
#line 2035 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 329 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = concat_types(typeof_typemod((yyvsp[-1].val)), (yyvsp[0].typ));}
#line 2041 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 333 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_TYPEDEF;}
#line 2047 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 334 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_EXTERN;}
#line 2053 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 335 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_STATIC;}
#line 2059 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 336 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_AUTO;}
#line 2065 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 337 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_REGISTER;}
#line 2071 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 338 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_INLINE;}
#line 2077 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 345 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = (yyvsp[0].typ);}
#line 2083 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 346 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typemod((yyvsp[0].val));}
#line 2089 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 350 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_void();}
#line 2095 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 351 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_char();}
#line 2101 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 352 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_int();}
#line 2107 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 353 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_float();}
#line 2113 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 354 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_double();}
#line 2119 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 356 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_specifier((yyvsp[0].sym));}
#line 2125 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 357 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_specifier((yyvsp[0].sym));}
#line 2131 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 361 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_SHORT;}
#line 2137 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 362 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_LONG;}
#line 2143 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 363 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_SIGNED;}
#line 2149 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 364 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_UNSIGNED;}
#line 2155 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 368 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_CONST;}
#line 2161 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 369 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = TYPEMOD_VOLATILE;}
#line 2167 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 374 "grammar.y" /* yacc.c:1646  */
    { (yyval.sym) = anonymous_rec((yyvsp[-5].val), (yyvsp[-2].sym));}
#line 2173 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 376 "grammar.y" /* yacc.c:1646  */
    { (yyval.sym) = named_rec((yyvsp[-6].val), (yyvsp[-5].nod), (yyvsp[-2].sym));}
#line 2179 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 377 "grammar.y" /* yacc.c:1646  */
    { (yyval.sym) = rec_reference((yyvsp[-1].val), (yyvsp[0].nod));}
#line 2185 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 381 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = 0;}
#line 2191 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 382 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = 1;}
#line 2197 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 387 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = concat_symbols((yyvsp[-1].sym),(yyvsp[0].sym));}
#line 2203 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 392 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_List, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2209 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 397 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Assign, (yyvsp[-4].nod), (yyvsp[-1].nod));}
#line 2215 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 403 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = field_declaration((yyvsp[-3].typ), (yyvsp[-2].nod));}
#line 2221 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 407 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typespec((yyvsp[0].typ));}
#line 2227 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 408 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typespec(concat_types((yyvsp[-1].typ), (yyvsp[0].typ)));}
#line 2233 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 409 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typespec(typeof_typemod((yyvsp[0].val)));}
#line 2239 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 410 "grammar.y" /* yacc.c:1646  */
    {(yyval.typ) = typeof_typespec(concat_types(typeof_typemod((yyvsp[-1].val)),(yyvsp[0].typ)));}
#line 2245 "y.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 415 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_List, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2251 "y.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 420 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Bit_Field, 0, (yyvsp[0].nod));}
#line 2257 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 421 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Bit_Field, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2263 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 425 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = anonymous_enum((yyvsp[-2].sym));}
#line 2269 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 427 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = named_enum((yyvsp[-4].nod), (yyvsp[-2].sym));}
#line 2275 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 428 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = enum_reference((yyvsp[0].nod));}
#line 2281 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 433 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = concat_symbols((yyvsp[-2].sym),(yyvsp[0].sym));}
#line 2287 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 437 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = grok_enumerator((yyvsp[0].nod),0);}
#line 2293 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 438 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = grok_enumerator((yyvsp[-2].nod),(yyvsp[0].nod));}
#line 2299 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 448 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = access_to((yyvsp[-2].nod), (yyvsp[-1].nod));}
#line 2305 "y.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 458 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = (yyvsp[-2].nod);}
#line 2311 "y.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 460 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, (yyvsp[-2].nod), 0);}
#line 2317 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 462 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, (yyvsp[-3].nod), (yyvsp[-1].nod));}
#line 2323 "y.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 464 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-4].nod), new_node(_Sym,(yyvsp[-2].sym)));}
#line 2329 "y.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 466 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-3].nod), 0);}
#line 2335 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 468 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-4].nod), (yyvsp[-2].nod));}
#line 2341 "y.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 475 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = access_to((yyvsp[-2].nod), (yyvsp[-1].nod));}
#line 2347 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 490 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-3].nod), new_node(_Sym,(yyvsp[-1].sym)));}
#line 2353 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 492 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-2].nod), 0);}
#line 2359 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 494 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-3].nod), (yyvsp[-1].nod));}
#line 2365 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 498 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Indirect, 0);}
#line 2371 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 499 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Indirect, 0);}
#line 2377 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 500 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Indirect, (yyvsp[0].nod));}
#line 2383 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 501 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Indirect, (yyvsp[0].nod));}
#line 2389 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 506 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = (yyvsp[-1].val) | (yyvsp[0].val);}
#line 2395 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 510 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = (yyvsp[0].sym);}
#line 2401 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 511 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = concat_ellipsis((yyvsp[-2].sym));}
#line 2407 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 516 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = concat_symbols((yyvsp[-2].sym),(yyvsp[0].sym));}
#line 2413 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 521 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = named_abstract_param((yyvsp[-2].typ), (yyvsp[-1].nod));}
#line 2419 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 523 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = noname_simple_param((yyvsp[-1].typ));}
#line 2425 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 525 "grammar.y" /* yacc.c:1646  */
    {(yyval.sym) = noname_abstract_param((yyvsp[-2].typ), (yyvsp[-1].nod));}
#line 2431 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 530 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_List, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2437 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 536 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Aggregate,reshape_list((yyvsp[-1].nod)));}
#line 2443 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 538 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Aggregate,reshape_list((yyvsp[-2].nod)));}
#line 2449 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 543 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_List, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2455 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 549 "grammar.y" /* yacc.c:1646  */
    { (yyval.typ) = abstract_declarator_type( (yyvsp[-2].typ), (yyvsp[0].nod) ); }
#line 2461 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 555 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = access_to((yyvsp[-1].nod), (yyvsp[0].nod));}
#line 2467 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 560 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = (yyvsp[-1].nod);}
#line 2473 "y.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 562 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, 0, 0);}
#line 2479 "y.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 564 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, 0, (yyvsp[-1].nod));}
#line 2485 "y.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 566 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, (yyvsp[-2].nod), 0);}
#line 2491 "y.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 568 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, (yyvsp[-3].nod), (yyvsp[-1].nod));}
#line 2497 "y.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 570 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, 0, 0);}
#line 2503 "y.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 572 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, 0, new_node(_Sym,(yyvsp[-1].sym)));}
#line 2509 "y.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 574 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-2].nod), 0);}
#line 2515 "y.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 576 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-3].nod), new_node(_Sym,(yyvsp[-1].sym)));}
#line 2521 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 606 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Labelled( (yyvsp[-3].nod), (yyvsp[-1].com), (yyvsp[0].stmt) ); }
#line 2527 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 608 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_Case( (yyvsp[-4].pos), (yyvsp[-3].com), (yyvsp[-2].nod), (yyvsp[0].stmt) );}
#line 2533 "y.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 610 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_Default( (yyvsp[-3].pos), (yyvsp[-2].com), (yyvsp[0].stmt) );}
#line 2539 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 615 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_Null((yyvsp[0].pos));}
#line 2545 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 616 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_Expr( (yyvsp[-1].nod));}
#line 2551 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 624 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Compound( (yyvsp[-2].pos), (symbol_t*)0, (stmt_pt)0 );
    }
#line 2559 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 627 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Compound( (yyvsp[-3].pos), (symbol_t*)0, (yyvsp[-2].stmt) );
    }
#line 2567 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 630 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Compound( (yyvsp[-3].pos), (yyvsp[-2].sym), (stmt_pt)0 );
    }
#line 2575 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 633 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Compound( (yyvsp[-4].pos), (yyvsp[-3].sym), (yyvsp[-2].stmt) );
    }
#line 2583 "y.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 648 "grammar.y" /* yacc.c:1646  */
    { (yyval.sym) = nested_declarations((yyvsp[0].sym)); }
#line 2589 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 651 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_list((yyvsp[0].stmt)); }
#line 2595 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 652 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = append_stmt( (yyvsp[-1].stmt), (yyvsp[0].stmt) );}
#line 2601 "y.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 656 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_If( (yyvsp[-5].pos), (yyvsp[-4].com), (yyvsp[-2].nod), (yyvsp[0].stmt) );
    }
#line 2609 "y.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 659 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Ifelse( (yyvsp[-7].pos), (yyvsp[-6].com), (yyvsp[-4].nod), (yyvsp[-2].stmt), (yyvsp[0].stmt) );
    }
#line 2617 "y.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 662 "grammar.y" /* yacc.c:1646  */
    {
        (yyval.stmt) = new_stmt_Switch( (yyvsp[-5].pos), (yyvsp[-4].com), (yyvsp[-2].nod), (yyvsp[0].stmt) );
    }
#line 2625 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 669 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_While( (yyvsp[-5].pos), (yyvsp[-4].com), (yyvsp[-2].nod), (yyvsp[0].stmt) );}
#line 2631 "y.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 671 "grammar.y" /* yacc.c:1646  */
    {(yyval.stmt) = new_stmt_Do( (yyvsp[-7].pos), (yyvsp[-6].com), (yyvsp[-2].nod), (yyvsp[-5].stmt) );}
#line 2637 "y.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 673 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_For( (yyvsp[-9].pos), (yyvsp[-8].com), (yyvsp[-6].nod), (yyvsp[-4].nod), (yyvsp[-2].nod), (yyvsp[0].stmt) ); }
#line 2643 "y.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 677 "grammar.y" /* yacc.c:1646  */
    { (yyval.nod) = (node_t *) 0; }
#line 2649 "y.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 682 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Goto( (yyvsp[-2].pos), (yyvsp[-1].nod) ); }
#line 2655 "y.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 683 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Continue( (yyvsp[-1].pos) ); }
#line 2661 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 684 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Break( (yyvsp[-1].pos) ); }
#line 2667 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 685 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Return( (yyvsp[-1].pos), (node_t*)0 ); }
#line 2673 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 686 "grammar.y" /* yacc.c:1646  */
    { (yyval.stmt) = new_stmt_Return( (yyvsp[-2].pos), (yyvsp[-1].nod) ); }
#line 2679 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 692 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Comma, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2685 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 699 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node((node_kind_t)(yyvsp[-1].val), (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2691 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 703 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Assign;}
#line 2697 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 704 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Mul_Assign;}
#line 2703 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 705 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Div_Assign;}
#line 2709 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 706 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Mod_Assign;}
#line 2715 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 707 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Add_Assign;}
#line 2721 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 708 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Sub_Assign;}
#line 2727 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 709 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Shl_Assign;}
#line 2733 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 710 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Shr_Assign;}
#line 2739 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 711 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Band_Assign;}
#line 2745 "y.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 712 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Xor_Assign;}
#line 2751 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 713 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Bor_Assign;}
#line 2757 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 720 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Cond, (yyvsp[-4].nod), (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2763 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 731 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Lor, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2769 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 736 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Land, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2775 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 741 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Bor, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2781 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 746 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Xor, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2787 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 751 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Band, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2793 "y.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 756 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Eq, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2799 "y.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 757 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Ne, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2805 "y.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 762 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Lt, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2811 "y.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 763 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Gt, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2817 "y.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 764 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Le, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2823 "y.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 765 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Ge, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2829 "y.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 770 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Shl, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2835 "y.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 771 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Shr, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2841 "y.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 776 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Add, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2847 "y.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 777 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Sub, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2853 "y.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 782 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Mul, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2859 "y.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 783 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Div, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2865 "y.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 784 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Rem, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2871 "y.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 789 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Type_Cast, new_node(_Type, (yyvsp[-2].typ)), (yyvsp[0].nod));}
#line 2877 "y.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 795 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Pre_Inc, (yyvsp[0].nod));}
#line 2883 "y.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 796 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Pre_Dec, (yyvsp[0].nod));}
#line 2889 "y.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 797 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node((yyvsp[-1].val), (yyvsp[0].nod));}
#line 2895 "y.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 798 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Sizeof, (yyvsp[0].nod));}
#line 2901 "y.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 799 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Sizeof, new_node(_Type, (yyvsp[-1].typ)));}
#line 2907 "y.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 804 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Addrof;}
#line 2913 "y.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 805 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Indirect;}
#line 2919 "y.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 806 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Unary_Plus;}
#line 2925 "y.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 807 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Unary_Minus;}
#line 2931 "y.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 808 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Ones_Complement;}
#line 2937 "y.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 809 "grammar.y" /* yacc.c:1646  */
    {(yyval.val) = _Not;}
#line 2943 "y.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 817 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Array_Index, (yyvsp[-3].nod), (yyvsp[-1].nod));}
#line 2949 "y.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 820 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-2].nod), 0);}
#line 2955 "y.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 823 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Func_Call, (yyvsp[-3].nod), reshape_list((yyvsp[-1].nod)));}
#line 2961 "y.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 826 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Dot_Selected, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2967 "y.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 829 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Arrow_Selected, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 2973 "y.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 832 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Post_Inc, (yyvsp[-1].nod));}
#line 2979 "y.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 835 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_Post_Dec, (yyvsp[-1].nod));}
#line 2985 "y.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 839 "grammar.y" /* yacc.c:1646  */
    { (yyval.nod) = bind_to_sym( (yyvsp[0].nod) ); }
#line 2991 "y.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 842 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = (yyvsp[-1].nod);}
#line 2997 "y.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 847 "grammar.y" /* yacc.c:1646  */
    {(yyval.nod) = new_node(_List, (yyvsp[-2].nod), (yyvsp[0].nod));}
#line 3003 "y.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 861 "grammar.y" /* yacc.c:1646  */
    { (yyval.pos) = yypos; }
#line 3009 "y.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 868 "grammar.y" /* yacc.c:1646  */
    { (yyval.com) = fetch_comment_block(); }
#line 3015 "y.tab.c" /* yacc.c:1646  */
    break;


#line 3019 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 873 "grammar.y" /* yacc.c:1906  */



