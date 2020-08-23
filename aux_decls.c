/* $Source: /home/CVSROOT/c2ada/aux_decls.c,v $ */
/* $Revision: 1.3 $  $Date: 1999/02/09 18:16:51 $ */

#include <string.h>
#undef DEBUG
#include <Python.h>
#include <pythonrun.h>
#include <import.h>

#include "c2ada.h"


/* Unchecked conversions */

typedef struct unchecked_cvt
{
    typeinfo_pt from_type;
    typeinfo_pt to_type;
    int unit;
    symbol_t* cvt_func;
    bool in_spec;

    struct unchecked_cvt* next;

} unchecked_cvt_t, *unchecked_cvt_pt;

unchecked_cvt_pt unchecked_cvt_list, unchecked_cvt_tail;

static void
new_unchecked_cvt(symbol_t* sym, typeinfo_pt from_type, typeinfo_pt to_type, int unit, bool in_spec)
{
    unchecked_cvt_pt uc = allocate(sizeof(unchecked_cvt_t));
    uc->from_type = from_type;
    uc->to_type = to_type;
    uc->unit = unit;
    uc->cvt_func = sym;
    uc->in_spec = in_spec;

    uc->next = 0;
    if(!unchecked_cvt_list)
        unchecked_cvt_list = uc;
    if(unchecked_cvt_tail)
        unchecked_cvt_tail->next = uc;
    unchecked_cvt_tail = uc;
}

void gen_unchecked_conversion_funcs(int unit, gen_unchecked_conversion_func_pt genf)
{
    unchecked_cvt_pt uc;
    for(uc = unchecked_cvt_list; uc; uc = uc->next)
    {
        if(uc->unit == unit)
        {
            (*genf)(uc->cvt_func, uc->from_type, uc->to_type);
        }
    }
}

symbol_t*
unchecked_conversion_func(typeinfo_pt from_type, typeinfo_pt to_type, file_pos_t pos, bool in_spec)
{
    unchecked_cvt_pt cvt;
    unit_n unit = pos_unit(pos);

    /* First look to see if we're duplicating a typedef */
    for(cvt = unchecked_cvt_list; cvt; cvt = cvt->next)
    {
        if(cvt->unit == unit && equal_types(from_type, cvt->from_type)
           && equal_types(to_type, cvt->to_type))
        {
            if(in_spec)
            {
                cvt->in_spec = true;
                set_unchecked_conversion(unit, in_spec);
            }
            return cvt->cvt_func;
        }
    }

    /* (Else) we'll create a new conversion. */
    {
        char* basename = type_nameof(to_type, false, false);
        char* fname = new_strf("To_%s", tail(basename));
        symbol_t* sym;

        sym = new_sym();
        sym->sym_ada_name = ada_name(fname, true);
        sym->sym_type = add_function_type(copy_type(to_type));
        sym->sym_kind = func_symbol;
        sym->sym_def = pos;

        new_unchecked_cvt(sym, from_type, to_type, unit, in_spec);

        set_unchecked_conversion(unit, in_spec);

        deallocate(fname);
        return sym;
    }
} /*unchecked_conversion_func*/

/****************** declarations for stdarg.concat **********************/

typedef struct
{
    PyObject* unitDict;
    PyObject* dict;
} unit_type_usage_t, *unit_type_usage_pt;

static unit_type_usage_t stdarg_concat_o;
static unit_type_usage_t use_type_o;

static PyObject* module_UnitDict;
static PyObject* class_UnitDict;

void init_unit_dict(void)
{
    PyErr_Clear();
    module_UnitDict = PyImport_ImportModule("UnitDict");
    assert(module_UnitDict);

    class_UnitDict = PyObject_GetAttrString(module_UnitDict, "UnitDict");
    assert(class_UnitDict);

    use_type_o.unitDict = PyObject_GetAttrString(module_UnitDict, "use_type");
    assert(use_type_o.unitDict);

    use_type_o.dict = PyObject_GetAttrString(use_type_o.unitDict, "dict");
    assert(use_type_o.dict);

    stdarg_concat_o.unitDict = PyObject_GetAttrString(module_UnitDict, "stdarg_concat");
    assert(stdarg_concat_o.unitDict);

    stdarg_concat_o.dict = PyObject_GetAttrString(stdarg_concat_o.unitDict, "dict");
    assert(stdarg_concat_o.dict);
}

/* The symbol for Ada Stdarg.Empty */
static symbol_t* stdarg_empty_sym;

node_pt stdarg_empty_node(file_pos_t pos)
{
    if(!stdarg_empty_sym)
    {
        symbol_t* sym = new_sym();
        sym->sym_ada_name = "Stdarg.Empty";
        sym->sym_kind = var_symbol;
        sym->intrinsic = true;
        /* sym->sym_type  = TBD; */
        /* sym->sym_def   =  TBD ; */
        stdarg_empty_sym = sym;
    }

    /* TBD: there ought to be a way to indicate that we need
     * this for the body, not the header.
     */
    set_ellipsis(pos_unit(pos));
    return new_pos_node(pos, _Sym, stdarg_empty_sym);
}

void unit_uses_type(unit_type_usage_pt usage, unit_n unit, typeinfo_pt type)
{
    PyObject* u_list;
    int i;
    PyObject* item;
    typeinfo_pt type_item;
    PyObject* result;

    if(!usage->unitDict)
        init_unit_dict();

    u_list = PyObject_CallMethod(usage->unitDict, "entry", "(i)", unit);
    assert(u_list);

    for(i = 0;; i++)
    {
        item = PySequence_GetItem(u_list, i);
        if(!item)
            break;
        /*type_item = (typeinfo_pt) PyInt_AsLong( item );*/
        type_item = (typeinfo_pt)PyCObject_AsVoidPtr(item);
        /* Py_DECREF(item); */
        if(same_ada_type(type_item, type))
            return;
    }

    result = PyObject_CallMethod(u_list, "append", "(i)", (long)type);
    assert(result);
    /* Py_DECREF(item); */
}

void use_stdarg_concat(unit_n unit, typeinfo_pt type)
{
    unit_uses_type(&stdarg_concat_o, unit, type);
}

typedef void (*gen_unit_type_usage_func_pt)(typeinfo_pt type);

static void gen_unit_type_usages(unit_type_usage_pt usage, unit_n unit, gen_unit_type_usage_func_pt f)
{
    typeinfo_pt type_item;
    int i;

    PyObject* key; /* unit */
    PyObject* entry; /* entry = stdarg_concat.dict[key] */
    PyObject* item; /* item  = entry[i] */

    if(!usage->unitDict)
        return;

    key = PyInt_FromLong(unit);
    entry = PyDict_GetItem(usage->dict, key);
    Py_DECREF(key);
    if(!entry)
        return;

    for(i = 0;; i++)
    {
        item = PySequence_GetItem(entry, i);
        if(!item)
            break;
        /*type_item = (typeinfo_pt) PyInt_AsLong(item);*/
        type_item = (typeinfo_pt)PyCObject_AsVoidPtr(item);
        if(!type_item)
            break;
        (*f)(type_item);
        /* Py_DECREF(item); */
    }
    /*  Py_DECREF(entry); */
    if(i > 0)
        putf("\n");
}

void gen_stdarg_concat_funcs(unit_n unit, gen_stdarg_concat_func_pt f)
{
    gen_unit_type_usages(&stdarg_concat_o, unit, f);
}

/************ use type declarations share the same implementation *****/

void use_type(unit_n unit, typeinfo_pt type)
{
    unit_uses_type(&use_type_o, unit, type);
}

void gen_use_type_decls(unit_n unit, gen_use_type_decl_pt f)
{
    gen_unit_type_usages(&use_type_o, unit, f);
}
