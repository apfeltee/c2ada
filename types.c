/*
 * Most of the semantic routines are found here.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "c2ada.h"
#include "hostinfo.h"


/* Import from scanner */
extern file_pos_t yypos;
extern int auto_package;

static file_pos_t current_file_pos;
static char* current_name;

#define nyi() not_implemented(__FILE__, __LINE__)

#undef NULL
#define NULL 0L

#define MAKING_BODY (!current_unit_is_header)

#undef N_BUILTINS
#define N_BUILTINS 5

/* TBD: This is a temp flag to disable handling const macros as
 * constants until the facility is in better shape.
 * It's defined in cpp.c */
extern bool do_const_macros;

static symbol_t builtins[N_BUILTINS];
static typeinfo_t builtin_types[N_BUILTINS];

void not_implemented(file, line) char* file;
int line;
{
    extern file_pos_t yypos;
    fatal(file_name(yypos), line_number(yypos),
          "This has not yet been implemented [%s:%d]", file, line);
}

/*
 * All types get hashed to make type comparisons faster and
 * to maintain the anonymous type tables
 */
static unsigned int set_hash_for_type(typ) typeinfo_t* typ;
{
    unsigned int hash = 0;

    assert(typ != NULL);

    hash += 7 * typ->type_kind;
    /* hash += typ->type_sizeof;
       commented out so arrays of different sizes get the same hash value */
    hash += typ->type_alignof;

    if(typ->is_unsigned)
        hash += 3;
    if(typ->is_signed)
        hash += 5;

    if(typ->type_next != NULL)
    {
        hash += set_hash_for_type(typ->type_next);
    }

    typ->type_hash = hash;
    return hash;
}

/*
 * These are the builtin/intrinsic types
 */
static void init_common_types(void);

void type_init()
{
    int i;
    symbol_t* sym;
    typeinfo_t* typ;

    /* int must be 0 */
    sym = &builtins[0];
    sym->gened = 1;
    sym->intrinsic = 1;
    sym->sym_type = &builtin_types[0];
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, "int");
    sym->sym_ada_name = new_string(predef_name("int"));
    sym->sym_type->type_kind = int_type;
    sym->sym_type->type_sizeof = SIZEOF_INT;
    sym->sym_type->type_alignof = ALIGNOF_INT;
    sym->sym_type->type_base = sym;
    /*store_sym(sym);*/
    /* void must be 1 */
    sym = &builtins[1];
    sym->gened = 1;
    sym->intrinsic = 1;
    sym->sym_type = &builtin_types[1];
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, "void");
    sym->sym_ada_name = new_string(predef_name("void"));
    sym->sym_type->type_kind = void_type;
    sym->sym_type->type_sizeof = SIZEOF_INT;
    sym->sym_type->type_alignof = ALIGNOF_INT;
    sym->sym_type->type_base = sym;
    /*store_sym(sym);*/

    /* char must be 2 */
    sym = &builtins[2];
    sym->gened = 1;
    sym->intrinsic = 1;
    sym->sym_type = &builtin_types[2];
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, "char");
    sym->sym_type->type_kind = int_type;
    sym->sym_type->type_sizeof = SIZEOF_CHAR;
    sym->sym_type->type_alignof = ALIGNOF_CHAR;
    sym->sym_type->type_base = sym;
    sym->sym_ada_name = new_string(predef_name("char"));

    /*store_sym(sym);*/
    /* float must be 3 */
    sym = &builtins[3];
    sym->intrinsic = 1;
    sym->gened = 1;
    sym->sym_type = &builtin_types[3];
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, "float");
    sym->sym_ada_name = new_string(predef_name("float"));
    sym->sym_type->type_kind = float_type;
    sym->sym_type->type_sizeof = SIZEOF_FLOAT;
    sym->sym_type->type_alignof = ALIGNOF_FLOAT;
    sym->sym_type->type_base = sym;
    /*store_sym(sym);*/
    /* double must be 4 */
    sym = &builtins[4];
    sym->intrinsic = 1;
    sym->gened = 1;
    sym->sym_type = &builtin_types[4];
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, "double");
    sym->sym_ada_name = new_string(predef_name("double"));
    sym->sym_type->type_kind = float_type;
    sym->sym_type->type_sizeof = SIZEOF_DOUBLE;
    sym->sym_type->type_alignof = ALIGNOF_DOUBLE;
    sym->sym_type->type_base = sym;
    /*store_sym(sym);*/

    for(i = 0; i < N_BUILTINS; i++)
    {
        typ = &builtin_types[i];
        typ->is_builtin = 1;
        set_hash_for_type(&builtin_types[i]);
    }

    init_anonymous_types();
    init_common_types();
    init_predef_names();
}

bool is_typedef(symbol_t* sym)
{
    return sym->sym_kind == type_symbol;
}

bool is_enum_literal(symbol_t* sym)
{
    return sym->sym_kind == enum_literal;
}

bool is_access_to_record(typeinfo_t* typ)
{
    assert(typ != NULL);

    if(typ->type_kind == pointer_to)
    {
        typ = typ->type_next;
        assert(typ != NULL);

        switch(typ->type_kind)
        {
            case struct_of:
            case union_of:
                return 1;
            default:
                break;
        }
    }

    return 0;
}

bool is_function_pointer(typeinfo_t* typ)
{
    assert(typ != NULL);

    if(typ->type_kind == pointer_to)
    {
        assert(typ->type_next != NULL);
        return typ->type_next->type_kind == function_type;
    }

    return 0;
}

static bool sym_aliases(symbol_t* s1, symbol_t* s2)
/* Tests if s1 is an alias for s2 */
{
    return (s1->aliases && s1->sym_value.aliased_sym == s2);
}

/*
 * Tags can be function parameters, fields of
 * unions and structs or enumeration literals.
 */
static int equal_tags(t1, t2) typeinfo_t *t1, *t2;
{
    symbol_t *bt1, *bt2;

    if(t1 == t2)
        return 1;

    bt1 = t1->type_base;
    bt2 = t2->type_base;

    if(bt1 == bt2)
        return 1;
    if(sym_aliases(bt1, bt2))
        return 1;
    if(sym_aliases(bt2, bt1))
        return 1;

    if(bt1 == NULL || bt2 == NULL)
        return 0;

    /* If two type syms share (i.e. point to, not just string equality),
       the same name, they're the same type. */
    if(bt1->sym_ada_name == bt2->sym_ada_name)
        return true;

    /* otherwise 2 pointers to unknown structs will have the same type */
    if((bt1->sym_tags == NULL) && (bt2->sym_tags == NULL))
        return (strcmp(bt1->sym_ada_name, bt2->sym_ada_name) == 0);

    return bt1->sym_tags == bt2->sym_tags;
}

static bool void_param_list(symbol_t* formals)
{
    if(!formals)
        return true;
    assert(formals->sym_type);
    return (formals->sym_type->type_kind == void_type);
}

static bool equal_formals(typeinfo_pt t1, typeinfo_pt t2)
{
    symbol_t* f1 = t1->type_info.formals;
    symbol_t* f2 = t2->type_info.formals;
    symbol_t* s1;
    symbol_t* s2;

    /* We're considering an empty formals list and a list (void)
     * to be equivalent.
     */
    if(void_param_list(f1) && void_param_list(f2))
        return true;

    for(s1 = f1, s2 = f2; s1 && s2; s1 = s1->sym_parse_list, s2 = s2->sym_parse_list)
    {
        /* Both formals lists must have ellipsis at same place */
        if(s1 == ellipsis_sym)
            return s2 == ellipsis_sym;
        if(s2 == ellipsis_sym)
            return false;

        if(!equal_types(s1->sym_type, s2->sym_type))
            return false;
    }
    return !s1 && !s2;
}

/*
 * Type comparison routine.
 */
static bool matching_types(typeinfo_t* t1, typeinfo_t* t2, bool assignment)
/* assignment==true if we're trying to assign a t2 to a t1 */
{
    if(t1 == NULL)
        return 0;
    if(t2 == NULL)
        return 0;
    if(t1 == t2)
        return 1;

    for(; t1 && t2; t1 = t1->type_next, t2 = t2->type_next)
    {
        if((t1 == NULL) || (!(t1->type_hash)))
            return 0;
        if((t2 == NULL) || (!(t2->type_hash)))
            return 0;
        if(t1->type_hash != t2->type_hash)
            return 0;
        if(t1->type_kind != t2->type_kind)
            return 0;
        if(t1->is_unsigned != t2->is_unsigned)
            return 0;
        if(t1->is_signed != t2->is_signed)
            return 0;
        if((t1->type_sizeof != t2->type_sizeof) && (decl_class(t1) != array_decl))
            return 0;
        if(t1->type_alignof != t2->type_alignof)
            return 0;
        if(t1->is_constant != t2->is_constant)
        {
            if(!assignment)
                return false;
            if(t2->is_constant)
                return false;
        }
        if(t1->is_long != t2->is_long)
            return 0;
        if(t1->is_boolean != t2->is_boolean)
            return 0;

        switch(t1->type_kind)
        {
            case struct_of:
            case union_of:
                if(!equal_tags(t1, t2))
                {
                    return 0;
                }
                break;
            case enum_type:
                return equal_tags(t1, t2);
            case pointer_to:
                continue; /* types pointed to must match */
            case function_type:
                /*
                 * For two functions types to be equal, the
                 * argument types must be equal.
                 */
                if(!equal_formals(t1, t2))
                    return false;
                continue; /* return types must be the same */
            default:
                break;
        }
    }

    return ((t1 == NULL) && (t2 == NULL));
}

bool equal_types(typeinfo_pt t1, typeinfo_pt t2)
{
    return matching_types(t1, t2, false);
}

bool assignment_equal_types(typeinfo_pt t1, typeinfo_pt t2)
{
    return matching_types(t1, t2, true);
}

/*
 * Type allocator.
 */
typeinfo_t* new_type(kind) typekind_t kind;
{
    typeinfo_t* typ;

    typ = (typeinfo_t*)allocate(sizeof(typeinfo_t));
    typ->type_kind = kind;
    set_hash_for_type(typ);
    return typ;
}

typeinfo_t* copy_type(typ) typeinfo_t* typ;
{
    typeinfo_t* t;

    if(typ == NULL)
    {
        return NULL;
    }

    t = (typeinfo_t*)allocate(sizeof(typeinfo_t));
    *t = *typ;
    t->type_next = copy_type(typ->type_next);
    return t;
}

symbol_t* copy_sym(sym) symbol_t* sym;
{
    symbol_t* d;
    d = new_sym();
    *d = *sym;
    d->sym_type = copy_type(sym->sym_type);
    return d;
}

typeinfo_t* typeof_int()
{
    return copy_type(&builtin_types[0]);
}

static symbol_t* int_basetype()
{
    return &builtins[0];
}

typeinfo_t* typeof_void()
{
    return copy_type(&builtin_types[1]);
}

typeinfo_t* typeof_char()
{
    return copy_type(&builtin_types[2]);
}

typeinfo_t* typeof_float()
{
    return copy_type(&builtin_types[3]);
}

typeinfo_t* typeof_double()
{
    return copy_type(&builtin_types[4]);
}

static typeinfo_t* typeof_enum()
{
    typeinfo_t* typ;

    typ = new_type(enum_type);
    typ->type_sizeof = SIZEOF_ENUM; /* See host.h */
    typ->type_alignof = ALIGNOF_ENUM;
    set_hash_for_type(typ);

    return typ;
}

static typeinfo_t* typeof_rec(is_union) int is_union;
{
    return new_type(is_union ? union_of : struct_of);
}

enum
{
    I_void,
    I_signed_char,
    I_unsigned_char,
    I_short,
    I_unsigned_short,
    I_int,
    I_unsigned,
    I_long,
    I_unsigned_long,
    I_char,
    I_boolean,
    I_float,
    I_double,
    I_long_double,

    I_string,
    I_const_charp,
    I_char_array,

    I_max
};

static typeinfo_pt type[I_max];

static void init_common_types(void)
/* Create standard typeinfo_pt representations for standard types. */
{
    typeinfo_pt t;
    int i;

    /* void */
    type[I_void] = typeof_void();

    /* signed char */
    type[I_signed_char] = t = typeof_int();
    t->is_signed = true;
    t->type_sizeof = SIZEOF_CHAR;
    t->type_alignof = ALIGNOF_CHAR;

    /* unsigned char */
    type[I_unsigned_char] = t = typeof_int();
    t->is_unsigned = true;
    t->type_sizeof = SIZEOF_CHAR;
    t->type_alignof = ALIGNOF_CHAR;

    /* short int */
    type[I_short] = t = typeof_int();
    t->is_short = true;
    t->type_sizeof = SIZEOF_SHORT;
    t->type_alignof = ALIGNOF_SHORT;

    /* unsigned short int */
    type[I_unsigned_short] = t = typeof_int();
    t->is_short = true;
    t->is_unsigned = true;
    t->type_sizeof = SIZEOF_SHORT;
    t->type_alignof = ALIGNOF_SHORT;

    /* int */
    type[I_int] = typeof_int();

    /* unsigned int */
    t = typeof_int();
    t->is_unsigned = 1;
    type[I_unsigned] = t;

    /* long */
    t = typeof_int();
    t->is_long = 1;
    t->type_sizeof = SIZEOF_LONG;
    t->type_alignof = ALIGNOF_LONG;
    type[I_long] = t;

    /* unsigned long */
    t = typeof_int();
    t->is_long = 1;
    t->is_unsigned = 1;
    t->type_sizeof = SIZEOF_LONG;
    t->type_alignof = ALIGNOF_LONG;
    type[I_unsigned_long] = t;

    /* char */
    t = typeof_char();
    t->type_sizeof = SIZEOF_CHAR;
    t->type_alignof = ALIGNOF_CHAR;
    type[I_char] = t;

    /* bool */
    t = typeof_int();
    t->is_boolean = 1;
    type[I_boolean] = t;

    /* float */
    t = typeof_float();
    type[I_float] = t;

    /* double */
    t = typeof_double();
    t->type_sizeof = SIZEOF_DOUBLE;
    t->type_alignof = SIZEOF_DOUBLE;
    type[I_double] = t;

    /* long double */
    t = typeof_float();
    t->is_long = 1;
    t->type_sizeof = SIZEOF_DOUBLE;
    t->type_alignof = ALIGNOF_DOUBLE;
    type[I_long_double] = t;

    /* string (char *) */
    t = add_pointer_type(typeof_char());
    type[I_string] = get_anonymous_type(t)->sym_type;

    t = add_pointer_type(typeof_char());
    t->type_next->is_constant = true;
    type[I_const_charp] = get_anonymous_type(t)->sym_type;

    /* char_array (char[]) */
    t = typeof_char_array();
    type[I_char_array] = get_anonymous_type(t)->sym_type;

    for(i = 0; i < I_max; i++)
    {
        set_hash_for_type(type[i]);
    }
}

typeinfo_pt type_void(void)
{
    return type[I_void];
}
typeinfo_pt type_signed_char(void)
{
    return type[I_signed_char];
}
typeinfo_pt type_unsigned_char(void)
{
    return type[I_unsigned_char];
}
typeinfo_pt type_short(void)
{
    return type[I_short];
}
typeinfo_pt type_unsigned_short(void)
{
    return type[I_unsigned_short];
}
typeinfo_pt type_int(void)
{
    return type[I_int];
}
typeinfo_pt type_unsigned(void)
{
    return type[I_unsigned];
}
typeinfo_pt type_long(void)
{
    return type[I_long];
}
typeinfo_pt type_unsigned_long(void)
{
    return type[I_unsigned_long];
}
typeinfo_pt type_char(void)
{
    return type[I_char];
}
typeinfo_pt type_float(void)
{
    return type[I_float];
}
typeinfo_pt type_double(void)
{
    return type[I_double];
}
typeinfo_pt type_long_double(void)
{
    return type[I_long_double];
}

typeinfo_pt type_boolean(void)
{
    return type[I_boolean];
}

typeinfo_pt type_string(void)
{
    return type[I_string];
}
typeinfo_pt type_charp(void)
{
    return type[I_string];
}
typeinfo_pt type_const_charp(void)
{
    return type[I_const_charp];
}

typeinfo_pt type_char_array(void)
{
    return type[I_char_array];
}

typeinfo_pt typeof_char_array(void)
/*
 * return a type equivalent to "char[]"
 */
{
    return add_array_type(typeof_char(), 0);
}

decl_class_t decl_class(typ) typeinfo_t* typ;
{
    if(typ == NULL)
    {
        return int_decl;
    }

    switch(typ->type_kind)
    {
        case field_type:
            return field_decl;
        case pointer_to:
            return pointer_decl;
        case array_of:
            return array_decl;
        case struct_of:
        case union_of:
            return struct_decl;
        case int_type:
            return int_decl;
        case float_type:
            return fp_decl;
        case void_type:
            return int_decl;
        case function_type:
            return func_decl;
        case enum_type:
            return enum_decl;
        default:
            fatal(__FILE__, __LINE__, "Unexpected type kind %d", typ->type_kind);
            return int_decl;
    }
}

/*
 * This really is a crappy way of dealing with long long, but
 * it was convenient at the time.
 */
static int how_long(tmod) int tmod;
{
    return tmod >> 12;
}

static void apply_type_mod(tmod, typ) int tmod;
typeinfo_t* typ;
{
    if(decl_class(typ) == int_decl)
    {
        if(tmod & TYPEMOD_UNSIGNED)
        {
            typ->is_unsigned = 1;
        }
        if(tmod & TYPEMOD_VOLATILE)
        {
            typ->is_volatile = 1;
        }
        if(tmod & TYPEMOD_SHORT)
        {
            typ->type_sizeof = SIZEOF_SHORT;
            typ->type_alignof = ALIGNOF_SHORT;
        }
        if(tmod & TYPEMOD_UNSIGNED)
        {
            typ->is_unsigned = 1;
        }
        if(tmod & TYPEMOD_VOLATILE)
        {
            typ->is_volatile = 1;
        }
        switch(how_long(tmod))
        {
#ifdef SIZEOF_LONG_LONG
            case 2:
                typ->type_sizeof = SIZEOF_LONG_LONG;
                typ->type_alignof = ALIGNOF_LONG_LONG;
                break;
#endif
            case 1:
                typ->type_sizeof = SIZEOF_LONG;
                typ->type_alignof = ALIGNOF_LONG;
                break;
            default:
                break;
        }
    }
}

bool inline_decl(typeinfo_pt type)
{
    typeinfo_pt t;
    for(t = type; t; t = t->type_next)
    {
        if(t->is_inline)
            return true;
    }
    return false;
}

bool static_decl(typeinfo_pt t, bool erase)
{
    if(t->is_static)
    {
        if(erase)
            t->is_static = false;
        return true;
    }
    if(t->type_next)
        return static_decl(t->type_next, erase);
    return false;
}

static symbol_t* sym_decl(typeinfo_pt typ, node_pt n, bool uniq)
{
    symbol_t* sym;

    assert(typ != NULL);
    assert(n != NULL);
    assert(n->node_kind == _Ident);
    assert(n->node.id.name != NULL);

    sym = new_sym();
    sym->sym_def = n->node_def;
    sym->sym_ident = n;

    configured_sym_info(sym, typ);

    if(sym->private)
    {
        set_unit_has_private_part(pos_unit(sym->sym_def));
    }

    if(!sym->sym_ada_name)
    {
        sym->sym_ada_name
        = ada_name(sym->sym_ident->node.id.name, uniq ? pos_unit(sym->sym_def) : -1);
    }

    if(!sym->sym_type)
    {
        sym->sym_type = copy_type(typ);
        sym->is_static = static_decl(sym->sym_type, true);
    }

    /* if declaration (i.e. type) was marked inline, mark
     * this symbol as inline.
     */
    sym->is_inline = inline_decl(typ);

    n->node.id.sym = sym;

    return sym;
}

typeinfo_t *concat_types(t1, t2) typeinfo_t *t1, *t2;
{
    typeinfo_t* t;

    if(t1 == NULL)
        return t2;
    if(t2 == NULL)
        return t1;

    for(t = t1; t->type_next; t = t->type_next)
        ;

    t->type_next = t2;
    return t1;
}

symbol_t *concat_symbols(s1, s2) symbol_t *s1, *s2;
{
    symbol_t* s;

    if(s1 == NULL)
        return s2;
    if(s2 == NULL)
        return s1;

    for(s = s1; s->sym_parse_list; s = s->sym_parse_list)
        ;

    s->sym_parse_list = s2;
    return s1;
}

symbol_t* ellipsis_sym = NULL;

symbol_t* concat_ellipsis(symbol_t* sym)
{
    if(ellipsis_sym == NULL)
    {
        ellipsis_sym = new_sym();
        ellipsis_sym->sym_ada_name = "Args";
    }
    set_ellipsis(pos_unit(sym->sym_def));
    return concat_symbols(sym, ellipsis_sym);
}

static int alignto(val, align) int val, align;
{
    if(align <= 2)
    {
        return val;
    }
    align--;
    return (val + align) & ~align;
}

static int type_alignof(typ) typeinfo_t* typ;
{
    assert(typ != NULL);
    return typ->type_alignof;
}

int type_sizeof(typ) typeinfo_t* typ;
{
    assert(typ != NULL);
    return typ->type_sizeof;
}

static void warn_negative_array(elem, nelem) node_t* elem;
host_int_t nelem;
{
    warning(NODE_FNAME(elem), NODE_FLINE(elem),
            "Array length %d is not supported (see %s:%d)", nelem, __FILE__, __LINE__);
}

typeinfo_pt add_array_type(typeinfo_pt typ, node_pt elem)
{
    host_int_t nelem = -1;
    typeinfo_t* array_type;
    unsigned long bsize;

    assert(typ != NULL);

    if(elem != NULL)
    {
        reduce_node(elem);
        if(elem->node_kind == _Type_Cast)
            elem = elem->node.binary.r;
        if(elem->node_kind == _Int_Number)
        {
            nelem = elem->node.ival;
            if(nelem < 0)
            {
                warn_negative_array(elem, nelem);
                nelem = -1;
            }
            free_node(elem);
        }
        else if((elem->node_kind == _Sym) && (elem->node.sym != NULL)
                && (elem->node.sym->sym_type->type_kind == enum_type))
        {
            nelem = elem->node.sym->sym_value.intval;
            if(nelem < 0)
            {
                warn_negative_array(elem, nelem);
                nelem = -1;
            }
        }
        else if(elem->node_kind == _Macro_ID)
        {
            /* TBD: assert this makes sense! */
            nelem = elem->node.macro->const_value.eval_result.ival;
        }
        else
        {
            nelem = -1;
            /*
             * Hell!  Probably reduce_node() in nodeop.c isn't
             * doing the right thing.  Or it could be in cpp_eval
             */
            warning(NODE_FNAME(elem), NODE_FLINE(elem),
                    "Failed reducing array index to a constant "
                    "integer value (see %s:%d)",
                    __FILE__, __LINE__);
        }
    }

    array_type = new_type(array_of);
    array_type->type_info.array.elements = nelem;
    array_type->type_next = typ;
    array_type->is_typedef = typ->is_typedef;

    array_type->type_alignof = type_alignof(typ);

    switch(nelem)
    {
        case -1:
            array_type->type_sizeof = 0; /* TBD: how is this used? */
            break;
        case 0:
            array_type->type_sizeof = 0;
            break;
        default:
            bsize = alignto(type_sizeof(typ), array_type->type_alignof);
            array_type->type_sizeof = bsize * nelem;
            break;
    }

    set_hash_for_type(array_type);
    return array_type;
}

typeinfo_t* typeof_typemod(adj) int adj;
{
    typeinfo_t* typ;

    typ = new_type(typemodifier);

    switch(adj)
    {
        case TYPEMOD_SHORT:
            typ->is_short = 1;
            break;
        case TYPEMOD_LONG:
            typ->is_long = 1;
            break;
        case TYPEMOD_SIGNED:
            typ->is_signed = 1;
            typ->is_unsigned = 0;
            break;
        case TYPEMOD_UNSIGNED:
            typ->is_unsigned = 1;
            typ->is_signed = 0;
            break;
        case TYPEMOD_CONST:
            typ->is_constant = 1;
            break;
        case TYPEMOD_VOLATILE:
            typ->is_volatile = 1;
            break;
        case TYPEMOD_TYPEDEF:
            typ->is_typedef = 1;
            break;
        case TYPEMOD_EXTERN:
            typ->is_extern = 1;
            break;
        case TYPEMOD_STATIC:
            typ->is_static = 1;
            break;
        case TYPEMOD_AUTO:
            typ->is_auto = 1;
            break;
        case TYPEMOD_REGISTER:
            typ->is_register = 1;
            break;
        case TYPEMOD_INLINE:
            typ->is_inline = 1;
            break;
        default:
            assert(0);
            break;
    }

    return typ;
}

static void combine_typespec(tmod) typeinfo_t* tmod;
{
    typeinfo_t* typ;

    assert(tmod != NULL);
    assert(tmod->type_kind == typemodifier);
    assert(tmod->type_next != NULL);

    typ = tmod->type_next;

    if(typ->is_volatile)
        tmod->is_volatile = 1;
    if(typ->is_constant)
        tmod->is_constant = 1;
    if(typ->is_extern)
        tmod->is_extern = 1;
    if(typ->is_static)
        tmod->is_static = 1;
    if(typ->is_auto)
        tmod->is_auto = 1;
    if(typ->is_register)
        tmod->is_register = 1;
    if(typ->is_typedef)
        tmod->is_typedef = 1;

    if(tmod->is_signed)
    {
        tmod->is_unsigned = 0;
    }
    else if(tmod->is_unsigned)
    {
        tmod->is_signed = 0;
    }
    else if(typ->is_unsigned)
    {
        tmod->is_unsigned = 1;
        tmod->is_signed = 0;
    }
    else if(typ->is_signed)
    {
        tmod->is_unsigned = 0;
        tmod->is_signed = 1;
    }

    if(tmod->is_short == 0)
    {
        if(typ->is_short)
            tmod->is_short = 1;
        if(typ->is_long_long)
            tmod->is_long_long = 1;

        if(typ->is_long)
        {
            if(tmod->is_long)
            {
                tmod->is_long_long = 1;
            }
            else
            {
                tmod->is_long = 1;
            }
        }
    }

    tmod->type_kind = typ->type_kind;
    tmod->type_info = typ->type_info;

    tmod->type_sizeof = typ->type_sizeof;
    tmod->type_alignof = typ->type_alignof;

    tmod->type_base = typ->type_base;
    tmod->type_next = typ->type_next;

    set_hash_for_type(tmod);

    deallocate(typ);
}

typeinfo_t* typeof_typespec(tlist) typeinfo_t* tlist;
{
    typeinfo_t* result = tlist;

top:
    assert(tlist != NULL);

    switch(tlist->type_kind)
    {
        case typemodifier:
            if(tlist->type_next == NULL)
            {
                /*
                 * If type is not specified it's assumed to be an int.
                 * ie.  short foo; == short int foo;
                 */
                result = tlist;
                result->type_kind = int_type;
                result->type_base = int_basetype();
                if(result->is_short)
                {
                    result->type_sizeof = SIZEOF_SHORT;
                    result->type_alignof = ALIGNOF_SHORT;
                }
                else if(result->is_long)
                {
                    result->type_sizeof = SIZEOF_LONG;
                    result->type_alignof = ALIGNOF_LONG;
                }
                else
                {
                    result->type_sizeof = SIZEOF_INT;
                    result->type_alignof = ALIGNOF_INT;
                    result->is_anon_int = !(result->is_unsigned || result->is_signed);
                }
                set_hash_for_type(result);
            }
            else
            {
                combine_typespec(tlist);
                goto top;
            }
            break;
        case struct_of:
            /* fixes error in gccs/Z/FontSelect.h that I don't understand */
            if(tlist->is_typedef && (tlist->type_next != NULL))
            {
                result = tlist->type_next;
                break;
            }
        case void_type:
        case int_type:
        case union_of:
        case enum_type:
        case float_type:
            if(!tlist->is_builtin && (tlist->type_next != NULL)
               && !tlist->type_next->is_constant)
                fatal(file_name(yypos), line_number(yypos),
                      "duplicate typedef?", __FILE__, __LINE__);
            break;
        case array_of:
        case pointer_to:
        case function_type:
            return result;
        default:
            assert(0);
            break;
    }

    return result;
}

typeinfo_t* typeof_specifier(sym) symbol_t* sym;
{
    typeinfo_t* typ;

    assert(sym != NULL);
    assert(sym->sym_type != NULL);

    typ = copy_type(sym->sym_type);
    typ->type_base = sym;

    return typ;
}

typeinfo_t* add_pointer_type(typ) typeinfo_t* typ;
{
    typeinfo_t* ptr_type;

    assert(typ != NULL);
    ptr_type = new_type(pointer_to);
    ptr_type->type_next = typ;
    ptr_type->is_typedef = typ->is_typedef;
    ptr_type->is_static = typ->is_static;
    ptr_type->type_sizeof = SIZEOF_ADDRESS;
    ptr_type->type_alignof = ALIGNOF_ADDRESS;
    set_hash_for_type(ptr_type);
    return ptr_type;
}

typeinfo_t* add_function_type(typ) typeinfo_t* typ;
{
    typeinfo_t* ftype;

    assert(typ != NULL);
    ftype = new_type(function_type);
    ftype->type_next = typ;
    ftype->is_typedef = typ->is_typedef;
    set_hash_for_type(ftype);
    return ftype;
}

typeinfo_t* pointer_to_sym(sym) symbol_t* sym;
{
    typeinfo_t* ptr_type;
    static typeinfo_t* int_pointer = NULL;

    if(int_pointer == NULL)
        int_pointer = add_pointer_type(typeof_int());

    ptr_type = new_type(pointer_to);
    ptr_type->type_sizeof = SIZEOF_ADDRESS;
    ptr_type->type_alignof = ALIGNOF_ADDRESS;
    ptr_type->type_base = sym;
    set_hash_for_type(ptr_type);
    return ptr_type;
}

static symbol_t* KnR_formals(params) node_t* params;
{
    symbol_t *p1, *p2;

    if(params == NULL)
        return NULL;

    switch(params->node_kind)
    {
        case _List:
            p1 = KnR_formals(params->node.binary.l);
            p2 = KnR_formals(params->node.binary.r);
            free_node(params);
            p1 = concat_symbols(p1, p2);
            break;
        case _Ident:
            p1 = sym_decl(typeof_int(), params, 0);
            break;
        default:
            assert(0);
            break;
    }

    return p1;
}

static symbol_t* grok_formals(params) node_t* params;
{
    symbol_t* p;

    if(params == NULL)
        return NULL;

    if(params->node_kind == _Sym)
    { /* Ansi function decl */
        p = params->node.sym;
        free_node(params);
        return p;
    }

    return KnR_formals(params);
}

static typeinfo_t* add_field(typ, width) typeinfo_t* typ;
node_t* width;
{
    typeinfo_t* ftype;

    assert(typ != NULL);
    assert(width != NULL);

    reduce_node(width);

    if(width->node_kind != _Int_Number || width->node.ival < 1 || width->node.ival > 31)
    {
        warning(NODE_FNAME(width), NODE_FLINE(width),
                "Bit filed width not handled properly (see %s:%d)", __FILE__, __LINE__);
        return typ;
    }

    ftype = new_type(field_type);
    ftype->type_next = typ;
    ftype->type_sizeof = width->node.ival;

    free_node(width);

    set_hash_for_type(ftype);
    return ftype;
}

static symbol_t* grok_decl_list(tspec, vlist, uniq) typeinfo_t* tspec;
node_t* vlist;
int uniq; /* Generate uniq Ada idenifer name */
{
    symbol_t *d1, *d2;

    assert(tspec != NULL);
    assert(vlist != NULL);

    switch(vlist->node_kind)
    {
        case _List:
            d1 = grok_decl_list(tspec, vlist->node.binary.l, uniq);
            d2 = grok_decl_list(tspec, vlist->node.binary.r, uniq);
            free_node(vlist);
            return concat_symbols(d1, d2);
        case _Ident:
            assert(vlist->node.id.name != NULL);
            d1 = sym_decl(tspec, vlist, uniq);
            return d1;
        case _Assign:
            d1 = grok_decl_list(tspec, vlist->node.binary.l, uniq);
            d1->sym_value.initializer = vlist->node.binary.r;
            d1->has_initializer = 1;
            free_node(vlist);
            return d1;
        case _Bit_Field:
            tspec = add_field(tspec, vlist->node.binary.r);
            if(vlist->node.binary.l == NULL)
            {
                d1 = new_sym();
                d1->sym_type = copy_type(tspec);
            }
            else
            {
                d1 = grok_decl_list(tspec, vlist->node.binary.l, 0);
            }
            free_node(vlist);
            return d1;
        case _Array_Index:
            /* vlist->node.binary.l is base declaration */
            /* vlist->node.binary.r is array index      */
            {
                typeinfo_pt atspec = add_array_type(tspec, vlist->node.binary.r);
                if(vlist->node.binary.l)
                {
                    d1 = grok_decl_list(atspec, vlist->node.binary.l);
                }
                else
                {
                    d1 = new_sym();
                    d1->sym_type = atspec;
                }
                return d1;
            }

        case _Indirect:
            tspec = add_pointer_type(tspec);
            if(vlist->node.unary == NULL)
            {
                d1 = new_sym();
                d1->sym_type = tspec;
                return d1;
            }
            return grok_decl_list(tspec, vlist->node.unary, uniq);
        case _Func_Call:
            /*
             * We're declaring a function.
             * vlist->node.binary.r is the formal parameter list.
             */
            {
                symbol_t* formals = grok_formals(vlist->node.binary.r);

                tspec = add_function_type(tspec);
                tspec->type_info.formals = formals;
                d1 = grok_decl_list(tspec, vlist->node.binary.l, uniq);
                if(d1->sym_tags == NULL)
                {
                    d1->sym_tags = formals;
                }
                assert(d1->sym_type);
                return d1;
            }

        default:
            fatal(file_name(yypos), line_number(yypos), "Unhandled node kind %d [%s:%d]",
                  vlist->node_kind, __FILE__, __LINE__);
            break;
    }

    return NULL;
}

static void check_type_base(sym) symbol_t* sym;
{
    typeinfo_t* typ;

    assert(sym != NULL);

    typ = sym->sym_type;
    assert(typ != NULL);

    switch(decl_class(typ))
    {
        case pointer_decl:
        case array_decl:
            typ->type_base = sym;
            break;
        default:
            assert(typ->type_base != NULL);
            break;
    }
}

static symbol_t* set_symbol_kind(vlist) symbol_t* vlist;
{
    symbol_t* s;
    typeinfo_t* typ;

    assert(vlist != NULL);

    for(s = vlist; s; s = s->sym_parse_list)
    {
        typ = s->sym_type;
        if(typ->is_typedef)
        {
            s->sym_kind = type_symbol;
        }
        else if(typ->type_kind == function_type)
        {
            s->sym_kind = func_symbol;
        }
        else
        {
            s->sym_kind = var_symbol;
        }
    }

    return vlist;
}

/*
 * Useful routine for debugging if your debugger lets
 * you invoke it directly.  If using verdix debugger
 * you can say:	 p debug_type(typ)
 * buy vads :-)
 */
static void dump_type(typ) typeinfo_t* typ;
{
    symbol_t* basetype;

    for(; typ; typ = typ->type_next)
    {
        if(typ->is_static)
            fputs("static/", stderr);
        if(typ->is_typedef)
            fputs("typedef/", stderr);
        if(typ->is_short)
            fputs("short/", stderr);
        if(typ->is_long)
            fputs("long/", stderr);
        if(typ->is_long_long)
            fputs("long_long/", stderr);
        if(typ->is_signed)
            fputs("signed/", stderr);
        if(typ->is_unsigned)
            fputs("unsigned/", stderr);
        switch(typ->type_kind)
        {
            case pointer_to:
                fputs("pointer_to", stderr);
                break;
            case array_of:
                fputs("array_of", stderr);
                break;
            case struct_of:
                fputs("struct_of", stderr);
                break;
            case union_of:
                fputs("union_of", stderr);
                break;
            case field_type:
                fputs("field_type", stderr);
                break;
            case int_type:
                fputs("int_type", stderr);
                break;
            case float_type:
                fputs("float_type", stderr);
                break;
            case void_type:
                fputs("void_type", stderr);
                break;
            case function_type:
                fputs("function_type", stderr);
                break;
            case enum_type:
                fputs("enum_type", stderr);
                break;
            case typemodifier:
                fputs("typemodifier", stderr);
                break;
            default:
                fputs("default", stderr);
                break;
        }
        fputc('[', stderr);
        basetype = typ->type_base;
        if(basetype != NULL)
        {
            assert(basetype->sym_ident != NULL);
            assert(basetype->sym_ident->node_kind == _Ident);
            assert(basetype->sym_ident->node.id.name != NULL);
            fputs(basetype->sym_ident->node.id.name, stderr);
        }
        fputs("] ", stderr);
    }
    fputc('\n', stderr);
}

static int simple_ptr_typedef(typ) typeinfo_t* typ;
{
    if(typ->is_typedef && typ->type_kind == pointer_to)
    {
        switch(decl_class(typ->type_next))
        {
            case int_decl:
            case fp_decl:
                return 1;
            default:
                break;
        }
    }
    return 0;
}

/*
 * OK.	The following looks really crappy.  Since C lets you
 * generate array types and pointer types without any formal
 * declaration, we'll need to traverse all types seen in the
 * C source and generate similar Ada types.  For example, if
 * foo(int*) is seen we'll need to generate type a_int_t is
 * access c.int automatically.  OK wise guy, do you
 * have a better idea?
 */

void all_types_gened(typeinfo_pt typ, file_pos_t pos)
{
    symbol_t* basetype;

    if(!typ || typ->is_builtin)
        return;

    if(typ->type_next)
    {
        all_types_gened(typ->type_next, pos);
    }

    switch(typ->type_kind)
    {
        case pointer_to:
        case array_of:
        case struct_of:
        case union_of:
        case enum_type:
            break;
        case field_type:
        case void_type:
        case int_type:
        case float_type:
        case function_type:
            return;
        case typemodifier:
            if(typ->is_constant)
                return;
        default:
            assert(0);
    }

    basetype = typ->type_base;

    if(basetype == NULL)
    {
        if(simple_ptr_typedef(typ))
            return;
        basetype = get_anonymous_type(typ);
        assert(basetype != NULL);
    }
    if((!basetype->gened))
    {
        basetype->sym_def = pos;
        basetype->gened = 1;
        gen_tag_types(basetype->sym_tags, 0);
        /* This isn't right in the case of a
         * new type coming from a function pointer
         * that's a param of another function
         */
        gen_ada_type(basetype);
    }
}

void gen_tag_types(symbol_t* tags, bool is_func)
{
    symbol_t* sym;
    for(sym = tags; sym; sym = sym->sym_parse_list)
    {
        typeinfo_pt t = sym->sym_type;
        if(!t)
            continue;

        if(is_func)
        {
            typeinfo_pt tnext = t->type_next;
            if((t->type_kind == pointer_to && !tnext->is_constant) || (t->type_kind == array_of))
            {
                /* For argument pointer types (T*) we intend
                 * to generate "access T"; but for (const T *)
                 * we want to generate a type.
                 * TBD: Is this also the right thing for
                 * array types?
                 */
                all_types_gened(t->type_next, sym->sym_def);
                continue;
            }
        }
        all_types_gened(t, sym->sym_def);
    }
}

static symbol_t* decl_defined(symbol_t* sym)
/* Return existing symbol for sym if it's already in the symbol
 * table.  Otherwise return 0.
 */

{
    symbol_t* dup;

    assert(sym->sym_ident != NULL);
    assert(sym->sym_ident->node_kind == _Ident);
    assert(sym->sym_ident->node.id.name != NULL);
    dup = find_sym(sym->sym_ident->node.id.name);

    if(dup && sym->sym_kind == dup->sym_kind && equal_types(sym->sym_type, dup->sym_type))
    {
        return dup;
    }
    else
    {
        store_sym(sym);
        return 0;
    }

} /* decl_defined */
static void find_units(unit_n ord, typeinfo_pt typ, bool from_body);

static void get_basetype_unit_list(unit_n ord, symbol_t* sym, bool from_body)
{
    symbol_t* tags;

    for(tags = sym->sym_tags; tags; tags = tags->sym_parse_list)
    {
        find_units(ord, tags->sym_type, from_body);
    }

    if(sym->sym_type->type_base != sym)
    {
        find_units(ord, sym->sym_type, from_body);
    }
}

static void find_units(unit_n ord, typeinfo_pt typ, bool from_body)
{
    typeinfo_pt t;
    symbol_t* basetype;
    unit_n unit_ord;

    for(t = typ; t; t = t->type_next)
    {
        if(!t->is_builtin)
        {
            basetype = t->type_base;
            if(basetype != NULL && !basetype->intrinsic && basetype->traversal_unit != ord)
            {
                basetype->traversal_unit = ord;
                get_basetype_unit_list(ord, basetype, from_body);
                unit_ord = pos_unit(basetype->sym_def);
                if(unit_ord != ord)
                {
                    unit_dependency(ord, unit_ord, from_body);
                }
            }
        }
    }
}

/*
 * Determine which Ada package need to be with'd
 * for the types of this symbol
 */
static void get_unit_list(symbol_t* sym)
{
    symbol_t* tags;
    unit_n unit_ord;
    bool from_body = !sym->is_declared_in_header;

    if(!auto_package)
        return;

    for(tags = sym->sym_tags; tags; tags = tags->sym_parse_list)
    {
        unit_ord = pos_unit(tags->sym_def);
        find_units(unit_ord, tags->sym_type, from_body);
    }

    unit_ord = pos_unit(sym->sym_def);
    find_units(unit_ord, sym->sym_type, from_body);
}

static bool is_static_function_type(typeinfo_t* typ)
{
top:
    assert(typ != NULL);

    if(typ->is_static)
    {
        return 1;
    }

    switch(typ->type_kind)
    {
        case function_type:
        case pointer_to:
            typ = typ->type_next;
            goto top;
        default:
            break;
    }

    return 0;
}

static void adjust_param_types(symbol_t* func)
/* Adjust the parameter types of a function:
 * a parameter "array of <type>" is adjusted to "pointer to <type>";
 * a parameter "function" is adjusted to "pointer to function"
 * See C Std 3.7.1
 */
{
    symbol_t* parm;
    typeinfo_pt type;

    for(parm = func->sym_tags; parm; parm = parm->sym_parse_list)
    {
        if(parm == ellipsis_sym)
            break;
        type = parm->sym_type;
        assert(type);
        switch(type->type_kind)
        {
            case array_of:
                parm->sym_type = add_pointer_type(type->type_next);
                break;
            case function_type:
                parm->sym_type = add_pointer_type(type);
                break;
            default:
                break;
        }
    }

} /* adjust_param_types */

static void grok_decl(symbol_t* sym)
{
    typeinfo_pt typ;

    assert(sym != NULL);

    typ = sym->sym_type;
    assert(typ != NULL);

    if(typ->is_typedef)
    {
        if(typ->type_kind == union_of || typ->type_kind == struct_of)
        {
            assert(typ->type_base != NULL);
            if(typ->type_base != sym)
            {
                sym->sym_tags = typ->type_base->sym_tags;
            }
        }
    }

    all_types_gened(typ, sym->sym_def);
    gen_tag_types(sym->sym_tags, (sym->sym_kind == func_symbol));

    get_unit_list(sym);

    switch(sym->sym_kind)
    {
        case type_symbol:
            store_sym(sym);
            if(!sym->gened)
            {
                sym->gened = 1;
                gen_ada_type(sym);
            }
            break;
        case func_symbol:
        {
            symbol_t* dup;
            adjust_param_types(sym);
            dup = decl_defined(sym);

            if(MAKING_BODY || !is_static_function_type(typ))
            {
                if(dup)
                {
                    sym->sym_ada_name = dup->sym_ada_name;
                    if(sym->has_initializer
                       && pos_unit(sym->sym_def) == pos_unit(dup->sym_def))
                    {
                        dup->interfaced = true;
                    }
                }

                if(!sym->gened)
                {
                    if(!dup || sym->has_initializer)
                    {
                        sym->gened = true;
                        gen_ada_func(sym, 0);
                    }
                }
            }
        }
        break;
        case var_symbol:
            if((!MAKING_BODY) && typ->is_static)
            {
                warning(file_name(sym->sym_def), line_number(sym->sym_def),
                        "Static variable %s in header, no Ada variable generated",
                        sym->sym_ada_name);
            }
            else if((MAKING_BODY || !typ->is_static) && (!decl_defined(sym))
                    && (!sym->gened))
            {
                sym->gened = 1;
                gen_ada_var(sym);
            }
            break;
        case param_symbol:
            break;
        case enum_literal:
        default:
            fatal(__FILE__, __LINE__, "Unhandled symbol kind %d", sym->sym_kind);
            break;
    }
}

void grok_declarations(list) symbol_t* list;
{
    symbol_t* next;

    for(; list; list = next)
    {
        next = list->sym_parse_list;
        grok_decl(list);
    }
}

void grok_func_param_decls(symbol_t* func)
{
    symbol_t* sym;
    if(func->sym_scope == 0)
        grok_decl(func);
    for(sym = func->sym_tags; sym; sym = sym->sym_parse_list)
    {
        sym->sym_kind = param_symbol;
        grok_decl(sym);
    }
}

symbol_t* nested_declarations(list) symbol_t* list;
{
    grok_declarations(list);
    return list;
}

static void grok_enum_lits(tags, typ) symbol_t* tags;
typeinfo_t* typ;
{
    symbol_t* sym;
    int ord = 0;

    for(sym = tags; sym; sym = sym->sym_parse_list)
    {
        assert(sym->sym_type == NULL);
        sym->sym_type = typ;
        if(sym->sym_value.intval == 0xBAD)
        {
            sym->sym_value.intval = ord++;
        }
        else
        {
            ord = sym->sym_value.intval + 1;
        }
    }
}

static symbol_t* gen_enum_sym(id) node_t* id;
{
    symbol_t* sym;
    char enum_name[2048];

    assert(id != NULL);
    assert(id->node_kind == _Ident);

    enum_name[0] = ENUM_PREFIX;
    strcpy(&enum_name[1], id->node.id.name);
    id->node.id.name = new_string(enum_name);

    sym = new_sym();
    sym->sym_type = typeof_enum();
    sym->sym_kind = type_symbol;
    sym->sym_ident = id;
    sym->sym_ada_name = ada_name(enum_name, pos_unit(sym->sym_def));
    sym->sym_def = id->node_def;

    return sym;
}

static void add_tags(decls, tags) symbol_t *decls, *tags;
{
    symbol_t* sym;

    for(sym = decls; sym; sym = sym->sym_parse_list)
    {
        assert(sym->sym_tags == NULL);
        sym->sym_tags = tags;
    }
}

static void grok_sizeof_struct(styp, fields) typeinfo_t* styp;
symbol_t* fields;
{
    symbol_t* sym;
    typeinfo_t *typ, *ftype;
    int min = 0;
    int aggsize = 0;
    int bitsize = 0;
    int width, tmp;

    for(sym = fields; sym; sym = sym->sym_parse_list)
    {
        typ = sym->sym_type;
        assert(typ != NULL);

        if(typ->type_kind == field_type)
        {
            width = typ->type_sizeof;
            ftype = typ->type_next;
            assert(ftype != NULL);

            if(ftype->type_sizeof > min)
            {
                min = ftype->type_sizeof;
            }

        align_field:
            tmp = alignto(aggsize, ftype->type_alignof);
            if(tmp != aggsize)
            {
                aggsize = tmp;
                bitsize = 0;
            }

            tmp = bitsize + width;
            if(tmp > ftype->type_sizeof * BITS_PER_BYTE)
            {
                aggsize += (bitsize + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
                bitsize = 0;
                goto align_field;
            }

            sym->bitoffset = aggsize * BITS_PER_BYTE + bitsize;
            typ->type_alignof = bitsize;

            bitsize += width;
        }
        else
        {
            if(bitsize != 0)
            {
                aggsize += (bitsize + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
                bitsize = 0;
            }

            aggsize = alignto(aggsize, typ->type_alignof);
            sym->bitoffset = aggsize * BITS_PER_BYTE;

            aggsize += typ->type_sizeof;
        }
    }

    if(bitsize != 0)
    {
        aggsize += (bitsize + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
    }

    if(aggsize < min)
    {
        aggsize = min;
    }

    styp->type_sizeof = aggsize;
    set_hash_for_type(styp);
}

static void grok_sizeof_union(utyp, fields) typeinfo_t* utyp;
symbol_t* fields;
{
    symbol_t* sym;
    typeinfo_t* typ;
    int aggsize = 0;

    for(sym = fields; sym; sym = sym->sym_parse_list)
    {
        typ = sym->sym_type;
        assert(typ != NULL);
        assert(typ->type_kind != field_type);
        if(typ->type_sizeof > aggsize)
        {
            aggsize = typ->type_sizeof;
        }
    }

    utyp->type_sizeof = aggsize;
    set_hash_for_type(utyp);
}

static void grok_alignof_record(typeinfo_pt rtyp, symbol_t* fields)
{
    symbol_t* sym;
    typeinfo_t* typ;
    int max = 0;

    for(sym = fields; sym; sym = sym->sym_parse_list)
    {
        typ = sym->sym_type;
        assert(typ != NULL);
        if(typ->type_kind == field_type)
        {
            typ = typ->type_next;
            assert(typ != NULL);
        }
        if(typ->type_alignof > max)
        {
            max = typ->type_alignof;
        }
    }

    rtyp->type_alignof = max;
    set_hash_for_type(rtyp);
}

static void grok_type(typ) typeinfo_t* typ;
{
    if(typ == NULL)
        return;

    switch(decl_class(typ))
    {
        case pointer_decl:
            typ->type_sizeof = SIZEOF_ADDRESS;
            typ->type_alignof = ALIGNOF_ADDRESS;
            if(is_function_pointer(typ))
            {
                grok_type(typ->type_next->type_next);
            }
            else
            {
                grok_type(typ->type_next);
            }
            set_hash_for_type(typ);
            break;
        case int_decl:
#ifdef SIZEOF_LONG_LONG
            if(typ->is_long_long)
            {
                typ->type_sizeof = SIZEOF_LONG_LONG;
                typ->type_alignof = ALIGNOF_LONG_LONG;
                typ->is_short = 0;
                typ->is_long = 0;
                set_hash_for_type(typ);
                return;
            }
#endif
            if(typ->is_long)
            {
                typ->type_sizeof = SIZEOF_LONG;
                typ->type_alignof = ALIGNOF_LONG;
                typ->is_short = 0;
                typ->is_long_long = 0;
                set_hash_for_type(typ);
                return;
            }
            if(typ->is_short)
            {
                typ->type_sizeof = SIZEOF_SHORT;
                typ->type_alignof = ALIGNOF_SHORT;
                typ->is_long_long = 0;
                set_hash_for_type(typ);
                return;
            }
            break;
        case fp_decl:
#if 0
	if (typ->is_long) {
	    typ->type_sizeof = SIZEOF_DOUBLE;
	    typ->type_alignof = ALIGNOF_DOUBLE;
	} else {
	    typ->type_sizeof = SIZEOF_LONG;
	    typ->type_alignof = ALIGNOF_LONG;
	}
#endif
            assert(typ->type_sizeof > 0 && typ->type_alignof > 0);
            typ->is_short = 0;
            typ->is_long_long = 0;
            set_hash_for_type(typ);
            return;
        case func_decl:
            /*
             *	fatal(file_name(yypos), line_number(yypos),
             *	      "function error", __FILE__, __LINE__);
             *	fprintf(stderr, "assertion failure line %d, %s\n",
             *	    line_number(current_file_pos), current_name);
             *	assert(0);
             */
            break;
        case enum_decl:
            typ->type_sizeof = SIZEOF_INT;
            typ->type_alignof = ALIGNOF_INT;
            set_hash_for_type(typ);
            break;
        case array_decl:
        case struct_decl:
            set_hash_for_type(typ);
            break;
        default:
            assert(0);
            break;
    }
}

static symbol_t* gen_rec_sym(node_pt id, typeinfo_pt typ)
{
    symbol_t* sym;
    char prefix;

    assert(id != NULL);
    assert(id->node_kind == _Ident);

    prefix = (typ->type_kind == struct_of) ? STRUCT_PREFIX : UNION_PREFIX;
    id->node.id.name = new_strf("%c%s", prefix, id->node.id.name);

    sym = sym_decl(typ, id, true);
    sym->sym_kind = type_symbol;

    sym->sym_type->type_base = sym;
    return sym;
}

symbol_t* grok_enumerator(node_pt id, node_pt val)
{
    symbol_t* sym;

    assert(id != NULL);
    assert(id->node_kind == _Ident);
    assert(id->node.id.name != NULL);

    sym = new_sym();
    sym->sym_kind = enum_literal;
    sym->sym_ident = id;
    sym->sym_ada_name = ada_name(sym->sym_ident->node.id.name, pos_unit(sym->sym_def));

    if(val == NULL)
    {
        sym->sym_value.intval = 0xBAD;
    }
    else
    {
        reduce_node(val);
        if(val->node_kind == _Int_Number)
        {
            sym->sym_value.intval = val->node.ival;
        }
        else
        {
            warning(NODE_FNAME(val), NODE_FLINE(val), "Enum literal value ignored");
            sym->sym_value.intval = 0xBAD;
        }
    }

    store_sym(sym);
    return sym;
}

static char* gen_type_name(prefix) int prefix;
{
    char name[128];

    sprintf(name, "%canonymous%d_t", prefix, next_anonymous_ord());
    return new_string(name);
}

static char *add_prefix(name, prefix, buf, len) char *name, *buf;
int prefix, len;
{
    char* result;
    int nlen;

    assert(name != NULL);

    nlen = strlen(name) + 2;

    if(buf == NULL || nlen > len)
    {
        result = (char*)malloc(strlen(name) + 2);
    }
    else
    {
        result = buf;
    }
    assert(result != NULL);
    result[0] = prefix;
    strcpy(&result[1], name);
    return result;
}

bool new_naming_scheme = false;

symbol_t* anonymous_enum(literals) symbol_t* literals;
{
    symbol_t* sym;

    assert(literals != NULL);

    sym = new_sym();
    sym->sym_type = typeof_enum();
    sym->sym_type->type_base = sym;
    sym->sym_kind = type_symbol;
    sym->sym_ident
    = new_naming_scheme ?
      new_node(_Ident, new_strf("%c%s_etc_t", ENUM_PREFIX, literals->sym_ada_name)) :
      new_node(_Ident, gen_type_name(ENUM_PREFIX));

    sym->sym_ada_name = ada_name(sym->sym_ident->node.id.name, pos_unit(sym->sym_def));
    sym->is_created_name = true;
    grok_enum_lits(literals, sym->sym_type);
    add_tags(sym, literals);
    return sym;
}

symbol_t* named_enum(id, literals) node_t* id;
symbol_t* literals;
{
    symbol_t* sym;
    char buf[1024];

    assert(id != NULL);
    assert(literals != NULL);
    assert(id->node_kind == _Ident);

    buf[0] = ENUM_PREFIX;
    strcpy(&buf[1], id->node.id.name);
    sym = find_sym(buf);

    if(sym == NULL)
    {
        sym = new_sym();
        sym->sym_type = typeof_enum();
        sym->sym_type->type_base = sym;
        sym->sym_ident = new_node(_Ident, new_string(buf));
        sym->sym_ada_name
        = ada_name(sym->sym_ident->node.id.name, pos_unit(sym->sym_def));
        sym->sym_kind = type_symbol;
        store_sym(sym);
    }
    else
    {
        sym->is_created_by_reference = 0;
    }

    sym->sym_def = id->node_def;
    grok_enum_lits(literals, sym->sym_type);
    add_tags(sym, literals);
    return sym;
}

symbol_t* enum_reference(id) node_t* id;
{
    char buf[256];
    char* name;
    symbol_t* sym;

    assert(id != NULL);
    assert(id->node_kind == _Ident);

    name = add_prefix(id->node.id.name, ENUM_PREFIX, buf, sizeof(buf));

    sym = find_sym(name);
    if(sym == NULL || !is_typedef(sym))
    {
        sym = new_sym();
        sym->sym_type = typeof_enum();
        sym->sym_type->type_base = sym;
        sym->sym_kind = type_symbol;
        if(name != buf)
        {
            id->node.id.name = name;
        }
        else
        {
            id->node.id.name = new_string(name);
        }
        sym->sym_ident = id;
        sym->sym_ada_name
        = ada_name(sym->sym_ident->node.id.name, pos_unit(sym->sym_def));
        sym->is_created_by_reference = 1;
        store_sym(sym);
    }
    else
    {
        if(name != buf)
        {
            free(name);
        }
    }

    return sym;
}

static char* anonymous_rec_name(is_union) int is_union;
{
    return gen_type_name(is_union ? UNION_PREFIX : STRUCT_PREFIX);
}

static symbol_t* delete_unamed_fields(tags) symbol_t* tags;
{
    symbol_t* head = NULL;
    symbol_t* last = NULL;

    if(tags == NULL)
        return NULL;

    for(; tags; tags = tags->sym_parse_list)
    {
        if(tags->sym_ident == NULL)
        {
            if(head != NULL)
            {
                last->sym_parse_list = tags->sym_parse_list;
            }
        }
        else
        {
            if(head == NULL)
            {
                head = tags;
            }
            else
            {
                last->sym_parse_list = tags;
            }
            last = tags;
        }
    }

    return head;
}

symbol_t* anonymous_rec(is_union, tags) int is_union;
symbol_t* tags;
{
    symbol_t* sym;
    typeinfo_t* rtyp;

    assert(tags != NULL);

    rtyp = typeof_rec(is_union);

    sym = new_sym();
    sym->sym_type = rtyp;
    sym->sym_kind = type_symbol;
    sym->sym_ident = new_node(_Ident, anonymous_rec_name(is_union));
    sym->is_created_name = true;
    sym->sym_ada_name = ada_name(sym->sym_ident->node.id.name, pos_unit(sym->sym_def));
    sym->sym_tags = tags;

    rtyp->type_base = sym;

    if(is_union)
    {
        grok_sizeof_union(rtyp, tags);
    }
    else
    {
        grok_sizeof_struct(rtyp, tags);
        tags = delete_unamed_fields(tags);
        sym->sym_tags = tags;
    }

    grok_alignof_record(rtyp, tags);

    return sym;
}

symbol_t* named_rec(bool is_union, node_pt id, symbol_t* tags)
{
    symbol_t* sym;
    symbol_t* newsym;
    symbol_t* result;
    typeinfo_pt rtyp;
    char buf[1024];

    assert(id != NULL);
    assert(id->node_kind == _Ident);
    assert(tags != NULL);

    /* See if the record's name is already in use. */
    buf[0] = is_union ? UNION_PREFIX : STRUCT_PREFIX;
    strcpy(&buf[1], id->node.id.name);
    sym = find_sym(buf);

    if(sym == NULL)
    {
        rtyp = typeof_rec(is_union);
        newsym = gen_rec_sym(id, rtyp);
        store_sym(newsym);
        sym = newsym;
        result = newsym;
    }
    else
    {
        /* If symbol name already in use, figure out how
         * to interrelate record definition with existing symbol.
         */

        int incomplete_ord, complete_ord;

        sym->is_created_by_reference = 0;

        incomplete_ord = pos_unit(sym->sym_def);
        complete_ord = pos_unit(tags->sym_def);
        if(incomplete_ord != complete_ord)
        {
            unit_dependency(incomplete_ord, complete_ord, !sym->is_declared_in_header);

            rtyp = typeof_rec(is_union);
            newsym = gen_rec_sym(id, rtyp);
            sym->aliases = true;
            sym->sym_value.aliased_sym = newsym;
            store_sym(newsym);

            newsym->gened = 1;
            newsym->sym_tags = tags;
            gen_tag_types(tags, 0);
            gen_ada_type(newsym);

            result = newsym;
        }
        else
        {
            /*
             * If we're here, this was an incomplete type in the
             * current file that is now being completed.
             * Make its definition come out here.
             */
            sym->sym_def = tags->sym_def;
            sym->is_declared_in_header = tags->is_declared_in_header;
            rtyp = sym->sym_type;
            sym->sym_tags = tags;
            gen_tag_types(tags, 0);
            if(!sym->gened)
            {
                sym->gened = 1;
                gen_ada_type(sym);
            }
            result = sym;
        }
    }

    if(is_union)
    {
        grok_sizeof_union(rtyp, tags);
    }
    else
    {
        grok_sizeof_struct(rtyp, tags);
        tags = delete_unamed_fields(tags);
    }

    result->sym_tags = tags;
    grok_alignof_record(rtyp, tags);

    return result;

} /* named_rec */

symbol_t* rec_reference(bool is_union, node_pt id)
{
    symbol_t* sym;
    typeinfo_pt rtyp;
    char buf[1024];

    assert(id != NULL);
    assert(id->node_kind == _Ident);

    buf[0] = is_union ? UNION_PREFIX : STRUCT_PREFIX;
    strcpy(&buf[1], id->node.id.name);

    sym = find_sym(buf);
    if(sym == NULL)
    {
        rtyp = typeof_rec(is_union);
        sym = gen_rec_sym(id, rtyp);
        sym->is_created_by_reference = true;
        store_sym(sym);
    }

    return sym;
}

static int no_typemods(typ) typeinfo_t* typ;
{
    for(; typ; typ = typ->type_next)
    {
        if((typ->type_kind == typemodifier) && !typ->is_constant)
        {
            return 0;
        }
    }
    return 1;
}

symbol_t* novar_declaration(tlist) typeinfo_t* tlist;
{
    symbol_t* result = NULL;

    assert(tlist != NULL);

    tlist = typeof_typespec(tlist);
    assert(no_typemods(tlist));

    switch(decl_class(tlist))
    {
        case int_decl:
            return NULL;
        case enum_decl:
        case struct_decl:
            assert(tlist->type_base != NULL);
            return tlist->type_base;
        case pointer_decl:
        case fp_decl:
        case func_decl:
        case array_decl:
            break;
        default:
            assert(0);
            break;
    }
    assert(0);
    return result;
}

static node_pt find_direct_name(node_pt vlist)
/*
 * Find first node that is an identifier, on a list.
 * Just look for direct ones, not indirect.
 */
{
    node_t* result = NULL;

    if(vlist->node_kind == _Ident)
        return vlist;
    if(vlist->node_kind == _List)
    {
        result = find_direct_name(vlist->node.binary.l);
        if(!result)
        {
            result = find_direct_name(vlist->node.binary.r);
        }
    }
    return result;
}

#define substr(s1, s2) (strncmp((s1), (s2), sizeof(s2) - 1) == 0)

symbol_t* var_declaration(typeinfo_pt tlist, node_pt vlist)
{
    symbol_t* decl_list;
    node_pt typedef_name = 0;

    assert(tlist != NULL);
    assert(vlist != NULL);

    tlist = typeof_typespec(tlist);
    assert(no_typemods(tlist));

    /*
     * If the type is an anonymous struct, union, or enum,
     * and it is being given a real name with a typedef,
     * substitute the real name and throw away the anonymous one.
     */
    if(tlist->is_typedef
       && (tlist->type_kind == struct_of || tlist->type_kind == union_of
           || tlist->type_kind == enum_type)
       && tlist->type_base && tlist->type_base->is_created_name)
    {
        typedef_name = find_direct_name(vlist);
        /* Actual name substitution happens below, after
         * the declaration list has been digested.
         */
    }

    current_file_pos = vlist->node_def;
    current_name = vlist->node.id.name;
    grok_type(tlist);

    decl_list = grok_decl_list(tlist, vlist, 1);

    /*
     * If a direct name for a typedef was found above, share the mapped
     * Ada name between the declaration and the type.
     */
    if(typedef_name)
    {
        tlist->type_base->sym_ada_name = typedef_name->node.id.sym->sym_ada_name;
    }

    return set_symbol_kind(decl_list);
}

symbol_t* function_spec(typeinfo_pt tlist, node_pt f, int scope_level)
{
    symbol_t* fdecl;
    symbol_t* s;

    assert(f != NULL);

    if(tlist == NULL)
    {
        tlist = typeof_int();
        tlist->is_anon_int = 1;
    }
    else
    {
        tlist = typeof_typespec(tlist);
    }

    assert(no_typemods(tlist));
    grok_type(tlist);

    fdecl = grok_decl_list(tlist, f, 1);
    assert(fdecl != NULL);

    /* Put param names in symbol table. */
    for(s = fdecl->sym_tags; s; s = s->sym_parse_list)
    {
        store_sym(s); /* TBD this call necessary? */
    }

    fdecl->sym_kind = func_symbol;
    fdecl->sym_scope = scope_level;

    return fdecl;
}

static void set_field_names(tags) symbol_t* tags;
{
    for(; tags; tags = tags->sym_parse_list)
    {
        if(tags->sym_ada_name == NULL)
        {
            if(tags->sym_ident != NULL)
            {
                tags->sym_ada_name
                = ada_name(tags->sym_ident->node.id.name, pos_unit(tags->sym_def));
            }
        }
    }
}

symbol_t* field_declaration(typeinfo_t* tlist, node_t* vlist)
{
    symbol_t* decl_list;
    symbol_t* sym;

    assert(tlist != NULL);
    assert(vlist != NULL);

    tlist = typeof_typespec(tlist);
    assert(no_typemods(tlist));
    grok_type(tlist);

    decl_list = grok_decl_list(tlist, vlist, 0);
    set_field_names(decl_list);
    gen_tag_types(decl_list, 0);

    /* mark symbols as being struct/union members */
    for(sym = decl_list; sym; sym = sym->sym_parse_list)
    {
        sym->is_struct_or_union_member = true;
    }

    return set_symbol_kind(decl_list);
}

void typed_external_decl(symbol_t* syms, comment_block_pt comment)
{
    symbol_t* next;

    if(syms)
        syms->comment = comment;

    for(; syms; syms = next)
    {
        next = syms->sym_parse_list;
        grok_decl(syms);
    }
}

static char* next_param_name()
{
    char buf[48];
    sprintf(buf, "p%d", next_param());
    return new_string(buf);
}

symbol_t* noname_simple_param(typ) typeinfo_t* typ;
{
    symbol_t* sym;
    char* name;

    assert(typ != NULL);

    name = next_param_name();

    typ = typeof_typespec(typ);
    assert(no_typemods(typ));
    grok_type(typ);

    sym = new_sym();
    sym->sym_ident = new_node(_Ident, name);
    sym->sym_ada_name = name;
    sym->sym_type = typ;
    sym->sym_kind = param_symbol;

    return sym;
}

static symbol_t* abstract_param(typeinfo_t* typ, node_t* adecl, bool named)
{
    symbol_t* sym;
    char* name;

    assert(typ != NULL);
    assert(adecl != NULL);

    if(named)
    {
        next_param();
    }
    else
    {
        name = next_param_name();
    }

    typ = typeof_typespec(typ);
    assert(no_typemods(typ));
    grok_type(typ);

    sym = grok_decl_list(typ, adecl, 0);
    if(!named)
    {
        sym->sym_ident = new_node(_Ident, name);
        sym->sym_ada_name = name;
    }
    sym->sym_kind = param_symbol;

    return sym;
}

symbol_t* noname_abstract_param(typ, adecl) typeinfo_t* typ;
node_t* adecl;
{
    return abstract_param(typ, adecl, false);
}

symbol_t* named_abstract_param(typ, adecl) typeinfo_t* typ;
node_t* adecl;
{
    return abstract_param(typ, adecl, true);
}

typeinfo_t* abstract_declarator_type(typeinfo_pt typ, node_pt adecl)
{
    symbol_t* sym;
    typeinfo_pt result;
    scope_push(Unspecified_scope); /* create new symbol in dummy scope */
    sym = noname_abstract_param(typ, adecl);
    result = sym->sym_type;
    scope_pop();
    return result;
}

static void KnR_tag_type(p, params) symbol_t *p, *params;
{
    for(; params; params = params->sym_parse_list)
    {
        if(!strcmp(p->sym_ident->node.id.name, params->sym_ident->node.id.name))
        {
            p->sym_type = params->sym_type;
            return;
        }
    }
}

void KnR_params(func, params) symbol_t *func, *params;
{
    symbol_t* tag;

    assert(func != NULL);
    assert(params != NULL);

    for(tag = func->sym_tags; tag; tag = tag->sym_parse_list)
    {
        tag->sym_kind = param_symbol;
        KnR_tag_type(tag, params);
    }
}

void function_def(f) symbol_t* f;
{
    symbol_t* dup;

    assert(f != NULL);

    if(!MAKING_BODY && is_static_function_type(f->sym_type))
    {
        return;
    }

    assert(f->sym_ident != NULL);
    assert(f->sym_ident->node_kind == _Ident);
    assert(f->sym_ident->node.id.name != NULL);

    dup = find_sym(f->sym_ident->node.id.name);

    grok_decl(f);

#if 0
    if (dup->sym_tags == NULL && f->sym_tags != NULL) {
	/* Must be K&R crap */

	dup->sym_tags = f->sym_tags;
	gen_tag_types(dup->sym_tags, 1);
    }
#endif
}

/* look up an identifier in the symbol table */

node_pt bind_to_sym(node_pt id)
{
    symbol_t* s;
    macro_t* m;

    if(do_const_macros && (m = macro_find(id->node.id.name)))
    {
        return new_node(_Macro_ID, m);
    }

    s = find_sym(id->node.id.name);

    if(!s)
    {
        warning_at(id->node_def, "use of undeclared identifier '%s'",
                   id->node.id.name);
        return id;
    }

    /* TBD: when does id get deallocated? */
    /* TBD: is it really necessary to allocate a new sym each time? */
    return new_node(_Sym, s);
}

int num_dimensions(typeinfo_pt typ)
{
    int res = 1;

    assert((typ != NULL) && (typ->type_kind == array_of));
    while(1)
    {
        typ = typ->type_next;
        assert(typ != NULL);
        if(typ->type_kind != array_of)
            return res;
        res++;
    }
}

/*
 * Allocate an array of integers and return the address of its start.
 * Element 0 is the number of dimensions.
 * Elements 1..n are the value of each dimension.
 */
int* get_dimensions(typ) typeinfo_t* typ;
{
    int ndim = num_dimensions(typ);
    int* res = allocate((ndim + 1) * sizeof(int));
    int i;

    res[0] = ndim;
    for(i = 1; i <= ndim; i++)
    {
        assert(typ->type_kind == array_of);
        res[i] = typ->type_info.array.elements;
        typ = typ->type_next;
    }
    return res;
}

symbol_t* private_type_null(symbol_t* tsym)
{
    static symmap_t map;
    symbol_t* nsym;
    char name[512];

    if(!map)
        map = new_symmap("private_type_null");
    nsym = get_symmap(map, tsym);
    if(nsym)
        return nsym;

    /* create new null */
    nsym = new_sym();
    nsym->sym_kind = var_symbol;
    nsym->sym_scope_id = tsym->sym_scope_id;
    nsym->sym_scope = tsym->sym_scope;
    nsym->sym_def = tsym->sym_def;
    nsym->sym_type = tsym->sym_type;
    nsym->private = true;

    sprintf(name, "null_%s", tsym->sym_ada_name);
    nsym->sym_ada_name = ada_name(name, pos_unit(nsym->sym_def));

    set_symmap(map, tsym, nsym);
    return nsym;
}
