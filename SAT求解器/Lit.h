#ifndef LIT_H_INCLUDE
#define LIT_H_INCLUDE
#include "Basic_def.h"
#include "Lbool.h"

/*文字类型*/
struct Lit_s {
	int idx_sig;//== var*2 - sign? 1 : 0;
};

extern int Lit_var(Lit_t lit);//返回对应的变量
extern Lbool Lit_value(Lit_t lit, Lbool assign);//根据现在的assign，返回值这个文字的值，可能是True,False,Undef
extern void Lits_sort(const unsigned int length, Lit_t * base);//在现有的base空间中排序，相当于数组排序
extern unsigned int Lit_index(Lit_t lit);//返回对应的变量的index
extern bool Lit_sign(Lit_t lit);//返回文字是否有符号
extern Lit_t re_Lit(Lit_t lit);//返回相反符号的文字
extern bool exist_var(Lit_t * Lit_base, int length, Lit_t lit, int *index);//变量是否存在于base后长度为len的数组中
#endif // !LIT_H_INCLUDE
