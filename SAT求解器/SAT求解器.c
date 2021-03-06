// SAT求解器.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

/*需要的组件*/
#include "Basic_def.h"
#include "Solver.h"
#include "Sudoku.h"


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>   //用到clock()函数

/*从打开的cnf文件获得保存的res文件名*/
void cnf_to_res(char * res_filename, char * cnf_filename) {
	int lenth = strlen(cnf_filename), i ;
	for (i = 0; i < lenth - 3; i++) {
		res_filename[i] = cnf_filename[i];
	}
	res_filename[i++] = 'r';
	res_filename[i++] = 'e';
	res_filename[i++] = 's';
	res_filename[i++] = '\0';
}

int main()
{
	int begintime, endtime, op = -1;/*计时*/
	char filename[FILENAME_MAX], save_fileame[FILENAME_MAX];
	FILE * fp = NULL; //待读取文件指针
	Solver_t * S = NULL;
	printf("Sudoku (0) 或 SAT (1) 或 测试（2）: \n");
	scanf_s("%d", &op);
	switch (op) {
	case 0://求解数独
		printf("\n****************************IN__SUDOKU****************************\n");
		printf("输入难度系数（1-5），和随机数种子（正整数）：\n");
		int level, seed;
		scanf_s("%d %d", &level, &seed);

		begintime = clock();/*计时开始*/
		Sudoku_t * sudoku = Sudoku_new(level, seed);
		errno_t err = fopen_s(&fp, "test\\sudoku\\sud.cnf", "w") != 0;

		Sudoku_print(sudoku, stdout);//打印

		Sudoku_toCNF(sudoku,fp);
		endtime = clock();	/*计时结束*/
		printf("t %d\n", endtime - begintime);

		fclose(fp);
		printf("文件已保存到test\\sudoku\\sud.cnf\n");
		printf("\n****************************IN__DPLL****************************\n");

		fp = Cnf_openfile("test\\sudoku\\sud.cnf");/*打开文件*/

		begintime = clock();/*计时开始*/
		S = Solver_read_file(fp);/*读取*/
		fclose(fp);/*关闭打开的文件*/
		
		Solver_solve(S);/*解答*/

		endtime = clock();	/*计时结束*/
		Solver_reply(S, endtime - begintime, stdout);/*输出求解结果*/

		err = fopen_s(&fp, "test\\sudoku\\sud.res", "w") != 0;
		Solver_reply(S, endtime - begintime, fp);
		Solver_test(S);/*测试*/

		Sudoku_test(sudoku,S);//数独测试
		fclose(fp);
		printf("文件已保存到test\\sudoku\\sud.res\n");

		break;
	case 1:
		srand(123);/*随机数种子固定*/
		printf("输入文件地址\n");
		scanf_s("%s", filename, sizeof(char)*FILENAME_MAX);//带读取文件名

		fp = Cnf_openfile(filename);/*打开文件*/

		begintime = clock();/*计时开始*/
		S = Solver_read_file(fp);/*读取*/
		Solver_solve(S);/*解答*/

		endtime = clock();	/*计时结束*/
		Solver_reply(S, endtime - begintime, stdout);/*输出求解结果*/

		cnf_to_res(save_fileame,filename);//获得保存文件名

		err = fopen_s(&fp, save_fileame, "w") != 0;
		Solver_reply(S, endtime - begintime, fp);
		Solver_test(S);/*测试*/

		fclose(fp);
		printf("文件已保存到%s",save_fileame);

		Solver_test(S);/*测试*/
		break;
	case 2:
		printf("输入CNF文件地址\n");
		scanf_s("%s", filename, sizeof(char)*FILENAME_MAX);//带读取文件名
		fp = Cnf_openfile(filename);/*打开文件*/
		S = Solver_read_file(fp);/*读取*/
		fclose(fp);/*关闭CNF文件*/

		printf("输入RES文件地址\n");
		scanf_s("%s", filename, sizeof(char)*FILENAME_MAX);//带读取文件名
		fopen_s(&fp, filename, "r");
		int i,n;
		char c;
		fscanf_s(fp, "%c", &c, sizeof(char));
		fscanf_s(fp, "%d", &n);
		fscanf_s(fp, "%c", &c, sizeof(char));
		fscanf_s(fp, "%c", &c, sizeof(char));
		for (i = 1; i <= S->var_num; i++) {
			fscanf_s(fp, "%d", &n);
			if (n < 0)
				S->model[i] = FALSE;
			else
				S->model[i] = TRUE;
		}
		Solver_test(S);/*测试*/
		fclose(fp);/*关闭CNF文件*/
	}
	system("pause");
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
