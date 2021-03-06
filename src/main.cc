#include "cmdline.h"
#include "abc_parser.h"
#include "abc_write.h"
#include "abc_convert.h"
#include "abc_rewrite.h"
#include "abc_frame.h"
#include "contest_limit.h"
#include "contest_show.h"
#include "contest_rewrite.h"
#include "why_limit.h"
#include <iomanip>

using namespace std;
using namespace cmdline;


parser Cmdline_Parser( int argc, char * argv[] )
{
    parser option;
    option.add < string > ( "input",  'i', "Input BLIF file (SOP style)",  true, "./benchmarks/c17.blif");
    option.add < string > ( "output", 'o', "Output BLIF file (SOP style)", false, "");
    option.add < int >    ( "limit",  'l', "Maximum fanout Constraint",    false, 2);
    option.parse_check( argc, argv );
    return option;
}


int main( int argc, char * argv[] )
{
    // command line parser
    parser option = Cmdline_Parser( argc, argv );
    string input = option.get <string> ( "input" );
    string output = option.get <string> ( "output" );
    int limit = option.get <int> ("limit");
    if ( output == "" ) {
        int npos = input.find( ".blif" );
        if ( npos == static_cast < int >( string::npos ) )
            output = "output.blif";
        else {
            output = input;
            output.replace( npos, 5, "_constraint_" );
            stringstream ss;
            string str;
            ss << limit;
            ss >> str;
            output += str;
            output += ".blif";
        }
    }
    // cout << "The AIG with maximum fanout constraint is in \"" << output << "\"" << endl;

    // initialize
    Abc_Start();
    Abc_Ntk_t * pNtkNetlist = Io_ReadBlif( const_cast <char *>(input.c_str()), 1 );
    int size0 = Abc_NtkNodeNum( pNtkNetlist );
    int level0 = Abc_NtkLevel( pNtkNetlist );
    Abc_Ntk_t * pNtkLogic = Abc_NtkToLogic( pNtkNetlist );
    Contest_PrintStats( pNtkLogic, true );
    Abc_NtkDelete( pNtkNetlist );

    cout << "The network is " << Abc_NtkGetFaninMax( pNtkLogic ) << "-bounded" << endl;
    Vec_Ptr_t * violation = Contest_CollectFanoutExceedNodes( pNtkLogic, limit, false );

    // operations on AbC network.
    Mem_Flex_t * pMan = Mem_FlexStart();

    cout << "delay:" << std::setprecision(5) << WHY_Evaluate( pNtkLogic ) << endl;
    WHY_Rewrite( pMan, pNtkLogic, violation, 1000 );
    cout << "delay:" << std::setprecision(5) << WHY_Evaluate( pNtkLogic ) << endl;

    Vec_PtrFree( violation );

    pNtkNetlist = Abc_NtkToNetlist( pNtkLogic );
    Abc_NtkDelete( pNtkLogic );
    Io_WriteBlif( pNtkNetlist, const_cast < char * >( output.c_str() ), 0, 0, 0 );
    Abc_NtkDelete( pNtkNetlist );

    Abc_Stop();
    Mem_FlexStop( pMan, 0 );
    return 0;
}
