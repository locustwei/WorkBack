#pragma once
#include "atlimage.h"

 /*九宫法拉伸（压缩）图片
 把原图片拆分成九个部分：
 左上、上中、右上
 左中、中心、右中
 左下、下中、右下。
 其中4个角保持（左上、右上、 左下、右下）不变；
 上中、下中水平拉伸；
 左中、右中垂直拉伸；
 中心则水平和垂直同时拉伸。

 同理缩小图片。
 */
typedef struct tagImageSplit{ 
	UINT nFixLeft;
	UINT nFixTop;
	UINT nFixRight;            //从0开始的右边距。
	UINT nFixBottm;            //从0开始的下边距。
}IMAGESPLIT, *LPIMAGESPLIT;

class CLdImage :public CImage
{
public:
	CLdImage(void);
	~CLdImage(void);

	IMAGESPLIT m_ImgSplit;

	BOOL DrawSplit(HDC hDestDC, const RECT& rectDest);           //九宫拉伸（压缩）画。
	void ImgageTran();                                           //透明处理（有alpha值的图片需要处理一下才能画出透明效果来）
};

