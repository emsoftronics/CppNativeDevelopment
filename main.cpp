/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  This file contains main function which starts the project.
 *
 *        Version:  1.0
 *        Created:  Sunday 08 April 2018 09:48:35  IST
 *       Revision:  1.1
 *       Compiler:  g++
 *
 *         Author:  Jagdish Prajapati (JP), prajapatijagdish@gmail.com
 *        Company:  Emsoftronic
 *
 * =====================================================================================
 */

extern void RunTestCases(char *p_cmd);

int main(int argc, char **argv)
{
    if (argc==1) RunTestCases((char *)0);
    else RunTestCases(argv[1]);
    return 0;
}
