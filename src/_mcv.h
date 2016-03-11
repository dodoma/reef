#ifndef ___MCV_H__
#define ___MCV_H__

#define _MCV_GET_8U_VALUE(pos, i)  (((unsigned char*)(pos))[i])
#define _MCV_GET_32S_VALUE(pos, i) (((int*)(pos))[i])
#define _MCV_GET_32F_VALUE(pos, i) (((float*)(pos))[i])
#define _MCV_GET_64S_VALUE(pos, i) (((int64_t*)(pos))[i])
#define _MCV_GET_64F_VALUE(pos, i) (((double*)(pos))[i])

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

#endif
