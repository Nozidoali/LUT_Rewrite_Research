#include "why_limit.h"
#include <iostream>
#include <map>
#include <utility>
#include <algorithm>
#include <fstream>

using namespace std;
int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj, Abc_Obj_t * pObj1 ) {
    Abc_Obj_t * pObjFanout;
    int i;
    map<int, bool> mem;
    mem.clear();
    Abc_ObjForEachFanin( pObj, pObjFanout, i ) {
        if (Abc_ObjId(pObjFanout) != Abc_ObjId(pObj))
            mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj1, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    return mem.size(); 
}

int WHY_CheckFaninOverlap ( Abc_Obj_t * pObj, Abc_Obj_t * pObj1, Abc_Obj_t * pObj2 ) {
    Abc_Obj_t * pObjFanout;
    int i;
    map<int, bool> mem;
    mem.clear();
    Abc_ObjForEachFanin( pObj1, pObjFanout, i ) {
        if (Abc_ObjId(pObjFanout) != Abc_ObjId(pObj))
            mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj2, pObjFanout, i ) {
        if (Abc_ObjId(pObjFanout) != Abc_ObjId(pObj))
            mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    Abc_ObjForEachFanin( pObj, pObjFanout, i ) {
        mem[ Abc_ObjId(pObjFanout) ] = true;
    }
    return mem.size(); 
}

void WHY_Rewrite( Abc_Ntk_t * pNtk, Vec_Ptr_t * vNodes, int worktime ) {
    Abc_Obj_t * pObj;
    int i;
    int count = 0;
    ofstream fout;
    fout.open("adder.out",ios::out);
    float results[worktime];
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pObj, i )
    {
        if ( worktime <= count++ )
            break;
        // find the "closest two fanout"
        pair<int,Abc_Obj_t *> * fanoutList = new pair<int,Abc_Obj_t *> [Abc_ObjFanoutNum(pObj)];
        int score = Abc_ObjFanoutNum(pObj), edit = 0;
        Abc_Obj_t * pObjFanout;
        int j;
        Abc_ObjForEachFanout( pObj, pObjFanout, j ) {
            // fanoutList[count++] = make_pair( WHY_CheckFaninOverlap( pObj, pObjFanout ), pObjFanout );
            if ( WHY_CheckFaninOverlap( pObj, pObjFanout )<= 6 ) {
                edit++;
            }
        }
        results[i] = (float)edit/(float)score;
        // sort( fanoutList, fanoutList+count );
        // cout << Abc_ObjId( pObj ) << "(" << Abc_ObjFanoutNum(pObj)  << ") ";
        // cout << "[" << WHY_CheckFaninOverlap( pObj, fanoutList[0].second, fanoutList[1].second ) << "] " << endl;
        // for (int k=0;k<count;k++) {
        //     cout << " : " << Abc_ObjId( fanoutList[k].second ) << "(" << fanoutList[k].first  << ")";
        //     if (fanoutList[k].first <= 6) {
        //         // merge two nodes: pObj & fanoutList[k]
        //         Abc_Obj_t * node =  Abc_NtkCreateNode ( pNtk );
        //     }
        // }
        // cout << endl;
        // cout << "-------------------------" << endl;
        delete [] fanoutList;
    }
    sort( results, results+count, greater<float>() );
    for (int i=0;i<count;i++) {
        cout << ".";
        fout << results[i] << endl;
    }
    fout.close();
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