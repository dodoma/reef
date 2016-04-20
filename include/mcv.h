#ifndef __MCV_H__
#define __MCV_H__

/*
 * mcv, moon computer vision
 *
 * 恩，又一个机器视觉的库。
 * 类似 ccv, 但 ccv 更偏向模式识别、人工神经网络之类，mcv 更偏向单一的机器视觉、工业检测。
 * mcv 的目标是实现一套易用、高效的类 opencv core, imageproc, feature 2d 工具函数
 *
 * Many Thanks To [ccv](http://libccv.org/) by liuliu
 */
__BEGIN_DECLS

MCV_MAT* mcv_matrix_new(int rows, int cols, int type);
MCV_MAT  mcv_matrix(int rows, int cols, int type, void *data);

void mcv_matrix_destroy(MCV_MAT **mat);

/*
 * 取矩阵的子矩阵，使用原矩阵内存
 * 类似 numpy 中的 new_image = image[y:y+rows, x:x+cols]
 * 注意：该函数速度虽然很快，但对其进行操作需要注意，该矩阵实际值在内存中可能是不连续的。
 */
MCV_MAT mcv_matrix_attach(MCV_MAT *mat, MCV_RECT rect);
/*
 * 取矩阵的子矩阵，新申请内存
 */
MCV_MAT* mcv_matrix_detach(MCV_MAT *mat, MCV_RECT rect);

/*
 * 矩阵克隆，新申请内存，
 *   等价于 mcv_matrix_detach(mat, mcv_rect(0, 0, mat->cols, mat->rows))
 *   等价于 numpy.copy()
 * 但速度更快，特别是对于 rows 很大的 matrix
 */
MCV_MAT* mcv_matrix_clone(MCV_MAT *mat);

/*
 * 判断2矩阵是否相等
 */
bool mcv_matrix_eq(MCV_MAT *mata, MCV_MAT *matb);

/*
 * 将矩阵，指定矩形内元素赋值为 pixel， rect 为 NULL 时，操作全部矩阵
 */
MERR* mcv_matrix_set_pixel(MCV_MAT *mat, MCV_RECT rect, MCV_PIXEL pixel);
MERR* mcv_matrix_set_gray(MCV_MAT *mat, MCV_RECT rect, unsigned char v);


/*
 * 计算矩阵内所有元素之和
 */
double mcv_summary(MCV_MAT *mat, int flag);

/*
 * 计算矩阵内所有指定像素个数
 */
unsigned int mcv_pixel_number(MCV_MAT *mat, MCV_PIXEL pixel);

/*
 * 查找位置
 * TODO direction flag
 */
MERR* mcv_pixel_position(MCV_MAT *mat, MCV_PIXEL pixel, MCV_POINT *point);
MERR* mcv_matrix_submat_position(MCV_MAT *mata, MCV_MAT *matb,
                                 int direction, MCV_POINT *point);
MERR* mcv_matrix_subwin_position(MCV_MAT *mat, MCV_SIZE msize, MCV_PIXEL pixel,
                                 int direction, MCV_POINT *point);

/*
 * 计算矩阵内从左至右 第一条竖线的倾斜角度（常用于矩形物体旋转度计算）
 * 返回弧度值
 */
MERR* mcv_vline_angle(MCV_MAT *mat, int targetv, float *r);

/*
 * 计算矩阵内从上至下 第一条横线的倾斜角度（常用于矩形物体旋转度计算）
 * 返回弧度值
 */
MERR* mcv_hline_angle(MCV_MAT *mat, int targetv, float *r);


/*
 * 将矩阵绕Y轴（旋转）角度r
 * 旋转：Y轴不变，逐行、绕该行第一个像素进行旋转，故离Y轴越近，旋转距离越小，反之越大
 * 内部实现为，目标图中的每个像素，绕该行的第一个像素，从源图中找对应坐标，取值、赋值
 */
MERR* mcv_vertical_rotate(MCV_MAT *mata, MCV_MAT *matb, MCV_PIXEL padpix, float r);


__END_DECLS
#endif
