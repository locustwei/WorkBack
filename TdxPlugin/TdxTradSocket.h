#pragma once

#include "..\PublicLib\socket\LdSocket.h"
#include "TDXSocketData.h"

#define MM_TDXSOCKET WM_USER + 0x367

class CTdxTradSocket : public CLdSocket
{
	friend class CTradSocketListenner;
public:
	CTdxTradSocket(void);
	~CTdxTradSocket(void);
	void ConnectCtrlor();
private:
	BOOL m_bAvailable;
	PLD_CLIENT_SOCKET GetActiveSocket();
	BOOL SendStockData(TDX_TRAD_FUN fID, LPVOID pData, int nSize);
};

