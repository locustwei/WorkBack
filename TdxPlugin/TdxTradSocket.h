#pragma once

#include "..\PublicLib\socket\LdSocket.h"
#include "TDXSocketData.h"

#define MM_TDXSOCKET WM_USER + 0x235

class CTdxTradSocket : public CLdSocket
{
	friend class CTradSocketListenner;
public:
	CTdxTradSocket(void);
	~CTdxTradSocket(void);
	void ConnectCtrlor();
	void SendStockByResult(DWORD htid);;
	void SendStockSellResult(DWORD htid);
	void SendStockZjgfResult(PTDX_STOCK_ZJGF pZjgf, int nSize);
private:
	BOOL m_bAvailable;
	PLD_CLIENT_SOCKET GetActiveSocket();
};

