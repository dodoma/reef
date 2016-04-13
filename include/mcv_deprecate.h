#ifndef __MCV_DEPRECATE_H__
#define __MCV_DEPRECATE_H__

/*
 * 找到矩阵内从上自下 第一条水平直线的左边起点（常用于物体顶点定位）
 * 输入矩阵必须是已经旋转至平放的
 */
MERR* mcv_hline_left_point(MCV_MAT *mat, int targetv, int step, MCV_POINT *point);

#endif
