// memoryMonitor.cpp :  内存监视器
// Ste-Made
#define _CRT_SECURE_NO_WARNINGS
#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<tchar.h>
#include<string.h>
#include<Psapi.h>
#include<TlHelp32.h>

#pragma comment(lib, "user32.lib")

#define DIV 1024    //从字节转换到KB
#define WIDTH 3

int kb2gb = 1024 * 1024;//用于从KB转换到GB

void showSystemInfo(void);                               //系统地址空间布局，包括程序所能使用的地址空间信息
void _tglobalMemoryStatus(void);                         //物理内存和虚拟内存的使用情况
void performanceInformation(void);                       //获取性能信息
void GetAllProcess(void);                                //打印所有进程的名称，pid以及工作集大小
void ProcessDetail(DWORD processID);                     //详细展示单个进程的地址空间
char* getBlockState(MEMORY_BASIC_INFORMATION mpi);       //得到块的提交、保留或者空闲状态
char* getBlockProtection(MEMORY_BASIC_INFORMATION mpi);  //块的保护类型
char* getBlockType(MEMORY_BASIC_INFORMATION mpi);        //得到块的类型
void fakeUI(void);                                       //创建一个和谐的用户界面
void fakeFront(void);

int main()
{
    fakeFront();
    showSystemInfo();       //在监视器开始时先输出系统信息、内存使用信息以及性能信息
    _tglobalMemoryStatus();
    performanceInformation();
    fakeUI();   //fake UI为监视器的交互界面，用户通过输入相应数字获取相应信息
    return 0;
}

void fakeFront(void)
{
    for (int i = 0;i < 60;i++)
        putchar('*');
    putchar('\n');
    printf("*\t\t%s\t\t   *\n", "Welcome to Ste Memory Monitor");
    for (int i = 0;i < 60;i++)
        putchar('*');
    putchar('\n');
    putchar('\n');
    printf("Below shows some basic information about your system's memory status:\n\n");
}

void fakeUI(void)
{
    int chioce;      //记录用户操作选项
    int pid;        //获取进程标识符
    while (true)
    {
        printf("\nWhich do you favor:\n");
        printf("\t1 - Show general information about every process.\n");
        printf("\t2 - Show a certain process's detail information.\n");
        printf("\t3 - quit.\n");
        printf("your choice :");
        scanf("%d", &chioce);
        switch (chioce)
        {
        case 1:
            printf("Show general information about every process.\n");
            GetAllProcess();
            break;
        case 2:
            printf("Show a certain process's detail information.\n");
            printf("Give me a PID and I will show you:");
            scanf("%d", &pid);
            putchar('\n');
            ProcessDetail((DWORD)pid);
            break;
        default:
            printf("ByeBye! Thx for using Ste Memory Monitor. Best regards.\n");
            return;
        }
    }
}

char* getBlockState(MEMORY_BASIC_INFORMATION mpi)       //得到块的提交、保留或者空闲状态
{
    char* state = (char*)malloc(12 * sizeof(char));
    if (mpi.State == MEM_COMMIT)
    {
        strcpy(state, "Commited");
    }
    else if (mpi.State == MEM_FREE)
    {
        strcpy(state, "Free");
    }
    else if (mpi.State == MEM_RESERVE)
    {
        strcpy(state, "Reserved");
    }
    return state;
}
char* getBlockType(MEMORY_BASIC_INFORMATION mpi)        //得到块的类型
{
    char* type = (char*)malloc(12 * sizeof(char));
    if (mpi.Type == MEM_IMAGE)
    {
        strcpy(type, "Image");
    }
    else if (mpi.Type == MEM_MAPPED)
    {
        strcpy(type, "Mapped");
    }
    else if (mpi.Type == MEM_PRIVATE)
    {
        strcpy(type, "Private");
    }
    else
    {
        strcpy(type, " ");      //针对空闲块
    }
    return type;
}

char* getBlockProtection(MEMORY_BASIC_INFORMATION mpi)  //块的保护类型
{
    char* protection = (char*)malloc(20 * sizeof(char));
    if (mpi.Protect == PAGE_EXECUTE)
    {
        strcpy(protection, "EXECUTE");
    }
    else if (mpi.Protect == PAGE_EXECUTE_READ)
    {
        strcpy(protection, "EXECUTE_READ");
    }
    else if (mpi.Protect == PAGE_EXECUTE_READWRITE)
    {
        strcpy(protection, "EXECUTE_READWRITE");
    }
    else if (mpi.Protect == PAGE_EXECUTE_WRITECOPY)
    {
        strcpy(protection, "EXECUTE_WRITECOPY");
    }
    else if (mpi.Protect == PAGE_NOACCESS)
    {
        strcpy(protection, "NOACCESS");
    }
    else if (mpi.Protect == PAGE_READONLY)
    {
        strcpy(protection, "READONLY");
    }
    else if (mpi.Protect == PAGE_READWRITE)
    {
        strcpy(protection, "READWRITE");
    }
    else {
        strcpy(protection, "OTHERTYPE");
    }
    return protection;
}

void ProcessDetail(DWORD processID)    //详细展示单个进程的地址空间
{
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);    //将系统信息写入SYSTEM_INFO结构中
    MEMORY_BASIC_INFORMATION mpi;  //进程虚拟地址空间信息
    ZeroMemory(&mpi, sizeof(mpi));  //分配缓冲区

    LPCVOID pBegin, pEnd;
    char* blockStat;
    char* blockType;
    char* blockProt;
    pBegin = (LPVOID)siSysInfo.lpMinimumApplicationAddress; //起始地址
    unsigned int blockSize;
    HANDLE hProcess;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS,
        FALSE, processID);
    if (hProcess == NULL)       //打开失败的处理
    {
        fprintf(stderr, "failed to open a process.\n");
        return;
    }

    printf("---------------------------------process %5d---------------------------------\n", processID);
    printf("Block Address\t\tBlock Size\tBlock stat\tProtection\tBlock Type\n");
    printf("=============\t\t==========\t==========\t==========\t==========\t\n");
    while (true)
    {
        if (VirtualQueryEx(hProcess, pBegin, &mpi, sizeof(mpi)) != sizeof(mpi))
        {
            continue;
        }
        pEnd = (PBYTE)pBegin + mpi.RegionSize;      //得到块的结束信息
        //判断块状态、保护位和类型
        blockStat = getBlockState(mpi);
        blockType = getBlockType(mpi);
        blockProt = getBlockProtection(mpi);
        //计算块的大小
        blockSize = mpi.RegionSize * DIV / kb2gb;
        printf("0x%08lx-0x%08lx\t%8dKB\t%10s\t%10s\t%10s\n", (unsigned int)pBegin, (unsigned int)pEnd, blockSize, blockStat, blockProt, blockType);
        pBegin = pEnd;
        if ((unsigned int)pBegin >= (unsigned int)siSysInfo.lpMaximumApplicationAddress)    //达到地址空间最大值时退出
            break;
    }
}

void GetAllProcess(void)        //打印所有进程的名称，pid以及工作集大小
{
    PROCESSENTRY32 pe32;    //存储进程信息的结构
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;    //用于获取进程工作集大小
    bool getProcess;
    pe32.dwSize = sizeof(pe32);     //不初始化会导致失败
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //进程快照句柄
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "failed to make snapshot.\n");   //快照失败
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("-------------------------------------Process information-------------------------------------\n");
        printf("---------------------------------------------------------------------------------------------\n");
        printf("  process name\t\t\t\t  \tprocess id\tthreads amount\tworking set size\n");
        printf("  ============\t\t\t\t  \t==========\t==============\t================\n");
        getProcess = Process32First(hSnapshot, &pe32); //存入第一个进程的信息

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |      //打开进程句柄
            PROCESS_VM_READ,
            FALSE, (DWORD)pe32.th32ProcessID);

        while (getProcess)
        {
            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |      //打开进程句柄
                PROCESS_VM_READ,
                FALSE, (DWORD)pe32.th32ProcessID);
            GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));  //将信息存入pmc结构中
            if (hProcess == NULL)
                pmc.WorkingSetSize = 0;
            printf("%-36ls\t\t%6d\t\t  %3d\t%10dMB\n", pe32.szExeFile, pe32.th32ProcessID, pe32.cntThreads, pmc.WorkingSetSize / kb2gb);    //因为传递的是宽类型字符串，使用ls类型输出
            getProcess = Process32Next(hSnapshot, &pe32);       //遍历获取所有进程的信息
        }
        printf("---------------------------------------------------------------------------------------------\n");
    }
    CloseHandle(hSnapshot); //关闭句柄
}

void performanceInformation(void)   //获取性能信息
{
    //获取系统的存储器使用情况
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    GetPerformanceInfo(&pi, sizeof(pi));
    int pagesize = pi.PageSize;
    printf("----------------Performance information-------------------\n");
    printf("Total commit page and commit limit: %ld/%ld.\n", pi.CommitTotal, pi.CommitLimit);   //已提交的页面和最大提交页面数
    printf("Page commit peak: %ld.\n", pi.CommitPeak);
    printf("Physical memory allocated by page: %ld/%ld.\n", pi.PhysicalAvailable, pi.PhysicalTotal); //当前可用的物理内存量（以页为单位）。这是可以立即重用而不必先将其内容写入磁盘的物理内存量。它是备用列表，空闲列表和零列表大小的总和。
    printf("System cache: %.2f GB.\n", double(pi.SystemCache) * 4 / kb2gb);     //系统高速缓存内存量（以页为单位）,为备用列表加上系统工作集的大小。
    printf("Open handles: %ld.\n", pi.HandleCount);             //句柄数量
    printf("Process amount: %ld.\n", pi.ProcessCount);          //进程计数
    printf("Thread amount: %ld.\n", pi.ThreadCount);            //线程计数
    printf("----------------------------------------------------------\n");
}

void _tglobalMemoryStatus(void)         //物理内存和虚拟内存的使用情况
{
    MEMORYSTATUSEX statex;

    statex.dwLength = sizeof(statex);

    GlobalMemoryStatusEx(&statex);

    printf("----------------Memory usage information------------------\n");

    printf("There is  %*ld percent of memory in use.\n",        //正在使用的内存百分比
        statex.dwMemoryLoad);
    printf("There are %.2f/%.2f free  GB of physical memory.\n", double(statex.ullAvailPhys) / (DIV * kb2gb),
        double(statex.ullTotalPhys) / (DIV * kb2gb));   //物理内存的使用量与总量信息
    printf("There are %.2f/%.2f free  GB of page file.\n", double(statex.ullAvailPageFile) / (DIV * kb2gb),
        double(statex.ullTotalPageFile) / (DIV * kb2gb));  //页面文件的使用量与总量信息
    printf("There are %.2f/%.2f free  GB of virtual memory.\n", double(statex.ullAvailVirtual) / (DIV * kb2gb),
        double(statex.ullTotalVirtual) / (DIV * kb2gb));   //虚拟内存的使用量与总量信息

    printf("----------------------------------------------------------\n");
}

void showSystemInfo(void)       //系统地址空间布局，包括程序所能使用的地址空间信息
{
    SYSTEM_INFO siSysInfo;

    GetSystemInfo(&siSysInfo);    //将系统信息写入SYSTEM_INFO结构中
    printf("--------------------System information--------------------\n");
    // Display the contents of the SYSTEM_INFO structure. 
    printf("Page size: %uKB\n", siSysInfo.dwPageSize / DIV);  //页面大小
    printf("Minimum application address: 0x%lx\n",
        siSysInfo.lpMinimumApplicationAddress);
    printf("Maximum application address: 0x%lx\n",
        siSysInfo.lpMaximumApplicationAddress);
    printf("----------------------------------------------------------\n");
}