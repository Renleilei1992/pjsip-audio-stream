#ifndef CAUDIOUTIL_H
#define CAUDIOUTIL_H

#include <string>

#include "pjlib.h"
#include "pjlib-util.h"
#include "pjmedia.h"
#include "pjmedia-codec.h"
#include "pjmedia/transport_srtp.h"

/**
* 音频方向
*/
typedef enum AUDIO_DIR
{
	AUDIO_DIR_ENCODING = 1,				// 只编码:只采集音频打包为RTP发送到远程地址
	AUDIO_DIR_DECODING = 2,				// 只解码:只从远程地址接收RTP并处理音频
	AUDIO_DIR_ENCODING_DECODING = 3,	// 同时编解码:同时上述两种操作
} AUDIO_DIR;

class CAudioUtil
{
public:
	CAudioUtil();
	~CAudioUtil();

	static int Init();
	static void Clear();

	static pjmedia_endpt* s_pMedEndpt;				// 媒体端点
	static pj_caching_pool s_cachPool;				// 缓存池，用于创建内存池工厂
	static pj_pool_t* s_pPool;						// 内存池，为其他对象提供内存
	static std::string s_strErr;					// 错误信息
	static bool s_bPJLIBInit;						// 是否成功初始化PJLIB
	static bool s_bInit;							// 是否成功初始化

public:
    void SetAudioDir(AUDIO_DIR dir);
	// 创建音频设备流
    pj_status_t SoundStreamCreate(unsigned short uLocalPort,					// 指定本地端口用来接收远程RTP/发送本地RTP到远程地址
        const char* strRemoteIP,					// 远程ip地址
		unsigned short uRemotePort);				
	// 销毁音频设备流
	void SoundStreamDestroy();
	// 暂停音频设备流
	void SoundStreamPause(AUDIO_DIR dir);
	// 恢复音频设备流
	void SoundStreamResume(AUDIO_DIR dir);

private:
	// 创建媒体传输和音频流
	pj_status_t CreateAudStream(pj_pool_t* pPool,	// 内存池
		pjmedia_endpt* pMedEndpt,					// 媒体端口
		const pjmedia_codec_info* pCodecInfo,		// 音频编解码信息
		pjmedia_dir dir,							// 编解码描述，控制音频流只编码/只解码/同时编解码
		pj_uint16_t uLocalPort,						// 本地传输端口
		const pj_sockaddr_in* pRemAddr,				// 远程地址
		pjmedia_stream** pStream);					// 成功创建的音频流
	// 销毁媒体传输和音频流
	void DestroyAudStream(pjmedia_stream* pStream);

	// 创建声音设备端口,并连接到音频流
	pj_status_t CreateSoundPort(pjmedia_stream* pStream, pjmedia_snd_port** pSoundPort);
	// 销毁声音设备端口
	void DestroySoundPort(pjmedia_snd_port* pSoundPort);

    pjmedia_dir GetMeidaDir(AUDIO_DIR dir);

private:
	// 音频流：借助媒体传输从远端接收音频rtp并解码/编码音频数据并发送音频rtp到远端	
	pjmedia_stream* m_pStream;		
	// 声音设备端口：用来从麦克采集音频/用喇叭播放音频
	pjmedia_snd_port* m_pSoundPort;

    // 媒体参数：

    // 音频方向
    AUDIO_DIR m_audioDir;
};
#endif // CAUDIOUTIL_H
