/* $Source: /home/CVSROOT/c2ada/symset.c,v $ */
/* $Revision: 1.3 $  $Date: 1999/02/09 18:16:51 $ */

#include <stdlib.h>
#include <stdbool.h>
#include "il.h"
#include "errors.h"
#include "order.h"
#include "symset.h"

/* NB: DEBUG is defined in the c2ada code, but used for a different, and
 * here useless, purpose in the Python code.
 */
#undef DEBUG
#include <Python.h>
#include <pythonrun.h>
#include <import.h>

static PyObject* pymod_Symbol;
static PyObject* oSymbol;

/* This definition is really a dummy; we always cast between
 * (PyObject*) and (struct symset *) -- i.e. symbols_t.
 */
struct symset
{
    PyObject* o;
};

static PyObject* primSymbol_undone(PyObject*, PyObject*);
static PyObject* py_primSymbol_undone;

static PyMethodDef PrimSymbolMethods[] = { { "undone", primSymbol_undone, 1 }, { 0, 0 } };

static PyObject* pymod_PrimSymbol;

static char* getpythonpath(void);

/* {{{{ begin ridiculous hacks ... */

/* fairly-ish quick n dirty dirname */
static char* mydirname(char* s)
{
    char *p;
    bool isntpath;
    unsigned int slen;
    isntpath = false;
    if((s == NULL) || (*s == 0))
    {
        return  ".";
    }
    slen = strlen(s);
    p = s + slen;
    /* check for i.e., "c:/" - must return "c:/"! */
    if(slen > 2)
    {
        if(isalnum(s[0]) && (s[1] == ':') && ((s[2] == '/') || (s[2] == '\\')))
        {
            isntpath = true;
        }
    }
    while((p != s) && ((*p == '/') || (*p == '\\')))
    {
        p--;
    }
    if((p == s) && ((*p == '/') || (*p == '\\')))
    {
        return "/";
    }
    while(p != s)
    {
        p--;
        if((*p == '/') || (*p == '\\'))
        {
            while((*p == '/') || (*p == '\\'))
            {
                p--;
            }
            *++p = '\0';
            return s;
        }
    }
    return ".";
}


static char* getselfpath()
{
    enum { maxpath = 1024 * 8 };
    char* tmp;
    char* dname;
    char epath[maxpath];
    #if defined(__unix__) || defined(__linux__) || defined(__CYGWIN__)
        if(readlink("/proc/self/exe", epath, maxpath) == -1)
        {
            fprintf(stderr, "getselfpath: readlink() failed\n");
            return NULL;
        }
    #elif (defined(_WIN32) || defined(_WIN64)
        if(GetModuleFileName(NULL, epath, maxpath) == 0)
        {
            fprintf(stderr, "getselfpath: GetModuleFileName failed\n");
            return NULL;
        }
    #else
        fprintf(stderr, "no way to get selfpath for this platform :-(\n");
        return NULL;
    #endif
    dname = (char*)malloc(maxpath);
    tmp = mydirname(epath);
    strcpy(dname, tmp);
    return dname;
}

static void addsyspath(const char* newpath)
{
    PyObject* sys;
    PyObject* path;
    sys = PyImport_ImportModule("sys");
    path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString(newpath));
}

static void addlocalpath()
{
    enum { maxpath = 1024 * 8};
    char* selfp;
    char finalpath[maxpath];
    if((selfp = getselfpath()) != NULL)
    {
        strcpy(finalpath, selfp);
        strcat(finalpath, "/");
        strcat(finalpath, "pylibc2ada");
        addsyspath(finalpath);
        free(selfp);
    }
    else
    {
        fprintf(stderr, "failed to add self path - c2ada will likely break if called outside of the source directory\n");
    }
}

/* end ridiculous hacks... }}}} */


void symset_init(void)
{
    /* We have to set PYTHONPATH before initializing Python. */
    // setenv("PYTHONPATH", getpythonpath(), 1);

    Py_Initialize();
    addlocalpath();
    /* PyRun_SimpleString("import sys, pprint; pprint.pprint(sys.path);"); */

    pymod_PrimSymbol = Py_InitModule("primSymbol", PrimSymbolMethods);
    py_primSymbol_undone = PyObject_GetAttrString(pymod_PrimSymbol, "undone");

    pymod_Symbol = PyImport_ImportModule("Symbol");
    if(!pymod_Symbol)
    {
        fprintf(stderr, "Unable to load Symbol.py\n");
        exit(1);
    }

    oSymbol = PyObject_GetAttrString(pymod_Symbol, "oSymbol");
    assert(oSymbol);
}

symbols_t new_symbols_set(void)
{
    PyObject* result = PyDict_New();
    assert(result);
    return (symbols_t)result;
}

static PyObject* toPyObject(symbol_pt sym)
{
    return PyInt_FromLong((long)sym);
}

static symbol_pt fromPyObject(PyObject* obj)
{
    /*return (symbol_pt) PyInt_AsLong( obj );*/
    return (symbol_pt)PyCObject_AsVoidPtr(obj);
}

static PyObject* primSymbol_undone(PyObject* self, PyObject* args)
{
    long symaddr;
    if(!PyArg_ParseTuple(args, "l", &symaddr))
        return 0;
    return PyInt_FromLong((long)!sym_done((symbol_pt)symaddr));
}

void symset_add(symbols_t syms, symbol_pt sym)
{
    int result;
    PyObject* addr;

    assert(sym);

    addr = toPyObject(sym);
    result = PyObject_SetItem((PyObject*)syms, addr, addr);
    assert(result != -1);
}

boolean symset_has(symbols_t syms, symbol_pt sym)
{
    PyObject* symObj = toPyObject(sym);
    boolean result;
    result = PyMapping_HasKey((PyObject*)syms, symObj);
    Py_DECREF(symObj);
    return result;
}

int symset_size(symbols_t syms)
{
    int result = PyMapping_Length((PyObject*)syms);
    assert(result != -1);
    return result;
}

void symset_filter_undone(symbols_t syms)
{
    PyObject* args;
    PyObject* result;
    static PyObject* symFilter;

    if(!symFilter)
    {
        symFilter = PyObject_GetAttrString(pymod_Symbol, "symFilter");
        assert(symFilter);
    }

    args = Py_BuildValue("(OO)", (PyObject*)syms, py_primSymbol_undone);
    result = PyEval_CallObject(symFilter, args);
    assert(result);
    Py_DECREF(args);
    Py_DECREF(result);
}

/* Symbol abstraction */

PyObject* pySymbol(symbol_pt sym)
{
    PyObject* args;
    PyObject* pySym;
    PyObject* symObj = toPyObject(sym);

    args = Py_BuildValue("(O)", symObj);
    pySym = PyEval_CallObject(oSymbol, args);
    assert(pySym);
    Py_DECREF(symObj);
    Py_DECREF(args);
    return pySym;
}

symbols_t get_undone_requisites(symbol_pt sym)
{
    PyObject* pySym = pySymbol(sym);
    PyObject* result;

    result = PyObject_GetAttrString(pySym, "requisites");

    return (symbols_t)result;
}

void set_undone_requisites(symbol_pt sym, symbols_t syms)
{
    int result;
    PyObject* pySym = pySymbol(sym);
    result = PyObject_SetAttrString(pySym, "requisites", (PyObject*)syms);
    assert(result != -1);
}

/* Symbol map */

symmap_t new_symmap(char* mapname)
{
    return (symmap_t)PyDict_New();
}

symbol_pt get_symmap(symmap_t map, symbol_pt sym)
{
    PyObject* dict = (PyObject*)map;
    PyObject* symObj = toPyObject(sym);
    PyObject* resultObj = PyObject_GetItem(dict, symObj);
    Py_DECREF(symObj);
    if(resultObj)
    {
        return fromPyObject(resultObj);
    }
    else
    {
        PyErr_Clear();
        return 0;
    }
}

void set_symmap(symmap_t map, symbol_pt key, symbol_pt value)
{
    PyObject* dict = (PyObject*)map;
    PyObject* keyObj = toPyObject(key);
    PyObject* valObj = toPyObject(value);

    PyObject_SetItem(dict, keyObj, valObj);
    Py_DECREF(keyObj);
    Py_DECREF(valObj);
}

/*****************************************************/
char* getpythonpath(void)
{
    char* envpath = getenv("C2ADA_PYTHONPATH");
    if(envpath)
    {
        return envpath;
    }
    else
    {
        return PPATH;
    }
}

/* for debugging */
static char* repr(PyObject* py)
{
    PyObject* result = PyObject_Repr(py);
    return PyString_AsString(result);
}
