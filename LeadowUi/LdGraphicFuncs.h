#pragma once

#include "LdImage.h"


//************************************
// Qualifier:设置CImage的透明。有透明度的图片需调用此方法处理
// Parameter: CImage * image
//************************************
void PngImgTransparency(CLdImage* image);
//************************************
// Qualifier: 水平方向平均分割图片。
// Parameter: CImage * image 原图片
// Parameter: int nCount  要分成多少个
// Parameter: CImage * * result 接收分隔后的图片数组（要事先分配好内存空间）
// return 返回成功与否
//************************************
BOOL HSplitImg(CLdImage* image, int nCount, CLdImage** result);

void DrawCtrlBackgnd(HDC pDc, RECT rect, CLdImage* img);
