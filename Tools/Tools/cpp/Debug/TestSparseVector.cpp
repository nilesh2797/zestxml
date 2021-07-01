#include <iostream>
#include <algorithm>
#include "SparseVector.h"

using namespace std;

int main()
{
    SVecF svec;
    svec.push_back( make_pair(0,1) );
    svec.push_back( make_pair(1,1) );
    cout << svec << endl;

    SVecF vec2( svec );
    cout << vec2 << endl;

    typedef pair<int,float> pairIF;
    pairIF* p = new pairIF[10];
    fill( p, p+10, make_pair(2,2) );

    SVecF vec3( 10, make_pair(3,3) );
    cout << vec3 << endl;

    SVecF vec4( 10, p );
    cout << vec4 << endl;

    SVecF vec5;
    vec5+= make_pair( 3,1 );
    vec5+= make_pair( 2,1 );
    vec5+= make_pair( 1,1 );

    vec5.sort( 0, true );
    cout << vec5 << endl;

    delete [] p;
}