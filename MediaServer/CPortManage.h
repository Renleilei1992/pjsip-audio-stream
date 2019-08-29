#pragma once
#include <netinet/in.h>
#include <vector>
/*
一路rtp流需要两个端口号，rtp/rtcp
一般为连续的两个端口号，小偶数rtp，大奇数rtcp
*/
typedef struct RTP_PORTS
{
	in_port_t uRTPPort;
	in_port_t uRTCPPort;
}RTP_PORTS;

typedef struct PM_DATA
{
	RTP_PORTS rtpPorts;		// RTP_PORTS在第一位，保证可以RTP_PORTS*可以强转为PM_DATA*
	bool bUse;
}PM_DATA;

class CPortManage
{
public:
	CPortManage();
	~CPortManage();
	// 只会使用[start_port, end_port)范围内的端口
	bool InitPorts(in_port_t uStartPort, in_port_t uEndPort);
	RTP_PORTS* GetPort();
	void ReleasePort(RTP_PORTS* pPort);
private:
	std::vector<PM_DATA> m_vPorts;
};

extern CPortManage g_portManage;
