#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/time.h>

#define MILLION 1000000

int main(int argc,char *argv[])
{
    pid_t pid;
    struct timeval stv,etv;
    
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

        child_pid=wait(&stat_val);
        printf("\nThe child process has ended, it's pid is %d\n",child_pid);
        if(WIFEXITED(stat_val))     //如果正常退出打印状态码
            printf("child exited with code %d\n",WEXITSTATUS(stat_val));
        else        //对非正常退出
            printf("child terminated abnormally.\n");
    }

    exit(0);

}