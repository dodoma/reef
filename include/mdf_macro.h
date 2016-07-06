#ifndef __MDF_MACRO_H__
#define __MDF_MACRO_H__

/*
 * mdf_macro, macros and extend funcs for mdf
 */
__BEGIN_DECLS

char* mdf_get_valuef(MDF *node, char *dftvalue, const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);;
MERR* mdf_set_valuef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MERR* mdf_set_typef(MDF *node, MDF_TYPE type, const char *fmt, ...)  ATTRIBUTE_PRINTF(3,4);
MERR* mdf_copyf(MDF *dst, MDF *src, const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);
bool  mdf_path_existf(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MDF*  mdf_get_nodef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MDF*  mdf_get_or_create_nodef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
MERR* mdf_json_import_filef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);
MERR* mdf_json_export_filef(MDF *node, const char *fmt, ...) ATTRIBUTE_PRINTF(2, 3);


__END_DECLS
#endif
