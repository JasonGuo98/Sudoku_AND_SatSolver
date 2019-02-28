#include "Clause.h"

/*��lit_num ��base�е�����д��*out_clause�У�ָ��δ����ռ䣬���Ӿ��Ѿ�����orδ���㣬�򷵻�FALSE*/
bool Clause_new(Solver_t * S,int Lit_num, Lit_t * Lit_base, bool learnt, Clause_t  ** out_clausepp) {
	*out_clausepp = NULL;
	if (!learnt) {//ֻ�л����Ӿ��ִ��
		int i, j = 0, index = -1;
		//������
		for (i = 0; i < Lit_num; i++) {
			if (Solver_lit_value(S, Lit_base[i]) == LTrue)//������Ӿ����Ѿ���ֵΪ������֣�����TRUE
				return TRUE;
			if (Solver_lit_value(S, Lit_base[i]) == LFalse)	//����������
				continue;
			else if (exist_var(Lit_base, j, Lit_base[i],&index))//�����ظ���������ͬ��λ�ü�¼��index��
			{
				if (Lit_base[index].idx_sig == Lit_base[i].idx_sig) {//�ظ�����
					continue;
				}
				else {//�෴����
					return TRUE;
				}
			}
			else
				Lit_base[j++] = Lit_base[i];//ʲôҲû�з����򣬽�û������������`д��
		}//end_for
		//������free
		Lit_num = j;
	}//end_if_!learnt

	if (Lit_num == 0) {
		return FALSE;//���Ѿ�û�����ַ��ش���
	}
	else if (Lit_num == 1) {
		return Solver_enqueue(S, Lit_base[0], NULL);//����Ψһ��������ӵ�������ҿ������ڴ�����
	//�����Ȼ����true
	}
	else {//��ʼ�����Ӿ�i
		Clause_t * out_clause = (Clause_t *)malloc(sizeof(Clause_t));
		*out_clausepp = out_clause;//ָ��
		out_clause->learnt = learnt;
		out_clause->activity = 0;
		out_clause->Lit_num = Lit_num;
		out_clause->base = (Lit_t *)malloc(Lit_num * sizeof(Lit_t));//������ڴ洦����ֵ�ÿ���
		memcpy(out_clause->base, Lit_base, Lit_num * sizeof(Lit_t)); //���ƹ�ȥ��ԭ����δfree

		if (learnt) {//�����ѧϰ�����Ӿ�
			//ѡ��һ����Ϊ�ڶ������������۲죨��ע����һ��Ӧ���ǹ̶���
			int max_i = Lit_of_highest_decisionlevel(S,Lit_num, Lit_base);//���������ԭʼ�Ĳ������������б����о������decision_level��һ������base�е��±�
			out_clause->base[1] = Lit_base[max_i], out_clause->base[max_i] = Lit_base[1];//�����������н���Ԫ��

			//���ӻ�Ծ��
			Solver_claBumpActivity(S, out_clause);//�´������Ӿ�Ҫ���ǻ�Ծ��
			int i;
			/*���費���Ǳ�����Ծ�ȣ��ܶ�������*/
			for (i = 0; i < Lit_num; i++) {//���Ǳ����Ļ�Ծ��
				Solver_varBumpActivity(S,Lit_var(Lit_base[i]));
			}
			

		}//end_if_learn

		/*�����Ӿ����watcher list*/
		Solver_watches_push(S, Lit_index(re_Lit(out_clause->base[0])), out_clause);//������Ӿ���룬��0��Ԫ�صĸ����ֶ�Ӧ��watches list��
		Solver_watches_push(S, Lit_index(re_Lit(out_clause->base[1])), out_clause);//������Ӿ���룬��1��Ԫ�صĸ����ֶ�Ӧ��watches list��

		return TRUE;
	}//end_else
}

/*�����Ƿ�����*/
bool Clause_locked(Clause_t * cla,Solver_t * S) {
	//���watch�����Ӿ���Ǹ��Ӿ䣬˵�����Ӿ��ǹؼ����Ӿ�
	if ((S->reason[Lit_var(cla->base[0])]) == cla)
		return TRUE;
	return FALSE;
	//������ȵ��о��Ƕ��ߵ�ָ����ͬ����ַ��ͬ
}

/*ɾ���Ӿ�*/
void Clause_remove(Clause_t * cla, Solver_t * S) {
	removeElem(cla, S, re_Lit(cla->base[0]));
	removeElem(cla, S, re_Lit(cla->base[1]));//ɾ���۲������е����Ԫ��

	//����ռ����
	free(cla->base);//ɾ������Ӿ��������ֵĿռ�

	if (cla->learnt ==TRUE) {
		//ָ�������������������Ը�ָ����������������͵��ֽ���
		unsigned int i, j;
		for (i = 0, j = 0; i < S->learnts_num; i++)
		{
			if (S->learnts[i] == cla) i++;
			S->learnts[j++] = S->learnts[i];//�����и����ֱ�Ӹ��Ʒ���
		}
		S->learnts_num--;
		if ((int)S->learnts_num <= (int)S->spaceallowed_learnts_num - 2*SPACE_INC)//��С�ռ�
		{
			S->spaceallowed_learnts_num = S->learnts_num + SPACE_INC;
			S->learnts = (Clause_t **)realloc(S->learnts, sizeof(Clause_t*)*(S->spaceallowed_learnts_num));
		}
	}
	else {//cla->learnt == FALSE
		unsigned int i, j;
		for (i = 0, j = 0; i < S->constrs_num; i++)
		{
			if (S->constrs[i] == cla) i++;
			S->constrs[j++] = S->constrs[i];//�����и����ֱ�Ӹ��Ʒ���
		}
		S->constrs_num--;
	}
}

/*�����Ӿ䣬ֻ��decision level == root levelִ��*/
bool Clause_simplify(Clause_t * cla, Solver_t * S) {
	int j = 0,i;
	for (i = 0; i < cla->Lit_num; i++) {
		if (Solver_lit_value(S,cla->base[i]) == LTrue) {
			return TRUE;
		}
		else if (Solver_lit_value(S, cla->base[i]) == LUndef)
			cla->base[j++] = cla->base[i];//ֻ��i>=2��ִ��
	}
	cla->Lit_num = j;
	cla->base = (Lit_t*)realloc(cla->base, j * sizeof(Lit_t));
	return FALSE;
}

/*�����Ӿ�*/
bool Clause_propagate(Clause_t * clas,  Solver_t * S,Lit_t p) {
	/*
	����־�һ����������־䣬˵������p�Ĺ۲��־�֮һ�����Ե�pΪ���ִ������
	����һ��Ҳ��p��������
	����һ���� ��p�����ڶ����۲������Ƿ������㣬�ҶԵ�0,1λ��
	�������µĹ۲�������p��Ȼ��1���Ҳ�����
	*/
	/*��֤�������ڡ�1��*/
	if (clas->base[0].idx_sig == re_Lit(p).idx_sig)
	{
		clas->base[0] = clas->base[1];
		clas->base[1] = re_Lit(p);
	}
	/*��0��Ԫ����true��������Ӿ�sat*/
	if (Solver_lit_value(S, clas->base[0]) == LTrue) {
		Solver_watches_push(S, Lit_index(p), clas);//���Ӿ��ٴ�ѹ��ջ
		return TRUE;
	}
	//�ҵ��µĹ۲�����
	int i;
	for (i = 2; i < clas->Lit_num; i++) {
		if (Solver_lit_value(S, clas->base[i]) != LFalse) {
			clas->base[1] = clas->base[i];
			clas->base[i] = re_Lit(p);//����û�㶮
			Solver_watches_push(S, Lit_index(re_Lit(clas->base[1])), clas);//�������־�Ŀǰ��Ϊ����1�Ĺ۲��־�
			return TRUE;//���ǵ�Ԫ�Ӿ�ʱ�ĳ���
		}
	}
	/*���ڵ�ǰ��ָ���£�clause �ǵ�ԭ�Ӿ�ִ������*/
	Solver_watches_push(S, Lit_index(p), clas);
	return Solver_enqueue(S, clas->base[0], clas);
}

/*����reason*/
void Clause_calcReason(Clause_t * clas,  Solver_t * S, Lit_t p, Lit_t ** outreason, int * reason_len) {
	//������p==Undef or p == clas->base[0]
	int i, j;
	i = (p.idx_sig == 0) ? 0:1;//�����0��ʾundef������
	//ԭ����i = (p == undef) ? 0:1
	//p��ֵ��δ����	�� ��0��ʼ
	//��p�ľ����־�ĵ�һ�����֣���1��ʼ������p
	*reason_len = clas->Lit_num - i;//reason�ĳ���
	(*outreason) = (Lit_t *)malloc(sizeof(Lit_t ) * (*reason_len));
	
	/* ��ʣ�µ�����д��outreason��*/
	for (j = 0; i < clas->Lit_num; i++) {
		(*outreason)[j++] = re_Lit(clas->base[i]);
	}
	if (clas->learnt)//ѧϰ�Ӿ����ӻ�Ծ��
		Solver_claBumpActivity(S, clas);
}

/*undo��default to do nothing*/
void Clause_undo(Clause_t * clas, Solver_t * S, Lit_t  p) {
	return;//default to do nothing
}

/*ɾ��watch�����е�clas�Ӿ�*/
void removeElem(Clause_t * clas, Solver_t * S,Lit_t lit) {
	unsigned int i ,j;
	unsigned int length = S->watches[Lit_index(lit)].length;
	Vec_t * watch = &(S->watches[Lit_index(lit)]);
	//print_watches(S);
	for (j = 0,i = 0; i < length; i++) {
		if (watch->base[i] == clas) i++;//�ҵ�����Ҫɾ�����Ӿ�
		watch->base[j++] = watch->base[i];
	}
	watch->length--;//��С������Ӿ���

	if ((int)watch->length < (int)watch->sapceallow_length - 2 * SPACE_INC)//��Ҫ��С�ռ�
	{
		watch->sapceallow_length = watch->length + SPACE_INC;
		watch->base = (Clause_t **)realloc(watch->base, sizeof(Clause_t *)*watch->sapceallow_length);
	}
	//print_watches(S);
}

/*��ӡһ���Ӿ�*/
void print_cla(Clause_t * cla) {
	if (!cla) return;
	int i;
	for (i = 0; i < cla->Lit_num; i++) {
		printf("%d ", cla->base[i].idx_sig);
	}
	putchar('\n');
}