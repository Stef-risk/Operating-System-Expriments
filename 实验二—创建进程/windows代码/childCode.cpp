#include<iostream>
#include<windows.h>
#include<stdlib.h>

using namespace std;

int main()
{
	SYSTEMTIME sst,est;
	float runtime_sec;
	float runtime_msec;
	DWORD start_time, end_time;
	start_time = GetTickCount();

	GetSystemTime(&sst);
	//打印子程序开始时间
	cout<<"Child process begins at "<<sst.wHour<<":"<<sst.wMinute<<":"<<sst.wSecond<<"."<<sst.wMilliseconds<<" (UTC)"<<endl;
	cout<<"Hi, my name is Stefan"<<endl;
	sleep(3);
	GetSystemTime(&est);
	//打印子程序结束时间
	cout<<"Child process ends at "<<est.wHour<<":"<<est.wMinute<<":"<<est.wSecond<<"."<<est.wMilliseconds<<" (UTC)"<<endl;
	//以毫秒为单位计算总时间
	runtime_sec=est.wSecond-sst.wSecond;
	runtime_msec=est.wMilliseconds-sst.wMilliseconds;
	/*
	if(runtime_msec<0)
	{
		runtime_msec+=1000;
		runtime_sec-=1;
	}
	*/
	end_time=GetTickCount();
	//cout<<"Child process run time "<<runtime_sec*1000+runtime_msec<<" milliseconds."<<endl;
	cout<<"Child process run time "<<(end_time-start_time)<<" milliseconds."<<endl;
	return 0;
}