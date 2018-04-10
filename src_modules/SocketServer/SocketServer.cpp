/*
 * =====================================================================================
 *
 *       Filename:  SocketServer.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Wednesday 11 April 2018 01:25:58  IST
 *       Revision:  1.1
 *       Compiler:  g++
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include "SocketServer.h"
#include <cstring>
#include <cstdio>
extern "C" {
#include <sys/poll.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
};

using namespace server;

SocketServer::SocketServer(bool aNetLinkPidAsCurrentPid, int aNetlinkGroups /* = -1*/,
        int aCommType /* = SOCK_DGRAM*/, int aProtocol /* = NETLINK_KOBJECT_UEVENT */)
{
    struct sockaddr_nl *sa = NULL;

    mSocketFd = -1;
    mSocketType = AF_NETLINK;
    sa = new char[sizeof(struct sockaddr_nl)];
    mSockAddr = sa;
    mAddrLen = sizeof(struct sockaddr_nl);
    mCommType = aCommType;
    mProtocol = aProtocol;
    memset(sa, 0, mAddrLen);
    sa->nl_family = mSocketType;
    sa->nl_groups = aNetlinkGroups;
    sa->nl_pid = 0;
    if (aNetLinkPidAsCurrentPid) sa->nl_pid = getpid();
}

SocketServer::SocketServer(unsigned short aListeningNwPort, int aCommType /* = SOCK_STREAM*/, int aProtocol /* = IPPROTO_IP*/)
{
    if (aProtocol == IPPROTO_IP) {
        struct sockaddr_in *sa = NULL;
        mSocketFd = -1;
        mSocketType = AF_INET;
        sa = new char[sizeof(struct sockaddr_in)];
        mSockAddr = sa;
        mAddrLen = sizeof(struct sockaddr_in);
        mCommType = aCommType;
        mProtocol = aProtocol;
        memset(sa, 0, mAddrLen);
        sa->sin_family = mSocketType;
        sa->sin_addr.s_addr = htonl(INADDR_ANY);
        sa->sin_port = htons(aListeningNwPort);
    }
    else {
        struct sockaddr_in6 *sa = NULL;
        mSocketFd = -1;
        mSocketType = AF_INET6;
        sa = new char[sizeof(struct sockaddr_in6)];
        mSockAddr = sa;
        mAddrLen = sizeof(struct sockaddr_in6);
        mCommType = aCommType;
        mProtocol = aProtocol;
        memset(sa, 0, mAddrLen);
        sa->sin6_family = mSocketType;
        sa->sin6_addr = in6addr_any;
        sa->sin6_port = htons(aListeningNwPort);
    }
}

SocketServer::SocketServer(const char *aLocalSocketName, int aCommType /* = SOCK_STREAM*/, bool aAbstract /* = true*/)
{
    struct sockaddr_un *sa = NULL;
    mSocketFd = -1;
    mSocketType = AF_UNIX;
    sa = new char[sizeof(struct sockaddr_un)];
    mSockAddr = sa;
    mAddrLen = sizeof(struct sockaddr_un);
    mCommType = aCommType;
    mProtocol = 0;
    memset(sa, 0, mAddrLen);
    sa->sun_family = mSocketType;
    if (aAbstract) {
        strncpy(&(sa->sun_path[1]), aLocalSocketName, sizeof(sa->sun_path) - 2);
    }
    else {
        strncpy(sa->sun_path, aLocalSocketName, sizeof(sa->sun_path) - 1);
    }
}

SocketServer::~SocketServer(void)
{
    delete mSockAddr;
}


int SocketServer::start(void)
{
    mSocketFd = socket(mSocketType, mCommType, mProtocol);
    if (mSocketFd < 0) {
    }

    if (bind(mSocketFd, (struct sockaddr *) mSockAddr, mAddrLen) < 0) {
    }
}
int SocketServer::resume(void);
int SocketServer::pause(void);
int SocketServer::stop(void);
static void *SocketServer::ServiceThread(void *arg);
