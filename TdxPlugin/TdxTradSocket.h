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
private:
	BOOL m_bAvailable;
};

