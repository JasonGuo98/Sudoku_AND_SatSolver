#pragma once


#ifndef VARORDER_INCLUDE_H
#define VARORDER_INCLUDE_H
#include "Basic_def.h"
#include "Solver.h"


//这个结构是有必要的，数据直接引用solver中的数据就够了
//顺序要保存 在这个数组中
//或者将activity写到这里，而不是在Solver中

 struct Var_Order_s {
	int * sorted_var;//变量的数组，按照活跃度递减顺序排列变量
	int * order_for_var;//顺序的数组，oredr_for_var[i]，返回变量i的顺序
	unsigned int var_num;//变量数
};

extern Var_Order_t * new_Var_Order(int var_num);//创建新的var_order对象的函数
extern void Var_Order_updata(Solver_t * S, Var_Order_t * order, int varr);//一个变量的活跃度改变时使用，用以改变顺序
extern void Var_Order_updataAll(Solver_t * S, Var_Order_t * order);//更新所有顺序
extern void Var_Order_undo(Solver_t * S, Var_Order_t * order, int var);//当变量变成unbound的时候使用
extern int Var_Order_select(Solver_t * S, Var_Order_t * order);//选择一个新的没有被指派的变量

#endif // !VARORDER_INCLUDE_H
