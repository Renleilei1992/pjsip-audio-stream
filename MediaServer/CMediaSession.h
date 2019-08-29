#pragma once
#include "CPortManage.h"
#include <vector>
#define MS_USE_RTCP			// 是否使用rtcp(TODO)
#define MS_AUTO_JOIN		// 自动加入会话
typedef struct MS_CLIENT_DATA
{
	sockaddr_in addr;
	bool bVaild;
}MS_CLIENT_DATA;
// TODO锁保护
class CMediaSession
{
public:
	CMediaSession();
	~CMediaSession();

	bool InitSession(const char* pszIP);
	void CloseSession();
	int GetRTPfd();
	void Add(int fd, sockaddr_in addr);
	void Remove(int fd, sockaddr_in addr);
	void OnRecvRTP(int fd, sockaddr_in& addr, char* cRtpData, size_t szDataSize);
	// 定时移除无效客户端，非线程安全
	void TimerRemoveInvaildClient();
	int GetClientNum();
private:
	RTP_PORTS* m_pPort;
	int m_fdRTP;
#ifdef MS_USE_RTCP
	int m_fdRTCP;
#endif
	// 存储当前媒体会话中所有的客户数据
	std::vector<MS_CLIENT_DATA> m_vClientData;
};

