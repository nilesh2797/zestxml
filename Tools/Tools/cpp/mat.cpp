#include <iostream>
#include "mat.h"
#include "config.h"

using namespace std;

void reindex_VecIF( VecIF& vec, VecI& index )
{
    for( _int i=0; i<vec.size(); i++ )
        vec[i].first = index[ vec[i].first ];
    return;
}

SMatF* p_copy( SMatF* inmat )
{
	SMatF* outmat = new SMatF( inmat );
	return outmat;
}

void p_add( SMatF* mat1, SMatF* mat2, float coeff )
{
	mat1->add( mat2, coeff );
}

void p_shrink_mat( SMatF* refmat, vector<int>& cols, SMatF*& s_mat, vector<int>& rows, vector<int>& countmap, bool transpose )
{
	refmat->shrink_mat( cols, s_mat, rows, countmap, transpose );
}

SMatF* p_get_rank_mat( SMatF* refmat, string order )
{
	SMatF* rankmat = refmat->get_rank_mat( order );
	return rankmat;
}

SMatF* p_transpose( SMatF* refmat )
{
	SMatF* transmat = refmat->transpose();
	return transmat;
}

SMatF* p_prod( SMatF* refmat, SMatF* mat2 )
{
	SMatF* prodmat = refmat->prod( mat2 );
	return prodmat;
}

SMatF* p_sparse_prod( SMatF* refmat, SMatF* mat2, SMatF* pat_mat )
{
	SMatF* prodmat = refmat->sparse_prod( mat2, pat_mat );
	return prodmat;
}

void p_visualize( SMatF* mat, VecS& X, VecS& Y, string file_name, string sort_type )
{
	mat->visualize( X, Y, file_name, sort_type );
}
