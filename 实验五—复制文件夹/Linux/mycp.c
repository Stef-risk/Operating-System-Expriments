#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<string.h>
#include<utime.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<unistd.h>

#define SIZE 1024       
#define MAX_DIR_LENGTH 256

void CreateDirectory(char *dirname);    //创建指定目录
void RecursionClone(char *dirname);     //递归复制目录以及文件
void timeNmode(struct stat statbuf);                   //修改文件的权限以及修改、访问时间信息

static char beforeWEntry[MAX_DIR_LENGTH];    //存储进入一个目录之前写入目录的位置
static char beforeREntry[MAX_DIR_LENGTH];    //存储进入一个目录之前读取目录的位置
static char currRdir[MAX_DIR_LENGTH];       //当前的写入相对目录
static char currwRdir[MAX_DIR_LENGTH];      //当前的读取相对目录
static char writedirName[MAX_DIR_LENGTH];   //记录写入目录的位置
static char writefileName[MAX_DIR_LENGTH];  //记录写入文件时的位置
static char readdirName[MAX_DIR_LENGTH];    
static char replacement[MAX_DIR_LENGTH];
static int count=1;

int main(int argc,char *argv[])
{
    char *srcdir=".";   //用户指定的原目录
    char *destdir=".";  //目标目录

    if(argc<3)
    {
        printf("not enough arguments.\n");
        exit(EXIT_FAILURE);
    }
    srcdir=argv[1];
    destdir=argv[2];

    strcpy(currRdir,destdir);       //维护相对读取目录和相对写入目录
    strcpy(currwRdir,srcdir);
    strcpy(writedirName,destdir);
    strcat(writedirName,"/");

    CreateDirectory(destdir);   //先建立一个目标根目录
    RecursionClone(srcdir);     //核心递归复制过程
    return 0;
}
void CreateDirectory(char destdirr[])       //根据输入的绝对路径创建目录
{
    struct stat file_stat;
    int ret;
    ret=stat(destdirr,&file_stat);
    if(ret<0)
    {
        ret=mkdir(destdirr,0775);
      //  printf("create dir %s\n",destdirr);
        if(ret<0)
        {
            printf("failed to create directory\n");
            return;
        }
    }
}

void RecursionClone(char* dirname)
{
    DIR *dp;    //目录流
    struct dirent *entry;   //指向目录结构
    struct stat statbuf;

    int in,out;     //用于复制文件操作
    int nread;
    char block[SIZE];

    if((dp=opendir(dirname))==NULL)     //打开目录
    {
        perror("failed to open directory.\n");
        exit(EXIT_FAILURE);
    }
    chdir(dirname); //进入目录

    if(count!=1)
    {
        strcpy(beforeWEntry,currRdir);   //建立现在的相对工作目录
        strcat(currRdir,"/");
        strcat(currRdir,dirname);

        strcpy(beforeREntry,currwRdir);
        strcat(currwRdir,"/");          //建立现在的相对读取目录
        strcat(currwRdir,dirname);
    }
    count++;

    while((entry=readdir(dp))!=NULL)        //遍历读取目录
    {
        lstat(entry->d_name,&statbuf);  //将当前读取到的信息存入statbuf

        if(S_ISDIR(statbuf.st_mode))    //对于读取到的目录项
        {
            //在文件夹中创建该目录
            strcpy(writedirName,currRdir);
            strcat(writedirName,"/");
            strcat(writedirName,entry->d_name); //构造创建时的目录名
            CreateDirectory(writedirName);

            strcpy(readdirName,currwRdir);
            strcat(readdirName,"/");
            strcat(readdirName,entry->d_name); //构造创建时的目录名

            if(strcmp(".",entry->d_name)==0 ||
                strcmp("..",entry->d_name)==0)
                continue;

            //在创建后修改文件夹信息使其与原来相同
            strcpy(replacement,writedirName);

            timeNmode(statbuf);     //修改权限以及时间信息
            RecursionClone(entry->d_name);      //递归进入子目录

        }
        else        //非目录项
        {
            
            strcpy(writefileName,writedirName);     //构造写入文件时的路径
            strcat(writefileName,"/");
            strcat(writefileName,entry->d_name);
            strcpy(replacement,writefileName);

            if(S_ISLNK(statbuf.st_mode))        //单独处理符号链接
            {
                char readfileName[MAX_DIR_LENGTH];  //当前的读取名
                char writelinkedfile[MAX_DIR_LENGTH];   //被链接的文件

                strcpy(readfileName,readdirName);     //构造写入文件时的路径
                strcat(readfileName,"/");
                strcat(readfileName,entry->d_name);

                //printf("link:%s\n",readfileName);
                char buf[MAX_DIR_LENGTH];       //存储链接指向的文件信息
                if(readlink(readfileName,buf,sizeof(buf))<0)
                {
                    perror("failed to read symbol link.\n");
                    exit(EXIT_FAILURE);
                }
                //printf("%s\n",buf);
                strcpy(writelinkedfile,writedirName);
                strcat(writelinkedfile,"/");
                strcat(writelinkedfile,buf);
      
                //读取到链接指向文件之后，先创建对应文件再创建链接
                in=open(buf,O_RDONLY);  //打开被指向的文件
                out=open(writelinkedfile,0775);
                while((nread=read(in,block,sizeof(block)))>0)
                {
                    write(out,block,nread);
                }

                //printf("%s\n%s\n",writelinkedfile,writefileName);

                //创建完被链接文件之后创建链接
                if(symlink(buf,writefileName)<0)
                {
                    //perror("failed to create symbol link.\n");
                    //exit(EXIT_FAILURE);
                    continue;
                }
                //修改文件权限及属性信息
                timeNmode(statbuf);

            }
            else
            {
                in=open(entry->d_name,O_RDONLY);    //打开文件
                out=open(writefileName,0775);
                while((nread=read(in,block,sizeof(block)))>0)
                {
                    write(out,block,nread);
                }

                //修改文件权限及属性信息
                timeNmode(statbuf);
            }
        }
        
    }
    chdir("..");  //读完当前目录便返回上一级
    strcpy(currRdir,beforeWEntry);   //维护相对工作目录信息
    strcpy(currwRdir,beforeREntry);
    closedir(dp);   //关闭目录流
}

void timeNmode(struct stat statbuf)         //修改权限以及时间信息
{
    struct utimbuf timebuf;     //存储时间信息
    mode_t new_mode;            //存储权限信息

    timebuf.actime=statbuf.st_atime;
    timebuf.modtime=statbuf.st_mtime;
    new_mode=statbuf.st_mode;

    int modtime=utime(replacement,&timebuf);
    if(modtime==-1)
    {
        perror("failed to change time info.\n");
        exit(EXIT_FAILURE);
    }
    int modmode=chmod(replacement,new_mode);
    if(modmode==-1)
    {
        perror("failed to change mode info.\n");
        exit(EXIT_FAILURE);
    }

}
