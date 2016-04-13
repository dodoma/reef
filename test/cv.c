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

    MTEST_ASSERT(MCV_GET_GEO_VALUE(mata, 0, 2, 0) == 0);
    MTEST_ASSERT(MCV_GET_GEO_VALUE(matb, 1, 1, 0) == 0);

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

void test_matrix_set()
{
    MCV_MAT *mata;
    int rows, cols;
    MERR *err;

    rows = 80;
    cols = 120;

    unsigned char white = 255;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);

    mcv_matrix_set_gray(mata, mcv_rect(0, 0, mata->cols, mata->rows), 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT matx = mcv_matrix_attach(mata, mcv_rect(j, i, cols - j, rows - i));

            err = mcv_matrix_set_gray(&matx, mcv_rect(0, 0, matx.cols, matx.rows), 255);
            TRACE_NOK(err);

            int num = mcv_pixel_number(&matx, mcv_pixel_gray(&white));
            MTEST_ASSERT(num == (cols - j) * (rows - i));

            num = mcv_pixel_number(mata, mcv_pixel_gray(&white));
            MTEST_ASSERT(num == (cols - j) * (rows - i));

            err = mcv_matrix_set_gray(&matx, mcv_rect(0, 0, matx.cols, matx.rows), 0);
            TRACE_NOK(err);
        }
    }

    mcv_matrix_set_gray(mata, mcv_rect(0, 0, mata->cols, mata->rows), 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT *matx = mcv_matrix_detach(mata, mcv_rect(j, i, cols - j, rows - i));

            err = mcv_matrix_set_gray(matx, mcv_rect(0, 0, matx->cols, matx->rows), 255);
            TRACE_NOK(err);

            int num = mcv_pixel_number(matx, mcv_pixel_gray(&white));
            MTEST_ASSERT(num == (cols - j) * (rows - i));

            num = mcv_pixel_number(mata, mcv_pixel_gray(&white));
            MTEST_ASSERT(num == 0);

            err = mcv_matrix_set_gray(matx, mcv_rect(0, 0, matx->cols, matx->rows), 0);
            TRACE_NOK(err);

            mcv_matrix_destroy(&matx);
        }
    }

    mcv_matrix_destroy(&mata);
}

void test_sub_matrix_gray()
{
    MCV_MAT *mata, *matb;
    int rows, cols;

    rows = 720;
    cols = 47;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);
    matb = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);

    unsigned char white = 255;
    MCV_POINT point;

    for (int i = 0; i < rows / 2; i++) {
        for (int j = 0; j < cols / 2; j++) {
            mcv_matrix_set_gray(mata, mcv_rect(j, i, j, i), 255);

            mcv_pixel_position(mata, mcv_pixel_gray(&white), &point);
            if (i == 0 || j == 0) {
                /* rect 非法 */
                MTEST_ASSERT(point.x == -1);
                MTEST_ASSERT(point.y == -1);
            } else {
                MTEST_ASSERT(point.x == j);
                MTEST_ASSERT(point.y == i);
            }

            mcv_matrix_subwin_position(mata, mcv_rect(j, i, j, i),
                                       mcv_pixel_gray(&white), &point);
            if (i == 0 || j == 0) {
                /* rect 非法 */
                MTEST_ASSERT(point.x == -1);
                MTEST_ASSERT(point.y == -1);
            } else {
                MTEST_ASSERT(point.x == j);
                MTEST_ASSERT(point.y == i);
            }

            mcv_matrix_set_gray(mata, mcv_rect(j, i, j, i), 0);
        }
    }

    mcv_matrix_destroy(&mata);
    mcv_matrix_destroy(&matb);
}

void test_sub_matrix_color()
{
    MCV_MAT *mata;
    int rows, cols;

    rows = 45;
    cols = 600;

    int type = MCV_CPP_3 | MCV_BPC_64F;

    mata = mcv_matrix_new(rows, cols, type);

    double white[3] = {255.0938493, 256.3939494, 257.9858939};
    double white2[3] = {255.093844, 256.3939493, 257.985892};
    double black[3] = {0, 0, 0};
    MCV_POINT point;

    for (int i = 1; i < rows / 2; i++) {
        for (int j = 1; j < cols / 2; j++) {
            mcv_matrix_set_pixel(mata, mcv_rect(j, i, j, i), mcv_pixel(type, &white));

            mcv_pixel_position(mata, mcv_pixel(type, &white2), &point);
            if (i == 0 || j == 0) {
                /* rect 非法 */
                MTEST_ASSERT(point.x == -1);
                MTEST_ASSERT(point.y == -1);
            } else {
                MTEST_ASSERT(point.x == j);
                MTEST_ASSERT(point.y == i);
            }

            mcv_matrix_subwin_position(mata, mcv_rect(j, i, j, i),
                                       mcv_pixel(type, &white2), &point);
            if (i == 0 || j == 0) {
                /* rect 非法 */
                MTEST_ASSERT(point.x == -1);
                MTEST_ASSERT(point.y == -1);
            } else {
                MTEST_ASSERT(point.x == j);
                MTEST_ASSERT(point.y == i);
            }

            mcv_matrix_set_pixel(mata, mcv_rect(j, i, j, i), mcv_pixel(type, &black));
        }
    }

    mcv_matrix_destroy(&mata);
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
    mtest_add_test(test_matrix_set, "test matrix set");
    mtest_add_test(test_sub_matrix_gray, "test gray matrix suboption");
    mtest_add_test(test_sub_matrix_color, "test color float matrix suboption");
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
