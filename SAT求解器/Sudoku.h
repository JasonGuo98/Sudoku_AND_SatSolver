#ifndef SUDOKU_INCLUDE
#define SUDOKU_INCLUDE
#include "Basic_def.h"
#include "Solver.h"
#define n_given 11
typedef struct Sudoku_s Sudoku_t;

struct Sudoku_s {
	int difficult_level;//�Ѷ�ϵ��
	int randseed;//�������
	int game_board[9][9];//����
	int ans[9][9];//��
};


extern Sudoku_t * Sudoku_new(int level, int seed);//��������
extern int Sudoku_assign_num(Sudoku_t * sud, int x, int y);//ѡ��һ��ֵ��ָ��λ�ã��޷���ֵ�򷵻�0
extern bool Sudoku_terminalPattern(Sudoku_t * sud);//�õ�һ�ָ�ֵ��������������������True
extern void Sudoku_print(Sudoku_t * sud,FILE * fp);//��ӡ
extern void Sudoku_LRTB_dig(Sudoku_t * sud, int * x, int * y);//L2R T2B dig
extern void Sudoku_S_dig(Sudoku_t * sud, int * x, int * y);//S
extern void Sudoku_jump_S_dig(Sudoku_t * sud, int * x, int * y);//jump S
extern void Sudoku_S_dig_for_jump(Sudoku_t * sud, int * x, int * y);//��ͨS�����jump S ʹ�ã�����static�����ظ�
extern void Sudoku_random_dig(Sudoku_t * sud, int * x, int * y);//random select
extern bool Sudoku_unique(Sudoku_t * sud, int x, int  y);//�ж����ڵ������֮���������Ƿ�Ψһ
extern bool Sudoku_confl(Sudoku_t * sud, int x, int y, int assign);//x,y����ֵ�Ƿ��г�ͻ
extern bool Sudoku_has_answer(Sudoku_t * sud);//�ж��Ƿ��н�
extern void Sudoku_solve(Sudoku_t * sud, Sudoku_t * temp, int * nof_not_assigns, int x, int y);
extern void Sudoku_Propagating(Sudoku_t * sud);//���ֽ��������н������齻������ת

extern void Sudoku_toCNF(Sudoku_t * sud, FILE * fp);//ת��ΪSAT���⣬����ΪCNF�ļ�

extern void Sudoku_toCNF_PLUS(Sudoku_t * sud, FILE * fp);//ת��ΪSAT���⣬����ΪCNF�ļ�����������

extern void Sudoku_toCNF_LITE(Sudoku_t * sud, FILE * fp);//ת��ΪSAT���⣬����ΪCNF�ļ�����������

extern void Sudoku_test(Sudoku_t * sud,Solver_t * S);//������ȷ�ԣ����solverʹ��
#endif // !SUDOKU_INCLUDE
