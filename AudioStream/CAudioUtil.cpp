#include "CAudioUtil.h"

pjmedia_endpt* CAudioUtil::s_pMedEndpt = nullptr;
pj_caching_pool CAudioUtil::s_cachPool;
pj_pool_t* CAudioUtil::s_pPool = nullptr;
std::string CAudioUtil::s_strErr = "";
bool CAudioUtil::s_bPJLIBInit = false;
bool CAudioUtil::s_bInit = false;

#define AUDIOUTIL_INIT_FAILED_CLEAR(status) \
		if (status != PJ_SUCCESS)	\
		{ \
			char errmsg[PJ_ERR_MSG_SIZE]; \
			pj_strerror(status, errmsg, sizeof(errmsg)); \
			CAudioUtil::s_strErr = errmsg; \
			goto Clear;  \
		}

CAudioUtil::CAudioUtil()
    : m_pStream(nullptr)
    , m_pSoundPort(nullptr)
    , m_audioDir(AUDIO_DIR_ENCODING_DECODING)
{
}


CAudioUtil::~CAudioUtil()
{
}

int CAudioUtil::Init()
{
	pj_status_t status;
	if (s_bInit)
	{
		CAudioUtil::s_strErr = "repeat init";
		return -1;
	}
	// 初始化PJLIB
	status = pj_init();
	AUDIOUTIL_INIT_FAILED_CLEAR(status);
	s_bPJLIBInit = true;
	// 创建内存池工厂	
	pj_caching_pool_init(&s_cachPool, &pj_pool_factory_default_policy, 0);
	// 创建内存池
    s_pPool = pj_pool_create(&s_cachPool.factory, "CAudioUtil", 4000, 4000, nullptr);
	if (!s_pPool)
	{
		CAudioUtil::s_strErr = "pool create failed";
		goto Clear;
	}
    /* Create event manager */
    status = pjmedia_event_mgr_create(s_pPool, 0, nullptr);
    AUDIOUTIL_INIT_FAILED_CLEAR(status);

	// 创建媒体端点 开启1个工作线程用于poll io
    status = pjmedia_endpt_create(&s_cachPool.factory, nullptr, 1, &s_pMedEndpt);
	AUDIOUTIL_INIT_FAILED_CLEAR(status);
	
	// 注册所有支持的音频编解码器
    status = pjmedia_codec_register_audio_codecs(s_pMedEndpt, nullptr);
	AUDIOUTIL_INIT_FAILED_CLEAR(status);
	s_bInit = true;
	return 0;
Clear:
	Clear();
	return -1;
}

void CAudioUtil::Clear()
{
	if (s_pMedEndpt)
	{
		// 销毁媒体端口
		pjmedia_endpt_destroy(s_pMedEndpt);
        s_pMedEndpt = nullptr;
	}

    /* Destroy event manager */
    if (pjmedia_event_mgr_instance())
    {

        pjmedia_event_mgr_destroy(nullptr);
    }

	if (s_pPool)
	{        
		// 释放内存池
		pj_pool_release(s_pPool);
        s_pPool = nullptr;
	}
	if (s_bPJLIBInit)
	{
		// 销毁池工厂
		pj_caching_pool_destroy(&s_cachPool);
		// 关闭pjlib
		pj_shutdown();
		s_bPJLIBInit = false;
	}
    s_bInit = false;
}

void CAudioUtil::SetAudioDir(AUDIO_DIR dir)
{
    m_audioDir = dir;
}

pj_status_t CAudioUtil::SoundStreamCreate(unsigned short uLocalPort, /* 指定本地端口用来接收远程RTP/发送本地RTP到远程地址 */
    const char* strRemoteIP, /* 远程ip地址 */
	unsigned short uRemotePort)
{
	if (!m_pStream)
	{
		pj_status_t status;
		// 设置远端地址
		pj_sockaddr_in remoteAddr;
		pj_bzero(&remoteAddr, sizeof(remoteAddr));
        pj_str_t ip = pj_str((char*)strRemoteIP);
		pj_uint16_t port = (pj_uint16_t)uRemotePort;
		status = pj_sockaddr_in_init(&remoteAddr, &ip, port);
		if (status != PJ_SUCCESS)
		{
			CAudioUtil::s_strErr = "remote addr error";
			return status;
		}

		// 获取音频格式PCMU的编解码信息
        const pjmedia_codec_info* pCodecInfo = nullptr;
		unsigned uCount = 1;
		//目前只开启了"PCMU/8000/1" "PCMA/8000/1" "G722/16000/1"
        char codeId[] = "PCMU";
        pj_str_t strCodeID = pj_str(codeId);// 不区分大小写，支持部分匹配
		pjmedia_codec_mgr* pCodecMgr = pjmedia_endpt_get_codec_mgr(s_pMedEndpt);
		status = pjmedia_codec_mgr_find_codecs_by_id(pCodecMgr,
			&strCodeID, &uCount,
            &pCodecInfo, nullptr);
		if (status != PJ_SUCCESS)
		{
			CAudioUtil::s_strErr = "find codec info error";
			return status;
		}        

        pjmedia_dir media_dir = GetMeidaDir(m_audioDir);
        if (PJMEDIA_DIR_NONE == media_dir) {
            CAudioUtil::s_strErr = "audio dir invalid";
            return -1;
        }
		// 创建音频流
        pjmedia_stream* pStream = nullptr;
		status = CreateAudStream(s_pPool, s_pMedEndpt, pCodecInfo,
			media_dir, uLocalPort, &remoteAddr, &pStream);
		if (status != PJ_SUCCESS)
		{
			return status;
		}
		// 创建声音设备
        pjmedia_snd_port* pSoundPort = nullptr;
		status = CreateSoundPort(pStream, &pSoundPort);
		if (status != PJ_SUCCESS)
		{
			DestroyAudStream(pStream);
			return status;
		}
		// 启动音频流
		status = pjmedia_stream_start(pStream);
		if (status != PJ_SUCCESS)
		{
			DestroyAudStream(pStream);
			DestroySoundPort(pSoundPort);
			CAudioUtil::s_strErr = "start steam failed";
			return status;
		}
		m_pStream = pStream;
		m_pSoundPort = pSoundPort;
		return PJ_SUCCESS;
	}
	CAudioUtil::s_strErr = "repeat create";
	return -1;
}

void CAudioUtil::SoundStreamDestroy()
{
	if (m_pStream)
	{
		DestroySoundPort(m_pSoundPort);
        m_pSoundPort = nullptr;
		DestroyAudStream(m_pStream);
        m_pStream = nullptr;
	}
}

void CAudioUtil::SoundStreamPause(AUDIO_DIR dir)
{
	if (m_pStream)
	{
        pjmedia_stream_pause(m_pStream, GetMeidaDir(dir));
	}
}

void CAudioUtil::SoundStreamResume(AUDIO_DIR dir)
{
	if (m_pStream)
	{
        pjmedia_stream_resume(m_pStream, GetMeidaDir(dir));
	}
}

pj_status_t CAudioUtil::CreateAudStream(pj_pool_t* pPool,
	pjmedia_endpt* pMedEndpt,
	const pjmedia_codec_info* pCodecInfo,
	pjmedia_dir dir,
	pj_uint16_t uLocalPort,
	const pj_sockaddr_in* pRemAddr,
	pjmedia_stream** pStream)
{
	pjmedia_stream_info info;				//音频流信息，用于创建音频流
    pjmedia_transport* pTransport = nullptr;	// 媒体传输，用于传输音频流
	pj_status_t status;

	// 初始化音频流信息
	pj_bzero(&info, sizeof(info));
	info.type = PJMEDIA_TYPE_AUDIO;
	info.dir = dir;
    // incoming code info
	pj_memcpy(&info.fmt, pCodecInfo, sizeof(pjmedia_codec_info));
    // out pt
	info.tx_pt = pCodecInfo->pt;
    // incoming pt
	info.rx_pt = pCodecInfo->pt;
	info.ssrc = pj_rand();
	// 远程地址相关的信息保存在音频流信息中
	pj_memcpy(&info.rem_addr, pRemAddr, sizeof(pj_sockaddr_in));
	// 不用指定rtcp，默认使用rem_addr的下一个端口
	//info.rem_rtcp

	// 指定本地端口号创建媒体传输
    status = pjmedia_transport_udp_create(s_pMedEndpt, nullptr, uLocalPort,
		0, &pTransport);
	if (status != PJ_SUCCESS)
	{
		CAudioUtil::s_strErr = "create transport failed";
		return status;
	}

	// 使用音频流信息创建音频流
	status = pjmedia_stream_create(s_pMedEndpt, s_pPool, &info,
        pTransport,nullptr, pStream);

	if (status != PJ_SUCCESS) {
		pjmedia_transport_close(pTransport);
		CAudioUtil::s_strErr = "create stream failed";
		return status;
	}

    /* Start media transport */
    pjmedia_transport_media_start(pTransport, 0, 0, 0, 0);

	return PJ_SUCCESS;
}

void CAudioUtil::DestroyAudStream(pjmedia_stream* pStream)
{
	if (!pStream)
	{
		return;
	}
	pjmedia_transport* pTransport;
	pTransport = pjmedia_stream_get_transport(pStream);
	// 销毁音频流
	pjmedia_stream_destroy(pStream);
	// 关闭媒体传输
    pjmedia_transport_media_stop(pTransport);
	pjmedia_transport_close(pTransport);
}

pj_status_t CAudioUtil::CreateSoundPort(pjmedia_stream* pStream, pjmedia_snd_port** pSoundPort)
{
	if (!pStream || !pSoundPort)
	{
		return -1;
	}
	pj_status_t status;
    pjmedia_port* pStreamPort = nullptr;		// 音频流接口
	pjmedia_stream_info info;				// 音频流信息
    pjmedia_snd_port* pTmpSoundPort = nullptr;	// 声音设备端口

	status = pjmedia_stream_get_port(pStream, &pStreamPort);
	if (status != PJ_SUCCESS)
	{
		CAudioUtil::s_strErr = "null stream port";
		return -1;
	}
	status = pjmedia_stream_get_info(pStream, &info);
	if (status != PJ_SUCCESS)
	{
		CAudioUtil::s_strErr = "null stream info";
		return -1;
	}

	// 根据音频流方向创建声音设备
	if (info.dir == PJMEDIA_DIR_ENCODING_DECODING)
	{
		status = pjmedia_snd_port_create(s_pPool, -1, -1,
			PJMEDIA_PIA_SRATE(&pStreamPort->info),
			PJMEDIA_PIA_CCNT(&pStreamPort->info),
			PJMEDIA_PIA_SPF(&pStreamPort->info),
			PJMEDIA_PIA_BITS(&pStreamPort->info),
			0, &pTmpSoundPort);
	}
	else if (info.dir == PJMEDIA_DIR_ENCODING)
	{
		status = pjmedia_snd_port_create_rec(s_pPool, -1,
			PJMEDIA_PIA_SRATE(&pStreamPort->info),
			PJMEDIA_PIA_CCNT(&pStreamPort->info),
			PJMEDIA_PIA_SPF(&pStreamPort->info),
			PJMEDIA_PIA_BITS(&pStreamPort->info),
			0, &pTmpSoundPort);
	}		
	else
	{
		status = pjmedia_snd_port_create_player(s_pPool, -1,
			PJMEDIA_PIA_SRATE(&pStreamPort->info),
			PJMEDIA_PIA_CCNT(&pStreamPort->info),
			PJMEDIA_PIA_SPF(&pStreamPort->info),
			PJMEDIA_PIA_BITS(&pStreamPort->info),
			0, &pTmpSoundPort);
	}

	if (status != PJ_SUCCESS) 
	{
		CAudioUtil::s_strErr = "create sound port failed";
		return -1;
	}

	// 连接声音设备和音频流
	status = pjmedia_snd_port_connect(pTmpSoundPort, pStreamPort);
	if (status != PJ_SUCCESS)
	{
		DestroySoundPort(pTmpSoundPort);
		CAudioUtil::s_strErr = "connect stream and sound port failed";
		return -1;
	}
	*pSoundPort = pTmpSoundPort;
	return PJ_SUCCESS;
}

void CAudioUtil::DestroySoundPort(pjmedia_snd_port* pSoundPort)
{
	// 销毁声音设备端口
	if (pSoundPort) 
	{
		pjmedia_snd_port_destroy(pSoundPort);
    }
}

pjmedia_dir CAudioUtil::GetMeidaDir(AUDIO_DIR dir)
{
    pjmedia_dir media_dir = PJMEDIA_DIR_NONE;
    switch (dir) {
    case AUDIO_DIR_ENCODING:
        media_dir = PJMEDIA_DIR_ENCODING;
        break;
    case AUDIO_DIR_DECODING:
        media_dir = PJMEDIA_DIR_DECODING;
        break;
    case AUDIO_DIR_ENCODING_DECODING:
        media_dir = PJMEDIA_DIR_ENCODING_DECODING;
        break;
    default:
        media_dir = PJMEDIA_DIR_NONE;
        break;
    }
    return media_dir;
}

