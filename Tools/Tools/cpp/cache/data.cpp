#include "data.h"

using namespace std;

VecS read_desc_file( string fname )
{
	VecS descs;

	ifstream fin;
	fin.open( fname );
	string line;
	while(getline(fin, line))
	{
		descs.push_back( line );
	}
	fin.close();

	return descs;
}

void print_descs( VecS descs )
{
	for( int i=0; i<descs.size(); i++ )
	{
		cout << descs[i] << endl;
	}
}