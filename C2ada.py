# $Source: /home/CVSROOT/c2ada/C2ada.py,v $
# $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $

# Provide the framework for reading in a configuration script.

# The configuration script contains the declaration of project-specific
# information about c2ada's task.  It initializes various attributes
# of a single global data structure called "the", containing statements
# like
#
#    the.reserved_names = ["x","stdarg"]
#    the.source("blah.h").interfaces("blah.c")
#
# or whatever.  The general facilities of Python can be used to streamline
# repetitive "declarations":

#    for x in ["a.h", "b.h", "c.h"]:
#        the.source(x).output_dir = "/dev/null"
#
# The principal routine called from C is "configure_project", which
# takes as argument the name of the configuration script. It executes
# a script in an environment which exposes "the", then returns the
# resulting value.  C code then accesses the various attributes
# of this return value.


###################

import os
import traceback
import string
True = 1
False = 0

# The type of "the" is C2ada_project.  Declaring this as a class allows
# us to provide default values, and provides the hooks for catching
# assignments to non-existent attributes.
#
class C2ada_project:

    # writable_attrs is for documentation & eventual input checking
    #
    writable_attrs = { 'reserved_names' : 'list of strings' }


    def __init__(self):
        self.output_dir = './bindings'
        # self.ada_compiler = 'gnat'
        # self.comments_preserved = False
        # self.strings_to_char_arrays = False
        self.source = Instance_dict(C_source)
	# self.source_directories = None
	self.reserved_names = []

# A C_source object captures properties associated with a C source file
# name.
#
class C_source:
    writable_attrs = {'unchecked_conversions_to_spec' : 'boolean',
		      'partner'                       : 'string'  }

    def __init__(self, name):
        self.c_name   = name 
	self.is_header= os.path.splitext(name)[1] == '.h'

        self.decl       = Source_decl_dict()
        self.macro      = Instance_dict(Macro)

        # self.ada_name = None
	#self.output_dir = the.output_dir

    def interfaces(self, name):
	self.partner = name
	the.source(name).partner = self.c_name

# An Instance_dict is a dictionary that contains instances of the
# argument class "iclass". It creates an initial instance of this
# class whenever a key does not already have an instance.
#
class Instance_dict:
    def __init__(self,iclass):
        self.dict = {}
        self.iclass = iclass

    def __getitem__(self,key):
	return self.dict[key]

    def __call__(self,key):
        try:
            return self.dict[key]
        except KeyError:
            result = self.iclass(key)
            self.dict[key] = result
            return result

# These must correspond to the values of {ENUM,STRUCT,UNION}_PREFIX
# in il.h
#
prefix_dict = {'enum':'1', 'struct':'2', 'union':'3'}

def alt_key(key):
    if string.find(key,' ')== -1:
	return None
    words = string.split(key)
    if len(words)!=2: return None
    try:
	return prefix_dict[words[0]] + words[1]
    except:
	return None
    

class Source_decl_dict(Instance_dict):
    def __init__(self):
	Instance_dict.__init__(self, Decl)

    def __call__(self,key):
	try:
	    return self.dict[key]
	except KeyError:
	    result = self.iclass(key)
	    self.dict[key] = result
	    alt = alt_key(key)
	    if alt:
		self.dict[alt] = result
	    return result

class Macro:

    writeable_attrs = { 'replacement' : 'string',
			'returns'     : 'string',
			'signature'   : 'string' }

    def __init__(self,name):
        self.name = name
  
    # reconstitute restores a macro body, undoing the encoding
    # that happened in grok_macro (cpp.c).
    # This is really a subroutine, not a method that's
    # intended to be invoked outside this class.
    #
    def reconstitute(self,body,formals):
	result = body
	for i in range(1,len(formals)+1):
	    mark = chr(1)+chr(i)
	    result = string.joinfields(string.splitfields(result,mark),
				       formals[i-1])
	return result 

    returns   = None  #default attribute
    signature = ''    #default attribute

    # rewrite: turn a #define directive into a declaration
    # TBD: currently only handles function-like macros
    #
    def rewrite(self, name, body, formals, eol_comment):

	cbody = self.reconstitute(body,formals)

	sig= self.signature
	
	if self.returns:
	    rtype = self.returns
	    stmt = "return %s;" % cbody
	else:
	    rtype = 'void'
	    stmt = "%s;" % cbody

	if eol_comment:
	    comment = '/*%s*/' % eol_comment
	else:
	    comment = ''

	fmt = 'inline %(rtype)s %(name)s (%(sig)s) { %(stmt)s } %(comment)s'

	return fmt % locals()
	


class Decl:
    writeable_attrs = {'ada_name'            : 'string',
		       'return_type_is_void' : 'boolean',
		       'private'             : 'boolean' }
		         
    def __init__(self,name):
        self.name = name
        # self.ada_name = None
        # self.scope = Instance_dict(Decl)


#################

# define "the"; accessed as "the_data" in this module.
#
the_data = C2ada_project()
the      = the_data

# "configure" takes as argument the name of a file which contains
# the configuration script for this run.
#
def configure(filename):
    globals = {'the':the_data, 'True':1, 'False':0}
    if filename:
	try:
	    execfile(filename, globals)
        except:
	    print "error in initialization file", filename
	    traceback.print_exc()
	    return None
    return the_data


# "source_partner" returns the name, if any, of a source file associated
# with the argument file.  "partners" are .h/.c files with a spec/body
# relationship, as specified in the configuration file.
#
# This function must be called after "configure" has been called.

def source_partner(filename):
    try:
	return the.source[filename].partner
    except:
	return ""
    
