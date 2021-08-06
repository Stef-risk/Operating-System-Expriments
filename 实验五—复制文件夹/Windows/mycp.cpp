// copy_folder.cpp : 递归复制文件夹以及修改文件信息
//  Ste-Made

#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<windows.h>

#define MAX_DIR_LENGTH 256

static char currRdir[MAX_DIR_LENGTH];	//存取当前的相对目录
static char beforeEntry[MAX_DIR_LENGTH];	//保存进入一个子目录之前的位置
static char writeFileName[MAX_DIR_LENGTH];	//要创建的文件名
static char writeDirectoryName[MAX_DIR_LENGTH];	//要创建的目录名
static char readFileName[MAX_DIR_LENGTH];	//要读取的文件名

void RecursionClone(char dir[]);	//递归复制文件夹
bool DirectoryCreate(char dir[]);		//创建文件夹

int main(int argc, char* argv[])
{
	char* srcdir =(char*)malloc(sizeof(char)*MAX_DIR_LENGTH);
	char* destdir= (char*)malloc(sizeof(char) * MAX_DIR_LENGTH);
	
	if (argc < 3)
	{
		printf("Usage <source directory> <destination directory>.\n");
		exit(EXIT_FAILURE);
	}
	strcpy(srcdir, argv[1]);	//对源文件夹以及目标文件夹名赋值
	strcpy(destdir, argv[2]);

	strcpy(currRdir, destdir);	//构造相对目录信息
	strcat(currRdir, "/");
	

	DirectoryCreate(destdir);

	RecursionClone(srcdir);	//核心递归复制过程
	return 0;
}

bool DirectoryCreate(char dir[])
{
	if (CreateDirectory(dir, NULL))
	{
		printf("%s has been created.\n",dir);
		return true;
	}
	printf("failed to create directory %s\n", dir);
	return false;
}

void RecursionClone(char dir[])
{

	WIN32_FIND_DATA FileData;	//存储文件相关信息
	HANDLE hSearch;			//找到的文件的句柄信息
	char fileSearch[MAX_DIR_LENGTH];	

	strcpy(fileSearch, dir);	//加入目录通配符，用于搜索第一个文件
	strcat(fileSearch, "\\*.*");

	hSearch = FindFirstFile(fileSearch, &FileData);	//返回句柄信息
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		printf("No files found.\n");
		return;
	}
	do
	{
		if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 && strcmp(FileData.cFileName, "." )!= 0 && strcmp(FileData.cFileName,".." )!= 0)
		{
			//对于目录项且不为.\..

			strcpy(writeDirectoryName, currRdir);		//创建目录
			strcat(writeDirectoryName, "/");
			strcat(writeDirectoryName, FileData.cFileName);
			DirectoryCreate(writeDirectoryName);

			strcpy(fileSearch, dir);		//编辑进入下一层文件夹的格式
			strcat(fileSearch, "/");
			strcat(fileSearch, FileData.cFileName);

			strcpy(beforeEntry, currRdir);		//维护相对目录信息
			strcat(currRdir, FileData.cFileName);
			strcat(currRdir, "/");


			RecursionClone(fileSearch);		//递归进入子文件夹
		}
		else
		{
			if (strcmp(FileData.cFileName , "." ) == 0 || strcmp(FileData.cFileName , ".." ) == 0)
			{
				continue;
			}
			//对非目录文件
			strcpy(writeFileName, currRdir);
			strcat(writeFileName, "");
			strcat(writeFileName, FileData.cFileName);

			strcpy(readFileName, dir);
			strcat(readFileName, "/");
			strcat(readFileName, FileData.cFileName);

			printf("original file: %s\n", readFileName);
			//复制文件，最后的参数设置为false表示允许覆盖已有文件
			if (!CopyFile(readFileName, writeFileName, false))
			{
				printf("Could not copy file.%d\n",GetLastError());
				
			}
			else
			{
				printf("Copy completed.\n");
			}
		}
	} while (FindNextFile(hSearch, &FileData));

	strcpy(currRdir, beforeEntry);	//返回上一层文件夹
	FindClose(hSearch);				//关闭句柄
}