#ifndef __MDF_MACRO_H__
#define __MDF_MACRO_H__

/*
 * mdf_macro, macros and extend funcs for mdf
 */
__BEGIN_DECLS

#define MDF_TRACE(node)                                     \
    do {                                                    \
        char *_zstra = mdf_json_export_string_pretty(node); \
        mtc_foo("\n%s", _zstra);                            \
        mos_free(_zstra);                                   \
    } while (0)

#define MDF_TRACE_MT(node)                                  \
    do {                                                    \
        char *_zstra = mdf_json_export_string_pretty(node); \
        mtc_mt_foo("\n%s", _zstra);                         \
        mos_free(_zstra);                                   \
    } while (0)

void  mdf_set_typef(MDF *node, MDF_TYPE type, const char *fmt, ...)  ATTRIBUTE_PRINTF(3,4);

char*   mdf_get_valuef(MDF *node, char *dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);;
int     mdf_get_int_valuef(MDF *node, int dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
int64_t mdf_get_int64_valuef(MDF *node, int64_t dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
float   mdf_get_float_valuef(MDF *node, float dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
bool    mdf_get_bool_valuef(MDF *node, bool dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);

MERR* mdf_set_valuevf(MDF *node, char *key, const char *fmt, va_list ap);
MERR* mdf_set_valuef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MERR* mdf_set_int_valuef(MDF *node, int val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
MERR* mdf_set_int64_valuef(MDF *node, int64_t val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
MERR* mdf_set_float_valuef(MDF *node, float val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
MERR* mdf_set_bool_valuef(MDF *node, bool val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);

int     mdf_add_int_valuef(MDF *node, int val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
int64_t mdf_add_int64_valuef(MDF *ndoe, int64_t val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
float   mdf_add_float_valuef(MDF *node, float val, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
char*   mdf_append_string_valuef(MDF *node, char *str, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);
char*   mdf_preppend_string_valuef(MDF *node, char *str, const char *fmt, ...) ATTRIBUTE_PRINTF(3, 4);

MERR* mdf_copyf(MDF *dst, MDF *src, bool overwrite, const char *fmt, ...) ATTRIBUTE_PRINTF(4, 5);
bool  mdf_path_existf(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MDF*  mdf_get_nodef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MDF*  mdf_get_or_create_nodef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MERR* mdf_json_import_filef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
MERR* mdf_json_export_filef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);


__END_DECLS
#endif
