/**
* 先进先出队列模板类，可以存储任意类型数据
* author: Xiaomu<070105083@163.com>
* ctime:2012-5-24
*
*/
#ifndef QUEUE_H
#define QUEUE_H

/**
* Queue结构体
* @base 指向存储用户数据的内存空间
* @qsize 队列数组大小
* @head 队列开始位置
* @tail 队列尾部位置
* @userDataSize 用户数据所占字节数
*/
typedef struct
{
    void *base;
    int qsize;
    int head;
    int tail;
    int userDataSize;
}Queue;


#define QUEUE_OK 0
#define QUEUE_ERROR -1

/**
* 初始队列数组大小
* 当队列内存不足时，增量大小
*/
#define DEFAULT_QUEUE_SIZE 100
#define INCREAMENT_QUEUE_SIZE 50

/**
* 创建队列
* @userDataSize 用户数据所占字节数
* @return 正确则返回队列指针，否则返回NULL
*/
Queue* createQueue(int userDataSize);
/**
* 创建队列
* @userDataSize 用户数据所占字节数
* @initSize 指定初始队列数组大小
* @return 正确则返回队列指针，否则返回NULL
*/
Queue* createQueue_InitSize(int userDataSize,int initSize);

/**
* 删除队列中尾部元素，并返回
* @q 队列指针
* @data 指向一块内存空间，用以存储返回的队列尾部数据
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int queueRemove(Queue *q,void *data);

/**
* 添加元素到队列头部
* @q 队列指针
* @data 指向存储的用户数据的指针
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int queueAdd(Queue *q,const void *data);

/**
* 判断队列是否为空
* @q 队列指针
* @return 1 for empty，0 for unempty， QUEUE_ERROR for error
*/
int queueIsEmpty(const Queue *q);
/**
* 释放队列
* @q 队列指针
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int freeQueue(Queue *q);
#endif
