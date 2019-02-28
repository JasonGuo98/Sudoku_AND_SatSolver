#pragma once
#include "Var_Order.h"
/*创建新的var_order对象的函数*/
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

/*变量的活跃度改变时使用*/
void Var_Order_updata(Solver_t * S,Var_Order_t * order, int var) {
	//使用插入排序,从大到小排序
	double act = S->var_activity[var];
	unsigned int index = order->order_for_var[var];
	while (index < order->var_num&&act < S->var_activity[order->sorted_var[index+1]])
	{//向后移动
		order->sorted_var[index] = order->sorted_var[index + 1];
		order->order_for_var[order->sorted_var[index]] = index;
		index++;
	}
	while (index >1 && act > S->var_activity[order->sorted_var[index - 1]])
	{//或者向前移动
		order->sorted_var[index] = order->sorted_var[index - 1];
		order->order_for_var[order->sorted_var[index]] = index;
		index--;
	}


	order->sorted_var[index] = var;
	order->order_for_var[var] = index;//将原变量写回去
}

void Var_Order_updataAll(Solver_t * S, Var_Order_t * order) {
	//从大到小
	unsigned int i,j, flag ,left,right,var_num = order->var_num;
	double temp_act = 0;
	double * var_activity = S->var_activity;
	int * sorted_var = order->sorted_var, *order_for_var = order->order_for_var;

	/*更新极性
	for (i = 1; i <= var_num; i++) {
		S->polarity[i] = S->watches[i * 2].length > S->watches[i * 2 - 1].length ? TRUE : FALSE;
	}
	*/

	for (i = 1; i <= var_num; i++) {

		flag = 0;
		for (j = 1; j <= var_num - i ; j++) {
			left = sorted_var[j], right = sorted_var[j + 1];
			if (var_activity[left] < var_activity[right]) {//交换
				flag = 1;
				sorted_var[j] = right;
				sorted_var[j+1] = left;
			}
		}
		if (!flag) break;
	}
	for (i = 1; i <= var_num; i++) {//这里有优化空间
		order_for_var[sorted_var[i]] = i;
	}
}

void Var_Order_undo(Solver_t * S, Var_Order_t * order, int var) {
	return;
	S->var_activity[var]  /= 10;
	Var_Order_updata(S, order, var);
	//do what?
	//减小变量的act，并且？？，似乎只是将变量改成unbound
	//那么在这里就没有事情发生了
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
	//以为选择非最优会有所优化
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