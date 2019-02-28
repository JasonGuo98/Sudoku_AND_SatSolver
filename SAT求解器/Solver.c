#include "Solver.h"

/*读取cnf文件并返回一个solver对象*/
extern Solver_t * Solver_read_file(FILE *  fp) {
	printf("\n****************************READING****************************\n");
	if (!fp)
		return NULL;
	int begintime, endtime;/*计时*/
	begintime = clock();/*计时开始*/
	unsigned int var_num, constrs_num;//变量数，子句数
	unsigned int i, j, Lit_num;
	Lit_t * Lit_base = NULL;
	double acts[4] = {
		10,			//cla_inc
		0.95,		//cla_decay
		10,			//var_inc
		0.999			//var_decay
	};
	Cnf_read_Basic_info(fp, &var_num, &constrs_num);//读取变量子句数
	printf("var_num = %d, constrs_num = %d, ", var_num, constrs_num);
	Solver_t * S = new_Solver(var_num, constrs_num, acts);
	for (i = 0, j = 0; j < constrs_num; j++) {
		//printf("j = %d\n", j);
		Lit_num = Cnf_read_Clause(fp, &Lit_base);//读取子句，存入base
		if (!Clause_new(S, Lit_num, Lit_base, FALSE, &(S->constrs[i]))) {
			return NULL;
			//应该还有清理之前的空间
		}
		if (S->constrs[i]) {//否则持续利用空间
			//print_cla(S->constrs[i]);
			i++;
		}
		free(Lit_base);
	}
	S->constrs = (Clause_t **)realloc(S->constrs, sizeof(Clause_t*)*i);//清理空间
	S->constrs_num = i;
	for (i = 1; i <= var_num; i++) {
		S->polarity[i] = S->watches[i * 2].length > S->watches[i * 2 - 1].length ?  TRUE: FALSE;
	}
	endtime = clock();	/*计时结束*/
	printf("time = %d\n", endtime - begintime);
	return S;
}

/*创建新的Solver对象的函数，写完肯定不能考靠全局变量吧*/
Solver_t * new_Solver(int var_num,int constrs_num,double act[4]) {
	Solver_t * solver = (Solver_t *)malloc(sizeof(Solver_t));//创建新的solver对象

	/*初始化求解状态为 不定*/
	solver->sta = LUndef;

	/*子句数和变量数赋值*/
	solver->constrs_num = constrs_num;
	solver->var_num = var_num;//子句数和变量数赋值
	solver->learnts_num = 0;//开始的学习子句数为0
	solver->spaceallowed_learnts_num = SPACE_INC;//空间允许的学习约束数，是一个常数，还没想好怎么设置

	/*子句空间分配*/
	solver->constrs = (Clause_t **)malloc(constrs_num * sizeof(Clause_t*));//分配基础约束空间
	solver->learnts = (Clause_t **)malloc(solver->spaceallowed_learnts_num * sizeof(Clause_t*));//学习子句的空间分配

	/*四个有关活跃度的double类型的常数*/
	solver->cla_inc = act[0];
	solver->cla_decay = act[1];
	solver->var_inc = act[2];
	solver->var_decay = act[3];

	/*变量顺序*/
	solver->order = new_Var_Order(var_num);//变量顺序对象

	/*变量的activity空间分配*/
	solver->var_activity = (double*)malloc(sizeof(double)*(var_num + 1));
	
	/*指派的空间分配和变量基础值*/
	solver->assigns = (Lbool*)malloc(sizeof(Lbool)*(var_num+1));

	/*变量的极性*/
	solver->polarity = (bool *)malloc(sizeof(bool)*(var_num + 1));
	
	/*传播*/
	solver->watches = (Vec_t*)malloc(sizeof(Vec_t)*(1+ var_num*2));
	solver->undos = (Vec_t*)malloc(sizeof(Vec_t)*(1 + var_num));
	solver->PropQ = Queue_new(var_num);//传播队列对象

	/*搜索路径*/
	solver->trail_num = 0;
	solver->spaceallowed_trail_num = SPACE_INC;//或许不是0，but没想到
	solver->trail = (Lit_t*)malloc(sizeof(Lit_t)*solver->spaceallowed_trail_num);
	
	/*level，reason等*/
	solver->trail_lim_num = 0;
	solver->spaceallowed_trail_lim_num = SPACE_INC;//一定不是0
	solver->trail_lim = (unsigned int *)malloc(sizeof(unsigned int)*solver->spaceallowed_trail_lim_num);
	solver->reason = (Clause_t **)malloc(sizeof(Clause_t * )*(var_num + 1));
	
	//没有赋初始值
	solver->level = (int *)malloc((var_num + 1) * sizeof(int));
	solver->root_level = 0;//root-level先设置为0

	/*模型在这里*/
	solver->model = (bool*)malloc(sizeof(bool)*(1 + var_num));

	/*初始化变量*/
	int i;
	for (i = 1; i <= var_num; i++) {
		Solver_newVar(solver, i);
	}
	return solver;
}



/*返回变量数*/
unsigned int Solver_nVars(Solver_t * S) {
	return S->var_num;
}

/*返回当前路径的大小（已经尝试的大小）*/
unsigned int Solver_nAssigns(Solver_t * S) {
	return S->trail_num;
}

/*基本限制的大小*/
unsigned int Solver_nConstraints(Solver_t * S) {
	return S->constrs_num;
}

/*学习子句大小*/
unsigned int Solver_nLearnt(Solver_t * S) {
	return S->learnts_num;
}

/*变量的值*/
Lbool Solver_var_value(Solver_t * S, int var) {
	return S->assigns[var];
}

/*文字的值*/
Lbool Solver_lit_value(Solver_t * S, Lit_t p) {
	return Lit_value(p, Solver_var_value(S, Lit_var(p)));//返回文字对应变量的指派在当前文字上作用的值
}

/*返回当前的决策层级*/
unsigned int Solver_decisionLevel(Solver_t * S) {
	return S->trail_lim_num;
}

/*创建新的变量，返回这个变量的index（几乎就是本身额*/
int Solver_newVar(Solver_t * S, int var) {
	S->polarity[var] = TRUE;//统一变量赋值的极性先为true
	S->reason[var] = NULL;//新变量的reason
	S->assigns[var] = LUndef;//新的变量的赋值为Undef
	S->level[var] = -1;//新变量的决定的决策等级是-1
	S->var_activity[var] = 0;//变量的活跃度为0
	{//初始化undos数组
		S->undos[var].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->undos[var].length = 0;
		S->undos[var].sapceallow_length = SPACE_INC;
	}
	{//初始化watch数组
		S->watches[var * 2 - 1].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->watches[var * 2 - 1].length = 0;
		S->watches[var * 2 - 1].sapceallow_length = SPACE_INC;
		S->watches[var * 2].base = (Clause_t **)malloc(sizeof(Clause_t *)*(SPACE_INC));
		S->watches[var * 2].length = 0;
		S->watches[var * 2].sapceallow_length = SPACE_INC;
	}
	return var;
}

/*在watch list中的传播*/
Clause_t * Solver_propagate(Solver_t * S) {
	while ((S->PropQ)->size > 0) {
		Lit_t p = PropQ_dequeue(S->PropQ);//p出队列
		int i, j = 0, old_length = S->watches[Lit_index(p)].length;
		if (!old_length) continue;//watches list 中没有
		Clause_t ** tmp = (Clause_t **)malloc(sizeof(Clause_t*)*old_length);
		memcpy(tmp, S->watches[Lit_index(p)].base, sizeof(Clause_t*)*old_length);

		S->watches[Lit_index(p)].length = 0;
		//现在想来应该倒着观察，因为新的学习子句都在后面，可能提前出现冲突，引导回溯
		for (i = old_length - 1; i >= 0; i--) {
			i;
			if (!Clause_propagate(tmp[i], S, p)) {
				Clause_t * conf = tmp[i];
				for (i--; i >= 0; i--) 
					Solver_watches_push(S, Lit_index(p), tmp[i]);
				PropQ_clear(S->PropQ);//清空当前的传播数组
				free(tmp);
				return conf;
			}//end_if
		}//end_for

		/*正序
		for (i = 0; i < old_length; i++) {
			if (!Clause_propagate(tmp[i], S, p)) {
				Clause_t * conf = tmp[i];
				for(i++; i< old_length;i++)
					Solver_watches_push(S, Lit_index(p), tmp[i]);
				PropQ_clear(S->PropQ);//清空当前的传播数组
				free(tmp);
				return conf;
			}//end_if
		}//end_for
		*/
		if ((int)S->watches[Lit_index(p)].length < (int)S->watches[Lit_index(p)].sapceallow_length - 2 * SPACE_INC) {//空间收缩
			S->watches[Lit_index(p)].sapceallow_length = S->watches[Lit_index(p)].length + SPACE_INC;
			S->watches[Lit_index(p)].base = (Clause_t **)realloc(S->watches[Lit_index(p)].base, sizeof(Clause_t *)*(S->watches[Lit_index(p)].sapceallow_length));
		}
		free(tmp);
	}//end_while
	
	return NULL;
}

/*进入传播队列的过程*/
bool Solver_enqueue(Solver_t * S, Lit_t p, Clause_t * from) {
	if (Solver_lit_value(S,p) != LUndef) {
		if (Solver_lit_value(S, p) == LFalse)//冲突入队
			return FALSE;
		else
			//已有的 固定的 指派，不入队
			return TRUE;
	}
	else {
		//新的fact，记录
		S->assigns[Lit_var(p)] = Lit_sign(p) == TRUE ? LFalse:LTrue;//让这个文字为真
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

/*分析冲突并学习的过程*/
void Solver_analyze(Solver_t * S, Clause_t * confl, Lit_t ** out_learnt, int * learnt_len, int * out_btlevel) {
	//print_cla(confl);
	bool * seen = (bool *)malloc(sizeof(bool)*(1+Solver_nVars(S)));
	memset(seen, 0, sizeof(bool)*(1 + Solver_nVars(S)));//将所有的变量都作为false
	int counter = 0;
	Lit_t p;
	p.idx_sig = 0;//表示未定义的文字
	Lit_t * p_reason = NULL;
	int p_reason_len = 0;

	//leave room for the asserting literal
	*out_learnt = (Lit_t *)malloc(sizeof(Lit_t)*SPACE_INC);
	int spaceallowed_out_len = SPACE_INC;
	out_learnt[0]->idx_sig = 0;
	//out_learnt.push();这个是留给最后一行用的
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

		Clause_calcReason(confl, S, p, &p_reason, &p_reason_len);//常量： confl != NULL

		//追踪 p 的 reason
		int j;
		for (j = 0; j < p_reason_len; j++) {
			Lit_t q = p_reason[j];
			if (!seen[Lit_var(q)]) {
				seen[Lit_var(q)] = TRUE;
				if (S->level[Lit_var(q)] == Solver_decisionLevel(S))//如果是decision level
					counter++;
				else if (S->level[Lit_var(q)] > 0)//排除决策level 0 的变量
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
		/*选择另一个文字来观察*/
		do {
			p = S->trail[S->trail_num - 1];
			confl = S->reason[Lit_var(p)];
			//printf("in analyze\n");
			Solver_undoONE(S);
		} while (!seen[Lit_var(p)]);//直到找到一个属于上面reason的变量

		counter--;
	} while (counter > 0);//end_do_while
	(*out_learnt)[0] = re_Lit(p);
	free(seen);
}

/*数组中decision level最高的文字（在这个数组中的下标*/
int Lit_of_highest_decisionlevel(Solver_t * S, int lit_num, Lit_t * base) {
	//这里是从0 or 1 开始
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

/*记录学习子句并引导回溯*/
void Solver_record(Solver_t * S, Lit_t * clause, int len) {
	//因为没有引用，这里写法不同
	Clause_t *c; // 创造的子句，如果clause是unit，则是NULL
	Clause_new(S, len, clause, TRUE,&c);//在这里不可能fail
	Solver_enqueue(S, clause[0], c);//在这里不可能fail
	S->polarity[Lit_var(clause[0])] = S->polarity[Lit_var(clause[0])] ? FALSE : TRUE;
	//print_cla(c);
	if (c) {//学习子句不为空，则写入
		//一开始就用的上面的空间，所以直接加就好
		if (S->learnts_num == S->spaceallowed_learnts_num)//增加空间
		{
			S->spaceallowed_learnts_num += SPACE_INC;
			S->learnts = (Clause_t **)realloc(S->learnts, sizeof(Clause_t*)*S->spaceallowed_learnts_num);
		}
		(S->learnts[S->learnts_num]) = c;
		S->learnts_num++;
	}//end_if c
}

/*回跳一步*/
void Solver_undoONE(Solver_t * S) {
	Lit_t p = S->trail[S->trail_num - 1];
	int x = Lit_var(p);//int == var
	S->assigns[x] = LUndef;
	S->reason[x] = NULL;
	S->level[x] = -1;
	Var_Order_undo(S,(S->order),x);

	S->trail_num--; //trail.pop()
	if ((int)S->trail_num < (int)S->spaceallowed_trail_num - SPACE_INC * 2) {//空间空的很多，则收缩空间
		S->spaceallowed_trail_num = S->trail_num + SPACE_INC;
		S->trail = (Lit_t *)realloc(S->trail, sizeof(Lit_t)*(S->spaceallowed_trail_num));
	}
	
	while (S->undos[x].length > 0) {
		//撤销x的最后一步
		Clause_undo((S->undos[x].base[S->undos[x].length - 1]), S, p);//不知道这一步是要干啥啊
		S->undos[x].length--;//pop
	}
	if (S->undos[x].sapceallow_length > SPACE_INC) {
		//收缩空间
		S->undos[x].length = 0;
		S->undos[x].sapceallow_length = SPACE_INC;
		S->undos[x].base = (Clause_t **)realloc(S->undos[x].base, sizeof(Clause_t*)*SPACE_INC);
	}
}

/*指派一个文字*/
bool Solver_assume(Solver_t * S, Lit_t p) {
	if (S->trail_lim_num == S->spaceallowed_trail_lim_num) {//增加空间
		S->spaceallowed_trail_lim_num += SPACE_INC;
		S->trail_lim = (unsigned int*)realloc(S->trail_lim,sizeof(unsigned int)*S->spaceallowed_trail_lim_num);
	}
	S->trail_lim[S->trail_lim_num] = S->trail_num;//指派时，增加决策等级
	S->trail_lim_num++;
	return Solver_enqueue(S, p, NULL);//入队传播
}

/*撤销一步*/
void Solver_cancel(Solver_t * S) {
	int c = S->trail_num - (S->trail_lim[S->trail_lim_num-1]);
	//printf("c = %d\n", c);
	for (; c > 0; c--)//撤销c步
		Solver_undoONE(S);
	S->trail_lim_num--;//trail_lim.pop()
	if ((int)S->trail_lim_num < (int)S->spaceallowed_trail_lim_num - 2 * SPACE_INC)//缩减空间
	{
		S->spaceallowed_trail_lim_num = S->trail_lim_num + SPACE_INC;
		unsigned int * tmp = (unsigned int *)malloc(sizeof(unsigned int)*(S->spaceallowed_trail_lim_num));
		memcpy(tmp, S->trail_lim, sizeof(unsigned int)*(S->spaceallowed_trail_lim_num));
		free(S->trail_lim);
		S->trail_lim = tmp;
	}
}

/*撤销到level*/
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

/*减小data base，只有学习子句*/
void Solver_reduceDB(Solver_t * S) {
	unsigned int i, j;
	double lim = S->cla_inc / S->learnts_num;
	sortOnActivity(S->learnts,S);//按照活跃度排序学习子句
	Clause_t * cla = NULL;
	for (i = j = 0; i < S->learnts_num / 2; i++) {
		//前一半活跃度低的不是锁定直接删除
		if (!Clause_locked(S->learnts[i], S)) 
		{
			//Clause_remove(S->learnts[i], S);
			cla = S->learnts[i];
			removeElem(cla, S, re_Lit(cla->base[0]));
			removeElem(cla, S, re_Lit(cla->base[1]));//删除观察数组中的这个元素
			free(cla->base);//删除这个子句申请文字的空间
			free(cla); //删除这个子句
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
			removeElem(cla, S, re_Lit(cla->base[1]));//删除观察数组中的这个元素
			free(cla->base);//删除这个子句申请文字的空间
			free(cla); //删除这个子句
		}
		else
			S->learnts[j++] = S->learnts[i];
	}
	//shrink learnts_size
	S->learnts_num = j;
	S->spaceallowed_learnts_num = j + SPACE_INC;
	S->learnts = (Clause_t **)realloc(S->learnts, sizeof(Clause_t*)*(j + SPACE_INC));
}

/*根据已知简化data base*/
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
				removeElem(cla, S, re_Lit(cla->base[1]));//删除观察数组中的这个元素
				free(cla->base);//删除这个子句申请文字的空间
				free(cla); //删除这个子句
			}
			else
				cs[j++] = cs[i];
		}
		cs = (Clause_t **)realloc(cs, sizeof(Clause_t*)*(j+SPACE_INC));//空间减小，原始地址不改变，只是占用的空间减小了
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

/*解题的入口，solve函数*/
void Solver_solve(Solver_t * S) {
	printf("\n****************************SOLVING****************************\n");
	if (!S)
		return;
	int begintime, endtime;/*计时*/
	double var_decay = 0.95;
	double cla_decay = 0.999;
	double nof_conflicts = 100;//Solver_nConstraints(S)/S->var_num + 100;
	double nof_learnts = Solver_nConstraints(S)/3;
	Lbool status = LUndef;
	S->root_level = Solver_decisionLevel(S);

	/*预处理*/
	Solver_simplifyDB(S);
	Solver_staticVarOrder(S);

	/*Solve*/
	while (status == LUndef) {
		begintime = clock();/*计时开始*/
		printf("allowed CONFL = %d, ", (int)nof_conflicts);//此次尝试允许的冲突数量
		status = Solver_search(S, (int)nof_conflicts, (int)nof_learnts, var_decay, cla_decay);
		nof_conflicts *= 1.5;
		nof_learnts *= 1.1; 	

		endtime = clock();	/*计时结束*/
		printf("VAR assigned %d, time = %d \n",  S->trail_lim[0], endtime - begintime);//此次确定的赋值
		Var_Order_updataAll(S, S->order);
  	}
	Solver_cancelUntil(S, 0);
	S->sta = status;//记录解答状态
}

/*搜索过程，可能还有两个decay参数*/
Lbool Solver_search(Solver_t * S,  int nof_confilt,  int nof_learnts, double var_decay, double cla_decay) {
	int conflictC = 0, num_assinged = S->trail_lim[0];
	S->var_decay =  1/var_decay;
	S->cla_decay =  1/cla_decay;
	static Lit_t p = { 0 };
	while (1) {
		/*
		if (S->trail_lim[0] > num_assinged) {
			num_assinged = S->trail_lim[0];
			nof_confilt *=1.001;//增加本次的尝试可能性
		}
		else {
			nof_confilt *= 0.9999;
		}
		*/

		//printf("conflictC = %d allowed = %d\n", conflictC, nof_confilt);
		Clause_t * confl = Solver_propagate(S);
		if (confl != NULL) {//找到冲突
			conflictC++;
			Lit_t * learnt_clause;
			int learnt_len = 0;
			int backtrack_level;
			if (Solver_decisionLevel(S) == S->root_level)//根决策层的错误，表明unsat
			{
				unsigned int i;
				for (i = 1; i <= Solver_nVars(S); i++) {//保存最后的模型
					S->model[i] = (Solver_var_value(S, i) == LTrue) ? TRUE : FALSE;
				}
				return LFalse;
			}
			Solver_analyze(S, confl, &learnt_clause, &learnt_len, &backtrack_level);//分析
			Solver_cancelUntil(S, backtrack_level > S->root_level ? backtrack_level : S->root_level);//回溯
			Solver_record(S, learnt_clause, learnt_len);//记录
			//printf("learnt_len = %d\n", learnt_len);
			free(learnt_clause);
			Solver_decayActivity(S);//衰减act
		}//end_if for conflict found
		else {//No conflict
			if (Solver_decisionLevel(S) == 0) {//根决策层，简化db
				Solver_simplifyDB(S);
			}
			if (  (int)((int)S->learnts_num - (int)Solver_nAssigns(S)) > nof_learnts) {//学习子句过多
				Solver_reduceDB(S);
			}
			if (Solver_nAssigns(S) == Solver_nVars(S)) {//找到模型，保存并返回
				printf("confl = %d ",conflictC);
				unsigned int i;
				for (i = 1; i <= Solver_nVars(S); i++) {
					S->model[i] = (Solver_var_value(S, i) == LTrue)? TRUE:FALSE;
				}
				Solver_cancelUntil(S, S->root_level);
				return LTrue;
			}
			else if (conflictC >= nof_confilt) {//冲突过多，返回
				p.idx_sig = S->trail[0].idx_sig;
				Solver_cancelUntil(S, S->root_level);
				return LUndef;
			}
			else {
				//选择新的变量
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

/*返回结果到out文件*/
void Solver_reply(Solver_t * S, int time, FILE * out) {
	printf("\n***************************ANSWERING***************************\n");
	if (!S) {
		fprintf(out, "s 0\n");
		fprintf(out, "t %d\n", time);//打印计时
		return;
	}
	fprintf(out,"s %d\n", S->sta);//解答状态
	fprint_ans(S,out);//打印解
	fprintf(out, "t %d\n", time);//打印计时
}


/*undo 中入栈*/
void Solver_undos_push(Solver_t * S, int var, Clause_t  * c) {
	if (S->undos[var].length == S->undos[var].sapceallow_length)
	{
		S->undos[var].sapceallow_length += SPACE_INC;
		S->undos[var].base = (Clause_t **)realloc(S->undos[var].base, sizeof(Clause_t *)*S->undos[var].sapceallow_length);
	}
	S->undos[var].base[S->undos[var].length] = c;
	S->undos[var].length++;
}

/*watch list 中入栈*/
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

/*将学习子句按活跃度从小到大排序*/
void sortOnActivity(Clause_t ** clas, Solver_t * S) {
	if ((S->learnts_num) == 0) return;//没有字句直接返回
	qsort(clas, S->learnts_num, sizeof(Clause_t *), compare);
}


/*根据基础子句计算活跃度的函数*/
void Solver_staticVarOrder(Solver_t * S){
	//clear activity
	unsigned int i,j;
	for (i = 1; i <= S->var_num; i++) {
		S->var_activity[i] = 0;
	}
	//基础的变量活跃度探索
	for (i = 0; i < S->constrs_num; i++) {
		Clause_t * c = S->constrs[i];
		double add = pow(2, -(c->Lit_num));
		for (j = 0; j < c->Lit_num; j++) {
			S->var_activity[ Lit_var(c->base[j])  ] += add;
		}
	}
	Var_Order_updataAll(S, S->order);
}

/*打印watches*/
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

/*打印答案*/
void fprint_ans(Solver_t * S,FILE *out) {
	if (S->sta == LTrue) {
		fprintf(out, "v ");
		int i;
		for (i = 1; i <= S->var_num; i++)
			fprintf(out, "%d ", S->model[i] ? i : -i );
		fprintf(out, "\n");
	}
}

/*测试答案*/
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

/*输出测试*/
void Solver_test(Solver_t * S) {
	printf("\n**************************SAT_TESTING**************************\n");
	if (S && test_ans(S)) {
		printf("right\n");
	}
	else printf("wrong\n");
}