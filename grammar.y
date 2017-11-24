%{
/*
** This is a bare-bones prototype for an ANSI C parser.
**
** It is based on _The C Programming Language,
** Second Edition_, Kernighan and Ritchie, Printice Hall, 1988.
*/
#include <sys/types.h>

#include "host.h"
#include "hash.h"
#include "files.h"
#include "il.h"
#include "nodeop.h"
#include "types.h"
#include "stmt.h"
#include "stab.h"
#include "comment.h"

/* from scan.c */
extern file_pos_t yypos;  /* in scan.c */
extern comment_block_pt fetch_comment_block(void);
extern void yield_typedef(boolean);
extern void td(void);

%}

%union {
    int 	val;
    node_t *	nod;
    symbol_t *	sym;
    typeinfo_t *typ;
    stmt_t * 	stmt;
    file_pos_t  pos;
    comment_block_pt com;
}

%token BAD_TOKEN
%token MARKER
%token <nod>    INTEGER_CONSTANT
%token <nod>    CHARACTER_CONSTANT
%token <nod>    FLOATING_CONSTANT
%token <nod>    IDENTIFIER
%token <nod>    STRING

%token <typ>    TYPEDEF_NAME

%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS DOTDOT

%token CASE DEFAULT IF SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%left  THEN
%left  ELSE

%type <val>     type_adjective
%type <val>     type_qualifier
%type <val>     type_qualifier_list
%type <val>     storage_class_specifier
%type <val>     unary_operator
%type <val>     assignment_operator
%type <val>     struct_or_union
%type <typ>     type_specifier
%type <typ>     type_name
%type <typ>     actual_type_specifier
%type <typ>     specifier_qualifier_list
%type <typ>     declaration_specifiers
%type <sym>     declaration
%type <sym>     declaration_list
%type <sym>     nested_declaration_list
%type <sym>     enumerator
%type <sym>     enumerator_list
%type <sym>     enum_specifier
%type <sym>     struct_or_union_specifier
%type <sym>     struct_declaration_list
%type <sym>     struct_declaration
%type <sym>     parameter_declaration
%type <sym>     parameter_list
%type <sym>     parameter_type_list
%type <sym>     function_definition
%type <sym>     function_head
%type <nod>     constant
%type <nod>     identifier
%type <nod>     identifier_list
%type <nod>     constant_expression
%type <nod>     primary_expression
%type <nod>     assignment_expression
%type <nod>     opt_expr
%type <nod>     expression
%type <nod>     postfix_expression
%type <nod>     unary_expression
%type <nod>     cast_expression
%type <nod>     multiplicative_expression
%type <nod>     additive_expression
%type <nod>     shift_expression
%type <nod>     relational_expression
%type <nod>     equality_expression
%type <nod>     and_expression
%type <nod>     exclusive_or_expression
%type <nod>     inclusive_or_expression
%type <nod>     logical_and_expression
%type <nod>     logical_or_expression
%type <nod>     conditional_expression
%type <nod>     argument_expression_list
%type <nod>     initializer
%type <nod>     initializer_list
%type <nod>     declarator
%type <nod>     direct_declarator
%type <nod>     init_declarator
%type <nod>     init_declarator_list
%type <nod>     untyped_declaration
%type <nod>     struct_declarator
%type <nod>     struct_declarator_list
%type <nod>     pointer
%type <nod>     abstract_declarator
%type <nod>     direct_abstract_declarator
%type <nod>     function_declarator
%type <nod>     direct_function_declarator


%type <stmt>    statement
%type <stmt>    labeled_statement
%type <stmt>    compound_statement
%type <stmt>    expression_statement
%type <stmt>    selection_statement
%type <stmt>    iteration_statement
%type <stmt>    jump_statement

%type <stmt>    statement_list
%type <stmt>    function_body

%type <pos>     .pos

%type <com>     .com


%start translation_unit
%%

/****************************************************************
 ********* Name-Space and scanner-feedback productions **********
 ****************************************************************/

/* The occurrence of a type_specifier in the input turns off
 * scanner-recognition of typedef-names as such, so that they can
 * be re-defined within a declarator-list.
 */


NS_ntd  : { yield_typedef(FALSE); }
        ;

/* Once the declarators (if any) are parsed, the scanner is returned
 * to the state where typedef-names are recognized.
 */
NS_td   : { yield_typedef(TRUE); }
        ;

/* NS_scope_push creates a new scope in the id/typedef/enum-const
 * name-space. New levels are created by function-declarators
 * and are created and destroyed by compound-statements.
 * Thus, every occurance of a function-declarator must be
 * followed, at the end of the scope of that declarator,
 * by an NS_scope_pop.
 */
NS_scope_push  : { scope_push(Unspecified_scope); td(); }
    ;
NS_scope_pop : { scope_pop(); }
    ;

NS_block_scope_push : { scope_push(Block_scope); td(); }
    ;

/* NS_struct_push creates a new name-space for a struct or union
 * NS_struct_pop finishes one.
 */
NS_struct_push : { td(); }
;

NS_struct_pop:   /* { struct_pop(); } */
;

NS_id: /* { new_declaration(name_space_decl); } */
;

/* Begin a new declaration of a parameter */
NS_new_parm: { td(); }
;

/* Remember that declarators while define typedef-names. */
NS_is_typedef:  /* { set_typedef(); } */
;

/* Finish a direct-declarator */
NS_direct_decl:           /* { direct_declarator(); } */
;

/* Finish a pointer-declarator */
NS_ptr_decl: /* { pointer_declarator(); } */
;

/* The scanner must be aware of the name-space which
 * differentiates typedef-names from identifiers. But the
 * distinction is only useful within limited scopes. In other
 * scopes the distinction may be invalid, or in cases where
 * typedef-names are not legal, the semantic-analysis phase
 * may be able to generate a better error message if the parser
 * does not flag a syntax error. We therefore use the following
 * production...
 */

identifier
        : NS_ntd TYPEDEF_NAME NS_td     {$$ = id_from_typedef($2);}
        | IDENTIFIER
        ;

/************************************************************
 *****************  The C grammar per se. *******************
 ************************************************************/

/*
 * What follows is based on the grammar in _The C Programming Language_,
 * Kernighan & Ritchie, Prentice Hall 1988. See the README file.
 */

/* The MARKER case in translation_unit is intended for use in
 * processing macro bodies to see if they are syntactically expressions.
 * This processing happens after the parse of the main translation unit.
 * MARKER is a special token that's used to delimit this usage.
 */

translation_unit
    : /* empty */
    | translation_unit external_declaration
/*
    | MARKER expression MARKER
           { process_macro_expression($2); }
 */
    ;

external_declaration
    : NS_id .com function_definition         {define_func($3,$2);}
    | NS_id .com declaration                 {typed_external_decl($3,$2);}
    | NS_id .com untyped_declaration         {$3->comment = $2;}
    ;

function_definition
    : function_head function_body       {$$ = new_func($1,$2);}
    | function_head declaration_list
        function_body { KnR_params($1, $2); $$ = new_func($1,$3);}
    ;

function_head
    : function_declarator NS_td         {$$ = function_spec(0, $1, 0);}
    | declaration_specifiers function_declarator NS_td
                                        {$$ = function_spec($1, $2, 0);}
    ;

function_body
    : compound_statement NS_scope_pop   { $$ = $1; }
    ;

declaration
    : declaration_specifiers NS_td ';'
        {$$ = novar_declaration($1);
	 if ($$) $$->comment = fetch_comment_block();}
    | declaration_specifiers init_declarator_list NS_td ';'
        {$$ = var_declaration($1, $2); $$->comment = fetch_comment_block();}
    ;

untyped_declaration
    : init_declarator_list ';'
    ;

declaration_list
    : declaration
    | declaration_list declaration      {$$ = concat_symbols($1,$2);}
    ;

declaration_specifiers
    : storage_class_specifier           {$$ = typeof_typemod($1);}
    | storage_class_specifier declaration_specifiers
        {$$ = concat_types(typeof_typemod($1), $2);}
    | type_specifier
    | type_specifier declaration_specifiers     {$$ = concat_types($1, $2);}
    | type_qualifier                            {$$ = typeof_typemod($1);}
    | type_qualifier declaration_specifiers     {$$ = concat_types(typeof_typemod($1), $2);}
    ;

storage_class_specifier
    : NS_is_typedef TYPEDEF         {$$ = TYPEMOD_TYPEDEF;}
    | EXTERN                        {$$ = TYPEMOD_EXTERN;}
    | STATIC                        {$$ = TYPEMOD_STATIC;}
    | AUTO                          {$$ = TYPEMOD_AUTO;}
    | REGISTER                      {$$ = TYPEMOD_REGISTER;}
    | INLINE                        {$$ = TYPEMOD_INLINE;}
    ;

/* Once an actual type-specifier is seen, it acts as a "trigger" to
 * turn typedef-recognition off while scanning declarators, etc.
 */
type_specifier
    : NS_ntd actual_type_specifier  {$$ = $2;}
    | type_adjective                {$$ = typeof_typemod($1);}
    ;

actual_type_specifier
    : VOID                          {$$ = typeof_void();}
    | CHAR                          {$$ = typeof_char();}
    | INT                           {$$ = typeof_int();}
    | FLOAT                         {$$ = typeof_float();}
    | DOUBLE                        {$$ = typeof_double();}
    | TYPEDEF_NAME
    | struct_or_union_specifier     {$$ = typeof_specifier($1);}
    | enum_specifier                {$$ = typeof_specifier($1);}
    ;

type_adjective
    : SHORT                         {$$ = TYPEMOD_SHORT;}
    | LONG                          {$$ = TYPEMOD_LONG;}
    | SIGNED                        {$$ = TYPEMOD_SIGNED;}
    | UNSIGNED                      {$$ = TYPEMOD_UNSIGNED;}
    ;

type_qualifier
    : CONST                         {$$ = TYPEMOD_CONST;}
    | VOLATILE                      {$$ = TYPEMOD_VOLATILE;}
    ;

struct_or_union_specifier
    : struct_or_union NS_struct_push
      '{' struct_declaration_list NS_struct_pop '}' { $$ = anonymous_rec($1, $4);}
    | struct_or_union identifier NS_struct_push
      '{' struct_declaration_list NS_struct_pop '}' { $$ = named_rec($1, $2, $5);}
    | struct_or_union identifier                    { $$ = rec_reference($1, $2);}
    ;

struct_or_union
    : STRUCT                        {$$ = 0;}
    | UNION                         {$$ = 1;}
    ;

struct_declaration_list
    : struct_declaration
    | struct_declaration_list struct_declaration    {$$ = concat_symbols($1,$2);}
    ;

init_declarator_list
    : init_declarator
    | init_declarator_list ',' init_declarator  {$$ = new_node(_List, $1, $3);}
    ;

init_declarator
    : declarator
    | declarator NS_td '=' initializer NS_ntd   {$$ = new_node(_Assign, $1, $4);}
    ;

struct_declaration
    : /* { new_declaration(struct_decl); } */
          specifier_qualifier_list struct_declarator_list NS_td ';'
        {$$ = field_declaration($1, $2);}
    ;

specifier_qualifier_list
    : type_specifier                            {$$ = typeof_typespec($1);}
    | type_specifier specifier_qualifier_list   {$$ = typeof_typespec(concat_types($1, $2));}
    | type_qualifier                            {$$ = typeof_typespec(typeof_typemod($1));}
    | type_qualifier specifier_qualifier_list   {$$ = typeof_typespec(concat_types(typeof_typemod($1),$2));}
    ;

struct_declarator_list
    : struct_declarator
    | struct_declarator_list ',' struct_declarator  {$$ = new_node(_List, $1, $3);}
    ;

struct_declarator
    : declarator
    | ':' constant_expression                       {$$ = new_node(_Bit_Field, 0, $2);}
    | declarator ':' constant_expression            {$$ = new_node(_Bit_Field, $1, $3);}
    ;

enum_specifier
    : ENUM '{' enumerator_list opt_comma '}'        {$$ = anonymous_enum($3);}
    | ENUM identifier '{' enumerator_list opt_comma '}'
                                                    {$$ = named_enum($2, $4);}
    | ENUM identifier                               {$$ = enum_reference($2);}
    ;

enumerator_list
    : enumerator
    | enumerator_list ',' enumerator                {$$ = concat_symbols($1,$3);}
    ;

enumerator
    : IDENTIFIER                                    {$$ = grok_enumerator($1,0);}
    | IDENTIFIER '=' constant_expression            {$$ = grok_enumerator($1,$3);}
    ;

opt_comma
    : ','
    | /*nothing*/
    ;

declarator
    : direct_declarator  NS_direct_decl
    | pointer direct_declarator NS_ptr_decl         {$$ = access_to($1, $2);}
    ;

/* Note: ')' NS_scope_pop  is used rather than  rpar  to avoid
 * shift-reduce conflicts with the productions in direct_function_declarator.
 */

direct_declarator
    : IDENTIFIER
    | lpar declarator ')' NS_scope_pop
                           {$$ = $2;}
    | direct_declarator lbra rbra
                           {$$ = new_node(_Array_Index, $1, 0);}
    | direct_declarator lbra constant_expression rbra
                           {$$ = new_node(_Array_Index, $1, $3);}
    | direct_declarator lpar parameter_type_list ')' NS_scope_pop
                           {$$ = new_node(_Func_Call, $1, new_node(_Sym,$3));}
    | direct_declarator lpar ')' NS_scope_pop
                           {$$ = new_node(_Func_Call, $1, 0);}
    | direct_declarator lpar identifier_list ')' NS_scope_pop
                           {$$ = new_node(_Func_Call, $1, $3);}
    ;


function_declarator
    : direct_function_declarator NS_direct_decl
    | pointer direct_function_declarator NS_ptr_decl
        {$$ = access_to($1, $2);}
    ;

/*
 * In direct_function_declarator, note the assymetry between lpar and
 * ').  the lpar production opens a scope. This scope is closed by
 * NS_pop_scope in function_body -- function_declarator is used only
 * in the production function_head, and function_head and function_body
 * are paired in function_definition.  The underlying semantics is that
 * the formal parameters of a function are in scope in the function's
 * body.
 */

direct_function_declarator
    : direct_declarator lpar parameter_type_list ')'
        {$$ = new_node(_Func_Call, $1, new_node(_Sym,$3));}
    | direct_declarator lpar ')'
        {$$ = new_node(_Func_Call, $1, 0);}
    | direct_declarator lpar identifier_list ')'
        {$$ = new_node(_Func_Call, $1, $3);}
    ;

pointer
    : '*'                               {$$ = new_node(_Indirect, 0);}
    | '*' type_qualifier_list           {$$ = new_node(_Indirect, 0);}
    | '*' pointer                       {$$ = new_node(_Indirect, $2);}
    | '*' type_qualifier_list pointer   {$$ = new_node(_Indirect, $3);}
    ;

type_qualifier_list
    : type_qualifier
    | type_qualifier_list type_qualifier        {$$ = $1 | $2;}
    ;

parameter_type_list
    : parameter_list            	{$$ = $1;}
    | parameter_list ',' ELLIPSIS       {$$ = concat_ellipsis($1);}
    ;

parameter_list
    : parameter_declaration
    | parameter_list ',' parameter_declaration      {$$ = concat_symbols($1,$3);}
    ;

parameter_declaration
    :  NS_new_parm declaration_specifiers declarator NS_td
        {$$ = named_abstract_param($2, $3);}
    |  NS_new_parm declaration_specifiers NS_td
        {$$ = noname_simple_param($2);}
    |  NS_new_parm declaration_specifiers abstract_declarator NS_td
        {$$ = noname_abstract_param($2, $3);}
    ;

identifier_list
    : IDENTIFIER
    | identifier_list ',' IDENTIFIER            {$$ = new_node(_List, $1, $3);}
    ;

initializer
    : assignment_expression
    | '{' initializer_list '}'
                         {$$ = new_node(_Aggregate,reshape_list($2));}
    | '{' initializer_list ',' '}'
                         {$$ = new_node(_Aggregate,reshape_list($2));}
    ;

initializer_list
    : initializer
    | initializer_list ',' initializer          {$$ = new_node(_List, $1, $3);}
    ;

type_name
    : specifier_qualifier_list NS_td
    | specifier_qualifier_list NS_td abstract_declarator
        { $$ = abstract_declarator_type( $1, $3 ); }
    ;

abstract_declarator
    : pointer
    | direct_abstract_declarator
    | pointer direct_abstract_declarator                        {$$ = access_to($1, $2);}
    ;

direct_abstract_declarator
    : lpar abstract_declarator rpar
                           {$$ = $2;}
    | lbra rbra
                           {$$ = new_node(_Array_Index, 0, 0);}
    | lbra constant_expression rbra
                           {$$ = new_node(_Array_Index, 0, $2);}
    | direct_abstract_declarator lbra rbra
                           {$$ = new_node(_Array_Index, $1, 0);}
    | direct_abstract_declarator lbra constant_expression rbra
                           {$$ = new_node(_Array_Index, $1, $3);}
    | lpar rpar
                           {$$ = new_node(_Func_Call, 0, 0);}
    | lpar  parameter_type_list rpar
                           {$$ = new_node(_Func_Call, 0, new_node(_Sym,$2));}
    | direct_abstract_declarator lpar rpar
                           {$$ = new_node(_Func_Call, $1, 0);}
    | direct_abstract_declarator lpar  parameter_type_list rpar
                           {$$ = new_node(_Func_Call, $1, new_node(_Sym,$3));}
    ;

lpar
    : NS_scope_push '('
    ;

rpar
    : NS_scope_pop ')'
    ;

lbra
    : NS_td  '['
    ;

rbra
    : NS_ntd ']'
    ;

statement
    : labeled_statement
    | compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    ;

labeled_statement
    : identifier ':' .com statement
		{ $$ = new_stmt_Labelled( $1, $3, $4 ); }
    | CASE .pos .com constant_expression ':' statement
		{$$ = new_stmt_Case( $2, $3, $4, $6 );}
    | DEFAULT .pos .com ':' statement
		{$$ = new_stmt_Default( $2, $3, $5 );}

    ;

expression_statement
    : ';' .pos          {$$ = new_stmt_Null($2);}
    | expression ';'    {$$ = new_stmt_Expr( $1);}
    ;

begin_compound
    : NS_block_scope_push '{'
    ;

compound_statement
    : begin_compound .pos NS_scope_pop '}' {
        $$ = new_stmt_Compound( $2, (symbol_pt)0, (stmt_pt)0 );
    }
    | begin_compound .pos statement_list NS_scope_pop '}' {
        $$ = new_stmt_Compound( $2, (symbol_pt)0, $3 );
    }
    | begin_compound .pos nested_declaration_list NS_scope_pop '}' {
        $$ = new_stmt_Compound( $2, $3, (stmt_pt)0 );
    }
    | begin_compound .pos nested_declaration_list statement_list NS_scope_pop '}' {
        $$ = new_stmt_Compound( $2, $3, $4 );
    }
    ;

/* TBD: nested_declaration_list causes a reduce/reduce conflict in the
 * grammar, on the token TYPEDEF_NAME: the grammar doesn't know whether
 * to reduce NS_ntd at the start of the "identifier" rule, or to perform
 * this reduction. (It chooses NS_ntd.)  It's not clear that this causes
 * any problems, and it's very convenient to have nested_declaration_list
 * so we can put all the declared symbols in the symbol table before
 * we start parsing statements, so I've let the conflict stand. -- rgh
 */

nested_declaration_list
    : declaration_list   { $$ = nested_declarations($1); }

statement_list
    : statement                 {$$ = new_stmt_list($1); }
    | statement_list statement  {$$ = append_stmt( $1, $2 );}
    ;

selection_statement
    : IF .pos .com '(' expression ')' statement %prec THEN {
        $$ = new_stmt_If( $2, $3, $5, $7 );
    }
    | IF .pos .com '(' expression ')' statement ELSE statement {
        $$ = new_stmt_Ifelse( $2, $3, $5, $7, $9 );
    }
    | SWITCH .pos .com  '(' expression ')' statement {
        $$ = new_stmt_Switch( $2, $3, $5, $7 );
    }
    ;

iteration_statement
    : WHILE .pos .com '(' expression ')' statement
		{$$ = new_stmt_While( $2, $3, $5, $7 );}
    | DO .pos .com statement WHILE '(' expression ')' ';'
		{$$ = new_stmt_Do( $2, $3, $7, $4 );}
    | FOR .pos .com  '(' opt_expr ';' opt_expr ';' opt_expr ')' statement
	        { $$ = new_stmt_For( $2, $3, $5, $7, $9, $11 ); }

opt_expr
    : expression
    | /* nothing */ { $$ = (node_t *) 0; }



jump_statement
    : GOTO .pos identifier ';'   { $$ = new_stmt_Goto( $2, $3 ); }
    | CONTINUE .pos ';'          { $$ = new_stmt_Continue( $2 ); }
    | BREAK .pos ';'             { $$ = new_stmt_Break( $2 ); }
    | RETURN .pos ';'            { $$ = new_stmt_Return( $2, (node_t*)0 ); }
    | RETURN .pos expression ';' { $$ = new_stmt_Return( $2, $3 ); }
    ;

expression
    : assignment_expression
    | expression ',' assignment_expression
        {$$ = new_node(_Comma, $1, $3);}
    ;


assignment_expression
    : conditional_expression
    | unary_expression assignment_operator assignment_expression
        {$$ = new_node($2, $1, $3);}
    ;

assignment_operator
    : '='               {$$ = _Assign;}
    | MUL_ASSIGN        {$$ = _Mul_Assign;}
    | DIV_ASSIGN        {$$ = _Div_Assign;}
    | MOD_ASSIGN        {$$ = _Mod_Assign;}
    | ADD_ASSIGN        {$$ = _Add_Assign;}
    | SUB_ASSIGN        {$$ = _Sub_Assign;}
    | LEFT_ASSIGN       {$$ = _Shl_Assign;}
    | RIGHT_ASSIGN      {$$ = _Shr_Assign;}
    | AND_ASSIGN        {$$ = _Band_Assign;}
    | XOR_ASSIGN        {$$ = _Xor_Assign;}
    | OR_ASSIGN         {$$ = _Bor_Assign;}
    ;


conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression
        {$$ = new_node(_Cond, $1, $3, $5);}
    ;


constant_expression
    : conditional_expression
    ;


logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OP logical_and_expression    {$$ = new_node(_Lor, $1, $3);}
    ;

logical_and_expression
    : inclusive_or_expression
    | logical_and_expression AND_OP inclusive_or_expression {$$ = new_node(_Land, $1, $3);}
    ;

inclusive_or_expression
    : exclusive_or_expression
    | inclusive_or_expression '|' exclusive_or_expression   {$$ = new_node(_Bor, $1, $3);}
    ;

exclusive_or_expression
    : and_expression
    | exclusive_or_expression '^' and_expression        {$$ = new_node(_Xor, $1, $3);}
    ;

and_expression
    : equality_expression
    | and_expression '&' equality_expression            {$$ = new_node(_Band, $1, $3);}
    ;

equality_expression
    : relational_expression
    | equality_expression EQ_OP relational_expression   {$$ = new_node(_Eq, $1, $3);}
    | equality_expression NE_OP relational_expression   {$$ = new_node(_Ne, $1, $3);}
    ;

relational_expression
    : shift_expression
    | relational_expression '<' shift_expression        {$$ = new_node(_Lt, $1, $3);}
    | relational_expression '>' shift_expression        {$$ = new_node(_Gt, $1, $3);}
    | relational_expression LE_OP shift_expression      {$$ = new_node(_Le, $1, $3);}
    | relational_expression GE_OP shift_expression      {$$ = new_node(_Ge, $1, $3);}
    ;

shift_expression
    : additive_expression
    | shift_expression LEFT_OP additive_expression      {$$ = new_node(_Shl, $1, $3);}
    | shift_expression RIGHT_OP additive_expression     {$$ = new_node(_Shr, $1, $3);}
    ;

additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression {$$ = new_node(_Add, $1, $3);}
    | additive_expression '-' multiplicative_expression {$$ = new_node(_Sub, $1, $3);}
    ;

multiplicative_expression
    : cast_expression
    | multiplicative_expression '*' cast_expression     {$$ = new_node(_Mul, $1, $3);}
    | multiplicative_expression '/' cast_expression     {$$ = new_node(_Div, $1, $3);}
    | multiplicative_expression '%' cast_expression     {$$ = new_node(_Rem, $1, $3);}
    ;

cast_expression
    : unary_expression
    | '(' type_name ')' cast_expression {$$ = new_node(_Type_Cast, new_node(_Type, $2), $4);}
    ;


unary_expression
    : postfix_expression
    | INC_OP unary_expression           {$$ = new_node(_Pre_Inc, $2);}
    | DEC_OP unary_expression           {$$ = new_node(_Pre_Dec, $2);}
    | unary_operator cast_expression    {$$ = new_node($1, $2);}
    | SIZEOF unary_expression           {$$ = new_node(_Sizeof, $2);}
    | SIZEOF '(' type_name ')'          {$$ = new_node(_Sizeof, new_node(_Type, $3));}
    ;


unary_operator
    : '&'       {$$ = _Addrof;}
    | '*'       {$$ = _Indirect;}
    | '+'       {$$ = _Unary_Plus;}
    | '-'       {$$ = _Unary_Minus;}
    | '~'       {$$ = _Ones_Complement;}
    | '!'       {$$ = _Not;}
    ;


postfix_expression
    : primary_expression

    | postfix_expression '[' expression ']'
                            {$$ = new_node(_Array_Index, $1, $3);}

    | postfix_expression '(' ')'
                            {$$ = new_node(_Func_Call, $1, 0);}

    | postfix_expression '(' argument_expression_list ')'
                            {$$ = new_node(_Func_Call, $1, reshape_list($3));}

    | postfix_expression '.' identifier
                            {$$ = new_node(_Dot_Selected, $1, $3);}

    | postfix_expression PTR_OP identifier
                            {$$ = new_node(_Arrow_Selected, $1, $3);}

    | postfix_expression INC_OP
                             {$$ = new_node(_Post_Inc, $1);}

    | postfix_expression DEC_OP
                             {$$ = new_node(_Post_Dec, $1);}
    ;

primary_expression
    : IDENTIFIER       { $$ = bind_to_sym( $1 ); }
    | constant
    | STRING
    | '(' expression ')'                                    {$$ = $2;}
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list ',' assignment_expression    {$$ = new_node(_List, $1, $3);}
    ;

constant
    : INTEGER_CONSTANT
    | CHARACTER_CONSTANT
    | FLOATING_CONSTANT
    ;

/* This production lets us figure out the location of keywords
 * for labelling statements with the proper position.
 */

.pos
    : /*nothing*/  { $$ = yypos; }
    ;

/* This production lets us grab block comments at the appropriate point.
 */

.com
    : /*nothing*/  { $$ = fetch_comment_block(); }
    ;



%%


