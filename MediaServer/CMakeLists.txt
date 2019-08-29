cmake_minimum_required(VERSION 2.6)

project (MEDIASERVER)

ADD_DEFINITIONS( -g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

set(network_srcs
network/BaseSocket.cpp
network/EventDispatch.cpp
network/Lock.cpp
network/netlib.cpp
network/SimpleBuffer.cpp
network/TcpClient.cpp
network/Tcpconn.cpp
network/TcpServer.cpp
network/util.cpp
)

set(mediaserver_srcs
main.cpp
)

add_executable(mediaserver ${network_srcs} ${mediaserver_srcs})
#光包含库目录是没用的，还必须使用TARGET_LINK_LIBRARIES链接该库
TARGET_LINK_LIBRARIES(mediaserver pthread)






