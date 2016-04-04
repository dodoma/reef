#include "reef.h"
#include "_mcv.h"

MCV_MAT* mcv_matrix_new(int rows, int cols, int type)
{
    MCV_MAT *mat;

    mat = mos_calloc(1, MCV_MATRIX_SIZE(rows, cols, type));
    mat->type = type;
    mat->rows = rows;
    mat->cols = cols;
    /* padding step, so it divisible by 4 */
    mat->step = (cols * MCV_GET_CPP(type) * MCV_GET_CHANNEL_SIZE(type) + 3) & -4;
    mat->data.u8 = (unsigned char*)(mat + 1);

    return mat;
}

void mcv_matrix_destroy(MCV_MAT **mat)
{
    if (!mat) return;

    mos_free(*mat);

    *mat = NULL;
}

MCV_MAT mcv_matrix(int rows, int cols, int type, void *data)
{
    MCV_MAT mat;

    mat.type = type;
    mat.rows = rows;
    mat.cols = cols;
    mat.step = (cols * MCV_GET_CPP(type) * MCV_GET_CHANNEL_SIZE(type) + 3) & -4;
    mat.data.u8 = (unsigned char*)data;

    return mat;
}

double mcv_summary(MCV_MAT *mat, int flag)
{
    int cpr; /* channel per row */
    unsigned char *pos;

    /*
     * int almost 3 times faster than double
     */
    double sum_d;
    unsigned int sum_u;
    int sum_i;

    if (!mat) return 0;

    sum_d = sum_u = sum_i = 0;
    pos = mat->data.u8;
    cpr = mat->cols * MCV_GET_CPP(mat->type);

#define FOR_BLOCK(_, getter)                            \
    switch (flag) {                                     \
    case MCV_FLAG_TINY_UNSIGNED:                        \
        for (int i = 0; i < mat->rows; i++) {           \
            for (int j = 0; j < cpr; j++) {             \
                sum_u += abs((int)getter(pos, j));      \
            }                                           \
            pos += mat->step;                           \
        }                                               \
        sum_d = sum_u;                                  \
        break;                                          \
    case MCV_FLAG_TINY_SIGNED:                          \
        for (int i = 0; i < mat->rows; i++) {           \
            for (int j = 0; j < cpr; j++) {             \
                sum_i += getter(pos, j);                \
            }                                           \
            pos += mat->step;                           \
        }                                               \
        sum_d = sum_i;                                  \
        break;                                          \
    case MCV_FLAG_UNSIGNED:                             \
        for (int i = 0; i < mat->rows; i++) {           \
            for (int j = 0; j < cpr; j++) {             \
                sum_d += fabs((double)getter(pos, j));  \
            }                                           \
            pos += mat->step;                           \
        }                                               \
        break;                                          \
    case MCV_FLAG_SIGNED:                               \
    default:                                            \
        for (int i = 0; i < mat->rows; i++) {           \
            for (int j = 0; j < cpr; j++) {             \
                sum_d += getter(pos, j);                \
            }                                           \
            pos += mat->step;                           \
        }                                               \
    }

    _MCV_MAT_GETTER(mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return sum_d;
}
