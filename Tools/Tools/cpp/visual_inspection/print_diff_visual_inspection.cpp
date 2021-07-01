#include <iostream>

#include "data.h"
#include "visual_inspection.h"

using namespace std;

int main( int argc, char* argv[] )
{
	string vis_file = string( argv[1] );
	VecS X = read_desc_file( string( argv[2] ) );
	SMatF* score_mat1 = new SMatF( string( argv[3] ) );
	SMatF* score_mat2 = new SMatF( string( argv[4] ) );
	SMatF* X_Y = new SMatF( string( argv[5] ) );
	VecS Y = read_desc_file( string( argv[6] ) );
	bool weighted = (bool)stoi( string( argv[7] ) );
	int k = stoi( string( argv[8] ) );
	int diff_type = stoi( string( argv[9] ) );

	print_diff_visual_inspection( vis_file, X, score_mat1, score_mat2, X_Y, Y, weighted, k, diff_type );

	delete score_mat1;
	delete score_mat2;
	delete X_Y;

	return 0;
}