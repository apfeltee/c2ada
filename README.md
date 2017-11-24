# c2ada
revised version of c2ada, fixing several segmentation faults.  
builds cleanly(-ish) on 64bit cygwin. will probably build on Linux et al. as well.

files that are not part of the *official* c2ada distrib are in the public domain (unless stated otherwise).
this includes, for example, "./test/".

original readme below.

--------------------

This is a 'port' of c2ada to Linux redhat 5.2 on Intel.

The original program can be obtained from:

http://www.inmet.com/~mg/c2ada/c2ada.html


to BUILD:
---------
Edited Makefile and modify the following

GPERF=
PYTHON=
PYTHON_HOME=
PYTONLINS=
PYHTON_LIB=

see Makefile for explanation of the above macro values.

Next, build it:

$gmake realclean
$gmake

At the end, a program called c2ada will be build in same directory where
the sources are.

simple copy this c2ada to your /usr/local/bin, or anyother place, and
make sure it is in your path.

Next, you need to define an environemnt path to be able to run
c2ada. This path needs to point to where the sources of c2ada where
unpacked. For example, I untarred the tar file into a directory called

/home/nabbasi/c2ada/

so, in your .bash_profile (depending on your shell) I added this line

PYTHONPATH=/home/nabbasi/c2ada
export PYTHONPATH

Now, do 

$c2ada

have fun with it.  

few notes on changes I made:
============================

I simply downloaded the above program, and made slight changes
to make it compile cleanly on Linux.

In the places where I had to change the code to make it compile clean,
I added

#if defined(LINUX)
... changed code
#else
.. original code
#endif

In places where I added new code, I did:

#if defined(LINUX)
..new code
#endif

And add -DLINUX to the Makefile compiler options.

Nasser Abbasi
2/8/99
nabbasi@earthlink.net
