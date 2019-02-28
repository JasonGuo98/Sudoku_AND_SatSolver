//定义solver类型的状态


#ifndef SOLVER_INCLUDE_H
#define SOLVER_INCLUDE_H
#include "Clause.h"
#include "Lit.h"
#include "Vec.h"
#include "Queue.h"
#include "Var_Order.h"
#include "Cnf.h"
#include <math.h>

struct Solver_s {
	Lbool sta;//求解状态

	Clause_t ** constrs;//开始的约束+
	unsigned int constrs_num;//开始约束数+
	Clause_t ** learnts;//学习到的子句+
	unsigned int learnts_num;//学习约束数+
	unsigned int spaceallowed_learnts_num;//空间允许的学习约束数，因为有栈的操作需要多余空间+

	double cla_inc;//子句的活跃度的增量+
	double cla_decay;//子句的活跃度衰减率+

	/*变量顺序*/
	double * var_activity;//变量的活跃度+
	unsigned int var_num;//应该还有个变量，为变量的数量+
	double var_inc;//+
	double var_decay;//+
	Var_Order_t * order;//变量的顺序+

	/*传播*/
	Vec_t * watches;//对每一个文字的watch，长度是变量数的二倍
	//这里的设计是，watch[i]对应文字i的观察序列，第一个是0没有对应变量

	Vec_t * undos;//对每一个变量，一个约束的list，当变量变成unbound的时候跟新使用
	Queue_t * PropQ;//传播队列

	/*指派*/
	bool * polarity;//变量赋值的极性
	Lbool *assigns;//当前变量的assigns+
	Lit_t * trail;//list of assignments in chronological(时间) order+
	unsigned int trail_num;//当前的trail的大小+
	unsigned int spaceallowed_trail_num;//空间允许的trail数，因为有栈的操作需要多余空间+

	unsigned int * trail_lim;//这个可以用来算level+
	unsigned int trail_lim_num;//当前的trail_lim的大小+
	unsigned int spaceallowed_trail_lim_num;//空间允许的trail_lim数，因为有栈的操作需要多余空间+
	Clause_t ** reason;//对每个变量，可以示意变量的值的约束+

	int *level;//对每个变量，他被赋值的decision level+
	int root_level;//+

	bool * model;//得到的模型保存在这里
};


extern Solver_t * new_Solver(int var_num, int constrs_num, double act[4]);//创建新的Solver对象的函数，写完肯定不能考靠全局变量吧

/*辅助函数*/
extern unsigned int Solver_nVars(Solver_t * S);//assigns.size
extern unsigned int Solver_nAssigns(Solver_t * S);//trail.size
extern unsigned int Solver_nConstraints(Solver_t * S);//constrs.size
extern unsigned int Solver_nLearnt(Solver_t * S);//learnt.size
extern Lbool Solver_var_value(Solver_t * S,int var);//返回变量的指派
extern Lbool Solver_lit_value(Solver_t * S,Lit_t p);//返回文字的值
extern unsigned int Solver_decisionLevel(Solver_t * S);//返回当前的决策层级

/*核心功能*/
extern Solver_t * Solver_read_file(FILE *  fp);//读取CNF文件创建Solver对象
extern int Solver_newVar(Solver_t * S, int var);//创建新的变量，返回这个变量的index（几乎就是本身额
extern Clause_t * Solver_propagate(Solver_t * S);//在watch list中的传播
extern bool Solver_enqueue(Solver_t * S,Lit_t p, Clause_t * from);//进入传播队列的过程
extern void Solver_analyze(Solver_t * S, Clause_t * confl, Lit_t ** out_learnt, int * learnt_len, int * out_btlevel);//分析冲突并学习的过程
extern void Solver_record(Solver_t * S,Lit_t * clause, int len);//记录子句并引导回溯
extern void Solver_undoONE(Solver_t * S);//回跳一步
extern bool Solver_assume(Solver_t * S,Lit_t p);//指派一个文字
extern void Solver_cancel(Solver_t * S);//撤销
extern void Solver_cancelUntil(Solver_t * S,int level);//撤销到level
extern Lbool Solver_search(Solver_t * S,  int nof_confilt,  int nof_learnts, double var_decay, double cla_decay);//搜索过程，可能还有两个decay参数
extern void Solver_varBumpActivity(Solver_t * S,int var);//增加var的活跃度
extern void Solver_varDecayActivity(Solver_t * S);//var活跃度衰减
extern void Solver_varRescaleActivity(Solver_t * S);//var整体缩小
extern void Solver_claBumpActivity(Solver_t * S,Clause_t * c);//增加cla的活跃度
extern void Solver_claDecayActivity(Solver_t * S);//var活跃度衰减
extern void Solver_claRescaleActivity(Solver_t * S);//cla整体缩小
extern void Solver_decayActivity(Solver_t * S);//全都缩小
extern void Solver_reduceDB(Solver_t * S);//减小学习到的子句数
extern bool Solver_simplifyDB(Solver_t * S);//化简子句
extern void Solver_solve(Solver_t * S);//根据初始条件求解
extern void Solver_reply(Solver_t * S, int time,FILE * out);//返回结果到out文件

extern void Solver_undos_push(Solver_t * S, int var, Clause_t  * c);//将变量的undo传入
extern void Solver_watches_push(Solver_t * S, int lit_index, Clause_t  * c);//将子句加入watch list中
extern int Lit_of_highest_decisionlevel(Solver_t * S, int lit_num, Lit_t * base);//数组中decision level最高的文字（在这个数组中的下标

extern void sortOnActivity(Clause_t ** clas, Solver_t * S);//将学习子句按照活跃度排序
extern void Solver_staticVarOrder(Solver_t * S);//根据基础子句计算活跃度的函数

extern void print_watches(Solver_t * S);//打印watches
extern void fprint_ans(Solver_t * S,FILE * out);//打印答案
extern bool test_ans(Solver_t * S);//测试答案
extern void Solver_test(Solver_t * S);//输出测试

#endif // !SOLVER_INCLUDE_H

