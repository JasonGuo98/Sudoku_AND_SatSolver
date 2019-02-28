#include "Lit.h"
/*���ض�Ӧ�ı���*/
unsigned int Lit_var(Lit_t lit) {
	//new_lit.idx_sig = (lit.idx_sig & 0x1 ? 1 : -1) + lit.idx_sig;
	return (lit.idx_sig+1)>>1;
}

/*����assign������ֵ������ֵ�ֵ��������True,False,Undef*/
Lbool Lit_value(Lit_t lit, Lbool assign) {
	if (assign == LUndef) return LUndef;//���δָ����ֱ�ӷ���δָ��
	//ֻ��assignֻ����0 or 1
	//1 ^ 0 = 1,����ΪTrue ������ ����TRUE����֮����True
	if (assign ^ Lit_sign(lit) )
		return LTrue;
	return LFalse;
}

/*����*/
void Lits_sort(const unsigned int length, Lit_t * base) {
	//ʹ�û�����ð������,��Ϊ����Ļ�������,��ԭ�����м�¼
	unsigned int i, j,Flag=0,temp;
	for (i = 1; i < length; i++) {
		Flag = 0;
		for (j = 0; j < length-i; j++) {
			if (base[j].idx_sig > base[j+1].idx_sig)
			{
				Flag = 1;
				temp = base[j].idx_sig; base[j].idx_sig = base[j + 1].idx_sig; base[j + 1].idx_sig = temp;
			}
		}
		if (!Flag) return;
	}
}

/*�������ֵ�index��������ͬ��*/
unsigned int Lit_index(Lit_t lit) {
	return lit.idx_sig;
}

/*���������Ƿ��з���*/
bool Lit_sign(Lit_t lit) {
	if (lit.idx_sig & 0x1)
		return TRUE;
	return FALSE;
}

/*�����෴���ŵ�����*/
Lit_t re_Lit(Lit_t lit) {
	Lit_t re_lit;
	re_lit.idx_sig = Lit_sign(lit) ? lit.idx_sig + 1 : lit.idx_sig - 1;//[��->��][��->��]
	return re_lit;
}

/*�����Ƿ������base�󳤶�Ϊlen��������*/
bool exist_var(Lit_t * Lit_base, int length, Lit_t lit, int *index) {
	if (!length) return FALSE;
	int i;
	for (i = 0; i < length; i++) {
		if (Lit_var(lit) == Lit_var(Lit_base[i]))
		{
			*index = i;
			return TRUE;
		}
	}
	return FALSE;
}


