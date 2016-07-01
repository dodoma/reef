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

MCV_MAT* mcv_matrix_new_nalloc(int rows, int cols, int type, void *data)
{
    MCV_MAT *mat;

    mat = mos_calloc(1, sizeof(MCV_MAT));
    mat->type = type;
    mat->rows = rows;
    mat->cols = cols;
    mat->step = (cols * MCV_GET_CPP(type) * MCV_GET_CHANNEL_SIZE(type) + 3) & -4;
    mat->data.u8 = data;

    return mat;
}

MCV_MAT* mcv_matrix_new_memcpy(int rows, int cols, int type, void *data)
{
    MCV_MAT *mat;

    mat = mos_calloc(1, MCV_MATRIX_SIZE(rows, cols, type));
    mat->type = type;
    mat->rows = rows;
    mat->cols = cols;
    mat->step = (cols * MCV_GET_CPP(type) * MCV_GET_CHANNEL_SIZE(type) + 3) & -4;
    mat->data.u8 = (unsigned char*)(mat + 1);
    memcpy(mat->data.u8, data, MCV_MAT_DATA_SIZE(mat));

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

MCV_MAT mcv_matrix_attach(MCV_MAT *mat, MCV_RECT rect)
{
    MCV_MAT rmat;

    memset(&rmat, 0, sizeof(rmat));

    if (!mat) return rmat;

    if (rect.x + rect.w > mat->cols || rect.y + rect.h > mat->rows) return rmat;

    int offset = rect.x * MCV_GET_CPP(mat->type) * MCV_GET_CHANNEL_SIZE(mat->type);
    offset += mat->step * rect.y;

    rmat.type = mat->type;
    rmat.rows = rect.h;
    rmat.cols = rect.w;
    rmat.step = mat->step;
    rmat.data.u8 = mat->data.u8 + offset;

    return rmat;
}

MCV_MAT* mcv_matrix_detach(MCV_MAT *mat, MCV_RECT rect)
{
    MCV_MAT *rmat;

    if (!mat) return NULL;

    if (rect.x + rect.w > mat->cols || rect.y + rect.h > mat->rows) return NULL;

    rmat = mcv_matrix_new(rect.h, rect.w, mat->type);

    int pixellen = MCV_GET_CPP(mat->type) * MCV_GET_CHANNEL_SIZE(mat->type);
    int rowlen = rect.w * pixellen;
    unsigned char *posa = rmat->data.u8;
    unsigned char *posb = mat->data.u8;
    posb += mat->step * rect.y + rect.x * pixellen;

    for (int i = 0; i < rect.h; i++) {
        /*
         * must use rowlen instead of rmat->step, because rmat->step >= rowlen
         */
        memcpy(posa, posb, rowlen);
        posa += rmat->step;
        posb += mat->step;
    }

    return rmat;
}

MCV_MAT* mcv_matrix_scale(MCV_MAT *mat, int num, int denom)
{
    MCV_MAT *rmat;

    if (!mat) return NULL;

    int remain = num > denom ? num % denom : denom % num;
    int times = num > denom ? num / denom : denom / num;
    if (remain != 0 || times % 2 != 0) {
        mtc_warn("%d %d can't scale", num, denom);
        return NULL;
    }

    float ratio = (float)num / denom;
    int rows = mat->rows * ratio;
    int cols = mat->cols * ratio;

    rmat = mcv_matrix_new(rows, cols, mat->type);

    int cpp = MCV_GET_CPP(mat->type);
    unsigned char *posa = mat->data.u8;
    unsigned char *posb = rmat->data.u8;

#define FOR_BLOCK(setter, getter)                                       \
    for (int i = 0; i < rmat->rows; i++) {                              \
        for (int j = 0; j < rmat->cols; j++) {                          \
            for (int k = 0; k < cpp; k++) {                             \
                setter(posb, j * cpp + k, getter(posa, (int)(j / ratio) * cpp + k)); \
            }                                                           \
        }                                                               \
        posa = mat->data.u8 + mat->step * (int)(i / ratio);             \
        posb += rmat->step;                                             \
    }

    _MCV_MAT_SETTER(rmat->type, _MCV_MAT_GETTER, mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return rmat;
}

MCV_MAT* mcv_matrix_clone(MCV_MAT *mat)
{
    MCV_MAT *rmat;

    if (!mat) return NULL;

    rmat = mcv_matrix_new(mat->rows, mat->cols, mat->type);

    memcpy(rmat->data.u8, mat->data.u8, MCV_MAT_DATA_SIZE(mat));

    return rmat;
}

MCV_MAT* mcv_read(const char *fname, int type)
{
    MCV_FILE_TYPE filetype;
    MCV_MAT *rmat;

    if (!fname) return NULL;

    rmat = NULL;

    FILE *fd = fopen(fname, "rb");
    if (fd) {
        unsigned char sig[8];
        fread(sig, 1, 8, fd);
        filetype = MCV_FILE_RAW;
		if (memcmp(sig, "\x89\x50\x4e\x47\xd\xa\x1a\xa", 8) == 0)
			filetype = MCV_FILE_PNG;
		else if (memcmp(sig, "\xff\xd8\xff", 3) == 0)
			filetype = MCV_FILE_JPEG;
		else if (memcmp(sig, "BM", 2) == 0)
			filetype = MCV_FILE_BMP;
		else if (memcmp(sig, "CCVBINDM", 8) == 0)
			filetype = MCV_FILE_RAW;
		fseek(fd, 0, SEEK_SET);

        switch (filetype) {
        case MCV_FILE_JPEG:
            rmat = mcv_read_jpeg(fd, type);
            fclose(fd);
            break;
        case MCV_FILE_RAW:
        {
            int type, rows, cols;
            fread(&type, 1, 4, fd);
            fread(&rows, 1, 4, fd);
            fread(&cols, 1, 4, fd);

            rmat = mcv_matrix_new(rows, cols, type);
            fread(rmat->data.u8, 1, rmat->step * rmat->rows, fd);
            fclose(fd);
            break;
        }
        default:
            fclose(fd);
            break;
        }
    } else {
        mtc_warn("can't open %s for read", fname);
    }

    return rmat;
}

void mcv_write(MCV_MAT *mat, const char *fname, MCV_FILE_TYPE filetype)
{
    if (!mat || !fname) return;

    FILE *fd = fopen(fname, "wb");
    if (fd) {
        switch (filetype) {
        case MCV_FILE_JPEG:
            mcv_write_jpeg(mat, fd);
            fclose(fd);
            break;
        case MCV_FILE_RAW:
            fwrite("MCVBINDM", 1, 8, fd);
            fwrite(&(mat->type), 1, 4, fd);
            fwrite(&(mat->rows), 1, 4, fd);
            fwrite(&(mat->cols), 1, 4, fd);
            fwrite(mat->data.u8, 1, mat->step * mat->rows, fd);
            fflush(fd);
            fclose(fd);
            break;
        default:
            fclose(fd);
            break;
        }
    } else {
        mtc_warn("can't open %s for write", fname);
    }
}

bool mcv_matrix_eq(MCV_MAT *mata, MCV_MAT *matb)
{
    if (!mata || !matb) return false;

    if (mata->type != matb->type ||
        mata->rows != matb->rows ||
        mata->cols != matb->cols) return false;

    int cpp = MCV_GET_CPP(mata->type);
    unsigned char *posa = mata->data.u8;
    unsigned char *posb = matb->data.u8;
    float epsilon = (MCV_GET_BPC(mata->type) == MCV_BPC_8U ||
                     MCV_GET_BPC(mata->type) == MCV_BPC_32S ||
                     MCV_GET_BPC(mata->type) == MCV_BPC_64S) ? 1 : 1e-4;

#define FOR_BLOCK(_, getter)                                            \
    for (int i = 0; i < mata->rows; i++) {                              \
        for (int j = 0; j < mata->cols * cpp; j++) {                    \
            if (fabs((double)(getter(posa, j) - getter(posb, j))) > epsilon) { \
                return false;                                           \
            }                                                           \
        }                                                               \
        posa += mata->step;                                             \
        posb += matb->step;                                             \
    }

    _MCV_MAT_GETTER(mata->type, FOR_BLOCK);
#undef FOR_BLOCK

    return true;
}

MERR* mcv_rect_set_pixel(MCV_MAT *mat, MCV_RECT rect, MCV_PIXEL pixel)
{
    MERR_NOT_NULLA(mat);

    if (mat->type != pixel.type) return merr_raise(MERR_ASSERT, "type mismatch");

    if (rect.x + rect.w > mat->cols || rect.y + rect.h > mat->rows)
        return merr_raise(MERR_ASSERT, "rect size exceed (%d, %d) %d x %d > %d x %d",
                          rect.x, rect.y, rect.w, rect.h, mat->cols, mat->rows);

    int pixellen = MCV_GET_CPP(pixel.type) * MCV_GET_CHANNEL_SIZE(pixel.type);
    int rowlen = rect.w * pixellen;
    unsigned char *buf = mos_calloc(1, rowlen);
    unsigned char *pos = buf;
    for (int i = 0; i < rect.w; i++) {
        memcpy(pos, pixel.data.u8, pixellen);
        pos += pixellen;
    }

    unsigned char *posa = mat->data.u8;
    posa += mat->step * rect.y + rect.x * pixellen;
    for (int i = 0; i < rect.h; i++) {
        memcpy(posa, buf, rowlen);
        posa += mat->step;
    }

    mos_free(buf);

    return MERR_OK;
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

unsigned int mcv_pixel_number(MCV_MAT *mat, MCV_PIXEL pixel)
{
    if (!mat) return 0;

    unsigned int num = 0;
    int cpp = MCV_GET_CPP(mat->type);
    unsigned char *posa = mat->data.u8;
    unsigned char *posb = pixel.data.u8;
    float epsilon = (MCV_GET_BPC(mat->type) == MCV_BPC_8U ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_32S ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_64S) ? 1 : 1e-4;

#define FOR_BLOCK(_, getter)                                            \
    for (int i = 0; i < mat->rows; i++) {                               \
        for (int j = 0; j < mat->cols; j++) {                           \
            int k = 0;                                                  \
            while (k < cpp && fabs((double)(getter(posa, j * cpp + k) - \
                                            getter(posb, k))) < epsilon) { \
                k++;                                                    \
            }                                                           \
            if (k == cpp) num++;                                        \
        }                                                               \
        posa += mat->step;                                              \
    }

    _MCV_MAT_GETTER(mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return num;
}

unsigned int mcv_nonzero_pixel_number(MCV_MAT *mat)
{
    if (!mat) return 0;

    unsigned int num = 0;
    int cpp = MCV_GET_CPP(mat->type);
    unsigned char *pos = mat->data.u8;

#define FOR_BLOCK(_, getter)                                    \
    for (int i = 0; i < mat->rows; i++) {                       \
        for (int j = 0; j < mat->cols; j++) {                   \
            int k = 0;                                          \
            while (k < cpp && getter(pos, j * cpp + k) == 0) {  \
                k++;                                            \
            }                                                   \
            if (k != cpp) num++;                                \
        }                                                       \
        pos += mat->step;                                       \
    }

    _MCV_MAT_GETTER(mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return num;
}

MERR* mcv_subtract(MCV_MAT *mata, MCV_MAT *matb, MCV_MAT *matc)
{
    MERR_NOT_NULLC(mata, matb, matc);

    if (!mcv_matrix_match(mata, matb)) return merr_raise(MERR_ASSERT, "matrix mismatch");

    int cpp = MCV_GET_CPP(mata->type);
    unsigned char *posa = mata->data.u8;
    unsigned char *posb = matb->data.u8;
    unsigned char *posc = matc->data.u8;

#define FOR_BLOCK(setter, getter)                               \
    for (int i = 0; i < mata->rows; i++) {                      \
        for (int j = 0; j < mata->cols * cpp; j++) {            \
            setter(posc, j, getter(posa, j) - getter(posb, j)); \
        }                                                       \
        posa += mata->step;                                     \
        posb += matb->step;                                     \
        posc += matc->step;                                     \
    }

    _MCV_MAT_SETTER(matc->type, _MCV_MAT_GETTER, mata->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}

MERR* mcv_rotate_quart(MCV_MAT *mata, MCV_MAT *matb)
{
    MERR_NOT_NULLB(mata, matb);

    if (mata->type != matb->type || mata->rows != matb->cols || mata->cols != matb->rows)
        return merr_raise(MERR_ASSERT, "matrix mismatch");

    int cpp = MCV_GET_CPP(mata->type);
    unsigned char *posa = mata->data.u8 + (mata->rows - 1) * mata->step;
    unsigned char *posb = matb->data.u8;

#define FOR_BLOCK(setter, getter)                                       \
    for (int i = 0; i < matb->rows; i++) {                              \
        for (int j = 0; j < matb->cols; j++) {                          \
            for (int k = 0; k < cpp; k++) {                             \
                setter(posb, j * cpp + k, getter(posa, -j * mata->step + k)); \
            }                                                           \
        }                                                               \
        posa += cpp;                                                    \
        posb += matb->step;                                             \
    }

    _MCV_MAT_SETTER(matb->type, _MCV_MAT_GETTER, mata->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}

MERR* mcv_pixel_position(MCV_MAT *mat, MCV_PIXEL pixel, MCV_POINT *point)
{
    MERR_NOT_NULLA(mat);

    if (mat->type != pixel.type) return merr_raise(MERR_ASSERT, "type mismatch");

    int cpp = MCV_GET_CPP(mat->type);
    unsigned char *posa = mat->data.u8;
    unsigned char *posb = pixel.data.u8;
    float epsilon = (MCV_GET_BPC(mat->type) == MCV_BPC_8U ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_32S ||
                     MCV_GET_BPC(mat->type) == MCV_BPC_64S) ? 1 : 1e-4;

    point->x = point->y = -1;

    int k;

#define FOR_BLOCK(_, getter)                                            \
    for (int i = 0; i < mat->rows; i++) {                               \
        for (int j = 0; j < mat->cols; j++) {                           \
            k = 0;                                                      \
            while (k < cpp && fabs((double)(getter(posa, j * cpp + k) - \
                                            getter(posb, k))) < epsilon) { \
                k++;                                                    \
            }                                                           \
            if (k == cpp) {                                             \
                point->x = j;                                           \
                point->y = i;                                           \
                return MERR_OK;                                         \
            }                                                           \
        }                                                               \
        posa += mat->step;                                              \
    }

    _MCV_MAT_GETTER(mat->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}

MERR* mcv_matrix_submat_position(MCV_MAT *mata, MCV_MAT *matb,
                                 int direction, MCV_POINT *point)
{
    MERR_NOT_NULLC(mata, matb, point);

    point->x = point->y = -1;

    if (mata->type != matb->type) return merr_raise(MERR_ASSERT, "type mismatch");

    if (matb->rows > mata->rows || matb->cols > mata->cols)
        return merr_raise(MERR_ASSERT, "window size exceed %d %d %d %d",
                          matb->rows, matb->cols, mata->rows, mata->cols);

    if (matb->rows <= 0 || matb->cols <= 0) return MERR_OK;

    int cpp = MCV_GET_CPP(mata->type);
    int rows = mata->rows - matb->rows;
    int cols = mata->cols - matb->cols;
    unsigned char *posb = matb->data.u8;
    float epsilon = (MCV_GET_BPC(mata->type) == MCV_BPC_8U ||
                     MCV_GET_BPC(mata->type) == MCV_BPC_32S ||
                     MCV_GET_BPC(mata->type) == MCV_BPC_64S) ? 1 : 1e-4;

    unsigned char *posa;
    int istart, jstart, istep, jstep;

    switch (direction) {
    case MCV_DIR_NW:
    default:
        posa = mata->data.u8;
        istart = 0;
        jstart = 0;
        istep = 1;
        jstep = 1;
        break;
    case MCV_DIR_NE:
        posa = mata->data.u8;
        istart = 0;
        jstart = mata->cols - matb->cols;
        istep = 1;
        jstep = -1;
        break;
    case MCV_DIR_SW:
        posa = mata->data.u8 + (mata->rows - matb->rows) * mata->step;
        istart = mata->rows - matb->rows;
        jstart = 0;
        istep = -1;
        jstep = 1;
        break;
    case MCV_DIR_SE:
        posa = mata->data.u8 + (mata->rows - matb->rows) * mata->step;
        istart = mata->rows - matb->rows;
        jstart = mata->cols - matb->cols;
        istep = -1;
        jstep = -1;
        break;
    }

    int k, counter;

#define FOR_BLOCK(_, getter)                                            \
    for (int i = istart, icounter = 0; icounter < rows; i += istep, icounter++) { \
        /* 比较该行是否满足要求 */                                      \
        for (int j = jstart, jcounter = 0; jcounter < cols; j += jstep, jcounter++) { \
            counter = 0;                                                \
            while (counter < matb->cols) {                              \
                k = 0;                                                  \
                while (k < cpp &&                                       \
                       fabs((double)(getter(posa, (j + counter) * cpp + k) - \
                                     getter(posb, counter * cpp + k))) < epsilon) { \
                    k++;                                                \
                }                                                       \
                if (k == cpp) counter++;                                \
                else break;                                             \
            }                                                           \
            if (counter == matb->cols) {                                \
                MCV_MAT matx = mcv_matrix_attach(mata,                  \
                                                 mcv_rect(j, i, matb->cols, matb->rows)); \
                if (mcv_matrix_eq(&matx, matb)) {                       \
                    point->x = j;                                       \
                    point->y = i;                                       \
                    goto found;                                         \
                }                                                       \
            }                                                           \
        }                                                               \
        posa += mata->step * istep;                                     \
    }

    _MCV_MAT_GETTER(mata->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;

found:
    switch (direction) {
    case MCV_DIR_NW:
    default:
        break;
    case MCV_DIR_NE:
        point->x += matb->cols;
        break;
    case MCV_DIR_SW:
        point->y += matb->rows;
        break;
    case MCV_DIR_SE:
        point->x += matb->cols;
        point->y += matb->rows;
        break;
    }

    return MERR_OK;
}

MERR* mcv_matrix_subwin_position(MCV_MAT *mat, MCV_SIZE msize, MCV_PIXEL pixel,
                                 int direction, MCV_POINT *point)
{
    MERR *err;

    MERR_NOT_NULLB(mat, point);

    if (mat->type != pixel.type) return merr_raise(MERR_ASSERT, "type mismatch");

    MCV_MAT *matx = mcv_matrix_new(msize.h, msize.w, pixel.type);

    err = mcv_rect_set_pixel(matx, mcv_rect(0, 0, matx->cols, matx->rows), pixel);
    if (err) return merr_pass(err);

    err = mcv_matrix_submat_position(mat, matx, direction, point);
    if (err) return merr_pass(err);

    mcv_matrix_destroy(&matx);

    return MERR_OK;
}

MERR* mcv_vertical_rotate(MCV_MAT *mata, MCV_MAT *matb, MCV_PIXEL padpix, float r)
{
    MERR_NOT_NULLB(mata, matb);

    if (mata->type != matb->type ||
        mata->type != padpix.type ||
        mata->rows != matb->rows ||
        mata->cols != matb->cols) return merr_raise(MERR_ASSERT, "mat mismatch");

    int cpp = MCV_GET_CPP(mata->type);
    unsigned char *posa = mata->data.u8;
    unsigned char *posb = matb->data.u8;
    unsigned char *posp = padpix.data.u8;

    r = 2 * MCV_PI - r;
    float sn = sin(r), cs = cos(r);

#define FOR_BLOCK(setter, getter)                                       \
    for (int i = 0; i < matb->rows; i++) {                              \
        for (int j = 0; j < matb->cols; j++) {                          \
            /* m = cos(a) * x - sin(a) * y, n = sin(a) * x + cos(a) * y */ \
            int sx = cs * j;                                            \
            int sy = sn * j;                                            \
            sy += i;                                                    \
            if (sx >= 0 && sx <= mata->cols && sy >= 0 && sy <= mata->rows) { \
                for (int k = 0; k < cpp; k++) {                         \
                    setter(posb, j * cpp + k,                           \
                           getter(posa, (sy * mata->step) + sx * cpp + k)); \
                }                                                       \
            } else {                                                    \
                for (int k = 0; k < cpp; k++) {                         \
                    setter(posb, j * cpp + k, getter(posp, k));         \
                }                                                       \
            }                                                           \
        }                                                               \
        posb += matb->step;                                             \
    }

    _MCV_MAT_SETTER(matb->type, _MCV_MAT_GETTER, mata->type, FOR_BLOCK);
#undef FOR_BLOCK

    return MERR_OK;
}
