#include "abc_sop.h"

char * Abc_SopCreateConst1( Mem_Flex_t * pMan )
{
    return Abc_SopRegister( pMan, " 1\n" );
}

char * Abc_SopCreateConst0( Mem_Flex_t * pMan )
{
    return Abc_SopRegister( pMan, " 0\n" );
}

char * Abc_SopCreateFromTruth( Mem_Flex_t * pMan, int nVars, unsigned * pTruth )
{
    char * pSop, * pCube;
    int nMints, Counter, i, k;
    if ( nVars == 0 )
        return pTruth[0] ? Abc_SopCreateConst1(pMan) : Abc_SopCreateConst0(pMan);
    // count the number of true minterms
    Counter = 0;
    nMints = (1 << nVars);
    for ( i = 0; i < nMints; i++ )
        Counter += ((pTruth[i>>5] & (1 << (i&31))) > 0);
    // SOP is not well-defined if the truth table is constant 0
    assert( Counter > 0 );
    if ( Counter == 0 )
        return NULL;
    // start the cover
    pSop = Abc_SopStart( pMan, Counter, nVars );
    // create true minterms
    Counter = 0;
    for ( i = 0; i < nMints; i++ )
        if ( (pTruth[i>>5] & (1 << (i&31))) > 0 )
        {
            pCube = pSop + Counter * (nVars + 3);
            for ( k = 0; k < nVars; k++ )
                pCube[k] = '0' + ((i & (1 << k)) > 0);
            Counter++;
        }
    return pSop;
}

char * Abc_SopRegister( Mem_Flex_t * pMan, const char * pName )
{
    char * pRegName;
    if ( pName == NULL ) return NULL;
    pRegName = Mem_FlexEntryFetch( pMan, strlen(pName) + 1 );
    strcpy( pRegName, pName );
    return pRegName;
}


int Abc_SopGetVarNum( char * pSop )
{
    char * pCur;
    for ( pCur = pSop; *pCur != '\n'; pCur++ )
        if ( *pCur == 0 )
            return -1;
    return pCur - pSop - 2;
}


int Abc_SopCheck( char * pSop, int nFanins )
{
    char * pCubes, * pCubesOld;
    int fFound0 = 0, fFound1 = 0;

    // check the logic function of the node
    for ( pCubes = pSop; *pCubes; pCubes++ )
    {
        // get the end of the next cube
        for ( pCubesOld = pCubes; *pCubes != ' '; pCubes++ );
        // compare the distance
        if ( pCubes - pCubesOld != nFanins )
        {
            fprintf( stdout, "Abc_SopCheck: SOP has a mismatch between its cover size (%d) and its fanin number (%d).\n",
                (int)(ABC_PTRDIFF_T)(pCubes - pCubesOld), nFanins );

            // modified by why
            fprintf( stdout, "%s", pSop );
            return 0;
        }
        // check the output values for this cube
        pCubes++;
        if ( *pCubes == '0' )
            fFound0 = 1;
        else if ( *pCubes == '1' )
            fFound1 = 1;
        else if ( *pCubes != 'x' && *pCubes != 'n' )
        {
            fprintf( stdout, "Abc_SopCheck: SOP has a strange character (%c) in the output part of its cube.\n", *pCubes );
            return 0;
        }
        // check the last symbol (new line)
        pCubes++;
        if ( *pCubes != '\n' )
        {
            fprintf( stdout, "Abc_SopCheck: SOP has a cube without new line in the end.\n" );
            return 0;
        }
    }
    if ( fFound0 && fFound1 )
    {
        fprintf( stdout, "Abc_SopCheck: SOP has cubes in both phases.\n" );
        return 0;
    }
    return 1;
}

#ifdef LIN
  #define ABC_CONST(number) number ## ULL 
#else // LIN64 and windows
  #define ABC_CONST(number) number
#endif

word Abc_SopToTruth( char * pSop, int nInputs )
{
    static word Truth[8] = {
        ABC_CONST(0xAAAAAAAAAAAAAAAA),
        ABC_CONST(0xCCCCCCCCCCCCCCCC),
        ABC_CONST(0xF0F0F0F0F0F0F0F0),
        ABC_CONST(0xFF00FF00FF00FF00),
        ABC_CONST(0xFFFF0000FFFF0000),
        ABC_CONST(0xFFFFFFFF00000000),
        ABC_CONST(0x0000000000000000),
        ABC_CONST(0xFFFFFFFFFFFFFFFF)
    };
    word Cube, Result = 0;
    int v, lit = 0;
    int nVars = Abc_SopGetVarNum(pSop);
    assert( nVars >= 0 && nVars <= 6 );
    assert( nVars == nInputs );
    do {
        Cube = Truth[7];
        for ( v = 0; v < nVars; v++, lit++ )
        {
            if ( pSop[lit] == '1' )
                Cube &=  Truth[v];
            else if ( pSop[lit] == '0' )
                Cube &= ~Truth[v];
            else if ( pSop[lit] != '-' )
                assert( 0 );
        }
        Result |= Cube;
        assert( pSop[lit] == ' ' );
        lit++;
        lit++;
        assert( pSop[lit] == '\n' );
        lit++;
    } while ( pSop[lit] );
    if ( Abc_SopIsComplement(pSop) )
        Result = ~Result;
    return Result;
}

int Abc_SopIsConst0( char * pSop )
{
    return pSop[0] == ' ' && pSop[1] == '0';
}


int Abc_SopIsConst1( char * pSop )
{
    return pSop[0] == ' ' && pSop[1] == '1';
}


int Abc_SopGetCubeNum( char * pSop )
{
    char * pCur;
    int nCubes = 0;
    if ( pSop == NULL )
        return 0;
    for ( pCur = pSop; *pCur; pCur++ )
        nCubes += (*pCur == '\n');
    return nCubes;
}


int Abc_SopIsExorType( char * pSop )
{
    char * pCur;
    for ( pCur = pSop; *pCur; pCur++ )
        if ( *pCur == '\n' )
            return (int)(*(pCur - 1) == 'x' || *(pCur - 1) == 'n');
    assert( 0 );
    return 0;
}


int Abc_SopIsComplement( char * pSop )
{
    char * pCur;
    for ( pCur = pSop; *pCur; pCur++ )
        if ( *pCur == '\n' )
            return (int)(*(pCur - 1) == '0' || *(pCur - 1) == 'n');
    assert( 0 );
    return 0;
}


char * Abc_SopCreateAnd2( Mem_Flex_t * pMan, int fCompl0, int fCompl1 )
{
    char Buffer[6];
    Buffer[0] = '1' - fCompl0;
    Buffer[1] = '1' - fCompl1;
    Buffer[2] = ' ';
    Buffer[3] = '1';
    Buffer[4] = '\n';
    Buffer[5] = 0;
    return Abc_SopRegister( pMan, Buffer );
}


char * Abc_SopCreateOrMultiCube( Mem_Flex_t * pMan, int nVars, int * pfCompl )
{
    char * pSop, * pCube;
    int i;
    pSop = Abc_SopStart( pMan, nVars, nVars );
    i = 0;
    Abc_SopForEachCube( pSop, nVars, pCube )
    {
        pCube[i] = '1' - (pfCompl? pfCompl[i] : 0);
        i++;
    }
    return pSop;
}


char * Abc_SopStart( Mem_Flex_t * pMan, int nCubes, int nVars )
{
    char * pSopCover, * pCube;
    int i, Length;

    Length = nCubes * (nVars + 3);
    pSopCover = Mem_FlexEntryFetch( pMan, Length + 1 );
    memset( pSopCover, '-', (size_t)Length );
    pSopCover[Length] = 0;

    for ( i = 0; i < nCubes; i++ )
    {
        pCube = pSopCover + i * (nVars + 3);
        pCube[nVars + 0] = ' ';
        pCube[nVars + 1] = '1';
        pCube[nVars + 2] = '\n';
    }
    return pSopCover;
}


void Abc_SopComplementVar( char * pSop, int iVar )
{
    char * pCube;
    int nVars = Abc_SopGetVarNum(pSop);
    assert( iVar < nVars );
    Abc_SopForEachCube( pSop, nVars, pCube )
    {
        if ( pCube[iVar] == '0' )
            pCube[iVar] = '1';
        else if ( pCube[iVar] == '1' )
            pCube[iVar] = '0';
    }
}


void Abc_SopComplement( char * pSop )
{
    char * pCur;
    for ( pCur = pSop; *pCur; pCur++ )
        if ( *pCur == '\n' )
        {
            if ( *(pCur - 1) == '0' )
                *(pCur - 1) = '1';
            else if ( *(pCur - 1) == '1' )
                *(pCur - 1) = '0';
            else if ( *(pCur - 1) == 'x' )
                *(pCur - 1) = 'n';
            else if ( *(pCur - 1) == 'n' )
                *(pCur - 1) = 'x';
            else
                assert( 0 );
        }
}
