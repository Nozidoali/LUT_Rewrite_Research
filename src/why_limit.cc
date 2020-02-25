#include "why_limit.h"
#include <iostream>
#include <map>
#include <utility>
#include <algorithm>

using namespace std;
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2 ) {
    Abc_Obj_t * pObjFanout;
    int i;
    map<int, bool> mem;
    mem.clear();
    Abc_ObjForEachFanin( pObj1, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj2, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    return mem.size(); 
}

int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj1, Abc_Obj_t * pObj2, Abc_Obj_t * pObj3 ) {
    Abc_Obj_t * pObjFanout;
    int i;
    map<int, bool> mem;
    mem.clear();
    Abc_ObjForEachFanin( pObj1, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj2, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj3, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    return mem.size(); 
}

void WHY_Rewrite( Vec_Ptr_t * vNodes, int worktime ) {
    Abc_Obj_t * pObj;
    int i;
    int count = 0;
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pObj, i )
    {
        if ( worktime <= count++ )
            return;
        // find the "closest two fanout"
        pair<int,Abc_Obj_t *> * fanoutList = new pair<int,Abc_Obj_t *> [Abc_ObjFanoutNum(pObj)];
        int count = 0;
        Abc_Obj_t * pObjFanout;
        int j;
        Abc_ObjForEachFanout( pObj, pObjFanout, j ) {
            fanoutList[count++] = make_pair( WHY_CheckFaninOverlap( pObj, pObjFanout ), pObjFanout );
        }
        sort( fanoutList, fanoutList+count );
        cout << Abc_ObjId( pObj ) << "(" << Abc_ObjFanoutNum(pObj)  << ") ";
        cout << "[" << WHY_CheckFaninOverlap( pObj, fanoutList[0].second, fanoutList[1].second ) << "] " << endl;
        for (int k=0;k<count;k++) {
            cout << " : " << Abc_ObjId( fanoutList[k].second ) << "(" << fanoutList[k].first  << ")";
        }

        cout << endl;
        cout << "-------------------------" << endl;
        delete [] fanoutList;
    }
}

void WHY_LimitFanout( Abc_Ntk_t * pNtk, int limit ) {
    Abc_Obj_t * pObj;
    int i;
    Abc_NtkForEachNode( pNtk, pObj, i ) {
        if ( Abc_ObjFanoutNum( pObj ) > limit ) {
            // add
        }
    }
}