#ifndef QUEUE_INCLUDE_H
#define QUEUE_INCLUDE_H
#include "Lit.h"
//��������ʵ��

struct Queue_s {//����ʹ�ö�̬����ʮ������ʱ�䣬��ʡȥ�Ŀռ�Ҳ���࣬ʹ�þ�̬���У�����Ϊ������
	int head;//ͷ��ָ�򼴽����ӵ�Ԫ��
	int tail;//β��ָ��һ���µ����Ԫ�صĿյĿռ�
	int size;//��ǰ�ĳ���
	int max_size;//��󳤶ȣ���������һ
	Lit_t * base;
};

extern Queue_t * Queue_new(int max_length);//�����µĶ��ж���
extern Lit_t  PropQ_dequeue(Queue_t * Q);//������
extern void PropQ_clear(Queue_t *Q);//��ն���
extern void PropQ_insert(Queue_t * Q, Lit_t p);//���
#endif // !QUEUE_INCLUDE_H
