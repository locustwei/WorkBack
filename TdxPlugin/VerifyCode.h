/************************************************************************/
/* 登录窗口的验证码破解                                                 */
/************************************************************************/

#pragma once

#include "stdafx.h"

//验证码编码（像素点阵）。每一位表示一个像素，一个DWORD表示一列
struct CODECONST{  
	BYTE length;  //数字最大14，实际可能不需要。
	DWORD yzm[14];
};

//各数字：出现过的编码情况（由于字体大小变化，画笔偏移的误差使得编码有变化）。
const CODECONST VCODE0[] = {
	{6,{496,1560,7183,7174,1548,504}},
	{6,{504,1548,7175,7175,1548,504}},
	{7,{504,1548,1030,6147,3078,1548,504}},
	{7,{504,1548,3078,6147,3078,1548,504}},
	{7,{1016,3084,6150,12295,6156,3096,1008}},
	{7,{2040,6156,12294,24591,12312,6192,2016}},
	{8,{2040,6156,12294,24579,24588,12312,6192,2016}}
};
const CODECONST VCODE1[] = {
	{3,{2050,4095,2048}},
	{3,{4098,8191,4096}},
	{3,{8194,16383,8192}},
	{3,{16386,32767,16384}}
};
const CODECONST VCODE2[] = {
	{9,{5127,20993,20737,20609,20545,20513,20511,20494,6144}},
	{9,{9223,8705,8449,8321,8257,8225,8223,8206,12288}},
	{10,{14342,9223,8705,8449,8321,8257,8225,8223,8206,12288}},
	{10,{14342,9223,8705,24833,8321,24641,24609,24607,24590,12288}}
};
const CODECONST VCODE3[] = {
	{8,{28679,16449,16449,16449,16449,16449,32767,8094}},
	{9,{1542,3591,2113,2113,2113,2113,2113,4095,926}},
	{9,{3078,7175,4161,4161,4161,4161,4161,8191,1950}},
	{9,{6150,14343,8257,8257,8257,8257,8257,16383,3998}},
	{9,{12294,28679,16449,16449,16449,16449,16449,32767,8094}}
};
const CODECONST VCODE4[] = {
	{8,{95,64,64,32764,32764,64,64,64}},
	{9,{127,95,64,64,8188,8188,64,64,64}},
	{9,{127,95,64,64,16380,16380,64,64,64}},
	{9,{127,95,64,64,32764,32764,64,64,64}},
	{10,{96,127,95,64,64,4092,4092,64,64,64}},
	{10,{96,127,95,64,64,8188,8188,64,64,64}},
	{10,{96,127,95,64,64,16380,16380,64,64,64}},
	{10,{96,127,95,64,64,32764,32764,64,64,64}}
};
const CODECONST VCODE5[] = {
	{7,{4159,4161,4161,6209,1088,961,384}},
	{8,{63,4159,4161,4161,6209,1089,961,384}},
	{9,{63,4159,4161,12353,6209,9281,25537,8576,8192}}
};
const CODECONST VCODE6[] = {
	{8,{768,3456,6336,12400,6360,3468,774,3}},
	{8,{1792,6528,12480,24688,12504,6540,1798,3}},
	{8,{3840,12672,24768,49264,24792,12684,3846,3}},
	{8,{7936,24960,49344,98416,49368,24972,7942,3}}
};
const CODECONST VCODE7[] = {
	{8,{1,1,1,61441,7681,961,121,15}},
	{9,{1,1,1,1,28673,7681,961,121,15}},
	{9,{1,1,1,1,61441,7681,961,121,15}}
};
const CODECONST VCODE8[] = {
	{8,{32383,16449,16449,16449,16449,16449,32319,15390}},
	{9,{1950,4095,2113,2113,2113,2113,2113,4095,1950}},
	{9,{3870,8191,4161,4161,4161,4161,4161,8127,3870}}
};
const CODECONST VCODE9[] = {
	{7,{56,204,29062,7939,1798,460,56}},
	{7,{120,396,58118,15875,3590,908,120}}, 
	{7,{248,780,116230,31747,7174,1804,248}},
	{7,{504,1548,232454,63491,14342,3596,504}}
};

//下面定义只是为了程序编写方便
struct CONSTCOUNT{
	BYTE count;
	const CODECONST* yzmarray;
};

CONSTCOUNT VCODECONST[10] = {
	{sizeof(VCODE0)/sizeof(CODECONST), VCODE0},
	{sizeof(VCODE1)/sizeof(CODECONST), VCODE1},
	{sizeof(VCODE2)/sizeof(CODECONST), VCODE2},
	{sizeof(VCODE3)/sizeof(CODECONST), VCODE3},
	{sizeof(VCODE4)/sizeof(CODECONST), VCODE4},
	{sizeof(VCODE5)/sizeof(CODECONST), VCODE5},
	{sizeof(VCODE6)/sizeof(CODECONST), VCODE6},
	{sizeof(VCODE7)/sizeof(CODECONST), VCODE7},
	{sizeof(VCODE8)/sizeof(CODECONST), VCODE8},
	{sizeof(VCODE9)/sizeof(CODECONST), VCODE9}
};

//窗口上的验证码图编码
bool getCodeFromWnd(HWND hwnd, int x, int y, CODECONST* yzm){
	
	//验证码4个字，每个字高度最高18，,宽度最大14。每一位表示一个像素点（背景色为0，其他色为1）
	bool result = false;

	HDC wndDc = 0, hdc = 0;
	HBITMAP memBM=0;
	do 
	{
		wndDc = GetDC(0);
		if(wndDc==0)
			break;

		hdc = CreateCompatibleDC(wndDc);
		if(hdc==0)
			break;;
		HBITMAP memBM = CreateCompatibleBitmap ( wndDc, 100, 20 );

		HGDIOBJ oldBm = SelectObject(hdc, memBM );

		if(!BitBlt(hdc, 0, 0, 60, 20, wndDc, x, y, SRCCOPY))  //Copy到内存提高速度
			break;

		int k = -1, pos = 0;
		for (int i=0; i<60; i++){
			DWORD w = 0;  //临时用，
			for(int j=0; j<18; j++){  
				if((GetPixel(hdc, i, j)&0x00FFFFFF)!=0xC0C0C0)
					w |= (1 << j);
			}

			if(w){
				if(pos==0){
					k++;
					if(k==4)
						break;
				}
				yzm[k].yzm[pos++] = w;
				if(pos==14)
					break; //出错了。
				yzm[k].length = pos;
			}else
				pos = 0;
		}

		SelectObject(hdc, oldBm);
		result = true;
	}while(FALSE);

	if(wndDc!=0)
		ReleaseDC(0, wndDc);
	if(memBM!=0)
		DeleteObject(memBM);
	if(hdc!=0)
		DeleteDC(hdc);

	return result;
}

//比较两个验证码是否相等
//相等的条件是：数组每一项都相等或同一个整数倍
bool compareCode(const DWORD* yzm1, DWORD* yzm2, int n)
{
	bool equ = TRUE;

	int w=0;
	w = yzm2[0] / yzm1[0];        //验证码有可能左右移动，常数已经全部靠左     
	if(w==0){                     //不是整数倍则表示他们不是一个东西
		equ = FALSE;
	}

	for(int j=0; j<=n; j++){
		if(yzm1[j]==0 && yzm2[j]==0)
			continue;
		if((yzm1[j]==0 && yzm2[j]!=0)||
			(yzm2[j]==0 && yzm1[j]!=0)||
			(yzm2[j] / yzm1[j]!=w)){
				equ = FALSE;
				break;
		}
	}

	return equ;
}

BOOL getVierifyCode(HWND hwnd, POINT pos, LPTSTR szCode)
{
	BOOL result = TRUE;
	RECT r = {0};

	CODECONST yzm[4] = {0};
	if(!getCodeFromWnd(hwnd, pos.x, pos.y, yzm))
		return FALSE;

	for(int k=0; k<4; k++){
		BOOL find = FALSE;

		int n = yzm[k].length;  //确定字体高度

		for (int i=0; i<10; i++){
			for (int j=0; j<VCODECONST[i].count; j++){
				if(VCODECONST[i].yzmarray[j].length!=n)
					continue;
				if(compareCode(VCODECONST[i].yzmarray[j].yzm, yzm[k].yzm, n-1)){
					szCode[k] = '0' + i;
					find = TRUE;
					break;
				}
			}
			if(find)
				break;
		}

		if(!find){
			szCode[k] = 'E';
			result = FALSE;
			//break;
		}
	}


	return result;
}
