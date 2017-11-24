# $Source: /home/CVSROOT/c2ada/AdaMain.py,v $
# $Revision: 1.1.1.1 $   $Date: 1999/02/02 12:01:51 $

# Write an Ada main procedure to wrap a C main().

import sys

# text: the (generic) text of the Ada main procedure.
# 
# ---------------------------------
text = '''
with %(cmain)s;
with Ada.Command_Line;
with %(predef)s;
procedure %(unit)s is

    package AC renames Ada.Command_line;

    type charp_array is array(%(predef)s.natural_int range <>) 
        of aliased %(predef)s.charp;

    argc : %(predef)s.int := %(predef)s.int(AC.Argument_Count);

    argv : charp_array(0..argc);

begin

    argv(0) := %(predef)s.new_string(AC.Command_Name);

    for i in 1 .. argc loop
        argv(i) := %(predef)s.new_string(AC.argument( integer(i) ));
    end loop;

    %(cmain)s.main(argc, argv(0)'access );

end %(unit)s;
''' # end text
# ---------------------------------
    
# ada_main: write the main procedure for the output Ada program
#    cmain:  the Ada name of the package holding the translated
#            C main() program
#    predef: the "predef" package name
#    unit  : the name of the Ada unit to be produced
#    filename: file to write unit to, if specificed
#
def ada_main(cmain, predef, unit, filename=None):
    result = text % locals()
    if filename:
	f = open(filename,'w')
	f.write(result)
	f.close()
    else:
	return result

usage_msg = '''
usage: python AdaMain.py cmain predef unit [filename]

       cmain:    the name of the Ada package containing the C program main()
       predef:   name of the predefined package
       unit:     package name of output compilation unit
       filename: path name of output file
                 if omitted, output to stdout
'''
       

# main: the command line interface
# the call is AdaMain cmain predef unit [filename]
def main():
    argv = sys.argv
    if len(argv)==4 or len(argv)==5:
	cmain =  argv[1]
	predef = argv[2]
	unit   = argv[3]
	if len(argv)==5:
	    filename = argv[4]
	else:
	    filename = None
        result = ada_main(cmain, predef, unit, filename)
	if result:
	    print result
    else:
	print usage_msg

if __name__ == '__main__' : main()




