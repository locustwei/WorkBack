/*
使用MSXML在VC++中解析XML文件时候，只需要做到下面几点：
	1、初始化COM库，CoInitialize(NULL)；可以放在InitInstance()函数里面。释放COM库，CoUninitialize()；可以放在ExitInstance()函数里面。
	2、在头文件里面加入如下代码
*/
#pragma once

#include <comdef.h>


#define XMLNode IXMLDOMElement*
#define XMLList IXMLDOMNodeList*

class CLdXml
{
public:
	CLdXml();
	~CLdXml();
	BOOL OpenFile(LPCTSTR szFileName);
	XMLNode FindNode(LPCTSTR szPath);
	XMLNode AddChild( XMLNode node, LPCTSTR szName );
	BOOL setAttribute(XMLNode node, LPCTSTR szName, VARIANT value);
	BOOL CLdXml::setAttribute(XMLNode node, LPCTSTR szName, LPCTSTR value);
	BOOL SaveFile( LPCTSTR szFileName );
	LPTSTR GetNodeAttrubeAsStr(IXMLDOMNode* node, LPCTSTR szName);
	XMLList FindNodes(LPCTSTR szPath);
private:
	IXMLDOMDocument* pXMLDoc;
	IXMLDOMElement* pRoot;
};
