#ifndef __MCV_MACRO_H__
#define __MCV_MACRO_H__

__BEGIN_DECLS

#define MCV_PI (3.141592653589793)
#define MCV_DEG_PER_RAD (57.295779513082323)
#define MCV_DEGREE_2_RADIAN(x) ((x)/MCV_DEG_PER_RAD)
#define MCV_RADIAN_2_DEGREE(x) ((x)*MCV_DEG_PER_RAD)

static const int m_channel_size[] = {
    -1, 1, 4, -1, 4, -1, -1, -1, 8, -1, -1, -1, -1, -1, -1, -1, 8
};

#define MCV_GET_CPP(type) ((type) & 0xFFF)
#define MCV_GET_BPC(type) ((type) & 0xFF000)
#define MCV_GET_CHANNEL_SIZE(type) (m_channel_size[MCV_GET_BPC(type) >> 12])

#define MCV_MATRIX_SIZE(rows, cols, type) ( sizeof(MCV_MAT) +           \
                                            (((cols) *                  \
                                              MCV_GET_CPP(type) *       \
                                              MCV_GET_CHANNEL_SIZE(type) + 3) & \
                                             -4) * rows )

#define MCV_MATRIX_DATA_SIZE(rows, cols, type) ( (((cols) *             \
                                                   MCV_GET_CPP(type) *  \
                                                   MCV_GET_CHANNEL_SIZE(type) + 3) & \
                                                  -4) * rows )

/*
 * basic utils
 */
#define MCV_CLAMP(x, a, b) ( ((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define MCV_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MCV_MAX(a, b) ((a) > (b) ? (a) : (b))



/*
 * access utils
 * row, col, ch start from 0
 */
#define MCV_GET_GEO_MEM(mat, row, col, ch)                              \
    ( ((mat)->type & MCV_BPC_8U) ?                                      \
      (void*)((mat)->data.u8 +                                          \
              (row) * (mat)->step + (col) * MCV_GET_CPP((mat)->type) + (ch)) : \
      ((mat)->type & MCV_BPC_32S) ?                                     \
      (void*)((mat)->data.i32 +                                         \
              ((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)) : \
      ((mat)->type & MCV_BPC_32F) ?                                     \
      (void*)((mat)->data.f32 +                                         \
              ((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)) : \
      ((mat)->type & MCV_BPC_64S) ?                                     \
      (void*)((mat)->data.i64 +                                         \
              ((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)) : \
      (void*)((mat)->data.f64 +                                         \
              ((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)) )

#define MCV_GET_GEO_VALUE(mat, row, col, ch)                            \
    ( ((mat)->type & MCV_BPC_8U) ?                                      \
      (mat)->data.u8[(row) * (mat)->step + (col) * MCV_GET_CPP((mat)->type) + (ch)] : \
      ((mat)->type & MCV_BPC_32S) ?                                     \
      (mat)->data.i32[((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)] : \
      ((mat)->type & MCV_BPC_32F) ?                                     \
      (mat)->data.f32[((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)] : \
      ((mat)->type & MCV_BPC_64S) ?                                     \
      (mat)->data.i64[((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)] : \
      (mat)->data.f64[((row) * (mat)->cols + (col)) * MCV_GET_CPP((mat)->type) + (ch)] )

#define MCV_GET_MEM_VALUE(type, ptr, i)                     \
    ( ((type) & MCV_BPC_8U) ? ((unsigned char*)(ptr))[i] :  \
      ((type) & MCV_BPC_32S) ? ((int*)(ptr))[i] :           \
      ((type) & MCV_BPC_32F) ? ((float*)(ptr))[i] :         \
      ((type) & MCV_BPC_64S) ? ((int64_t*)(ptr))[i] :       \
      ((double*)(ptr))[i] )

#define MCV_SET_MEM_VALUE(type, ptr, i, value)                          \
    switch (MCV_GET_BPC(type)) {                                        \
    case MCV_BPC_32S: ((int*)(ptr))[(i)] = (int)(value); break;         \
    case MCV_BPC_32F: ((float*)(ptr))[(i)] = (float)(value); break;     \
    case MCV_BPC_64S: ((int64_t*)(ptr))[(i)] = (int64_t)(value); break; \
    case MCV_BPC_64F: ((double*)(ptr))[(i)] = (double)(value); break;   \
    default: ((unsigned char*)(ptr))[(i)] = MCV_CLAMP((int)(value), 0, 255); break; }



__END_DECLS
#endif
