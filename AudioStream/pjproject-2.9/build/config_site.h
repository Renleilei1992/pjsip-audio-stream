

//取消注释以获得最佳性能
#define PJ_CONFIG_MAXIMUM_SPEED
#include "config_site_sample.h"

// 设置音频编解码
#define PJMEDIA_HAS_L16_CODEC		0
#define PJMEDIA_HAS_ILBC_CODEC		0
#define PJMEDIA_HAS_GSM_CODEC		0
#define PJMEDIA_HAS_SPEEX_CODEC		0
#define PJMEDIA_HAS_G722_CODEC		1

// 取消srtp
#define PJMEDIA_HAS_SRTP			0
