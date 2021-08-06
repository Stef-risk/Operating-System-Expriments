// ParentProcess.cpp : 使用CreateProcess方法创建进程
//  Ste-Made

#include <iostream>
#include <windows.h>
#include <tchar.h>
using namespace std;

int main(int argc,TCHAR *argv[])
{
    STARTUPINFO si; //所打开的新窗口的信息
    PROCESS_INFORMATION pi;

    //将si与pi初始化
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    if (argc != 2)
    {
        cout << "Please use in command line, input is this program's name and child program's name." << endl;
        exit(1);
    }

    if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        cout << "Failed to create process." << endl;    //创建不成功打印相应信息
        exit(1);
    }
    // 等待子进程退出
    WaitForSingleObject(pi.hProcess, INFINITE); //传入子进程的进程句柄信息，并允许等待无限长的时间

    //子进程退出后关闭进程和线程的句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

}


