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
    //#include <sys/poll.h>
#include <linux/types.h>
    //#include <sys/un.h>
    //#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
};

#define PRINT_ERROR(tag)   printf("[SocketServer::%s] : " #tag " ERROR : %s\n", __func__, strerror(errno))
#define PRINT_ERROR_MESSAGE(tag, msg)   printf("[SocketServer::%s] : " #tag " ERROR : %s\n", __func__, msg)
#define PRINT_INFO(format, arg...)      printf("[SocketServer] : " format "\n", ##arg)
using namespace server;

SocketServer::SockInfo::SockInfo(void) : event(SOCKET_DATA_BUFFERED),
    threadJoined(true), threadRunning(false) {
        sockServer = NULL;
        pthread_mutex_init(&threadLock, NULL);
    }

SocketServer::SockInfo::~SockInfo(void)
{
    void *threadstatus;
    bool runningstatus = false;
    pthread_mutex_lock(&threadLock);
    runningstatus = threadRunning;
    pthread_mutex_unlock(&threadLock);
    if (runningstatus) {
        pthread_cancel(workerThread);
    }
    if (!threadJoined) {
        pthread_join(workerThread, &threadstatus);
    }
    pthread_mutex_destroy(&threadLock);
}

void SocketServer::SockInfo::raiseEvent(SocketServer *aSocketServer, SocketEvent aEvent) {
    void *threadstatus;
    bool runningstatus = false;
    int threadJoinedFlag = true;
    pthread_mutex_lock(&threadLock);
    runningstatus = threadRunning;
    //fd = aFileDesc;
    event = aEvent;
    threadJoinedFlag = threadJoined;
    sockServer = aSocketServer;
    pthread_mutex_unlock(&threadLock);

    if (!runningstatus) {
        if (!threadJoinedFlag) {
            pthread_join(workerThread, &threadstatus);
        }
        threadJoined = true;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        threadRunning = true;
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        if (pthread_create(&workerThread, &attr, eventThread, this) < 0) {
            PRINT_ERROR_MESSAGE(pthread_create,"Thread Creation failed!!");
            threadRunning = false;
        }
        pthread_attr_destroy(&attr);
    }
}

void SocketServer::SockInfo::AcquireLock(void)
{
    pthread_mutex_lock(&threadLock);
}

void SocketServer::SockInfo::ReleaseLock(void)
{
    pthread_mutex_unlock(&threadLock);
}

void SocketServer::SockInfo::handleEvent(void)
{
    if (sockServer)
        sockServer->eventHandler(event,*this);
}

void *SocketServer::SockInfo::eventThread(void *arg)
{
    SockInfo *sInfo = (SockInfo *)arg;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_lock(&sInfo->threadLock);
    sInfo->threadJoined = false;
    pthread_mutex_unlock(&sInfo->threadLock);
    sInfo->handleEvent();
    pthread_mutex_lock(&sInfo->threadLock);
    sInfo->threadRunning = false;
    pthread_mutex_unlock(&sInfo->threadLock);
    pthread_exit(NULL);
}


SocketServer::SocketServer(bool aNetLinkPidAsCurrentPid, int aNetlinkGroups /* = -1*/,
        int aCommType /* = SOCK_DGRAM*/, int aProtocol /* = NETLINK_KOBJECT_UEVENT */)
{
    mRunning = false;
    mThreadJoined = true;
    if (sem_init(&mSem, 0, 1) < -1) PRINT_ERROR(sem_init);
    pthread_mutex_init(&mThreadMutex, NULL);

    struct sockaddr_nl *sa = NULL;
    mSocketFd = -1;
    mSocketType = AF_NETLINK;
    sa = (struct sockaddr_nl*)(new char[sizeof(struct sockaddr_nl)]);
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
    mRunning = false;
    mThreadJoined = true;
    if (sem_init(&mSem, 0, 1) < -1) PRINT_ERROR(sem_init);
    pthread_mutex_init(&mThreadMutex, NULL);

    if (aProtocol == IPPROTO_IP) {
        struct sockaddr_in *sa = NULL;
        mSocketFd = -1;
        mSocketType = AF_INET;
        sa = (struct sockaddr_in*)(new char[sizeof(struct sockaddr_in)]);
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
        sa = (struct sockaddr_in6*)(new char[sizeof(struct sockaddr_in6)]);
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
    mRunning = false;
    mThreadJoined = true;
    if (sem_init(&mSem, 0, 1) < -1) PRINT_ERROR(sem_init);
    pthread_mutex_init(&mThreadMutex, NULL);

    struct sockaddr_un *sa = NULL;
    mSocketFd = -1;
    mSocketType = AF_UNIX;
    sa = (struct sockaddr_un*)(new char[sizeof(struct sockaddr_un)]);
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
    if (isRunning()) stop();
    char *tPtr = (char *)mSockAddr;
    delete tPtr;
    mSockAddr = NULL;
    pthread_mutex_destroy(&mThreadMutex);
}


int SocketServer::start(void)
{
    if (!mThreadJoined) stop();
    mSocketFd = socket(mSocketType, mCommType, mProtocol);
    if (mSocketFd < 0) {
        PRINT_ERROR(socket);
        return -errno;
    }

    if ((mSocketType == AF_INET) || (mSocketType == AF_INET6)) {
        int flag = 1;
        if (setsockopt(mSocketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
            PRINT_ERROR(setsockopt);
            close(mSocketFd);
            mSocketFd = -1;
            return -errno;
        }
    }

    if (bind(mSocketFd, (struct sockaddr *) mSockAddr, mAddrLen) < 0) {
        PRINT_ERROR(bind);
        close(mSocketFd);
        mSocketFd = -1;
        return -errno;
    }

    if (mSocketType != AF_NETLINK) {
        if (listen(mSocketFd, CLIENT_QUEUE_LEN) < 0) {
            PRINT_ERROR(listen);
            close(mSocketFd);
            mSocketFd = -1;
            return -errno;
        }
    }

    mThreadJoined = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    mRunning = true;
    if (pthread_create(&mListeningThread, &attr, serviceThread, this) < 0) {
        PRINT_ERROR_MESSAGE(pthread_create,"Thread Creation failed!!");
        close(mSocketFd);
        mSocketFd = -1;
        mRunning = false;
    }
    else {
        SockInfo &si = mFdList[mSocketFd];
        memcpy(&(si.sockAddr), mSockAddr, mAddrLen);
        si.fd = mSocketFd;
        si.rxDataLen = 0;
        if ((mSocketType == AF_INET)
                || (mSocketType == AF_INET6)) gethostname(si.name, sizeof(si.name));
        si.raiseEvent(this, SocketServer::SOCKET_STARTED);

        printf("SockAddr : \n");
        write(1,&si.sockAddr,sizeof(struct sockaddr_un));
        printf("\n\n");
    }

    pthread_attr_destroy(&attr);

    return -1*(!(mSocketFd > 0));
}

void SocketServer::resume(void)
{
    int semVal = 0;
    pthread_mutex_lock(&mThreadMutex);
    sem_getvalue(&mSem, &semVal);
    if (semVal <= 0) {
        if (sem_post(&mSem) < 0) PRINT_ERROR(sem_post);
    }
    pthread_mutex_unlock(&mThreadMutex);
}

void SocketServer::pause(void)
{
    int semVal = 0;
    pthread_mutex_lock(&mThreadMutex);
    sem_getvalue(&mSem, &semVal);
    pthread_mutex_unlock(&mThreadMutex);
    if (semVal > 0) {
        if (sem_wait(&mSem) < 0)PRINT_ERROR(sem_post);
    }
}

void SocketServer::stop(void)
{
    void *status;
    pthread_mutex_lock(&mThreadMutex);
    mRunning = false;
    pthread_mutex_unlock(&mThreadMutex);
    resume();

    if(!mThreadJoined) pthread_join(mListeningThread, &status);

    SockInfo &si = mFdList[mSocketFd];
    si.raiseEvent(this, SocketServer::SOCKET_STOPPED);
    /*for (std::map<int,SockInfo>::iterator it=mFdList.begin();
                                it!=mFdList.end(); ++it) {
        if ((int)it->first != mSocketFd) close((int)it->first);
    } */
    pthread_mutex_lock(&mThreadMutex);
    /* clear the socket set */
    FD_ZERO(&mReadfds);
    mFdList.clear();
    mThreadJoined = true;
    close(mSocketFd);
    mSocketFd = -1;
    pthread_mutex_unlock(&mThreadMutex);
}

int SocketServer::getFd(void) {
    return mSocketFd;
}

bool SocketServer::isRunning(void)
{
    bool ret = false;
    bool isServiceThread = false;

    pthread_mutex_lock(&mThreadMutex);
    ret = mRunning;
    if (ret) isServiceThread = !!pthread_equal(mListeningThread, pthread_self());
    pthread_mutex_unlock(&mThreadMutex);

    if (isServiceThread && catchPauseAndResumeEvents()) {
        pthread_mutex_lock(&mThreadMutex);
        ret = mRunning;
        pthread_mutex_unlock(&mThreadMutex);
    }

    return ret;
}

bool SocketServer::isPaused(void)
{
    int semVal = 0;
    pthread_mutex_lock(&mThreadMutex);
    sem_getvalue(&mSem, &semVal);
    pthread_mutex_unlock(&mThreadMutex);
    return (semVal <= 0);
}

void *SocketServer::serviceThread(void *arg) {
    printf("SocketServer::serviceThread just entered...\n");
    SocketServer *sockServer = (SocketServer *)arg;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_mutex_lock(&sockServer->mThreadMutex);
    sockServer->mThreadJoined = false;
    pthread_mutex_unlock(&sockServer->mThreadMutex);
    printf("SocketServer::serviceThread started...\n");
    /* clear the socket set */
    FD_ZERO(&sockServer->mReadfds);

    /* add master socket to set */
    FD_SET(sockServer->mSocketFd, &sockServer->mReadfds);

    int maxSockFd = sockServer->mSocketFd;
    int activity = -1;
    int ret = 0;
    struct timeval tv = {0, 10000L /* 10ms */};
    printf("maxSockFd: %d\n", maxSockFd);
    while(sockServer->isRunning()) {
        /* wait for an activity on one of the sockets , timeout is 10ms , so wait indefinitely */
     //   printf("wait for event\n");
        activity = select(maxSockFd + 1 , &sockServer->mReadfds , NULL , NULL , &tv);
        if ((activity < 0) && (errno!=EINTR))
        {
            PRINT_ERROR(select);
        }

        if (activity <= 0) {
     //       printf("Wait for event timed out!!\n");
            continue;
        }
        else {
            printf("Some thing happend\n");
        }

        for ( const auto &fdRecord : sockServer->mFdList ) {
            if (FD_ISSET((int)fdRecord.first, &sockServer->mReadfds)) {
                if (((int)fdRecord.first == sockServer->mSocketFd) && (sockServer->mSocketType != AF_NETLINK)) {
                    /* If something happened on the master socket , then its an incoming connection */
                    struct sockaddr tmpSockAddr;
                    int clientSocket = -1;
                    socklen_t tmpAddrLen = sizeof(tmpSockAddr);
                    if ((clientSocket = accept(sockServer->mSocketFd, &tmpSockAddr, &tmpAddrLen)) < 0)
                    {
                        PRINT_ERROR(accept);
                    }
                    else {
                        SockInfo &si = sockServer->mFdList[clientSocket];
                        memcpy(&(si.sockAddr), &tmpSockAddr, tmpAddrLen);
                        si.fd = clientSocket;
                        si.rxDataLen = 0;
                        si.raiseEvent(sockServer, SocketServer::SOCKET_CLIENT_CONNECTED);
                        /* add client socket to select file descriptor list */
                        FD_SET(clientSocket, &sockServer->mReadfds);
                        if (clientSocket > maxSockFd) maxSockFd = clientSocket;
                        break;
                    }
                }
                else {
                    SockInfo &si = sockServer->mFdList[(int)fdRecord.first];
                    /* Check if it was for closing , and also read the incoming message */
                    si.AcquireLock();
                    ret = si.rxDataLen = read(si.fd, si.buffer, sizeof(si.buffer)-1);
                    if (ret >= 0) si.buffer[ret] = '\0';
                    si.ReleaseLock();
                    if (ret <= 0) {
                        si.raiseEvent(sockServer, SocketServer::SOCKET_CLIENT_DISCONNECTED);
                        close(si.fd);
                        /* remove client socket from select file descriptor list */
                        FD_CLR(si.fd, &sockServer->mReadfds);
                        sockServer->mFdList.erase(si.fd);
                        for (std::map<int,SockInfo>::iterator it=sockServer->mFdList.begin();
                                it!=sockServer->mFdList.end(); ++it) {
                            if ((int)it->first > maxSockFd)maxSockFd = (int)it->first;
                        }
                        break;
                    }
                    else {
                        si.raiseEvent(sockServer, SocketServer::SOCKET_DATA_BUFFERED);
                    }
                }
            }
        }
    }

    printf("SocketServer::serverThread exiting\n");
    pthread_mutex_lock(&sockServer->mThreadMutex);
    sockServer->mRunning = false;
    pthread_mutex_unlock(&sockServer->mThreadMutex);
    pthread_exit(NULL);
}

bool SocketServer::catchPauseAndResumeEvents(void)
{
    int semVal = 0;
    int ret = false;
    pthread_mutex_lock(&mThreadMutex);
    sem_getvalue(&mSem, &semVal);
    pthread_mutex_unlock(&mThreadMutex);
    if (semVal == 0) {
        if (sem_wait(&mSem) < 0)PRINT_ERROR(sem_post);
        ret = true;
    }
    return ret;
}

void SocketServer::eventHandler(SocketServer::SocketEvent aSocketEvent, SockInfo &aSockInfo) {
    switch (aSocketEvent) {
        case SocketServer::SOCKET_STARTED: PRINT_INFO("Captured Socket Event is : SOCKET_STARTED");
                                           break;
        case SocketServer::SOCKET_STOPPED: PRINT_INFO("Captured Socket Event is : SOCKET_STOPPED");
                                           break;
        case SocketServer::SOCKET_CLIENT_CONNECTED: PRINT_INFO("Captured Socket Event is : SOCKET_CLIENT_CONNECTED");
                                                    break;
        case SocketServer::SOCKET_CLIENT_DISCONNECTED: PRINT_INFO("Captured Socket Event is : SOCKET_CLIENT_DISCONNECTED");
                                                       break;
        case SocketServer::SOCKET_DATA_BUFFERED: PRINT_INFO("Captured Socket Event is : SOCKET_DATA_BUFFERED");
                                                 aSockInfo.AcquireLock();
                                                 PRINT_INFO("Data Received: %s", aSockInfo.buffer);
                                                 aSockInfo.ReleaseLock();
                                                 break;
    }
}
