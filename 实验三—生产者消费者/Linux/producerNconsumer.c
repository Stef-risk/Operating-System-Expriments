#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>

#define BUFFER_SIZE 4
#define PRODUCER_AMOUNT 3       //生产者和消费者的数量
#define CONSUMER_AMOUNT 4

char nameset[4]={'S','Z','H','F'};  //定义生产者每次向缓冲区中加入字符的集合

static void setSemValue(int sem_id,int val);    //定义需要用到的对信号量的赋值操作与PV操作
static int P(int sem_id);
static int V(int sem_id);
static int delSem(int sem_id);

static int mutex;       //互斥访问信号量以及写满，空余信号量
static int fillcount;
static int emptycount;

union semun     //用在semctl赋值时的联合结构
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };

struct buffet{
    char for_name[BUFFER_SIZE][2];
    int written_by_me;
};

static struct buffet *hereBuffer;   //指向缓冲区的指针

static int shmid;   //共享内存区的id
void *shared_memory = (void *)0;

static int traceP=0;
static int traceS=0;  //用于跟踪上次存放的缓冲区位置，以实现先生产先消费

void buffer_set(void);  //初始化缓冲区
void putItemIntoBuffer(char word);  //生产者将字符写入
char produceItem(void);     //生产物品
char removeItemFromBuffer(void);   //消费者将字符从缓冲区取出
void showProducerStatus(char item);      //打印当前缓冲区信息
void showConsumerStatus(char item);
//定义生产者进程
void producer()
{
    for(int i=0;i<4;i++)  //重复指定的次数
    {   
        srand((unsigned int)getgid()); // 初始化随机数
        sleep(rand()%3);    //等待一段三秒以内的时间
        char item;
        item=produceItem();
        P(emptycount);
        P(mutex);
        putItemIntoBuffer(item);
        showProducerStatus(item);
        V(mutex);
        //printf("release mutex.\n");
        V(fillcount);
        //printf("release fillcount.\n");
    }
}
//定义消费者进程
void consumer()
{
    for(int i=0;i<3;i++)  //重复指定的次数
    { 
        srand((unsigned int)getpid()); // 初始化随机数
        sleep(rand()%4);
        char item;
        P(fillcount);
        P(mutex);
        item=removeItemFromBuffer();
        showConsumerStatus(item);
        V(mutex);
        //printf("release mutex.\n");
        V(emptycount);  
        //printf("release emptycount.\n");
    }
}

int main()
{
    //获取信号量标识符
    mutex=semget((key_t)6666,1,0666|IPC_CREAT);
    emptycount=semget((key_t)6789,1,0666|IPC_CREAT);
    fillcount=semget((key_t)6999,1,0666|IPC_CREAT);
    //先初始化信号量
    setSemValue(mutex,1);
    setSemValue(fillcount,0);
    setSemValue(emptycount,BUFFER_SIZE);

    pid_t processID;
 
    processID=fork();   //创建子进程
    printf("Now begin the simulation of Producer and Consumer...\n");

    switch (processID)
    {
    case -1:
        fprintf(stderr,"Failed to create process.\n");
        exit(EXIT_FAILURE);
    case 0:
         //由消费者进程创建共享内存区
        shmid=shmget((key_t)1234,sizeof(struct buffet),0666|IPC_CREAT); //获取共享区信号量标识符
        if(shmid==-1){
            fprintf(stderr,"shmget failed.\n");
            exit(EXIT_FAILURE);
        }

        shared_memory=shmat(shmid,(void*)0,0);  //返回一个指向共享内存第一个字节的指针
        if(shared_memory==(void*)-1)
        {
            fprintf(stderr,"shmat failed.\n");
            exit(EXIT_FAILURE);
        }
        //printf("memory attached at %X\n",(int)shared_memory);

        hereBuffer=(struct buffet*)shared_memory;   //将sharedmemory分配给hereBuffer
        buffer_set();
        mutex=semget((key_t)6666,1,0666|IPC_CREAT);
        emptycount=semget((key_t)6789,1,0666|IPC_CREAT);
        fillcount=semget((key_t)6999,1,0666|IPC_CREAT);
        for(int i=0;i<CONSUMER_AMOUNT;i++)
            consumer();     //父进程运行消费者进程

        //分离共享内存并删除
        if(shmdt(shared_memory)==-1)
        {
            fprintf(stderr,"shmdt failed.\n");
            exit(EXIT_FAILURE);
        }
        if(shmctl(shmid,IPC_RMID,0)==-1)
        {
            fprintf(stderr,"shmctl(IPC_RMID ) failed.\n");
            exit(EXIT_FAILURE);
        }
        break;
    default:
    //先进行共享内存的连接
        shmid=shmget((key_t)1234,sizeof(struct buffet),0666|IPC_CREAT);
        if(shmid==-1)
        {
            fprintf(stderr,"failed to shmget.\n");
            exit(EXIT_FAILURE);
        }
        shared_memory=shmat(shmid,(void*)0,0);
        if(shared_memory==(void*)-1)
        {
            fprintf(stderr,"failed to shmat.\n");
            exit(EXIT_FAILURE);
        }
        hereBuffer=(struct buffet *)shared_memory;

        mutex=semget((key_t)6666,1,0666|IPC_CREAT);     //获取信号量标识符
        emptycount=semget((key_t)6789,1,0666|IPC_CREAT);
        fillcount=semget((key_t)6999,1,0666|IPC_CREAT);

        for(int i=0;i<PRODUCER_AMOUNT;i++)
            producer();     //子进程运行生产者进程

        if(shmdt(shared_memory)==-1)        //与共享内存区分离
        {
            fprintf(stderr,"failed to shmdt.\n");
            exit(EXIT_FAILURE);
        }
        break;
       
    }

    if(processID>0)
    {
        pid_t child_pid;
        int stat_val;   //存储状态信息

        child_pid=wait(&stat_val);
        printf("\nThe child process has ended, it's pid is %d\n",child_pid);
        if(WIFEXITED(stat_val))     //如果正常退出打印状态码
            printf("child exited with code %d\n",WEXITSTATUS(stat_val));
        else        //对非正常退出
            printf("child terminated abnormally.\n");
        printf("the simulation ends.\n");
        }

    exit(EXIT_SUCCESS);
}

static void setSemValue(int sem_id,int value)   //设置信号量的值
{
    union semun sen_b;
    sen_b.val=value;
    semctl(sem_id,0,SETVAL,sen_b);
}

static int P(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=-1;        //减1操作
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)==-1)
    {
        fprintf(stderr,"Failed to P.\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}
static int V(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=1;     //加1操作
    sem_b.sem_flg=SEM_UNDO;
    if(semop(sem_id,&sem_b,1)==-1)
    {
        fprintf(stderr,"Failed to P.\n");
        exit(EXIT_FAILURE);
    }
    return 1;
}

static int delSem(int sem_id)   //将信号量标识符删除
{
    union semun sen_b;
    if(semctl(sem_id,0,IPC_RMID,sen_b)==-1)
        fprintf(stderr,"Failed to delete sid.\n");
    return 1;
}


void putItemIntoBuffer(char word) //生产者将字符写入
{
    for(int i=0;i<BUFFER_SIZE;i++)
    {
        if(hereBuffer->for_name[i][0]=='#')
        {
            hereBuffer->for_name[i][0]=word;
            hereBuffer->for_name[i][1]=traceS++; //使用written_by_me记录填入次序
            break;
        }
    }
}
char produceItem(void)   //生产物品
{
    srand((unsigned int)time(NULL));
    char word;
    word=nameset[rand()%4];
    return word;
}
char removeItemFromBuffer(void)   //消费者将字符从缓冲区取出
{
    char takeword;

    for(int i=0;i<BUFFER_SIZE;i++)
    {
        if(hereBuffer->for_name[i][0]=='#')
        {
            continue;
        }
        else
        {
            if(hereBuffer->for_name[i][1]==traceP)
            {
                takeword=hereBuffer->for_name[i][0];
                hereBuffer->for_name[i][0]='#';
                traceP++;
                break;
            }
            else
            {
                continue;
            }
        }
    }
    return takeword;
}
void showProducerStatus(char item)      //打印当前缓冲区信息
{
    printf("生产者本次生产：\t%c.\n",item);
    printf("当前缓冲区：    \t%c\t%c\t%c\t%c.\n",hereBuffer->for_name[0][0],hereBuffer->for_name[1][0],hereBuffer->for_name[2][0],hereBuffer->for_name[3][0]);
}
void showConsumerStatus(char item)
{
    printf("消费者本次消费：\t%c.\n",item);
    printf("当前缓冲区：    \t%c\t%c\t%c\t%c.\n",hereBuffer->for_name[0][0],hereBuffer->for_name[1][0],hereBuffer->for_name[2][0],hereBuffer->for_name[3][0]);
}

void buffer_set(void)
{
    for(int i=0;i<BUFFER_SIZE;i++)
    {
        hereBuffer->for_name[i][0]='#';
        hereBuffer->for_name[i][1]=0;
    }
}