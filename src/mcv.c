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

MERR* mcv_vline_angle(MCV_MAT *mat, int targetv, float *r)
{
    MCV_POINT pstart, pend;
    unsigned char *pos;
    bool reached;
    int section;
    float accuracy;

    MERR_NOT_NULLB(mat, r);

    accuracy = 0.01;
    section = 10;

    /*
     * just accpet gray bitmap currently
     */
    if (mat->type != MCV_DATA_GRAY) return merr_raise(MERR_ASSERT, "expect gray matrix");

    if (mat->rows < section * 2)
        return merr_raise(MERR_ASSERT, "expect %d+ rows matrix", section * 2);

    *r = 0.0;

#define FOR_BLOCK(rowstart, point)                                      \
    reached = false;                                                    \
    pos = mat->data.u8 + (rowstart)*mat->step;                          \
    for (int i = rowstart; i < mat->rows; i++) {                        \
        for (int j = 0; j < mat->cols; j++) {                           \
            if (pos[j] == targetv) {                                    \
                point.x = j;                                            \
                point.y = i;                                            \
                reached = true;                                         \
                break;                                                  \
            }                                                           \
        }                                                               \
        if (reached) break;                                             \
        pos += mat->step;                                               \
    }                                                                   \
    if (!reached) return merr_raise(MERR_ASSERT, "target not found from %d row", rowstart);

    FOR_BLOCK(0, pstart);
    FOR_BLOCK(mat->rows - section, pend);
#undef FOR_BLOCK

    int width, height;
    float tanv;

    width = pend.x - pstart.x;
    height = pend.y - pstart.y;
    tanv = (float)width / height;

    if (fabs(tanv) < accuracy) *r = 0.0;
    else *r = atan(tanv);

    return MERR_OK;
}

MERR* mcv_hline_left_point(MCV_MAT *mat, int targetv, int step, MCV_POINT *point)
{
    unsigned char *pos;
    int counter, end;

    MERR_NOT_NULLB(mat, point);

    if (mat->type != MCV_DATA_GRAY) return merr_raise(MERR_ASSERT, "expect gray matrix");

    if (mat->cols < step)
        return merr_raise(MERR_ASSERT, "matrix width %d < %d", mat->cols, step);

    point->x = point->y = -1;
    end = mat->cols - step;

    pos = mat->data.u8;
    for (int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < end; j++) {
            if (pos[j] == targetv) {
                counter = 0;
                while (counter < step && pos[j+counter] == targetv) counter++;
                if (counter == step) {
                    point->x = j;
                    point->y = i;
                    return MERR_OK;
                } else break;
            }
        }
        pos += mat->step;
    }

    return MERR_OK;
}
