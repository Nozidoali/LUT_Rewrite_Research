#ifndef WHY_LIMIT_H
#define WHY_LIMIT_H

#include "abc_aig.h"
#include "abc_ntk.h"
#include "abc_convert.h"

void WHY_Rewrite( Vec_Ptr_t * vNodes, int worktime );
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2 );
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2, Abc_Obj_t * pObj3 );
void WHY_LimitFanout( Abc_Ntk_t * pNtk, int limit );
void WHY_LimitFanout_Iter( Abc_Ntk_t * pNtk, int limit );

#endif