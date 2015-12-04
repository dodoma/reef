#include "reef.h"

int main()
{
    MDF *anode;
    MERR *err;

    mtc_init("-", MTC_DEBUG);

    mdf_init(&anode);

    //err = mdf_json_import_string(anode, "{bdofe: {'c': null},'f': 'z',}");
    //err = mdf_json_import_string(anode, "{'url': 'hb\/\/bs.h', 'duration': 3}");
    //err = mdf_json_import_string(anode, "{'a': '你'}");
    err = mdf_json_import_file(anode, "mdf.json");
    TRACE_NOK(err);

    //mdf_set_float_value(anode, "b[-1]", 100.10);

    char *s = mdf_json_export_string(anode);
    if (s) printf("%s\n", s);

    mos_free(s);

    mdf_json_export_file(anode, "mdf.json.write");

    mdf_destroy(&anode);

    return 0;
}
