#include <iostream>

#include "mat.h"
#include "utils.h"

using namespace std;

int main( int argc, char* argv[] )
{
    SMatF* mat = new SMatF( string( argv[1] ) );
    VecS cols = read_desc_file( string( argv[2] ) );
    VecS rows = read_desc_file( string( argv[3] ) );
    string vis_file = string( argv[4] );
    string sort_type = "";
    int max_pred = 1000;
    if( argc>5 )
    {
        sort_type = string( argv[5] );
        assertmesg( sort_type=="SECOND" || sort_type=="SECONDDESC", "ERROR: invalid 'sort_type' in 'visualize_mat' executable" );
    }

    if( argc>6 )
    {
        max_pred = stoi( string( argv[6] ) );
    }

    mat->trunc_max_nnz_per_col( max_pred );
    mat->visualize( cols, rows, vis_file, sort_type );
    delete mat;
}