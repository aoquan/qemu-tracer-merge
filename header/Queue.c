#include "Queue.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/**
* 创建队列
* @userDataSize 用户数据所占字节数
* @return 正确则返回队列指针，否则返回NULL
*/
Queue* createQueue(int userDataSize)
{
    return createQueue_InitSize(userDataSize,DEFAULT_QUEUE_SIZE);
}
/**
* 创建队列
* @userDataSize 用户数据所占字节数
* @initSize 指定初始队列数组大小
* @return 正确则返回队列指针，否则返回NULL
*/
Queue* createQueue_InitSize(int userDataSize,int initSize)
{
    if(userDataSize<=0 || initSize<=0)
        return(NULL);
    Queue *q=(Queue*)malloc(sizeof(Queue));
    if(q==NULL)
        return(NULL);

    q->base=malloc(userDataSize*initSize);
    if(q->base==NULL)
    {
        free(q);
        return(NULL);
    }
    q->qsize=initSize;
    q->head=0;
    q->tail=0;
    q->userDataSize=userDataSize;
    return(q);
}

/**
* 删除队列中尾部元素，并返回
* @q 队列指针
* @data 指向一块内存空间，用以存储返回的队列尾部数据，如果为NULL，则不返回
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int queueRemove(Queue *q,void *data)
{
    if(q==NULL || q->base==NULL)
        return(QUEUE_ERROR);

    if(queueIsEmpty(q))
    {
        return(QUEUE_ERROR);
    }
    if(NULL==memcpy(data,q->base+q->tail*q->userDataSize,q->userDataSize))
        return(QUEUE_ERROR);

    q->tail=(q->tail+1)%q->qsize;
    return(QUEUE_OK);
}

/**
* 内部函数，判断队列是否满
*/
static int queueFulled(const Queue *q)
{
    if((q->head+1)%q->qsize==q->tail)
        return(1);
    return(0);
}
/**
* 添加元素到队列头部
* @q 队列指针
* @data 指向存储的用户数据的指针
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int queueAdd(Queue *q,const void *data)
{
    if(q==NULL || q->base==NULL || data==NULL)
        return(QUEUE_ERROR);
    if(queueFulled(q))
    {
//        puts("in queue full");
        //队列满
        void *newBase=malloc((q->qsize+INCREAMENT_QUEUE_SIZE)*q->userDataSize);
        void *cur=newBase;
        if(newBase==NULL)
            return(QUEUE_ERROR);
        //将原队列中内容拷贝至新内存区域
        while(!queueIsEmpty(q))
        {
            queueRemove(q,cur);
            cur+=q->userDataSize;
        }
        free(q->base);
        q->qsize+=INCREAMENT_QUEUE_SIZE;
        q->base=newBase;
        q->tail=0;
        q->head=(cur-newBase)/q->userDataSize;
    }
    if(NULL==memcpy(q->base+q->head*q->userDataSize,data,q->userDataSize))
        return(QUEUE_ERROR);
    q->head=(q->head+1)%q->qsize;
    return(QUEUE_OK);
}

/**
* 判断队列是否为空
* @q 队列指针
* @return 1 for empty，0 for unempty， QUEUE_ERROR for error
*/
int queueIsEmpty(const Queue *q)
{
    if(q==NULL || q->base==NULL)
        return(QUEUE_ERROR);

    if(q->tail==q->head)
    {
        //空
        return(1);
    }
    return(0);
}
/**
* 释放队列
* @q 队列指针
* @return QUEUE_OK for ok, QUEUE_ERROR for error
*/
int freeQueue(Queue *q)
{
    if(q==NULL)
        return(QUEUE_ERROR);
    free(q->base);
    free(q);
    return(QUEUE_OK);
}
