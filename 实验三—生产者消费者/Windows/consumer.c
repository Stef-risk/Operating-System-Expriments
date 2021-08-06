// Consumer.cpp :生产者消费者中的消费者进程 
//  Ste-Made

#include<windows.h>
#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include<tchar.h>
#include<string.h>

#define BUFFER_SIZE 4
#define CONSUMER_AMOUNT 4
#define PRODUCER_AMOUNT 3

struct buffet {     //定义缓冲区
    char for_name[BUFFER_SIZE][2];    
    int written_by_me;
};
struct buffet* hereBuffer;  //指向缓冲区的指针
static int traceS = 0;      //用于追踪生产和消费的顺序
static int traceP = 0;

static int P(HANDLE hsem);  //定义P，V操作以及删除信号量操作
static int V(HANDLE hsem);
static int delSem(HANDLE hsem);

HANDLE mutex;       //声明需要用到的三个信号量的句柄
HANDLE fillcount;
HANDLE emptycount;

HANDLE shared_memory;   //指向共享内存区的句柄
LPCTSTR myBuf;

void buffer_set(void);  //初始化缓冲区
char removeItemFromBuffer(void);
void showConsumerStatus(char consu);  //打印消费者状态

void Consumer(void)     //消费者进程函数
{
    for (int i = 0 ; i < 3;i++)
    {
        srand((unsigned int)time(NULL));
        Sleep((rand()%5)*1000);        //   随机睡眠五秒以内的时间
        char item;
        P(fillcount);
        P(mutex);
        item = removeItemFromBuffer();  //从缓冲区中取出字母
        showConsumerStatus(item);
        V(mutex);
        V(emptycount);
    }
}

int main(int argc, TCHAR* argv[])
{
    //创建信号量
    mutex = CreateSemaphore(NULL, 1, 1, "mutexx");
    fillcount = CreateSemaphore(NULL, 0, BUFFER_SIZE, "fillcountt");
    emptycount = CreateSemaphore(NULL, BUFFER_SIZE, BUFFER_SIZE, "emptycountt");

    //创建共享内存区域的映射
    shared_memory = CreateFileMapping(
        INVALID_HANDLE_VALUE,    
        NULL,                    
        PAGE_READWRITE,          
        0,                       
        sizeof(struct buffet),                
        (LPCTSTR)L"share_memory"              
    );

    if (shared_memory == NULL)
    {
        printf("Could not create file mapping object (%d).\n",GetLastError());
        exit(EXIT_FAILURE);
    }
    myBuf = (LPTSTR)MapViewOfFile(shared_memory,   
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        sizeof(struct buffet));

    if (myBuf == NULL)
    {
        printf("Could not map view of file (%d).\n",GetLastError());

        CloseHandle(shared_memory);
        exit(EXIT_FAILURE);
    }
    hereBuffer = (struct buffet*)myBuf; //将指向共享内存的指针修改为指向所需结构的指针
    buffer_set();   //初始化缓冲区

    //创建生产者进程
    STARTUPINFO si; //所打开的新窗口的信息
    PROCESS_INFORMATION pi;

    //将si与pi初始化
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    if (argc != 2)
    {
        printf("Please use in command line, input is this program's name and child program's name.\n");
        exit(EXIT_FAILURE);
    }

    if (!CreateProcess(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))      //创建子进程来运行生产者进程
    {
        printf("Failed to create process.\n");
        printf("%d", GetLastError());
        exit(EXIT_FAILURE);
    }
    printf("开始模拟生产者消费者...\n");

    //运行消费者进程
    for(int i=0;i<CONSUMER_AMOUNT;i++)
    	Consumer();

    // 等待子进程退出
    WaitForSingleObject(pi.hProcess, INFINITE); //传入子进程的进程句柄信息，并允许等待无限长的时间
    printf("模拟结束。\n");

    //子进程退出后关闭进程和线程的句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(shared_memory);
    delSem(mutex);
    delSem(emptycount);
    delSem(fillcount);
}

char removeItemFromBuffer(void)
{
    char item;
    for (int i = 0;i < BUFFER_SIZE;i++)
    {
        if (hereBuffer->for_name[i][0] == '#')
        {
            continue;
        }
        else
        {
            if (hereBuffer->for_name[i][1] == traceP)
            {
                item = hereBuffer->for_name[i][0];
                hereBuffer->for_name[i][0] = '#';
                traceP++;
                break;
            }
            else
            {
                continue;
            }
        }
    }
    return item;
}
void showConsumerStatus(char consu)
{
    printf("本次消费者消费：\t%c\n", consu);
    printf("当前缓冲区为：   \t%c\t%c\t%c\t%c\n", hereBuffer->for_name[0][0], hereBuffer->for_name[1][0], hereBuffer->for_name[2][0], hereBuffer->for_name[3][0]);
}

void buffer_set(void)       //将没有被放置生产物品的地方设置为#
{
    for (int i = 0;i < BUFFER_SIZE;i++)
    {
        hereBuffer->for_name[i][0] = '#';
        hereBuffer->for_name[i][1] = 0;
    }
}

static int P(HANDLE hsem) 
{
    WaitForSingleObject(hsem, INFINITE);
    return 0;
}
static int V(HANDLE hsem)
{
    ReleaseSemaphore(hsem, 1, NULL);
    return 0;
}
static int delSem(HANDLE hsem)      
{
    CloseHandle(hsem);
    return 0;
}

