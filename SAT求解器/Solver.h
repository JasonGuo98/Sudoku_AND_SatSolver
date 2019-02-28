//����solver���͵�״̬


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
	Lbool sta;//���״̬

	Clause_t ** constrs;//��ʼ��Լ��+
	unsigned int constrs_num;//��ʼԼ����+
	Clause_t ** learnts;//ѧϰ�����Ӿ�+
	unsigned int learnts_num;//ѧϰԼ����+
	unsigned int spaceallowed_learnts_num;//�ռ������ѧϰԼ��������Ϊ��ջ�Ĳ�����Ҫ����ռ�+

	double cla_inc;//�Ӿ�Ļ�Ծ�ȵ�����+
	double cla_decay;//�Ӿ�Ļ�Ծ��˥����+

	/*����˳��*/
	double * var_activity;//�����Ļ�Ծ��+
	unsigned int var_num;//Ӧ�û��и�������Ϊ����������+
	double var_inc;//+
	double var_decay;//+
	Var_Order_t * order;//������˳��+

	/*����*/
	Vec_t * watches;//��ÿһ�����ֵ�watch�������Ǳ������Ķ���
	//���������ǣ�watch[i]��Ӧ����i�Ĺ۲����У���һ����0û�ж�Ӧ����

	Vec_t * undos;//��ÿһ��������һ��Լ����list�����������unbound��ʱ�����ʹ��
	Queue_t * PropQ;//��������

	/*ָ��*/
	bool * polarity;//������ֵ�ļ���
	Lbool *assigns;//��ǰ������assigns+
	Lit_t * trail;//list of assignments in chronological(ʱ��) order+
	unsigned int trail_num;//��ǰ��trail�Ĵ�С+
	unsigned int spaceallowed_trail_num;//�ռ������trail������Ϊ��ջ�Ĳ�����Ҫ����ռ�+

	unsigned int * trail_lim;//�������������level+
	unsigned int trail_lim_num;//��ǰ��trail_lim�Ĵ�С+
	unsigned int spaceallowed_trail_lim_num;//�ռ������trail_lim������Ϊ��ջ�Ĳ�����Ҫ����ռ�+
	Clause_t ** reason;//��ÿ������������ʾ�������ֵ��Լ��+

	int *level;//��ÿ��������������ֵ��decision level+
	int root_level;//+

	bool * model;//�õ���ģ�ͱ���������
};


extern Solver_t * new_Solver(int var_num, int constrs_num, double act[4]);//�����µ�Solver����ĺ�����д��϶����ܿ���ȫ�ֱ�����

/*��������*/
extern unsigned int Solver_nVars(Solver_t * S);//assigns.size
extern unsigned int Solver_nAssigns(Solver_t * S);//trail.size
extern unsigned int Solver_nConstraints(Solver_t * S);//constrs.size
extern unsigned int Solver_nLearnt(Solver_t * S);//learnt.size
extern Lbool Solver_var_value(Solver_t * S,int var);//���ر�����ָ��
extern Lbool Solver_lit_value(Solver_t * S,Lit_t p);//�������ֵ�ֵ
extern unsigned int Solver_decisionLevel(Solver_t * S);//���ص�ǰ�ľ��߲㼶

/*���Ĺ���*/
extern Solver_t * Solver_read_file(FILE *  fp);//��ȡCNF�ļ�����Solver����
extern int Solver_newVar(Solver_t * S, int var);//�����µı������������������index���������Ǳ����
extern Clause_t * Solver_propagate(Solver_t * S);//��watch list�еĴ���
extern bool Solver_enqueue(Solver_t * S,Lit_t p, Clause_t * from);//���봫�����еĹ���
extern void Solver_analyze(Solver_t * S, Clause_t * confl, Lit_t ** out_learnt, int * learnt_len, int * out_btlevel);//������ͻ��ѧϰ�Ĺ���
extern void Solver_record(Solver_t * S,Lit_t * clause, int len);//��¼�Ӿ䲢��������
extern void Solver_undoONE(Solver_t * S);//����һ��
extern bool Solver_assume(Solver_t * S,Lit_t p);//ָ��һ������
extern void Solver_cancel(Solver_t * S);//����
extern void Solver_cancelUntil(Solver_t * S,int level);//������level
extern Lbool Solver_search(Solver_t * S,  int nof_confilt,  int nof_learnts, double var_decay, double cla_decay);//�������̣����ܻ�������decay����
extern void Solver_varBumpActivity(Solver_t * S,int var);//����var�Ļ�Ծ��
extern void Solver_varDecayActivity(Solver_t * S);//var��Ծ��˥��
extern void Solver_varRescaleActivity(Solver_t * S);//var������С
extern void Solver_claBumpActivity(Solver_t * S,Clause_t * c);//����cla�Ļ�Ծ��
extern void Solver_claDecayActivity(Solver_t * S);//var��Ծ��˥��
extern void Solver_claRescaleActivity(Solver_t * S);//cla������С
extern void Solver_decayActivity(Solver_t * S);//ȫ����С
extern void Solver_reduceDB(Solver_t * S);//��Сѧϰ�����Ӿ���
extern bool Solver_simplifyDB(Solver_t * S);//�����Ӿ�
extern void Solver_solve(Solver_t * S);//���ݳ�ʼ�������
extern void Solver_reply(Solver_t * S, int time,FILE * out);//���ؽ����out�ļ�

extern void Solver_undos_push(Solver_t * S, int var, Clause_t  * c);//��������undo����
extern void Solver_watches_push(Solver_t * S, int lit_index, Clause_t  * c);//���Ӿ����watch list��
extern int Lit_of_highest_decisionlevel(Solver_t * S, int lit_num, Lit_t * base);//������decision level��ߵ����֣�����������е��±�

extern void sortOnActivity(Clause_t ** clas, Solver_t * S);//��ѧϰ�Ӿ䰴�ջ�Ծ������
extern void Solver_staticVarOrder(Solver_t * S);//���ݻ����Ӿ�����Ծ�ȵĺ���

extern void print_watches(Solver_t * S);//��ӡwatches
extern void fprint_ans(Solver_t * S,FILE * out);//��ӡ��
extern bool test_ans(Solver_t * S);//���Դ�
extern void Solver_test(Solver_t * S);//�������

#endif // !SOLVER_INCLUDE_H

