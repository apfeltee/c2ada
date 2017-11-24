/* $Source: /home/CVSROOT/c2ada/configure.c,v $ */
/* $Revision: 1.2 $  $Date: 1999/02/03 19:45:03 $ */

#include <stdlib.h>

#include "errors.h"
#include "allocate.h"
#include "files.h"
#include "units.h"
#include "il.h"
#include "types.h"

#undef DEBUG
#include "Python.h"
#include "pythonrun.h"
#include "import.h"

#include "configure.h"

static PyObject * c2ada;             /* Python module "C2ada" */
static PyObject * the_data;          /* Python "the" */
static PyObject * the_sources;       /* Python "the.source" */

boolean configured = FALSE;  /* set by calling configure_project */


void
configure_project( char * filename )
{
    Py_Initialize();

    c2ada = PyImport_ImportModule("C2ada");
    assert(c2ada);
    the_data = PyObject_CallMethod(c2ada, "configure", "(s)", filename);
    assert(the_data);
    the_sources = PyObject_GetAttrString(the_data, "source");
    assert(the_sources);
}

char **
configured_reserved_ids(int * count_p)
    /* return the.reserved_names as a C string list */
{
    PyObject * list = PyObject_GetAttrString(the_data, "reserved_names");
    int        count;
    char **    result;
    int        i;

    assert(list);
    assert(PySequence_Check(list));

    count = PyObject_Length(list);
    assert(count!=-1);
    *count_p = count;

    result = allocate( (sizeof (char*)) * (count+1) );
    for (i=0; i<count; i++) {
	PyObject * item;
	item = PySequence_GetItem(list, i);
	assert(item);
	result[i] = PyString_AsString(item);
    }
    result[count] = 0;
    Py_DECREF(list);

    configured = TRUE;

    return result;
}

boolean
configured_source_flag(char * source,
		       char * attribute,
		       boolean default_result)
{
    PyObject * sourceObj = PyMapping_GetItemString(the_sources,source);
    PyObject * attribObj;
    boolean    result = default_result;
    if (sourceObj) {
	attribObj = PyObject_GetAttrString(sourceObj, attribute);
	if (attribObj) {
	    result = PyObject_IsTrue( attribObj );
	    Py_DECREF(attribObj);
	} else {
	    PyErr_Clear();
	}
    } else {
	PyErr_Clear();
    }

    return result;
}

typedef struct {
    file_id_t  file;
    PyObject * sourceObj;
    PyObject * declsObj;
    PyObject * macrosObj;
} source_t, *source_pt;

source_t cached = {-1};

static source_pt
file_source( file_id_t file )
{
    if (cached.file != file) {

	char * filename = file_name_from_ord(file);
	cached.sourceObj = PyMapping_GetItemString(the_sources, filename);
	if (cached.sourceObj) {
	    cached.declsObj  =
		PyObject_GetAttrString(cached.sourceObj, "decl");
	    assert(cached.declsObj);
	    cached.macrosObj =
		PyObject_GetAttrString(cached.sourceObj, "macro");
	} else {
	    PyErr_Clear();
	}
	cached.file = file;
    }
    return &cached;
}


boolean
configured_sym_info( symbol_pt sym, typeinfo_pt type )
    /*
     * Called when a symbol has just been created (and the
     * sym_def and sym_ident fields filled; searches the
     * configuration data for any extra information to fill
     * in for this declaration.
     */
{
    source_pt  source;
    PyObject * declObj;
    PyObject * return_type_is_voidObj;
    PyObject * ada_nameObj;

    source = file_source( pos_file(sym->sym_def) );
    if (!source) return FALSE;

    declObj = PyMapping_GetItemString(source->declsObj,
				      sym->sym_ident->node.id.name);
    if (!declObj) {
	PyErr_Clear();
	return FALSE;
    }

    /* return_type_is_void, boolean */

    return_type_is_voidObj =
	PyObject_GetAttrString(declObj, "return_type_is_void");
    if (!return_type_is_voidObj) PyErr_Clear();

    if (return_type_is_voidObj && PyObject_IsTrue(return_type_is_voidObj)) {
	typeinfo_pt t = add_function_type(type_void());
	t->type_info.formals = type->type_info.formals;
	sym->sym_type = t;
    }

    /* ada_name : string */
    ada_nameObj = PyObject_GetAttrString(declObj, "ada_name");
    if (ada_nameObj) {
	sym->sym_ada_name = PyString_AsString(ada_nameObj);
    } else {
	PyErr_Clear();
    }

    /* private : boolean */
    {
	PyObject * privateObj =
	    PyObject_GetAttrString(declObj, "private");
	if (privateObj) {
	    sym->private = PyObject_IsTrue(privateObj);
	} else {
	    PyErr_Clear();
	}
    }

    /* declare_in_spec: boolean */
    {
	PyObject * dspecObj =
	    PyObject_GetAttrString(declObj, "declare_in_spec");
	if (dspecObj) {
	    sym->declare_in_spec = PyObject_IsTrue(dspecObj);
	} else {
	    PyErr_Clear();
	}
    }
    return TRUE;

} /* configured_sym_info */


static char * configured_output_dir_value;

void
set_output_dir( char * pathname )
    /* Called from cbfe.c to set up any value from command line switch. */
{
    configured_output_dir_value = pathname;
}

char *
configured_output_dir()
{
    return configured_output_dir_value;
}


/* "Source partner" is the relationship between a .c file and the .h
 * file that describes its interface. The function <configured_source_partner>
 * takes either name and returns the other.
 */

char *
configured_source_partner( char * fname )
{
    PyObject * resultObj =
	PyObject_CallMethod(c2ada, "source_partner", "(s)", fname );
    char * result;
    if (PyObject_IsTrue(resultObj)) {
	result = PyString_AsString(resultObj);
    } else {
	result = 0;
    }
    return result;
}


static PyObject *
stringListObj( int count, char ** items )
{
    int i;
    PyObject * list;

    if (count==-1) return Py_None;
    list = PyList_New(count);

    for (i=0; i<count; i++) {

	char *s = items[i];
	PyObject *sObj = PyString_FromString(s);
	PyList_SetItem( list, i, sObj );
    }
    return list;
}


char *
configured_macro_replacement(file_id_t file,
			     char * macro_name,
			     char * macro_body,
			     int    body_len,
			     int    nformals,
			     char **formals,
			     char * eol_comment)
{
    source_pt  source = file_source(file);
    PyObject * macroObj;
    PyObject * replacementObj;
    PyObject * formalsObj;
    PyObject * resultObj;
    char *     result;

    if (!source) return 0;
    macroObj = PyMapping_GetItemString(source->macrosObj,
				       macro_name);
    if (!macroObj) {
	PyErr_Clear();
	return 0;
    }


    replacementObj = PyObject_GetAttrString(macroObj, "replacement");
    if (replacementObj) {
	return PyString_AsString(replacementObj);
    }

    /* We're going to pass the information to a Python method */
    formalsObj = stringListObj( nformals, formals );

    resultObj = PyObject_CallMethod(macroObj, "rewrite", "(ssOs)",
				    macro_name,
				    macro_body,
				    formalsObj,
				    eol_comment);
    Py_DECREF(formalsObj);

    if (resultObj) {
	result = PyString_AsString(resultObj);
	/* TBD: how about cleaning up storage? */
	return result;
    }

    PyErr_Clear();
    return 0;

}
