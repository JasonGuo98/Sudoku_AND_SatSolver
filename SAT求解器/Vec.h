#pragma once
#ifndef VEC_INCLUDE_H
#define VEC_INCLUDE_H
#include "Clause.h"

struct Vec_s {
	Clause_t ** base;//数组的基地址，数组长度为length，数组中每个元素都是一个子句的指针
	unsigned int length;//每个变量对应的子句的数量
	int sapceallow_length;//空间允许的字句数量
};
#endif // !VEC_INCLUDE_H
