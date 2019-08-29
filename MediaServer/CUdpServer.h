#pragma once
#include <string>
#include <vector>
#include "CMediaSession.h"
#define RTP_BUFFER_SIZE 1500
#define	US_TIMEOUT		10000		// 超时没有rtp数据则移除，单位ms
//#define US_TIMEOUT_DELETE_SESSION	// 超时自动删除空会话
// TODO锁保护

class CUdpServer
{
public:
	CUdpServer(char* pszIP);
	~CUdpServer();
	int CreateSession();

	void Loop();
	void StopLoop();
private:
	// 会话采用30s自动删除,此接口不暴露（或者改为异步安全）
	void DestroySession(int nSessionID);
	bool AddReadEvent(CMediaSession* pms);
	bool RemoveReadEvent(CMediaSession* pms);
	void TimerEvent();
	int	m_epfd;
	bool m_bRuning;
	std::string m_strIP;
	std::vector<CMediaSession*> m_vpMediaSessions;
	char m_buffer[RTP_BUFFER_SIZE];
	// 用于唤醒epoll_wait
	int m_pipefd[2];
};

