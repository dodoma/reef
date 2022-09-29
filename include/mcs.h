#ifndef __MCS_H__
#define __MCS_H__

/*
 * mcs, template system stolen from [clearsiver](http://www.clearsilver.net/docs/man_templates.hdf)
 *
 * ***PARTLY DONE***
 * currently support:
 *     parse time: include
 *     rend time: var, name, each, if, elif, else
 */
__BEGIN_DECLS

/*
 * path: include search path, NULL for ./
 */
MERR* mcs_parse_string(const char *str, const char *path, MDF *node, MCS **tpl);
MERR* mcs_parse_file(const char *fname, const char *path, MDF *node, MCS **tpl);
MERR* mcs_rend(MCS *tpl, MDF *node, const char *fname);
void  mcs_destroy(MCS **tpl);
void  mcs_dump(MCS *tpl);

double mcs_eval_numberic(char *expr);

__END_DECLS
#endif
