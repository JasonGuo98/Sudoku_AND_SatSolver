#pragma once
#ifndef VEC_INCLUDE_H
#define VEC_INCLUDE_H
#include "Clause.h"

struct Vec_s {
	Clause_t ** base;//����Ļ���ַ�����鳤��Ϊlength��������ÿ��Ԫ�ض���һ���Ӿ��ָ��
	unsigned int length;//ÿ��������Ӧ���Ӿ������
	int sapceallow_length;//�ռ�������־�����
};
#endif // !VEC_INCLUDE_H
