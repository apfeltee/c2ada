/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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

#ifndef YY_YY_Y_TAB_H_INCLUDED
#define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
    #define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
    #define YYTOKENTYPE
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
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
    #line 28 "grammar.y" /* yacc.c:1909  */

    int val;
    node_t* nod;
    symbol_t* sym;
    typeinfo_t* typ;
    stmt_t* stmt;
    file_pos_t pos;
    comment_block_pt com;

    #line 198 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
    #define YYSTYPE_IS_TRIVIAL 1
    #define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

int yyparse(void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
