#ifndef __MCV_FORSPEED_H__
#define __MCV_FORSPEED_H__

__BEGIN_DECLS

static inline MCV_PIXEL mcv_pixel(int type, void *data)
{
    MCV_PIXEL pixel;
    pixel.type = type;
    pixel.data.u8 = (unsigned char*)data;
    return pixel;
}

static inline MCV_PIXEL mcv_pixel_gray(unsigned char *v)
{
    MCV_PIXEL pixel;
    pixel.type = MCV_DATA_GRAY;
    pixel.data.u8 = v;
    return pixel;
}

static inline MCV_POINT mcv_point(int x, int y)
{
    MCV_POINT point;
    point.x = x;
    point.y = y;
    return point;
}

static inline MCV_DECIMAL_POINT mcv_decimal_point(float x, float y)
{
    MCV_DECIMAL_POINT point;
    point.x = x;
    point.y = y;
    return point;
}

static inline MCV_SIZE mcv_size(int width, int height)
{
    MCV_SIZE size;
    size.w = width;
    size.h = height;
    return size;
}

static inline int mcv_size_is_zero(MCV_SIZE size)
{
    return size.w == 0 && size.h == 0;
}

static inline MCV_RECT mcv_rect(int x, int y, int width, int height)
{
    MCV_RECT rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    return rect;
}

static inline bool mcv_matrix_match(MCV_MAT *mata, MCV_MAT *matb)
{
    if (!mata || !matb) return false;

    if (mata->type != matb->type ||
        mata->rows != matb->rows ||
        mata->cols != matb->cols) return false;

    return true;
}

__END_DECLS
#endif
