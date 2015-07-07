#include "stdafx.h"
#include "LdGraphicFuncs.h"

void PngImgTransparency(CLdImage* image)
{
	if((image==NULL)||(image->IsNull())||(image->GetBPP()!=32))
		return;
	for(int x=0; x<image->GetWidth(); x++)
		for(int y=0; y<image->GetHeight(); y++){
			PBYTE pDes=(PBYTE)image->GetPixelAddress(x, y);
			pDes[0]=pDes[0]*pDes[3]/0xFF;
			pDes[1]=pDes[1]*pDes[3]/0xFF;
			pDes[2]=pDes[2]*pDes[3]/0xFF;
			pDes[3]=pDes[3];
		}
}

BOOL HSplitImg(CLdImage* image, int nCount, CLdImage** result)
{
	if((image==NULL)||(image->IsNull()))
		return FALSE;  

	if( image->GetWidth() % nCount!=0)
		return FALSE;                       //图片宽度不能被整除。

	int nFlag=0;
	int nWidth=image->GetWidth()/ nCount;
	int nHeight=image->GetHeight();
	if(image->GetBPP()==32)
		nFlag=AC_SRC_ALPHA;                //需要透明
	CRect rDes(0, 0, nWidth, nHeight);
	POINT ptSrc = {0, 0};

	for(int i=0; i<nCount; i++){
		result[i]=new CLdImage();
		result[i]->Create(nWidth, image->GetHeight(), image->GetBPP(), nFlag);
		ptSrc.x=i*nWidth;
		image->BitBlt(result[i]->GetDC(), rDes, ptSrc);
		result[i]->ReleaseDC();
	}
	return TRUE;
}

void DrawCtrlBackgnd( HDC pDc, RECT rect, CLdImage* img )
{
	/*
	CBitmap cbmp;
	int nWidth=rect.right-rect.left;
	int nHeight=rect.bottom-rect.top;
	cbmp.CreateCompatibleBitmap(pDc, nWidth, nHeight);
	HDC dc;
	dc.CreateCompatibleDC(pDc);
	dc.SelectObject(cbmp.GetSafeHandle());
	img->DrawSplit(dc.GetSafeHdc(), rect);
	if(TRUE&&img->GetBPP()==32){
		BLENDFUNCTION blf={0};
		blf.AlphaFormat=AC_SRC_ALPHA;
		blf.SourceConstantAlpha=0xFF;
		pDc->AlphaBlend(rect.left, rect.top, nWidth, nHeight, &dc, 0, 0, nWidth, nHeight, blf);
	}
	else
		pDc->BitBlt(rect.left, rect.top, nWidth, nHeight, &dc, 0, 0, SRCCOPY);
		*/
	img->DrawSplit(pDc, rect);
}

