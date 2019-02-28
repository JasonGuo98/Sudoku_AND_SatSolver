#include "Cnf.h"

/*��һ��cnf�ļ�������FILEָ�룬��ʧ�ܷ���NULL*/
FILE * Cnf_openfile(char * filename) {
	FILE *  fp;//����ȡ�ļ�ָ��
	errno_t err = fopen_s(&fp, filename, "r") != 0;
	if (err)//��ȡ�ļ�ʧ��
	{
		printf("not found file.\n");
		getchar();
		return NULL;
	}
	return fp;
}

/*��ȡһ��clause�����ؾ��ӳ���*/
unsigned int Cnf_read_Clause(FILE * fp, Lit_t ** litspp) {
	unsigned int l = 0;
	unsigned space_allowed_l = SPACE_INC;
	int var;
	*litspp = (Lit_t *)malloc(sizeof(Lit_t)*space_allowed_l);//ָ��ռ����

	while (fscanf_s(fp, "%d", &var)) {
		if (var != 0) {
			if (l == space_allowed_l) {//���ɿռ�
				space_allowed_l += SPACE_INC;
				*litspp = (Lit_t *)realloc(*litspp, sizeof(Lit_t)*space_allowed_l);
			}
			(*litspp)[l++].idx_sig = var > 0 ? (var) * 2 : (-var) * 2 - 1;//����������
			//printf("%d ", (*litspp)[l - 1].idx_sig);
		}
		else
		{
			break;//��ǰ�н���
		}
	}
	*litspp = (Lit_t *)realloc((*litspp), sizeof(Lit_t)*l);//��С�ռ�
	return l;
}

/*����һ��*/
void Cnf_read_line(FILE  * fp) {
	char c;
	while (1) {
		fscanf_s(fp, "%c", &c, sizeof(char));
		if (c == '\n') return;
	}
}

/*��ȡ�������Ӿ���*/
void Cnf_read_Basic_info(FILE  * fp, unsigned int * var_num, unsigned int * constrs_num) {
	char c;
	char s[5];
	while (1) {
		fscanf_s(fp, "%c", &c, sizeof(char));
		if (c == 'c')
			Cnf_read_line(fp);
		else if (c == 'p') {
			fscanf_s(fp, "%s", s, sizeof(char) * 4);//����ȡcnf
			fscanf_s(fp, "%ud", var_num);
			fscanf_s(fp, "%ud", constrs_num);
			return;
		}
	}
}
