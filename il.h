/*
 * Parse tree representation types
 */

#ifndef _H_IL_
#define _H_IL_

/* include definition of host_int_t */
#include "host.h"

/* include definition of hash_t */
#include "hash.h"

#include "hostinfo.h"

/* include definition of file_pos_t */
#include "files.h"

#include "comment.h"

/* Forward reference, resolved in stmt module */
struct stmt_t;

/*
 * Symbol names are prefixed by the following for enums, structs and unions.
 * NB: These values must be the same as used in prefix_dict in
 * C2ada.py.
 */
#define ENUM_PREFIX '1'
#define STRUCT_PREFIX '2'
#define UNION_PREFIX '3'

/*
 * Type Modifiers
 */
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

struct node_t;
struct symbol_t;
struct typeinfo_t;
typedef struct node_t* node_pt;
typedef struct symbol_t* symbol_pt;
typedef struct typeinfo_t* typeinfo_pt;

typedef struct pkg_def_t* pkg_def_pt;

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
            symbol_pt sym;
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
#define NODE_FNAME(n) file_name(n->node_def)
#define NODE_FLINE(n) line_number(n->node_def)

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

typedef struct typeinfo_t
{
    typekind_t type_kind;

    unsigned int _unsigned : 1;
    unsigned int _signed : 1;
    unsigned int _short : 1;
    unsigned int _long : 1;
    unsigned int _long_long : 1;
    unsigned int _volatile : 1;
    unsigned int _constant : 1;
    unsigned int _extern : 1;
    unsigned int _static : 1;
    unsigned int _auto : 1;
    unsigned int _register : 1;
    unsigned int _typedef : 1;
    unsigned int _builtin : 1; /* An intrinsic type */
    unsigned int _anonymous : 1;
    unsigned int _anon_int : 1; /* no type changed to int */
    unsigned int _inline : 1; /* inline function */
    unsigned int _boolean : 1; /* special marker for Ada-output boolean type */

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
        symbol_pt formals;
    } type_info;

    unsigned int _sizeof;
    unsigned int _alignof;
    unsigned int type_hash; /* Comparison heuristic */
    symbol_pt type_base;
    typeinfo_pt type_anonymous_list;
    typeinfo_pt type_next;
} typeinfo_t;

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

typedef struct symbol_t
{
    sym_kind_t sym_kind;
    scope_id_t sym_scope_id;
    unsigned char sym_scope;

    unsigned int intrinsic : 1; /* intrinsic/builtin symbol */

    unsigned int _volatile : 1;
    unsigned int _const : 1;
    unsigned int _inline : 1; /* inline function */
    unsigned int _static : 1; /* declared "static" */

    unsigned int _created_name : 1; /* has synthesized name */
    unsigned int _created_by_reference : 1;
    unsigned int _declared_in_header : 1;
    unsigned int _struct_or_union_member : 1;

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
        symbol_pt aliased_sym; /* If this->aliases is set,
                                * then this sym is equivalent
                                * to this->aliased_sym.
                                */

    } sym_value;

    comment_block_pt comment; /* block comment */

    symbol_pt sym_tags; /* List of struct/union fields
                   or enum literals */

    struct symbol_t* sym_parse_list; /* List used by front end */
    struct symbol_t* sym_scope_list; /* List used by front end */
    struct symbol_t* sym_gen_list; /* List used by back end */

    /* hash table info */
    hash_t sym_hash; /* Comparison heuristic */
    struct symbol_t* sym_hash_list;
} symbol_t;

#endif
