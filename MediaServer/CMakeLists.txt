cmake_minimum_required(VERSION 2.6)

project (MEDIASERVER)

ADD_DEFINITIONS( -g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

set(mediaserver_srcs
CPortManage.cpp
CMediaSession.cpp
CUdpServer.cpp
main.cpp
)

add_executable(mediaserver ${mediaserver_srcs})

TARGET_LINK_LIBRARIES(mediaserver pthread)






