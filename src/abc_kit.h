#ifndef ABC_KIT_H
#define ABC_KIT_H


#include "abc_vec.h"


typedef struct Kit_Sop_t_ Kit_Sop_t;
typedef struct Kit_Edge_t_ Kit_Edge_t;
typedef struct Kit_Node_t_ Kit_Node_t;
typedef struct Kit_Graph_t_ Kit_Graph_t;
typedef struct Kit_DsdNtk_t_ Kit_DsdNtk_t;
typedef struct Kit_DsdMan_t_ Kit_DsdMan_t;


struct Kit_Sop_t_
{
    int               nLits;          // the number of literals
    int               nCubes;         // the number of cubes
    unsigned *        pCubes;         // the storage for cubes
};


struct Kit_Edge_t_
{
    unsigned          fCompl   :  1;   // the complemented bit
    unsigned          Node     : 30;   // the decomposition node pointed by the edge
};


struct Kit_Node_t_
{
    Kit_Edge_t        eEdge0;          // the left child of the node
    Kit_Edge_t        eEdge1;          // the right child of the node
    // other info
    union { int       iFunc;           // the function of the node (BDD or AIG)
    void *            pFunc; };        // the function of the node (BDD or AIG)
    unsigned          Level    : 14;   // the level of this node in the global AIG
    // printing info
    unsigned          fNodeOr  :  1;   // marks the original OR node
    unsigned          fCompl0  :  1;   // marks the original complemented edge
    unsigned          fCompl1  :  1;   // marks the original complemented edge
    // latch info
    unsigned          nLat0    :  5;   // the number of latches on the first edge
    unsigned          nLat1    :  5;   // the number of latches on the second edge
    unsigned          nLat2    :  5;   // the number of latches on the output edge
};


struct Kit_Graph_t_
{
    int               fConst;          // marks the constant 1 graph
    int               nLeaves;         // the number of leaves
    int               nSize;           // the number of nodes (including the leaves)
    int               nCap;            // the number of allocated nodes
    Kit_Node_t *      pNodes;          // the array of leaves and internal nodes
    Kit_Edge_t        eRoot;           // the pointer to the topmost node
};


// DSD node types
typedef enum {
    KIT_DSD_NONE  = 0,  // 0: unknown
    KIT_DSD_CONST1,     // 1: constant 1
    KIT_DSD_VAR,        // 2: elementary variable
    KIT_DSD_AND,        // 3: multi-input AND
    KIT_DSD_XOR,        // 4: multi-input XOR
    KIT_DSD_PRIME       // 5: arbitrary function of 3+ variables
} Kit_Dsd_t;


// DSD node
typedef struct Kit_DsdObj_t_ Kit_DsdObj_t;
struct Kit_DsdObj_t_
{
    unsigned       Id         : 6;  // the number of this node
    unsigned       Type       : 3;  // none, const, var, AND, XOR, MUX, PRIME
    unsigned       fMark      : 1;  // finished checking output
    unsigned       Offset     : 8;  // offset to the truth table
    unsigned       nRefs      : 8;  // offset to the truth table
    unsigned       nFans      : 6;  // the number of fanins of this node
    unsigned short pFans[0];        // the fanin literals
};


// DSD network
struct Kit_DsdNtk_t_
{
    unsigned short nVars;           // at most 16 (perhaps 18?)
    unsigned short nNodesAlloc;     // the number of allocated nodes (at most nVars)
    unsigned short nNodes;          // the number of nodes
    unsigned short Root;            // the root of the tree
    unsigned *     pMem;            // memory for the truth tables (memory manager?)
    unsigned *     pSupps;          // supports of the nodes
    Kit_DsdObj_t** pNodes;          // the nodes
};


// DSD manager
struct Kit_DsdMan_t_
{
    int            nVars;           // the maximum number of variables
    int            nWords;          // the number of words in TTs
    Vec_Ptr_t *    vTtElems;        // elementary truth tables
    Vec_Ptr_t *    vTtNodes;        // the node truth tables
    // BDD representation
    // CloudManager * dd;              // BDD package
    Vec_Ptr_t *    vTtBdds;         // the node truth tables
    Vec_Int_t *    vNodes;          // temporary array for BDD nodes
};


static inline unsigned        Kit_DsdObjOffset( int nFans )          { return (nFans >> 1) + ((nFans & 1) > 0);                    }
static inline unsigned *      Kit_DsdObjTruth( Kit_DsdObj_t * pObj ) { return pObj->Type == KIT_DSD_PRIME ? (unsigned *)pObj->pFans + pObj->Offset: NULL; }
static inline int             Kit_DsdNtkObjNum( Kit_DsdNtk_t * pNtk ){ return pNtk->nVars + pNtk->nNodes; }
static inline Kit_DsdObj_t *  Kit_DsdNtkObj( Kit_DsdNtk_t * pNtk, int Id )      { assert( Id >= 0 && Id < pNtk->nVars + pNtk->nNodes ); return Id < pNtk->nVars ? NULL : pNtk->pNodes[Id - pNtk->nVars]; }
static inline Kit_DsdObj_t *  Kit_DsdNtkRoot( Kit_DsdNtk_t * pNtk )             { return Kit_DsdNtkObj( pNtk, Abc_Lit2Var(pNtk->Root) );                      }
static inline int             Kit_DsdLitIsLeaf( Kit_DsdNtk_t * pNtk, int Lit )   { int Id = Abc_Lit2Var(Lit); assert( Id >= 0 && Id < pNtk->nVars + pNtk->nNodes ); return Id < pNtk->nVars; }
static inline unsigned        Kit_DsdLitSupport( Kit_DsdNtk_t * pNtk, int Lit )  { int Id = Abc_Lit2Var(Lit); assert( Id >= 0 && Id < pNtk->nVars + pNtk->nNodes ); return pNtk->pSupps? (Id < pNtk->nVars? (1 << Id) : pNtk->pSupps[Id - pNtk->nVars]) : 0; }

#define Kit_DsdNtkForEachObj( pNtk, pObj, i )                                      \
    for ( i = 0; (i < (pNtk)->nNodes) && ((pObj) = (pNtk)->pNodes[i]); i++ )
#define Kit_DsdObjForEachFanin( pNtk, pObj, iLit, i )                              \
    for ( i = 0; (i < (int)(pObj)->nFans) && ((iLit) = (pObj)->pFans[i], 1); i++ )
#define Kit_DsdObjForEachFaninReverse( pNtk, pObj, iLit, i )                       \
    for ( i = (int)(pObj)->nFans - 1; (i >= 0) && ((iLit) = (pObj)->pFans[i], 1); i-- )

#define Kit_PlaForEachCube( pSop, nFanins, pCube )                \
    for ( pCube = (pSop); *pCube; pCube += (nFanins) + 3 )
#define Kit_PlaCubeForEachVar( pCube, Value, i )                     \
    for ( i = 0; (pCube[i] != ' ') && (Value = pCube[i]); i++ )


#define KIT_MIN(a,b)       (((a) < (b))? (a) : (b))
#define KIT_MAX(a,b)       (((a) > (b))? (a) : (b))
#define KIT_INFINITY       (100000000)

static inline int          Kit_CubeHasLit( unsigned uCube, int i )                        { return(uCube &  (unsigned)(1<<i)) > 0;  }
static inline unsigned     Kit_CubeSetLit( unsigned uCube, int i )                        { return uCube |  (unsigned)(1<<i);       }
static inline unsigned     Kit_CubeXorLit( unsigned uCube, int i )                        { return uCube ^  (unsigned)(1<<i);       }
static inline unsigned     Kit_CubeRemLit( unsigned uCube, int i )                        { return uCube & ~(unsigned)(1<<i);       }

static inline int          Kit_CubeContains( unsigned uLarge, unsigned uSmall )           { return (uLarge & uSmall) == uSmall;     }
static inline unsigned     Kit_CubeSharp( unsigned uCube, unsigned uMask )                { return uCube & ~uMask;                  }
static inline unsigned     Kit_CubeMask( int nVar )                                       { return (~(unsigned)0) >> (32-nVar);     }

static inline int          Kit_CubeIsMarked( unsigned uCube )                             { return Kit_CubeHasLit( uCube, 31 );     }
static inline unsigned     Kit_CubeMark( unsigned uCube )                                 { return Kit_CubeSetLit( uCube, 31 );     }
static inline unsigned     Kit_CubeUnmark( unsigned uCube )                               { return Kit_CubeRemLit( uCube, 31 );     }

static inline int          Kit_SopCubeNum( Kit_Sop_t * cSop )                             { return cSop->nCubes;                    }
static inline unsigned     Kit_SopCube( Kit_Sop_t * cSop, int i )                         { return cSop->pCubes[i];                 }
static inline void         Kit_SopShrink( Kit_Sop_t * cSop, int nCubesNew )               { cSop->nCubes = nCubesNew;               }
static inline void         Kit_SopPushCube( Kit_Sop_t * cSop, unsigned uCube )            { cSop->pCubes[cSop->nCubes++] = uCube;   }
static inline void         Kit_SopWriteCube( Kit_Sop_t * cSop, unsigned uCube, int i )    { cSop->pCubes[i] = uCube;                }

static inline Kit_Edge_t   Kit_EdgeCreate( int Node, int fCompl )                         { Kit_Edge_t eEdge = { static_cast < unsigned >( fCompl ), static_cast < unsigned >( Node ) }; return eEdge;  }
static inline unsigned     Kit_EdgeToInt( Kit_Edge_t eEdge )                              { return (eEdge.Node << 1) | eEdge.fCompl;            }
static inline Kit_Edge_t   Kit_IntToEdge( unsigned Edge )                                 { return Kit_EdgeCreate( Edge >> 1, Edge & 1 );       }
//static inline unsigned     Kit_EdgeToInt_( Kit_Edge_t eEdge )                             { return *(unsigned *)&eEdge;                         }
//static inline Kit_Edge_t   Kit_IntToEdge_( unsigned Edge )                                { return *(Kit_Edge_t *)&Edge;                        }
static inline unsigned     Kit_EdgeToInt_( Kit_Edge_t m )                                 { union { Kit_Edge_t x; unsigned y; } v; v.x = m; return v.y;  }
static inline Kit_Edge_t   Kit_IntToEdge_( unsigned m )                                   { union { Kit_Edge_t x; unsigned y; } v; v.y = m; return v.x;  }

static inline int          Kit_GraphIsConst( Kit_Graph_t * pGraph )                       { return pGraph->fConst;                              }
static inline int          Kit_GraphIsConst0( Kit_Graph_t * pGraph )                      { return pGraph->fConst && pGraph->eRoot.fCompl;      }
static inline int          Kit_GraphIsConst1( Kit_Graph_t * pGraph )                      { return pGraph->fConst && !pGraph->eRoot.fCompl;     }
static inline int          Kit_GraphIsComplement( Kit_Graph_t * pGraph )                  { return pGraph->eRoot.fCompl;                        }
static inline int          Kit_GraphIsVar( Kit_Graph_t * pGraph )                         { return pGraph->eRoot.Node < (unsigned)pGraph->nLeaves; }
static inline void         Kit_GraphComplement( Kit_Graph_t * pGraph )                    { pGraph->eRoot.fCompl ^= 1;                          }
static inline void         Kit_GraphSetRoot( Kit_Graph_t * pGraph, Kit_Edge_t eRoot )     { pGraph->eRoot = eRoot;                              }
static inline int          Kit_GraphLeaveNum( Kit_Graph_t * pGraph )                      { return pGraph->nLeaves;                             }
static inline int          Kit_GraphNodeNum( Kit_Graph_t * pGraph )                       { return pGraph->nSize - pGraph->nLeaves;             }
static inline Kit_Node_t * Kit_GraphNode( Kit_Graph_t * pGraph, int i )                   { return pGraph->pNodes + i;                          }
static inline Kit_Node_t * Kit_GraphNodeLast( Kit_Graph_t * pGraph )                      { return pGraph->pNodes + pGraph->nSize - 1;          }
static inline int          Kit_GraphNodeInt( Kit_Graph_t * pGraph, Kit_Node_t * pNode )   { return pNode - pGraph->pNodes;                      }
static inline int          Kit_GraphNodeIsVar( Kit_Graph_t * pGraph, Kit_Node_t * pNode ) { return Kit_GraphNodeInt(pGraph,pNode) < pGraph->nLeaves; }
static inline Kit_Node_t * Kit_GraphVar( Kit_Graph_t * pGraph )                           { assert( Kit_GraphIsVar( pGraph ) );    return Kit_GraphNode( pGraph, pGraph->eRoot.Node );      }
static inline int          Kit_GraphVarInt( Kit_Graph_t * pGraph )                        { assert( Kit_GraphIsVar( pGraph ) );    return Kit_GraphNodeInt( pGraph, Kit_GraphVar(pGraph) ); }
static inline Kit_Node_t * Kit_GraphNodeFanin0( Kit_Graph_t * pGraph, Kit_Node_t * pNode ){ return Kit_GraphNodeIsVar(pGraph, pNode)? NULL : Kit_GraphNode(pGraph, pNode->eEdge0.Node);     }
static inline Kit_Node_t * Kit_GraphNodeFanin1( Kit_Graph_t * pGraph, Kit_Node_t * pNode ){ return Kit_GraphNodeIsVar(pGraph, pNode)? NULL : Kit_GraphNode(pGraph, pNode->eEdge1.Node);     }
static inline int          Kit_GraphRootLevel( Kit_Graph_t * pGraph )                     { return Kit_GraphNode(pGraph, pGraph->eRoot.Node)->Level;                                        }

static inline int          Kit_SuppIsMinBase( int Supp )                                  { return (Supp & (Supp+1)) == 0;                      }

static inline int          Kit_BitWordNum( int nBits )    { return nBits/(8*sizeof(unsigned)) + ((nBits%(8*sizeof(unsigned))) > 0); }
static inline int          Kit_TruthWordNum( int nVars )  { return nVars <= 5 ? 1 : (1 << (nVars - 5));                             }
static inline unsigned     Kit_BitMask( int nBits )       { assert( nBits <= 32 ); return ~((~(unsigned)0) << nBits);               }

static inline void         Kit_TruthSetBit( unsigned * p, int Bit )   { p[Bit>>5] |= (1<<(Bit & 31));               }
static inline void         Kit_TruthXorBit( unsigned * p, int Bit )   { p[Bit>>5] ^= (1<<(Bit & 31));               }
static inline int          Kit_TruthHasBit( unsigned * p, int Bit )   { return (p[Bit>>5] & (1<<(Bit & 31))) > 0;   }

static inline int Kit_WordFindFirstBit( unsigned uWord )
{
    int i;
    for ( i = 0; i < 32; i++ )
        if ( uWord & (1 << i) )
            return i;
    return -1;
}
static inline int Kit_WordHasOneBit( unsigned uWord )
{
    return (uWord & (uWord - 1)) == 0;
}
static inline int Kit_WordCountOnes( unsigned uWord )
{
    uWord = (uWord & 0x55555555) + ((uWord>>1) & 0x55555555);
    uWord = (uWord & 0x33333333) + ((uWord>>2) & 0x33333333);
    uWord = (uWord & 0x0F0F0F0F) + ((uWord>>4) & 0x0F0F0F0F);
    uWord = (uWord & 0x00FF00FF) + ((uWord>>8) & 0x00FF00FF);
    return  (uWord & 0x0000FFFF) + (uWord>>16);
}
static inline int Kit_TruthCountOnes( unsigned * pIn, int nVars )
{
    int w, Counter = 0;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        Counter += Kit_WordCountOnes(pIn[w]);
    return Counter;
}
static inline int Kit_TruthFindFirstBit( unsigned * pIn, int nVars )
{
    int w;
    for ( w = 0; w < Kit_TruthWordNum(nVars); w++ )
        if ( pIn[w] )
            return 32*w + Kit_WordFindFirstBit(pIn[w]);
    return -1;
}
static inline int Kit_TruthFindFirstZero( unsigned * pIn, int nVars )
{
    int w;
    for ( w = 0; w < Kit_TruthWordNum(nVars); w++ )
        if ( ~pIn[w] )
            return 32*w + Kit_WordFindFirstBit(~pIn[w]);
    return -1;
}
static inline int Kit_TruthIsEqual( unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn0[w] != pIn1[w] )
            return 0;
    return 1;
}
static inline int Kit_TruthIsEqualWithCare( unsigned * pIn0, unsigned * pIn1, unsigned * pCare, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( (pIn0[w] & pCare[w]) != (pIn1[w] & pCare[w]) )
            return 0;
    return 1;
}
static inline int Kit_TruthIsOpposite( unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn0[w] != ~pIn1[w] )
            return 0;
    return 1;
}
static inline int Kit_TruthIsEqualWithPhase( unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    if ( (pIn0[0] & 1) == (pIn1[0] & 1) )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            if ( pIn0[w] != pIn1[w] )
                return 0;
    }
    else
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            if ( pIn0[w] != ~pIn1[w] )
                return 0;
    }
    return 1;
}
static inline int Kit_TruthIsConst0( unsigned * pIn, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn[w] )
            return 0;
    return 1;
}
static inline int Kit_TruthIsConst1( unsigned * pIn, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn[w] != ~(unsigned)0 )
            return 0;
    return 1;
}
static inline int Kit_TruthIsImply( unsigned * pIn1, unsigned * pIn2, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn1[w] & ~pIn2[w] )
            return 0;
    return 1;
}
static inline int Kit_TruthIsDisjoint( unsigned * pIn1, unsigned * pIn2, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn1[w] & pIn2[w] )
            return 0;
    return 1;
}
static inline int Kit_TruthIsDisjoint3( unsigned * pIn1, unsigned * pIn2, unsigned * pIn3, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        if ( pIn1[w] & pIn2[w] & pIn3[w] )
            return 0;
    return 1;
}
static inline void Kit_TruthCopy( unsigned * pOut, unsigned * pIn, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn[w];
}
static inline void Kit_TruthClear( unsigned * pOut, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = 0;
}
static inline void Kit_TruthFill( unsigned * pOut, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = ~(unsigned)0;
}
static inline void Kit_TruthNot( unsigned * pOut, unsigned * pIn, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = ~pIn[w];
}
static inline void Kit_TruthAnd( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] & pIn1[w];
}
static inline void Kit_TruthOr( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] | pIn1[w];
}
static inline void Kit_TruthXor( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] ^ pIn1[w];
}
static inline void Kit_TruthSharp( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = pIn0[w] & ~pIn1[w];
}
static inline void Kit_TruthNand( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = ~(pIn0[w] & pIn1[w]);
}
static inline void Kit_TruthAndPhase( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars, int fCompl0, int fCompl1 )
{
    int w;
    if ( fCompl0 && fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = ~(pIn0[w] | pIn1[w]);
    }
    else if ( fCompl0 && !fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = ~pIn0[w] & pIn1[w];
    }
    else if ( !fCompl0 && fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = pIn0[w] & ~pIn1[w];
    }
    else // if ( !fCompl0 && !fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = pIn0[w] & pIn1[w];
    }
}
static inline void Kit_TruthOrPhase( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, int nVars, int fCompl0, int fCompl1 )
{
    int w;
    if ( fCompl0 && fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = ~(pIn0[w] & pIn1[w]);
    }
    else if ( fCompl0 && !fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = ~pIn0[w] | pIn1[w];
    }
    else if ( !fCompl0 && fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = pIn0[w] | ~pIn1[w];
    }
    else // if ( !fCompl0 && !fCompl1 )
    {
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = pIn0[w] | pIn1[w];
    }
}
static inline void Kit_TruthMux( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, unsigned * pCtrl, int nVars )
{
    int w;
    for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
        pOut[w] = (pIn0[w] & ~pCtrl[w]) | (pIn1[w] & pCtrl[w]);
}
static inline void Kit_TruthMuxPhase( unsigned * pOut, unsigned * pIn0, unsigned * pIn1, unsigned * pCtrl, int nVars, int fComp0 )
{
    int w;
    if ( fComp0 )
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = (~pIn0[w] & ~pCtrl[w]) | (pIn1[w] & pCtrl[w]);
    else
        for ( w = Kit_TruthWordNum(nVars)-1; w >= 0; w-- )
            pOut[w] = (pIn0[w] & ~pCtrl[w]) | (pIn1[w] & pCtrl[w]);
}
static inline void Kit_TruthIthVar( unsigned * pTruth, int nVars, int iVar )
{
    unsigned Masks[5] = { 0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000 };
    int k, nWords = (nVars <= 5 ? 1 : (1 << (nVars - 5)));
    if ( iVar < 5 )
    {
        for ( k = 0; k < nWords; k++ )
            pTruth[k] = Masks[iVar];
    }
    else
    {
        for ( k = 0; k < nWords; k++ )
            if ( k & (1 << (iVar-5)) )
                pTruth[k] = ~(unsigned)0;
            else
                pTruth[k] = 0;
    }
}


#define Kit_SopForEachCube( cSop, uCube, i )                                      \
    for ( i = 0; (i < Kit_SopCubeNum(cSop)) && ((uCube) = Kit_SopCube(cSop, i)); i++ )
#define Kit_CubeForEachLiteral( uCube, Lit, nLits, i )                            \
    for ( i = 0; (i < (nLits)) && ((Lit) = Kit_CubeHasLit(uCube, i)); i++ )

#define Kit_GraphForEachLeaf( pGraph, pLeaf, i )                                              \
    for ( i = 0; (i < (pGraph)->nLeaves) && (((pLeaf) = Kit_GraphNode(pGraph, i)), 1); i++ )
#define Kit_GraphForEachNode( pGraph, pAnd, i )                                               \
    for ( i = (pGraph)->nLeaves; (i < (pGraph)->nSize) && (((pAnd) = Kit_GraphNode(pGraph, i)), 1); i++ )


Kit_DsdNtk_t * Kit_DsdDecompose( unsigned * pTruth, int nVars );
Kit_DsdNtk_t * Kit_DsdDecomposeInt( unsigned * pTruth, int nVars, int nDecMux );
Kit_DsdNtk_t * Kit_DsdNtkAlloc( int nVars );
Kit_DsdObj_t * Kit_DsdObjAlloc( Kit_DsdNtk_t * pNtk, Kit_Dsd_t Type, int nFans );
unsigned Kit_TruthSupport( unsigned * pTruth, int nVars );
int Kit_TruthVarInSupport( unsigned * pTruth, int nVars, int iVar );
void Kit_DsdDecompose_rec( Kit_DsdNtk_t * pNtk, Kit_DsdObj_t * pObj, unsigned uSupp, unsigned short * pPar, int nDecMux );
void Kit_TruthShrink( unsigned * pOut, unsigned * pIn, int nVars, int nVarsAll, unsigned Phase, int fReturnIn );
void Kit_TruthSwapAdjacentVars( unsigned * pOut, unsigned * pIn, int nVars, int iVar );
void Kit_TruthCofactor0New( unsigned * pOut, unsigned * pIn, int nVars, int iVar );
void Kit_TruthCofactor1New( unsigned * pOut, unsigned * pIn, int nVars, int iVar );
void Kit_TruthMuxVar( unsigned * pOut, unsigned * pCof0, unsigned * pCof1, int nVars, int iVar );
Kit_DsdObj_t * Kit_DsdNonDsdPrimeMax( Kit_DsdNtk_t * pNtk );
void Kit_DsdNtkFree( Kit_DsdNtk_t * pNtk );
void Kit_TruthPermute( unsigned * pOut, unsigned * pIn, int nVars, char * pPerm, int fReturnIn );
void Kit_TruthChangePhase( unsigned * pTruth, int nVars, int iVar );
void Kit_TruthCountOnesInCofs0( unsigned * pTruth, int nVars, int * pStore );
int Kit_TruthVarIsVacuous( unsigned * pOnset, unsigned * pOffset, int nVars, int iVar );
void Kit_TruthExist( unsigned * pTruth, int nVars, int iVar );
void Kit_TruthExistNew( unsigned * pRes, unsigned * pTruth, int nVars, int iVar );
void Kit_TruthExistSet( unsigned * pRes, unsigned * pTruth, int nVars, unsigned uMask );
void Kit_TruthForallNew( unsigned * pRes, unsigned * pTruth, int nVars, int iVar );
int Kit_TruthBestCofVar( unsigned * pTruth, int nVars, unsigned * pCof0, unsigned * pCof1 );
int Kit_TruthSupportSize( unsigned * pTruth, int nVars );


#endif
