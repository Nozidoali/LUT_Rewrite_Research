#include "why_limit.h"
#include <iostream>
#include <map>
#include <utility>
#include <algorithm>
#include <fstream>

using namespace std;

void Abc_ObjTransferFanout( Abc_Obj_t * pNodeFrom, Abc_Obj_t * pNodeTo ) {
    Vec_Ptr_t * vFanouts;
    int nFanoutsOld, i;
    assert( !Abc_ObjIsComplement(pNodeFrom) );
    assert( !Abc_ObjIsComplement(pNodeTo) );
    assert( !Abc_ObjIsPo(pNodeFrom) && !Abc_ObjIsPo(pNodeTo) );
    assert( pNodeFrom->pNtk == pNodeTo->pNtk );
    assert( pNodeFrom != pNodeTo );
    assert( !Abc_ObjIsNode(pNodeFrom) || Abc_ObjFanoutNum(pNodeFrom) > 0 );
    // get the fanouts of the old node
    nFanoutsOld = Abc_ObjFanoutNum(pNodeTo);
    vFanouts = Vec_PtrAlloc( nFanoutsOld );
    Abc_NodeCollectFanouts( pNodeFrom, vFanouts );
    // patch the fanin of each of them
    for ( i = 0; i < vFanouts->nSize; i++ )
        Abc_ObjPatchFanin( (Abc_Obj_t *)vFanouts->pArray[i], pNodeFrom, pNodeTo );
    assert( Abc_ObjFanoutNum(pNodeFrom) == 0 );
    assert( Abc_ObjFanoutNum(pNodeTo) == nFanoutsOld + vFanouts->nSize );
    Vec_PtrFree( vFanouts );
}

/**
 * WHY_PushNode
 * 
 * Push the logic of pObj1 into pObj2, remove pObj2 original nodes and insert a new node
 * Note that pObj2 might have pObj1 as fanin, but reverse will cause error
 * 
 * pObj1 is the resulting obj
 * [this will avoid assertion error of PO]
 */
void  WHY_PushNode( Mem_Flex_t * pMan, Abc_Ntk_t * pNtk, Abc_Obj_t * pObjTo, Abc_Obj_t * pObjFrom ) {
    int id, i;
    Vec_Int_t * fanin = Vec_IntDup( Abc_ObjFaninVec( pObjTo ) );
    
    Abc_ObjDeleteFanin( pObjTo, pObjFrom );

    Abc_ObjForEachFaninId(pObjFrom, id, i) {
        if (Vec_IntFind(fanin, id) == -1) {
            // push
            Vec_IntPush(fanin, id);
            Abc_ObjAddFanin( pObjTo, Abc_NtkObj( pNtk, id ) );
        }
    }    

    int nVar = Abc_ObjFaninNum( pObjTo );

    char buffer[nVar+5];
    for(int j=0;j<nVar;j++)
        buffer[j] = '1';
    buffer[nVar] = ' ';
    buffer[nVar+1] = '1';
    buffer[nVar+2] = '\n';
    buffer[nVar+3] = 0;
    pObjTo->pData = Abc_SopRegister( pMan, buffer );

    Abc_NtkDeleteObj( pObjFrom );
    Vec_IntFree(fanin);
}

/**
 * WHY_MergeNode
 * 
 * Push the logic of pObj1 into pObj2, remove the 2 original nodes and insert a new node
 * Note that pObj2 might have pObj1 as fanin, but reverse will cause error
 */
Abc_Obj_t *  WHY_MergeNode( Mem_Flex_t * pMan, Abc_Ntk_t * pNtk, Abc_Obj_t * pObj1, Abc_Obj_t * pObj2 ) {
    Abc_Obj_t * pObj =  Abc_NtkCreateNode( pNtk );

    Vec_Int_t * fanin = Vec_IntDup( Abc_ObjFaninVec( pObj1 ) );
    int id, i, pos;
    Abc_ObjForEachFaninId(pObj1, id, i) {
        Abc_ObjAddFanin( pObj, Abc_NtkObj( pNtk, id ) );
    }
    Abc_ObjForEachFaninId(pObj2, id, i) {
        if (id == Abc_ObjId(pObj1)) {
            pos = i;
            continue;
        }
        if (Vec_IntFind(fanin, id) == -1) {
            // push
            Vec_IntPush(fanin, id);
            Abc_ObjAddFanin( pObj, Abc_NtkObj( pNtk, id ) );
        }
    }
    // bug when for each fanout
    Abc_ObjTransferFanout(pObj2, pObj);

    int nVar = Abc_ObjFaninNum( pObj );

    char buffer[nVar+5];
    for(int j=0;j<nVar;j++)
        buffer[j] = '1';
    buffer[nVar] = ' ';
    buffer[nVar+1] = '1';
    buffer[nVar+2] = '\n';
    buffer[nVar+3] = 0;
    pObj->pData = Abc_SopRegister( pMan, buffer );
    
    Abc_NtkDeleteObj( pObj1 );
    Abc_NtkDeleteObj( pObj2 );
    Vec_IntFree(fanin);

    return pObj;
} 

double WHY_Evaluate_helper( Abc_Obj_t * pObj, double * delay ) {

    // define the parameter of the delay model
    double A = 1, B = 1.0/550;

    if (delay[Abc_ObjId( pObj )] != NOTHING ) {
        // return if obj is visited
        return delay[Abc_ObjId( pObj )];
    }
    if (Abc_ObjIsCi(pObj)) {
        // return 0 if obj is PI
        delay[Abc_ObjId( pObj )] = 0;
        return 0;
    }
    Abc_Obj_t * pFanin;
    int j;
    int pId = Abc_ObjId( pObj );
    delay[pId] = 0;
    Abc_ObjForEachFanin( pObj, pFanin, j ) {
        double _delay = WHY_Evaluate_helper(pFanin, delay);
        if(!Abc_ObjIsPi(pFanin)) {
            _delay += A+B*(double)Abc_ObjFanoutNum(pFanin);
        }
        delay[pId] = max(delay[pId], _delay);
    }
    return delay[pId];
}

/**
 * WHY_Evaluete:
 *  
 * Return the information of the network, evaluate the delay basic on a 
 *      Ax+b 
 * model
 */
double WHY_Evaluate ( Abc_Ntk_t * pNtk ) {

    double * delay = new double [Abc_NtkObjNumMax( pNtk )];    // map the id of the object to the delay
    for (int i=0;i<Abc_NtkObjNumMax( pNtk );i++) {
        // initialization, -1 marks untraversed.
        delay[i] = NOTHING;
    }

    Abc_Obj_t * p;
    int i;
    double max_delay = 0;
    // topological order
    Abc_NtkForEachPo( pNtk, p, i) {
        max_delay = max(max_delay, WHY_Evaluate_helper(p, delay) );
    }
    delete [] delay;
    return max_delay;
}

/**
 * WHY_CheckFaninalp
 * 
 * return the total number of fanins shared by several pObj
 * It is the preliminary to rewrite
 */
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


/**
 * WHY_Rewrite
 * 
 * rewrite the network by simply merging the network on large fanout
 */
void WHY_Rewrite( Mem_Flex_t * pMan, Abc_Ntk_t * pNtk, Vec_Ptr_t * vNodes, int worktime ) {
    Abc_Obj_t * pObj;
    int i;
    int count = 0;
    // for each fanout violation passed by vNodes
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pObj, i )
    {
        // break if time limit exceeded
        if ( worktime <= count++ )
            break;
        // find the "closest two fanout"
        Abc_Obj_t * pObjFanout;
        int j;
        Abc_ObjForEachFanout( pObj, pObjFanout, j ) {
            if ( WHY_CheckFaninOverlap( pObj, pObjFanout )<= 6 ) {
                // duplicate the node obj
                Abc_Obj_t * _pObj = Abc_NtkDupObj( pNtk, pObj, 1 );
                int k;
                Abc_Obj_t * fanin;
                // link the fanin of pObj to _pObj
                Abc_ObjForEachFanin(pObj, fanin, k)
                    Abc_ObjAddFanin(_pObj, fanin);

                // link the fanin of pObjFanout to _pObj
                Abc_ObjPatchFanin(pObjFanout, pObj, _pObj);

                // merge two nodes
                WHY_PushNode( pMan, pNtk, pObjFanout, _pObj);
                
                // break if all of the fanouts are merged
                if ( Abc_ObjFanoutNum( pObj )<=1 ) {
                    break;
                }
            }
        }
    }
}
