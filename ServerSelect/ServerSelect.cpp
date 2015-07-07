#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <winsock2.h>
#include <vector>
#include "LdSocket.h"

class CSocketListener: public ISocketListener
{


	virtual void OnConnected(PLD_CLIENT_SOCKET pClient)
	{
		printf("OnConnected Socket:%d\n", pClient->m_Socket);
	}

	virtual void OnRecv(PLD_CLIENT_SOCKET pClient)
	{
		printf("Recv: Socket:%d %s\n", pClient->m_Socket, pClient->lpRecvedBuffer);
	}

	virtual void OnClosed(PLD_CLIENT_SOCKET pClient)
	{
		printf("OnClosed Socket:%d\n", pClient->m_Socket);
	}

	virtual void OnAccept(PLD_CLIENT_SOCKET pClient)
	{
		printf("OnAccept Socket:%d\n", pClient->m_Socket);
	}

	virtual void OnError(PLD_CLIENT_SOCKET pClient, int nErr)
	{
		printf("OnError Socket:%d error code=%d\n", pClient->m_Socket, nErr);
	}

};

int main(int argc, char *argv[])
{
	CLdSocket svr;
	svr.SetListener(new CSocketListener());

	if(argc==1){
		svr.Listen(0x1345);
	}else{
		svr.ConnectTo(argv[1], 0x1345);
	}

	while(TRUE){
		char buffer[81] = {0};
		int i, ch;

		for (i = 0; (i < 80) && ((ch = getchar()) != EOF) && (ch != '\n'); i++){
			buffer[i] = (char) ch;
		}
		svr.Send(buffer, strlen(buffer));
		svr.Close();
	}

	return 0;
}

