#include "Stack.h"

#include <string.h>

/**
* 初始化栈结构
* @s 需要初始化的指针
* @userDataSize 用户数据字节数
* @return STACK_OK for ok， STACK_ERROR for error
*/
int initStack(Stack *s,int userDataSize)
{
    if(s==NULL || userDataSize<=0)
        return(STACK_ERROR);

    s->pArray=malloc(STACK_DEFAULT_SIZE*userDataSize);
    if(s->pArray==NULL)
        return(STACK_ERROR);
    s->pTop=s->pArray;
    s->size=STACK_DEFAULT_SIZE;
    s->userDataSize=userDataSize;
    return(STACK_OK);
}
/**
* 将新元素压入栈
* @s 栈指针
* @node 新元素
* @return STACK_OK for ok， STACK_ERROR for error
*/
int pushStack(Stack *s, const void *node)
{
    if(s==NULL || s->pArray==NULL || node==NULL)
        return(STACK_ERROR);

    if((s->pTop - s->pArray)>=s->size*s->userDataSize)
    {
        s->pArray=realloc(s->pArray,(s->size+STACK_INCREMENT_SIZE)*s->userDataSize);
        if(s->pArray==NULL)
            return(STACK_ERROR);
        s->pTop=s->pArray + s->size*s->userDataSize;
    }
    if(NULL==memcpy(s->pTop,node,s->userDataSize))
    {
        return(STACK_ERROR);
    }
    s->pTop+=s->userDataSize;
    return(STACK_OK);
}
/**
* 将栈顶元素弹出栈, 并返回
* @s 栈指针
* @node 指向一块内存区域(大小为userDataSize)，存储栈顶元素
* @return STACK_OK for ok, STACK_ERROR for error
*/
int popStack(Stack *s, void *node)
{
    if(s==NULL || s->pArray==NULL || node==NULL)
        return(STACK_ERROR);

    if(s->pArray==s->pTop)
        return(STACK_ERROR);

    s->pTop-=s->userDataSize;
    if(NULL==memcpy(node,s->pTop,s->userDataSize))
        return(STACK_ERROR);
    return(STACK_OK);
}

int GetTopStack(Stack *s, void *node)
{
    if(s==NULL || s->pArray==NULL || node==NULL)
        return(STACK_ERROR);

    if(s->pArray==s->pTop)
        return(STACK_ERROR);
	void * p = s->pTop - s->userDataSize;
    if(NULL==memcpy(node,p,s->userDataSize))
        return(STACK_ERROR);
    return(STACK_OK);
}

/**
* 检测栈是否为空
* @s 栈指针
@ @return 1 for empty, 0 for unempty, STACK_ERROR for error
*/
int isStackEmpty(const Stack *s)
{
    if(s==NULL || s->pArray==NULL)
        return(STACK_ERROR);

    return (s->pArray==s->pTop)? 1:0;
}
/**
* 施放栈资源
* @s 栈指针
* @return STACK_OK for ok, STACK_ERROR for error
*/
int freeStack(Stack *s)
{
    if(s==NULL)
        return(STACK_ERROR);

    free(s->pArray);
    return(STACK_OK);
}
