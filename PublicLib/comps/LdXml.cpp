#include "LdXml.h"
#include <Propvarutil.h>

#pragma comment(lib, "Propsys.lib")


CLdXml::CLdXml()
{
	::CoInitialize(NULL);
	pXMLDoc = NULL;

	HRESULT hr = CoInitialize(NULL); 
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
		IID_IXMLDOMDocument, (void**)&pXMLDoc);
};

CLdXml::~CLdXml()
{
	::CoUninitialize();
};

BOOL CLdXml::OpenFile(LPCTSTR szFileName)
{
	if(!pXMLDoc)
		return FALSE;

	VARIANT_BOOL succeed = VARIANT_FALSE;

	pXMLDoc->load(_variant_t(szFileName), &succeed);
	if(succeed){
		HRESULT hr = pXMLDoc->get_documentElement(&pRoot);
		if(!SUCCEEDED(hr)){
			pXMLDoc->Release();
			pXMLDoc = NULL;
			return FALSE;
		}
	}
	return succeed;
}

XMLNode CLdXml::FindNode(LPCTSTR szPath)
{
	if(pRoot==NULL)
		return NULL;
	XMLNode result = NULL;
	HRESULT hr = pRoot->selectSingleNode(_bstr_t(szPath), (IXMLDOMNode**)&result);
	if(SUCCEEDED(hr))
		return result;
	else
		return NULL;
}

XMLNode CLdXml::AddChild( XMLNode node, LPCTSTR szName )
{
	if(!pXMLDoc)
		return NULL;

	XMLNode result = NULL;

	HRESULT hr = pXMLDoc->createElement(_bstr_t(szName), &result);
	if(SUCCEEDED(hr)){
		node->appendChild(result, (IXMLDOMNode**)&result);
	}
	return result;
}

BOOL CLdXml::setAttribute(XMLNode node, LPCTSTR szName, LPCTSTR value)
{

	return SUCCEEDED(node->setAttribute(_bstr_t(szName), _variant_t(value)));
}

BOOL CLdXml::setAttribute(XMLNode node, LPCTSTR szName, VARIANT value)
{
	return SUCCEEDED(node->setAttribute(_bstr_t(szName), value));
}

BOOL CLdXml::SaveFile( LPCTSTR szFileName )
{
	if(!pXMLDoc)
		return FALSE;
	return SUCCEEDED(pXMLDoc->save(_variant_t(szFileName)));
}

LPTSTR CLdXml::GetNodeAttrubeAsStr(IXMLDOMNode* node, LPCTSTR szName)
{
	IXMLDOMNamedNodeMap* attributes;
	if(SUCCEEDED(node->get_attributes(&attributes))){
		IXMLDOMNode* att;
		if(SUCCEEDED(attributes->getNamedItem(_bstr_t(szName), &att)) && att!=NULL){
			VARIANT value;
			if(SUCCEEDED(att->get_nodeValue(&value))){
				LPTSTR result;
				if(SUCCEEDED(VariantToStringAlloc(value, &result)))
					return result;
			}
		}
	}
	return NULL;
}

XMLList CLdXml::FindNodes(LPCTSTR szPath)
{
	XMLList result;

	if(SUCCEEDED(pRoot->selectNodes(_bstr_t(szPath), &result)))
		return result;
	else
		return NULL;
}

