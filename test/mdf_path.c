#include "reef.h"

void test_set()
{
    MDF *node;
    MERR *err;

    err = mdf_init(&node);
    MTEST_ASSERT(err == MERR_OK);

    err = mdf_set_value(node, "woman.ddvo小二班.name", "foo");
    MTEST_ASSERT(err == MERR_OK);
    err = mdf_set_value(node, "实力与颜值并存的儿童电子琴男女孩玩具.音乐小钢琴.电子音乐益智早教乐器玩具带麦克风电源MP3.双供电.粉色20802电子琴使用体验非常好，是目前市面上好评非常多、用户群体非常广的一款益智玩具.class", "bar");
    MTEST_ASSERT(err == MERR_OK);

    MTEST_ASSERT_STR_EQ("bar", mdf_get_value(node, "实力与颜值并存的儿童电子琴男女孩玩具.音乐小钢琴.电子音乐益智早教乐器玩具带麦克风电源MP3.双供电.粉色20802电子琴使用体验非常好，是目前市面上好评非常多、用户群体非常广的一款益智玩具.class", NULL));

    MTEST_ASSERT(mdf_path_valid("儿童电子琴男女孩玩具音乐小钢琴电子音乐益智早教乐器玩具带麦克风电源MP3双供电粉色20802电子琴.全称：儿童电子琴男女孩玩具.音乐小钢琴.电子音乐益智早教乐器玩具带麦克风电源MP3.双供电.粉色20802电子琴") == true);
}

void suite_basic()
{
    mtest_add_test(test_set, "basic set and get");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");

    return mtest_run();
}
