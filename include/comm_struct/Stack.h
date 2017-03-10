/**
* function: 该头文件实现单向链表
*           该实现使用void指针和size来保存用户数据
* author: Xiaomu<070105083@163.com>
* time: 2012-5-20-morning
*
*/
#ifndef STACK_H
#define STACK_H
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#define STACK_DEFAULT_SIZE 200
#define STACK_INCREMENT_SIZE 50
//栈动态增长存在问题，当初次分配的栈空间用完后，动态增加有错

#define STACK_OK 0
#define STACK_ERROR -1

/**
* 保存栈信息的结构体
* @pArray 指向存储用户数据的数组
* @pTop 栈顶端
* @size 栈大小
* @userDataSize 保存用户数据
*/
typedef struct Stack
{
    void *pArray;
    void *pTop;
    int size;
    int userDataSize;
}Stack;

/**
* 初始化栈结构
* @s 需要初始化的指针
* @userDataSize 用户数据字节数(>0)
* @return STACK_OK for ok， STACK_ERROR for error
*/
int initStack(Stack *s,int userDataSize);

/**
* 将新元素压入栈
* @s 栈指针
* @node 新元素
* @return STACK_OK for ok， STACK_ERROR for error
*/
int pushStack(Stack *s, const void *node);

/**
* 将栈顶元素弹出栈, 并返回
* @s 栈指针
* @node 指向一块内存区域(大小为userDataSize)，存储栈顶元素
* @return STACK_OK for ok, STACK_ERROR for error
*/
int popStack(Stack *s, void *node);

/**
* 检测栈是否为空
* @s 栈指针
@ @return 1 for empty, 0 for unempty, STACK_ERROR for error
*/
int isStackEmpty(const Stack *s);

/**
* 施放栈资源
* @s 栈指针
* @return STACK_OK for ok, STACK_ERROR for error
*/
int freeStack(Stack *s);


int GetTopStack(Stack *s, void *node);

/////////////////////////////////////////////////////////////////////




#endif /*STACK_H*/
