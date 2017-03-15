/**
* function: 该头文件实现单向链表
*           在该链表中使用void指针和size来确定一个内存对象，而不管其实际类型
* author: Xiaomu<070105083@163.com>
* time: 2012-5-14
*
*/
#ifndef LIST_H
#define LIST_H
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "MachineBit.h"
#include "Stack.h"



#define LIST_OK 0
#define LIST_ERROR -1
//typedef unsigned long long uint64_t;
typedef struct logData{
    char type;
    char* processName;
    uint64_t curAddr;
    uint64_t goAddr;
    uint64_t pid;
    uint64_t ppid;
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




// get current thread
int GetCurThread(const List *head,uint64_t pid,uint64_t tid ,void *e);

//index of element
int IndexOf(const List *head,int element);

//index of element
int IndexOfStr(const List *head,char * element);

int lengthOfList(const List *head);

/////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif /*LIST_H*/
