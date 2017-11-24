#include <stdio.h>

#include "gen_stmt.h"
#include "gen_expr.h"
#include "print.h"

#define NEXT 2		/* next indentation level */
#define MAXPRINT	1000

static int nprinted = 0;

static void
spaces(n)
    int n;
{
    int i;

    for(i=1; i<=n; i++)
	if((i > 8) && (i % 4) == 0)
	    putchar('|');
	else
	    putchar(' ');
}

static int
compar(left, right)
    char **left, **right;
{
    return(*left != *right);
}

static int 		/* return 1: already printed, 0: not yet */
print_addr(s, addr, indent)
    char *s;
    int addr, indent;
{
    int tmp;
    char *res;
    extern char *lsearch();
    static char *printed_addrs[MAXPRINT];

    spaces(indent);
    printf("%s address=%x\n", s, addr);

    tmp = nprinted;
    res = lsearch(&addr, printed_addrs, &nprinted, sizeof(char *), compar);
    return (nprinted == tmp);
}

static void print_stmt_t(stmt_pt s, int indent);
static void print_node_t(node_pt n, int indent);
static void print_symbol_t(symbol_pt s, int indent);
static void print_typeinfo_t(typeinfo_t *t, int indent);

static void
print_unknown(s, val)
    char *s;
    int val;
{
    printf("unknown %s %d", s, val);
}


char*
nameof_stmt_kind(stmt_kind_t s)
{
    switch(s) {
        case _Labelled: return "_Labelled";
        case _Case: return "_Case";
        case _Default: return "_Default";
        case _Compound: return "_Compound";
        case _SList: return "_SList";
        case _Expr: return "_Expr";
        case _If: return "_If";
        case _Ifelse: return "_Ifelse";
        case _Switch: return "_Switch";
        case _While: return "_While";
        case _Do: return "_Do";
        case _For: return "_For";
        case _Goto: return "_Goto";
        case _Continue: return "_Continue";
        case _Break: return "_Break";
        case _Return: return "_Return";
        case _Null: return "_Null";
        case _FuncDef: return "_FuncDef";
	case _MacroBody: return "_MacroBody";
	default: return "<?" "?" "?>";
    }
}


char *
nameof_typekind(typekind_t t)

{
    switch(t) {
        case pointer_to: return "pointer_to";
        case array_of: return "array_of";
        case struct_of: return "struct_of";
        case union_of: return "union_of";
        case field_type: return "field_type";
        case int_type: return "int_type";
        case float_type: return "float_type";
        case void_type: return "void_type";
        case function_type: return "function_type";
        case enum_type: return "enum_type";
        case typemodifier: return "typemodifier";
	default: return "<?" "?" "?>";
    }
}

char *
nameof_node_kind(node_kind_t n)
{
    switch(n) {
	case _Error: return "_Error";
	case _Ellipsis: return "_Ellipsis";
	case _FP_Number: return "_FP_Number";
	case _Int_Number: return "_Int_Number";
	case _Type: return "_Type";
	case _Sym: return "_Sym";
	case _Ident: return "_Ident";
	case _Macro_ID: return "_Macro_ID";
	case _String: return "_String";
	case _List: return "_List";
	case _Comma: return "_Comma";
	case _Bit_Field: return "_Bit_Field";
	case _Dot_Selected: return "_Dot_Selected";
	case _Arrow_Selected: return "_Arrow_Selected";
	case _Array_Index: return "_Array_Index";
	case _Func_Call: return "_Func_Call";
	case _Type_Cast: return "_Type_Cast";
	case _Assign: return "_Assign";
	case _Mul_Assign: return "_Mul_Assign";
	case _Div_Assign: return "_Div_Assign";
	case _Mod_Assign: return "_Mod_Assign";
	case _Add_Assign: return "_Add_Assign";
	case _Sub_Assign: return "_Sub_Assign";
	case _Shl_Assign: return "_Shl_Assign";
	case _Shr_Assign: return "_Shr_Assign";
	case _Band_Assign: return "_Band_Assign";
	case _Xor_Assign: return "_Xor_Assign";
	case _Bor_Assign: return "_Bor_Assign";
	case _Eq: return "_Eq";
	case _Ne: return "_Ne";
	case _Lt: return "_Lt";
	case _Le: return "_Le";
	case _Gt: return "_Gt";
	case _Ge: return "_Ge";
	case _Land: return "_Land";
	case _Lor: return "_Lor";
	case _Band: return "_Band";
	case _Bor: return "_Bor";
	case _Xor: return "_Xor";
	case _Add: return "_Add";
	case _Sub: return "_Sub";
	case _Mul: return "_Mul";
	case _Div: return "_Div";
	case _Rem: return "_Rem";
	case _Shl: return "_Shl";
	case _Shr: return "_Shr";
	case _Exp: return "_Exp";
	case _Sizeof: return "_Sizeof";
	case _Pre_Inc: return "_Pre_Inc";
	case _Pre_Dec: return "_Pre_Dec";
	case _Post_Inc: return "_Post_Inc";
	case _Post_Dec: return "_Post_Dec";
	case _Addrof: return "_Addrof";
	case _Unary_Plus: return "_Unary_Plus";
	case _Unary_Minus: return "_Unary_Minus";
	case _Ones_Complement: return "_Ones_Complement";
	case _Not: return "_Not";
	case _Aggregate: return "_Aggregate";
	case _Indirect: return "_Indirect";
	case bool: return "_Bool";
	case _UnBool: return "_UnBool";
	case _Cond: return "_Cond";
	default: return "<?" "?" "?>";
    }
}

char *
nameof_sym_kind(sym_kind_t s)
{
    switch(s) {
	case type_symbol: return "type_symbol";
	case func_symbol: return "func_symbol";
	case param_symbol: return "param_symbol";
	case var_symbol: return "var_symbol";
	case enum_literal: return "enum_literal";
	default: return "<?" "?" "?>";
    }
}

char *
nameof_cpp_eval_result_kind_t(cpp_eval_result_kind_t res)
{
    switch(res) {
	case eval_failed: return "eval_failed";
	case eval_int: return "eval_int";
	case eval_float: return "eval_float";
	case eval_string: return "eval_string";
	case eval_type:return "eval_type";
	default: return "???";
    }

}

static void
print_stmt_t(stmt_pt s, int indent)
{
    if(s == NULL) { spaces(indent); printf("(null)\n"); return; }
    if(print_addr("stmt", s, indent)) return;
    spaces(indent);
    printf("stmt_def line=%d file=%s\n",
	    (int) line_number(s->stmt_def),
            file_name(s->stmt_def));
    indent += NEXT;
    spaces(indent);
    printf("kind: ");
    print_stmt_kind(s->stmt_kind);
    printf("\n");

    switch(s->stmt_kind) {
        case _Labelled:
        case _Case:
	    spaces(indent);
	    printf("label.id:\n");
	    print_node_t(s->stmt.label.id, indent+NEXT);
	    spaces(indent);
	    printf("label.stmt:\n");
	    print_stmt_t(s->stmt.label.stmt, indent+NEXT);
	    break;

        case _Default:
	    spaces(indent);
	    printf("default_stmt:\n");
	    print_stmt_t(s->stmt.default_stmt, indent+NEXT);
	    break;

        case _SList:
	    spaces(indent);
	    printf("stmt_list.first:\n");
	    print_stmt_t(s->stmt.stmt_list.first, indent+NEXT);
	    spaces(indent);
	    printf("stmt_list.rest:\n");
	    print_stmt_t(s->stmt.stmt_list.rest, indent+NEXT);
	    break;

        case _FuncDef:
	    spaces(indent);
	    printf("funcdef.decl:\n");
	    print_symbol_t(s->stmt.funcdef.decl, indent+NEXT);
	    spaces(indent);
	    printf("funcdef.body:\n");
	    print_stmt_t(s->stmt.funcdef.body, indent+NEXT);
	    break;

        case _Compound:
	    spaces(indent);
	    printf("compound.decls:\n");
	    print_symbol_t(s->stmt.compound.decls, indent+NEXT);
	    spaces(indent);
	    printf("compound.stmts:\n");
	    print_stmt_t(s->stmt.compound.stmts, indent+NEXT);
	    break;

        case _Expr:
	    spaces(indent);
            printf("expr:\n");
            print_node_t(s->stmt.expr, indent+NEXT);
	    break;

        case _If:
        case _Switch:
        case _While:
        case _Do:
	    spaces(indent);
            printf("controlled.expr:\n");
            print_node_t(s->stmt.controlled.expr, indent+NEXT);
            spaces(indent);
            printf("controlled.stmt:\n");
            print_stmt_t(s->stmt.controlled.stmt, indent+NEXT);
	    break;

        case _For:
	    spaces(indent);
            printf("for_stmt.e1:\n");
            print_node_t(s->stmt.for_stmt.e1, indent+NEXT);
            spaces(indent);
            printf("for_stmt.e2:\n");
            print_node_t(s->stmt.for_stmt.e2, indent+NEXT);
            spaces(indent);
            printf("for_stmt.e3:\n");
            print_node_t(s->stmt.for_stmt.e3, indent+NEXT);
            spaces(indent);
            printf("for_stmt.stmt:\n");
            print_stmt_t(s->stmt.for_stmt.stmt, indent+NEXT);
	    break;

        case _Goto:
	    spaces(indent);
            printf("goto_label:\n");
            print_node_t(s->stmt.goto_label, indent+NEXT);
	    break;

        case _Return:
	    spaces(indent);
            printf("return_value:\n");
            print_node_t(s->stmt.return_value, indent+NEXT);
	    break;

        case _Ifelse:
	    spaces(indent);
            printf("if_else_stmt.expr:\n");
            print_node_t(s->stmt.if_else_stmt.expr, indent+NEXT);
            printf("if_else_stmt.then_stmt:\n");
            print_stmt_t(s->stmt.if_else_stmt.then_stmt, indent+NEXT);
            printf("if_else_stmt.else_stmt:\n");
            print_stmt_t(s->stmt.if_else_stmt.else_stmt, indent+NEXT);
	    break;

        case _Null:
        case _Continue:
        case _Break:
        default:
	    break;
    }
}

static void
print_node_t(node_pt n, int indent)
{
    if(n == NULL) { spaces(indent); printf("(null)\n"); return; }
    if(print_addr("node", n, indent)) return;
    spaces(indent);
    printf("node_def line=%d file=%s\n",
	    (int) line_number(n->node_def),
            file_name(n->node_def));
    indent += NEXT;
    spaces(indent);
    printf("node_kind: ");
    print_node_kind(n->node_kind);
    printf("\n");
    spaces(indent);
    switch (n->node_kind) {
	case _Error:
	    printf("error\n");
	    break;

	case _Ellipsis:
	    printf("error\n");
	    break;

	case _FP_Number:
	    printf("node.fval %f\n", n->node.fval);
	    break;

	case _Int_Number:
	    printf("node.ival %d\n", (int) n->node.ival);
	    break;

	case _Type:
	    printf("node.typ:\n");
	    print_typeinfo_t(n->node.typ, indent+NEXT);
	    break;

	case _Sym:
	    printf("node.sym:\n");
	    print_symbol_t(n->node.sym, (int) indent+NEXT);
	    break;

	case _Macro_ID:
	    break;

	case _Ident:
	    printf("node.id %s %s\n", n->node.id.name, n->node.id.cmnt);
	    printf("node.id.sym:\n");
	    print_symbol_t(n->node.id.sym, indent+NEXT);
	    break;

	case _String:
	    printf("node.str %s %d\n", n->node.str.form, n->node.str.len);
	    break;

	case _List:		/* binary */
	case _Comma:
	case _Bit_Field:
	case _Dot_Selected:
	case _Arrow_Selected:
	case _Array_Index:
	case _Func_Call:
	case _Type_Cast:
	case _Assign:
	case _Mul_Assign:
	case _Div_Assign:
	case _Mod_Assign:
	case _Add_Assign:
	case _Sub_Assign:
	case _Shl_Assign:
	case _Shr_Assign:
	case _Band_Assign:
	case _Xor_Assign:
	case _Bor_Assign:
	case _Eq:
	case _Ne:
	case _Lt:
	case _Le:
	case _Gt:
	case _Ge:
	case _Land:
	case _Lor:
	case _Band:
	case _Bor:
	case _Xor:
	case _Add:
	case _Sub:
	case _Mul:
	case _Div:
	case _Rem:
	case _Shl:
	case _Shr:
	    printf("node.binary.l:\n");
	    print_node_t(n->node.binary.l, indent+NEXT);
	    spaces(indent);
	    printf("node.binary.r:\n");
	    print_node_t(n->node.binary.r, indent+NEXT);
	    break;

	case _Sizeof:		/* unary */
	case _Pre_Inc:
	case _Pre_Dec:
	case _Post_Inc:
	case _Post_Dec:
	case _Addrof:
	case _Unary_Plus:
	case _Unary_Minus:
	case _Ones_Complement:
	case _Not:
	case _Aggregate:
	case _Indirect:
	    printf("node.unary:\n");
	    print_node_t(n->node.unary, indent+NEXT);
	    break;

	case _Cond:
	    printf("node.cond.bool:\n");
	    print_node_t(n->node.cond.bool, indent+NEXT);
	    spaces(indent);
	    printf("node.cond.tru:\n");
	    print_node_t(n->node.cond.tru, indent+NEXT);
	    spaces(indent);
	    printf("node.cond.fals:\n");
	    print_node_t(n->node.cond.fals, indent+NEXT);
	    break;

	default:
	    printf("unknown node\n");
    }
    spaces(indent);
    printf("baseval %d\n", n->baseval);
}

static void
print_uns_pair(s, i, indent)
    char *s;
    unsigned i;
    int indent;
{
    if(i == 0) return;		/* don't print 0 int fields */
    spaces(indent);
    printf("%s\t%u\n", s, i);
}

static void
print_typeinfo_t(typeinfo_t *t, int indent)
{
    if(t == NULL) { spaces(indent); printf("(null)\n"); return; }
    if(print_addr("typeinfo", t, indent)) return;
    spaces(indent);
    printf("typeinfo:\n");
    indent += NEXT;
    spaces(indent);
    printf("type_kind:");
    print_typekind(t->type_kind);
    printf("\n");
    print_uns_pair("_unsigned", t->_unsigned, indent);
    print_uns_pair("_signed", t->_signed, indent);
    print_uns_pair("_short", t->_short, indent);
    print_uns_pair("_long\t", t->_long, indent);
    print_uns_pair("_long_long", t->_long_long, indent);
    print_uns_pair("_volatile", t->_volatile, indent);
    print_uns_pair("_constant", t->_constant, indent);
    print_uns_pair("_extern", t->_extern, indent);
    print_uns_pair("_static", t->_static, indent);
    print_uns_pair("_auto\t", t->_auto, indent);
    print_uns_pair("_register", t->_register, indent);
    print_uns_pair("_typedef", t->_typedef, indent);
    print_uns_pair("_builtin", t->_builtin, indent);
    print_uns_pair("_anonymous", t->_anonymous, indent);
    print_uns_pair("_anon_int", t->_anon_int, indent);
    switch(t->type_kind) {
	case array_of:
	    print_uns_pair("array.elements",
			   t->type_info.array.elements, indent);
	    printf("typeinfo.array.size_expr");
	    print_node_t(t->type_info.array.size_expr, indent+NEXT);
	    break;
	case struct_of:
	    spaces(indent);
	    printf("type_info.struct_fields:\n");
	    print_typeinfo_t(t->type_info.struct_fields, indent+NEXT);
	    break;
	default:
	    break;
    }
    print_uns_pair("_sizeof", t->_sizeof, indent);
    print_uns_pair("_alignof", t->_alignof, indent);
    /* print_uns_pair("type_hash", t->type_hash, indent);  boring */
    spaces(indent);
    printf("type_base:\n");
    print_symbol_t(t->type_base, indent+NEXT);
    spaces(indent);
    printf("type_anonymous_list:\n");
    print_typeinfo_t(t->type_anonymous_list, indent+NEXT);
    spaces(indent);
    printf("type_next:\n");
    print_typeinfo_t(t->type_next, indent+NEXT);
}

static void
print_symbol_t(symbol_pt s, int indent)
{
    if(s == NULL) { spaces(indent); printf("(null)\n"); return; }
    if(print_addr("symbol", s, indent)) return;
    spaces(indent);
    printf("sym_kind: ");
    print_sym_kind(s->sym_kind);
    printf("\n");
    indent += NEXT;
    print_uns_pair("sym_scope", s->sym_scope, indent);
    print_uns_pair("intrinsic", s->intrinsic, indent);
    print_uns_pair("_volatile", s->_volatile, indent);
    print_uns_pair("_const", s->_const, indent);
    print_uns_pair("_created_name", s->_created_name, indent);
    print_uns_pair("_created_by_reference", s->_created_by_reference, indent);
    print_uns_pair("has_initializer", s->has_initializer, indent);
    print_uns_pair("gened", s->gened, indent);
    print_uns_pair("cleared", s->cleared, indent);
    print_uns_pair("stored", s->stored, indent);
    print_uns_pair("interfaced", s->interfaced, indent);
    print_uns_pair("emitted", s->emitted, indent);
    print_uns_pair("traversal_unit", s->traversal_unit, indent);
    spaces(indent);
    /* printf("sym_ident:\n");
	print_node_t(s->sym_ident, indent+NEXT);
	spaces(indent); */
    printf("sym_ada_name %s\n", s->sym_ada_name);
    spaces(indent);
    printf("sym_def line=%d file=%s\n", (int) line_number(s->sym_def),
	    file_name(s->sym_def));
    spaces(indent);
    printf("sym_type:\n");
    print_typeinfo_t(s->sym_type, indent+NEXT);
    print_uns_pair("bitoffset", s->bitoffset, indent);
    spaces(indent);
    printf("sym_value");
    switch(s->sym_kind) {
	case type_symbol:		/* ? */
	case param_symbol:		/* ? */
	case enum_literal:
	    printf(".intval %x\n", (unsigned int) s->sym_value.intval);
	    break;
	case func_symbol:
	    printf(".body:\n");
	    print_stmt_t(s->sym_value.body, indent+NEXT);
	    break;
	case var_symbol:
            printf(".initializer:\n");
	    if(s->has_initializer)
		print_node_t(s->sym_value.initializer, indent+NEXT);
	    break;
	default:
	    printf("\n");
	    break;
    }
    spaces(indent);
    printf("sym_tags:\n");
    print_symbol_t(s->sym_tags, indent+NEXT);
    spaces(indent);
    printf("sym_parse_list:\n");
    print_symbol_t(s->sym_parse_list, indent+NEXT);
    spaces(indent);
    printf("sym_scope_list:\n");
    print_symbol_t(s->sym_scope_list, indent+NEXT);
    spaces(indent);
    printf("sym_gen_list:\n");
    print_symbol_t(s->sym_gen_list, indent+NEXT);
    /* probably not interested in hash chains */
}

/* Externally visible */
void
print_stmt_kind(stmt_kind_t s)
{
    printf("%s", nameof_stmt_kind(s));
}

void
print_stmt(stmt_pt s, int indent)
{
    nprinted = 0;
    print_stmt_t(s, indent);
}

void
print_node(node_pt n, int indent)
{
    nprinted = 0;
    print_node_t(n, indent);
}

void
print_symbol(symbol_pt s, int indent)
{
    nprinted = 0;
    print_symbol_t(s, indent);
}

void
print_typekind(typekind_t t)
{
    printf("%s", nameof_typekind(t));
}

void
print_node_kind(node_kind_t n)
{
    printf("%s", nameof_node_kind(n));
}

void
print_typeinfo(typeinfo_t *t, int indent)
{
    nprinted = 0;
    print_typeinfo_t(t, indent);
}

void
print_sym_kind(sym_kind_t s)
{
    printf("%s", nameof_sym_kind(s));
}

void
print_case_alist(case_alist_pt ap, int indent)
{
    spaces(indent);
    if(ap == NULL) { printf("(null)\n"); return; }
    printf("case_alist at %lx:\n", (unsigned long int) ap);
    print_node(ap->exp, indent+NEXT);
    spaces(indent);
    printf("case_alist.rest:\n");
    print_case_alist(ap->rest, indent+NEXT);
}

void
print_case_slist(case_slist_pt sp, int indent)
{
    spaces(indent);
    if(sp == NULL) { printf("(null)\n"); return; }
    printf("case_slist at %lx:\n", (unsigned long int) sp);
    print_stmt(sp->stm, indent+NEXT);
    spaces(indent);
    printf("case_slist.rest:\n");
    print_case_slist(sp->rest, indent+NEXT);
}

void
print_case_blist(case_blist_pt bp, int indent)
{
    spaces(indent);
    if(bp == NULL) { printf("(null)\n"); return; }
    printf("case_blist at %lx:\n", (unsigned long int) bp);
    print_case_alist(bp->alts, indent+NEXT);
    spaces(indent);
    printf("case_blist.stms:\n");
    print_case_slist(bp->stms, indent+NEXT);
    spaces(indent);
    printf("case_blist.rest:\n");
    print_case_blist(bp->rest, indent+NEXT);
    print_uns_pair("has_default", bp->has_default, indent+NEXT);
    spaces(indent);
    printf("case_blist.last_stmt at %lx:\n", (unsigned long int) bp->last_stmt);
}

void
print_case_stmt(case_stmt_pt cp, int indent)
{
    spaces(indent);
    if(cp == NULL) { printf("(null)\n"); return; }
    printf("case_stmt.exp:\n");
    print_node(cp->exp, indent+NEXT);
    spaces(indent);
    printf("case_stmt.branches:\n");
    print_case_blist(cp->branches, indent+NEXT);
    spaces(indent);
    printf("case_stmt.default_branch at %lx:\n",
            (unsigned long int) cp->default_branch);
}

void
print_macro(m, indent)
    macro_t *m;
    int indent;
{
    int i;

    spaces(indent);
    if(m == NULL) { printf("(null)\n"); return; }
    printf("macro_name %s\n", m->macro_name);
    spaces(indent);
    printf("macro_ada_name %s\n", m->macro_ada_name);
    spaces(indent);
    printf("macro_body <%s>\n", m->macro_body);
    spaces(indent);
    printf("macro_body_len %d\n", m->macro_body_len);
    spaces(indent);
    printf("macro_params %d\n", m->macro_params);
    for(i=0; i<m->macro_params; i++) {
	spaces(indent+4);
	printf("param[%d] %s\n", i, m->macro_param_vec[i]);
    }
    spaces(indent);
    printf("macro_next %lx\n", (unsigned long int) m->macro_next);
    spaces(indent);
    printf("macro_hash %d\n", (int) m->macro_hash);
    spaces(indent);
    printf("macro_hash_link %lx\n", (unsigned long int) m->macro_hash_link);
    spaces(indent);
    printf("macro_func\n");
    print_macro_function(m->macro_func, indent+4);
    spaces(indent);
    printf("comment\n");
    print_comment_block(m->comment, indent+4);
    spaces(indent);
    printf("eol_comment %s\n", m->eol_comment);
    spaces(indent);
    printf("const_value\n");
    print_cpp_eval_result_t(&m->const_value, indent+4);
    spaces(indent);
    printf("macro_declared_in_header %u\n",
	(unsigned)m->macro_declared_in_header);
    spaces(indent);
    printf("macro_gened %u\n", (unsigned)m->macro_gened);
    spaces(indent);
    printf("macro_valid %u\n", (unsigned)m->macro_valid);
    spaces(indent);
    printf("macro_eval_tried %u\n", (unsigned)m->macro_eval_tried);
    spaces(indent);
    printf("macro_evald %u\n", (unsigned)m->macro_evald);
}

void
print_macro_function(macro_function_t *f, int indent)
{
    int i;

    spaces(indent);
    if(f == NULL) { printf("(null)\n"); return; }
    printf("macro function name %s, %d params\n", f->mf_fname, f->mf_nparams);
    spaces(indent);
    printf("params: ");
    for(i=0; i<f->mf_nparams; i++)
	printf("<%s> ", f->mf_params[i]);
    printf("\n");
    spaces(indent);
    if(f->mf_coercion != NULL)
	printf("coercion <%s>\n", f->mf_coercion);
    if(f->mf_is_pointer)
	printf("function pointer\n");
    printf("\n");
}

void
print_cpp_eval_result_t (res, indent)
    cpp_eval_result_t *res;
    int indent;
{
    spaces(indent);
    printf("eval_result_kind %s\n",
	nameof_cpp_eval_result_kind_t(res->eval_result_kind));
    switch(res->eval_result_kind) {
	case eval_failed:
	    break;
	case eval_int:
	    printf("ival %d\n", (int) res->eval_result.ival);
	    break;
	case eval_float:
	    printf("fval %d\n", (int) res->eval_result.fval);
	    break;
	case eval_string:
	    printf("sval %s\n", res->eval_result.sval);
	    break;
	case eval_type:
	    printf("tval\n");
	    print_typeinfo_t(res->eval_result.tval, indent+4);
	    break;
	default:
	    break;
    }
    spaces(indent);
    printf("base %d\n", res->base);
    spaces(indent);
    printf("explicit_type:\n");
    print_typeinfo_t(res->explicit_type, indent+4);
}

void
print_file_pos (pos)
    file_pos_t pos;
{
    printf("%d = file=%s line=%d\n", (int) pos,
           file_name(pos), (int) line_number(pos));
}

void
print_comment_block(bl, indent)
    struct comment_block *bl;
    int indent;
{
    int i;

    spaces(indent);
    if(bl == NULL) { printf("(null)\n"); return; }
    printf("next %lx\n", (unsigned long int) bl->next);
    spaces(indent);
    printf("count %d\n", bl->count);
    for(i=0; i<COMMENT_BLOCKSIZE; i++) {
	spaces(indent+4);
	printf("line %s\n", bl->line[i]);
    }
}
