/*
 * =====================================================================================
 *
 *       Filename:  testcases.cpp
 *
 *    Description:  This file has available all classes testcases which are
 *                  available in src directory.
 *
 *        Version:  1.0
 *        Created:  Wednesday 04 April 2018 11:47:06  IST
 *       Revision:  1.1
 *       Compiler:  g++
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

#include <iostream>
#include <map>
#include <string>
#include <cstring>
#define ADD_TEST_CASE(test)   ({g_HashMapTestCaseCallbackList[#test] = test;})
using namespace std;

typedef int (*t_TestCaseCallback)(void);

extern int SQLiteCppTest(void);


static map<string, t_TestCaseCallback> g_HashMapTestCaseCallbackList;

/*===================== Test Cases ==============================*/
static int TestSQLiteCpp(void)
{
    return SQLiteCppTest();
}

static int Test2(void)
{
    cout << "calling "<<__func__<<"\n";
    return 0;
}

static int hello(void)
{
    cout << "hello world!!\n";
    return 0;
}
/*==============================================================*/

static void InitTestCaseList(void)
{
    ADD_TEST_CASE(TestSQLiteCpp);
    ADD_TEST_CASE(Test2);
    ADD_TEST_CASE(hello);
}

static void ListOutTestCases(void)
{
    int count = 0;
    cout << "   The list of testcases:-\n"<<"   ======================\n";
    for (map<string, t_TestCaseCallback>::iterator it=g_HashMapTestCaseCallbackList.begin();
            it != g_HashMapTestCaseCallbackList.end(); ++it)
        cout << " " << ++count << ". " << it->first << '\n';
}

void RunTestCases(char *p_cmd)
{
    InitTestCaseList();

    if (p_cmd == NULL) {
        ListOutTestCases();
        return;
    }

    int status = 0;
    t_TestCaseCallback testCase = NULL;
    if(strcmp(p_cmd,"all") == 0) {
        int count = 0, passed = 0;

        cout<< "\tTest cases are starting...\n";
        for (map<string, t_TestCaseCallback>::iterator it=g_HashMapTestCaseCallbackList.begin();
                it != g_HashMapTestCaseCallbackList.end(); ++it) {
            cout << "\n " << ++count << ". Executing testcase : " << it->first << '\n';
            cout << "-----------------------------------------------------\n";
            testCase = it->second;
            status = testCase();
            if (!status) passed++;
            cout << "-----------------------------------------------------\n";
            cout << " Testcase - " << it->first << " : " << ((status)?"FAILED!!": "PASSED") << "\n";
            cout << "===================================================================\n\n";
        }
        cout<< "\tTest cases are Finished!!\n";
        cout<< "\n\tTotal testcases passed : " << passed << "/" <<count << "\n";
    }
    else {
        map<string, t_TestCaseCallback>::iterator it = g_HashMapTestCaseCallbackList.find(p_cmd);
        if ( it != g_HashMapTestCaseCallbackList.end()) {
            cout << "\n\tExecuting testcase : " << it->first << '\n';
            cout << "-----------------------------------------------------\n";
            testCase = it->second;
            status = testCase();
            cout << "-----------------------------------------------------\n";
            cout << "\t Testcase - " << it->first << " : " << ((status)?"FAILED!!": "PASSED") << "\n";
        }
        else {
            cout << "Error: Testcase \"" << p_cmd << "\"  not found. Please refer to below list:\n";
            ListOutTestCases();
        }
    }
}
