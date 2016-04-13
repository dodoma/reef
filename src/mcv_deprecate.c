#include "reef.h"
#include "_mcv.h"

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

MERR* mcv_matrix_set_matrix(MCV_MAT *mata, MCV_RECT *rect, MCV_MAT *matb)
{
    MERR_NOT_NULLB(mata, matb);

    if (mata->type != matb->type) return merr_raise(MERR_ASSERT, "type mismatch");

    MCV_RECT lrect = mcv_rect(0, 0, mata->cols, mata->rows);
    if (rect == NULL) rect = &lrect;

    matb->rows = rect->h;
    matb->cols = rect->w;
    matb->step = (matb->cols * MCV_GET_CPP(matb->type)
        * MCV_GET_CHANNEL_SIZE(matb->type) + 3) & -4;

    int xoffset = rect->x * MCV_GET_CPP(mata->type) * MCV_GET_CHANNEL_SIZE(matb->type);
    unsigned char *posa = mata->data.u8;
    unsigned char *posb = matb->data.u8;

    posa += mata->step * rect->y + xoffset;

    mtimer_start();
    for (int i = 0; i < rect->h; i++) {
        //memcpy(posb, posa, matb->step);
        posa += mata->step;
        posb += matb->step;
    }
    mtimer_stop("yyy %d", rect->h);

    return MERR_OK;
}

MERR* xmcv_matrix_set_pixel(MCV_MAT *mat, MCV_PIXEL *pixel, MCV_RECT *rect)
{
    MERR_NOT_NULLB(mat, pixel);

    if (mat->type != pixel->type) return merr_raise(MERR_ASSERT, "type mismatch");

    MCV_RECT lrect = mcv_rect(0, 0, mat->cols, mat->rows);
    if (rect == NULL) rect = &lrect;

    int cpp = MCV_GET_CPP(mat->type);
    unsigned char *posa = mat->data.u8;
    unsigned char *posb = pixel->data.u8;

    posa += mat->step * rect->y;

#define FOR_BLOCK(setter, getter)                                       \
    for (int i = 0; i < rect->h; i++) {                                 \
        for (int j = 0; j < rect->w; j++) {                             \
            for (int k = 0; k < cpp; k++) {                             \
                setter(posa, (rect->x + j) * cpp + k, getter(posb, k)); \
            }                                                           \
        }                                                               \
        posa += mat->step;                                              \
    }

    _MCV_MAT_SETTER(mat->type, _MCV_MAT_GETTER, pixel->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}
