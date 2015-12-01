#include "reef.h"

int main()
{
    MDF *node;
    MERR *err;

    mtc_init("test.log", MTC_DEBUG);

    mdf_init(&node);

    //err = mdf_import_json_string(node, "{b: {'c': 'x'},'f': 'z'}");
    //err = mdf_import_json_string(node, "{'url': 'hb\/\/bs.h', 'duration': 3}");
    //err = mdf_import_json_string(node, "{'a': 1}");
    err = mdf_import_json_file(node, "mdf.json");
    TRACE_NOK(err);

    char *s = mdf_export_json_string(node);
    printf("%s\n", s);

    mos_free(s);

    mdf_export_json_file(node, "mdf.json.write");

    mdf_destroy(&node);

    return 0;
}
