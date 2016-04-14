#include "reef.h"

void load_matrix_sub()
{
    MCV_MAT *mata;

    int rows, cols;

    rows = 640;
    cols = 480;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT matx = mcv_matrix_attach(mata, mcv_rect(j, i, cols - j, rows - i));
        }
    }
    mtimer_stop("attach");

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT *matx = mcv_matrix_detach(mata, mcv_rect(j, i, cols - j, rows - i));
            mcv_matrix_destroy(&matx);
        }
    }
    mtimer_stop("detach part");

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT *matx = mcv_matrix_detach(mata, mcv_rect(0, 0, mata->cols, mata->rows));
            mcv_matrix_destroy(&matx);
        }
    }
    mtimer_stop("detach whole");

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT *matx = mcv_matrix_clone(mata);
            mcv_matrix_destroy(&matx);
        }
    }
    mtimer_stop("clone");

    mcv_matrix_destroy(&mata);
}

void load_matrix_set()
{
    MCV_MAT *mata;

    int rows, cols;

    rows = 640;
    cols = 480;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT *matx = mcv_matrix_clone(mata);
            mcv_matrix_set_gray(matx, mcv_rect(j, i, cols - j, rows - i), 255);
            mcv_matrix_destroy(&matx);
        }
    }
    mtimer_stop("set");

    mcv_matrix_destroy(&mata);
}

void load_matrix_sum()
{
    MCV_MAT *mata;

    int rows, cols;

    rows = 640;
    cols = 480;

    mata = mcv_matrix_new(rows, cols, MCV_DATA_GRAY);

    mtimer_start();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            MCV_MAT matx = mcv_matrix_attach(mata, mcv_rect(j, i, cols - j, rows - i));
            mcv_summary(&matx, MCV_FLAG_NONE);
        }
    }
    mtimer_stop("summary");

    mcv_matrix_destroy(&mata);
}

int main()
{
    mtc_init("-", MTC_DEBUG);

    load_matrix_sub();
    load_matrix_set();
    load_matrix_sum();
}
