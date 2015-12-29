#ifndef __MDF_MACRO_H__
#define __MDF_MACRO_H__

/*
 * mdf_macro, macros and extend funcs for mdf
 */
__BEGIN_DECLS

MERR* mdf_set_valuef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MERR* mdf_copyf(MDF *dst, MDF *src, const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);
bool  mdf_path_existf(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);

#endif
