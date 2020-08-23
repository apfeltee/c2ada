
#pragma once
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "hostinfo.h"

#define NODE_FNAME(n) file_name(n->node_def)
#define NODE_FLINE(n) line_number(n->node_def)
#ifdef NO_DEALLOC
    #undef free
    #define free(x)
#endif
#define NEW(type) ((type*)allocate(sizeof(type)))
#define FALSE 0
#define TRUE 1
#ifndef MAX_OUTBUF_LEN
    #define MAX_OUTBUF_LEN 1024
#endif
#define COMMENT_BLOCKSIZE 10
#define EVAL_FAILED(x) ((x).eval_result_kind == eval_failed)
#define IS_EVAL_INT(x) ((x).eval_result_kind == eval_int)
#define IS_EVAL_FLOAT(x) ((x).eval_result_kind == eval_float)
#define IS_EVAL_STRING(x) ((x).eval_result_kind == eval_string)
#define EVAL_INT(x) ((x).eval_result.ival)
#define EVAL_FLOAT(x) ((x).eval_result.fval)
#define EVAL_STRING(x) ((x).eval_result.sval)
#define BUILTIN_FILE -2
#define BUILTIN_LINE -3
#define BAD_INPUT 3
#define PUNCT 0
#define PARAM_START 1
#define END_INPUT 2
#define ALPHA 4
#define DIGIT 8
#define XDIGIT 16
#define WHITE 32
#define END_OF_LINE 64
#define MSTART 128
#define is_eof(c) ((cpp_char_class[(int)(c)] & END_INPUT) != 0)
#define is_eol(c) ((cpp_char_class[(int)(c)] & END_OF_LINE) != 0)
#define is_hex_digit(c) ((cpp_char_class[(int)(c)] & XDIGIT) != 0)
#define is_octal_digit(c) ((c) >= '0' && (c) <= '7')
#define is_digit(c) ((cpp_char_class[(int)(c)] & DIGIT) != 0)
#define is_white(c) ((cpp_char_class[(int)(c)] & WHITE) != 0)
#define is_punct(c) (cpp_char_class[(int)(c)] == PUNCT)
#define is_alpha(c) ((cpp_char_class[(int)(c)] & ALPHA) != 0)
#define is_alpha_numeric(c) ((cpp_char_class[(int)(c)] & (ALPHA | DIGIT)) != 0)
#define classof(c) (cpp_char_class[(int)(c)])
#define int_modifier(c) ((c) == 'l' || (c) == 'L' || (c) == 'u' || (c) == 'U')
#define float_modifier(c) ((c) == 'F' || (c) == 'f' || (c) == 'D' || (c) == 'd')
#define is_magnitude(c) ((c) == 'E' || (c) == 'e')
#ifndef FILE_ORD_BITS
    #define FILE_ORD_BITS 10
#endif
#if !defined(BITS_PER_BYTE)
    #define BITS_PER_BYTE 8
#endif
#if !defined(SIZEOF_LONG)
    #define SIZEOF_LONG 4
#endif
#define LINE_NUMBER_BITS ((SIZEOF_LONG * BITS_PER_BYTE) - FILE_ORD_BITS)
#define MAX_UNIQ_FNAMES (1 << FILE_ORD_BITS)
#define line_number(x) ((x) & ((1L << LINE_NUMBER_BITS) - 1))
#undef START_INDENT
#define START_INDENT 0
#ifndef COMMENT_POS
    #define COMMENT_POS 64
#endif
#define MAX_INDENT(x)          \
    {                          \
        int _i = cur_indent(); \
        if(_i > (x))           \
            (x) = _i;          \
    }

#ifndef HASH_PRIME
    #define HASH_PRIME 31741
#endif
#ifdef aix
    #define C_VAR_PREFIX "."
#else
    #define C_VAR_PREFIX "_"
#endif
#ifdef aix
    #define C_SUBP_PREFIX "."
#else
    #define C_SUBP_PREFIX "_"
#endif
#define SIZEOF_ENUM SIZEOF_INT
#define ALIGNOF_ENUM ALIGNOF_INT
#define ENUM_PREFIX '1'
#define STRUCT_PREFIX '2'
#define UNION_PREFIX '3'
#define TYPEMOD_EXTERN 0x0001
#define TYPEMOD_AUTO 0x0002
#define TYPEMOD_REGISTER 0x0004
#define TYPEMOD_SHORT 0x0008
#define TYPEMOD_STATIC 0x0010
#define TYPEMOD_SIGNED 0x0020
#define TYPEMOD_UNSIGNED 0x0040
#define TYPEMOD_VOLATILE 0x0080
#define TYPEMOD_TYPEDEF 0x0100
#define TYPEMOD_CONST 0x0200
#define TYPEMOD_LONG 0x1000
#define TYPEMOD_INLINE 0x2000
#define unimplemented() not_implemented(__FILE__, __LINE__)


typedef unsigned long file_pos_t;
typedef int file_id_t;
typedef unsigned long line_nt;
typedef long cpp_int_t;

typedef int unit_n; /* (type of) unique output Ada PACKAGE number */
/*
 * Internal types for all integer and floating point constants.
 * If you decide to change these types you better check the
 * print_value() and print_fp_value() routines in gen.c.  They
 * will most likely have to be updated.
 */
typedef unsigned long hash_t;
typedef long int host_int_t;
typedef double host_float_t;

struct cpp_file_t;
struct node_t;
struct symbol_t;
struct typeinfo_t;
typedef struct symbol_t symbol_t;
typedef struct cpp_file_t cpp_file_t, *cpp_file_pt;
typedef struct node_t* node_pt;
typedef struct typeinfo_t* typeinfo_pt;
typedef struct pkg_def_t* pkg_def_pt;
typedef struct comment_block* comment_block_pt;
typedef struct stmt_t* stmt_pt;



typedef enum
{
    unspecified_vendor,
    Rational,
    VADS,
    ICC, /* Irvine Compiler */
    GNAT
} vendor_t;





/*
 * Scopes
 */
typedef short int scope_id_t;

/*
 * Possible symbol declarations
 */
typedef enum
{
    type_symbol = 256,
    func_symbol,
    param_symbol,
    var_symbol,
    enum_literal,
    pkg_symbol /* designates an Ada output package */
} sym_kind_t;

typedef enum
{
    eval_failed,
    eval_int,
    eval_float,
    eval_string,
    eval_type
} cpp_eval_result_kind_t;

enum
{
    Define = 1,
    Elif,
    Else,
    Endif,
    Error,
    Ident,
    If,
    Ifdef,
    Ifndef,
    Include,
    Line,
    Pragma,
    Undef
};

typedef enum
{
    Report_fatal,
    Report_error,
    Report_warning,
    Report_inform
} report_t;


typedef enum
{
    _Labelled,
    _Case,
    _Default,
    _Compound,
    _SList,
    _Expr,
    _If,
    _Ifelse,
    _Switch,
    _While,
    _Do,
    _For,
    _Goto,
    _Continue,
    _Break,
    _Return,
    _Null,
    _FuncDef,
    _MacroBody
} stmt_kind_t;

typedef enum
{
    Unspecified_scope,
    Func_scope,
    File_scope,
    Block_scope,
    Proto_scope
} scope_kind_t;


/*
 * Possible declaration types
 */
typedef enum
{
    pointer_to = 128,
    array_of,
    struct_of,
    union_of,
    field_type,
    int_type,
    float_type,
    void_type,
    function_type,
    enum_type,
    typemodifier
} typekind_t;


typedef enum
{
    Upper,
    Lower,
    Cap
} ident_case_t;

typedef enum
{
    scan_file,
    scan_macro_expansion,
    scan_text
} scan_kind_t;


/*
 * Our enums ordinals don't overlap nor do
 * they start with zero so that it's easy
 * to trap pointers to trash when debugging
 */
typedef enum
{
    pointer_decl = 1,
    int_decl,
    fp_decl,
    field_decl,
    func_decl,
    /* TBD: add separate enum for func_def ? */
    enum_decl,
    array_decl,
    struct_decl
} decl_class_t;

/*
 * expression kinds
 */
typedef enum
{
    _Error = 32,
    _Ellipsis,
    _FP_Number, /* floating  literal */
    _Int_Number, /* integral  literal */
    _Char_Lit, /* character literal */
    _Type,
    _Sym,
    _Ident,
    _Macro_ID,
    _String,
    _List, /* first binary */
    _Comma,
    _Bit_Field,
    _Dot_Selected,
    _Arrow_Selected,
    _Array_Index,
    _Func_Call,
    _Type_Cast,
    _Assign,
    _Mul_Assign,
    _Div_Assign,
    _Mod_Assign,
    _Add_Assign,
    _Sub_Assign,
    _Shl_Assign,
    _Shr_Assign,
    _Band_Assign,
    _Xor_Assign,
    _Bor_Assign,
    _Eq,
    _Ne,
    _Lt,
    _Le,
    _Gt,
    _Ge,
    _Land,
    _Lor,
    _Band,
    _Bor,
    _Xor,
    _Add,
    _Sub,
    _Mul,
    _Div,
    _Rem,
    _Shl,
    _Shr, /* last binary */
    _Exp, /* exponentiation (**), Ada only, binary */
    _Concat, /* concatenation  (&),  Ada only, binary */
    _Sizeof, /* first unary */
    _Pre_Inc,
    _Pre_Dec,
    _Post_Inc,
    _Post_Dec,
    _Addrof,
    _Unary_Plus,
    _Unary_Minus,
    _Ones_Complement,
    _Not,
    _Aggregate,
    _Indirect, /* last unary */
    _BoolTyp, /* cvt to Boolean, Ada only, unary */
    _UnBool, /* cvt bool->int,  Ada only, unary */
    _Char_to_Int,
    _Int_to_Char,
    _Cond
} node_kind_t;



/*
 * If a macro is a function, split up the right-hand-side
 * into function name, # parameters, and parameter vector
 */
typedef struct macro_function_t
{
    char* mf_fname; /* function name */
    int mf_nparams; /* number of parameters */
    char** mf_params; /* list of parameters */
    char* mf_coercion; /* type coercion, if any */
    int mf_is_pointer; /* is it a pointer-to-function? */
    char* mf_rhs; /* right-hand-side of definition */
} macro_function_t;

typedef struct
{
    cpp_eval_result_kind_t eval_result_kind;
    union
    {
        cpp_int_t ival;
        double fval;
        char* sval;
        typeinfo_pt tval;
    } eval_result;
    int base; /* 10, 8, or 16 */
    typeinfo_pt explicit_type;
} cpp_eval_result_t;



typedef struct macro_t
{
    char* macro_name;
    char* macro_ada_name;
    char* macro_body;
    int macro_body_len;
    int macro_params;
    char** macro_param_vec;
    file_pos_t macro_definition;
    struct macro_t* macro_next;
    hash_t macro_hash;
    struct macro_t* macro_hash_link;
    macro_function_t* macro_func;
    struct comment_block* comment; /* preceding block comment */
    char* eol_comment; /* comment on end-of-line  */
    cpp_eval_result_t const_value;
    bool macro_declared_in_header : 1;
    bool macro_gened : 1;
    bool macro_valid : 1;
    bool macro_eval_tried : 1;
    bool macro_evald : 1;
} macro_t;
typedef struct node_t
{
    node_kind_t node_kind;
    file_pos_t node_def;
    union
    { /* node.     */
        struct
        {
            struct node_t* boolval;
            struct node_t* tru;
            struct node_t* fals;
        } cond;
        struct
        {
            char* form;
            int len;
        } str;
        struct
        {
            struct node_t *l, *r;
        } binary;
        struct
        { /* node.id. */
            char* name;
            char* cmnt;
            symbol_t* sym;
        } id;
        struct macro_t* macro;
        struct node_t* unary;
        struct symbol_t* sym;
        struct typeinfo_t* typ;
        host_int_t ival;
        host_float_t fval;
    } node;
    short int baseval; /* for integers */
    unsigned int fixed : 1; /* does not need processing by
           fix_expr */
    unsigned int no_nul : 1; /* for _String: don't append Nul */
    unsigned int char_lit : 1; /* for _Int_Number: is a char literal*/
    struct typeinfo_t* type;
    comment_block_pt comment;
} node_t;

/*
 * Macros to get the file name and line number
 * info from the original C source
 */



typedef struct typeinfo_t
{
    typekind_t type_kind;

    unsigned int is_unsigned : 1;
    unsigned int is_signed : 1;
    unsigned int is_short : 1;
    unsigned int is_long : 1;
    unsigned int is_long_long : 1;
    unsigned int is_volatile : 1;
    unsigned int is_constant : 1;
    unsigned int is_extern : 1;
    unsigned int is_static : 1;
    unsigned int is_auto : 1;
    unsigned int is_register : 1;
    unsigned int is_typedef : 1;
    unsigned int is_builtin : 1; /* An intrinsic type */
    unsigned int is_anonymous : 1;
    unsigned int is_anon_int : 1; /* no type changed to int */
    unsigned int is_inline : 1; /* inline function */
    unsigned int is_boolean : 1; /* special marker for Ada-output bool type */

    /* type_info. */
    union
    {
        /*     array. */
        struct
        {
            int elements;
            node_pt size_expr;
        } array;
        typeinfo_pt struct_fields;
        symbol_t* formals;
    } type_info;

    unsigned int type_sizeof;
    unsigned int type_alignof;
    unsigned int type_hash; /* Comparison heuristic */
    symbol_t* type_base;
    typeinfo_pt type_anonymous_list;
    typeinfo_pt type_next;
} typeinfo_t;


typedef struct symbol_t
{
    sym_kind_t sym_kind;
    scope_id_t sym_scope_id;
    unsigned char sym_scope;

    unsigned int intrinsic : 1; /* intrinsic/builtin symbol */

    unsigned int is_volatile : 1;
    unsigned int is_const : 1;
    unsigned int is_inline : 1; /* inline function */
    unsigned int is_static : 1; /* declared "static" */

    unsigned int is_created_name : 1; /* has synthesized name */
    unsigned int is_created_by_reference : 1;
    unsigned int is_declared_in_header : 1;
    unsigned int is_struct_or_union_member : 1;

    unsigned int has_initializer : 1;

    unsigned int emit_initializer : 1;
    /* on a local var,
     * forces initializer rather
     * than separate assignment statement
     */

    unsigned int gened : 1; /* Has been passed to gen routines */
    unsigned int cleared : 1; /* Symbol can be queued */
    unsigned int stored : 1; /* Has been added to symbol table */
    unsigned int interfaced : 1; /* interface done */
    unsigned int emitted : 1; /* Ada has been output for this sym*/

    unsigned int renames : 1; /* sym renames a designator expr */
    unsigned int aliases : 1; /* This sym is equivalent to another
                               * sym, namely this->sym_value->
                               *  aliased_sym.
                               */
    unsigned int has_return : 1; /* body for function sym contains
                                  * a return statement.
                                  */
    unsigned int private : 1; /* configured as private */
    unsigned int declare_in_spec : 1;
    /* configured to ensure sym
     * has a declaration in the Ada spec
     */

    int traversal_unit; /* heuristic to eliminate
                   redundant unit traversals */

    node_pt sym_ident; /* C name - node saved for comments */
    char* sym_ada_name; /* Generated Ada name */
    file_pos_t sym_def; /* File,line definition */
    typeinfo_pt sym_type; /* Symbols type info */
    int bitoffset; /* For fields only */

    union
    { /* sym_value. */
        node_pt initializer;
        host_int_t intval;
        struct stmt_t* body; /* "value" of func def is
                its body */
        pkg_def_pt pkg_def;
        symbol_t* aliased_sym; /* If this->aliases is set,
                                * then this sym is equivalent
                                * to this->aliased_sym.
                                */

    } sym_value;

    comment_block_pt comment; /* block comment */

    symbol_t* sym_tags; /* List of struct/union fields
                   or enum literals */

    struct symbol_t* sym_parse_list; /* List used by front end */
    struct symbol_t* sym_scope_list; /* List used by front end */
    struct symbol_t* sym_gen_list; /* List used by back end */

    /* hash table info */
    hash_t sym_hash; /* Comparison heuristic */
    struct symbol_t* sym_hash_list;
} symbol_t;

typedef struct
{
    macro_t* expand_macro;
    char** expand_actuals;
    int expand_nactuals;
} macro_expansion_t;


typedef struct scan_position_t
{
    scan_kind_t scan_kind;
    union
    {
        macro_expansion_t expansion;
        cpp_file_pt file;
        char* text;
    } scan;
    file_pos_t scan_pos;
    int scan_index;
    struct scan_position_t* scan_next;
} scan_position_t;




typedef struct
{
    int skip_else;
    int cur_scope;
    int gen_scope;
    int _parsing;
    file_pos_t position;
} cpp_control_state_t;



typedef struct buffer_t
{
    char buf[MAX_OUTBUF_LEN];
    unsigned short head, tail;
    struct buffer_t *next, *last;
} buffer_t;

struct comment_block
{
    struct comment_block* next;
    int count;
    char* line[COMMENT_BLOCKSIZE];
};


typedef struct
{
    node_pt* head;
    node_pt* rest;
    node_pt* tail; /* head + rest node */
} node_iter_t;


/* A statement is represented by a stmt_t structure. */

typedef struct stmt_t
{
    file_pos_t stmt_def;
    stmt_kind_t stmt_kind;
    comment_block_pt comment;
    scope_id_t scope;

    union /* (stmt_kind) */
    {
        /* case _Labelled, _Case: */
        struct
        {
            node_pt id;
            stmt_pt stmt;
        } label;

        /* case _Default: */
        stmt_pt default_stmt;

        /* case _SList: */
        struct
        {
            stmt_pt first; /* a single statement */
            stmt_pt rest; /* a statement list, or null */
        } stmt_list;

        /* case _Compound */
        struct
        {
            symbol_t* decls;
            stmt_pt stmts;
        } compound;

        /* case _FuncDef */
        struct
        {
            symbol_t* decl;
            stmt_pt body;
        } funcdef;

        /* case _Expr: */
        node_pt expr;

        /* case _If, _Switch, _While, _Do: */
        struct
        {
            node_pt expr;
            stmt_pt stmt;
        } controlled;

        /* case _Ifelse */
        struct
        {
            node_pt expr;
            stmt_pt then_stmt, else_stmt;
        } if_else_stmt;

        /* case _For: */
        struct
        {
            node_pt e1, e2, e3;
            stmt_pt stmt;
        } for_stmt;

        /* case _Goto: */
        node_pt goto_label;

        /* case _Return: */
        node_pt return_value;

        /* case _MacroBody: */
        char* macro_body;

        /* case _Null, _Continue, _Break, - nothing */

    } stmt;
} stmt_t;

typedef struct case_alist
{ /* list of alternatives */
    node_pt exp; /* case 1:   case 2:   ... */
    struct case_alist* rest;
} case_alist, *case_alist_pt;

typedef struct case_slist
{ /* list of statements after an alternative */
    stmt_pt stm;
    struct case_slist* rest;
} case_slist, *case_slist_pt;

typedef struct case_blist
{ /* list of blocks in case */
    case_alist_pt alts; /* a block is an alist followed by an slist */
    case_slist_pt stms;
    struct case_blist* rest;
    int has_default;
    stmt_pt last_stmt;
} case_blist, *case_blist_pt;

typedef struct case_stmt
{ /* C switch stmt, Ada case stmt */
    node_pt exp;
    case_blist_pt branches;
    case_blist_pt default_branch;
} case_stmt, *case_stmt_pt;

typedef void (*gen_unchecked_conversion_func_pt)(symbol_t* sym,
                                                 typeinfo_pt from_type,
                                                 typeinfo_pt to_type);

typedef void (*gen_stdarg_concat_func_pt)(typeinfo_pt type);


typedef void (*gen_use_type_decl_pt)(typeinfo_pt type);



bool is_ada_keyword(char*);
void make_ada_identifier(char*, char*);
char* uniq_name(char*, int);
char* ada_name(char*, int);
ident_case_t id_case(char*);
void id_format(char*, ident_case_t);

/* return a pointer to the final component name in an Ada name */
char* tail(char* component);



bool same_ada_type(typeinfo_pt t1, typeinfo_pt t2);



char* new_string(char*);
char* new_strf(char*, ...); /* uses printf style formatting */
void* allocate(size_t size);
void deallocate(void* ptr);



void init_anonymous_types();
symbol_t* get_anonymous_type(typeinfo_pt);
int next_anonymous_ord();

typeinfo_pt find_anonymous_type(typeinfo_pt);

extern symbol_t* anonymous_function_pointer;

void store_anonymous_type(typeinfo_t* typ);

char* predef_name(char* s);
char* predef_name_copy(char* s);

/* $Source: /home/CVSROOT/c2ada/aux_decls.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */



symbol_t* unchecked_conversion_func(typeinfo_pt from_type,
                                    typeinfo_pt to_type,
                                    file_pos_t pos,
                                    bool in_spec);


void gen_unchecked_conversion_funcs(unit_n unit, gen_unchecked_conversion_func_pt f);

/* Stdarg usage */
node_pt stdarg_empty_node(file_pos_t pos);

void use_stdarg_concat(unit_n unit, typeinfo_pt type);

void gen_stdarg_concat_funcs(unit_n unit, gen_stdarg_concat_func_pt f);

void use_type(unit_n unit, typeinfo_pt type);

void gen_use_type_decls(unit_n unit, gen_use_type_decl_pt f);





/* It goes against the grain to say this in C, but ... */


/*
 * MAX_OUTBUF_LEN must be a power of 2 and it should
 * be large enough that overflows are uncommon, but
 * we use many automatic instances of the queues so
 * it shouldn't be so large that our stack grows
 * like Jack's beanstock.
 */


int buf_empty(buffer_t*);
int buf_count(buffer_t*);
void buf_add(buffer_t*, int);
void buf_add_str(buffer_t*, char*);
char buf_get(buffer_t*);
void buf_init(buffer_t*);
void buf_destroy(buffer_t*);
void buf_concat(buffer_t*, buffer_t*);
void buf_move_to(buffer_t*, char*);
char* buf_get_str(buffer_t*);





void add_comment_line(comment_block_pt block, char* line);

comment_block_pt new_comment_block();

void put_comment_block(comment_block_pt block, int indent);



void configure_project(char* filename);

char** configured_reserved_ids(int* count_p);

extern bool configured;

bool configured_source_flag(char* source, char* attribute, bool default_result);

bool configured_sym_info(symbol_t* sym, typeinfo_pt type);

void set_output_dir(char* pathname);

char* configured_output_dir();

char* configured_source_partner(char* fname);

char* configured_macro_replacement(file_id_t file,
                                          char* macro_name,
                                          char* macro_body,
                                          int body_len,
                                          int nformals,
                                          char** formals,
                                          char* eol_comment);

typedef struct context_t ctxt_t, *ctxt_pt;

ctxt_pt new_context(scope_id_t scope);
ctxt_pt copy_context(ctxt_pt);
void free_context(ctxt_pt);

bool changed_pre(ctxt_pt ctxt);
bool changed_post(ctxt_pt ctxt);
bool added_decls(ctxt_pt ctxt);
void reset_changed(ctxt_pt ctxt);
void clear_pre_and_post(ctxt_pt ctxt);

stmt_pt pre_stmts(ctxt_pt ctxt);
void set_pre(ctxt_pt ctxt, stmt_pt stmts);
void append_pre(ctxt_pt ctxt, stmt_pt stmt);

stmt_pt post_stmts(ctxt_pt ctxt);
void set_post(ctxt_pt ctxt, stmt_pt stmts);
void append_post(ctxt_pt ctxt, stmt_pt stmt);

symbol_t* decls(ctxt_pt ctxt, int scope_id);
void set_decls(ctxt_pt ctxt, symbol_t* decls); /* TBD: how linked? */
void append_decl(ctxt_pt ctxt, symbol_t* decl);
void append_decls(ctxt_pt ctxt, symbol_t* decls);

scope_id_t ctxt_scope(ctxt_pt ctxt);



int cpp_open(char* path);
void cpp_cleanup();
int cpp_getc();
void cpp_search_path(char*);
void cpp_system_search_path(char*);
int in_system_search_path(char*);
void cpp_show_predefines();

extern bool at_file_start;


cpp_eval_result_t cpp_eval(char*);





/*
 * The builtin cpp macros will be added to the hash table as normal
 * macros.  They are identified by their num_params field which will
 * have one of the following builtin values.
 */



/*
 * Character classes for the preprocessor
 */





extern unsigned char cpp_char_class[];
extern macro_t* macro_list_head;

int cpp_getc_from(buffer_t*);
void cpp_set_state(scan_position_t*, cpp_control_state_t*, scan_position_t**, cpp_control_state_t*);



void vreport(report_t severity, char* filename, int linenum, char* format, va_list ap);

void fatal(char*, int, char*, ...);
void error(char*, int, char*, ...);
void warning(char*, int, char*, ...);
void inform(char*, int, char*, ...);

void assert_failed(char*, int, char*);
void syserr(char*, int);

/*
 * Routines for dealing with files and file names.
 *
 * The current file and line number during scanning is
 * maintained int packed long int.  This will reduce the size
 * of a lot of our IL types.
 */






/*
 * Upper FILE_ORD_BITS specify file ordinal.
 * Rest of integer specifies line number.
 */


file_id_t pos_file(file_pos_t);
line_nt pos_line(file_pos_t);


int num_files();

char* file_name(file_pos_t pos);
char* file_name_from_ord(int ord);
file_pos_t add_file(char* path);
file_pos_t set_file_pos(char* path, int line);

/* returns -1 if file not found */
file_pos_t find_file(char* path);

size_t sizeof_file(int fd);
void* map_file(int fd, size_t fsize);
int unmap_file(void* addr, size_t len);
int compare_path(char* s1, char* s2);

/* convenience functions to use file_pos_t for warnings */
void error_at(file_pos_t pos, char* fmt, ...);
void warning_at(file_pos_t pos, char* fmt, ...);
void inform_at(file_pos_t pos, char* fmt, ...);

void fix_func_body(symbol_t* func);
node_pt fix_initializer_expr(node_pt init, typeinfo_pt type);




int output_line();
void reset_output_line();

void reset_indent();
void new_line();
void indent_to(int);
void put_string(char*);
void put_char(int);
int cur_indent();

void putf(char*, ...);


void gen_ada_type(symbol_t* sym);
void gen_ada_func(symbol_t* sym, symbol_t* ante);
void gen_ada_var(symbol_t* sym);
void gen_ada_lit(symbol_t* sym);

void gen_any_func_ptr_type(symbol_t* sym);

char* int_type_builtin_name(typeinfo_pt typ);
char* type_nameof(typeinfo_pt, int use_parent_type, int is_param);

void gen_var_or_field(symbol_t* sym, int tabpos, int colonpos, int import, char* rename, int hidden);

void print_value(host_int_t val, int base);
void print_fp_value(host_float_t val);
void print_value(host_int_t val, int base);
void print_position(file_pos_t pos);

char* string_name(bool is_wide);
void gen_char_array(char* name, char* val, bool is_wide_string, bool is_const);

int should_import();
void print_string_value(char* val, int expected_len, bool c_string);
void print_char_value(int val);

void subtype_decl(char* subtype_name,
                         char* package_name,
                         char* type_name,
                         int indent,
                         node_t* ident,
                         file_pos_t pos);

void gen_local_func(symbol_t* sym, int indent);

void gen_subp(symbol_t* sym, char* rename, bool is_spec, bool is_inline, bool is_interfaced);

void print_comment(char*);
void c_comment(node_t* n);
bool is_function(symbol_t* subp);

extern struct typeinfo_t* bogus_type;

void gen_access_type(symbol_t* sym, bool private_part);
typeinfo_t* return_type(symbol_t* subp);

extern int max_const_name_indent;
char* char_to_string(int c, bool c_string);
void interface_c(symbol_t* sym, int indent);

void init_predef_names();
char* c_array_index_name();

/* $Source: /home/CVSROOT/c2ada/gen_expr.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */


void gen_expr(node_pt e, bool in_parens);

char* null_pointer_value_name(typeinfo_pt type);


void gen_macro_names();
void import_macro_constants();
void gen_macro_constants(macro_t* m, int import);
void gen_macro_types(macro_t* m, int import);
void gen_macro_funcs(macro_t* m, int import);
void gen_macro_vars(macro_t* m, int import, int colonpos);
void finish_macros(macro_t* m);
void rethread_macros();
void gen_macro_warnings();

/* $Source: /home/CVSROOT/c2ada/gen_stmt.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */


void gen_stmt(stmt_pt s, int ident);




hash_t common_hash(char*);
hash_t lcase_hash(char*);
int lcase(int);
int lcasecmp(char*, char*);



/*
 * External var symbol name prefix by C compiler for this host OS.
 */


/*
 * External function symbol name prefix by C compiler for this host OS.
 */


/*
 * Assume enum == int.  If this is not true you should
 * ifdef for your system and define it as in hostinfo.h.
 */

/* This file is automatically generated */







struct stmt_t;

/*
 * Symbol names are prefixed by the following for enums, structs and unions.
 * NB: These values must be the same as used in prefix_dict in
 * C2ada.py.
 */

/*
 * Type Modifiers
 */



/* $Source: /home/CVSROOT/c2ada/initializer.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

node_pt gen_initializer(typeinfo_pt t, node_pt e);
void gen_zero(typeinfo_pt t);
void fix_sym_initializer(symbol_t* sym);
/* $Source: /home/CVSROOT/c2ada/localfunc.h,v $ */
/* $Revision: 1.1.1.1 $   $Date: 1999/02/02 12:01:51 $ */


stmt_pt return_bool_stmt(bool value, file_pos_t pos);

symbol_t* new_local_func(typeinfo_pt return_type, symbol_t* decls, stmt_pt stmts, file_pos_t pos, scope_id_t scope);





extern macro_t* unit_macros[];

void macro_init(int);
void macro_undef(char*);
void macro_def(char*, char*, int, int, char**, file_pos_t, char*);
macro_t* macro_find(char*);


node_t* access_to(node_t*, node_t*);

node_pt new_node(node_kind_t, ...);
node_pt new_pos_node(file_pos_t pos, node_kind_t kind, ...);

node_t* id_from_typedef(typeinfo_t*);
void reduce_node(node_t*);
void free_node(node_t*);

/* returns the op corresponding to an assign-op; eg {*=} => {*} */
node_kind_t non_assign_op(node_kind_t);

node_pt reshape_list(node_pt e);

/* for iterating over a list */



node_iter_t init_node_iter(node_pt* n);
node_pt* next_list_ref(node_iter_t* iter);
node_pt* node_iter_tail(node_iter_t* iter);

/* null pointer */
bool is_null_ptr_value(node_pt node);

/* $Source: /home/CVSROOT/c2ada/order.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

bool has_undone_requisites(symbol_t* sym);

void postpone_doing(symbol_t* sym);

void set_symbol_done(symbol_t* sym);

bool sym_done(symbol_t* sym);
/* $Source: /home/CVSROOT/c2ada/package.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */


typeinfo_pt ptrs_type_for(typeinfo_pt ptr_type, ctxt_pt ctxt, file_pos_t pos);

void gen_pkg_def(symbol_t* sym, int indent);

void gen_unit_pkg_defs(int unit, int indent);

char* generic_ptrs_pkg_name(bool const_ptr);


char* nameof_stmt_kind(stmt_kind_t);
char* nameof_node_kind(node_kind_t);
char* nameof_typekind(typekind_t);
char* nameof_sym_kind(sym_kind_t);

void print_stmt_kind(stmt_kind_t s);
void print_stmt(stmt_pt s, int indent);
void print_node(node_pt n, int indent);
void print_symbol(symbol_t* s, int indent);
void print_typekind(typekind_t t);
void print_node_kind(node_kind_t n);
void print_typeinfo(typeinfo_t* t, int indent);
void print_sym_kind(sym_kind_t s);
void print_case_alist(case_alist_pt ap, int indent);
void print_case_slist(case_slist_pt sp, int indent);
void print_case_blist(case_blist_pt bp, int indent);
void print_case_stmt(case_stmt_pt cp, int indent);
void print_macro(macro_t* m, int indent);
void print_macro_function(macro_function_t* f, int indent);
void print_file_pos(file_pos_t pos);
void print_comment_block(struct comment_block* bl, int indent);
void print_cpp_eval_result_t(cpp_eval_result_t* res, int indent);


symbol_t* new_sym();
symbol_t* find_sym(char*);
void store_sym(symbol_t*);

/* Scopes */




scope_kind_t scope_kind(scope_id_t);
scope_id_t scope_parent(scope_id_t);
symbol_t* scope_symbol(scope_id_t);
symbol_t* scope_parent_func(scope_id_t);
int scope_level(scope_id_t);

void set_scope_kind(scope_id_t scope, scope_kind_t kind);
void set_scope_parent(scope_id_t scope, scope_id_t parent);
void set_scope_symbol(scope_id_t scope, symbol_t* sym);

void scope_push(scope_kind_t kind);
void scope_pop();

int next_param();

scope_id_t new_scope_id(scope_kind_t kind);
scope_id_t new_block_scope(scope_id_t parent);

scope_id_t current_scope();
void set_current_scope(scope_id_t);


/* Interface for generating a new statement. */

stmt_pt new_stmt_Labelled(node_pt label, comment_block_pt com, stmt_pt stmt);
stmt_pt new_stmt_Case(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt stmt);
stmt_pt new_stmt_Default(file_pos_t pos, comment_block_pt com, stmt_pt stmt);
stmt_pt new_stmt_Null(file_pos_t pos);
stmt_pt new_stmt_Expr(node_pt expr);
stmt_pt new_stmt_Compound(file_pos_t pos, symbol_t* decls, stmt_pt stmts);
stmt_pt new_stmt_If(file_pos_t pos, comment_block_pt com, node_pt cond, stmt_pt stmt);
stmt_pt new_stmt_Ifelse(file_pos_t pos, comment_block_pt com, node_pt cond, stmt_pt then_stmt, stmt_pt else_stmt);
stmt_pt new_stmt_Switch(file_pos_t pos, comment_block_pt com, node_pt cond, stmt_pt stmt);
stmt_pt new_stmt_While(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt stmt);
stmt_pt new_stmt_Do(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt stmt);
stmt_pt new_stmt_For(file_pos_t pos, comment_block_pt com, node_pt e1, node_pt e2, node_pt e3, stmt_pt stmt);
stmt_pt new_stmt_Goto(file_pos_t pos, node_pt label);
stmt_pt new_stmt_Continue(file_pos_t pos);
stmt_pt new_stmt_Break(file_pos_t pos);
stmt_pt new_stmt_Return(file_pos_t pos, node_pt expr);
stmt_pt new_stmt_MacroBody(file_pos_t pos, char* body);

/* A function definition comprises the header (basically a function
 * declaration) and the function body (basically a compound statement.
 * For now, a func def will be represented as a variant of a statement.
 */

symbol_t* new_func(symbol_t* decl, stmt_pt body);

void define_func(symbol_t* funcdef, comment_block_pt comment);

void set_stmts_scope(stmt_pt stmts, scope_id_t scope);

/* A statement list is implemented as a variant of stmt_t */

stmt_pt new_stmt_list(stmt_pt stmt);
stmt_pt append_stmt(stmt_pt stmts, stmt_pt stmt);
stmt_pt concat_stmts(stmt_pt s1, stmt_pt s2); /* destructive */



void gen_funcdef(symbol_t* funcdef, int indent);

/* $Source: /home/CVSROOT/c2ada/symset.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */


typedef struct symset* symbols_t;

void symset_init();

symbols_t new_symbols_set();
void symset_add(symbols_t syms, symbol_t* sym);

void symset_filter_undone(symbols_t syms);

bool symset_has(symbols_t syms, symbol_t* sym);
int symset_size(symbols_t syms);

/* Symbol abstraction */
symbols_t get_undone_requisites(symbol_t* sym);
void set_undone_requisites(symbol_t* sym, symbols_t syms);

/* symbol mapping */
typedef struct symmap* symmap_t;

symmap_t new_symmap(char* mapname);
symbol_t* get_symmap(symmap_t map, symbol_t* key);
void set_symmap(symmap_t map, symbol_t* key, symbol_t* value);



void not_implemented(char*, int);

void type_init();
bool is_typedef(symbol_t*);
bool is_access_to_record(typeinfo_t*);
bool is_enum_literal(symbol_t*);
bool is_function_pointer(typeinfo_t*);
int num_dimensions(typeinfo_t*);
int* get_dimensions(typeinfo_t*);

bool equal_types(typeinfo_pt t1, typeinfo_pt t2);
bool assignment_equal_types(typeinfo_pt tleft, typeinfo_pt tright);

typeinfo_t* abstract_declarator_type(typeinfo_pt, node_pt);
typeinfo_t* new_type(typekind_t);
typeinfo_t* copy_type(typeinfo_t*);
typeinfo_t* concat_types(typeinfo_t*, typeinfo_t*);

typeinfo_t* typeof_void();
typeinfo_t* typeof_char();
typeinfo_t* typeof_int();
typeinfo_t* typeof_float();
typeinfo_t* typeof_double();

typeinfo_t* typeof_typemod(int);
typeinfo_t* typeof_typespec(typeinfo_t*);
typeinfo_t* typeof_specifier(symbol_t*);

typeinfo_t* add_pointer_type(typeinfo_t*);
typeinfo_t* add_function_type(typeinfo_t*);
typeinfo_pt add_array_type(typeinfo_pt type, node_pt size);
typeinfo_t* pointer_to_sym(symbol_t*);

typeinfo_pt typeof_char_array();

decl_class_t decl_class(typeinfo_t*);

symbol_t* copy_sym(symbol_t*);
symbol_t* concat_symbols(symbol_t*, symbol_t*);
symbol_t* grok_enumerator(node_t*, node_t*);
symbol_t* concat_ellipsis(symbol_t*);
extern symbol_t* ellipsis_sym;

void grok_declarations(symbol_t*);
void grok_func_param_decls(symbol_t* fdecl);
symbol_t* nested_declarations(symbol_t*);

void start_typedef();
void storage_class(int);
void type_mod(int);
void type_qual(int);

symbol_t* anonymous_enum(symbol_t*);
symbol_t* named_enum(node_t*, symbol_t*);
symbol_t* enum_reference(node_t*);
symbol_t* anonymous_rec(int, symbol_t*);
symbol_t* named_rec(bool, node_t*, symbol_t*);
symbol_t* rec_reference(bool, node_t*);
symbol_t* novar_declaration(typeinfo_t*);
symbol_t* var_declaration(typeinfo_t*, node_t*);
symbol_t* field_declaration(typeinfo_t*, node_t*);
symbol_t* function_spec(typeinfo_pt t, node_pt f, int scope_level);

symbol_t* noname_simple_param(typeinfo_t*);
symbol_t* noname_abstract_param(typeinfo_t*, node_t*);
symbol_t* named_abstract_param(typeinfo_t*, node_t*);

void typed_external_decl(symbol_t*, comment_block_pt);
void function_def(symbol_t*);
void KnR_params(symbol_t*, symbol_t*);
void gen_tag_types(symbol_t*, bool);

int type_sizeof(typeinfo_t*);

node_t* bind_to_sym(node_t*);

/* Unlike the typeof_* functions, these always return the same node */

typeinfo_pt type_void();

typeinfo_pt type_signed_char();
typeinfo_pt type_unsigned_char();
typeinfo_pt type_short();
typeinfo_pt type_unsigned_short();
typeinfo_pt type_int();
typeinfo_pt type_unsigned();
typeinfo_pt type_long();
typeinfo_pt type_unsigned_long();

typeinfo_pt type_boolean();

typeinfo_pt type_float();
typeinfo_pt type_double();
typeinfo_pt type_long_double();

typeinfo_pt type_char();

typeinfo_pt type_string(); /* == type_charp */
typeinfo_pt type_charp();
typeinfo_pt type_const_charp();
typeinfo_pt type_char_array();

void all_types_gened(typeinfo_pt type, file_pos_t pos);

symbol_t* private_type_null(symbol_t* tsym);


/* Important note: The word "unit" should really be "package" in
 * these names, including "unit_n", due to a really embarassing
 * terminological lapse. There's a unique unit_n for each Ada package
 * in the output.  This terminology dates back to the "cbind" version
 * of this source, which only emitted package specs, so packages and
 * units could be conflated.  Not any more.
 */



void unit_start_gen();
bool set_unit(unit_n);
unit_n current_unit();
extern bool current_unit_is_header;
bool is_current_unit(unit_n);
void unit_completed();
void unit_included(file_pos_t, int);
void init_unit(file_pos_t);

int num_units();

bool pos_in_current_unit(file_pos_t pos);
unit_n pos_unit(file_pos_t pos);

/* unit <ord> needs a "with" for <dep>;
 * <from_body> => from <ord>'s body, else <ord>'s spec */
void unit_dependency(unit_n ord, unit_n dep, bool from_body);

char* cur_unit_source();
char* cur_unit_name();
char* cur_unit_path();

int cur_unit_has_const_string();
void set_cur_unit_has_const_string();

int cur_unit_is_child_of_predef();
void set_cur_unit_is_child_of_predef();

bool unit_has_private_part(unit_n unit);
void set_unit_has_private_part(unit_n unit);

bool cur_unit_header_comment_set();
comment_block_pt cur_unit_header_comment();
void set_cur_unit_header_comment(comment_block_pt);
comment_block_pt cur_unit_trailer_comment();
void set_cur_unit_trailer_comment(comment_block_pt);

extern char* predef_pkg;
char* unit_name(unit_n unit);

unit_n nth_ref_unit_ord(int);
unit_n nth_direct_ref_unit_ord(int);
unit_n nth_body_ref_unit_ord(int);

void set_ellipsis(unit_n unit);
bool has_ellipsis(unit_n unit);

void set_unchecked_conversion(unit_n unit, bool in_spec);
bool has_unchecked_conversion(unit_n unit);
bool unchecked_conversions_to_spec(unit_n unit);

void with_c_pointers(unit_n unit);
bool has_c_pointers(unit_n unit);

void with_c_const_pointers(unit_n unit);
bool has_c_const_pointers(unit_n unit);

void output_to_spec();
void output_to_body();
void output_to(bool to_spec);
bool output_is_spec();



extern vendor_t ada_compiler;

