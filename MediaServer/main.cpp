#include "CPortManage.h"
#include "CUdpServer.h"
#include <stdio.h>

int main(int argc, char* argv[])
{	
	char* ip = "0.0.0.0";
	if (!g_portManage.InitPorts(4000, 4012))
	{
		printf("init ports failed\n");
		return false;
	}
	printf("init ports success\n");
	CUdpServer srv(ip);
	// 测试
	int nID1 = srv.CreateSession();
	if (nID1 == -1)
	{
		printf("CreateSession failed\n");
		return false;
	}
	printf("loop...\n");
	srv.Loop();
	return 0;
}