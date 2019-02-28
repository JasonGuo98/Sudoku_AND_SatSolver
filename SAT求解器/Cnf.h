#ifndef CNF_INCLUDE
#define CNF_INCLUDE
#include "Basic_def.h"
#include "Clause.h"
#include "Lit.h"

extern FILE * Cnf_openfile(char * filename);
extern unsigned int Cnf_read_Clause(FILE * fp, Lit_t ** litspp);//读取一行clause，返回句子长度
extern void Cnf_read_line(FILE  * fp);//读掉一行
extern void Cnf_read_Basic_info(FILE  * fp, unsigned int * var_num, unsigned int * constrs_num); //读取变量和子句数
#endif // CNF_INCLUDE
