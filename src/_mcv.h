#ifndef ___MCV_H__
#define ___MCV_H__

#include <jpeglib.h>

#define _MCV_GET_8U_VALUE(pos, i)  (((unsigned char*)(pos))[(i)])
#define _MCV_GET_32S_VALUE(pos, i) (((int*)(pos))[(i)])
#define _MCV_GET_32F_VALUE(pos, i) (((float*)(pos))[(i)])
#define _MCV_GET_64S_VALUE(pos, i) (((int64_t*)(pos))[(i)])
#define _MCV_GET_64F_VALUE(pos, i) (((double*)(pos))[(i)])

#define _MCV_MAT_GETTER(type, block, ...)           \
    {                                               \
        switch (MCV_GET_BPC(type)) {                \
        case MCV_BPC_32S:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_GET_32S_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_32F:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_GET_32F_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_64S:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_GET_64S_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_64F:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_GET_64F_VALUE); \
            break;                                  \
        }                                           \
        default:                                    \
        {                                           \
            block(__VA_ARGS__, _MCV_GET_8U_VALUE);  \
            break;                                  \
        }                                           \
        }                                           \
    }


#define _MCV_SET_8U_VALUE(pos, i, value) \
    (((unsigned char*)(pos))[(i)] = MCV_CLAMP((int)(value), 0, 255))
#define _MCV_SET_32S_VALUE(pos, i, value) (((int*)(pos))[(i)] = (int)(value))
#define _MCV_SET_32F_VALUE(pos, i, value) (((float*)(pos))[(i)] = (float)(value))
#define _MCV_SET_64S_VALUE(pos, i, value) (((int64_t*)(pos))[(i)] = (int64_t)(value))
#define _MCV_SET_64F_VALUE(pos, i, value) (((double*)(pos))[(i)] = (double)(value))

#define _MCV_MAT_SETTER(type, block, ...)           \
    {                                               \
        switch (MCV_GET_BPC(type)) {                \
        case MCV_BPC_32S:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_SET_32S_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_32F:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_SET_32F_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_64S:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_SET_64S_VALUE); \
            break;                                  \
        }                                           \
        case MCV_BPC_64F:                           \
        {                                           \
            block(__VA_ARGS__, _MCV_SET_64F_VALUE); \
            break;                                  \
        }                                           \
        default:                                    \
        {                                           \
            block(__VA_ARGS__, _MCV_SET_8U_VALUE);  \
            break;                                  \
        }                                           \
        }                                           \
    }

MCV_MAT* mcv_read_jpeg(FILE* in, int type);
void mcv_write_jpeg(MCV_MAT* mat, FILE* fd);

#endif
