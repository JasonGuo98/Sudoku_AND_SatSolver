#include "Solver.h"

/*��ȡcnf�ļ�������һ��solver����*/
extern Solver_t * Solver_read_file(FILE *  fp) {
	printf("\n****************************READING****************************\n");
	if (!fp)
		return NULL;
	int begintime, endtime;/*��ʱ*/
	begintime = clock();/*��ʱ��ʼ*/
	unsigned int var_num, constrs_num;//���������Ӿ���
	unsigned int i, j, Lit_num;
	Lit_t * Lit_base = NULL;
	double acts[4] = {
		10,			//cla_inc
		0.95,		//cla_decay
		10,			//var_inc
		0.999			//var_decay
	};
	Cnf_read_Basic_info(fp, &var_num, &constrs_num);//��ȡ�����Ӿ���
	printf("var_num = %d, constrs_num = %d, ", var_num, constrs_num);
	Solver_t * S = new_Solver(var_num, constrs_num, acts);
	for (i = 0, j = 0; j < constrs_num; j++) {
		//printf("j = %d\n", j);
		Lit_num = Cnf_read_Clause(fp, &Lit_base);//��ȡ�Ӿ䣬����base
		if (!Clause_new(S, Lit_num, Lit_base, FALSE, &(S->constrs[i]))) {
			return NULL;
			//Ӧ�û�������֮ǰ�Ŀռ�
		}
		if (S->constrs[i]) {//����������ÿռ�
			//print_cla(S->constrs[i]);
			i++;
		}
		free(Lit_base);
	}
	S->constrs = (Clause_t **)realloc(S->constrs, sizeof(Clause_t*)*i);//����ռ�
	S->constrs_num = i;
	for (i = 1; i <= var_num; i++) {
		S->polarity[i] = S->watches[i * 2].length > S->watches[i * 2 - 1].length ?  TRUE: FALSE;
	}
	endtime = clock();	/*��ʱ����*/
	printf("time = %d\n", endtime - begintime);
	return S;
}

/*�����µ�Solver����ĺ�����д��϶����ܿ���ȫ�ֱ�����*/
Solver_t * new_Solver(int var_num,int constrs_num,double act[4]) {
	Solver_t * solver = (Solver_t *)malloc(sizeof(Solver_t));//�����µ�solver����

	/*��ʼ�����״̬Ϊ ����*/
	solver->sta = LUndef;

	/*�Ӿ����ͱ�������ֵ*/
	solver->constrs_num = constrs_num;
	solver->var_num = var_num;//�Ӿ����ͱ�������ֵ
	solver->learnts_num = 0;//��ʼ��ѧϰ�Ӿ���Ϊ0
	solver->spaceallowed_learnts_num = SPACE_INC;//�ռ������ѧϰԼ��������һ����������û�����ô����

	/*�Ӿ�ռ����*/
	solver->constrs = (Clause_t **)malloc(constrs_num * sizeof(Clause_t*));//�������Լ���ռ�
	solver->learnts = (Clause_t **)malloc(solver->spaceallowed_learnts_num * sizeof(Clause_t*));//ѧϰ�Ӿ�Ŀռ����

	/*�ĸ��йػ�Ծ�ȵ�double���͵ĳ���*/
	solver->cla_inc = act[0];
	solver->cla_decay = act[1];
	solver->var_inc = act[2];
	solver->var_decay = act[3];

	/*����˳��*/
	solver->order = new_Var_Order(var_num);//����˳�����

	/*������activity�ռ����*/
	solver->var_activity = (double*)malloc(sizeof(double)*(var_num + 1));
	
	/*ָ�ɵĿռ����ͱ�������ֵ*/
	solver->assigns = (Lbool*)malloc(sizeof(Lbool)*(var_num+1));

	/*�����ļ���*/
	solver->polarity = (bool *)malloc(sizeof(bool)*(var_num + 1));
	
	/*����*/
	solver->watches = (Vec_t*)malloc(sizeof(Vec_t)*(1+ var_num*2));
	solver->undos = (Vec_t*)malloc(sizeof(Vec_t)*(1 + var_num));
	solver->PropQ = Queue_new(var_num);//�������ж���

	/*����·��*/
	solver->trail_num = 0;
	solver->spaceallowed_trail_num = SPACE_INC;//������0��butû�뵽
	solver->trail = (Lit_t*)malloc(sizeof(Lit_t)*solver->spaceallowed_trail_num);
	
	/*level��reason��*/
	solver->trail_lim_num = 0;
	solver->spaceallowed_trail_lim_num = SPACE_INC;//һ������0
	solver->trail_lim = (unsigned int *)malloc(sizeof(unsigned int)*solver->spaceallowed_trail_lim_num);
	solver->reason = (Clause_t **)malloc(sizeof(Clause_t * )*(var_num + 1));
	
	//û�и���ʼֵ
	solver->level = (int *)malloc((var_num + 1) * sizeof(int));
	solver->root_level = 0;//root-level������Ϊ0

	/*ģ��������*/
	solver->model = (bool*)malloc(sizeof(bool)*(1 + var_num));

	/*��ʼ������*/
	int i;
	for (i = 1; i <= var_num; i++) {
		Solver_newVar(solver, i);
	}
	return solver;
}



/*���ر�����*/
unsigned int Solver_nVars(Solver_t * S) {
	return S->var_num;
}

/*���ص�ǰ·���Ĵ�С���Ѿ����ԵĴ�С��*/
unsigned int Solver_nAssigns(Solver_t * S) {
	return S->trail_num;
}

/*�������ƵĴ�С*/
unsigned int Solver_nConstraints(Solver_t * S) {
	return S->constrs_num;
}

/*ѧϰ�Ӿ��С*/
unsigned int Solver_nLearnt(Solver_t * S) {
	return S->learnts_num;
}

/*������ֵ*/
Lbool Solver_var_value(Solver_t * S, int var) {
	return S->assigns[var];
}

/*���ֵ�ֵ*/
Lbool Solver_lit_value(Solver_t * S, Lit_t p) {
	return Lit_value(p, Solver_var_value(S, Lit_var(p)));//�������ֶ�Ӧ������ָ���ڵ�ǰ���������õ�ֵ
}

/*���ص�ǰ�ľ��߲㼶*/
unsigned int Solver_decisionLevel(Solver_t * S) {
	return S->trail_lim_num;
}

/*�����µı������������������index���������Ǳ����*/
int Solver_newVar(Solver_t * S, int var) {
	S->polarity[var] = TRUE;//ͳһ������ֵ�ļ�����Ϊtrue
	S->reason[var] = NULL;//�±�����reason
	S->assigns[var] = LUndef;//�µı����ĸ�ֵΪUndef
	S->level[var] = -1;//�±����ľ����ľ��ߵȼ���-1
	S->var_activity[var] = 0;//�����Ļ�Ծ��Ϊ0
	{//��ʼ��undos����
		S->undos[var].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->undos[var].length = 0;
		S->undos[var].sapceallow_length = SPACE_INC;
	}
	{//��ʼ��watch����
		S->watches[var * 2 - 1].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->watches[var * 2 - 1].length = 0;
		S->watches[var * 2 - 1].sapceallow_length = SPACE_INC;
		S->watches[var * 2].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->watches[var * 2].length = 0;
		S->watches[var * 2].sapceallow_length = SPACE_INC;
	}
	return var;
}

/*��watch list�еĴ���*/
Clause_t * Solver_propagate(Solver_t * S) {
	while ((S->PropQ)->size > 0) {
		Lit_t p = PropQ_dequeue(S->PropQ);//p������
		int i, j = 0, old_length = S->watches[Lit_index(p)].length;
		if (!old_length) continue;//watches list ��û��
		Clause_t ** tmp = (Clause_t **)malloc(sizeof(Clause_t*)*old_length);
		memcpy(tmp, S->watches[Lit_index(p)].base, sizeof(Clause_t*)*old_length);

		S->watches[Lit_index(p)].length = 0;
		//��������Ӧ�õ��Ź۲죬��Ϊ�µ�ѧϰ�Ӿ䶼�ں��棬������ǰ���ֳ�ͻ����������
		for (i = old_length - 1; i >= 0; i--) {
			i;
			if (!Clause_propagate(tmp[i], S, p)) {
				Clause_t * conf = tmp[i];
				for (i--; i >= 0; i--) 
					Solver_watches_push(S, Lit_index(p), tmp[i]);
				PropQ_clear(S->PropQ);//��յ�ǰ�Ĵ�������
				free(tmp);
				return conf;
			}//end_if
		}//end_for

		/*����
		for (i = 0; i < old_length; i++) {
			if (!Clause_propagate(tmp[i], S, p)) {
				Clause_t * conf = tmp[i];
				for(i++; i< old_length;i++)
					Solver_watches_push(S, Lit_index(p), tmp[i]);
				PropQ_clear(S->PropQ);//��յ�ǰ�Ĵ�������
				free(tmp);
				return conf;
			}//end_if
		}//end_for
		*/
		if ((int)S->watches[Lit_index(p)].length < (int)S->watches[Lit_index(p)].sapceallow_length - 2 * SPACE_INC) {//�ռ�����
			S->watches[Lit_index(p)].sapceallow_length = S->watches[Lit_index(p)].length + SPACE_INC;
			S->watches[Lit_index(p)].base = (Clause_t **)realloc(S->watches[Lit_index(p)].base, sizeof(Clause_t *)*(S->watches[Lit_index(p)].sapceallow_length));
		}
		free(tmp);
	}//end_while
	
	return NULL;
}

/*���봫�����еĹ���*/
bool Solver_enqueue(Solver_t * S, Lit_t p, Clause_t * from) {
	if (Solver_lit_value(S,p) != LUndef) {
		if (Solver_lit_value(S, p) == LFalse)//��ͻ���
			return FALSE;
		else
			//���е� �̶��� ָ�ɣ������
			return TRUE;
	}
	else {
		//�µ�fact����¼
		S->assigns[Lit_var(p)] = Lit_sign(p) == TRUE ? LFalse:LTrue;//���������Ϊ��
		S->level[Lit_var(p)] = Solver_decisionLevel(S);
		S->reason[Lit_var(p)] = from;
		if (S->trail_num == S->spaceallowed_trail_num) {
			S->trail = (Lit_t *)realloc(S->trail, sizeof(Lit_t)*(S->spaceallowed_trail_num + SPACE_INC));
			S->spaceallowed_trail_num += SPACE_INC;
		}
		{//trail.push(p)
			S->trail[S->trail_num] = p;
			S->trail_num++;
		}

		PropQ_insert((S->PropQ), p);
		return TRUE;
	}
}

/*������ͻ��ѧϰ�Ĺ���*/
void Solver_analyze(Solver_t * S, Clause_t * confl, Lit_t ** out_learnt, int * learnt_len, int * out_btlevel) {
	//print_cla(confl);
	bool * seen = (bool *)malloc(sizeof(bool)*(1+Solver_nVars(S)));
	memset(seen, 0, sizeof(bool)*(1 + Solver_nVars(S)));//�����еı�������Ϊfalse
	int counter = 0;
	Lit_t p;
	p.idx_sig = 0;//��ʾδ���������
	Lit_t * p_reason = NULL;
	int p_reason_len = 0;

	//leave room for the asserting literal
	*out_learnt = (Lit_t *)malloc(sizeof(Lit_t)*SPACE_INC);
	int spaceallowed_out_len = SPACE_INC;
	out_learnt[0]->idx_sig = 0;
	//out_learnt.push();������������һ���õ�
	*learnt_len = 1;
	*out_btlevel = 0;

	do {
		//print_cla(confl);
		if (p_reason)
		{
			free(p_reason);
			p_reason = NULL;
			p_reason_len = 0;
		}//p_reason.clear()

		Clause_calcReason(confl, S, p, &p_reason, &p_reason_len);//������ confl != NULL

		//׷�� p �� reason
		int j;
		for (j = 0; j < p_reason_len; j++) {
			Lit_t q = p_reason[j];
			if (!seen[Lit_var(q)]) {
				seen[Lit_var(q)] = TRUE;
				if (S->level[Lit_var(q)] == Solver_decisionLevel(S))//�����decision level
					counter++;
				else if (S->level[Lit_var(q)] > 0)//�ų�����level 0 �ı���
				{
					if (*learnt_len == spaceallowed_out_len) {
						spaceallowed_out_len += SPACE_INC;
						*out_learnt = (Lit_t *)realloc(*out_learnt, sizeof(Lit_t)*spaceallowed_out_len);
					}
					{
						(*out_learnt)[*learnt_len] = re_Lit(q);
						(*learnt_len)++;
						*out_btlevel = *out_btlevel > S->level[Lit_var(q)] ? *out_btlevel : S->level[Lit_var(q)];
					}
				}//end_elif
			}//end_if (!seen[Lit_var(q)])
		}//end_for
		/*ѡ����һ���������۲�*/
		do {
			p = S->trail[S->trail_num - 1];
			confl = S->reason[Lit_var(p)];
			//printf("in analyze\n");
			Solver_undoONE(S);
		} while (!seen[Lit_var(p)]);//ֱ���ҵ�һ����������reason�ı���

		counter--;
	} while (counter > 0);//end_do_while
	(*out_learnt)[0] = re_Lit(p);
	free(seen);
}

/*������decision level��ߵ����֣�����������е��±�*/
int Lit_of_highest_decisionlevel(Solver_t * S, int lit_num, Lit_t * base) {
	//�����Ǵ�0 or 1 ��ʼ
	int level = S->root_level, index_now = 1, i = 0;
	for (i = 0; i < lit_num; i++) {
		if (S->level[Lit_var(base[i])] > level)
		{
			level = S->level[Lit_var(base[i])];
			index_now = i;
		}
	}
	return index_now;
}

/*��¼ѧϰ�Ӿ䲢��������*/
void Solver_record(Solver_t * S, Lit_t * clause, int len) {
	//��Ϊû�����ã�����д����ͬ
	Clause_t *c; // ������Ӿ䣬���clause��unit������NULL
	Clause_new(S, len, clause, TRUE,&c);//�����ﲻ����fail
	Solver_enqueue(S, clause[0], c);//�����ﲻ����fail
	S->polarity[Lit_var(clause[0])] = S->polarity[Lit_var(clause[0])] ? FALSE : TRUE;
	//print_cla(c);
	if (c) {//ѧϰ�Ӿ䲻Ϊ�գ���д��
		//һ��ʼ���õ�����Ŀռ䣬����ֱ�ӼӾͺ�
		if (S->learnts_num == S->spaceallowed_learnts_num)//���ӿռ�
		{
			S->spaceallowed_learnts_num += SPACE_INC;
			S->learnts = (Clause_t **)realloc(S->learnts, sizeof(Clause_t*)*S->spaceallowed_learnts_num);
		}
		(S->learnts[S->learnts_num]) = c;
		S->learnts_num++;
	}//end_if c
}

/*����һ��*/
void Solver_undoONE(Solver_t * S) {
	Lit_t p = S->trail[S->trail_num - 1];
	int x = Lit_var(p);//int == var
	S->assigns[x] = LUndef;
	S->reason[x] = NULL;
	S->level[x] = -1;
	Var_Order_undo(S,(S->order),x);

	S->trail_num--; //trail.pop()
	if ((int)S->trail_num < (int)S->spaceallowed_trail_num - SPACE_INC * 2) {//�ռ�յĺܶ࣬�������ռ�
		S->spaceallowed_trail_num = S->trail_num + SPACE_INC;
		S->trail = (Lit_t *)realloc(S->trail, sizeof(Lit_t)*(S->spaceallowed_trail_num));
	}
	
	while (S->undos[x].length > 0) {
		//����x�����һ��
		Clause_undo((S->undos[x].base[S->undos[x].length - 1]), S, p);//��֪����һ����Ҫ��ɶ��
		S->undos[x].length--;//pop
	}
	if (S->undos[x].sapceallow_length > SPACE_INC) {
		//�����ռ�
		S->undos[x].length = 0;
		S->undos[x].sapceallow_length = SPACE_INC;
		S->undos[x].base = (Clause_t **)realloc(S->undos[x].base, sizeof(Clause_t*)*SPACE_INC);
	}
}

/*ָ��һ������*/
bool Solver_assume(Solver_t * S, Lit_t p) {
	if (S->trail_lim_num == S->spaceallowed_trail_lim_num) {//���ӿռ�
		S->spaceallowed_trail_lim_num += SPACE_INC;
		S->trail_lim = (unsigned int*)realloc(S->trail_lim,sizeof(unsigned int)*S->spaceallowed_trail_lim_num);
	}
	S->trail_lim[S->trail_lim_num] = S->trail_num;//ָ��ʱ�����Ӿ��ߵȼ�
	S->trail_lim_num++;
	return Solver_enqueue(S, p, NULL);//��Ӵ���
}

/*����һ��*/
void Solver_cancel(Solver_t * S) {
	int c = S->trail_num - (S->trail_lim[S->trail_lim_num-1]);
	//printf("c = %d\n", c);
	for (; c > 0; c--)//����c��
		Solver_undoONE(S);
	S->trail_lim_num--;//trail_lim.pop()
	if ((int)S->trail_lim_num < (int)S->spaceallowed_trail_lim_num - 2 * SPACE_INC)//�����ռ�
	{
		S->spaceallowed_trail_lim_num = S->trail_lim_num + SPACE_INC;
		unsigned int * tmp = (unsigned int *)malloc(sizeof(unsigned int)*(S->spaceallowed_trail_lim_num));
		memcpy(tmp, S->trail_lim, sizeof(unsigned int)*(S->spaceallowed_trail_lim_num));
		free(S->trail_lim);
		S->trail_lim = tmp;
	}
}

/*������level*/
void Solver_cancelUntil(Solver_t * S, int level) {
	while ((int)Solver_decisionLevel(S) > level)
		Solver_cancel(S);
}

void Solver_varBumpActivity(Solver_t * S, int var) {
	S->var_activity[var] += S->var_inc;
	if (S->var_activity[var]  > 1e100)
		Solver_varRescaleActivity(S);
	Var_Order_updata(S,(S->order), var);//updata x's order
}

void Solver_varDecayActivity(Solver_t * S) {
	S->var_inc *= S->var_decay;
}

void Solver_varRescaleActivity(Solver_t * S) {
	unsigned int i;
	for (i = 1; i <= Solver_nVars(S); i++)
		S->var_activity[i] *= 1e-100;//decay activity
	S->var_inc *= 1e-100;//activity_inc decay
	S->var_inc += 1e-10;
}

void Solver_claBumpActivity(Solver_t * S, Clause_t * c) {
	c->activity += S->cla_inc;
	if (c->activity > 1e100)
		Solver_claRescaleActivity(S);
	//updata order dont know
}

void Solver_claDecayActivity(Solver_t * S) {
	S->cla_inc *= S->cla_decay;
}

void Solver_claRescaleActivity(Solver_t * S) {
	unsigned int i;
	for (i = 0; i < S->learnts_num; i++)
		S->learnts[i]->activity *= 1e-100;
	S->cla_inc *= 1e-100;
	S->cla_inc += 1e-10;
}

void Solver_decayActivity(Solver_t * S) {
	Solver_varDecayActivity(S);
	Solver_claDecayActivity(S);
}

/*��Сdata base��ֻ��ѧϰ�Ӿ�*/
void Solver_reduceDB(Solver_t * S) {
	unsigned int i, j;
	double lim = S->cla_inc / S->learnts_num;
	sortOnActivity(S->learnts,S);//���ջ�Ծ������ѧϰ�Ӿ�
	Clause_t * cla = NULL;
	for (i = j = 0; i < S->learnts_num / 2; i++) {
		//ǰһ���Ծ�ȵ͵Ĳ�������ֱ��ɾ��
		if (!Clause_locked(S->learnts[i], S)) 
		{
			//Clause_remove(S->learnts[i], S);
			cla = S->learnts[i];
			removeElem(cla, S, re_Lit(cla->base[0]));
			removeElem(cla, S, re_Lit(cla->base[1]));//ɾ���۲������е����Ԫ��
			free(cla->base);//ɾ������Ӿ��������ֵĿռ�
			free(cla); //ɾ������Ӿ�
		}
		else
			S->learnts[j++] = S->learnts[i];
	}
	for (; i < S->learnts_num; i++) {
		if (!Clause_locked(S->learnts[i], S) && S->learnts[i]->activity < lim)
		{
			//Clause_remove(S->learnts[i], S);
			cla = S->learnts[i];
			removeElem(cla, S, re_Lit(cla->base[0]));
			removeElem(cla, S, re_Lit(cla->base[1]));//ɾ���۲������е����Ԫ��
			free(cla->base);//ɾ������Ӿ��������ֵĿռ�
			free(cla); //ɾ������Ӿ�
		}
		else
			S->learnts[j++] = S->learnts[i];
	}
	//shrink learnts_size
	S->learnts_num = j;
	S->spaceallowed_learnts_num = j + SPACE_INC;
	S->learnts = (Clause_t **)realloc(S->learnts, sizeof(Clause_t*)*(j + SPACE_INC));
}

/*������֪��data base*/
bool Solver_simplifyDB(Solver_t * S) {
	if (Solver_propagate(S) != NULL)
		return FALSE;
	int type;
	unsigned int len;
	Clause_t ** cs;
	Clause_t * cla = NULL;
	for (type = 0; type < 2; type++) {
		cs = type ? S->learnts : S->constrs;
		len = type ? (S->learnts_num) : S->constrs_num;
		//if (type == 0) continue;
		unsigned int j = 0, i;
		for (i = 0; i < len; i++) {
			if (Clause_simplify(cs[i], S)) {
				cla = cs[i];
				if (Clause_locked(cla, S))
				{
					S->reason[Lit_var(cla->base[0])] = NULL;
				}
				removeElem(cla, S, re_Lit(cla->base[0]));
				removeElem(cla, S, re_Lit(cla->base[1]));//ɾ���۲������е����Ԫ��
				free(cla->base);//ɾ������Ӿ��������ֵĿռ�
				free(cla); //ɾ������Ӿ�
			}
			else
				cs[j++] = cs[i];
		}
		cs = (Clause_t **)realloc(cs, sizeof(Clause_t*)*(j+SPACE_INC));//�ռ��С��ԭʼ��ַ���ı䣬ֻ��ռ�õĿռ��С��
		if (type) {
			S->learnts = cs;
			S->learnts_num = j;
			S->spaceallowed_learnts_num = j + SPACE_INC;
		}
		else {
			S->constrs = cs;
			S->constrs_num = j;
		}
	}//end_for
	return TRUE;
}

/*�������ڣ�solve����*/
void Solver_solve(Solver_t * S) {
	printf("\n****************************SOLVING****************************\n");
	if (!S)
		return;
	int begintime, endtime;/*��ʱ*/
	double var_decay = 0.95;
	double cla_decay = 0.999;
	double nof_conflicts = 100;//Solver_nConstraints(S)/S->var_num + 100;
	double nof_learnts = Solver_nConstraints(S)/3;
	Lbool status = LUndef;
	S->root_level = Solver_decisionLevel(S);

	/*Ԥ����*/
	Solver_simplifyDB(S);
	Solver_staticVarOrder(S);

	/*Solve*/
	while (status == LUndef) {
		begintime = clock();/*��ʱ��ʼ*/
		printf("allowed CONFL = %d, ", (int)nof_conflicts);//�˴γ�������ĳ�ͻ����
		status = Solver_search(S, (int)nof_conflicts, (int)nof_learnts, var_decay, cla_decay);
		nof_conflicts *= 1.5;
		nof_learnts *= 1.1; 	

		endtime = clock();	/*��ʱ����*/
		printf("VAR assigned %d, time = %d \n",  S->trail_lim[0], endtime - begintime);//�˴�ȷ���ĸ�ֵ
		Var_Order_updataAll(S, S->order);
  	}
	Solver_cancelUntil(S, 0);
	S->sta = status;//��¼���״̬
}

/*�������̣����ܻ�������decay����*/
Lbool Solver_search(Solver_t * S,  int nof_confilt,  int nof_learnts, double var_decay, double cla_decay) {
	int conflictC = 0, num_assinged = S->trail_lim[0];
	S->var_decay =  1/var_decay;
	S->cla_decay =  1/cla_decay;
	static Lit_t p = { 0 };
	while (1) {
		/*
		if (S->trail_lim[0] > num_assinged) {
			num_assinged = S->trail_lim[0];
			nof_confilt *=1.001;//���ӱ��εĳ��Կ�����
		}
		else {
			nof_confilt *= 0.9999;
		}
		*/

		//printf("conflictC = %d allowed = %d\n", conflictC, nof_confilt);
		Clause_t * confl = Solver_propagate(S);
		if (confl != NULL) {//�ҵ���ͻ
			conflictC++;
			Lit_t * learnt_clause;
			int learnt_len = 0;
			int backtrack_level;
			if (Solver_decisionLevel(S) == S->root_level)//�����߲�Ĵ��󣬱���unsat
			{
				unsigned int i;
				for (i = 1; i <= Solver_nVars(S); i++) {//��������ģ��
					S->model[i] = (Solver_var_value(S, i) == LTrue) ? TRUE : FALSE;
				}
				return LFalse;
			}
			Solver_analyze(S, confl, &learnt_clause, &learnt_len, &backtrack_level);//����
			Solver_cancelUntil(S, backtrack_level > S->root_level ? backtrack_level : S->root_level);//����
			Solver_record(S, learnt_clause, learnt_len);//��¼
			//printf("learnt_len = %d\n", learnt_len);
			free(learnt_clause);
			Solver_decayActivity(S);//˥��act
		}//end_if for conflict found
		else {//No conflict
			if (Solver_decisionLevel(S) == 0) {//�����߲㣬��db
				Solver_simplifyDB(S);
			}
			if (  (int)((int)S->learnts_num - (int)Solver_nAssigns(S)) > nof_learnts) {//ѧϰ�Ӿ����
				Solver_reduceDB(S);
			}
			if (Solver_nAssigns(S) == Solver_nVars(S)) {//�ҵ�ģ�ͣ����沢����
				printf("confl = %d ",conflictC);
				unsigned int i;
				for (i = 1; i <= Solver_nVars(S); i++) {
					S->model[i] = (Solver_var_value(S, i) == LTrue)? TRUE:FALSE;
				}
				Solver_cancelUntil(S, S->root_level);
				return LTrue;
			}
			else if (conflictC >= nof_confilt) {//��ͻ���࣬����
				p.idx_sig = S->trail[0].idx_sig;
				Solver_cancelUntil(S, S->root_level);
				return LUndef;
			}
			else {
				//ѡ���µı���
				int var = Var_Order_select(S, (S->order));
				int add = S->polarity[var] ? 0 : 1;
				p.idx_sig = 2 * var - add;
				Solver_assume(S, p);

				//S->polarity[var] =  S->polarity[var] ? FALSE : TRUE;
				//S->polarity[var] = random(2) ? S->polarity[var]: S->polarity[var] ? FALSE : TRUE;
			}
		}//end_else for conflict not found
	}//end_while(1)
	return LUndef;
}

/*���ؽ����out�ļ�*/
void Solver_reply(Solver_t * S, int time, FILE * out) {
	printf("\n***************************ANSWERING***************************\n");
	if (!S) {
		fprintf(out, "s 0\n");
		fprintf(out, "t %d\n", time);//��ӡ��ʱ
		return;
	}
	fprintf(out,"s %d\n", S->sta);//���״̬
	fprint_ans(S,out);//��ӡ��
	fprintf(out, "t %d\n", time);//��ӡ��ʱ
}


/*undo ����ջ*/
void Solver_undos_push(Solver_t * S, int var, Clause_t  * c) {
	if (S->undos[var].length == S->undos[var].sapceallow_length)
	{
		S->undos[var].sapceallow_length += SPACE_INC;
		S->undos[var].base = (Clause_t **)realloc(S->undos[var].base, sizeof(Clause_t *)*S->undos[var].sapceallow_length);
	}
	S->undos[var].base[S->undos[var].length] = c;
	S->undos[var].length++;
}

/*watch list ����ջ*/
void Solver_watches_push(Solver_t * S, int lit_index, Clause_t  * c) {
	if (S->watches[lit_index].length == S->watches[lit_index].sapceallow_length)
	{
		S->watches[lit_index].sapceallow_length += SPACE_INC;
		S->watches[lit_index].base = (Clause_t **)realloc(S->watches[lit_index].base, sizeof(Clause_t *)*S->watches[lit_index].sapceallow_length);
	}
	S->watches[lit_index].base[S->watches[lit_index].length] = c;
	S->watches[lit_index].length++;
}

int compare(const void * a, const void * b) {
	//print_cla(*((Clause_t **)a));
	float da = (*(Clause_t **)a)->activity;
	float db = (*(Clause_t **)b)->activity;
	return db > da ? 1 : -1;
}

/*��ѧϰ�Ӿ䰴��Ծ�ȴ�С��������*/
void sortOnActivity(Clause_t ** clas, Solver_t * S) {
	if ((S->learnts_num) == 0) return;//û���־�ֱ�ӷ���
	qsort(clas, S->learnts_num, sizeof(Clause_t *), compare);
}


/*���ݻ����Ӿ�����Ծ�ȵĺ���*/
void Solver_staticVarOrder(Solver_t * S){
	//clear activity
	unsigned int i,j;
	for (i = 1; i <= S->var_num; i++) {
		S->var_activity[i] = 0;
	}
	//�����ı�����Ծ��̽��
	for (i = 0; i < S->constrs_num; i++) {
		Clause_t * c = S->constrs[i];
		double add = pow(2, -(c->Lit_num));
		for (j = 0; j < c->Lit_num; j++) {
			S->var_activity[ Lit_var(c->base[j])  ] += add;
		}
	}
	Var_Order_updataAll(S, S->order);
}

/*��ӡwatches*/
void print_watches(Solver_t * S) {
	unsigned int i, j, clas_num = 0;
	printf("\n***************************in watches :***************************\n");
	for (i = 1; i <= S->var_num * 2; i++) {
		printf("watch %d ::\n", i);
		for (j = 0; j < S->watches[i].length; j++) {
			print_cla(S->watches[i].base[j]);
			clas_num++;
		}

		putchar('\n');
	}
	printf("\n clas_num = %d\n", clas_num);
}

/*��ӡ��*/
void fprint_ans(Solver_t * S,FILE *out) {
	if (S->sta == LTrue) {
		fprintf(out, "v ");
		int i;
		for (i = 1; i <= S->var_num; i++)
			fprintf(out, "%d ", S->model[i] ? i : -i );
		fprintf(out, "\n");
	}
}

/*���Դ�*/
bool test_ans(Solver_t * S) {
	unsigned int i, j, right = 1;
	for (i = 0; i < S->constrs_num; i++) {
		right = 0;
		for (j = 0; j < S->constrs[i]->Lit_num; j++) {
			if (Lit_value(S->constrs[i]->base[j], S->model[Lit_var(S->constrs[i]->base[j])]) == LTrue)
			{
				right = 1; break;
			}
			else {
				continue;
			}
		}
		if (!right) {
			print_cla(S->constrs[i]); 
			return FALSE;
		}

	}
	return TRUE;
}

/*�������*/
void Solver_test(Solver_t * S) {
	printf("\n**************************SAT_TESTING**************************\n");
	if (S && test_ans(S)) {
		printf("right\n");
	}
	else printf("wrong\n");
}