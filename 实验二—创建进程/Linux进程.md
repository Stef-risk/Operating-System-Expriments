Linux进程



创建进程使用fork()函数，fork的作用为从原程序中创建一个分离的进程，被创建的新进程复制当前进程的映像，在进程表中创建一个新的表项，新进程与原进程执行的代码相同，但是新进程有自己的数据空间、环境和文件描述符。

fork函数的原型为：

```c
#include<sys/types.h>
#include<unistd.h>

pid_t fork(void);
```

其返回值为pid_t，即进程标识符。

实验中调用fork的代码如下：

```c
	pid_t pid;
   
    printf("Fork program starting:\n\n");
    pid=fork(); //复制进程映像
    if(pid==-1)
    {
        perror("Cannot create process.\n");  //返回值-1则表示fork失败
        exit(1);
    }

    else if(pid==0) //对子进程
    {

        char *const child_argv[]= {argv[1],0};
        //printf("%s\n",argv[1]);
        execv("/home/stefrisk/Ste_Codez/LinuxProcess/ChildProcess",child_argv);
    }

    else    //对父进程
    {
        pid_t child_pid;
        int stat_val;   //存储状态信息
	....
```

定义变量pid来接收fork的返回值，并在程序接下来以pid的值为依据区分父进程和子进程。fork返回值为1则表示创建进程失败，父进程的返回值大于0，而子进程的返回值为0.

在成功创建进程之后，通过execv来替换进程映像，使用execv函数后，原来的程序不再运行，转而运行我们指定的程序。execv函数原型如下：

```c
#include<unistd.h>

int execv(const char *path,char *const argv[])
```

第一个参数是程序的位置，第二个字符数组参数可以用于传入指定进程所需要的参数。

在实验中代码如下：

```c
 char *const child_argv[]= {argv[1],0};
 execv("/home/stefrisk/Ste_Codez/LinuxProcess/ChildProcess",child_argv);
```

argv[1]即父进程在命令行中所接受的字符串。

父进程创建子进程后调用wait()函数来等待子进程的退出。wait系统调用将暂停父进程直到其子进程结束为止。这个调用返回子进程的PID，它通常是已经结束运行的子进程的PID。

wait函数原型：

```c
#include<sys/types.h>
#include<sys/wait.h>

pid_t wait(int *stat_loc);
```

实验中父进程调用wait的代码如下：

```c
		pid_t child_pid;
        int stat_val;   //存储状态信息

        child_pid=wait(&stat_val);
        printf("\nThe child process has ended, it's pid is %d\n",child_pid);
        if(WIFEXITED(stat_val))     //如果正常退出打印状态码
            printf("child exited with code %d\n",WEXITSTATUS(stat_val));
        else        //对非正常退出
            printf("child terminated abnormally.\n");
```

stat_val存储子进程退出的状态信息，child_pid则接收wait的返回值，即子进程运行时的pid 。 最后的WIFEXITED通过解析状态信息来判断子进程是否正常退出。



子进程除了打印名字之外还需要完成计算运行时间的工作。

在子进程中，使用gettimeofday()获取当前的Unix epoch时间，即从1970年1月1日到现在所经过的时间，将其存储在timeval类型的结构中，timeval结构定义如下

```c
struct timeval{
	time_t   tv_sec//表示从epoch到现在的秒数
	time_t   te_usec//表示微秒
}
```



为了将时间表示成易读的格式，使用localtime函数将获得的时间转换到tm类型的结构中。localtime函数接收一个指向timeval类型tv_sec对象的指针并返回tm对象。

tm对象结构如下：

![image-20201221070111416](C:\Users\Stefanny\AppData\Roaming\Typora\typora-user-images\image-20201221070111416.png)



之后使用strftime对获得的时间进行正规化表示。过程如下：

```c++
	struct timeval stv,etv;
    struct tm* ptm;
    char SteTime[39];
	gettimeofday(&stv,NULL); //仅仅获取时间即可，不需要时区
    ptm=localtime(&stv.tv_sec); //将获得的时间转换为struct tm
    strftime(SteTime,sizeof(SteTime),"%Y-%m-%d %H:%M:%S",ptm);
```

计算程序运行时间则使用了timeval类型，运算公式如下：

```C
milletime=MILLION*(etv.tv_sec-stv.tv_sec)+etv.tv_usec-stv.tv_usec;
```



程序的运行如图：

![Screenshot from 2020-12-21 01-18-21](D:\scholar_stuffs\OS\Experiments\实验二—创建进程\LinuxProcess\Screenshot from 2020-12-21 01-18-21.png)

