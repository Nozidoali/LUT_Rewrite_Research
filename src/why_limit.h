#ifndef WHY_LIMIT_H
#define WHY_LIMIT_H

#include "abc_aig.h"
#include "abc_ntk.h"
#include "abc_convert.h"

#define NOTHING -1

void WHY_Rewrite( Mem_Flex_t * pMan, Abc_Ntk_t * pNtk, Vec_Ptr_t * vNodes, int worktime );
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2 );
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2, Abc_Obj_t * pObj3 );
double WHY_Evaluate ( Abc_Ntk_t * pNtk );

#endif