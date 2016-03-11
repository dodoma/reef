#include "reef.h"

void test_basic()
{
    int rows, cols;
    MCV_MAT *mata, *matb;
    unsigned char *posa;
    int *posb;

    rows = 2;
    cols = 4;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);
    matb = mcv_matrix_new(rows, cols, MCV_CPP_3 | MCV_BPC_32S);

    MTEST_ASSERT(MCV_MATRIX_SIZE(mata->rows, mata->cols, mata->type) <
                 MCV_MATRIX_SIZE(matb->rows, matb->cols, matb->type));

    MTEST_ASSERT(MCV_GET_GEO_VALUE(mata, 1, 2, 0) == 0);
    MTEST_ASSERT(MCV_GET_GEO_VALUE(matb, 2, 1, 0) == 0);

    posa = MCV_GET_GEO_MEM(mata, 0, 0, 0);
    posb = MCV_GET_GEO_MEM(matb, 1, 0, 0);
    for (int i = 0; i < cols; i++) {
        MCV_SET_MEM_VALUE(mata->type, posa, i, i+1);

        MTEST_ASSERT(MCV_GET_GEO_VALUE(mata, 0, i, 0) == i+1);

        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3, i+1);
        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3+1, i+1);
        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3+2, i+1);

        MTEST_ASSERT(MCV_GET_GEO_VALUE(matb, 1, i, 0) == i+1);
        MTEST_ASSERT(MCV_GET_GEO_VALUE(matb, 1, i, 1) == i+1);
        MTEST_ASSERT(MCV_GET_GEO_VALUE(matb, 1, i, 2) == i+1);
    }

    int data_number = rows * cols * MCV_CPP_3;
    posb = MCV_GET_GEO_MEM(matb, 0, 0, 0);
    for (int i = 0; i < data_number; i++) {
        if (i < cols * MCV_CPP_3) {
            MTEST_ASSERT(MCV_GET_MEM_VALUE(matb->type, posb, i) == 0);
        } else if (i < (cols+1) * MCV_CPP_3) {
            MTEST_ASSERT(MCV_GET_MEM_VALUE(matb->type, posb, i) == 1);
        } else if (i < (cols+2) * MCV_CPP_3) {
            MTEST_ASSERT(MCV_GET_MEM_VALUE(matb->type, posb, i) == 2);
        } else if (i < (cols+3) * MCV_CPP_3) {
            MTEST_ASSERT(MCV_GET_MEM_VALUE(matb->type, posb, i) == 3);
        } else if (i < (cols+4) * MCV_CPP_3) {
            MTEST_ASSERT(MCV_GET_MEM_VALUE(matb->type, posb, i) == 4);
        }
    }

    mcv_matrix_destroy(&mata);
    mcv_matrix_destroy(&matb);
}

void test_algebra()
{
    int rows, cols;
    MCV_MAT *mata, *matb;
    unsigned char *posa;
    int *posb;

    rows = 3600;
    cols = 2200;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);
    matb = mcv_matrix_new(rows, cols, MCV_CPP_3 | MCV_BPC_32S);

    MTEST_ASSERT(mcv_summary(mata, MCV_FLAG_SIGNED) == 0);
    MTEST_ASSERT(mcv_summary(matb, MCV_FLAG_SIGNED) == 0);

    posa = MCV_GET_GEO_MEM(mata, 0, 0, 0);
    posb = MCV_GET_GEO_MEM(matb, 0, 0, 0);
    for (int i = 0; i < cols; i++) {
        MCV_SET_MEM_VALUE(mata->type, posa, i, 255);

        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3, 255);
        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3+1, 255);
        MCV_SET_MEM_VALUE(matb->type, posb, i*MCV_CPP_3+2, 255);
    }

    //mtimer_start();
    MTEST_ASSERT(mcv_summary(mata, MCV_FLAG_TINY_UNSIGNED) == 255*cols);
    MTEST_ASSERT(mcv_summary(matb, MCV_FLAG_TINY_SIGNED) == 255*cols*MCV_CPP_3);
    //mtimer_stop("summary");

    mcv_matrix_destroy(&mata);
    mcv_matrix_destroy(&matb);
}

void suite_basic()
{
    mtest_add_test(test_basic, "test matrix basic");
}

void suite_expression()
{
    mtest_add_test(test_algebra, "test algebra");
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    mtest_add_suite(suite_basic, "basic");
    mtest_add_suite(suite_expression, "expression");

    return mtest_run();
}
