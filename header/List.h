/**
* function: 该头文件实现单向链表
*           在该链表中使用void指针和size来确定一个内存对象，而不管其实际类型
* author: Xiaomu<070105083@163.com>
* time: 2012-5-14
*
*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "MachineBit.h"
#ifndef LIST_H
#define LIST_H



#define LIST_OK 0
#define LIST_ERROR -1
//typedef unsigned long long uint64_t;
typedef struct logData{
    char type;
    char* processName;
    uint64_t curAddr;
    uint64_t goAddr;
    uint64_t pid;
    int tid;
    uint64_t esp;
}logData;

typedef struct threadList{
	Stack* stack;
	uint64_t tid;
	uint64_t pid;
//	int num; 
}threadList;



/**
* 遍历链表的回调函数指针
* @e 指向链表中每一个用户对象的指针
* @param 其它参数
* @return 1 正常终止遍历，-1 错误终止，否则继续遍历
*/
typedef int (*TRAVERSEFUNC)(const void *e,void *param);

/**
* 使用者不应该使用该类型
*/
struct __ListElement
{
    void *data;//存储用户数据
    struct __ListElement *next;//指向下一元素
};

/**
* 链表头结点(用户仅需要关注头结点)
* @userDataSize 链表中存储的用户数据字节数
* @next 指向链表中第一个元素
*/
typedef struct
{
    int userDataSize;
    struct __ListElement *next;
}List;


/**
* 初始化链表
* @head 链表指针
* @userDataSize 用户数据大小
* @return LIST_OK for ok, LIST_ERROR for error
*/
int initList(List *head,int userDataSize);

/**
* 在位置pos处插入元素
* @head 链表指针
* @e 指向新元素指针，该方法会创建新对象保存此指针指向的用户数据
* @pos 元素插入位置(0<=pos<=List.length)
* @return LIST_OK for ok, LIST_ERROR for error
*/
int insertList(List *head,const void *e,int pos);

/**
* 在链表结尾插入新元素
* @head 链表指针
* @e 指向新元素指针，该方法会创建新对象保存此指针指向的用户数据
* @return LIST_OK for ok, LIST_ERROR for error
*/
int appendList(List *head,const void *e);

/**
* 从链表中删除pos位置(zero-based)的元素，将元素赋值给 "e"
* @head 链表指针
* @pos 待删除元素位置(0<=pos<=List.length-1)
* @e 指向一块内存区域(大小为List.userDataSize)保存用户数据
* @return LIST_OK for ok, LIST_ERROR for error
*/
int deleteList(List *head, int pos, void *e);

/**
* 从链表头部遍历链表，对每一个用户数据对象调用func函数
* @head 链表指针
* @func 函数指针，函数类型 "int (*TRAVERSEFUNC)(const void *e,void *param)"
* @param 回调函数其他参数
* @return LIST_OK for ok, LIST_ERROR for error
*/
int traverseList(const List *head,TRAVERSEFUNC func,void *param);

/**
* 测试链表是否为空
* @head 链表指针
* @return 1 for empty, 0 for unempty, LIST_ERROR for error
*/
int isListEmpty(const List *head);

/**
* 施放链表使用内存, 销毁链表
* @head 链表指针
* @return LIST_OK for ok, LIST_ERROR for error
*/
int freeList(List *head);


#endif /*LIST_H*/


// get current thread
int GetCurThread(const List *head,uint64_t pid,uint64_t tid ,void *e);

//index of element
int IndexOf(const List *head,int element);



/////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
* 初始化链表
* @head 链表指针
* @userDataSize 用户数据大小
* @return LIST_OK for ok, LIST_ERROR for error
*/
int initList(List *head,int userDataSize)
{
    if(head==NULL || userDataSize<=0)
        return(LIST_ERROR);

    head->userDataSize=userDataSize;
    head->next=NULL;

    return LIST_OK;
}
/**
* 在位置pos处插入元素
* @head 链表指针
* @e 指向新元素指针，该方法会创建新对象保存此指针指向的用户数据
* @pos 元素插入位置(0<=pos<=List.length)
* @return LIST_OK for ok, LIST_ERROR for error
*/
int insertList(List *head,const void *e,int pos)
{
    if(head==NULL || e==NULL)
        return(LIST_ERROR);
    if(pos<0 || (pos>1 && head->next==NULL))
        return(LIST_ERROR);

    if(pos==0)
    {
        //在链表头部插入
        //申请内存空间拷贝用户数据，以及新的链表对象
        struct __ListElement *newElement=(struct __ListElement*)malloc(sizeof(struct __ListElement));
        if(newElement==NULL)
            return(LIST_ERROR);
        newElement->data=malloc(head->userDataSize);
        if(newElement->data==NULL)
        {
            free(newElement);
            return(LIST_ERROR);
        }
        if(NULL==memcpy(newElement->data,e,head->userDataSize))
        {
            free(newElement->data);
            free(newElement);
            return(LIST_ERROR);
        }

        newElement->next=head->next;
        head->next=newElement;
    }
    else
    {
        struct __ListElement *pre=head->next, *cur=head->next->next;
        int i=1;
        while(i<pos && cur!=NULL)
        {
            pre=cur;
            cur=cur->next;
            ++i;
        }
        if(i<pos)
            return(LIST_ERROR);

        //其他位置插入
        //申请内存空间拷贝用户数据，以及新的链表对象
        struct __ListElement *newElement=(struct __ListElement*)malloc(sizeof(struct __ListElement));
        if(newElement==NULL)
            return(LIST_ERROR);
        newElement->data=malloc(head->userDataSize);
        if(newElement->data==NULL)
        {
            free(newElement);
            return(LIST_ERROR);
        }
        if(NULL==memcpy(newElement->data,e,head->userDataSize))
        {
            free(newElement->data);
            free(newElement);
            return(LIST_ERROR);
        }

        newElement->next=cur;
        pre->next=newElement;
    }

    return(LIST_OK);
}
/**
* 在链表结尾插入新元素
* @head 链表指针
* @e 指向新元素指针，该方法会创建新对象保存此指针指向的用户数据
* @return LIST_OK for ok, LIST_ERROR for error
*/
int appendList(List *head,const void *e)
{
    if(head==NULL || e==NULL)
        return(LIST_ERROR);

    //其他位置插入
    //申请内存空间拷贝用户数据，以及新的链表对象
    struct __ListElement *newElement=(struct __ListElement*)malloc(sizeof(struct __ListElement));
    if(newElement==NULL)
        return(LIST_ERROR);
    newElement->data=malloc(head->userDataSize);
    if(newElement->data==NULL)
    {
        free(newElement);
        return(LIST_ERROR);
    }
    if(NULL==memcpy(newElement->data,e,head->userDataSize))
    {
        free(newElement->data);
        free(newElement);
        return(LIST_ERROR);
    }

    if(head->next==NULL)
    {
        newElement->next=NULL;
        head->next=newElement;
    }
    else
    {
        struct __ListElement *cur=head->next;
        while(cur->next!=NULL)
            cur=cur->next;
        newElement->next=cur->next;
        cur->next=newElement;
    }

    return(LIST_OK);
}
/**
* 从链表中删除pos位置(zero-based)的元素，将元素赋值给 "e"
* @head 链表指针
* @pos 待删除元素位置(0<=pos<=List.length-1)
* @e 指向一块内存区域(大小为List.userDataSize)保存用户数据
* @return LIST_OK for ok, LIST_ERROR for error
*/
int deleteList(List *head, int pos, void *e)
{
    if(head==NULL || head->next==NULL || e==NULL)
        return(LIST_ERROR);
    if(pos<0)
        return(LIST_ERROR);

    if(pos==0)
    {
        struct __ListElement *del=head->next;
        if(NULL==memcpy(e,del->data,head->userDataSize))
            return(LIST_ERROR);

        head->next=del->next;
        free(del);
    }
    else
    {
        struct __ListElement *pre=head->next,*cur=head->next->next;
        register int i=1;
        while(i<pos && cur!=NULL)
        {
            pre=cur;
            cur=cur->next;
            ++i;
        }
        if(i<pos)
            return(LIST_ERROR);

        if(NULL==memcpy(e,cur,head->userDataSize))
            return(LIST_ERROR);

        pre->next=cur->next;
        free(cur);
    }

    return(LIST_OK);
}
/**
* 从链表头部遍历链表，对每一个用户数据对象调用func函数
* @head 链表指针
* @func 函数指针，函数类型 "int (*TRAVERSEFUNC)(const void *e,void *param)"
* @param 回调函数其他参数
* @return LIST_OK for ok, LIST_ERROR for error
*/
int traverseList(const List *head,TRAVERSEFUNC func,void *param)
{
    if(head==NULL || func==NULL)
        return(LIST_ERROR);

    struct __ListElement *cur=head->next;
    register int res;
    while(cur!=NULL)
    {
        res=func(cur->data,param);
        if(1==res)
            break;
        else if(-1==res)
            return(LIST_ERROR);

        cur=cur->next;
    }
    return(LIST_OK);
}
/**
* 测试链表是否为空
* @head 链表指针
* @return 1 for empty, 0 for unempty, LIST_ERROR for error
*/
int isListEmpty(const List *head)
{
    if(head==NULL)
        return(LIST_ERROR);
    return (head->next==NULL)? 1:0;
}
/**
* 施放链表使用内存, 销毁链表
* @head 链表指针
* @return LIST_OK for ok, LIST_ERROR for error
*/
int freeList(List *head)
{
    if(head==NULL)
        return(LIST_ERROR);

    struct __ListElement *cur=head->next,*pre;
    while(cur!=NULL)
    {
        pre=cur;
        cur=cur->next;
        free(pre->data);
        free(pre);
    }
    return(LIST_OK);
}


// get current thread
int GetCurThread(const List *head,uint64_t pid,uint64_t tid ,void *e)
{
    if(head==NULL || e==NULL)
        return(LIST_ERROR);

    struct __ListElement *cur=head->next;
    while(cur!=NULL)
    {
        
        if(((threadList*)(cur->data))->tid == tid &&((threadList*)(cur->data))->pid == pid ){
            memcpy(e,cur->data,head->userDataSize);
            return (LIST_OK);
        }
        cur=cur->next;
    }
    return(LIST_ERROR);
}

// index of elemnet
int IndexOf(const List *head,int element){
    if(head==NULL)
        return(-1);

    struct __ListElement *cur=head->next;
    int count = 0;
    while(cur!=NULL)
    {
        //if((*(my_target_ulong*)cur->data)==element){
        if((*(int*)cur->data)==element){
            return count; 
        }
        cur=cur->next;
        count ++;
    }
    return(-1);
}

