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

extern "C" {
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
};

using namespace std;
namespace server {

    class SocketServer {
        public:
            SocketServer(bool aNetLinkPidAsCurrentPid, int aNetlinkGroups = -1,
                    int aCommType = SOCK_DGRAM, int aProtocol = NETLINK_KOBJECT_UEVENT) ;
            SocketServer(unsigned short aListeningNwPort, int aCommType = SOCK_STREAM, int aProtocol = IPPROTO_IP);
            SocketServer(const char *aLocalSocketName, int aCommType = SOCK_STREAM, bool aAbstract = true);
            ~SocketServer(void);

            int start(void);
            int resume(void);
            int pause(void);
            int stop(void);

        private:
            static void *ServiceThread(void *arg);

        private:
            int mSocketFd;
            void *mSockAddr;
            socklen_t mAddrLen;
            int mSocketType;
            int mCommType;
            int mProtocol;
            pthread_t mListeningThread;
            pthread_mutex_t mThreadMutex;
            sem_t mSem;
    };

};
