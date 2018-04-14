/*
 * =====================================================================================
 *
 *       Filename:  SocketServer.h
 *
 *    Description:  This file contains SocketServer class definition which is help full
 *                  to create socket server.
 *
 *        Version:  1.0
 *        Created:  Monday 09 April 2018 11:15:15  IST
 *       Revision:  1.1
 *       Compiler:  g++
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include <map>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
};

#define CLIENT_QUEUE_LEN    50

using namespace std;
namespace server {

    class SocketServer {
        public:
            enum SocketEvent {
                SOCKET_STARTED = 1,
                SOCKET_STOPPED,
                SOCKET_DATA_BUFFERED,
                SOCKET_CLIENT_CONNECTED,
                SOCKET_CLIENT_DISCONNECTED,
            };

        private:
            class SockInfo {
                private:
                    SocketEvent event;
                    int threadJoined;
                    bool threadRunning;
                    pthread_mutex_t threadLock;
                    pthread_t workerThread;

                public :
                    int fd;
                    SocketServer *sockServer;
                    struct sockaddr sockAddr;
                    char name[sizeof(struct sockaddr_un) - sizeof(struct sockaddr)];
                public:
                    SockInfo(void);
                    ~SockInfo(void);

                    void raiseEvent(SocketServer *aSocketServer, int aFileDesc, SocketEvent aEvent);

                private:
                    void handleEvent(void);
                    static void *eventThread(void *arg);

            };

        public:
            SocketServer(bool aNetLinkPidAsCurrentPid, int aNetlinkGroups = -1,
                    int aCommType = SOCK_DGRAM, int aProtocol = NETLINK_KOBJECT_UEVENT) ;
            SocketServer(unsigned short aListeningNwPort, int aCommType = SOCK_STREAM, int aProtocol = IPPROTO_IP);
            SocketServer(const char *aLocalSocketName, int aCommType = SOCK_STREAM, bool aAbstract = true);
            virtual ~SocketServer(void);

            int start(void);
            void resume(void);
            void pause(void);
            void stop(void);
            int getFd(void);
            bool isRunning(void);
            bool isPaused(void);
            struct sockaddr GetSocketAddress(int aFileDesc);

        protected:
            virtual void eventHandler(SocketEvent aSocketEvent, int aFileDesc);

        private:
            static void *serviceThread(void *arg);
            bool catchPauseAndResumeEvents(void);

        private:
            map<int, SockInfo> mFdList;
            int mSocketFd;
            void *mSockAddr;
            socklen_t mAddrLen;
            int mSocketType;
            int mCommType;
            int mProtocol;
            bool mRunning;
            bool mThreadJoined;
            pthread_t mListeningThread;
            pthread_mutex_t mThreadMutex;
            sem_t mSem;
            fd_set mReadfds;
    };

};
