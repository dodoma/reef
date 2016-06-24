#include "reef.h"
#include "_mcv.h"

MERR* mcv_rect_set_gray(MCV_MAT *mat, MCV_RECT rect, unsigned char v)
{
    MERR_NOT_NULLA(mat);

    if (mat->type != MCV_DATA_GRAY) return merr_raise(MERR_ASSERT, "expect gray matrix");

    if (rect.x + rect.w > mat->cols || rect.y + rect.h > mat->rows)
        return merr_raise(MERR_ASSERT, "rect size exceed (%d, %d) %d x %d > %d x %d",
                          rect.x, rect.y, rect.w, rect.h, mat->cols, mat->rows);

    unsigned char *pos = mat->data.u8;
    pos += mat->step * rect.y + rect.x;
    for (int i = 0; i < rect.h; i++) {
        memset(pos, v, rect.w);
        pos += mat->step;
    }

    return MERR_OK;
}

MERR* mcv_matrix_set_gray(MCV_MAT *mat, unsigned char v)
{
    MERR_NOT_NULLA(mat);

    if (mat->type != MCV_DATA_GRAY) return merr_raise(MERR_ASSERT, "expect gray matrix");

    memset(mat->data.u8, v, MCV_MAT_DATA_SIZE(mat));

    return MERR_OK;
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

MERR* mcv_hline_angle(MCV_MAT *mat, int targetv, float *r)
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

    if (mat->cols < section * 2)
        return merr_raise(MERR_ASSERT, "expect %d+ cols matrix", section * 2);

    *r = 0.0;

#define FOR_BLOCK(colstart, point)                                      \
    reached = false;                                                    \
    pos = mat->data.u8 + colstart;                                      \
    for (int i = colstart; i < mat->cols; i++) {                        \
        for (int j = 0; j < mat->rows; j++) {                           \
            if (pos[j*mat->step] == targetv) {                          \
                point.x = i;                                            \
                point.y = j;                                            \
                reached = true;                                         \
                break;                                                  \
            }                                                           \
        }                                                               \
        if (reached) break;                                             \
        pos++;                                                          \
    }                                                                   \
    if (!reached) return merr_raise(MERR_ASSERT, "target not found from %d col", colstart);

    FOR_BLOCK(0, pstart);
    FOR_BLOCK(mat->cols - section, pend);
#undef FOR_BLOCK

    int width, height;
    float tanv;

    width = pend.x - pstart.x;
    height = -(pend.y - pstart.y);
    tanv = (float)height / width;

    if (fabs(tanv) < accuracy) *r = 0.0;
    else *r = atan(tanv);

    return MERR_OK;
}
