#include "CUdpServer.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

CUdpServer::CUdpServer(char* pszIP)
	: m_bRuning(false)
	, m_strIP(pszIP)
{
	m_epfd = epoll_create(1024);
	// 创建管道
	assert(0 == pipe(m_pipefd));
	// 注册读事件
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = NULL;
	epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_pipefd[0], &ev);
}


CUdpServer::~CUdpServer()
{
	std::vector<CMediaSession*>::iterator it;
	for (it = m_vpMediaSessions.begin(); it != m_vpMediaSessions.end();)
	{
		RemoveReadEvent(*it);
		(*it)->CloseSession();
		delete (*it);
		it = m_vpMediaSessions.erase(it);
	}
	close(m_pipefd[0]);
	close(m_pipefd[1]);
	if (-1 != m_epfd)
	{
		close(m_epfd);
		m_epfd = -1;
	}
}

int CUdpServer::CreateSession()
{
	CMediaSession* pms = new CMediaSession;
	if (!pms->InitSession(m_strIP.c_str()))
	{
		delete pms;
		return -1;
	}
	m_vpMediaSessions.push_back(pms);
	AddReadEvent(pms);
	return m_vpMediaSessions.size() - 1;
}

void CUdpServer::DestroySession(int nSessionID)
{
	if (nSessionID < m_vpMediaSessions.size())
	{
		std::vector<CMediaSession*>::iterator it = m_vpMediaSessions.begin() + nSessionID;
		RemoveReadEvent(*it);
		(*it)->CloseSession();
		delete (*it);
		m_vpMediaSessions.erase(it);
	}
}

bool CUdpServer::AddReadEvent(CMediaSession* pms)
{
	if (-1 == m_epfd)
	{
		return false;
	}
	int fd = pms->GetRTPfd();
	bool bRet = false;
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.ptr = pms;
	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		bRet = true;
	}
	return bRet;
}

bool CUdpServer::RemoveReadEvent(CMediaSession* pms)
{
	if (-1 == m_epfd)
	{
		return false;
	}
	int fd = pms->GetRTPfd();
	epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
	return true;
}

void CUdpServer::TimerEvent()
{
	std::vector<CMediaSession*>::iterator it;
	for (it = m_vpMediaSessions.begin(); it != m_vpMediaSessions.end();)
	{
		// 移除超时客户端
		(*it)->TimerRemoveInvaildClient();
		if (0 == (*it)->GetClientNum())
		{
#ifdef US_TIMEOUT_DELETE_SESSION
			// 自动删除空会话
			RemoveReadEvent(*it);
			(*it)->CloseSession();
			delete (*it);
			it = m_vpMediaSessions.erase(it);
#endif // US_TIMEOUT_DELETE_SESSION
		}
		else
		{
			++it;
		}
	}
}

void CUdpServer::Loop()
{
	struct epoll_event events[1024];
	int nfds = 0;

	if (m_bRuning)
		return;
	m_bRuning = true;
	int fdRTP;
	sockaddr_in addr;
	socklen_t slLen;
	size_t szRecv;

	int timeout = US_TIMEOUT;
	time_t start;
	time_t end;
	while (m_bRuning)
	{
		start = time(NULL);
		nfds = epoll_wait(m_epfd, events, 1024, timeout);
		if ((nfds < 0) && (errno != EINTR))
		{
			break;
		}
		// 超时时间到，处理定时任务，重置定时器
		if (0 == nfds)
		{
			// 处理定时任务
			TimerEvent();
			// 重置定时器
			timeout = US_TIMEOUT;
			continue;
		}
		// 超时时间未到，检查剩余睡眠时间
		end = time(NULL);
		timeout -= (end - start) * 1000;
		// 剩余睡眠时间<=0说明即超时又有可读事件触发
		if (timeout <= 0)
		{
			// 处理定时任务
			TimerEvent();
			// 重置定时器
			timeout = US_TIMEOUT;
		}
		for (int i = 0; i < nfds; i++)
		{
			CMediaSession* pms = (CMediaSession*)events[i].data.ptr;
			if (NULL == pms)
			{
				// 唤醒pipe没有pms
				continue;
			}
			if (events[i].events & EPOLLIN)
			{
				// TODO 区分fdrtp和fdrtcp
				fdRTP = pms->GetRTPfd();
				slLen = sizeof(addr);
				szRecv = recvfrom(fdRTP, m_buffer, RTP_BUFFER_SIZE, 0,
					(sockaddr*)&addr, &slLen);
				if (szRecv > 0)
				{
					pms->OnRecvRTP(fdRTP, addr, m_buffer, szRecv);
				}
			}
		}
	}
}

void CUdpServer::StopLoop()
{
	m_bRuning = false;
	// pipe用读写read write，不用send recv
	write(m_pipefd[1], "1", 1);
}
