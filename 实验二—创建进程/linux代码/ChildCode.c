#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include<time.h>

#define MILLION 1000000

int main()
{
    struct timeval stv,etv;
    struct tm* ptm;
    char SteTime[39];
    long  millesec;
    double milletime; //以毫秒为单位计算耗时

    
    gettimeofday(&stv,NULL); //仅仅获取时间即可，不需要时区
    ptm=localtime(&stv.tv_sec); //将获得的时间转换为struct tm
    strftime(SteTime,sizeof(SteTime),"%Y-%m-%d %H:%M:%S",ptm);
    millesec=stv.tv_usec*1000;//毫秒计算
    printf("%s.%04ld\n",SteTime,millesec);

    //printf("你好，我的名字是张皓\n");
    printf("Hi, my name is 张皓\n");
    sleep(3);   //暂停三秒

    printf("\nChild process ends at ");       //结束时间
    gettimeofday(&etv,NULL); //仅仅获取时间即可，不需要时区
    ptm=localtime(&etv.tv_sec); //将获得的时间转换为struct tm
    strftime(SteTime,sizeof(SteTime),"%Y-%m-%d %H:%M:%S",ptm);
    millesec=etv.tv_usec*1000;//毫秒计算
    printf("%s.%04ld\n",SteTime,millesec);

    
    milletime=MILLION*(etv.tv_sec-stv.tv_sec)+etv.tv_usec-stv.tv_usec;
    printf("Time used : %.3lf milleseconds\n",milletime/1000);      //以毫秒为单位打印耗时


}