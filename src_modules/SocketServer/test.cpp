/*
 * =====================================================================================
 *
 *       Filename:  test.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  Sunday 29 April 2018 08:42:31  IST
 *       Revision:  1.1
 *       Compiler:  gcc
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include "SocketServer.h"
#include "stdio.h"

using namespace server;

int testLocalSocketServer(void)
{
    int ret = 0;
    int c = 0;
    SocketServer gTestLocalServer("hidden");
    printf("\nWaiting for input for start(s), stop(t), terminate(x):\n");
    while ((c = getchar()) != (int)'x') {
        getchar();
        switch(c) {
            case 's':
                ret = gTestLocalServer.start();
                if (ret < 0) goto tlssErr;
                printf("testServer started\n");
                break;
            case 't':
                gTestLocalServer.stop();
                printf("testServer stoped\n");
                break;
        }
        printf("\nWaiting for input for start(s), stop(t), terminate(x):\n");
    }

tlssErr:
    return ret;
}

