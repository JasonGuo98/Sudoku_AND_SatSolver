#ifndef BASIC_DEF_INCLUDE
#define BASIC_DEF_IINCLUDE

#define bool int
#define TRUE 1
#define FALSE 0//这种东西定义一次就好了

#define SPACE_INC 10//栈空间的增减量

typedef struct Solver_s Solver_t;
typedef struct Vec_s Vec_t;
typedef struct Clause_s Clause_t;
typedef struct Lit_s Lit_t;
typedef struct Queue_s Queue_t;
typedef struct Var_Order_s Var_Order_t;


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#endif // !BASIC_DEF_INCLUDE


