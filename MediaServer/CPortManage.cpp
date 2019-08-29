#include "CPortManage.h"
#include <cstddef>

CPortManage g_portManage;
CPortManage::CPortManage()
{
}


CPortManage::~CPortManage()
{
	m_vPorts.clear();
}

bool CPortManage::InitPorts(in_port_t uStartPort, in_port_t uEndPort)
{
	// 保证端口偶数开始，至少2个端口
	if (uEndPort - uStartPort < 2 || uStartPort % 2 != 0 || uEndPort % 2 != 0)
	{
		return false;
	}
	PM_DATA data;	
	for (in_port_t i = uStartPort; i < uEndPort;)
	{
		data.rtpPorts.uRTPPort = i;
		data.rtpPorts.uRTCPPort = i + 1;
		data.bUse = false;
		m_vPorts.push_back(data);
		i += 2;
	}
	return true;
}

RTP_PORTS* CPortManage::GetPort()
{
	RTP_PORTS* pPort = NULL;
	size_t szLen = m_vPorts.size();
	for (size_t i = 0; i < szLen; i++) {
		if (!m_vPorts[i].bUse)
		{
			pPort = &m_vPorts[i].rtpPorts;
			m_vPorts[i].bUse = true;
			break;
		}
	}
	return pPort;
}

void CPortManage::ReleasePort(RTP_PORTS* pPort)
{
	if(pPort)
	{ 
		((PM_DATA*)pPort)->bUse = false;
	}	
}
