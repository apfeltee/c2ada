#ifndef _GEN_MACROS_H
#define _GEN_MACROS_H

extern void gen_macro_names(void);
extern void import_macro_constants(void);
extern void gen_macro_constants(macro_t *m, int import);
extern void gen_macro_types(macro_t *m, int import);
extern void gen_macro_funcs(macro_t *m, int import);
extern void gen_macro_vars(macro_t *m, int import, int colonpos);
extern void finish_macros(macro_t *m);
extern void rethread_macros(void);
extern void gen_macro_warnings(void);

#endif /* _GEN_MACROS_H */
