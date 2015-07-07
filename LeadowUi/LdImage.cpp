#include "StdAfx.h"
#include "LdImage.h"
#include "LdGraphicFuncs.h"


CLdImage::CLdImage(void)
{
	ZeroMemory(&m_ImgSplit, sizeof(IMAGESPLIT));
}


CLdImage::~CLdImage(void)
{
}

BOOL CLdImage::DrawSplit( HDC hDestDC, const RECT& rectDest )
{
	UINT nWidth=GetWidth();
	UINT nHeight=GetHeight();

	if( ((m_ImgSplit.nFixLeft>nWidth)||(m_ImgSplit.nFixTop>nHeight)) ||           //边界错误
		((m_ImgSplit.nFixRight-m_ImgSplit.nFixLeft<=0)||(m_ImgSplit.nFixBottm-m_ImgSplit.nFixTop<=0))//面积为0
		)
		return CImage::Draw(hDestDC, rectDest);

	//HDC hDc=CreateCompatibleDC(hDestDC);
	HDC hDc=hDestDC;
	if(!hDc)
		return FALSE;


	CRect crImg(0, 0, m_ImgSplit.nFixLeft, m_ImgSplit.nFixTop);                                                   //Image左上角
	CRect crDest(rectDest.left, rectDest.top, rectDest.left+crImg.Width(), rectDest.top+crImg.Height());           //目标左上角
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.left=m_ImgSplit.nFixLeft;
	crImg.right=m_ImgSplit.nFixRight;                                                                             //Image上中间
	crDest.left=rectDest.left+m_ImgSplit.nFixLeft;
	crDest.right=rectDest.right-(nWidth-m_ImgSplit.nFixRight);                                                    //目标上中
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.left=m_ImgSplit.nFixRight;
	crImg.right=nWidth;                                                                                           //Image右上角
	crDest.left=rectDest.right-crImg.Width();
	crDest.right=rectDest.right;                                                                                  //目标右上角
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.top=m_ImgSplit.nFixTop;
	crImg.bottom=m_ImgSplit.nFixBottm;                                                                            //Image右中
	crDest.top=rectDest.top+m_ImgSplit.nFixTop;
	crDest.bottom=rectDest.bottom-(nHeight-m_ImgSplit.nFixBottm);                                                 //目标右中
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.top=m_ImgSplit.nFixBottm;
	crImg.bottom=nHeight;                                                                                         //Image右下
	crDest.top=rectDest.bottom-crImg.Height();
	crDest.bottom=rectDest.bottom;                                                                                //目标右下
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.left=m_ImgSplit.nFixLeft;
	crImg.right=m_ImgSplit.nFixRight;                                                                             //Imag下中
	crDest.left=rectDest.left+m_ImgSplit.nFixLeft;
	crDest.right=rectDest.right-(nWidth-m_ImgSplit.nFixRight);                                                    //目标下中
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.left=0;
	crImg.right=m_ImgSplit.nFixLeft;                                                                              //Image左下
	crDest.left=rectDest.left;
	crDest.right=rectDest.left+m_ImgSplit.nFixLeft;                                                               //目标左下
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.top=m_ImgSplit.nFixTop;
	crImg.bottom=m_ImgSplit.nFixBottm;                                                                            //Image左中
	crDest.top=rectDest.top+m_ImgSplit.nFixTop;
	crDest.bottom=rectDest.bottom-(nHeight-m_ImgSplit.nFixBottm);                                                 //目标左中
	CLdImage::Draw(hDc, crDest, crImg);

	crImg.left=m_ImgSplit.nFixLeft;
	crImg.right=m_ImgSplit.nFixRight;                                                                             //Image中心
	crDest.left=rectDest.left+m_ImgSplit.nFixLeft;
	crDest.right=rectDest.right-(nWidth-m_ImgSplit.nFixRight);                                                    //目标中心
	CLdImage::Draw(hDc, crDest, crImg);

	return TRUE;
}

void CLdImage::ImgageTran()
{
	PngImgTransparency(this);
}
