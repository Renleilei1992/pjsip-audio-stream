#include "CMediaSession.h"
#include <unistd.h> // close
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

CMediaSession::CMediaSession()
	: m_pPort(NULL)
	, m_fdRTP(-1)
#ifdef MS_USE_RTCP
	, m_fdRTCP(-1)
#endif	
{
}

CMediaSession::~CMediaSession()
{
}

bool CMediaSession::InitSession(const char* pszIP)
{
	m_pPort = g_portManage.GetPort();
	if (!m_pPort)
	{
		return false;
	}
	int nRet;
	sockaddr_in srvAddr;
	// 创建socket
	m_fdRTP = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_fdRTP < 0)
	{
		return false;
	}	
	// 设置socket属性为非阻塞
	int old_opt = fcntl(m_fdRTP, F_GETFL);
	int new_opt = old_opt | O_NONBLOCK;
	fcntl(m_fdRTP, F_SETFL, new_opt);
	//bind
	srvAddr.sin_family = AF_INET;
	inet_aton(pszIP, &srvAddr.sin_addr);
	srvAddr.sin_port = htons(m_pPort->uRTPPort);
	nRet = bind(m_fdRTP, (sockaddr*)&srvAddr, sizeof(srvAddr));
	if (nRet < 0)
	{
		int n = errno;
		close(m_fdRTP);
		return false;
	}
#ifdef MS_USE_RTCP
	m_fdRTCP = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_fdRTCP < 0)
	{
		close(m_fdRTP);
		return false;
	}
	old_opt = fcntl(m_fdRTCP, F_GETFL);
	new_opt = old_opt | O_NONBLOCK;
	fcntl(m_fdRTCP, F_SETFL, new_opt);
	//bind
	srvAddr.sin_port = htons(m_pPort->uRTCPPort);
	nRet = bind(m_fdRTCP, (sockaddr*)&srvAddr, sizeof(srvAddr));
	if (nRet < 0)
	{
		close(m_fdRTP);
		close(m_fdRTCP);
		return false;
	}
#endif
	return true;
}

void CMediaSession::CloseSession()
{
	g_portManage.ReleasePort(m_pPort);
	m_vClientData.clear();
	close(m_fdRTP);
	close(m_fdRTCP);
}

int CMediaSession::GetRTPfd()
{
	return m_fdRTP;
}

void CMediaSession::Add(int fd, sockaddr_in addr)
{
	if (fd == m_fdRTP)
	{
		MS_CLIENT_DATA data;
		data.addr = addr;
		data.bVaild = true;
		m_vClientData.push_back(data);
	}
#ifdef MS_USE_RTCP
	else if (fd == m_fdRTCP)
	{
	}
#endif // MS_USE_RTCP
}

void CMediaSession::Remove(int fd, sockaddr_in addr)
{
	if (fd == m_fdRTP)
	{
		std::vector <MS_CLIENT_DATA>::iterator iter = m_vClientData.begin();
		for (; iter != m_vClientData.end(); ++iter)
		{
			if (0 == memcmp(&addr, &(iter->addr), sizeof(addr)))
			{
				m_vClientData.erase(iter);
				break;
			}
		}
	}
#ifdef MS_USE_RTCP
	else if (fd == m_fdRTCP)
	{
	}
#endif // MS_USE_RTCP
}

void CMediaSession::OnRecvRTP(int fd, sockaddr_in& addr, char* cRtpData, size_t szDataSize)
{
	int nSize = m_vClientData.size();
	if (fd == m_fdRTP)
	{
		bool bExit = false;
		// 发送给会话中其他客户端
		for (int i = 0; i < nSize; i++)
		{
			if (0 == memcmp(&addr, &m_vClientData[i].addr, sizeof(addr)))
			{
				m_vClientData[i].bVaild = true;
				bExit = true;
			}
			else
			{
				sendto(m_fdRTP, cRtpData, szDataSize, 0, 
					(struct sockaddr*)&m_vClientData[i].addr, sizeof(m_vClientData[i].addr));
			}
		}
#ifdef MS_AUTO_JOIN
		if (!bExit)
		{
			Add(m_fdRTP, addr);
		}
#endif
	}
#ifdef MS_USE_RTCP
	else if (fd == m_fdRTCP)
	{
	}
#endif // MS_USE_RTCP
}

void CMediaSession::TimerRemoveInvaildClient()
{
	std::vector <MS_CLIENT_DATA>::iterator iter = m_vClientData.begin();
	while (iter != m_vClientData.end())
	{
		if (iter->bVaild)
		{
			iter->bVaild = false;
			++iter;
		}
		else // 定时周期内没有收到数据，则删除
		{
			iter = m_vClientData.erase(iter);
		}
	}
}

int CMediaSession::GetClientNum()
{
	return m_vClientData.size();
}

