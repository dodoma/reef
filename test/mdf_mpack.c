#include "reef.h"

int main()
{
    MDF *node;
    MERR *err;

    mtc_init("-", MTC_DEBUG);

    mdf_init(&node);

    //err = mdf_json_import_string(node, "{'h': 100.32, 'i': 'newman'}");
    //err = mdf_json_import_string(node, "{'a': ['a', 'b']}");
    //err = mdf_json_import_string(node, "{bdofe: {'c': null},'f': 'z'}");
    //err = mdf_json_import_string(node, "{'e': 200, 'e_msg': '', 'data': {'t': 2, 'z': 3, 'y': 4, 'f': {'a': 1, 'b': 2, 'c': 3, 'd': 4.2}}}");
    err = mdf_json_import_file(node, "mdf.json");
    TRACE_NOK(err);

    unsigned char buf[1024], output[1024];
    size_t len;

    len = mdf_mpack_serialize(node, buf, 1024);

    mstr_bin2hexstr(buf, len, output);

    printf("serialized %d byte data \n %s\n\n", len, output, sizeof(float));

    mdf_destroy(&node);
    mdf_init(&node);

    len = mdf_mpack_deserialize(node, buf, len);
    char *s = mdf_json_export_string(node);

    mdf_json_export_file(node, "mdf.json.mpack");

    printf("deserialzed %d byte data \n %s\n", len, s);

    return 0;
}
