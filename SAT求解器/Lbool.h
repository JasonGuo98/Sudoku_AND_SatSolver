//广义bool类型，分别是 真 假 未指定
#ifndef Lbool_INCLUDE_H
#define Lbool_INCLUDE_H
typedef enum Lbool
{
	LTrue = 1,
	LFalse = 0,
	LUndef = -1
} Lbool;
#endif // !Lbool_INCLUDE_H
