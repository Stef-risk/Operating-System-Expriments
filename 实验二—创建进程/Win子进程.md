```c++
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
	cout<<"Child process begins at "<<sst.wHour<<":"<<sst.wMinute<<":"<<sst.wSecond<<"."<<sst.wMilliseconds<<endl;
	cout<<"Hi, my name is Stefan"<<endl;
	sleep(3);
	GetSystemTime(&est);
	//打印子程序结束时间
	cout<<"Child process ends at "<<est.wHour<<":"<<est.wMinute<<":"<<est.wSecond<<"."<<est.wMilliseconds<<endl;
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
```

子进程除了完成输出的工作，还要显示开始时间、结束时间以及运行所花费的总时间。

本次实验中使用GetSystemTime（）来获取时间信息。

GetSystemTime（）函数获取到的是UTC时间，即世界协调时间。该函数定义如下：

```c++
void GetSystemTime(
  LPSYSTEMTIME lpSystemTime
);
```

输入的参数为一个指向SYSTEMTIME结构的指针，SYSTEMTIME的结构如下：

```c++
typedef struct _SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
```

结构中每个成员均为单字类型，其名称都明了的显示出其所代表的意义。

在本程序中，定义了SYSTEMTIME类型变量sst和est，输出有关小时，分钟，秒以及毫秒的相关信息如下：

```c++
cout<<"Child process begins at "<<sst.wHour<<":"<<sst.wMinute<<":"<<sst.wSecond<<"."<<sst.wMilliseconds<<endl;
```



![image-20201218134230288](C:\Users\Stefanny\AppData\Roaming\Typora\typora-user-images\image-20201218134230288.png)



而为了方便计算程序的运行时间,程序使用了GetTickCount()函数，其返回一个双字类型的值存储从操作系统启动所经过的毫秒数。

```
函数原型：
DWORD GetTickCount(void);
头文件：
C/C++头文件：winbase.h
```

![image-20201218134424324](C:\Users\Stefanny\AppData\Roaming\Typora\typora-user-images\image-20201218134424324.png)

