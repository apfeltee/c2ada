#! /usr/bin/env python

from pyparsing import *

def get_typedef_parser():

    declare_list = Forward()  # Will be defined later, after being referenced.
    ident = Word(alphas+'_', alphanums+'_')
    int_number = Word(nums+' ()+-*/')

    # "|" = MatchFirst
    simple_base = Group(
            ( Optional(Keyword('unsigned'))('unsigned') + oneOf('char int long short')('name') ) |
            oneOf('unsigned double float void')('name') |
            ident('name')
        )('simple')

    struct_base = Group( Keyword('struct') + Optional(ident('struct_name')) + '{' + declare_list + '}' )('struct')

    union_base = Group( Keyword('union') + '{' + declare_list + '}' )('union')  # "+" = And

    enum_base = Group( Keyword('enum') + Optional(ident('enum_name')) + '{'
            + delimitedList(ident) + Optional(',') +
            '}' )('enum')

    any_base = (struct_base | union_base | enum_base | simple_base)('type')

    array = Group( '[' + int_number('extent') + ']' )

    bit_size = Group( ':' + int_number('extent') )

    declare = Group(
                any_base + ident('instance') +
                Optional( OneOrMore(array)('array') | bit_size('bit') ) + ';'
            )('declare')

    # a results name with a trailing '*' character will be interpreted as setting listAllMatches to True
    declare_list << OneOrMore( declare )('declare_list')

    typedef_parser = Group( Keyword('typedef') + declare )('typedef')
    typedef_parser.ignore( '#' + restOfLine ) # prepro
    #typedef_parser.setDebug( True )

    return typedef_parser


test_data = '''
    typedef struct struct_name1 {
       int x;
       int y;
    } TEST_REC;
'''

import pprint

typedef_parser = get_typedef_parser()
test_data = open("c2ada.h").read()
while True:
    parser_result,start,end = typedef_parser.scanString(test_data).next()
    print start, end
    print parser_result
    print
#pprint.pprint(eval(`parser_result`))

