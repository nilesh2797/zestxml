#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <random>
#include <unordered_map>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <unistd.h>


using namespace std;

/* ------------------------ defines, typedefs, inlines, templates --------------------------- */

typedef long long int llint;
typedef pair<int,float> pairIF;
typedef pair<int,double> pairID;
typedef pair<int,int> pairII;
typedef pair<int,bool> pairIB;

#define LINE (cout<<__LINE__<<endl)
#define NAME_LEN 1000
#define SQ(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))
#define INF 1e+36
#define NEG_INF -1e+36
#define EPS 1e-10

enum LOGLVL {QUIET, PROGRESS, DEBUG};

#ifdef _WIN32
#define OS_SEP "\\"
#else
#define OS_SEP "/"
#endif

#define LOG(_mesg) 	cerr << "\r                                                                                                                      \r"; \
					cerr << _mesg << flush
#define LOGN(_mesg) cerr << "\r                                                                                                                      \r"; \
					cout << _mesg << flush << endl; usleep(200)

inline void assertmesg( bool cond, string mesg )
{
	if( !cond )
	{
		cerr << mesg << endl;
		assert( cond );
	}
}

/****************** Binary writing utils ******************/
template <typename T>
void write_bin(T& object, ofstream& fout)
{
    fout.write((char *)(&(object)), sizeof(T));
}

template <typename T>
void read_bin(T& object, ifstream& fin)
{
    fin.read((char *)(&(object)), sizeof(T));
}

template <typename T>
void write_vec_bin(vector<T>& object, ofstream& fout)
{
    size_t siz = object.size();
    fout.write((char *)(&(siz)), sizeof(siz));
    fout.write((char *)(object.data()), sizeof(T)*siz);
}

template <typename T>
void read_vec_bin(vector<T>& object, ifstream& fin)
{
    size_t siz;
    fin.read((char *)(&(siz)), sizeof(siz));
    object.resize(siz);
    fin.read((char *)(object.data()), sizeof(T)*siz);
}

template <typename T>
void write_arr_bin(T* object, ofstream& fout, size_t siz, bool hint_size = false)
{
    if(hint_size)
        fout.write((char *)(&(siz)), sizeof(siz));
    fout.write((char *)(object), sizeof(T)*siz);
}

template <typename T>
size_t read_arr_bin(T*& object, ifstream& fin, size_t siz = 0, bool hint_size = false)
{
    if(hint_size)
        fin.read((char *)(&(siz)), sizeof(siz));
    object = new T[siz];
    fin.read((char *)(object), sizeof(T)*siz);
    return siz;
}
/**********************************************************/

class DenseSVec
{
public:
	vector<float> dvec;
	vector<int> inds;
	vector<bool> touched;
	int siz = 0;
	float default_val = 0;

	void init(int _siz, float _default_val)
	{
		default_val = _default_val;
		siz = _siz;

		dvec.clear();
		dvec.resize(siz, default_val);
		touched.resize(siz, false);
	}

	DenseSVec() { init(1, 0); }

	DenseSVec(int _siz, float _default_val = 0) { init(_siz, _default_val); }

	void reset()
	{
		for(auto id : inds)
		{
			touched[id] = false;
			dvec[id] = default_val;
		}
		inds.clear();
	}

	float get(int id) { return dvec[id]; }

	void put(int id, float val)
	{
		if(not touched[id])
		{
			touched[id] = true;
			inds.push_back(id);
		}
		dvec[id] = val;
	}

	void add(int id, float val) { put(id, dvec[id]+val); }

	void add(pairIF* arr, int siz)
	{
		for(int i = 0; i < siz; ++i)
			this->add(arr[i].first, arr[i].second);
	}

	void max(int id, float val) { put(id, std::max(dvec[id], val)); }

	void max(pairIF* arr, int siz)
	{
		for(int i = 0; i < siz; ++i)
			this->max(arr[i].first, arr[i].second);
	}

	vector<pair<int, float>> vecif()
	{
		sort(inds.begin(), inds.end());
		vector<pair<int, float>> vec;
		for(auto id : inds)
			vec.push_back(pairIF(id, dvec[id]));
		return vec;
	}
};

template <typename T1,typename T2>
bool comp_pair_by_second_desc(pair<T1,T2> a, pair<T1,T2> b)
{
	if(a.second>b.second)
		return true;
	return false;
}

typedef tuple<int,int,int> tupleIII;
typedef vector<tupleIII> VecIII;

struct TupleSorter_first_second
{
    bool operator()(tupleIII const &lhs, tupleIII const &rhs ) const
    {
        return (get<0>(lhs) < get<0>(rhs)) || ( (get<0>(lhs) == get<0>(rhs)) && (get<1>(lhs) < get<1>(rhs)) );
    }
};

struct TupleSorter_second_first
{
    bool operator()(tupleIII const &lhs, tupleIII const &rhs ) const
    {
        return (get<1>(lhs) < get<1>(rhs)) || ( (get<1>(lhs) == get<1>(rhs)) && (get<0>(lhs) < get<0>(rhs)) );
    }
};

struct TupleSorter_third
{
    bool operator()(tupleIII const &lhs, tupleIII const &rhs ) const
    {
        return (get<2>(lhs) < get<2>(rhs));
    }
};

template <typename T1, typename T2, typename T3>
ostream& operator<< ( ostream& os, const vector< tuple<T1,T2,T3> >& vec )
{
	os << vec.size();
	for( int i=0; i<vec.size(); i++ )
		os << " " << get<0>(vec[i]) << ":" << get<1>(vec[i]) << ":" << get<2>(vec[i]);
	return os;
}

template <typename T1, typename T2, typename T3>
istream& operator>> ( istream& is, vector< tuple<T1,T2,T3> >& vec )
{
	int siz;
	is >> siz;
	vec.resize( siz );
	char c;
	for( int i=0; i<siz; i++ )
		is >> get<0>(vec[i]) >> c >> get<1>(vec[i]) >> c >> get<2>(vec[i]);
    return is;
}

template <typename T1,typename T2>
bool comp_pair_by_second(pair<T1,T2> a, pair<T1,T2> b)
{
	if(a.second<b.second)
		return true;
	return false;
}

template <typename T1,typename T2>
bool comp_pair_by_first_desc(pair<T1,T2> a, pair<T1,T2> b)
{
	if(a.first>b.first)
		return true;
	return false;
}

template <typename T1,typename T2>
bool comp_pair_by_first(pair<T1,T2> a, pair<T1,T2> b)
{
	if(a.first<b.first)
		return true;
	return false;
}

template <typename T>
void Realloc(int old_size, int new_size, T*& vec)
{
	T* new_vec = new T[new_size];
	int size = min(old_size,new_size);
	copy_n(vec,size,new_vec);
	delete [] vec;
	vec = new_vec;
}

/************** topk utils ***************/
template <typename T>
void retain_topk(vector<pair<int, T>>& vec, int k)
{
	if(vec.size() > k)
	{
		sort(vec.begin(), vec.end(), comp_pair_by_second_desc<int, T>);
		vec.resize(k);
	}
}

template <typename T>
void retain_topk(vector<T>& vec, int k)
{
	if(vec.size() > k)
	{
		sort(vec.begin(), vec.end(), greater<T>());
		vec.resize(k);
	}
}

template <typename T>
inline int retain_topk(pair<int, T>*& vec, int siz, int k)
{
	if(siz > k)
	{
		sort(vec, vec+siz, comp_pair_by_second_desc<int, T>);
		Realloc(siz, k, vec);
		return k;
	}
	return siz;
}
/****************************************/

template <typename T>
void copy_S_to_D(int size, pair<int,T>* sarr, T* darr)
{
	for(int i=0; i<size; i++)
	{
		darr[sarr[i].first] = sarr[i].second;
	}
}

template <typename T>
void reset_D(int size, pair<int,T>* sarr, T* darr)
{
	for(int i=0; i<size; i++)
	{
		darr[sarr[i].first] = 0;
	}
}

inline void check_valid_filename(string fname, bool read=true)
{
	bool valid;
	ifstream fin;
	ofstream fout;
	if(read)
	{
		fin.open(fname);
		valid = fin.good();
	}
	else
	{
		fout.open(fname);
		valid = fout.good();
	}

	if(!valid)
	{
		cerr<<"error: invalid file name: "<<fname<<endl<<"exiting..."<<endl;
		exit(1);
	}
	if(read)
	{
		fin.close();
	}
	else
	{
		fout.close();
	}
}

inline void check_valid_foldername(string fname)
{
	string tmp_file = fname+"/tmp.txt";
	ofstream fout(tmp_file);

	if(!fout.good())
	{
		cerr<<"error: invalid folder name: "<<fname<<endl<<"exiting..."<<endl;
		exit(1);
	}
	remove(tmp_file.c_str());
}

template <typename T>
ostream& operator<< ( ostream& os, const vector<T>& vec )
{
	os << vec.size();
	for( int i=0; i<vec.size(); i++ )
		os << " " << vec[i];
	return os;
}

template <typename T>
istream& operator>> ( istream& is, vector<T>& vec )
{
	int siz;
	is >> siz;
	vec.resize( siz );
	for( int i=0; i<siz; i++ )
		is >> vec[i];
    return is;
}

template <typename T1, typename T2>
ostream& operator<< ( ostream& os, const vector< pair<T1,T2> >& vec )
{
	os << vec.size();
	for( int i=0; i<vec.size(); i++ )
		os << " " << vec[i].first << ":" << vec[i].second;
	return os;
}

template <typename T1, typename T2>
istream& operator>> ( istream& is, vector< pair<T1,T2> >& vec )
{
	int siz;
	is >> siz;
	vec.resize( siz );
	char c;
	for( int i=0; i<siz; i++ )
		is >> vec[i].first >> c >> vec[i].second;
    return is;
}

inline int get_rand_num( int siz, mt19937& reng )
{
	llint r = reng();
	int ans = r % siz;
	return ans;
}

inline vector<string> split( string text, char sep )
{
	vector<string> chunks;
	while( true )
	{
		size_t pos = text.find_first_of( sep );

		if( pos == string::npos )
		{
			chunks.push_back( text );
			break;
		}
		else
		{
			string chunk = text.substr( 0, pos );
			chunks.push_back( chunk );

			if( pos == text.size()-1 )
			{
				chunks.push_back( "" );
				break;
			}
			else
			{
				text = text.substr( pos+1 );
			}
		}
	}
	return chunks;
}

inline vector<string> read_desc_file( string filename )
{
	ifstream fin;
	fin.open( filename );
	assertmesg( fin.is_open(), "ERROR: File " + filename + " could not be opened for reading" );

	vector<string> lines;
	string s;
	while( !fin.eof() )
	{
		getline( fin, s );
		lines.push_back( s );
	}
	fin.close();
	lines.resize( lines.size()-1 );

	return lines;
}

inline void write_desc_file( vector<string>& lines, string filename )
{
	ofstream fout;
	fout.open( filename );
	assertmesg( fout.is_open(), "ERROR: File " + filename + " could not be opened for writing" );

	for( int i=0; i<lines.size(); i++ )
		fout << lines[i] << "\n";
	
	fout.close();
}

/*
inline VecI sample_without_replacement( int a, int b, int n, bool* mask, mt19937& reng )
{
	uniform_int_distribution<int> udist( a, b );
	VecI samples;

	for( int i=0; i<n; i++ )
	{
		int samp = udist( reng );
		if( !mask[samp] )
		{
			mask[samp] = true;
			samples.push_back( samp );
		}
		else
			i--;
	}

	sort( samples.begin(), samples.end() );
	for( int i=0; i<samples.size(); i++ )
		mask[ samples[i] ] = false;

	return samples;
}
*/

template <typename T>
inline void unsorted_unique( vector<T>& vec )
{
	unordered_map<T,bool> mvec;
	for( int i=0; i<vec.size(); i++ )
		mvec[ vec[i] ] = true;

	vec.clear();	
 	for ( const auto &p : mvec )
        vec.push_back( p.first );
}