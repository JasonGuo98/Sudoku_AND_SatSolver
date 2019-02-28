#include "Cnf.h"

/*打开一个cnf文件并返回FILE指针，打开失败返回NULL*/
FILE * Cnf_openfile(char * filename) {
	FILE *  fp;//待读取文件指针
	errno_t err = fopen_s(&fp, filename, "r") != 0;
	if (err)//读取文件失败
	{
		printf("not found file.\n");
		getchar();
		return NULL;
	}
	return fp;
}

/*读取一行clause，返回句子长度*/
unsigned int Cnf_read_Clause(FILE * fp, Lit_t ** litspp) {
	unsigned int l = 0;
	unsigned space_allowed_l = SPACE_INC;
	int var;
	*litspp = (Lit_t *)malloc(sizeof(Lit_t)*space_allowed_l);//指针空间分配

	while (fscanf_s(fp, "%d", &var)) {
		if (var != 0) {
			if (l == space_allowed_l) {//增派空间
				space_allowed_l += SPACE_INC;
				*litspp = (Lit_t *)realloc(*litspp, sizeof(Lit_t)*space_allowed_l);
			}
			(*litspp)[l++].idx_sig = var > 0 ? (var) * 2 : (-var) * 2 - 1;//正、负文字
			//printf("%d ", (*litspp)[l - 1].idx_sig);
		}
		else
		{
			break;//当前行结束
		}
	}
	*litspp = (Lit_t *)realloc((*litspp), sizeof(Lit_t)*l);//减小空间
	return l;
}

/*读掉一行*/
void Cnf_read_line(FILE  * fp) {
	char c;
	while (1) {
		fscanf_s(fp, "%c", &c, sizeof(char));
		if (c == '\n') return;
	}
}

/*读取变量和子句数*/
void Cnf_read_Basic_info(FILE  * fp, unsigned int * var_num, unsigned int * constrs_num) {
	char c;
	char s[5];
	while (1) {
		fscanf_s(fp, "%c", &c, sizeof(char));
		if (c == 'c')
			Cnf_read_line(fp);
		else if (c == 'p') {
			fscanf_s(fp, "%s", s, sizeof(char) * 4);//最多读取cnf
			fscanf_s(fp, "%ud", var_num);
			fscanf_s(fp, "%ud", constrs_num);
			return;
		}
	}
}
