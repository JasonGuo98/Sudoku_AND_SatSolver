#ifndef LIT_H_INCLUDE
#define LIT_H_INCLUDE
#include "Basic_def.h"
#include "Lbool.h"

/*��������*/
struct Lit_s {
	int idx_sig;//== var*2 - sign? 1 : 0;
};

extern int Lit_var(Lit_t lit);//���ض�Ӧ�ı���
extern Lbool Lit_value(Lit_t lit, Lbool assign);//�������ڵ�assign������ֵ������ֵ�ֵ��������True,False,Undef
extern void Lits_sort(const unsigned int length, Lit_t * base);//�����е�base�ռ��������൱����������
extern unsigned int Lit_index(Lit_t lit);//���ض�Ӧ�ı�����index
extern bool Lit_sign(Lit_t lit);//���������Ƿ��з���
extern Lit_t re_Lit(Lit_t lit);//�����෴���ŵ�����
extern bool exist_var(Lit_t * Lit_base, int length, Lit_t lit, int *index);//�����Ƿ������base�󳤶�Ϊlen��������
#endif // !LIT_H_INCLUDE
