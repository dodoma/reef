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

MERR* mcv_vertical_align(MCV_MAT *mat, int maxstep,
                         MCV_PIXEL val_pixel, MCV_PIXEL pad_pixel)
{
    MERR_NOT_NULLA(mat);

    if (mat->type != val_pixel.type ||
        mat->type != pad_pixel.type) return merr_raise(MERR_ASSERT, "type mismatch");

    int cpp = MCV_GET_CPP(mat->type);
    int pixellen = MCV_GET_CPP(pad_pixel.type) * MCV_GET_CHANNEL_SIZE(pad_pixel.type);
    unsigned char *posa = mat->data.u8;
    unsigned char *posb = val_pixel.data.u8;
    float epsilon = (MCV_GET_BPC(mat->type) == MCV_BPC_8U ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_32S ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_64S) ? 1 : 1e-4;

    int k, delta, memlen, x = -1;
    unsigned char *pos_pad;

#define FOR_BLOCK(_, getter)                                            \
    for (int i = 0; i < mat->rows; i++) {                               \
        for (int j = 0; j < mat->cols; j++) {                           \
            k = 0;                                                      \
            while (k < cpp && fabs((double)(getter(posa, j * cpp + k) - \
                                            getter(posb, k))) < epsilon) { \
                k++;                                                    \
            }                                                           \
            if (k == cpp) {                                             \
                if (x == -1) x = j;                                     \
                delta = x - j;                                          \
                if (delta != 0 && abs(delta) <= maxstep) {              \
                    /* 移动 */                                          \
                    memlen = x > j ? mat->cols - x : mat->cols - j;     \
                    memlen *= pixellen;                                 \
                    memmove(posa + (x * pixellen), posa + (j * pixellen), memlen); \
                    /* 填空 */                                          \
                    int p = x > j ? j : mat->cols - delta;              \
                    pos_pad = posa + p * pixellen;                      \
                    for (int m = 0; m < delta; m++) {                   \
                        memcpy(pos_pad, pad_pixel.data.u8, pixellen);   \
                        pos_pad += pixellen;                            \
                    }                                                   \
                }                                                       \
                break;                                                  \
            }                                                           \
        }                                                               \
        posa += mat->step;                                              \
    }

    _MCV_MAT_GETTER(mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}
