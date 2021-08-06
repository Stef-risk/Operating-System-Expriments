// producer.cpp : 生产者进程程序
//  Ste-Made

#include <Windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define BUFFER_SIZE 4
#define CONSUMER_AMOUNT 4
#define PRODUCER_AMOUNT 3

char name_set[4] = { 'S','F','Z','H' };     //生产者生产空间

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

void PutItemIntoBuffer(char item);
char produceItem(void);
void showProducerStatus(char consu);  //打印消费者状态
void Producer(void)     //定义生产者进程
{
    for (int i = 0;i < 4;i++)
    {
        srand((unsigned int)time(NULL));
        Sleep((rand() % 3)*1000);
        char item;
        item = produceItem();       //随机选取一个集合中的字母
        P(emptycount);
        P(mutex);
        PutItemIntoBuffer(item);    //将所生产的字母放进缓冲区中
        showProducerStatus(item);
        V(mutex);
        V(fillcount);
    }
}

int main()
{
    srand((unsigned int)time(NULL));    //初始化随机数种子
    //打开信号量
    mutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "mutexx");
    emptycount = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE ,"emptycountt");
    fillcount = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "fillcountt");

    //连接到共享内存的区域
    shared_memory = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,   
        FALSE,                 
        (LPCTSTR)L"share_memory");             

    if (shared_memory == NULL)
    {
        printf("Could not create file mapping object (%d1).\n", GetLastError());
        exit(EXIT_FAILURE);
    }

    myBuf = (LPTSTR)MapViewOfFile(shared_memory, 
        FILE_MAP_ALL_ACCESS, 
        0,
        0,
        sizeof(struct buffet));

    if (myBuf == NULL)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        CloseHandle(shared_memory);
        exit(EXIT_FAILURE);
    }
    hereBuffer=(struct buffet*)myBuf;
    //运行消费者进程
    for(int i=0;i<PRODUCER_AMOUNT;i++)
    	Producer();

    
}

void PutItemIntoBuffer(char item)
{
    for (int i = 0;i < BUFFER_SIZE;i++)
    {
        if (hereBuffer->for_name[i][0] == '#')
        {
            hereBuffer->for_name[i][0] = item;
            hereBuffer->for_name[i][1] = traceS++;      //记录被生产的顺序
            break;
        }
        else
        {
        	continue;
		}
    }
}
char produceItem(void)
{
    char product;
    srand((unsigned int)time(NULL));
    product = name_set[rand() % 4];
    return product;
}
void showProducerStatus(char consu)
{
    printf("生产者本次生产： \t%c\n", consu);
    printf("当前缓冲区为：   \t%c\t%c\t%c\t%c\n", hereBuffer->for_name[0][0], hereBuffer->for_name[1][0], hereBuffer->for_name[2][0], hereBuffer->for_name[3][0]);
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
