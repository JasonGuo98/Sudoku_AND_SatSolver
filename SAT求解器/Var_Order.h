#pragma once


#ifndef VARORDER_INCLUDE_H
#define VARORDER_INCLUDE_H
#include "Basic_def.h"
#include "Solver.h"


//����ṹ���б�Ҫ�ģ�����ֱ������solver�е����ݾ͹���
//˳��Ҫ���� �����������
//���߽�activityд�������������Solver��

 struct Var_Order_s {
	int * sorted_var;//���������飬���ջ�Ծ�ȵݼ�˳�����б���
	int * order_for_var;//˳������飬oredr_for_var[i]�����ر���i��˳��
	unsigned int var_num;//������
};

extern Var_Order_t * new_Var_Order(int var_num);//�����µ�var_order����ĺ���
extern void Var_Order_updata(Solver_t * S, Var_Order_t * order, int varr);//һ�������Ļ�Ծ�ȸı�ʱʹ�ã����Ըı�˳��
extern void Var_Order_updataAll(Solver_t * S, Var_Order_t * order);//��������˳��
extern void Var_Order_undo(Solver_t * S, Var_Order_t * order, int var);//���������unbound��ʱ��ʹ��
extern int Var_Order_select(Solver_t * S, Var_Order_t * order);//ѡ��һ���µ�û�б�ָ�ɵı���

#endif // !VARORDER_INCLUDE_H
