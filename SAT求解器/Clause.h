#ifndef  CLAUSE_INCLUDE_H
#define CLAUSE_INCLUDE_H
#include "Lit.h"
#include "Solver.h"
//�����Ӿ�����

struct Clause_s {
	bool learnt;//�Ƿ���ѧϰ�Ӿ�
	float activity;//����Ӿ�Ļ�Ծ��
	int Lit_num;//literal ������
	Lit_t * base;//��һ�����ֵĻ���ַ
} ;

extern bool Clause_new(Solver_t * S, int Lit_num, Lit_t * Lit_base, bool learnt, Clause_t ** out_clause);//��lit_num ��base�е�����д��*out_clause��
extern bool Clause_locked(Clause_t * lits, Solver_t * S);//�����Ƿ�����
extern void Clause_remove(Clause_t * cla, Solver_t * S);//ɾ���Ӿ�
extern bool Clause_simplify(Clause_t * cla, Solver_t * S);//�����Ӿ䣬���򷵻��棬���򷵻ؼ�
extern bool Clause_propagate(Clause_t * clas, Solver_t * S, Lit_t p);//�����Ӿ�
extern void Clause_calcReason(Clause_t * clas, Solver_t * S, Lit_t p, Lit_t ** outreason, int * reason_len);//����reason
extern void Clause_undo(Clause_t * clas, Solver_t * S,Lit_t  p);//undo
extern void removeElem(Clause_t * clas, Solver_t * S, Lit_t lit);//ɾ��watch�����е�clas�Ӿ�

extern void print_cla(Clause_t * cla);//���һ���־�

#endif // ! CLAUSE_INCLUDE_H
