#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "c2ada.h"
#include "hostinfo.h"

#undef NULL
#define NULL 0

#define streq(a, b) (!strcmp(a, b))


extern int comment_size;
extern int repspec_flag;
extern int suppress_record_repspec;
extern int auto_package;
extern int flag_unions;
extern int import_decls;
extern int export_from_c;

enum array_gen_t
{
    init_var_array,
    var_array,
    bounds_array,
    box_array
};

typedef struct
{
    symbol_t *qhead, *qtail;
} sym_q;


struct compinfo_t
{
    sym_q simple_ptr_typeq;
    sym_q simple_array_typeq;
    sym_q simple_typeq;
    sym_q rec_ptr_typeq;
    sym_q funcq;
    sym_q varq;
    sym_q litq;

    /*
     * The following queue will need to be sorted so
     * that types get generated in an order appropriate
     * for valid Ada semantics.
     */
    sym_q sort_typeq;
};

static struct compinfo_t compilation[MAX_UNIQ_FNAMES];


extern int ada_version;
int max_const_name_indent = 24;

static int first_anon_func = 0;


static char* c_char;
static char* c_signed_char;
static char* c_function_pointer;
static char* c_bits;
static char* c_signed_long_long;
static char* c_unsigned_long_long;
static char* c_unsigned_long;
static char* c_signed_long;
static char* c_int;
static char* c_unsigned_int;
static char* c_signed_int;
static char* c_unsigned_short;
static char* c_signed_short;
static char* c_unsigned_char;
static char* c_void;
static char* c_char_ptr;
static char* c_const_char_ptr;
static char* c_char_array;
static char* c_void_star;
static char* c_const_void_star;
static char* c_array_index;
static char* c_char_array_index;

struct typeinfo_t* bogus_type;

/* forward references */
static int gen_params(symbol_t* params, int pass, char* result_to_add);

static void inline_func(symbol_t*, int);

static void enq(sym_q* q, symbol_t* sym)
{
    assert(sym->sym_gen_list == NULL);

    if(q->qhead == NULL)
    {
        q->qhead = sym;
    }
    else
    {
        q->qtail->sym_gen_list = sym;
    }

    q->qtail = sym;
}

static void enq_before(sym_q* q, symbol_t* sym, symbol_t* ante)
{
    if(!ante)
    {
        enq(q, sym);
    }
    else if(!q->qhead)
    {
        q->qhead = q->qtail = sym;
    }
    else if(q->qhead == ante)
    {
        q->qhead = sym;
        sym->sym_gen_list = ante;
    }
    else
    {
        symbol_t* s;
        for(s = q->qhead; s; s = s->sym_gen_list)
        {
            symbol_t* s2 = s->sym_gen_list;
            if(s2 == ante)
            {
                s->sym_gen_list = sym;
                sym->sym_gen_list = s2;
                return;
            }
        }
        /* ante wasn't in list */
        enq(q, sym);
    }
}

static decl_class_t points_to(typeinfo_t* typ)
{
    for(; decl_class(typ) == pointer_decl; typ = typ->type_next)
        ;
    return decl_class(typ);
}

bool simple_array_type(typeinfo_pt type)
{
    if(decl_class(type) != array_decl)
        return false;
    assert(type->type_next);
    switch(decl_class(type->type_next))
    {
        case int_decl:
        case fp_decl:
            return true;
        default:
            return false;
    }
}

void gen_ada_type(symbol_t* sym)
{
    typeinfo_pt typ;
    unit_n unit;

    assert(sym != NULL);
    assert(sym->sym_kind == type_symbol);

    typ = sym->sym_type;
    assert(typ != NULL);

    unit = pos_unit(sym->sym_def);

    switch(decl_class(typ))
    {
        case func_decl:
        case pointer_decl:
            switch(points_to(typ->type_next))
            {
                case int_decl:
                case fp_decl:
                    enq(&compilation[unit].simple_ptr_typeq, sym);
                    break;
                case struct_decl:
                    enq(&compilation[unit].rec_ptr_typeq, sym);
                    break;
                default:
                    enq(&compilation[unit].sort_typeq, sym);
                    break;
            }
            break;
        case enum_decl:
        case int_decl:
        case fp_decl:
            enq(&compilation[unit].simple_typeq, sym);
            break;
        case array_decl:
            if(simple_array_type(typ))
            {
                enq(&compilation[unit].simple_array_typeq, sym);
            }
            else
            {
                enq(&compilation[unit].sort_typeq, sym);
            }
            break;
        case struct_decl:
            enq(&compilation[unit].sort_typeq, sym);
            break;
        default:
            break;
    }
} /* gen_ada_type */

void gen_ada_func(symbol_t* sym, symbol_t* ante)
/* Add sym to the list of functions declared in the current unit.
 * If ante is not null, insert sym just before ante.
 */
{
    unit_n unit;

    assert(sym != NULL);
    assert(sym->sym_kind == func_symbol);
    assert(!(sym->sym_scope > 0 && sym->has_initializer));

    unit = pos_unit(sym->sym_def);

    enq_before(&compilation[unit].funcq, sym, ante);

    /* If a function is declared in an inner scope, then there's
     * (presumably) a reference to it in the static-level function,
     * so we need to move it in front that function.
     */
    if(sym == compilation[unit].funcq.qhead)
    {
        return;
    }
    if(sym->sym_scope > 0)
    {
        symbol_t* back2;
        symbol_t* back1;
        for(back2 = 0, back1 = compilation[unit].funcq.qhead;;
            back2 = back1, back1 = back1->sym_gen_list)
        {
            assert(back1->sym_gen_list);
            if(back1->sym_gen_list == sym)
            {
                /* reorder list */
                if(back2)
                {
                    back2->sym_gen_list = sym;
                }
                else
                {
                    compilation[unit].funcq.qhead = sym;
                }
                sym->sym_gen_list = back1;
                back1->sym_gen_list = 0;
                compilation[unit].funcq.qtail = back1;
                return;
            }
        }
    }
} /* gen_ada_func() */

void gen_ada_var(symbol_t* sym)
{
    unit_n unit;

    assert(sym != NULL);
    assert(sym->sym_kind == var_symbol);

    unit = pos_unit(sym->sym_def);
    enq(&compilation[unit].varq, sym);
}

void gen_ada_lit(symbol_t* sym)
{
    unit_n unit;
    assert(sym);
    unit = pos_unit(sym->sym_def);
    enq(&compilation[unit].litq, sym);
}

static void put_string_both(const char* s)
{
    put_string(s);
    if(output_is_spec())
    {
        output_to_body();
        put_string(s);
        output_to_spec();
    }
    else
    {
        output_to_spec();
        put_string(s);
        output_to_body();
    }
}

static void comment_start()
{
    putf("%>%-- ", COMMENT_POS);
}

void print_position(file_pos_t pos)
{
    char buf[200];
    extern int output_refs;
    #if 0
    if(output_refs)
    {
        sprintf(buf, "-- %s:%d\n", file_name(pos), (int)line_number(pos));
        /* right-justify out to column 79 */
        indent_to(80 - strlen(buf));
        put_string(buf);
    }
    else
    #endif
    {
        put_char('\n');
    }
}

static void mark_union(symbol_t* sym)
{
    assert(sym != NULL);

    inform(cur_unit_path(), output_line(), "Union %s generated from %s:%d",
           sym->sym_ada_name, file_name(sym->sym_def), line_number(sym->sym_def));
}

void print_value(host_int_t val, int base)
{
    char buf[64];

    switch(base)
    {
        case 16:
            sprintf(buf, "16#%x#", (unsigned int)val);
            break;
        case 8:
            sprintf(buf, "8#%o#", (unsigned int)val);
            break;
        default:
            sprintf(buf, "%d", (unsigned int)val);
            break;
    }
    put_string(buf);
}

void print_fp_value(host_float_t val)
{
    char buf[128];

    sprintf(buf, "%.20e", val);
    putf("%[%s%]", buf);
}

static void cond_concat(int* count, int* in_quote)
{
    if(*in_quote == 0 && *count != 0)
    {
        put_string(" & ");
        *count += 3;
    }
}

static void cond_start_quote(int* count, int* in_quote)
{
    if(!*in_quote)
    {
        putf("%{\""); /* put start-of-string-literal, quote-char */
        *in_quote = 1;
        (*count)++;
    }
}

static void cond_end_quote(int* count, int* in_quote)
{
    if(*in_quote)
    {
        putf("\"%}"); /* put quote-char, end-of-string-literal */
        *in_quote = 0;
        (*count)++;
    }
}

char* char_to_string(int c, bool c_string)
/*
 * c is the value of the character to represent;
 * c_string indicates whether a C string or an Ada string
 * is being generated.
 */
{
    static char buf[128];
    static char* to_c;
    if(!to_c)
        to_c = predef_name_copy("To_C");

    if(c == '"')
    {
        strcpy(buf, "\"\"");
    }
    else if(isprint(c))
    {
        sprintf(buf, "%c", c);
    }
    else if(ada_version >= 1995)
    {
        switch(c)
        {
            case '\0':
                sprintf(buf, c_string ? "%s.Nul" : "ascii.nul", predef_pkg);
                break;
            case '\n':
                sprintf(buf, c_string ? "%s(ascii.lf)" : "ascii.lf", to_c);
                break;
            case '\t':
                sprintf(buf, c_string ? "%s(ascii.ht)" : "ascii.ht", to_c);
                break;
            case '\r':
                sprintf(buf, c_string ? "%s(ascii.cr)" : "ascii.cr", to_c);
                break;
            default:
                if(c_string)
                {
                    sprintf(buf, "%s.char'val(%d)", predef_pkg, c);
                }
                else
                {
                    sprintf(buf, "Character'val(%d)", c);
                }
                break;
        }
    }
    else
    {
        switch(c)
        {
            case '\n':
                sprintf(buf, "ascii.lf");
                break;
            case '\t':
                sprintf(buf, "ascii.ht");
                break;
            case '\r':
                sprintf(buf, "ascii.cr");
                break;
            default:
                sprintf(buf, "character'val(%d)", c);
                break;
        }
    }
    return buf;
}

char* string_name(bool is_wide)
{
    static char* wide_string;
    static char* char_array;

    /* TBD: Could use a switch to see if Wide_String & Char_Array
     * have already been renamed, so that we don't need to predef
     * prefix.
     */

    if(!wide_string)
    {
        /* initialize names */
        wide_string = predef_name_copy("Wide_String");
        char_array = predef_name_copy("Char_Array");
    }

    if(is_wide)
    {
        return wide_string;
    }
    else if(ada_version >= 1995)
    {
        return char_array;
    }
    else
    {
        return "String";
    }
}

static void gen_string_stuff(void)
{
    /* Generate immediately visible string ops, consts */
    putf("%>subtype Char_Array is %s.Char_Array;\n", 4, predef_pkg);
    putf("%>use type Char_Array;\n", 4);
    putf("%>function \"&\"( S : Char_Array; C : %s.Char)", 4, predef_pkg);
    putf(" return Char_Array\n");
    putf("%>renames %s.\"&\";\n", 8, "Interfaces.C");
    putf("%>Nul : %s.Char renames %s.Nul;\n\n", 4, predef_pkg, predef_pkg);
}

void gen_char_array(char* name, char* val, bool is_wide_string, bool is_const)
{
    if(!cur_unit_has_const_string())
    {
        gen_string_stuff();
        set_cur_unit_has_const_string();
    }

    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(is_const ? ": constant " : ": ");
    put_string(string_name(is_wide_string));
    put_string(" := ");
    print_string_value(val ? val : "", -1, true);
    put_char(';');
}

static char* nul_name(int is_wide)
{
    if(is_wide)
        return ("Wide_Nul");
    else if(ada_version >= 1995)
        return "Nul";
    else
        return "Ascii.Nul";
}

void print_string_value(const char* val, int expected_len, bool c_string)
/* expected_len == -1 if there's no mandated length */
{
    int warned = 0;
    int in_quote = 0;
    int count; /* length of printed output */
    int len = strlen(val); /* number of chars in raw string */
    unsigned int c;
    char* repr;
    bool fixed_length = expected_len != -1;
    int excess;

    for(count = 0; *val; val++)
    {
        c = (unsigned int)*val;
        if((c > 127) && (ada_version < 1995))
        {
            if(!warned)
            {
                warned = 1;
                warning(cur_unit_path(), output_line(),
                        "Extended character set not yet supported");
            }
        }
        else
        {
            if(isprint(c))
            {
                cond_concat(&count, &in_quote);
                cond_start_quote(&count, &in_quote);
            }
            else
            {
                cond_end_quote(&count, &in_quote);
                cond_concat(&count, &in_quote);
            }
            repr = char_to_string(c, c_string);
            put_string(repr);
            count += strlen(repr);
        }
    }

    if(count == 0)
    {
        putf("%{\"\"%}");
    }
    else if(in_quote)
    {
        putf("\"%}");
    }

    /* "excess" is the number of nuls to append to the string */
    if(c_string && !fixed_length)
    {
        excess = 1;
    }
    else if(expected_len > len)
    {
        excess = expected_len - len;
    }
    else
    {
        excess = 0;
    }

    for(; excess; excess--)
    {
        putf(" & %s", nul_name(0));
    }
}

void print_char_value(int val)
{
    putf(isprint(val) ? "%{'%s'%}" : "%s", char_to_string(val, true));
}

static void comment_sizeof(unsigned int size, unsigned int align)
{
    char buf[80];
    comment_start();
    sprintf(buf, "sizeof(%d) alignof(%d)\n", size, align);
    put_string(buf);
}

static int valid_comment(node_t* n)
{
    return n != NULL && n->node_kind == _Ident && n->node.id.cmnt != NULL;
}

void print_comment(char* p)
{
    if(!p)
        return;

#if 0
	while (is_white((int)*p)) {
		p++;
	}
#else
    while(isspace(*p))
    {
        p++;
    }
#endif
    if(*p == 0)
        return;
    comment_start();
    for(; *p; p++)
    {
        if(*p == '\n')
        {
            new_line();
            print_comment(p + 1);
            return;
        }
        else
        {
            put_char(*p);
        }
    }
    new_line();
}

void c_comment(node_t* n)
{
    if(!valid_comment(n))
        return;
    print_comment(n->node.id.cmnt);
}

static void c_comment_or_position(symbol_t* sym)
{
    if(valid_comment(sym->sym_ident))
    {
        c_comment(sym->sym_ident);
    }
    else
    {
        print_position(sym->sym_def);
    }
}

static int from_header_file()
{
    char* p;

    for(p = cur_unit_source(); *p; p++)
    {
        if(p[0] == '.' && p[1] == 'h' && p[2] == 0)
        {
            return 1;
        }
    }

    return 0;
}

int should_import()
{
    if(!auto_package || !import_decls)
    {
        return 0;
    }
    return from_header_file();
}



void init_predef_names()
{
    static int initialized = 0;

    if(!initialized)
    {
        c_char = new_string(predef_name("char"));
        c_signed_char = predef_name_copy("signed_char");
        c_char_ptr = predef_name_copy("charp");
        c_const_char_ptr = predef_name_copy("const_charp");
        c_char_array = predef_name_copy("char_array");
        c_function_pointer = predef_name_copy("function_pointer");
        c_bits = new_string(predef_name("bits"));
        c_signed_long_long = predef_name_copy("long_long");
        c_unsigned_long_long = predef_name_copy("unsigned_long_long");
        c_unsigned_long = new_string(predef_name("unsigned_long"));
        c_signed_long = new_string(predef_name("long"));
        c_int = new_string(predef_name("int"));
        c_unsigned_int = new_string(predef_name("unsigned_int"));
        c_signed_int = new_string(predef_name("int"));
        c_unsigned_short = new_string(predef_name("unsigned_short"));
        c_signed_short = new_string(predef_name("short"));
        c_unsigned_char = new_string(predef_name("unsigned_char"));
        c_void = new_string(predef_name("void"));
        c_const_void_star = c_void_star = "System.Address";
        c_array_index = new_string(predef_name("natural_int"));
        c_char_array_index = predef_name_copy("size_t");
        bogus_type = new_type(int_type);

        initialized = 1;
    }
}

char* c_array_index_name(void)
{
    return c_array_index;
}

static char* botched_type(typeinfo_pt type)
{
    /* made into a function so it's easy to set a breakpoint */

    if(type->type_base)
    {
        error_at(type->type_base->sym_def, "botched type name");
    }
    else
    {
        error(__FILE__, __LINE__, "botched type name, no type symbol");
    }

    return " <botched type name> ";
}

static bool is_charp(typeinfo_pt);
static bool is_const_charp(typeinfo_pt);

#define BIGGEST_TYPE_NAME 100

static char* typesym_nameof(typeinfo_pt typ, bool use_parent_type)
/*
 * Return the type name associated with a type symbol;
 * ("type symbol" being the type_base field the "typ" argument.)
 */
{
    symbol_t* basetype = typ->type_base;
    symbol_t* typesym;
    unit_n unit_ord;
    static char buf[BIGGEST_TYPE_NAME];
    extern int yypos;

    if(basetype != NULL)
    {
        if(use_parent_type && decl_class(typ) == struct_decl)
        {
            assert(basetype->sym_type != NULL);
            basetype = basetype->sym_type->type_base;
            assert(basetype != NULL);
        }
        typesym = (basetype->aliases ? basetype->sym_value.aliased_sym : basetype);

        assert(typesym->sym_ada_name != NULL);

        /*
         * NB: The test against yypos is a way of detecting
         * symbols generated during fixup.
         */
        unit_ord = pos_unit(typesym->sym_def);
        if(!auto_package || is_current_unit(unit_ord) || typesym->intrinsic
           || typesym->sym_def == yypos)
        {
            return typesym->sym_ada_name;
        }
        sprintf(buf, "%s.%s", unit_name(unit_ord), typesym->sym_ada_name);
        return buf;
    }
    return 0;
}

char* int_type_builtin_name(typeinfo_pt typ)
{
    bool unsgnd = typ->is_unsigned;
    int size = typ->type_sizeof;

    assert(decl_class(typ) == int_decl);

    if(size == SIZEOF_CHAR)
    {
        return unsgnd ? c_unsigned_char : typ->is_signed ? c_signed_char : c_char;
    }
    else if(size == SIZEOF_SHORT)
    {
        return unsgnd ? c_unsigned_short : c_signed_short;
    }
    else if((size == SIZEOF_INT) && !(typ->is_long))
    {
        return unsgnd ? c_unsigned_int : c_signed_int;
    }
    else if(size == SIZEOF_LONG)
    {
        return unsgnd ? c_unsigned_long : c_signed_long;
    }
#ifdef SIZEOF_LONG_LONG
    if(size == SIZEOF_LONG_LONG)
        return unsgnd ? c_unsigned_long_long : c_signed_long_long;
#endif
    assert("non-standard int type");
    exit(1);

    return 0;
}

char* type_nameof(typeinfo_t* typ, int use_parent_type, int is_param)
{
    static char buf[BIGGEST_TYPE_NAME];

    symbol_t* basetype = typ->type_base;
    int dclass;

    assert(typ != NULL);

    if(typ == bogus_type)
        return "??? unknown type ???";

    if(typ->is_boolean)
        return "boolean";

    dclass = decl_class(typ);
    /*
     * special-case: (void *) and (const void *) parameters
     */
    if(is_param && (ada_version >= 1995)
       && ((dclass == pointer_decl) || (dclass == array_decl))
       && (typ->type_next) && (typ->type_next->type_kind == void_type))
    {
        if(typ->type_base)
        {
            return typesym_nameof(typ, use_parent_type);
        }
        else if(typ->type_next->is_constant)
        {
            return c_const_void_star;
        }
        else
        {
            return c_void_star;
        }
    }

    /*
     * if it's a parameter, that's a pointer, not to const, and Ada 95,
     * and not a typedef,
     * output "access typename" instead of the junk name
     */
    if(is_param && (ada_version >= 1995)
       && ((dclass == pointer_decl) || (dclass == array_decl))
       && (typ->type_next != NULL) && (!typ->type_next->is_constant)
       && ((typ->type_base == NULL)
           || ((typ->type_base->sym_type != NULL) && (!typ->type_base->sym_type->is_typedef))))
    {
        char buf2[BIGGEST_TYPE_NAME];
        char* res;

        res = type_nameof(typ->type_next, use_parent_type, 0);
        sprintf(buf2, "access %s", res);
        strcpy(buf, buf2);
        return buf;
    }

    if(basetype && equal_types(typ, basetype->sym_type))
    {
        return typesym_nameof(typ, use_parent_type);
    }

    {
        typeinfo_pt t = find_anonymous_type(typ);
        if(t)
            return t->type_base->sym_ada_name;
    }

    if(typ->type_kind == void_type)
        return c_void;

    switch(decl_class(typ))
    {
        case int_decl:
            return int_type_builtin_name(typ);
        case field_decl:
            sprintf(buf, "%s%d", c_bits, typ->type_sizeof);
            return buf;
        case enum_decl:
        case pointer_decl:
        case fp_decl:
        case func_decl:
        case array_decl:
        case struct_decl:
            break;
        default:
            assert(0);
            break;
    }

    if(basetype)
    {
        return typesym_nameof(typ, false);
    }

    return botched_type(typ);
} /* type_nameof */

static bool derived_in_same_unit(symbol_t* sym, typeinfo_pt typ)
{
    symbol_t* basetype = typ->type_base;

    if(basetype == NULL)
        return 0;
    if(pos_unit(sym->sym_def) != pos_unit(basetype->sym_def))
        return 0;
    if(!equal_types(typ, basetype->sym_type))
        return 0;
    return 1;
}

static void gen_int_type(symbol_t* sym)
{
    typeinfo_pt typ;

    assert(sym != NULL);
    assert(sym->sym_type != NULL);

    typ = sym->sym_type;

    putf("%>type %s is new ", 4, sym->sym_ada_name);

    if(derived_in_same_unit(sym, typ))
    {
        assert(typ->type_base != NULL);
        assert(typ->type_base->sym_type != NULL);
        put_string(type_nameof(typ->type_base->sym_type, 0, 0));
    }
    else
    {
        put_string(type_nameof(typ, 0, 0));
    }
    put_char(';');
    c_comment(sym->sym_ident);
    print_position(sym->sym_def);

    if(comment_size)
    {
        comment_sizeof(typ->type_sizeof, typ->type_alignof);
    }
    set_symbol_done(sym);
}

static void gen_fp_type(symbol_t* sym)
{
    gen_int_type(sym);
}

static void gen_size_rep(symbol_t* sym)
{
    typeinfo_t* typ;
    char buf[32];

    assert(sym != NULL);

    typ = sym->sym_type;
    assert(typ != NULL);

    putf("%>for %s'Size use ", 4, sym->sym_ada_name);

    if(typ->type_kind == enum_type)
    {
        sprintf(buf, "%s'Size;", c_int);
    }
    else
    {
        sprintf(buf, "%d;", typ->type_sizeof * BITS_PER_BYTE);
    }
    put_string(buf);
    print_position(sym->sym_def);
}

static int compar_enum(symbol_t** tag1, symbol_t** tag2)
/* Comparison routine used in sorting enum tags. */
/* Called from setup_tags */
{
    int val1 = (*tag1)->sym_value.intval;
    int val2 = (*tag2)->sym_value.intval;

    if(val1 < val2)
        return -1;
    else if(val1 == val2)
        return 0;
    else
        return 1;
}

static int
setup_tags(symbol_t* tag, symbol_t*** list_addr, int* ntags, int* default_order, int* has_equals)
/*
 * Prepares an enum tag list for printing, returns information
 * about the contents of the list.
 * Called from gen_enum_type.
 */
{
    typedef int (*cmpfunc_t)(const void*, const void*);
    symbol_t *t, **list;
    int ntg;
    int i;

    /* count the list */
    for(ntg = 0, t = tag; t; t = t->sym_parse_list)
        ntg++;
    *ntags = ntg;

    if(ntg != 0)
    {
        /* Make an array holding tags */
        *list_addr = list = (symbol_t**)allocate(ntg * sizeof(symbol_t*));
        for(i = 0, t = tag; t; t = t->sym_parse_list)
            list[i++] = t;

        /* sort list */
        qsort(list, ntg, sizeof(symbol_t*), (cmpfunc_t)compar_enum);
        for(i = 0; i < ntg - 1; i++)
        {
            if(list[i]->sym_value.intval != i)
            {
                *default_order = 0;
            }
            if(list[i]->sym_value.intval == list[i + 1]->sym_value.intval)
            {
                *has_equals = 1;
            }
        }
    }
    return ntg;
}

static void gen_tags(symbol_t* sym, symbol_t** list, int ntags, bool rep)
/* Called from gen_enum_type */
{
    char buf[64];
    int i;

    indent_to(4);
    if(rep)
    {
        putf("for %s use (\n", sym->sym_ada_name);
    }
    else
    {
        putf("type %s is (", sym->sym_ada_name);
        c_comment(sym->sym_ident);
        print_position(sym->sym_def);
    }

    for(i = 0; i < ntags; i++)
    {
        indent_to(8);
        put_string(list[i]->sym_ada_name);
        if(rep)
        {
            sprintf(buf, " => %d", (int)list[i]->sym_value.intval);
            put_string(buf);
        }
        if(i < ntags - 1)
            put_char(',');
        if(rep)
            put_char('\n');
        else
            c_comment_or_position(list[i]);
        while((i < ntags - 1)
              && (list[i]->sym_value.intval == list[i + 1]->sym_value.intval))
            i++;
    }
    indent_to(4);
    put_string(");\n");
}

static void gen_enum_type(symbol_t* sym)
{
    symbol_t** list;
    int ntags, i, default_order = 1, has_equals = 0;

    setup_tags(sym->sym_tags, &list, &ntags, &default_order, &has_equals);
    if(ntags == 0)
        return;

    gen_tags(sym, list, ntags, 0);
    if(!default_order)
    {
        gen_tags(sym, list, ntags, 1);
    }
    gen_size_rep(sym);
    if(has_equals)
    {
        put_char('\n');
        for(i = 1; i < ntags; i++)
            if(list[i]->sym_value.intval == list[i - 1]->sym_value.intval)
            {
                putf("%>%s : constant %s := %s;", 4, list[i]->sym_ada_name,
                     sym->sym_ada_name, list[i - 1]->sym_ada_name);
                c_comment_or_position(list[i]);
            }
        put_char('\n');
    }
    free(list);
    set_symbol_done(sym);
}

void subtype_decl(char* subtype_name, char* package_name, char* type_name, int indent, node_t* ident, file_pos_t pos)
{
    if((package_name != NULL) || (strcmp(subtype_name, type_name) != 0))
    {
        indent_to(indent);
        put_string("subtype ");
        put_string(subtype_name);
        put_string(" is ");
        if(package_name != NULL)
        {
            put_string(package_name);
            put_char('.');
        }
        put_string(type_name);
        put_char(';');
        if(ident != NULL)
            c_comment(ident);
        print_position(pos);
    }
}

typeinfo_pt return_type(symbol_t* subp)
{
    typeinfo_t *typ, *rtyp;

    typ = subp->sym_type;
    assert(typ != NULL);

    /* Allow for pointer to function as well as function */
    if(typ->type_kind == pointer_to)
        typ = typ->type_next;

    if(typ->type_kind != function_type)
    {
        warning_at(subp->sym_def, "Type not a function");
    }
    /*
    assert(typ->type_kind == function_type);
    */

    rtyp = typ->type_next; /* return type */
    assert(rtyp != NULL);

    return rtyp;
}

bool is_function(symbol_t* subp)
{
    typeinfo_t* rtyp;
    rtyp = return_type(subp);
    return rtyp->type_kind != void_type;
}

static char* anon_function_pointer_name(symbol_t* sym)
{
    static char buf[256];

    if(is_function(sym))
        sprintf(buf, "%s_func_access", sym->sym_ada_name);
    else
        sprintf(buf, "%s_proc_access", sym->sym_ada_name);
    return buf;
}

static bool is_anon_function_pointer(typeinfo_pt typ)
{
    if(typ->type_kind == function_type)
    {
        /* C allows   void f1(void (f2)())  no (*f2), and it's a pointer */
        return true;
    }
    else if(is_function_pointer(typ))
    {
        symbol_t* tsym = typ->type_base;
        if(tsym)
        {
            if(tsym->sym_type)
            {
                return !tsym->sym_type->is_typedef;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

static int is_static(symbol_t* sym)
{
    typeinfo_t* typ;

    if(sym->is_static)
        return true;

    /* TBD 1/10/96: change in is_static may invalidate this */
    if(sym->sym_kind == func_symbol)
    {
        typ = sym->sym_type->type_next;
    }
    else
    {
        typ = sym->sym_type;
    }
    return typ->is_static;
}

static char* upper_array_bound(int elem, char* index_name)
{
    static char buf[100];

    if(elem < 0)
    {
        sprintf(buf, "%s'Last", index_name);
    }
    else
    {
        sprintf(buf, "%d", elem - 1);
    }
    return buf;
}


/*
 * generate the (...) part of an array declaration.
 * return the type of the array element
 */
static typeinfo_t* gen_dimensions(typeinfo_t* typ, enum array_gen_t a)
{
    int i;
    int ndim;
    int *dimensions;

    assert(typ != NULL);
    dimensions = get_dimensions(typ);
    ndim = dimensions[0];
    assert(ndim >= 1);

    if(ndim == 1 && dimensions[1] < 0 && a == init_var_array)
    {
        /* Then the array needs no explicit bounds;
         * presumably it's implicitly bounded by its initializer.
         */
    }
    else
    {
        put_string("(");
        for(i = 1; i <= ndim; i++)
        {
            char* index_name;
            if(ndim == 1 && dimensions[1] < 0 && typ->type_base
               && streq(typ->type_base->sym_ada_name, c_char_array))
            {
                index_name = c_char_array_index;
            }
            else
            {
                index_name = c_array_index;
            }

            switch(a)
            {
                case init_var_array:
                case var_array:
                    putf("0..%s", upper_array_bound(dimensions[i], index_name));
                    break;
                case bounds_array:
                    putf("%s range 0..%s", index_name,
                         upper_array_bound(dimensions[i], index_name));
                    break;
                case box_array:
                    putf("%s range <>", index_name);
                    break;
                default:
                    assert(0);
            }

            if(i < ndim)
                put_string(", ");
            assert((typ->type_kind == array_of) && (typ->type_next != NULL));
            typ = typ->type_next;
        }
        put_string(")");
    }

    free(dimensions);
    return typ;
}

static bool interfaces_c(symbol_t* sym); /* forward ref */

void gen_var_or_field(symbol_t* sym, /* symbol to gen */
                      int indent, /* indentation column */
                      int colonpos, /* decl colon column */
                      int import, /* unit# to import from */
                      char* rename, /* renaming string for func type */
                      int hidden) /* if true, prefix sym name
                                   * with unit name */
{
    typeinfo_t *typ, *elemtyp;
    int unit_ord;

    assert(sym != NULL);
    assert(sym->sym_ada_name != NULL);
    assert(sym->sym_type != NULL);

    typ = sym->sym_type;

    put_comment_block(sym->comment, indent);

    putf("%>%s", indent, sym->sym_ada_name);

    if(colonpos != 0)
        indent_to(colonpos);
    put_string(": ");

    if((ada_version >= 1995) && (typ->type_kind != field_type) && !rename
       && (typ->type_kind != union_of))
    {
        put_string("aliased ");
    }
    if((ada_version >= 1995) && is_anon_function_pointer(typ))
    {
        put_string(anon_function_pointer_name(sym));
        if(rename)
            put_string(rename);
    }
    else
    {
        if(hidden)
        {
            unit_ord = pos_unit(typ->type_base->sym_def);
            putf("%s.", unit_name(unit_ord));
        }
        put_string(type_nameof(typ, 0, 0));

        if(rename)
        {
            putf(" renames %s", rename);
        }
        else if(import != -1)
        {
            putf(" renames %s.%s", unit_name(import), sym->sym_ada_name);
        }
        else if((decl_class(typ) == array_decl) && !typ->type_base->sym_type->is_typedef)
        {
            enum array_gen_t kind;
            if(sym->has_initializer)
            {
                kind = init_var_array;
            }
            else
            {
                kind = var_array;
            }
            elemtyp = gen_dimensions(typ, kind);
        }
        else if(sym->renames)
        {
            put_string(" renames ");
            gen_expr(fix_initializer_expr(sym->sym_value.initializer, 0), false);
        }
    }
    if(!sym->is_struct_or_union_member
       && (sym->sym_scope < 2 || sym->is_static || sym->emit_initializer))
    {
        if(sym->has_initializer)
        {
            /* NB: dynamically allocated variables are initialized
             * with separate statements, since in general the
             * fixup may require the generation of auxiliary
             * statements.  Cf. fix_stmt_Compound.
             */

            put_string(" := ");
            if(sym->sym_type->type_kind == array_of)
            {
                /* indent array initialization under decl */
                putf("\n%>", indent + 4);
            }
            gen_initializer(sym->sym_type, sym->sym_value.initializer);
        }
        else if(!current_unit_is_header && !interfaces_c(sym))
        {
            /* output an initializer for statically allocated vars */

            if(sym->sym_type->type_kind != pointer_to)
            {
                /* Emit a "zero" of the appropriate type. */
                put_string(" := ");
                gen_zero(sym->sym_type);
            }
        }
    }
    put_char(';');
    c_comment_or_position(sym);

    if(comment_size)
    {
        comment_sizeof(typ->type_sizeof, typ->type_alignof);
    }

    if((sym->sym_scope >= 2) && (typ->is_extern))
    {
        interface_c(sym, indent);
    }
    if(!rename)
    {
        /* Current implementation of renaming just reuses named symbol,
         * which may not itself have been emitted.
         */
        set_symbol_done(sym);
    }

} /* gen_var_or_field */

static void gen_simple_type(symbol_t* sym)
{
    typeinfo_t* typ;

    output_to(sym->is_declared_in_header);
    put_comment_block(sym->comment, 4);
    typ = sym->sym_type;

    switch(decl_class(typ))
    {
        case int_decl:
            gen_int_type(sym);
            break;
        case enum_decl:
            if(typ->type_base == sym)
            {
                new_line();
                gen_enum_type(sym);
            }
            else
            {
                subtype_decl(sym->sym_ada_name, NULL, sym->sym_type->type_base->sym_ada_name,
                             4, sym->sym_ident, sym->sym_def);
                set_symbol_done(sym);
            }
            break;
        case fp_decl:
            gen_fp_type(sym);
            break;
        default:
            break;
    }
}

static void gen_simple_types(sym_q* typeq)
{
    symbol_t* sym;

    if(typeq->qhead != NULL)
    {
        new_line();
    }

    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        gen_simple_type(sym);
    }
}

static void import_subtype(sym_q* typeq)
{
    symbol_t* sym;
    unit_n unit;

    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        unit = pos_unit(sym->sym_def);
        subtype_decl(sym->sym_ada_name, unit_name(unit), sym->sym_ada_name, 4,
                     sym->sym_ident, sym->sym_def);
    }
}

static int any_type_decls(int uord)
{
    return compilation[uord].simple_typeq.qhead != NULL
           || compilation[uord].simple_ptr_typeq.qhead != NULL
           || compilation[uord].simple_array_typeq.qhead != NULL
           || compilation[uord].rec_ptr_typeq.qhead != NULL
           || compilation[uord].sort_typeq.qhead != NULL;
}

static void import_types()
{
    int uord;
    int i;

    if(!should_import())
        return;

    new_line();

    for(i = 0;; i++)
    {
        uord = nth_direct_ref_unit_ord(i);
        if(uord == -1)
            break;
        if(any_type_decls(uord))
        {
            new_line();
            indent_to(4);
            put_string("-- imported subtypes from ");
            put_string(unit_name(uord));
            new_line();

            import_subtype(&compilation[uord].simple_typeq);
            import_subtype(&compilation[uord].simple_ptr_typeq);
            import_subtype(&compilation[uord].simple_array_typeq);
            import_subtype(&compilation[uord].rec_ptr_typeq);
            import_subtype(&compilation[uord].sort_typeq);
        }
    }
}

static int single_void(symbol_t* param)
{
    typeinfo_t* typ;

    if(param->sym_parse_list)
        return 0;

    typ = param->sym_type;
    assert(typ != NULL);

    return typ->type_kind == void_type;
}

static int has_void_params(symbol_t* param)
{
    return (param == NULL || single_void(param));
}

static void gen_function_pointer(char* name, symbol_t* sym, typeinfo_t* typ)
{
    int indent;
    typeinfo_pt func_return;
    symbol_t* typebase = typ->type_base;

    if(sym->emitted)
        return;

    if(ada_version < 1995)
    {
        putf("subtype %s is %s", sym->sym_ada_name, c_function_pointer);
    }
    else if(typebase && typebase->emitted)
    {
        putf("subtype %s is %s", name, typebase->sym_ada_name);
    }
    else if(typebase && !typebase->emitted && !streq(name, typebase->sym_ada_name))
    {
        gen_access_type(typebase, false);
        putf("subtype %s is %s", name, typebase->sym_ada_name);
    }
    else
    {
        symbol_t* params = typ->type_next->type_info.formals;

        putf("type %s is access ", name);

        if(typ->type_kind == pointer_to)
        {
            func_return = typ->type_next->type_next;
        }
        else
        {
            func_return = typ->type_next;
        }

        if(func_return->type_kind == void_type)
        {
            put_string("procedure");
            indent = gen_params(params, 1, NULL);
        }
        else
        {
            put_string("function");
            indent = gen_params(params, 1, NULL);

            if(!has_void_params(params))
            {
                new_line();
                indent_to(14);
            }
            putf(" return %s", type_nameof(func_return, 0, 0));
        }
        /* if (typebase) set_symbol_done(typebase); */
    }

    if(sym->sym_kind == type_symbol)
    {
        set_symbol_done(sym);
    }
}

void gen_access_type(symbol_t* sym, bool private_part)
{
    typeinfo_pt typ;
    bool is_fp;

    assert(sym != NULL);
    if(sym_done(sym))
        return;
    if(private_part && !sym->isprivate)
        return;

    output_to(sym->is_declared_in_header);

    if(sym->isprivate && !private_part)
    {
        symbol_t* nullsym = private_type_null(sym);

        putf("%>type %s is private;\n", 4, sym->sym_ada_name);
        putf("%>%s : constant %s;\n", 4, nullsym->sym_ada_name, sym->sym_ada_name);
        return;
    }

    typ = sym->sym_type;
    assert(typ != NULL);

    if(typ->type_kind == function_type)
        return;

    assert(typ->type_kind == pointer_to);
    assert(typ->type_next != NULL);

    indent_to(4);

    is_fp = is_function_pointer(typ);
    if(is_fp)
    {
        gen_function_pointer(sym->sym_ada_name, sym, typ);
    }
    else
    {
        put_string("type ");
        put_string(sym->sym_ada_name);
        if(typ->type_next->type_kind == void_type)
        {
            put_string(" is new System.Address");
        }
        else
        {
            put_string(" is access ");
            if(ada_version >= 1995)
            {
                if(typ->type_next->is_constant)
                    put_string("constant ");
                else
                    put_string("all ");
            }
            put_string(type_nameof(typ->type_next, 1, 0));
        }
        set_symbol_done(sym);
    }

    put_char(';');
    c_comment(sym->sym_ident);
    print_position(sym->sym_def);

    if(comment_size)
    {
        comment_sizeof(typ->type_sizeof, typ->type_alignof);
    }
    if(is_fp)
    {
        put_char('\n');
        putf("%>pragma Convention(C, %s);\n\n", 4, sym->sym_ada_name);
    }
    if(sym->isprivate)
    {
        symbol_t* nullsym = private_type_null(sym);
        putf("%>%s : constant %s := null;\n", 4, nullsym->sym_ada_name, sym->sym_ada_name);
    }
}

static void gen_access_types(sym_q* typeq, bool private_part)
{
    symbol_t* sym;

    if(typeq->qhead != NULL)
        new_line();

    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        output_to(sym->is_declared_in_header);
        gen_access_type(sym, private_part);
    }
}

static bool sym_for_incomplete_record(symbol_t* sym)
/*
 * The function tests whether an incomplete type declaration
 * for the given type should be emitted.  Aside from making
 * the obvious test (sym->sym_tags) as to the completeness of the
 * type declaration, this function performs a (mysterious) test on
 * a complete type to determine whether an incomplete declaration
 * should be emitted anyway. A known circumstance where this makes
 * a difference is when the module also contains a pointer-to-this-record
 * type.
 */
{
    typeinfo_pt type;

    type = sym->sym_type;
    assert(type);
    if(decl_class(type) != struct_decl)
        return false;

    if(sym->aliases)
        return false;

    if(!sym->sym_tags)
        return true;

    /* Here's the aforementioned mystery test. */
    /* TBD: Why is this the right test?? Is it? */
    /*
     * Basically what we want to know here is whether this module
     * is generating any pointers to this type.  Because of the
     * type reordering going on in gen_unit -- and in the gen module
     * generally -- we're going to emit (1) incomplete record types
     * (2) access to record types (3) complete record types.
     * The code here seems to work -- why, I (rgh) don't know.
     */
    return !type->type_base || type->type_base == sym;
}

static void gen_record_incompletes(sym_q* typeq, bool private_part)
{
    symbol_t* sym;
    typeinfo_pt typ;

    if(typeq->qhead != NULL)
    {
        new_line();
    }

    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        if(sym->isprivate == private_part && sym_for_incomplete_record(sym))
        {
            typ = sym->sym_type;

            output_to(sym->is_declared_in_header);
            putf("%>type %s;", 4, sym->sym_ada_name);
            print_position(sym->sym_def);

            if(comment_size)
            {
                comment_sizeof(typ->type_sizeof, typ->type_alignof);
            }
            set_symbol_done(sym);
        }
    }
} /* gen_record_incompletes() */

static int q_max_lhs_name_len(sym_q* q)
{
    symbol_t* sym;
    int max = 0;
    int len;

    for(sym = q->qhead; sym; sym = sym->sym_gen_list)
    {
        assert(sym->sym_ada_name != NULL);
        len = strlen(sym->sym_ada_name);
        if(len > max)
            max = len;
    }

    return max;
}

static int max_lhs_name_len(symbol_t* sym)
{
    int max, len;

    for(max = 0; sym; sym = sym->sym_parse_list)
    {
        assert(sym->sym_ada_name != NULL);
        len = strlen(sym->sym_ada_name);
        if(len > max)
            max = len;
    }

    return max;
}

static int has_bitfields(symbol_t* tags)
{
    typeinfo_t* typ;

    assert(tags != NULL);

    for(; tags; tags = tags->sym_parse_list)
    {
        typ = tags->sym_type;
        if(typ->type_kind == field_type)
        {
            return 1;
        }
    }

    return 0;
}

static int bit_sizeof(typeinfo_t* typ)
{
    assert(typ != NULL);
    return (typ->type_kind == field_type) ? typ->type_sizeof : typ->type_sizeof * BITS_PER_BYTE;
}

static void gen_record_rep(symbol_t* sym, int largest_lhs)
{
    symbol_t* tag;
    typeinfo_t* typ;
    int is_union;

    if(suppress_record_repspec)
    {
        return;
    }

    assert(sym != NULL);

    typ = sym->sym_type;
    assert(typ != NULL);
    tag = sym->sym_tags;
    assert(tag != NULL);

    is_union = typ->type_kind == union_of;

    if(repspec_flag || is_union || has_bitfields(sym->sym_tags))
    {
        new_line();

        if(flag_unions && is_union)
        {
            mark_union(sym);
        }

        indent_to(4);
        put_string("for ");
        put_string(sym->sym_ada_name);
        put_string(" use");
        print_position(sym->sym_def);
        indent_to(8);
        put_string("record at mod ");
        print_value(typ->type_alignof, 10);
        put_string(";\n");

        for(; tag; tag = tag->sym_parse_list)
        {
            typ = tag->sym_type;
            assert(typ != NULL);

            indent_to(12);
            put_string(tag->sym_ada_name);
            indent_to(largest_lhs + 12);
            put_string(" at 0 range ");
            print_value(tag->bitoffset, 10);
            put_string(" .. ");
            print_value(tag->bitoffset + bit_sizeof(typ) - 1, 10);
            put_char(';');
            print_position(tag->sym_def);
        }

        indent_to(8);
        put_string("end record;\n");
    }
    else if(ada_version >= 1995)
    {
        new_line();
        indent_to(4);
        put_string("pragma Convention(C,  ");
        put_string(sym->sym_ada_name);
        put_string(");");
        print_position(sym->sym_def);
    }
}

static int is_hidden_type_name_in_rec(symbol_t* rec_sym, char* type_name)
{
    symbol_t* tag;

    for(tag = rec_sym->sym_tags; tag; tag = tag->sym_parse_list)
    {
        if(!strcasecmp(type_name, tag->sym_ada_name))
            return 1;
    }
    return 0;
}

static void gen_record_t(symbol_t* sym, bool private_part)
{
    symbol_t* tag;
    symbol_t* biggest_tag;
    int biggest_tag_size;
    typeinfo_pt typ;
    int largest_lhs;
    int has_comment;
    int is_gnat_union;

    assert(sym != NULL);

    if(private_part)
    {
        /* TBD */
        return;
    }
    if(sym->is_created_by_reference && !sym->sym_tags)
        return;
    if(sym->aliases)
        return;

    typ = sym->sym_type;
    assert(typ != NULL);
    is_gnat_union = (ada_compiler == GNAT) && (typ->type_kind == union_of);

    new_line();

    has_comment = valid_comment(sym->sym_ident);

    if(typ->type_base != NULL && typ->type_base != sym)
    {
        subtype_decl(sym->sym_ada_name, NULL, typ->type_base->sym_ada_name, 4,
                     sym->sym_ident, sym->sym_def);
        set_symbol_done(sym);
        if(has_comment)
            new_line();
    }
    else
    {
        indent_to(4);
        if(is_gnat_union)
        {
            tag = sym->sym_tags;
            if(tag != NULL)
            {
                putf("type %s_kind is(", sym->sym_ada_name);
                if(has_comment)
                {
                    c_comment(sym->sym_ident);
                }
                else
                {
                    print_position(sym->sym_def);
                }
                for(; tag; tag = tag->sym_parse_list)
                {
                    putf("%>%s_kind", 8, tag->sym_ada_name);
                    if(tag->sym_parse_list != NULL)
                        put_string(",");
                    new_line();
                }
                putf("%>);\n\n", 4);
                indent_to(4);
            }
        }
        putf("type %s", sym->sym_ada_name);
        if(is_gnat_union)
        {
            /* default to first if all sizes are 0,
             * it seems like sizes can be 0 for subtypes
             */
            biggest_tag = sym->sym_tags;
            biggest_tag_size = 0;
            for(tag = sym->sym_tags; tag; tag = tag->sym_parse_list)
            {
                if(biggest_tag_size < tag->sym_type->type_sizeof)
                {
                    biggest_tag_size = tag->sym_type->type_sizeof;
                    biggest_tag = tag;
                }
            }
            putf(" (Which: %s_kind", sym->sym_ada_name);
            if(biggest_tag != NULL)
            {
                putf(" := %s_kind", biggest_tag->sym_ada_name);
            }
            put_string(")");
        }
        put_string(" is");
        if(has_comment)
        {
            c_comment(sym->sym_ident);
        }
        else
        {
            print_position(sym->sym_def);
        }

        indent_to(8);
        put_string("record");
        if(has_comment)
        {
            print_position(sym->sym_def);
        }
        else if(comment_size)
        {
            comment_sizeof(typ->type_sizeof, typ->type_alignof);
        }
        else
        {
            new_line();
        }

        tag = sym->sym_tags;

        if(tag == NULL)
        {
            indent_to(12);
            put_string("null;\n");
            indent_to(8);
            put_string("end record;\n");
        }
        else
        {
            largest_lhs = max_lhs_name_len(tag);

            if(is_gnat_union)
            {
                putf("%>case Which is\n", 12);
                for(; tag; tag = tag->sym_parse_list)
                {
                    putf("%>when %s_kind =>\n", 16, tag->sym_ada_name);
                    gen_var_or_field(tag, 20, largest_lhs + 20, -1, NULL,
                                     is_hidden_type_name_in_rec(
                                     sym, type_nameof(tag->sym_type, 0, 0)));
                }
                putf("%>end case;\n", 12);
                putf("%>end record;\n\n", 8);

                putf("%>pragma Convention(C, %s);\n", 4, sym->sym_ada_name);
                putf("%>pragma Unchecked_Union(%s);", 4, sym->sym_ada_name);
            }
            else
            {
                for(; tag; tag = tag->sym_parse_list)
                {
                    gen_var_or_field(tag, 12, largest_lhs + 12, -1, NULL,
                                     is_hidden_type_name_in_rec(
                                     sym, type_nameof(tag->sym_type, 0, 0)));
                }
                indent_to(8);
                put_string("end record;");
            }

            if(has_comment && comment_size)
            {
                comment_sizeof(typ->type_sizeof, typ->type_alignof);
            }
            else
            {
                new_line();
            }

            if(!is_gnat_union)
                gen_record_rep(sym, largest_lhs);
            set_symbol_done(sym);
        }
    }
}

static void gen_array_t(symbol_t* sym, bool private_part)
{
    typeinfo_t *typ, *elemtyp;
    int has_comment;

    assert(sym != NULL);

    if(private_part)
        return;

    typ = sym->sym_type;
    assert((typ != NULL) && (typ->type_kind == array_of));

    has_comment = valid_comment(sym->sym_ident);

    output_to(sym->is_declared_in_header);
    new_line();
    indent_to(4);
    putf("type %s is", sym->sym_ada_name);
    print_position(sym->sym_def);
    indent_to(8);
    put_string("array");
    if(typ->is_typedef)
        elemtyp = gen_dimensions(typ, bounds_array);
    else
        elemtyp = gen_dimensions(typ, box_array);
    if(comment_size)
    {
        comment_sizeof(typ->type_sizeof, typ->type_alignof);
    }
    else
    {
        new_line();
    }
    indent_to(8);
    put_string("of ");
    if((ada_version >= 1995) && (typ->type_next->type_kind != union_of))
        put_string("aliased ");
    put_string(type_nameof(elemtyp, 0, 0));
    put_char(';');
    if(has_comment)
    {
        c_comment(sym->sym_ident);
    }
    else
    {
        new_line();
    }
    set_symbol_done(sym);
}

static void gen_array_types(sym_q* typeq)
{
    symbol_t* sym;

    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        gen_array_t(sym, false);
    }
}

static void start_anon_funcs()
{
    if(!first_anon_func)
    {
        new_line();
        first_anon_func = 1;
    }
}

void gen_any_func_ptr_type(symbol_t* sym)
/* Generate any function pointer types required to declare symbol */
{
    char* fname;
    int indent = 4;

    if(sym->emitted)
        return;
    if(is_anon_function_pointer(sym->sym_type))
    {
        gen_tag_types(sym->sym_tags, 1);
        start_anon_funcs();
        indent_to(indent);
        fname = anon_function_pointer_name(sym);
        gen_function_pointer(fname, sym, sym->sym_type);
        put_char(';');
        c_comment_or_position(sym);

        putf("%>pragma Convention(C, %s);\n\n", indent, fname);
    }
}

static void gen_sorted_types(sym_q* typeq, bool isprivate)
{
    symbol_t* sym;
    symbol_t* tag;
    typeinfo_pt typ;
    decl_class_t prev;
    decl_class_t cur;

    /*
     * Make 2 passes through the types.
     * On the first pass, generate extra types for anonymous
     * function pointers that go with record fields.
     * Then on the 2nd pass generate the actual types.
     */
    if(!isprivate && ada_version >= 1995)
    {
        first_anon_func = 0;
        for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
        {
            output_to(sym->is_declared_in_header);
            typ = sym->sym_type;
            assert(typ != NULL);

            if(decl_class(typ) == struct_decl)
            {
                for(tag = sym->sym_tags; tag; tag = tag->sym_parse_list)
                {
                    if(is_anon_function_pointer(tag->sym_type) && !tag->gened)
                    {
                        gen_any_func_ptr_type(tag);
                        tag->gened = 1;
                    }
                }
            }
        }
    }

    /* 2nd pass: now we're actually emitting the type decls */
    prev = struct_decl;
    for(sym = typeq->qhead; sym; sym = sym->sym_gen_list)
    {
        typ = sym->sym_type;
        assert(typ != NULL);

        output_to(sym->is_declared_in_header);

        cur = decl_class(typ);
        switch(cur)
        {
            case func_decl:
            case pointer_decl:
                cur = pointer_decl;
                if(prev != cur)
                {
                    new_line();
                }
                gen_access_type(sym, isprivate);
                break;
            case array_decl:
                gen_array_t(sym, isprivate);
                break;
            case struct_decl:
                gen_record_t(sym, isprivate);
                break;
            default:
                assert(0);
                break;
        }

        prev = cur;
    }
}

static int aggs_passed_by_ref(void)
{
    static int initialized = 0;
    static int result;

    if(initialized)
        return result;

    switch(ada_compiler)
    {
        case Rational:
            result = 1;
            break;
        default:
            result = 0;
            break;
    }

    initialized = 1;
    return result;
}

static int access_to_agg(typeinfo_t* typ)
{
    assert(typ != NULL);

    if(typ->type_kind == pointer_to)
    {
        typ = typ->type_next;
        assert(typ != NULL);

        switch(typ->type_kind)
        {
            case array_of:
            case struct_of:
            case union_of:
                return 1;
                break;
            default:
                break;
        }
    }

    return 0;
}

static bool is_charp(typeinfo_pt typ)
{
    /* return (typ && strcmp(type_nameof(typ), "c.charp") == 0); */

    return same_ada_type(typ, type_charp());
}

static bool is_const_charp(typeinfo_pt typ)
{
    return same_ada_type(typ, type_const_charp());
}

static char* param_type_name(typeinfo_t* typ, int pass)
{
    static char* in_out_string;

    if(!in_out_string)
        in_out_string = new_strf("in out %s", string_name(0));

    if(is_const_charp(typ))
    {
        return (pass >= 2) ? string_name(0) : c_const_char_ptr;
    }
    else if(is_charp(typ))
    {
        return (pass >= 2) ? in_out_string : c_char_ptr;
    }
    else
    {
        return type_nameof(typ, false, true);
    }
}

static int gen_params(symbol_t* params, int pass, char* result_to_add)
{
    symbol_t* sym;
    int largest_lhs;
    int lhs_pos;
    typeinfo_t* typ;
    int has_const_charp, has_charp;
    int first_use = 1;

    static char* IC = "Interfaces.C";

    /*
     * This can be called with 4 "passes".
     * Pass 1 generates the parameter names and types.
     * Pass 2 generates the parameter names and but substitutes
     *        char_array and "in out char_array" for certain types.
     * Pass 3 generates tmp_param: constant char_array := param&nul
     *        declarations for in string parameters.
     * Pass 4 generates just a list of the parameter names,
     *        not the type names, e.g. (param1, param2(param2'first)'access).
     */
    if(has_void_params(params))
    {
        return cur_indent();
    }

    if(pass != 3)
        put_char('(');

    /* lhs_pos = cur_indent(); */
    lhs_pos = 16;
    new_line();
    largest_lhs = max_lhs_name_len(params);
    if((largest_lhs < 6) && result_to_add)
        largest_lhs = 6;

    /*
     * change to this format:
     *
     * function foo (
     *		p1: t1;
     *		...)
     *            return t2;
     *
     * because lines are getting too long.
     */
    for(sym = params; sym; sym = sym->sym_parse_list)
    {
        typ = sym->sym_type;
        switch(pass)
        {
            case 1:
            case 2:
                indent_to(lhs_pos);
                put_string(sym->sym_ada_name);
                indent_to(largest_lhs + lhs_pos);
                put_string(": ");
                if(sym == ellipsis_sym)
                {
                    put_string("Stdarg.ArgList := Stdarg.Empty");
                }
                else
                {
                    if((ada_version >= 1995) && is_anon_function_pointer(typ))
                    {
                        put_string(anon_function_pointer_name(sym));
                    }
                    else if(aggs_passed_by_ref() && access_to_agg(typ))
                    {
                        put_string(type_nameof(sym->sym_type->type_next, 0, 1));
                    }
                    else
                    {
                        put_string(param_type_name(typ, pass));
                    }

                    if(sym->sym_parse_list != NULL)
                    {
                        put_string(";\n");
                    }
                }
                break;
            case 3:
                if(is_const_charp(typ))
                {
                    if(first_use)
                    {
                        /* put_string("use type Interfaces.C.Char_Array;\n"); */
                        putf("%>function \"&\" (S: %s.Char_Array; C: %s.Char)", 8, IC, IC);
                        putf("%>return %s.Char_Array renames %s.\"&\";", 12, IC, IC);
                        if(!cur_unit_is_child_of_predef() && (ada_version >= 1995))
                        {
                            putf("%>Nul: %s.Char renames %s.Nul;\n\n", 8,
                                 predef_pkg, predef_pkg);
                        }
                        first_use = 0;
                    }
                    putf("%>Tmp_%s: constant %s := \n", 8, sym->sym_ada_name,
                         string_name(0));
                    putf("%>%s & %s;\n", 12, sym->sym_ada_name, nul_name(0));
                }
                break;
            case 4:
                indent_to(lhs_pos);
                has_charp = is_charp(typ);
                has_const_charp = is_const_charp(typ);
                putf(has_const_charp ? "Tmp_%s" : "%s", sym->sym_ada_name);
                if(has_charp || has_const_charp)
                {
                    putf("(%s%s'First)'unchecked_access",
                         has_const_charp ? "Tmp_" : "", sym->sym_ada_name);
                }
                if(sym->sym_parse_list != NULL)
                    put_string(",\n");
                break;
        }
        set_symbol_done(sym);
    } /* for */

    if(result_to_add)
    {
        put_string(";\n");
        indent_to(lhs_pos);
        put_string("result");
        indent_to(largest_lhs + lhs_pos);
        put_string(": out ");
        put_string(result_to_add);
    }

    if(pass != 3)
        put_char(')');

    if(params->sym_parse_list)
    {
        return largest_lhs + lhs_pos + 1;
    }

    return cur_indent();
}

static int multiple_params(symbol_t* params)
{
    return params != NULL && params->sym_parse_list != NULL;
}

static bool declaring_sym_in_spec(symbol_t* sym)
{
    if(sym->is_declared_in_header)
        return true;
    if(sym->sym_scope >= 2)
        return false;
    if(is_static(sym))
        return false;
    if(sym->declare_in_spec)
        return true;
    if(export_from_c)
        return true;
    return false;
}

static void gen_vars(sym_q* vq, int import)
{
    symbol_t* sym;
    int largest_lhs;
    typeinfo_pt typ;

    if(vq->qhead == NULL)
        return;

    largest_lhs = q_max_lhs_name_len(vq);

    new_line();

    /*
     * Make 2 passes through the variables, like the types.
     * On the first pass, generate extra types for anonymous
     * function pointers.
     */
    if(ada_version >= 1995)
    {
        first_anon_func = 0;
        for(sym = vq->qhead; sym; sym = sym->sym_gen_list)
        {
            if(!sym->emitted)
            {
                typ = sym->sym_type;
                assert(typ != NULL);

                if(is_anon_function_pointer(typ))
                {
                    output_to(declaring_sym_in_spec(sym));
                    gen_any_func_ptr_type(sym);
                }
            }
        }
    }

    /*
     * Then on the 2nd pass generate the actual variables.
     */
    for(sym = vq->qhead; sym; sym = sym->sym_gen_list)
    {
        if((sym->sym_scope < 2) || sym->is_static)
        {
            if(sym_done(sym))
                continue;
            if(sym->sym_type->type_kind == array_of &&
               /* TBD:only handling this special case now */
               has_undone_requisites(sym))
            {
                postpone_doing(sym);
                continue;
            }
            output_to(declaring_sym_in_spec(sym));
            /* Don't produce unit-level declarations for local vars */
            gen_var_or_field(sym, 4, largest_lhs + 4, import, NULL, 0);
        }
    }
} /* gen_vars */

static void import_vars(void)
{
    int uord;
    int i;

    if(!should_import())
        return;

    for(i = 0;; i++)
    {
        uord = nth_direct_ref_unit_ord(i);
        if(uord == -1)
            break;
        if(compilation[uord].varq.qhead != NULL)
        {
            putf("\n%>-- imported vars from %s\n", 4, unit_name(uord));
            gen_vars(&compilation[uord].varq, uord);
        }
    }
}

void gen_local_func(symbol_t* sym, int indent)
/*
 * A local function is a function within the scope of another.
 * This construct, which cannot appear in the C source, is
 * synthesized to handle C constructs that otherwise aren't
 * expressible in Ada.  See fix_stmt.c .
 */
/* TBD: For now we assume parameterless functions. */
{
    typeinfo_pt rtyp;
    char* return_name;

    put_comment_block(sym->comment, indent);

    rtyp = return_type(sym);

    putf("%>function %s", indent, sym->sym_ada_name);
    if(!has_void_params(sym->sym_tags))
    {
        /* TBD: error message */
    }

    return_name = new_string(type_nameof(rtyp, false, false));
    putf(" return %s", return_name);

    put_string(" is");
    c_comment(sym->sym_ident);
    print_position(sym->sym_def);
    gen_funcdef(sym, indent);

    inline_func(sym, indent);
    put_string("\n");
    set_symbol_done(sym);
}

void gen_subp(symbol_t* sym, char* rename, bool is_spec, bool is_inline, bool is_interfaced)
{
    int indent;
    symbol_t* param;
    typeinfo_t *typ, *rtyp;
    int has_const_charp = 0;
    int has_charp = 0;
    char* return_name;
    int is_func;
    int changing_func_to_proc = 0;

    /*
     * Make 2 passes through the parameters, like the types.
     * On the first pass, generate extra types for anonymous
     * function pointers.
     * Then on the 2nd pass generate the actual function and parameters.
     */
    if(ada_version >= 1995 && !rename)
    {
        first_anon_func = 0;
        for(param = sym->sym_tags; param; param = param->sym_parse_list)
        {
            if(param == ellipsis_sym)
                break;
            typ = param->sym_type;
            assert(typ != NULL);

            if(is_anon_function_pointer(typ))
            {
                gen_any_func_ptr_type(param);
            }
            else if(is_const_charp(typ))
            {
                has_const_charp = 1;
            }
            else if(is_charp(typ))
            {
                has_charp = 1;
            }
        }
    }

    if(rename != NULL)
    {
        output_to(is_spec);
    }

    put_comment_block(sym->comment, 4);
    put_string("\n");
    indent_to(4);

    is_func = is_function(sym);
    if(is_func)
    {
        rtyp = return_type(sym);
        if(rtyp->is_anon_int)
        {
            warning_at(sym->sym_def, "%s had no type in C, assume function returning int",
                       sym->sym_ada_name);
        }
        putf("function %s", sym->sym_ada_name);
        indent = gen_params(sym->sym_tags, 1, NULL);

        if(!has_void_params(sym->sym_tags))
        {
            new_line();
            indent_to(14);
        }
        return_name = new_string(type_nameof(rtyp, 0, 0));
        putf(" return %s", return_name);
    }
    else
    {
        putf("procedure %s", sym->sym_ada_name);
        indent = gen_params(sym->sym_tags, 1, NULL);
        return_name = NULL;
    }
    set_symbol_done(sym);

    if(rename == NULL)
    {
        put_string(is_spec ? ";" : " is ");
        c_comment(sym->sym_ident);
        print_position(sym->sym_def);
        if(is_spec)
        {
            new_line();
            if(is_inline)
            {
                inline_func(sym, 4);
            }
            else if(is_interfaced && (has_charp || has_const_charp))
            {
                interface_c(sym, 4);
            }
        }
        else
        {
            gen_funcdef(sym, 4);
            print_position(sym->sym_def);
        }
        if(!is_inline && (is_spec == declaring_sym_in_spec(sym)))
        {
            bool was_to_spec = output_is_spec();
            output_to(is_spec);
            putf("%>pragma Convention(C, %s);", 4, sym->sym_ada_name);
            output_to(was_to_spec);
        }
        new_line();
    }
    else
    {
        putf("%s;", rename);
        c_comment(sym->sym_ident);
        print_position(sym->sym_def);
        new_line();
    }

    if((ada_version >= 1995) && (has_charp || has_const_charp) && is_spec)
    {
        /*
         * If the function has a const charp parameter, generate:
         *    int f(const char *param);
         *    function f (param: const_charp) return int;
         *    pragma import(c, f, "f");
         *    procedure f (param: in out char_array; result: out int);
         *    pragma inline(f);
         *    procedure f (param: in out char_array; result: out int) is
         *    begin
         *        result := f(param(param'first)'access);
         *    end f;
         * and if it has a charp, generate:
         *    int f(char *param);
         *    function f (param: charp) return int;
         *    pragma import(c, f, "f");
         *    function f (param: char_array) return int;
         *    pragma inline(f);
         *    function f (param: char_array) return int is
         *        tmp_param: char_array := param & interfaces.c.nul;
         *    begin
         *        return f(tmp_param(tmp_param'first)'access);
         *    end f;
         */
        if(has_charp)
        {
            if(is_func)
                changing_func_to_proc = 1;
            is_func = 0;
        }
        if(sym->is_declared_in_header)
        {
            indent_to(4);
            if(is_func)
            {
                putf("function %s", sym->sym_ada_name);
                indent = gen_params(sym->sym_tags, 2, NULL);
                new_line();
                putf("%> return %s", 14, return_name);
            }
            else
            {
                putf("procedure %s", sym->sym_ada_name);
                indent = gen_params(sym->sym_tags, 2, return_name);
            }
            put_char(';');
            set_symbol_done(sym);
            c_comment(sym->sym_ident);
            print_position(sym->sym_def);
            new_line();
            inline_func(sym, 4);
            put_char('\n');
        }

        output_to_body();
        put_comment_block(sym->comment, 4);

        if(is_func)
        {
            putf("\n%>function %s", 4, sym->sym_ada_name);
            indent = gen_params(sym->sym_tags, 2, NULL);
            putf(" return %s is\n", return_name);
            indent = gen_params(sym->sym_tags, 3, NULL);
            putf("%>begin\n", 4);
            putf("%>return %s", 8, sym->sym_ada_name);
            indent = gen_params(sym->sym_tags, 4, NULL);
            put_string(";\n");
        }
        else
        {
            putf("%>procedure %s", 4, sym->sym_ada_name);
            indent = gen_params(sym->sym_tags, 2, return_name);
            put_string(" is\n");
            indent = gen_params(sym->sym_tags, 3, NULL);
            putf("%>begin\n", 4);

            indent_to(8);
            if(changing_func_to_proc)
                put_string("result := ");
            put_string(sym->sym_ada_name);
            indent = gen_params(sym->sym_tags, 4, NULL);
            put_string(";\n");
        }
        putf("%>end %s;\n\n", 4, sym->sym_ada_name);

        if(!sym->is_declared_in_header)
        {
            inline_func(sym, 4);
        }

        sym->interfaced = 1;
    }
}

static void gen_subps(sym_q* fq, int import)
/* NB: import argument unused */
{
    symbol_t* sym;
    typeinfo_t* t;

    for(sym = fq->qhead; sym; sym = sym->sym_gen_list)
    {
        if(sym->has_initializer)
        {
            /* it's a function definition, with body */

            t = sym->sym_type->type_next;
            if(declaring_sym_in_spec(sym))
            {
                output_to_spec();
                gen_subp(sym, NULL, 1, sym->is_inline, 0);
            }
            output_to_body();
            gen_subp(sym, NULL, 0, sym->is_inline, 0);
        }
        else
        {
            /* it's a function declaration, no body */
            output_to(sym->is_declared_in_header);
            if(!sym->sym_tags)
            {
                int pos = sym->sym_def;
                warning(file_name(pos), line_number(pos),
                        "function %s declared without params; "
                        "assuming no params",
                        sym->sym_ada_name);
            }

            gen_subp(sym, NULL, 1, 0, 1);
        }
    }
    output_to_spec();
}

static void import_subprograms()
{
    int uord;
    int i;

    if(!should_import())
        return;

    for(i = 0;; i++)
    {
        uord = nth_direct_ref_unit_ord(i);
        if(uord == -1)
            break;
        if(compilation[uord].funcq.qhead != NULL)
        {
            new_line();
            indent_to(4);
            put_string("-- imported subprograms from ");
            put_string(unit_name(uord));
            new_line();
            gen_subps(&compilation[uord].funcq, uord);
        }
    }
}

static void rational_parameter_mechanism(symbol_t* params)
{
    if(has_void_params(params))
        return;

    put_string(", mechanism => (");
    for(; params; params = params->sym_parse_list)
    {
        if(access_to_agg(params->sym_type))
        {
            put_string("reference");
        }
        else
        {
            put_string("value");
        }
        if(params->sym_parse_list != NULL)
        {
            put_string(", ");
        }
    }

    put_char(')');
}

static void rational_subp_interface_pragma(symbol_t* subp)
{
    indent_to(4);

    put_string("pragma import_");
    put_string(is_function(subp) ? "function(" : "procedure(");
    put_string(subp->sym_ada_name);
    put_string(", \"");
    put_string(C_SUBP_PREFIX);
    assert(subp->sym_ident != NULL);
    assert(subp->sym_ident->node_kind == _Ident);
    put_string(subp->sym_ident->node.id.name);
    put_char('\"');
    rational_parameter_mechanism(subp->sym_tags);
    put_string(");\n");
}

static bool interfaces_c(symbol_t* sym)
{
    typeinfo_t* typ;

    assert(sym != NULL);
    assert(sym->sym_ada_name != NULL);

    if(sym->has_initializer)
        return false;

    /* see extern.doc for explanation.  complicated. */
    if((sym->sym_kind == func_symbol)
       && (sym->has_initializer || sym->sym_type->type_next->is_inline))
        return false;

    if(sym->sym_kind == var_symbol)
    {
        if(sym->is_static)
            return false;
        typ = sym->sym_type;
        if((typ->type_kind == array_of) || (typ->type_kind == pointer_to))
        {
            assert(typ->type_next != NULL);
            typ = typ->type_next;
        }
        if((!sym->is_declared_in_header) && (!typ->is_extern))
            return false;
    }
    return true;
}

void interface_c(symbol_t* sym, int indent)
{
    if(!interfaces_c(sym))
        return;

    if(ada_version >= 1995)
    {
        assert(sym->sym_ident != NULL);
        assert(sym->sym_ident->node_kind == _Ident);
        assert(sym->sym_ident->node.id.name != NULL);

        putf("%>pragma Import(C, %s, \"%s\");", indent, sym->sym_ada_name,
             sym->sym_ident->node.id.name);
        print_position(sym->sym_def);
    }
    else
    {
        putf("\n%>pragma Interface(C, %s);", indent, sym->sym_ada_name);
        print_position(sym->sym_def);
    }
}

static void gen_var_interface_pragmas(sym_q* vq)
{
    symbol_t* sym;
    int indent = 4;

    for(sym = vq->qhead; sym; sym = sym->sym_gen_list)
    {
        if((!sym->interfaced) && (sym->sym_scope < 2))
        {
            output_to(declaring_sym_in_spec(sym));
            switch(ada_compiler)
            {
                case GNAT:

                    if(interfaces_c(sym))
                    {
                        interface_c(sym, indent);
                    }
                    else
                    {
                        putf("%>pragma Export(C, %s, \"%s\");", indent,
                             sym->sym_ada_name, sym->sym_ident->node.id.name);
                        print_position(sym->sym_def);
                    }
                    break;

                /* NB: cases other than GNAT appear here only for
                 * historical reasons.
                 */
                case VADS:
                    indent_to(4);
                    put_string("pragma interface_name(");
                    put_string(sym->sym_ada_name);
                    put_string(", language.c_prefix & \"");
                    assert(sym->sym_ident != NULL);
                    assert(sym->sym_ident->node_kind == _Ident);
                    assert(sym->sym_ident->node.id.name != NULL);
                    put_string(sym->sym_ident->node.id.name);
                    put_string("\");\n");
                    break;

                default:
                    interface_c(sym, 4);
                    indent_to(4);
                    put_string("pragma interface_name(");
                    put_string(sym->sym_ada_name);
                    put_string(", \"");
                    put_string(C_VAR_PREFIX);
                    assert(sym->sym_ident != NULL);
                    assert(sym->sym_ident->node_kind == _Ident);
                    assert(sym->sym_ident->node.id.name != NULL);
                    put_string(sym->sym_ident->node.id.name);
                    put_string("\");\n");
                    break;
            }
        }
    }
}

static void inline_func(symbol_t* sym, int indent)
{
    putf("%>pragma Inline(%s);", indent, sym->sym_ada_name);
    print_position(sym->sym_def);
}

static void gen_subp_interface_pragmas(sym_q* fq)
{
    symbol_t* sym;

    for(sym = fq->qhead; sym; sym = sym->sym_gen_list)
    {
        if(!sym->interfaced)
        {
            output_to(declaring_sym_in_spec(sym));
            interface_c(sym, 4);
            switch(ada_compiler)
            {
                case VADS:
                    indent_to(4);
                    put_string("pragma interface_name(");
                    put_string(sym->sym_ada_name);
                    put_string(", language.c_subp_prefix & \"");
                    assert(sym->sym_ident != NULL);
                    assert(sym->sym_ident->node_kind == _Ident);
                    assert(sym->sym_ident->node.id.name != NULL);
                    put_string(sym->sym_ident->node.id.name);
                    put_string("\");\n");
                    break;
                case GNAT:
                    break;
                case Rational:
                    rational_subp_interface_pragma(sym);
                    break;
                default:
                    indent_to(4);
                    put_string("pragma interface_name(");
                    put_string(sym->sym_ada_name);
                    put_string(", \"");
                    put_string(C_SUBP_PREFIX);
                    assert(sym->sym_ident != NULL);
                    assert(sym->sym_ident->node_kind == _Ident);
                    assert(sym->sym_ident->node.id.name != NULL);
                    put_string(sym->sym_ident->node.id.name);
                    put_string("\");\n");
                    break;
            }
        }
    }
}

void gen_unchecked_conversion_func(symbol_t* sym, typeinfo_pt from_type, typeinfo_pt to_type)
{
    indent_to(4);

    putf("function %s is new Ada.Unchecked_conversion\n", sym->sym_ada_name);
    indent_to(8);
    putf(" (%s, ", type_nameof(from_type, false, false));
    putf("%s);\n", type_nameof(to_type, false, false));
    set_symbol_done(sym);
}

static void fix_subps(sym_q* fq)
{
    /* Fixups are performed before the package body is emitted
     * because the fixups may cause new imports or declarations
     * that must be emitted early in the package body.
     */
    symbol_t* sym;
    for(sym = fq->qhead; sym; sym = sym->sym_gen_list)
    {
        if(sym->has_initializer)
        {
            fix_func_body(sym);
        }
    }
}

static void fix_vars(sym_q* vq)
{
    /* Perform fixups on variable initializers before the package
     * body is emitted: fixups may cause new imports or declarations
     * that must be emitted early in the package body.
     */
    symbol_t* sym;
    for(sym = vq->qhead; sym; sym = sym->sym_gen_list)
    {
        if(!sym->is_struct_or_union_member && sym->has_initializer
           && (sym->sym_scope < 2 || sym->is_static || sym->emit_initializer))
        {
            fix_sym_initializer(sym);
        }
    }
} /* fix_vars() */

static void gen_stdarg_concat_func(typeinfo_pt type)
{
    if(type)
    {
        bool t_is_modular = (type->type_kind == int_type) && type->is_unsigned;
        bool t_is_float = (type->type_kind == float_type);
        putf("%>function \"&\" is new Stdarg.Concat(%s, %s, %s);\n", 4,
             type_nameof(type, false, false), t_is_modular ? "True" : "False",
             t_is_float ? "True" : "False");
    }
}

static void gen_use_type_decl(typeinfo_pt type)
{
    putf("%>use type %s;\n", 4, type_nameof(type, false, false));
}

static void gen_unit(unit_n ord)
{
    int i, uord;
    char *unit, *source;
    macro_t* mlist;
    comment_block_pt comment;

    if(set_unit(ord))
        return;

    fix_subps(&compilation[ord].funcq);
    fix_vars(&compilation[ord].varq);

    set_cur_unit_is_child_of_predef();
    unit = cur_unit_name();
    source = cur_unit_source();
    current_unit_is_header = (source[strlen(source) - 1] == 'h');

    reset_output_line();
    reset_indent();

    output_to(current_unit_is_header);

    /* output header comment, if any */
    if((comment = cur_unit_header_comment()))
    {
        put_comment_block(comment, 0);
    }

    /*
     * If we are not a child of the predefined package, "with" it,
     * so we can get the built-in types
     */
    if(!cur_unit_is_child_of_predef())
    {
        putf("with %s;\n", predef_pkg);
    }

    /* Include the <predef>.Ops package to get definitions
     * of operators (like "and") on C types.
     */
    if(true)
    {
        /* TBD: we could implement "has_ops" on ord,
         * & use as a guard on generating this import.
         */
        output_to_body();
        putf("with %s.Ops;  use %s.Ops;\n", predef_pkg, predef_pkg);
        output_to(current_unit_is_header);
    }

    if(has_ellipsis(ord))
    {
        put_string("with Stdarg;\n");
    }

    if(has_c_pointers(ord))
    {
        putf("with %s;\n", generic_ptrs_pkg_name(false));
    }

    if(has_c_const_pointers(ord))
    {
        putf("with %s;\n", generic_ptrs_pkg_name(true));
    }

    if(has_unchecked_conversion(ord))
    {
        output_to(unchecked_conversions_to_spec(ord));
        put_string("with Ada.Unchecked_Conversion;\n");
        output_to(current_unit_is_header);
    }

    if(ada_compiler == VADS)
    {
        put_string("with language;\n");
    }

    /* emit "with" clauses for spec or body */
    for(i = 0;; i++)
    {
        uord = nth_ref_unit_ord(i);
        if(uord == -1)
            break;

        putf("with %s;\n", unit_name(uord));
    }

    /* emit "with" clauses for body */
    output_to_body();
    for(i = 0;; i++)
    {
        uord = nth_body_ref_unit_ord(i);
        if(uord == -1)
            break;

        putf("with %s;\n", unit_name(uord));
    }

    new_line();
    put_string_both("package ");
    output_to_body();
    put_string("body ");
    put_string_both(unit);
    put_string_both(" is\n\n");
    output_to(current_unit_is_header);

    if(ada_compiler == ICC)
    {
        /* Allow C unions */
        indent_to(4);
        put_string("pragma anarchy;");
        comment_start();
        put_string("Allow C unions\n\n");
    }

    if(cur_unit_has_const_string())
    {
        gen_string_stuff();
    }

    if(auto_package)
    {
        gen_macro_constants(mlist = unit_macros[ord], -1);
        import_macro_constants();
    }
    else
    {
        gen_macro_constants(mlist = macro_list_head, -1);
    }

    if(auto_package && import_decls)
    {
        import_types();
    }

    gen_simple_types(&compilation[ord].simple_typeq);
    gen_access_types(&compilation[ord].simple_ptr_typeq, false);
    gen_array_types(&compilation[ord].simple_array_typeq);
    gen_record_incompletes(&compilation[ord].sort_typeq, false);
    gen_access_types(&compilation[ord].rec_ptr_typeq, false);
    gen_sorted_types(&compilation[ord].sort_typeq, false);
    gen_macro_types(mlist, -1);

    /* Stdarg.Concat instantiations and "use type" declarations
     * are only required in the body.
     */
    output_to_body();
    gen_stdarg_concat_funcs(ord, gen_stdarg_concat_func);
    gen_use_type_decls(ord, gen_use_type_decl);

    output_to(current_unit_is_header);

    if(has_unchecked_conversion(ord))
    {
        output_to(unchecked_conversions_to_spec(ord));
        gen_unchecked_conversion_funcs(ord, gen_unchecked_conversion_func);
    }

    if(has_c_pointers(ord) || has_c_const_pointers(ord))
    {
        output_to_body();
        gen_unit_pkg_defs(ord, 4);
    }

    output_to(current_unit_is_header);
    gen_vars(&compilation[ord].litq, -1);
    gen_vars(&compilation[ord].varq, -1);
    gen_macro_vars(mlist, -1, 14);
    import_vars();
    gen_subps(&compilation[ord].funcq, -1);
    gen_macro_funcs(mlist, -1);

    /* try the undone vars again */
    gen_vars(&compilation[ord].varq, -1);

    import_subprograms();

    if(unit_has_private_part(current_unit()) || compilation[ord].varq.qhead != NULL
       || compilation[ord].funcq.qhead != NULL)
    {
        output_to_spec();
        putf("\nprivate\n");

        gen_record_incompletes(&compilation[ord].sort_typeq, true);
        gen_var_interface_pragmas(&compilation[ord].varq);
        gen_subp_interface_pragmas(&compilation[ord].funcq);
        gen_sorted_types(&compilation[ord].sort_typeq, true);
        gen_access_types(&compilation[ord].rec_ptr_typeq, true);
    }

    put_string_both("\nend ");
    put_string_both(unit);
    put_string_both(";\n");

    finish_macros(mlist);

    /* print any trailing comment for unit */
    if((comment = cur_unit_trailer_comment()))
    {
        put_comment_block(comment, 0);
    }

    unit_completed();
}

static bool type_deps_clear(typeinfo_pt type, symbol_t* sym);

static bool dependencies_clear(symbol_t* sym)
/* TBD: cf routines in order.c */
/* TBD: doesn't test function parameter types */
{
    assert(sym != NULL);
    assert(sym->sym_type);
    if(sym->cleared)
        return true;
    return type_deps_clear(sym->sym_type, sym);
}

static bool type_deps_clear(typeinfo_pt type, symbol_t* sym)
{
    symbol_t* tsym = type->type_base;
    switch(decl_class(type))
    {
        case pointer_decl:

            if(is_access_to_record(type))
            {
                /*
                 * This test assumes that all record types have
                 * been emitted by the time we get to the elements
                 * in the sort_typeq.
                 */
                return true;
            }
            if(tsym && equal_types(tsym->sym_type, type))
            {
                return sym == tsym || tsym->cleared;
            }
            return type_deps_clear(type->type_next, sym);

        case func_decl:
            return type_deps_clear(type->type_next, sym);

        case array_decl:

            if(simple_array_type(type))
            {
                return type_deps_clear(type->type_next, sym);
            }
            else
            {
                if(!tsym || sym == tsym)
                {
                    return type_deps_clear(type->type_next, sym);
                }
                else
                {
                    return tsym->cleared;
                }
            }

        case struct_decl:
        {
            symbol_t* basetype = type->type_base;
            assert(basetype != NULL);

            if(basetype != sym)
            {
                /* Typedef of struct OR incomplete record */

                return basetype->cleared;
            }
            else
            {
                /* Must check all tags */
                symbol_t* tag;
                for(tag = sym->sym_tags; tag; tag = tag->sym_parse_list)
                {
                    if(!dependencies_clear(tag))
                        return false;
                }
                return true;
            }
        }

        default:
            return true;
    }
} /* type_deps_clear() */

static bool typesort(sym_q* typeq)
/* Enqueues the symbols in <typeq> so that later symbols
 * do not depend on earlier ones.  Returns true if it
 * was possible to so order all the types.
 */
{
    symbol_t* q = typeq->qhead;
    symbol_t* s;
    symbol_t* last;
    symbol_t* next;
    bool changed;

    typeq->qhead = NULL;
    typeq->qtail = NULL;

    /*
     * Loop through list possibly many times adding symbols which
     * are ready to be generated back into the typeq.  In practice
     * this algorithm is efficient, but under some pathological
     * cases it could very bad.
     */
    do
    {
        changed = false; /* set to true if any syms are cleared
                          * and enqueued on this pass.
                          */
        last = NULL;

        for(s = q; s; s = next)
        {
            next = s->sym_gen_list;

            if(dependencies_clear(s))
            {
                if(last == NULL)
                {
                    q = next;
                }
                else
                {
                    last->sym_gen_list = next;
                }
                s->sym_gen_list = NULL;
                s->cleared = true;
                enq(typeq, s);
                changed = true;
                ;
            }
            else
            {
                last = s;
            }
        }
    } while(changed);

    /* <q> is now empty if all the symbols in <typeq> could be cleared
     * in the preceding loop.
     */
    if(q)
    {
        for(s = q; s; s = next)
        {
            next = s->sym_gen_list;
            s->sym_gen_list = NULL;
            enq(typeq, s);
        }
        return false;
    }
    else
    {
        return true;
    }
}

/*
 * Sort the output order for the following types
 * so that they obey Ada semantics
 */
static void order_types()
{
    int last, i;
    static char* order_warning = "Order of types MAY NOT be correct";

    if(auto_package)
    {
        last = num_units();
        for(i = 0; i < last; i++)
        {
            if(!typesort(&compilation[i].sort_typeq))
            {
                warning(unit_name(i), 0, order_warning);
            }
        }
    }
    else
    {
        if(!typesort(&compilation[0].sort_typeq))
        {
            warning(NULL, 0, order_warning);
        }
    }
}

void gen()
{
    /* dump_macros(macro_list_head, 100000); */
    gen_macro_names();
    unit_start_gen();

    order_types();

    if(auto_package)
    {
        int i, last;
        last = num_units();

        rethread_macros();

        for(i = 0; i < last; i++)
        {
            gen_unit(i);
        }
    }
    else
    {
        gen_unit(0);
    }
    gen_macro_warnings();
}
