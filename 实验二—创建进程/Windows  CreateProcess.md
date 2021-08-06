Windows  CreateProcess



CreateProcess定义如下：

```c++
BOOL CreateProcessA(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);
```

在本次实验中创建进程时需要关注LPSTR、 LPSTARTUPINFOA 以及LPPROCESS_INFORMATION类型变量，lpApplicationName为空时，lpCommandLine接收从命令行传入的字符并且打开。lpStartupInfo指向一个STARTUOINFO结构，lpProcessInformation结构接收新进程的标识信息。

创建进程成功之后，lpProcessInformation会被自动填充，其中包含了句柄和新进程的标识符以及新进程的主线程。	当不需要句柄时，使用CloseHandle关闭句柄。



PROCESS_INFORMATION结构

```c++
typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD  dwProcessId;
  DWORD  dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;
```

其中hProcess是新进程的句柄，用来在对流程对象执行操作的所有功能中制定流程



在创建进程之前先创建STARTUPINFO以及 PROCESS_INFORMATION变量，并使用ZeroMemory进行初始化，用0填充内存块

```c++
void ZeroMemory(
  [in] PVOID  Destination,
  [in] SIZE_T Length
);
```

对于STARTUPINFO结构，其作为CreateProcess的参数，指定在创建时进程的窗口站，桌面，标准句柄和主窗口的外观。

```c++
typedef struct _STARTUPINFOA {
  DWORD  cb;
  LPSTR  lpReserved;
  LPSTR  lpDesktop;
  LPSTR  lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  WORD   wShowWindow;
  WORD   cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;
```



其中，cb以字节为单位指定数据结构的大小



创建进程后父进程调用WaitForSingleObject函数等待子进程的结束

```c++
DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
);
```

该函数等待指定的对象直到其处于发送信号的状态或者超过指定的等待时间

hHandle用于指定要等待对象的句柄，dwMilliseconds指定等待的时间，在程序中指定为INFINITE,即不限时长。