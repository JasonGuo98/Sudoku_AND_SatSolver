#ifndef CNF_INCLUDE
#define CNF_INCLUDE
#include "Basic_def.h"
#include "Clause.h"
#include "Lit.h"

extern FILE * Cnf_openfile(char * filename);
extern unsigned int Cnf_read_Clause(FILE * fp, Lit_t ** litspp);//��ȡһ��clause�����ؾ��ӳ���
extern void Cnf_read_line(FILE  * fp);//����һ��
extern void Cnf_read_Basic_info(FILE  * fp, unsigned int * var_num, unsigned int * constrs_num); //��ȡ�������Ӿ���
#endif // CNF_INCLUDE
