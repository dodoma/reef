#ifndef __MCV_H__
#define __MCV_H__

/*
 * mcv, moon computer vision
 *
 * 恩，又一个机器视觉的库。
 * 类似 ccv, 但 ccv 更偏向模式识别、人工神经网络之类，mcv 更偏向单一的机器视觉、工业检测。
 * mcv 的目标是实现一套易用、高效的类 opencv core, imageproc 工具函数
 *
 * Many Thanks To [ccv](http://libccv.org/) by liuliu
 */
__BEGIN_DECLS

MCV_MAT* mcv_matrix_new(int rows, int cols, int type);
MCV_MAT  mcv_matrix(int rows, int cols, int type, void *data);

void mcv_matrix_destroy(MCV_MAT **mat);

/*
 * 计算矩阵内所有元素之和
 */
double mcv_summary(MCV_MAT *mat, int flag);

/*
 * 计算矩阵内从左至右 第一条竖线的倾斜角度（常用于矩形物体旋转度计算）
 * 返回弧度值
 */
MERR* mcv_vline_angle(MCV_MAT *mat, int targetv, float *r);

/*
 * 找到矩阵内从上自下 第一条水平直线的左边起点（常用于物体顶点定位）
 * 输入矩阵必须是已经旋转至平放的
 */
MERR* mcv_hline_left_point(MCV_MAT *mat, int targetv, int step, MCV_POINT *point);

__END_DECLS
#endif
