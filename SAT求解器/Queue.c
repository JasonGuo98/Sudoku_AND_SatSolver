#include "Queue.h"
/*返回新的队列对象*/
Queue_t * Queue_new(int max_length) {
	Queue_t * PropQ = (Queue_t *)malloc(sizeof(Queue_t));
	PropQ->head = PropQ->tail = 0;
	PropQ->size = 0;//初始化传播队列
	PropQ->max_size = max_length;
	PropQ->base = (Lit_t *)malloc(sizeof(Lit_t)*(max_length + 1));
	return PropQ;
}

/*入队*/
void PropQ_insert(Queue_t * Q, Lit_t p) {
	Q->base[Q->tail] = p;
	Q->tail = (Q->tail + Q->max_size + 1) % Q->max_size;
	Q->size++;
}

/*清空队列*/
void PropQ_clear(Queue_t *Q) {
	//printf("cleared\n");
	Q->head = Q->tail = 0;
	Q->size = 0;
}

/*出队列*/
Lit_t  PropQ_dequeue(Queue_t * Q) {
	Lit_t p;
	p.idx_sig = 0;
	if (Q->size) {
		p = Q->base[Q->head];
		Q->head = (Q->head+Q->max_size + 1)%Q->max_size;//（max_size -1 ）+max_size + 1 = 0 刚好为第一个
		Q->size--;
	}
	return p;
}