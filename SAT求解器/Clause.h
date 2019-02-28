#ifndef  CLAUSE_INCLUDE_H
#define CLAUSE_INCLUDE_H
#include "Lit.h"
#include "Solver.h"
//定义子句类型

struct Clause_s {
	bool learnt;//是否是学习子句
	float activity;//这个子句的活跃度
	int Lit_num;//literal 的数量
	Lit_t * base;//第一个文字的基地址
} ;

extern bool Clause_new(Solver_t * S, int Lit_num, Lit_t * Lit_base, bool learnt, Clause_t ** out_clause);//将lit_num 个base中的文字写入*out_clause中
extern bool Clause_locked(Clause_t * lits, Solver_t * S);//返回是否锁定
extern void Clause_remove(Clause_t * cla, Solver_t * S);//删除子句
extern bool Clause_simplify(Clause_t * cla, Solver_t * S);//化简子句，真则返回真，否则返回假
extern bool Clause_propagate(Clause_t * clas, Solver_t * S, Lit_t p);//传播子句
extern void Clause_calcReason(Clause_t * clas, Solver_t * S, Lit_t p, Lit_t ** outreason, int * reason_len);//计算reason
extern void Clause_undo(Clause_t * clas, Solver_t * S,Lit_t  p);//undo
extern void removeElem(Clause_t * clas, Solver_t * S, Lit_t lit);//删除watch数组中的clas子句

extern void print_cla(Clause_t * cla);//输出一个字句

#endif // ! CLAUSE_INCLUDE_H
