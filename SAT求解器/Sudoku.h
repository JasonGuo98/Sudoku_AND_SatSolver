#ifndef SUDOKU_INCLUDE
#define SUDOKU_INCLUDE
#include "Basic_def.h"
#include "Solver.h"
#define n_given 11
typedef struct Sudoku_s Sudoku_t;

struct Sudoku_s {
	int difficult_level;//难度系数
	int randseed;//随机种子
	int game_board[9][9];//棋盘
	int ans[9][9];//解
};


extern Sudoku_t * Sudoku_new(int level, int seed);//创建数独
extern int Sudoku_assign_num(Sudoku_t * sud, int x, int y);//选择一个值给指定位置，无法赋值则返回0
extern bool Sudoku_terminalPattern(Sudoku_t * sud);//得到一种赋值，满足终盘数独，返回True
extern void Sudoku_print(Sudoku_t * sud,FILE * fp);//打印
extern void Sudoku_LRTB_dig(Sudoku_t * sud, int * x, int * y);//L2R T2B dig
extern void Sudoku_S_dig(Sudoku_t * sud, int * x, int * y);//S
extern void Sudoku_jump_S_dig(Sudoku_t * sud, int * x, int * y);//jump S
extern void Sudoku_S_dig_for_jump(Sudoku_t * sud, int * x, int * y);//普通S，针对jump S 使用，避免static函数重复
extern void Sudoku_random_dig(Sudoku_t * sud, int * x, int * y);//random select
extern bool Sudoku_unique(Sudoku_t * sud, int x, int  y);//判断在挖掉这个空之后数独解是否唯一
extern bool Sudoku_confl(Sudoku_t * sud, int x, int y, int assign);//x,y处的值是否有冲突
extern bool Sudoku_has_answer(Sudoku_t * sud);//判断是否有解
extern void Sudoku_solve(Sudoku_t * sud, Sudoku_t * temp, int * nof_not_assigns, int x, int y);
extern void Sudoku_Propagating(Sudoku_t * sud);//数字交换，行列交换，块交换，旋转

extern void Sudoku_toCNF(Sudoku_t * sud, FILE * fp);//转化为SAT问题，表述为CNF文件

extern void Sudoku_toCNF_PLUS(Sudoku_t * sud, FILE * fp);//转化为SAT问题，表述为CNF文件，增量定义

extern void Sudoku_toCNF_LITE(Sudoku_t * sud, FILE * fp);//转化为SAT问题，表述为CNF文件，轻量定义

extern void Sudoku_test(Sudoku_t * sud,Solver_t * S);//测试正确性，配合solver使用
#endif // !SUDOKU_INCLUDE
