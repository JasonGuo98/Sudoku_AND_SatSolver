#ifndef QUEUE_INCLUDE_H
#define QUEUE_INCLUDE_H
#include "Lit.h"
//传播队列实现

struct Queue_s {//发现使用动态队列十分消耗时间，且省去的空间也不多，使用静态队列，长度为变量数
	int head;//头，指向即将出队的元素
	int tail;//尾，指向一个新的入队元素的空的空间
	int size;//当前的长度
	int max_size;//最大长度，变量数加一
	Lit_t * base;
};

extern Queue_t * Queue_new(int max_length);//返回新的队列对象
extern Lit_t  PropQ_dequeue(Queue_t * Q);//出队列
extern void PropQ_clear(Queue_t *Q);//清空队列
extern void PropQ_insert(Queue_t * Q, Lit_t p);//入队
#endif // !QUEUE_INCLUDE_H
