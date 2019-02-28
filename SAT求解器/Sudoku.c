#include "Sudoku.h"

int random(int m) {
	return rand() % m;//將随机数控制在0~m-1之间
}

/*创建数独*/
Sudoku_t * Sudoku_new(int level, int seed) {
	if (level <= 0 || level >5)
		return NULL;
	srand(seed);/*随机数种子固定*/

	Sudoku_t * sudoku = (Sudoku_t *)malloc(sizeof(Sudoku_t));
	sudoku->difficult_level = level;
	sudoku->randseed = seed;

	/*获得终盘数独*/
	int t = 0;
	do {
		memset(sudoku->game_board, 0, sizeof(int) * 9 * 9);//清零
		t++;
	} while (!Sudoku_terminalPattern(sudoku));//拉斯维加斯 算法，直到产生正确的终盘

	memcpy(sudoku->ans, sudoku->game_board, sizeof(int) * 81);

	bool dig_able[9][9];
	int r_bound[10], c_bound[10], lower_bound;//行和列的绑定数
	int x, y, can_be_dug_num = 81, max_be_dug_num,have_dug_num = 0;
	for (x = 1; x <= 9; x++) {
		r_bound[x] = c_bound[x] = 9;
	}
	for (x = 0; x < 9; x++) 
		for (y = 0; y < 9; y++) 
			dig_able[x][y] = TRUE;
	
	void(*select_next_to_dig)(Sudoku_t *, int *, int *) = NULL;;
	switch (level) {//根据难度不同选择挖洞方式
	case 1://extremely easy
		select_next_to_dig = Sudoku_random_dig;
		max_be_dug_num = random(12) + 20;//最少剩下50个
		lower_bound = 5;
		break;
	case 2://easy
		select_next_to_dig = Sudoku_random_dig;
		max_be_dug_num = random(14) + 32;//剩下36-49
		lower_bound = 4;
		break;
	case 3://medium
		select_next_to_dig = Sudoku_jump_S_dig;
		max_be_dug_num = random(4) + 46;//剩下32-35
		lower_bound = 3;
		break;
	case 4://hard
		select_next_to_dig = Sudoku_S_dig;
		max_be_dug_num = random(4) + 50;//剩下28-31
		lower_bound = 2;
		break;
	case 5://evil
		select_next_to_dig = Sudoku_LRTB_dig;
		max_be_dug_num = random(6) + 54;//剩下22-27
		lower_bound = 0;
		break;
	}

	x = random(9);
	y = random(9);//随机开始

	while (can_be_dug_num) {
		select_next_to_dig(sudoku,&x,&y);
		if (!dig_able[x][y])//无法挖洞，跳过
			continue;
		//if (Sudoku_violate_restriction(sudoku, x, y))
		if(r_bound[y] == lower_bound || c_bound[x] == lower_bound || have_dug_num >= max_be_dug_num){
			dig_able[x][y] = FALSE;
			can_be_dug_num--;
			if (have_dug_num >= max_be_dug_num)
				break;
		}
		else if (Sudoku_unique(sudoku, x, y)) {
			sudoku->game_board[x][y] = 0;//挖空
			r_bound[y]--;
			c_bound[x]--;
			have_dug_num++;
			dig_able[x][y] = FALSE;
			can_be_dug_num--;
		}
		else {
			dig_able[x][y] = FALSE;
			can_be_dug_num--;
		}
	}

	Sudoku_Propagating(sudoku);//数字交换，行列交换，块交换，旋转
	return sudoku;
}

/*得到一种赋值，满足终盘数独，返回True*/
bool Sudoku_terminalPattern(Sudoku_t * sud) {
	/*获得终盘数独*/
	int x, y, assign, i = 0;

	/*选择n_given个数来赋给初始值，论文上选择11个*/
	while (i < n_given) {
		x = random(9);
		y = random(9);
		if (sud->game_board[x][y])
			continue;
		else
		{
			sud->game_board[x][y] = Sudoku_assign_num(sud, x, y);
			i++;
		}
	}
	/*剩下的赋值*/
	for (x = 0; x < 9; x++) {
		for (y = 0; y < 9; y++) {
			if (!sud->game_board[x][y]) {
				assign = Sudoku_assign_num(sud, x, y);
				if (assign == 0)
					return FALSE;//无法赋值返回FALSE，这样有问题，待会儿改
				sud->game_board[x][y] = assign;
			}
		}//end_for y
	}//end_for x

	return TRUE;
}

/*选择一个值给指定位置，无法赋值则返回0*/
int Sudoku_assign_num(Sudoku_t * sud, int x, int y) {
	int seen[10];
	memset(seen, 0, sizeof(int)*10);
	seen[0] = 9;//记录未被查看的个数，为0表示没有可以赋的值
	int xy,var, x_start, x_end,y_start, y_end, xx, yy;
	
	/*行,列*/
	for (xy = 0; xy < 9; xy++) {
		/*行*/
		var = sud->game_board[xy][y];
		if (var)//有值
		{
			if (!seen[var]) {//没有见过的变量记录
				seen[var]++;
				seen[0]--;
			}	
		}
		/*列*/
		var = sud->game_board[x][xy];
		if (var)//有值
		{
			if (!seen[var]) {//没有见过的变量记录
				seen[var]++;
				seen[0]--;
			}
		}
	}

	/*九宫格*/

	/*确定搜索的九宫格*/
	if (x >= 0 && x < 3)			x_start = 0;
	else if (x >= 3 && x < 6)	x_start = 3;
	else										x_start = 6;

	if (y >= 0 && y < 3)			y_start = 0;
	else if (y >= 3 && y < 6)	y_start = 3;
	else										y_start = 6;

	x_end = x_start + 3;
	y_end = y_start + 3;

	/*搜索可能值*/
	for (xx = x_start; xx < x_end; xx++) {
		for (yy = y_start; yy < y_end; yy++) {
			var = sud->game_board[xx][yy];
			if (var)//有值
			{
				if (!seen[var]) {//没有见过的变量记录
					seen[var]++;
					seen[0]--;
				}
			}
		}//end_yy
	}//end_xx

	if (!seen[0])//没有可以搜索的值
		return 0;

	int start = random(9) + 1, time = random(9) + 1;
	do{
		start = start + 1;
		if (start == 10)
			start = 1;
		if (seen[start] == 0) {
			time--;
		}
	}while (time);//最后的start就是可以放置的随机数

	return start;
}

/*打印*/
void Sudoku_print(Sudoku_t * sud,FILE * fp) {
	fprintf(fp, "c level = %d, random_seed = %d\n", sud->difficult_level, sud->randseed);
	int x, y;
	fprintf(fp, "c puzzle:\n");
	for (x = 0; x < 9; x++) {
		fprintf(fp, "c ");
		for (y = 0; y < 9; y++)
		{
			if (sud->game_board[x][y] == 0)
				fprintf(fp, ". ");
			else
				fprintf(fp, "%d ", sud->game_board[x][y]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "c answer:\n");
	for (x = 0; x < 9; x++) {
		fprintf(fp, "c ");
		for (y = 0; y < 9; y++)
		{
			if (sud->ans[x][y] == 0)
				fprintf(fp, ". ");
			else
				fprintf(fp, "%d ", sud->ans[x][y]);
		}
		fprintf(fp, "\n");
	}
}

/*L2R T2B select*/
void Sudoku_LRTB_dig(Sudoku_t * sud, int * x, int * y) {
	if ((*y) < 8) {
		(*y)++;
	}
	else if ((*x) < 8) {
		(*x)++;
		(*y) = 0;
	}
	else {
		(*y) = 0;
		(*x) = 0;
	}
}

/*S形*/
void Sudoku_S_dig(Sudoku_t * sud, int * x, int * y) {
	static int add = 1;
	if ((*y) + add < 9 && (*y) + add >= 0) {
		(*y) += add;
	}
	else if ((*x) < 8) {//x + 1, y不变，方向改变
		(*x)++;
		add = -add;
	}
	else {
		(*x) = 0;
		add = -add;
	}
}

/*跨一个的S形*/
void Sudoku_jump_S_dig(Sudoku_t * sud, int * x, int * y) {
	Sudoku_S_dig_for_jump(sud, x, y);
	Sudoku_S_dig_for_jump(sud, x, y);
}

/*普通S，针对jump S 使用，避免static函数重复*/
void Sudoku_S_dig_for_jump(Sudoku_t * sud, int * x, int * y) {
	static int add = 1;
	if ((*y) + add < 9 && (*y) + add >= 0) {
		(*y) += add;
	}
	else if ((*x) < 8) {//x + 1, y不变，方向改变
		(*x)++;
		add = -add;
	}
	else {
		(*x) = 0;
		add = -add;
	}
}

/*random select*/
void Sudoku_random_dig(Sudoku_t * sud, int * x, int * y) {
	*x = random(9);
	*y = random(9);
}

/*x,y处的值是否有冲突*/
bool Sudoku_confl(Sudoku_t * sud, int x, int y,int assign) {
	int xy, var, x_start, x_end, y_start, y_end, xx, yy;

	/*行,列*/
	for (xy = 0; xy < 9; xy++) {
		/*行*/
		var = sud->game_board[xy][y];
		if (var == assign)//有值
			return TRUE;
		/*列*/
		var = sud->game_board[x][xy];
		if (var == assign)//有值
			return TRUE;
	}

	/*九宫格*/

	/*确定搜索的九宫格*/
	if (x >= 0 && x < 3)			x_start = 0;
	else if (x >= 3 && x < 6)	x_start = 3;
	else										x_start = 6;

	if (y >= 0 && y < 3)			y_start = 0;
	else if (y >= 3 && y < 6)	y_start = 3;
	else										y_start = 6;

	x_end = x_start + 3;
	y_end = y_start + 3;

	/*搜索可能值*/
	for (xx = x_start; xx < x_end; xx++) {
		for (yy = y_start; yy < y_end; yy++) {
			var = sud->game_board[xx][yy];
			if (var == assign)//有值
			{
				return TRUE;
			}
		}//end_yy
	}//end_xx

	return FALSE;
}

/*判断在挖掉这个空之后数独解是否唯一*/
bool Sudoku_unique(Sudoku_t * sud, int x,int  y) {
	int dug_num = sud->game_board[x][y], i;
	for (i = 1; i < 10; i++) {
		sud->game_board[x][y] = 0;
		if (i == dug_num)//跳过自身
			continue;
		if (Sudoku_confl(sud, x, y, i))
			continue;
		else
			sud->game_board[x][y] = i;

		if (Sudoku_has_answer(sud)) {
			sud->game_board[x][y] = dug_num;//还原
			return FALSE;
		}
	}
	sud->game_board[x][y] = dug_num;//还原
	return TRUE;
}

/*判断是否有解*/
bool Sudoku_has_answer(Sudoku_t * sud) {
	Sudoku_t temp_sud = *sud;
	int nof_not_assigns = 81;
	Sudoku_solve(sud, &temp_sud, &nof_not_assigns, 0, 0);
	if (nof_not_assigns == 0)
		return TRUE;
	return FALSE;
}

void Sudoku_solve(Sudoku_t * sud, Sudoku_t * temp, int * nof_not_assigns,int x,int y) {
	if ((*nof_not_assigns) == 0) {
		return;
	}
	if (sud->game_board[x][y]){
		(*nof_not_assigns)--;
		int xx, yy;
		if (y < 8)
		{
			xx = x;
			yy = y + 1;
		}
		else {
			xx = x + 1;
			yy = 0;
		}
		Sudoku_solve(sud, temp, nof_not_assigns, xx, yy);
		if ((*nof_not_assigns) == 0) {
			return;
		}
		(*nof_not_assigns)++;
	}
	else {
		int seen[10];
		memset(seen, 0, sizeof(int) * 10);
		seen[0] = 9;//记录未被查看的个数，为0表示没有可以赋的值
		int xy, var, x_start, x_end, y_start, y_end, xx, yy;

		/*行,列*/
		for (xy = 0; xy < 9; xy++) {
			/*行*/
			var = temp->game_board[xy][y];
			if (var)//有值
			{
				if (!seen[var]) {//没有见过的变量记录
					seen[var]++;
					seen[0]--;
				}
			}
			/*列*/
			var = temp->game_board[x][xy];
			if (var)//有值
			{
				if (!seen[var]) {//没有见过的变量记录
					seen[var]++;
					seen[0]--;
				}
			}
		}

		/*九宫格*/
		{
			/*确定搜索的九宫格*/
			if (x >= 0 && x < 3)			x_start = 0;
			else if (x >= 3 && x < 6)	x_start = 3;
			else										x_start = 6;

			if (y >= 0 && y < 3)			y_start = 0;
			else if (y >= 3 && y < 6)	y_start = 3;
			else										y_start = 6;

			x_end = x_start + 3;
			y_end = y_start + 3;

			/*搜索可能值*/
			for (xx = x_start; xx < x_end; xx++) {
				for (yy = y_start; yy < y_end; yy++) {
					var = temp->game_board[xx][yy];
					if (var)//有值
					{
						if (!seen[var]) {//没有见过的变量记录
							seen[var]++;
							seen[0]--;
						}
					}
				}//end_yy
			}//end_xx
		}

		if (!seen[0])//没有可以搜索的值
			return ;//回溯
		else {
			int i;
			for (i = 1; i <= 9; i++) {
				if (seen[i] == 0) 
				{
					(*nof_not_assigns)--;
					temp->game_board[x][y] = i;
					int xx, yy;
					if (y < 8)
					{
						xx = x;
						yy = y + 1;
					}
					else {
						xx = x + 1;
						yy = 0;
					}
					Sudoku_solve(sud, temp, nof_not_assigns, xx, yy);
					if ((*nof_not_assigns) == 0) {//找到解
						return;
					}
					temp->game_board[x][y] = 0;//恢复
					(*nof_not_assigns)++;
				}
			}
		}
	}
}

/*数字交换，行列交换，块交换，旋转*/
void Sudoku_Propagating(Sudoku_t * sud) {
	int x, y,p,q;
	p = random(9)+1;
	q = random(9)+1;//交换p,q
	for (x = 0; x < 9; x++) {
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y] == p)
				sud->game_board[x][y] = q;
			else if (sud->game_board[x][y] == q)
				sud->game_board[x][y] = p;

			if (sud->ans[x][y] == p)
				sud->ans[x][y] = q;
			else if (sud->ans[x][y] == q)
				sud->ans[x][y] = p;
		}
	}

	int xx = random(3), x1 = random(3), x2 = random(3),tmp;
	if (x1 != x2) {//交换xx*3+x1 和xx*3+x2行
		for (y = 0; y < 9; y++) {
			tmp = sud->game_board[xx * 3 + x1][y];
			sud->game_board[xx * 3 + x1][y] = sud->game_board[xx * 3 + x2][y];
			sud->game_board[xx * 3 + x2][y] = tmp;

			tmp = sud->ans[xx * 3 + x1][y];
			sud->ans[xx * 3 + x1][y] = sud->ans[xx * 3 + x2][y];
			sud->ans[xx * 3 + x2][y] = tmp;
		}
	}

	int b1 = random(3), b2 = random(3);
	if (b1 != b2) {//交换各三行
		int tmp[27];
		memcpy(tmp, &(sud->game_board[b1 * 3][0]), sizeof(int) * 27);
		memcpy(&(sud->game_board[b1 * 3][0]), &(sud->game_board[b2 * 3][0]), sizeof(int) * 27);
		memcpy(&(sud->game_board[b2 * 3][0]), tmp, sizeof(int) * 27);

		memcpy(tmp, &(sud->ans[b1 * 3][0]), sizeof(int) * 27);
		memcpy(&(sud->ans[b1 * 3][0]), &(sud->ans[b2 * 3][0]), sizeof(int) * 27);
		memcpy(&(sud->ans[b2 * 3][0]), tmp, sizeof(int) * 27);
	}

	int rot = random(2);
	if (rot) {//旋转，这里采用翻转代替,将右上角翻转到左下角
		for (x = 0; x < 9; x++) {
			for (y = 0; y < x; y++) {
				tmp = sud->game_board[x][y];
				sud->game_board[x][y] = sud->game_board[y][x];
				sud->game_board[y][x] = tmp;

				tmp = sud->ans[x][y];
				sud->ans[x][y] = sud->ans[y][x];
				sud->ans[y][x] = tmp;
			}
		}
	}

}

/*转化为SAT问题，表述为CNF文件，增量定义*/
void Sudoku_toCNF_PLUS(Sudoku_t * sud,FILE * fp) {
	Sudoku_print(sud, fp);
	int nassigned = 0;
	int x, y, i,j ,t = 0;
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				nassigned++;
			}
		}
	}

	fprintf(fp, "p cnf 729 %d\n",nassigned*9+(81-nassigned)*37 + 36* 81*2+ 3*81);
	
	//(x*81+y*9)+i 表示x y 取 i
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				for (i = 1; i <= 9; i++) {
					if (i != sud->game_board[x][y]){
						fprintf(fp, "-%d 0\n", (x * 81 + y * 9) + i);
						t++;
						}
					else{
						fprintf(fp, "%d 0\n", (x * 81 + y * 9) + i);
						t++;
					}
				}
			}//end_if_ sud->game_board[x][y]
			else//没有确定值只能九个一个为真，且有一个为真
			{
				for (i = 1; i <= 9; i++) {//不能有二值
					for (j = i+1; j <= 9; j++) {
						fprintf(fp, "-%d -%d 0\n", (x * 81 + y * 9) + i, (x * 81 + y * 9) + j);
						t++;
					}
				}
				/*可以不需要这个,因为已经包含在行，列的约束中了*/
				for (i = 1; i <= 9; i++) {//有一个为真
					fprintf(fp, "%d ", (x * 81 + y * 9) + i);
				}
				fprintf(fp, "0 \n");
				t++;
			}//end_else
		}
	}

	for (i = 1; i <= 9; i++) {//每个数字
		for (x = 0; x < 9; x++) {
			for (y = 0; y < 9; y++) {
				for (j = i + 1; j <= 9; j++) {//每种文字只有一次
					fprintf(fp, "-%d -%d 0\n", (x * 81 + y * 9) + i, (x * 81 + y * 9) + j);
					fprintf(fp, "-%d -%d 0\n", (y * 81 + x * 9) + i, (y * 81 + x * 9) + j);
					t += 2;
				}
			}

			for (y = 0; y < 9; y++) {//行,必须出现这个文字
				fprintf(fp, "%d ", (x * 81 + y * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
			for (y = 0; y < 9; y++) {//列
				fprintf(fp, "%d ", (y * 81 + x * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
		}
	}



	/*九宫格*/
	for (i = 1; i <= 9; i++) 
		for (x = 0; x < 3; x++) 
			for (y = 0; y < 3; y++) {
				int xx , yy;
				for (xx = 0; xx < 3; xx++) {
					for (yy = 0; yy < 3; yy++) {
						fprintf(fp, "%d ", (x*3+xx) * 81 + (y*3+yy) * 9 + i);
					}
				}
				t++;
				fprintf(fp, "0 \n");
			}
}

/*转化为SAT问题，表述为CNF文件*/
void Sudoku_toCNF(Sudoku_t * sud, FILE * fp) {
	Sudoku_print(sud, fp);
	int nassigned = 0;
	int x, y, i, j, t = 0;
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				nassigned++;
			}
		}
	}

	fprintf(fp, "p cnf 729 %d\n", nassigned * 9 + (81 - nassigned) * 37 + 3 * 81);

	//(x*81+y*9)+i 表示x y 取 i
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				for (i = 1; i <= 9; i++) {
					if (i != sud->game_board[x][y]) {
						fprintf(fp, "-%d 0\n", (x * 81 + y * 9) + i);
						t++;
					}
					else {
						fprintf(fp, "%d 0\n", (x * 81 + y * 9) + i);
						t++;
					}
				}
			}//end_if_ sud->game_board[x][y]
			else//没有确定值只能九个一个为真，且有一个为真
			{
				for (i = 1; i <= 9; i++) {//不能有二值
					for (j = i + 1; j <= 9; j++) {
						fprintf(fp, "-%d -%d 0\n", (x * 81 + y * 9) + i, (x * 81 + y * 9) + j);
						t++;
					}
				}
				for (i = 1; i <= 9; i++) {//有一个为真
					fprintf(fp, "%d ", (x * 81 + y * 9) + i);
				}
				fprintf(fp, "0 \n");
				t++;
			}//end_else
		}
	}

	for (i = 1; i <= 9; i++) {//每个数字
		for (x = 0; x < 9; x++) {
			for (y = 0; y < 9; y++) {//行
				fprintf(fp, "%d ", (x * 81 + y * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
			for (y = 0; y < 9; y++) {//列
				fprintf(fp, "%d ", (y * 81 + x * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
		}
	}

	/*九宫格*/
	for (i = 1; i <= 9; i++)
		for (x = 0; x < 3; x++)
			for (y = 0; y < 3; y++) {
				int xx, yy;
				for (xx = 0; xx < 3; xx++) {
					for (yy = 0; yy < 3; yy++) {
						fprintf(fp, "%d ", (x * 3 + xx) * 81 + (y * 3 + yy) * 9 + i);
					}
				}
				t++;
				fprintf(fp, "0 \n");
			}
}

/*转化为SAT问题，表述为CNF文件，轻量定义*/
void Sudoku_toCNF_LITE(Sudoku_t * sud, FILE * fp) {
	Sudoku_print(sud, fp);
	int nassigned = 0;
	int x, y, i, j, t = 0;
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				nassigned++;
			}
		}
	}
	int var_assigns[730];//变量的赋值
	for (i = 1; i <= 729; i++) {
		var_assigns[i] = -1;
	}

	fprintf(fp, "p cnf 729 %d\n", nassigned * 9 + (81 - nassigned) * 37 + 3 * 81);

	//(x*81+y*9)+i 表示x y 取 i
	for (x = 0; x < 9; x++) {//对于确定值的
		for (y = 0; y < 9; y++) {
			if (sud->game_board[x][y]) {
				for (i = 1; i <= 9; i++) {
					if (i != sud->game_board[x][y]) {
						fprintf(fp, "-%d 0\n", (x * 81 + y * 9) + i);
						var_assigns[(x * 81 + y * 9) + i] = 0;//false
						t++;
					}
					else {
						fprintf(fp, "%d 0\n", (x * 81 + y * 9) + i);
						var_assigns[(x * 81 + y * 9) + i] = 1;//true
						t++;
					}
				}
			}//end_if_ sud->game_board[x][y]
			else//没有确定值只能九个一个为真，且有一个为真
			{
				for (i = 1; i <= 9; i++) {//不能有二值
					for (j = i + 1; j <= 9; j++) {
						fprintf(fp, "-%d -%d 0\n", (x * 81 + y * 9) + i, (x * 81 + y * 9) + j);
						t++;
					}
				}
				for (i = 1; i <= 9; i++) {//有一个为真
					fprintf(fp, "%d ", (x * 81 + y * 9) + i);
				}
				fprintf(fp, "0 \n");
				t++;
			}//end_else
		}
	}




	for (i = 1; i <= 9; i++) {//每个数字都有出现
		for (x = 0; x < 9; x++) {
			for (y = 0; y < 9; y++) {//行
				fprintf(fp, "%d ", (x * 81 + y * 9) + i);
			}

			for (y = 0; y < 9; y++) {//行
				fprintf(fp, "%d ", (x * 81 + y * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
			for (y = 0; y < 9; y++) {//列
				fprintf(fp, "%d ", (y * 81 + x * 9) + i);
			}
			t++;
			fprintf(fp, "0 \n");
		}
	}

	/*九宫格*/
	for (i = 1; i <= 9; i++)
		for (x = 0; x < 3; x++)
			for (y = 0; y < 3; y++) {
				int xx, yy;
				for (xx = 0; xx < 3; xx++) {
					for (yy = 0; yy < 3; yy++) {
						fprintf(fp, "%d ", (x * 3 + xx) * 81 + (y * 3 + yy) * 9 + i);
					}
				}
				t++;
				fprintf(fp, "0 \n");
			}
}

/* 测试正确性*/
void Sudoku_test(Sudoku_t * sud, Solver_t * S) {
	printf("\n*************************SUDOKU_TESTING*************************\n");
	int x, y, i,ans;
	//(x*81+y*9)+i 表示x y 取 i
	for (x = 0; x < 9; x++) {
		for (y = 0; y < 9; y++) {
			ans = sud->ans[x][y];
			for (i = 1; i <= 9; i++) {
				if( (i != ans && S->model[(x * 81 + y * 9) + i] == TRUE) || (i == ans && S->model[(x * 81 + y * 9) + i] == FALSE)){
					printf("WRONG!\n");
					return;
				}
			}
		}
	}
	printf("RIGHT!\n");
}