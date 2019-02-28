#include "Lit.h"
/*返回对应的变量*/
unsigned int Lit_var(Lit_t lit) {
	//new_lit.idx_sig = (lit.idx_sig & 0x1 ? 1 : -1) + lit.idx_sig;
	return (lit.idx_sig+1)>>1;
}

/*根据assign，返回值这个文字的值，可能是True,False,Undef*/
Lbool Lit_value(Lit_t lit, Lbool assign) {
	if (assign == LUndef) return LUndef;//如果未指定，直接返回未指定
	//只后assign只能是0 or 1
	//1 ^ 0 = 1,变量为True 正文字 返回TRUE，反之返回True
	if (assign ^ Lit_sign(lit) )
		return LTrue;
	return LFalse;
}

/*排序*/
void Lits_sort(const unsigned int length, Lit_t * base) {
	//使用基础的冒泡排序,因为排序的基数不大,在原数组中记录
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

/*返回文字的index（正负不同）*/
unsigned int Lit_index(Lit_t lit) {
	return lit.idx_sig;
}

/*返回文字是否有符号*/
bool Lit_sign(Lit_t lit) {
	if (lit.idx_sig & 0x1)
		return TRUE;
	return FALSE;
}

/*返回相反符号的文字*/
Lit_t re_Lit(Lit_t lit) {
	Lit_t re_lit;
	re_lit.idx_sig = Lit_sign(lit) ? lit.idx_sig + 1 : lit.idx_sig - 1;//[正->负][负->正]
	return re_lit;
}

/*变量是否存在于base后长度为len的数组中*/
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


