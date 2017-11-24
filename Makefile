## This Makefile requires GNU make (gmake on Solaris).

## The initial target (real dependencies TBD)
all::

# HERE should be set to the directory containing the *.py files
# in the C2Ada source distribution. The form here simply sets this
# to the source directory.
#
# NOTE this requires a recent GNU Make; 3.81 is OK.
#
HERE		?= $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

#--------------------------------------------------------------------------
# Configuration
#
# THIS DOES NOT WORK
Makefile.config: setup
	./setup

include Makefile.config

## This variable can override the 'ar' executable on your system.
#
AR		?= ar

## This variable can override the "gperf" executable on your
## system; cf. ftp://ftp.gnu.ai.mit.edu/pub/gnu/cperf-2.1a.tar.gz .
#
GPERF		?= gperf

## Extra libraries required at link time (support for Python). Only
## necessary on Solaris (libpython only supplied in archive form, not
## as shareable object).
#
EXTRA_LIBS	?=

## This variable can override the 'ranlib' executable on your system.
#
RANLIB		?= ranlib

## This is the install top for Python on your system.
#
PYTHON_TOP	?= /usr

## This variable can override the version of python installed on your
## system.
#
PYTHON_VER	?= python2.4

## This variable can override the top-level Python distribution
## directory on your system; cf. http://www.python.org .
#
PYTHON_LIB	?= $(PYTHON_TOP)/lib/$(PYTHON_VER)

## This variable can override where python .h stuff is
#
PYTHON_INCLUDE	?= $(PYTHON_TOP)/include/$(PYTHON_VER)

## YACC should be set to your yacc equivalent if it's not called
## 'yacc'. For example,
##   YACC=bison; export YACC
## or
##   make YACC='bison -y'
YACC		?= yacc

### no need to change anything below this. Unless you want
### to change gcc flags to be non-debug.

####################################

# In PYTHONLIBS, the argument in the -L switch is the same as the
# target path for the "make libainstall" step in installing Python.
#
PYTHONLIBS	= -L$(PYTHON_LIB)/config \
		  -l$(PYTHON_VER) \
		  $(EXTRA_LIBS)

PYTHONINCLUDES	= -DHAVE_CONFIG_H -I$(PYTHON_INCLUDE)

# This is the path used to locate Python scripts and modules used by
# c2ada. See symset.c
#
PYTHON_SCRIPTS_PATH = $(HERE):$(PYTHON_LIB)
DEF_PPATH	= -DPPATH=\"$(PYTHON_SCRIPTS_PATH)\"

GNU_C_OPTS	= -g3 -ggdb -Wall -Wimplicit -Wreturn-type
CC		= gcc
CFLAGS		= $(GNU_C_OPTS) -DDEBUG $(PYTHONINCLUDES) $(DEF_PPATH)
LINKER		= gcc

MAKEFILE	= Makefile

LOCAL_LIBS	= libcbind.a

SRCS	        = allocate.c \
		anonymous.c \
		aux_decls.c \
		buffer.c \
		c_perf.c \
		cbfe.c \
		context.c \
		cpp.c \
		cpp_eval.c \
		cpp_perf.c \
		errors.c \
		files.c \
		fix_stmt.c \
		format.c \
		gen.c \
		gen_expr.c \
		gen_macros.c \
		gen_stmt.c \
		hash.c \
		initializer.c \
		htype.c \
		localfunc.c \
		macro.c \
		nodeop.c \
		order.c \
		package.c \
		scan.c \
		stab.c \
		stmt.c \
		symset.c \
		types.c \
		units.c \
		y.tab.c \
		ada_name.c \
		ada_perf.c

OBJS		= $(SRCS:.c=.o)

COMMON_SRCS	= allocate.c \
		buffer.c \
		cpp.c \
		errors.c \
		files.c \
		cpp_perf.c \
		cpp_eval.c \
		macro.c \
		hash.c

COMMON_OBJS	= $(COMMON_SRCS:.c=.o)

FESRCS		= \
		ada_types.c \
		aux_decls.c \
	        cbfe.c \
		comment.c \
		configure.c \
		context.c \
		fix_stmt.c \
		gen.c \
		gen_stmt.c \
		gen_expr.c \
		gen_macros.c \
		ada_perf.c \
		ada_name.c \
		format.c \
		initializer.c \
		order.c \
		print.c \
		y.tab.c \
		scan.c \
		nodeop.c \
		package.c \
		c_perf.c \
		types.c \
		stab.c \
		stmt.c \
		symset.c \
		units.c \
		anonymous.c \
		localfunc.c

FEOBJS		= $(FESRCS:.c=.o)

GENED_SRC	= y.tab.c \
		y.tab.h \
		c_perf.c \
		ada_perf.c \
		cpp_perf.c

SCRIPTS		= gen.last \
		gen.makefile \
		gen.diffs \
		hostinfo

# OBSOLETE TARGETS
#		cbind.o
#		cbpp.o
#		cdep.o


all::		make c2ada

%c%y:;
%o%y:;

make::		hostinfo.h
make::		cpp_perf.c
make::		ada_perf.c
make::		c_perf.c
make::		y.tab.h
make::		$(OBJS)
make::		$(COMMON_OBJS)

make::		$(LOCAL_LIBS)

# OBSOLETE
# make::	cdep
# make::	cbpp
# make::	cbind

make::		c2ada


# OBSOLETE RULES
#install: all
#	- strip cbpp cbfe cbind cdep
#	- rm -f $(HOME)/bin/cbpp $(HOME)/bin/cbfe $(HOME)/bin/cbind $(HOME)/bin/cdep
#	- cp cbpp cbfe cbind cdep $(HOME)/bin


# This is the executable for C2Ada.
#
c2ada:		$(FEOBJS) $(LOCAL_LIBS) config.o
		@echo "LOCAL_LIBS = $(LOCAL_LIBS)"
		@echo "LDFLAGS = $(LDFLAGS)"
		@echo "LIBS = $(LIBS)"
		@echo "PYTHONLIBS = $(PYTHONLIBS)"
		$(LINKER) $(LDFLAGS) -o c2ada $(FEOBJS) $(LOCAL_LIBS) \
                $(LIBS) config.o $(PYTHONLIBS)

#cbpp:	cbpp.o $(LOCAL_LIBS)
#		$(LINKER) $(LDFLAGS) cbpp.o $(LOCAL_LIBS) $(LIBS) -o $@

#cdep:	cdep.o $(LOCAL_LIBS)
#		$(LINKER) $(LDFLAGS) cdep.o $(LOCAL_LIBS) $(LIBS) -o $@

#cbind:	cbind.o $(LOCAL_LIBS)
#		$(LINKER) $(LDFLAGS) cbind.o $(LOCAL_LIBS) $(LIBS) -o $@

htype:	htype.c htype.o
		$(LINKER) $(LDFLAGS) htype.o $(LIBS) -o $@

libcbind.a:	$(COMMON_OBJS)
		@ rm -f $@
		$(AR) rc $@ $(COMMON_OBJS)
		$(RANLIB) $@

clean::;	- rm -f cbind cbfe cbpp cdep
clean::;	- rm -f *.o *.d *.pyc
clean::;	- rm -f $(LOCAL_LIBS)
clean::;	- rm -f hostinfo.h htype htype.o

realclean:	clean
		- rm -f $(GENED_SRC)

mf:		hostinfo.h cpp_perf.c y.tab.c y.tab.h
		mkmf -f $(MAKEFILE)

tarfile:
	tar cf - \
	 -C DOC . -C CCOMP . -C COMP . -C MISC . -C YACC . \
	 -C CM catalog.des \
	| gzip > c2ada.tar.gz

# probably obsolete:
backup:
		- rm -f $(HOME)/save/newc2a.tar.Z
		tar -chf $(HOME)/save/newc2a.tar *.1 *.c *.h *.y *.prf $(SCRIPTS) DOCS README makefile
		compress $(HOME)/save/newc2a.tar

touch:;	touch $(GENED_SRC)

hostinfo.h: htype hostinfo
		./hostinfo $@

c_perf.c:	c.prf
		$(GPERF) -t -p c.prf > $@

ada_perf.c:	ada.prf
		$(GPERF) -k1,4,'$$' -N ada_keyword ada.prf > $@

cpp_perf.c:	cpp.prf
		$(GPERF) -N cpp_keyword -t -p cpp.prf > $@

cpp_perf.o: cpp_perf.c files.h hash.h cpp.h buffer.h
ada_perf.o: ada_perf.c

y.tab.h:	grammar.y
		$(YACC) -d grammar.y

y.tab.c:	grammar.y
		echo "one reduce/reduce conflict expected"; $(YACC) grammar.y

#--------------------------------------------------------------------------
# Configuration file for Python module set

config.o : $(PYTHON_LIB)/config/config.c
	$(CC) $(CFLAGS) -DNO_MAIN -c $(PYTHON_LIB)/config/config.c

#--------------------------------------------------------------------------
# Dependencies

%.d: %.c
	$(SHELL) -ec 'gcc -MM $(CFLAGS) $< | sed "s/$*\\.o[n:]*/$@ &/g" >$@'

include $(SRCS:.c=.d)

