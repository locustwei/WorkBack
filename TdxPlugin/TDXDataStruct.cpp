/************************************************************************/
/* 通达信数据结构
和全局数据。
*/
/************************************************************************/

#include "TDXDataStruct.h"
#include "..\PublicLib\comps\LdXml.h"


LPTSTR AddPlugInConfig(CLdXml* xml)
{
	XMLNode node = xml->FindNode(L"//Addin");
	if(!node)
		return NULL;
	XMLList childs;
	LPTSTR fName = NULL;

	if(SUCCEEDED(node->get_childNodes(&childs))){
		long lengtth;
		childs->get_length(&lengtth);
		for(int i=0; i<lengtth; i++){
			IXMLDOMNode* child;
			childs->get_item(i, &child);

			fName = xml->GetNodeAttrubeAsStr(node, L"FileName");
			if(fName){
				if(wcscmp(PLUG_FILENAME, fName)==0)
					return xml->GetNodeAttrubeAsStr(node, L"MapFile");
			}
		}

		XMLNode xmlnode = xml->AddChild(node, L"ITEM");
		xml->setAttribute(xmlnode, L"FileName", PLUG_FILENAME);
		xml->setAttribute(xmlnode, L"Pivotal", L"NO");
		xml->setAttribute(xmlnode, L"Enable", L"YES");
		xml->setAttribute(xmlnode, L"MapFile", fName);
	}

	return fName;
}

/*
Nav_Tree NavTree = {0};

//读取Tdx左侧导航栏功能菜单
void LoadNavTree(CLdXml& xml, IXMLDOMNode* node, PNav_Tree navtree)
{
	if(node==NULL || navtree==NULL)
		return;

	LPTSTR nv = xml.GetNodeAttrubeAsStr(node, L"ID");
	if(nv){
		navtree->ID = nv;
	}
	nv = xml.GetNodeAttrubeAsStr(node, L"FeatureID");
	if(nv)
		navtree->FeatureID = nv;
	nv = xml.GetNodeAttrubeAsStr(node, L"Title");
	if(nv)
		navtree->Title = nv;
	nv = xml.GetNodeAttrubeAsStr(node, L"HomeFeatureID");
	if(nv)
		navtree->HomeFeatureID = nv;
	nv = xml.GetNodeAttrubeAsStr(node, L"HotKeytc");
	if(nv){
		navtree->HotKeytc = nv;
		LPTSTR title = new TCHAR[80];
		ZeroMemory(title, 80*sizeof(TCHAR));
		wsprintf(title, navtree->Title, L"[", nv, L"]");
		delete navtree->Title;
		navtree->Title = title;
	}


	XMLList childs;
	if(SUCCEEDED(node->get_childNodes(&childs))){
		childs->get_length((long*)&navtree->count);
		navtree->Items = new Nav_Tree[navtree->count];
		for(int i=0; i<navtree->count; i++){
			IXMLDOMNode* child;
			childs->get_item(i, &child);
			LoadNavTree(xml, (IXMLDOMNode*)child, &navtree->Items[i]);
		}
	}
}

BOOL InitNavTree()
{
	CLdXml xml;
	if(xml.OpenFile(L"..\\TcOem.xml")){
		XMLNode node = xml.FindNode(L"//UI/Nav/Content");
		if(node){
			LoadNavTree(xml, node, &NavTree);
			return TRUE;
		}
	}
	return FALSE;
}

int GetNavTreeItemCount(PNav_Tree treeitem)
{
	int result = 0;
	for(int i=0; i<treeitem->count; i++)
		result += GetNavTreeItemCount(&treeitem->Items[i]);
	return result + treeitem->count;
}
*/