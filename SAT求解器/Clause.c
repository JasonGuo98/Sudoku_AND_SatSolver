#include "Clause.h"

/*将lit_num 个base中的文字写入*out_clause中，指针未分配空间，若子句已经满足or未满足，则返回FALSE*/
bool Clause_new(Solver_t * S,int Lit_num, Lit_t * Lit_base, bool learnt, Clause_t  ** out_clausepp) {
	*out_clausepp = NULL;
	if (!learnt) {//只有基本子句才执行
		int i, j = 0, index = -1;
		//不排序
		for (i = 0; i < Lit_num; i++) {
			if (Solver_lit_value(S, Lit_base[i]) == LTrue)//如果在子句中已经有值为真的文字，返回TRUE
				return TRUE;
			if (Solver_lit_value(S, Lit_base[i]) == LFalse)	//跳过假文字
				continue;
			else if (exist_var(Lit_base, j, Lit_base[i],&index))//对于重复变量，相同的位置记录于index中
			{
				if (Lit_base[index].idx_sig == Lit_base[i].idx_sig) {//重复文字
					continue;
				}
				else {//相反文字
					return TRUE;
				}
			}
			else
				Lit_base[j++] = Lit_base[i];//什么也没有发生则，将没有跳过的文字`写入
		}//end_for
		//在外面free
		Lit_num = j;
	}//end_if_!learnt

	if (Lit_num == 0) {
		return FALSE;//当已经没有文字返回错误
	}
	else if (Lit_num == 1) {
		return Solver_enqueue(S, Lit_base[0], NULL);//返回唯一个文字入队的情况，且可以用于传播，
	//这里必然返回true
	}
	else {//初始化新子句i
		Clause_t * out_clause = (Clause_t *)malloc(sizeof(Clause_t));
		*out_clausepp = out_clause;//指向
		out_clause->learnt = learnt;
		out_clause->activity = 0;
		out_clause->Lit_num = Lit_num;
		out_clause->base = (Lit_t *)malloc(Lit_num * sizeof(Lit_t));//这里的内存处理方法值得考量
		memcpy(out_clause->base, Lit_base, Lit_num * sizeof(Lit_t)); //复制过去，原本的未free

		if (learnt) {//如果是学习到的子句
			//选择一个作为第二个的文字来观察（关注，第一个应该是固定的
			int max_i = Lit_of_highest_decisionlevel(S,Lit_num, Lit_base);//传入参数是原始的参数，返回所有变量中具有最高decision_level的一个的在base中的下标
			out_clause->base[1] = Lit_base[max_i], out_clause->base[max_i] = Lit_base[1];//在两个数组中交换元素

			//增加活跃度
			Solver_claBumpActivity(S, out_clause);//新创建的子句要考虑活跃度
			int i;
			/*假设不考虑变量活跃度，很多解题变慢*/
			for (i = 0; i < Lit_num; i++) {//考虑变量的活跃度
				Solver_varBumpActivity(S,Lit_var(Lit_base[i]));
			}
			

		}//end_if_learn

		/*将新子句加入watcher list*/
		Solver_watches_push(S, Lit_index(re_Lit(out_clause->base[0])), out_clause);//将这个子句加入，第0个元素的负文字对应的watches list中
		Solver_watches_push(S, Lit_index(re_Lit(out_clause->base[1])), out_clause);//将这个子句加入，第1个元素的负文字对应的watches list中

		return TRUE;
	}//end_else
}

/*返回是否锁定*/
bool Clause_locked(Clause_t * cla,Solver_t * S) {
	//如果watch到的子句就是该子句，说明该子句是关键的子句
	if ((S->reason[Lit_var(cla->base[0])]) == cla)
		return TRUE;
	return FALSE;
	//二者相等的判据是二者的指针相同（地址相同
}

/*删除子句*/
void Clause_remove(Clause_t * cla, Solver_t * S) {
	removeElem(cla, S, re_Lit(cla->base[0]));
	removeElem(cla, S, re_Lit(cla->base[1]));//删除观察数组中的这个元素

	//自身空间清空
	free(cla->base);//删除这个子句申请文字的空间

	if (cla->learnt ==TRUE) {
		//指针相减操作会对其结果除以该指针所代表的数据类型的字节数
		unsigned int i, j;
		for (i = 0, j = 0; i < S->learnts_num; i++)
		{
			if (S->learnts[i] == cla) i++;
			S->learnts[j++] = S->learnts[i];//这里有更快的直接复制方法
		}
		S->learnts_num--;
		if ((int)S->learnts_num <= (int)S->spaceallowed_learnts_num - 2*SPACE_INC)//减小空间
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
			S->constrs[j++] = S->constrs[i];//这里有更快的直接复制方法
		}
		S->constrs_num--;
	}
}

/*化简子句，只在decision level == root level执行*/
bool Clause_simplify(Clause_t * cla, Solver_t * S) {
	int j = 0,i;
	for (i = 0; i < cla->Lit_num; i++) {
		if (Solver_lit_value(S,cla->base[i]) == LTrue) {
			return TRUE;
		}
		else if (Solver_lit_value(S, cla->base[i]) == LUndef)
			cla->base[j++] = cla->base[i];//只有i>=2才执行
	}
	cla->Lit_num = j;
	cla->base = (Lit_t*)realloc(cla->base, j * sizeof(Lit_t));
	return FALSE;
}

/*传播子句*/
bool Clause_propagate(Clause_t * clas,  Solver_t * S,Lit_t p) {
	/*
	这个字句一定是特殊的字句，说不定是p的观察字句之一，所以当p为真才执行这里
	若第一个也是p，则满足
	若第一个是 ！p，看第二个观察文字是否是满足，且对调0,1位置
	所以找新的观察文字则！p必然在1，且不满足
	*/
	/*保证假文字在【1】*/
	if (clas->base[0].idx_sig == re_Lit(p).idx_sig)
	{
		clas->base[0] = clas->base[1];
		clas->base[1] = re_Lit(p);
	}
	/*第0个元素是true，则这个子句sat*/
	if (Solver_lit_value(S, clas->base[0]) == LTrue) {
		Solver_watches_push(S, Lit_index(p), clas);//将子句再次压入栈
		return TRUE;
	}
	//找到新的观察文字
	int i;
	for (i = 2; i < clas->Lit_num; i++) {
		if (Solver_lit_value(S, clas->base[i]) != LFalse) {
			clas->base[1] = clas->base[i];
			clas->base[i] = re_Lit(p);//这里没搞懂
			Solver_watches_push(S, Lit_index(re_Lit(clas->base[1])), clas);//这个这个字句目前作为文字1的观察字句
			return TRUE;//不是单元子句时的出口
		}
	}
	/*当在当前的指派下，clause 是单原子句执行下面*/
	Solver_watches_push(S, Lit_index(p), clas);
	return Solver_enqueue(S, clas->base[0], clas);
}

/*计算reason*/
void Clause_calcReason(Clause_t * clas,  Solver_t * S, Lit_t p, Lit_t ** outreason, int * reason_len) {
	//常量：p==Undef or p == clas->base[0]
	int i, j;
	i = (p.idx_sig == 0) ? 0:1;//这里的0表示undef的文字
	//原文是i = (p == undef) ? 0:1
	//p的值是未定义	则 从0开始
	//若p的就是字句的第一个文字，从1开始（跳过p
	*reason_len = clas->Lit_num - i;//reason的长度
	(*outreason) = (Lit_t *)malloc(sizeof(Lit_t ) * (*reason_len));
	
	/* 将剩下的文字写入outreason中*/
	for (j = 0; i < clas->Lit_num; i++) {
		(*outreason)[j++] = re_Lit(clas->base[i]);
	}
	if (clas->learnt)//学习子句增加活跃度
		Solver_claBumpActivity(S, clas);
}

/*undo，default to do nothing*/
void Clause_undo(Clause_t * clas, Solver_t * S, Lit_t  p) {
	return;//default to do nothing
}

/*删除watch数组中的clas子句*/
void removeElem(Clause_t * clas, Solver_t * S,Lit_t lit) {
	unsigned int i ,j;
	unsigned int length = S->watches[Lit_index(lit)].length;
	Vec_t * watch = &(S->watches[Lit_index(lit)]);
	//print_watches(S);
	for (j = 0,i = 0; i < length; i++) {
		if (watch->base[i] == clas) i++;//找到了需要删除的子句
		watch->base[j++] = watch->base[i];
	}
	watch->length--;//减小保存的子句数

	if ((int)watch->length < (int)watch->sapceallow_length - 2 * SPACE_INC)//需要减小空间
	{
		watch->sapceallow_length = watch->length + SPACE_INC;
		watch->base = (Clause_t **)realloc(watch->base, sizeof(Clause_t *)*watch->sapceallow_length);
	}
	//print_watches(S);
}

/*打印一个子句*/
void print_cla(Clause_t * cla) {
	if (!cla) return;
	int i;
	for (i = 0; i < cla->Lit_num; i++) {
		printf("%d ", cla->base[i].idx_sig);
	}
	putchar('\n');
}