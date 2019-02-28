#pragma once
#include "Var_Order.h"
/*�����µ�var_order����ĺ���*/
Var_Order_t * new_Var_Order(int var_num) {
	Var_Order_t * order = (Var_Order_t *)malloc(sizeof(Var_Order_t));
	order->order_for_var = (int *)malloc(sizeof(int)*(var_num + 1));
	order->sorted_var = (int *)malloc(sizeof(int)*(var_num + 1));
	order->var_num = var_num;
	unsigned int i;
	for (i = 0; i <= order->var_num; i++) {
		order->sorted_var[i] = i;
		order->order_for_var[i] = i;
	}
	return order;
}

/*�����Ļ�Ծ�ȸı�ʱʹ��*/
void Var_Order_updata(Solver_t * S,Var_Order_t * order, int var) {
	//ʹ�ò�������,�Ӵ�С����
	double act = S->var_activity[var];
	unsigned int index = order->order_for_var[var];
	while (index < order->var_num&&act < S->var_activity[order->sorted_var[index+1]])
	{//����ƶ�
		order->sorted_var[index] = order->sorted_var[index + 1];
		order->order_for_var[order->sorted_var[index]] = index;
		index++;
	}
	while (index >1 && act > S->var_activity[order->sorted_var[index - 1]])
	{//������ǰ�ƶ�
		order->sorted_var[index] = order->sorted_var[index - 1];
		order->order_for_var[order->sorted_var[index]] = index;
		index--;
	}


	order->sorted_var[index] = var;
	order->order_for_var[var] = index;//��ԭ����д��ȥ
}

void Var_Order_updataAll(Solver_t * S, Var_Order_t * order) {
	//�Ӵ�С
	unsigned int i,j, flag ,left,right,var_num = order->var_num;
	double temp_act = 0;
	double * var_activity = S->var_activity;
	int * sorted_var = order->sorted_var, *order_for_var = order->order_for_var;

	/*���¼���
	for (i = 1; i <= var_num; i++) {
		S->polarity[i] = S->watches[i * 2].length > S->watches[i * 2 - 1].length ? TRUE : FALSE;
	}
	*/

	for (i = 1; i <= var_num; i++) {

		flag = 0;
		for (j = 1; j <= var_num - i ; j++) {
			left = sorted_var[j], right = sorted_var[j + 1];
			if (var_activity[left] < var_activity[right]) {//����
				flag = 1;
				sorted_var[j] = right;
				sorted_var[j+1] = left;
			}
		}
		if (!flag) break;
	}
	for (i = 1; i <= var_num; i++) {//�������Ż��ռ�
		order_for_var[sorted_var[i]] = i;
	}
}

void Var_Order_undo(Solver_t * S, Var_Order_t * order, int var) {
	return;
	S->var_activity[var]  /= 10;
	Var_Order_updata(S, order, var);
	//do what?
	//��С������act�����ң������ƺ�ֻ�ǽ������ĳ�unbound
	//��ô�������û�����鷢����
	;
}

int Var_Order_select(Solver_t * S, Var_Order_t * order) {
	unsigned i;
#ifdef PRINT_INFO
	if (S->var_inc > 1e20)
		for (i = 1; i <= order->var_num; i++) {
			printf("%f\n", S->var_activity[order->sorted_var[i]]);
		}
#endif // PRINT_INFO	
	//Var_Order_updataAll(S, order);
	int var;
	for (i = 1; i <= order->var_num; i++) {
		var = order->sorted_var[i];
		if (S->assigns[var] == LUndef)
			return var;
	}
	/*
	//��Ϊѡ������Ż������Ż�
	if (random(2) && Solver_nAssigns(S) < Solver_nVars(S) / 2) {
		for (i++; i <= order->var_num; i++) {
			var = order->sorted_var[i];
			if (S->assigns[var] == LUndef)
				return var;
		}
	}
	else
		return var;
		*/
	return 0;
}