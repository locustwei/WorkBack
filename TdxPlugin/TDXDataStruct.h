/************************************************************************/
/* 通达信数据结构
和全局数据。
*/
/************************************************************************/

#pragma once
#include "stdafx.h"
#include <commctrl.h>

#define PLUG_FILENAME L"TdxPlugin.dll"

#define Stock_ID "股票" //FeatureID "group.stock" HomeFeatureID "" Title "股票">
#define Stock_Buy_ID "Stock.Buy" //FeatureID "Stock.Buy" Title "买入" Image "1"/>
#define Stock_Sell_ID "Stock.Sell" //FeatureID "Stock.Sell" Title "卖出" Image "2"/>
#define Stock_marketBuy_ID "Stock.marketBuy" //FeatureID "Stock.marketBuy" Title "市价买入" Image "1"/>
#define Stock_marketsell_ID "Stock.marketsell" //FeatureID "Stock.marketsell" Title "市价卖出" Image "2"/>
#define Stock_SingleBuylots_ID "Stock.SingleBuylots" //FeatureID "Stock.SingleBuylots" Title "单帐户批量买入" Image "1"/>
#define Stock_SingleSellLots_ID "Stock.SingleSellLots" //FeatureID "Stock.SingleSellLots" Title "单帐户批量卖出" Image "2"/>
#define query_ID "query" //FeatureID "AlwaysOn" Title "查询" Image "5">
#define Stock_zjgf_ID "Stock.zjgf" //FeatureID "Stock.zjgf" Title "资金股份"/>
#define Stock_lscjcx_ID "Stock.lscjcx" //FeatureID "Stock.lscjcx" Title "历史成交"/>
#define Stock_dzd_ID "Stock.dzd" //FeatureID "Stock.dzd" Title "资金明细"/>
#define Stock_zjls_ID "Stock.zjls" //FeatureID "Stock.zjls" Title "资金明细"/>
#define Stock_cjhzcx_id "Stock.cjhzcx" //FeatureID "Stock.cjhzcx" Title "当日成交汇总查询"/>
#define Stock_wthzcx_ID "Stock.wthzcx" //FeatureID "Stock.wthzcx" Title "当日委托汇总查询"/>
#define Stock_lscjhzcx_ID "Stock.lscjhzcx" //FeatureID "Stock.lscjhzcx" Title "历史成交汇总查询"/>		

#define YXJY_ID "YXJY" //FeatureID "AlwaysOn" Title "股份报价转让">
#define Stock_yxmr_ID "Stock.yxmr" //FeatureID "Stock.yxmr" Title "意向买入"/>
#define Stock_yxmc_ID "Stock.yxmc" //FeatureID "Stock.yxmc" Title "意向卖出"/>
#define Stock_djmr_ID "Stock.djmr" //FeatureID "Stock.djmr" Title "定价买入"/>
#define Stock_djmc_ID "Stock.djmc" //FeatureID "Stock.djmc" Title "定价卖出"/>
#define Stock_cjqrmr_ID "Stock.cjqrmr" //FeatureID "Stock.cjqrmr" Title "成交确认买入"/>	
#define Stock_cjqrmc_ID "Stock.cjqrmc" //FeatureID "Stock.cjqrmc" Title "成交确认卖出"/>		
#define Stock_yxhqcx_ID "Stock.yxhqcx" //FeatureID "Stock.yxhqcx" Title "意向行情查询"/>					

//#define lof_ID "lof" FeatureID "AlwaysOn" Title "其它委托" Image "5">
#define Stock_ymd_ID "Stock.ymd" //FeatureID "Stock.ymd" Title "预埋单" Image "4"/>
#define Stock_yjwt_ID "Stock.yjwt" //FeatureID "Stock.yjwt" Title "预警委托" Image "7"/>

//导航栏功能树结构(TcOem.xml文件)
typedef struct _Nav_Tree
{
	//ID "股票" FeatureID "group.stock" HomeFeatureID "" Title "股票"
	LPTSTR ID;
	LPTSTR FeatureID;
	LPTSTR Title;
	LPTSTR HomeFeatureID;
	LPTSTR HotKeytc;
	int count;
	_Nav_Tree* Items;
}Nav_Tree, *PNav_Tree;

//TDX运行时TreeItem Param数据中对应TcOem.xml文件的数据结构
typedef struct _Tdx_TreeItemData
{
	_Tdx_TreeItemData* pID;
	_Tdx_TreeItemData* pFeatureID;
	CHAR ID[0x40];
	CHAR FeatureID[0x40];
}Tdx_TreeItemData, *PTdx_TreeItemData;

//TDX导航栏TreeItem Param的结构(TreeView_GetItemParam)
typedef struct _Tdx_TreeItemParam
{
	_Tdx_TreeItemParam* pNext;
	_Tdx_TreeItemParam* pPrev;
	DWORD unknow_1;
	_Tdx_TreeItemParam* pParent;
	DWORD unknow_2;
	DWORD unknow_3;
	PTdx_TreeItemData pData;
}Tdx_TreeItemParam, *PTdx_TreeItemParam;

//通达信导航菜单树与TreeView窗口对应
typedef struct _Tdx_Tree_HWND
{
	HWND hTreeView;
	PTdx_TreeItemParam pItem;
}Tdx_Tree_HWND, *PTdx_Tree_HWND;

//通达信导航菜单项目与DialogBox对应
typedef struct _Tdx_TreeItem_Dlg
{
	HWND hTreeView;
	HTREEITEM hItem;
	HWND hDialog;
}Tdx_TreeItem_Dlg, *PTdx_TreeItem_Dlg;

//BOOL InitNavTree();
//int GetNavTreeItemCount(PNav_Tree treeitem);

//extern Nav_Tree NavTree;  //导航栏功能树（此数据从TcOem.xml文件中获取）