#include "reef.h"

void onbody(unsigned char *buf, size_t len, const char *type, void *p)
{
    FILE *fp = (FILE*)p;
    printf("================================\n");
    char zstra[(len)*2+1];
    mstr_bin2str((uint8_t*)(buf), (len), zstra);
    mtc_mt_dbg("%s %zu %s", "body size", (size_t)(len), zstra);
    //mtc_mt_dbg("%zu", len);

    //printf("================================\n");
    fwrite(buf, 1, len, fp);
}

void test_url_unescape()
{

    char s[100] = "%E4%BA%AE%E5%93%A5%E5%93%A5";

    MTEST_ASSERT_STR_EQ("亮哥哥", mhttp_url_unescape(s, strlen(s), '%'));

    char *es = mhttp_url_escape("http://weixin.qq.com/q/02zSfCgTe_e4i1DgfFhscJ我爱你");
    MTEST_ASSERT_STR_EQ("http%3A%2F%2Fweixin.qq.com%2Fq%2F02zSfCgTe_e4i1DgfFhscJ%E6%88%91%E7%88%B1%E4%BD%A0", es);
    mos_free(es);
}

void test_request()
{
    //char *url = "http://119.29.69.44/cgi/viki?aaaa=bbbb&aa=我们是";
    //char *url = "http://www.woshipm.com/operate/894550.html";
    char *url = "https://yun.tim.qq.com/";
    //char *url = "http://open.youzan.com/api/oauthentry/youzan.itemcategories.tags/3.0.0/get?access_token=e08d686ae35d38768c02f0ed2c623cba";
    //char *url = "http://open.youzan.com/oauth/token";
    //char *url = "http://open.youzan.com/api/oauthentry/youzan.materials.storage.platform.img/3.0.0/upload";
    url = "https://res.mbox.net.cn/audio/eMix.mp3";
    char *payload = "client_id=41c986e32e3342f0af&client_secret=81d9b72d5b01bb1543393671db061933&grant_type=silent&kdt_id=40134859";
    MDF *node, *dnode;
    MERR *err;
    mdf_init(&node);
    mdf_init(&dnode);

    mdf_set_value(dnode, "access_token", "e08d686ae35d38768c02f0ed2c623cba");
    mdf_set_value(dnode, "image", "@EveningReflections.jpg");
    //mdf_set_value(dnode, "image", "@10.jpeg");

    FILE *fp = fopen("result.html", "w");

    err = mhttp_get(url, NULL, node, onbody, fp);
    //err = mhttp_post(url, NULL, payload, node, onbody);
    //err = mhttp_post_with_file(url, dnode, node, onbody, fp);
    //err = mhttp_post_with_filef(url, node, onbody, fp, 2,
//                                "access_token", "e08d686ae35d38768c02f0ed2c623cba",
  //                              "image[]", "@10.jpeg");
    TRACE_NOK_MT(err);

    fclose(fp);

    MDF *xnode;
    mdf_init(&xnode);
    err = mdf_json_import_file(xnode, "result.html");
    TRACE_NOK_MT(err);
    mdf_json_export_file(xnode, "-");
    mdf_destroy(&xnode);

    MTEST_ASSERT(err == MERR_OK);

    mdf_json_export_file(node, "-");

    mdf_destroy(&node);
}

void suite_basic()
{
    mtest_add_test(test_url_unescape, "test url unescape");
    mtest_add_test(test_request, "test http request");
}

int main()
{
    mtc_mt_init("-", "main", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
