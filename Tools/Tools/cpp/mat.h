#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <set>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <cassert>

#include "config.h"
#include "utils.h"
#include "timer.h"

using namespace std;

/* ------------------- Sparse and dense matrix and vector resources ---------------------- */

typedef vector<int> VecI;
typedef vector<float> VecF;
typedef vector<_double> VecD;
typedef vector<pairII> VecII;
typedef vector<pairIF> VecIF;
typedef vector<bool> VecB;
typedef vector<string> VecS;

/* ------------------- Helper functions Begin -----------------------*/

template <typename T>
T* getDeepCopy(T* arr, int size)
{
	T* new_arr = new T[size];
	for(int i = 0; i < size; ++i)
		new_arr[i] = arr[i];

	return new_arr;
}

template <typename T>
T* getDeepCopy( vector<T>& arr )
{
	int size = arr.size();
	T* new_arr = new T[size];
	for(int i = 0; i < size; ++i)
		new_arr[i] = arr[i];

	return new_arr;
}

/* ------------------- Helper functions End -----------------------*/

template <typename T>
class SMat // a column-major sparse matrix of type T
{
public:
	bool contiguous = false;
	int nc = 0;
	int nr = 0;
	vector<int> size;
	vector<pair<int,T>*> data;
	pair<int,T>* cdata = NULL;
	vector<int> col_indices;
	bool owns_data = true;

	vector<unordered_map<int, int>> index_hash;
	vector<pairII> index;
	bool indexed = false;

	SMat( bool contiguous = false ) : contiguous(contiguous) { }

	SMat( int nr, int nc, bool contiguous = false ) : contiguous(contiguous), nr(nr), nc(nc)
	{
		size.resize(nc, 0);
		data.resize(nc, NULL);
	}

	SMat( int nr, int nc, _ullint nnz, bool contiguous = false ) : contiguous(contiguous), nr(nr), nc(nc) 
	{
		size.resize(nc, 0);
		data.resize(nc, NULL);

		if( contiguous )
			cdata = new pair<int,T>[ nnz ];
		else
			cdata = NULL;
	}

	SMat(SMat<T>* mat, bool deep_copy = true, bool mask = false, VecI active_cols = VecI(), bool shrink = true)
	{
		// assumption : active_cols is sorted
		// NOTE : if mask is true then only columns present in active_cols will be set in new matrix
		if(not mask)
		{
			active_cols.resize(mat->nc);
			iota(begin(active_cols), end(active_cols), 0);
		}

		nc = (shrink ? active_cols.size() : mat->nc);
		nr = mat->nr;
		size.resize(nc, 0);
		data.resize(nc, NULL);
		owns_data = deep_copy;

		for(int i = 0; i < active_cols.size(); ++i)
		{
			int col 	= active_cols[i];
			int new_col = (shrink ? i : col);
			size[new_col] = mat->size[col];

			if(deep_copy)
				data[new_col] = getDeepCopy(mat->data[col], mat->size[col]);
			else
				data[new_col] = mat->data[col];
		}
	}

	friend istream& operator>>( istream& fin, SMat<T>& mat )
	{
		typedef pair<int,T> pairIT;
		string line;
		getline(fin, line);
		std::istringstream iss(line);

		iss >> mat.nc >> mat.nr;

		mat.size.resize(mat.nc, 0);
		mat.data.resize(mat.nc, NULL);

		int col_no = 0; char colon;

		for( int lin=0; lin<mat.nc; lin++ )
		{
			getline(fin, line);
			std::istringstream iss(line);

			int label_id;
			T label_score;

			vector<pairIT> scores;

			while(iss >> label_id >> colon >> label_score)
				scores.push_back(pairIF(label_id, label_score));

			// sort to allow, mats with unsorted columns
			sort(scores.begin(), scores.end());

			mat.size[col_no] = scores.size();
			mat.data[col_no] = getDeepCopy(scores.data(), scores.size());

			col_no++;
			if(col_no > mat.nc)
				break;
		}

		return fin;
	}	

	SMat(string fname)
	{
		contiguous = false;
		this->load(fname);
	}

	// For reading in Scope/Aether
	SMat( string fname, int num_row )
	{
		contiguous = false;
		check_valid_filename(fname, true);

		ifstream fin;
		fin.open(fname);		

		int col_index;
		vector<int> inds;
		vector<T> vals;
		int max_row_index = -1;

		int capacity = 1;
		string line;
		int i = 0;
		size.resize(capacity);
		data.resize(capacity);
		col_indices.resize(capacity);

		while( getline( fin, line ) )
		{
			line += "\n";
			inds.clear();
			vals.clear();

			int pos = 0;
			int next_pos;
			next_pos = line.find_first_of( "\t", pos );
			string indstr = line.substr( pos, next_pos-pos );
			col_index = stoi( indstr );
			pos = next_pos+1;

			while(next_pos=line.find_first_of(": \n",pos))
			{
				if((size_t)next_pos==string::npos)
					break;

				string indstr = line.substr(pos,next_pos-pos);
				if( indstr=="" )
					break;
				int ind = stoi(indstr);

				pos = next_pos+1;
				next_pos = line.find_first_of(": \n",pos);

				if((size_t)next_pos==string::npos)
					break;
				string valstr = line.substr(pos,next_pos-pos);
				float val = stof( valstr );

				pos = next_pos+1;

				if( num_row != -1 )
				{
					if( ind >= num_row )
						continue;
				}
				else
				{
					max_row_index = ind>max_row_index ?  ind : max_row_index;
				}

				inds.push_back( ind );				
				vals.push_back( val );
			}

			assert(inds.size()==vals.size());
			//assert(inds.size()==0 || inds[inds.size()-1]<nr);

			if( i == capacity-1 )
			{
				int new_capacity = 2*capacity;

				size.resize(new_capacity, 0);
				data.resize(new_capacity, NULL);
				col_indices.resize(new_capacity, 0);

				capacity = new_capacity;
			}

			col_indices[i] = col_index;
			size[i] = inds.size();
			data[i] = new pair<int,T>[inds.size()];

			for(int j=0; j<size[i]; j++)
			{
				data[i][j].first = inds[j];
				data[i][j].second = (T)vals[j];
			}

			i++;
		}

		if( num_row == -1 )
			nr = max_row_index+1;
		else
			nr = num_row;

		nc = i;
		size.resize(nc, 0);
		data.resize(nc, NULL);
		col_indices.resize(nc, 0);

		fin.close();
	}

	/*------------- Indexed implementation ------------*/
	// NOTE : only use for frozen matrix, updating matrix after building index will corrupt index
	
	void build_index()
	{
		if(not indexed)
		{
			reset_index();
			sort_indices();

			int cnt = 0;
			for(int i = 0; i < nc; ++i)
			{
				for(int j = 0; j < size[i]; ++j)
				{
					index[cnt] = pairII(i, j);
					index_hash[i][data[i][j].first] = cnt++;
				}
			}

			indexed = true;
		}
	}

	void clear_index()
	{
		index.clear();
		index_hash.clear(); 
		indexed = false;
	}

	void reset_index()
	{
		clear_index();
		index.resize(get_nnz(), pairII(0, 0));
		index_hash.resize(nc, unordered_map<int, int>());
	}

	bool exists(int col, int row) 
	{ 
		assertmesg(indexed, "exists() : index not built" );
		return (index_hash[col].count(row) > 0); 
	}

	T get_val(int col, int row, T default_val = T()) 
	{
		if(exists(col, row)) 
			return data[col][index[index_hash[col][row]].second].second;
		return default_val;
	}

	T get_index_val(int ind, T default_val = T()) 
	{
		assertmesg((ind <= index.size()), "get_coo() : index doesn't exist" );
		int col = index[ind].first;
		int val = data[col][index[ind].second].second;
		return val;
	}

	int get_index(int col, int row, int default_val = -1) 
	{
		if(exists(col, row)) 
			return index_hash[col][row];
		return default_val;
	}

	pairII get_coo(int ind)
	{
		assertmesg((ind <= index.size()), "get_coo() : index doesn't exist" );
		int col = index[ind].first;
		int row = data[col][index[ind].second].first;
		return pairII(col, row);
	}

	SMat<T>* slice_mat_index(const VecI& active_inds)
	{
		SMat<T>* new_mat = new SMat<T>(nr, nc);
		vector<vector<pair<int, T>>> vec(nc);

		for(auto ind : active_inds)
		{
			pairII coo = get_coo(ind);
			vec[coo.first].push_back(pair<int, T>(coo.second, get_index_val(ind))); 
		}

		for(int i = 0; i < nc; ++i)
		{
			new_mat->size[i] = vec[i].size();
			if(vec[i].size())
				new_mat->data[i] = getDeepCopy(vec[i]);
		}

		new_mat->sort_indices();
		return new_mat;
	}
	/*---------- END Indexed implementation ----------*/

	vector<T> get_val() 
	{
		vector<T> vals;
		for(int i = 0; i < nc; ++i)
			for(int j = 0; j < size[i]; ++j)
				vals.push_back(data[i][j].second);
		return vals;
	}

	void fill_col(int col, VecIF vec)
	{
		if(size[col] > 0) delete[] data[col];
		size[col] = vec.size();
		data[col] = getDeepCopy(vec);
	}

	int retain_topk_helper(pair<int, T>*& vec, int siz, int k)
	{
		if(siz > k)
		{
			sort(vec, vec+siz, comp_pair_by_second_desc<int, T>);
			Realloc(siz, k, vec);
			return k;
		}
		return siz;
	}

	void retain_topk(int k)
	{
		for(int i = 0; i < nc; ++i)
		{
			int siz = retain_topk_helper(data[i], size[i], k);
			size[i] = siz;
		}
	}

	void retain_randomk(int k)
	{
		for(int i = 0; i < nc; ++i)
		{
			random_shuffle(data[i], data[i]+size[i]);
			int siz = min(size[i], k);
			Realloc(size[i], siz, data[i]);
			size[i] = siz;
		}
	}

	void filter(SMat<T>* filter_mat)
	{
	    DenseSVec temp(nr, 0);
	    for(int x = 0; x < nc; ++x)
	    {
	        temp.add(filter_mat->data[x], filter_mat->size[x]);
	        VecIF scores;

	        for(int j = 0; j < size[x]; ++j)
	        {
	            if(not temp.touched[data[x][j].first])
	                scores.push_back(data[x][j]);
	        }

	        sort(scores.begin(), scores.end());
	        delete[] data[x];
	        data[x] = getDeepCopy(scores);
	        size[x] = scores.size();
	        temp.reset();
	    }
	}

	void addCol(pair<int, T>* new_col, int new_col_size, bool deep_copy = true)
	{
		// TODO : write assumption
		size.push_back(new_col_size);
		nc += 1;

		data.push_back(NULL);
		if(deep_copy)
		{
			data[nc - 1] = getDeepCopy(new_col, new_col_size);
		}
		else
		{
			data[nc - 1] = new_col;
			owns_data = false;
		}
	}

	void addCol( vector< pair<int, T> >& new_col )
	{
		int new_col_size = new_col.size();
		// TODO : write assumption
		size.push_back(new_col_size);
		nc += 1;

		data.push_back(NULL);
		// only deep_copy allowed
		data[nc - 1] = getDeepCopy( new_col );
	}

	SMat<T>* hstack(SMat<T>* mat)
	{
		SMat<T>* new_mat = new SMat<T>(this);
		for(int i = 0; i < mat->nc; ++i)
			new_mat->addCol(mat->data[i], mat->size[i]);
		return new_mat;
	}

	void reindex_rows(int _nr, VecI& rows )
	{
		nr = _nr;
		for( int i=0; i < nc; i++ )
		{
			for( int j=0; j < size[i]; j++ )
				data[i][j].first = rows[ data[i][j].first ];
		}
	}

	void reindex_cols(int _nc, VecI& cols )
	{
		VecI new_size(_nc);
		vector<pairIF*> new_data(_nc, NULL);

		for( int i=0; i < nc; i++ )
		{
			new_data[cols[i]] = data[i];
			new_size[cols[i]] = size[i];
		}

		size = new_size;
		data = new_data;
		nc = _nc;
	}

	bool empty()
	{
		for(int i = 0; i < nc; i++)
			if(size[i] > 0) return false;
		return true;
	}

	_ullint get_nnz()
	{
		_ullint nnz = 0;
		for( int i=0; i<nc; i++ )
			nnz += size[i];
		return nnz;
	}

	float get_ram()
	{
		// TODO : verify
		float ram = sizeof( SMat<T> );
		ram += sizeof( int ) * nc;

		for( int i=0; i<nc; i++ )
			ram += sizeof( pair<int,T> ) * size[i];

		return ram;
	}

	SMat<T>* transpose()
	{
		SMat<T>* tmat = new SMat<T>(nc, nr);

		for(int i=0; i<nc; i++)
		{
			for(int j=0; j<size[i]; j++)
			{
				tmat->size[data[i][j].first]++;
			}
		}

		for(int i=0; i<tmat->nc; i++)
		{
			tmat->data[i] = new pair<int,T>[tmat->size[i]];
		}

		vector<int> count(tmat->nc, 0);
		for(int i=0; i<nc; i++)
		{
			for(int j=0; j<size[i]; j++)
			{
				int ind = data[i][j].first;
				T val = data[i][j].second;

				tmat->data[ind][count[ind]].first = i;
				tmat->data[ind][count[ind]].second = val;
				count[ind]++;
			}
		}

		return tmat;
	}

	void in_place_transpose()
	{
		vector<VecIF> tdata(nr);
		for(int i = 0; i < nc; ++i)
		{
			for(int j = 0; j < size[i]; ++j)
				tdata[data[i][j].first].push_back(pairIF(i, data[i][j].second));
			delete[] data[i];
		}

		swap(nr, nc);
		size.clear(); size.resize(nc, 0);
		data.clear(); data.resize(nc, NULL);

		for(int i = 0; i < nc; ++i)
		{
			sort(tdata[i].begin(), tdata[i].end());
			size[i] = tdata[i].size();
			data[i] = getDeepCopy(tdata[i]);
			tdata[i].clear();
		}

		tdata.clear();
	}

	void threshold( float th, bool ignore_sign=true )
	{
		for( int i=0; i<nc; i++ )
		{
			int count = 0;
			for( int j=0; j < size[i]; j++ )
				count += (fabs(data[i][j].second) > th);

			pair<int,T>* newvec = new pair<int,T>[count];
			count = 0;

			for( int j=0; j<size[i]; j++ )
			{
				int id = data[i][j].first;
				T val = data[i][j].second;
				T comp_val = ignore_sign ? fabs(val) : val;
				if( comp_val > th )
					newvec[ count++ ] = make_pair( id, val );
			}

			size[i] = count;
			delete [] data[i];
			data[i] = newvec;
		}
	}

	void unit_normalize_columns( string norm_type="L2" )
	{
		if( norm_type=="L2" )
		{
			for(int i=0; i<nc; i++)
			{
				T normsq = 0;
				for(int j=0; j<size[i]; j++)
					normsq += SQ(data[i][j].second);
				normsq = sqrt(normsq);

				if(normsq==0)
					normsq = 1;

				for(int j=0; j<size[i]; j++)
					data[i][j].second /= normsq;
			}
		}
		else if( norm_type=="L1" )
		{
			for(int i=0; i<nc; i++)
			{
				T norm = 0;
				for(int j=0; j<size[i]; j++)
					norm += fabs(data[i][j].second);

				if(norm==0)
					norm = 1;

				for(int j=0; j<size[i]; j++)
					data[i][j].second /= norm;
			}
		}
		else if( norm_type=="max" )
		{
			for(int i=0; i<nc; i++)
			{
				T norm = -1e8;
				for(int j=0; j<size[i]; j++)
					norm = max(norm, data[i][j].second);

				if(abs(norm) < 1e-8)
					norm = 1;

				for(int j=0; j<size[i]; j++)
					data[i][j].second /= norm;
			}
		}
	}

	vector<T> column_norms()
	{
		vector<T> norms(nc,0);

		for(int i=0; i<nc; i++)
		{
			T normsq = 0;
			for(int j=0; j<size[i]; j++)
				normsq += SQ(data[i][j].second);
			norms[i] = sqrt(normsq);
		}

		return norms;
	}

	~SMat()
	{
		if( contiguous )
		{
			if(owns_data)
				if(cdata)
					delete [] cdata;
		}
		else
		{
			if(owns_data)
				for( int i=0; i<nc; i++ )
					if(data[i])
						delete [] data[i];
		}
	}

	friend ostream& operator<<( ostream& fout, const SMat<T>& mat )
	{
		int nc = mat.nc;
		int nr = mat.nr;

		fout << nc << " " << nr << endl;

		for(int i=0; i<nc; i++)
		{
			for(int j=0; j<mat.size[i]; j++)
			{
				if(j==0)
					fout << mat.data[i][j].first << ":" << mat.data[i][j].second;
				else
					fout << " " << mat.data[i][j].first << ":" << mat.data[i][j].second;
			}
			fout<<endl;
		}

		return fout;
	}

	void write( string fname, int precision=6 )
	{
		check_valid_filename(fname,false);

		ofstream fout;
		fout.open(fname);
		fout << fixed << setprecision( precision );
		fout << (*this);

		fout.close();
	}

	void write_scope( string fname, int precision=3 )
	{
		check_valid_filename(fname,false);

		ofstream fout;
		fout.open(fname);
		fout << fixed << setprecision( precision );

		for( int i=0; i<nc; i++ )
		{
			fout << col_indices[i] << "\t";
			for( int j=0; j<size[i]; j++ )
				if( j==0 )
					fout << data[i][j].first << ":" << data[i][j].second;
				else
					fout << " " << data[i][j].first << ":" << data[i][j].second;
			fout << "\n";
		}

		fout.close();
	}

	void add(SMat<T>* smat, float coeff=1.0 )
	{
		if(nc != smat->nc || nr != smat->nr)
		{
			cerr << "SMat::add : Matrix dimensions do not match" << endl;
			cerr << "Matrix 1: " << nc << " x " << nr <<endl;
			cerr << "Matrix 2: " << smat->nc << " x " << smat->nr << endl;
			exit(1);
		}

		vector<bool> ind_mask(nr, 0);
		vector<T> sum(nr, 0);

		for(int i=0; i < nc; i++)
		{
			vector<int> inds;
			for(int j=0; j < size[i]; j++)
			{
				int ind = data[i][j].first;
				T val = data[i][j].second;

				sum[ind] += val;
				if(!ind_mask[ind])
				{
					ind_mask[ind] = true;
					inds.push_back(ind);
				}
			}

			for(int j=0; j < smat->size[i]; j++)
			{
				int ind = smat->data[i][j].first;
				T val = smat->data[i][j].second;

				sum[ind] += coeff*val;
				if(!ind_mask[ind])
				{
					ind_mask[ind] = true;
					inds.push_back(ind);
				}
			}

			sort(inds.begin(), inds.end());
			Realloc(size[i], inds.size(), data[i]);

			for(int j=0; j<inds.size(); j++)
			{
				int ind = inds[j];
				data[i][j] = make_pair(ind,sum[ind]);
				ind_mask[ind] = false;
				sum[ind] = 0;
			}
			size[i] = inds.size();
		}
	}

	void diff(SMat<T>* smat)
	{
		if(nc != smat->nc || nr != smat->nr)
		{
			cerr << "SMat::add : Matrix dimensions do not match" << endl;
			cerr << "Matrix 1: " << nc << " x " << nr <<endl;
			cerr << "Matrix 2: " << smat->nc << " x " << smat->nr << endl;
			exit(1);
		}

		vector<bool> ind_mask(nr, 0);
		vector<T> sum(nr, 0);

		for(int i=0; i < nc; i++)
		{
			vector<int> inds;
			for(int j=0; j < size[i]; j++)
			{
				int ind = data[i][j].first;
				T val = data[i][j].second;

				sum[ind] += val;
				if(!ind_mask[ind])
				{
					ind_mask[ind] = true;
					inds.push_back(ind);
				}
			}

			for(int j=0; j < smat->size[i]; j++)
			{
				int ind = smat->data[i][j].first;
				T val = smat->data[i][j].second;

				sum[ind] -= val;
				if(!ind_mask[ind])
				{
					ind_mask[ind] = true;
					inds.push_back(ind);
				}
			}

			sort(inds.begin(), inds.end());
			Realloc(size[i], inds.size(), data[i]);

			for(int j=0; j<inds.size(); j++)
			{
				int ind = inds[j];
				data[i][j] = make_pair(ind,sum[ind]);
				ind_mask[ind] = false;
				sum[ind] = 0;
			}
			size[i] = inds.size();
		}
	}

	void prod_helper( int siz, pair<int,T>* dat, vector<int>& indices, vector<T>& sum, string agg_type = "sum" )
	{		
		for( int j=0; j<siz; j++ )
		{
			int ind = dat[j].first;
			T prodval = dat[j].second;

			for(int k=0; k<size[ind]; k++)
			{
				int id = data[ind][k].first;
				T val = data[ind][k].second;

				if(sum[id]==0)
					indices.push_back(id);

				if( agg_type=="sum" )
					sum[id] += val*prodval;
				else if( agg_type=="max" )
					sum[id] = max( sum[id], val*prodval );
				else if (agg_type=="mean")
					sum[id] += ((val*prodval)/siz);
			}
		}

		sort(indices.begin(), indices.end());
	}

	// Returns sparse product matrix by retaining only top k highest scoring rows of (*this) for every column in mat2 if k > -1 else returns just the product
	SMat<T>* prod(SMat<T>* mat2, int k = -1, float th = -0.1, string agg_type = "sum", int num_thread = 1, bool pp = false, string ret_type = "val")
	{
		bool retain = ( k > -1 || th > -1e-5 );
		bool use_k=false, use_th=false;
		if( k>-1 )
			use_k = true;
		else if( th>-1e-5 )
			use_th = true;

		int dim1 = nr;
		int dim2 = mat2->nc;

		string mesg = "ERROR: dimension mismatch in 'prod': " + to_string(nr) + "x" + to_string(nc) + " , " + to_string(mat2->nr) + "x" + to_string(mat2->nc);
		assertmesg(nc==mat2->nr, mesg);

		if(pp) { LOGN("prod called with k : " << k << " th : " << th << " num thread : " << num_thread << " lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr << ", " << mat2->nc << ")" << ", agg type : " << agg_type); }

		SMat<T>* prodmat = new SMat<T>(dim1, dim2);
		TQDM tqdm(dim2, dim2/1000);

		#pragma omp parallel num_threads(num_thread)
		{
			vector<T> sum(dim1,0);

			#pragma omp for
			for(int i=0; i<dim2; i++)
			{
				if(pp) tqdm.step();
				vector<int> indices;
				prod_helper( mat2->size[i], mat2->data[i], indices, sum, agg_type );

				int siz = indices.size();
				prodmat->size[i] = siz;
				prodmat->data[i] = new pair<int,T>[siz];

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					T val = sum[id];
					prodmat->data[i][j] = make_pair(id,val);
					sum[id] = 0;
				}

				if(retain)
				{
					if(ret_type.compare("abs") == 0)
					{
						sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], 
							[](const pair<int, T> & a, const pair<int, T> & b) -> bool
							{ 
							    return abs(a.second) > abs(b.second);
							} );
					}
					else
					{
						sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int, T> );
					}
					int retk=0;

					if( use_k )
						retk = min( k, prodmat->size[i] );
					else if( use_th )
					{
						float norm = 0;
						for( int j=0; j<prodmat->size[i]; j++ )
							norm += SQ( prodmat->data[i][j].second );
						norm = sqrt( norm );

						for( int j=0; j<prodmat->size[i]; j++ )
						{
							if( prodmat->data[i][j].second < th*norm )
								break;
							retk++;
						}

						//cout << "\tretk: " << retk << endl;
					}
					Realloc( prodmat->size[i], retk, prodmat->data[i] );
					sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int, T>);
					prodmat->size[i] = retk;
				}
			}
		}
		return prodmat;
	}

	// Returns sparse product matrix by retaining only top k highest scoring rows of (*this) for every column in mat2 if k > -1 else returns just the product
	SMat<T>* irmprod(SMat<T>* mat2, int k = -1, float th = -0.1)
	{
		bool retain = ( k > -1 || th > -1e-5 );
		bool use_k=false, use_th=false;
		if( k>-1 )
			use_k = true;
		else if( th>-1e-5 )
			use_th = true;

		int dim1 = nr;
		int dim2 = mat2->nc;

		cout << nc << " " << mat2->nr << endl;
		assertmesg(nc==mat2->nr, "ERROR: dimension mismatch in 'prod'");

		mt19937 reng;
    	reng.seed( 0 );
		uniform_real_distribution<float> dist( -1e-5, 1e-5 );

		SMat<T>* prodmat = new SMat<T>(dim1, dim2);
		vector<T> sum(dim1,0);

		Timer timer;
		timer.tic();
		for(int i=0; i<dim2; i++)
		{
			if( i%10000==0 )
			{
				cout << '\r' << i << flush;
			}

			vector<int> indices;
			prod_helper( mat2->size[i], mat2->data[i], indices, sum );

			int siz = indices.size();
			prodmat->size[i] = siz;
			prodmat->data[i] = new pair<int,T>[siz];

			for(int j=0; j<indices.size(); j++)
			{
				int id = indices[j];
				T val = sum[id];
				prodmat->data[i][j] = make_pair(id,val);
				sum[id] = 0;
			}

			if(retain)
			{
				for( int j=0; j<prodmat->size[i]; j++ )
				{
					float r = dist( reng );
					prodmat->data[i][j].second += r;
				}

				sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int, T> );
				int retk=0;

				if( use_k )
					retk = min( k, prodmat->size[i] );
				else if( use_th )
				{
					float norm = 0;
					for( int j=0; j<prodmat->size[i]; j++ )
						norm += SQ( prodmat->data[i][j].second );
					norm = sqrt( norm );

					for( int j=0; j<prodmat->size[i]; j++ )
					{
						if( prodmat->data[i][j].second < th*norm )
							break;
						retk++;
					}

					//cout << "\tretk: " << retk << endl;
				}
				Realloc( prodmat->size[i], retk, prodmat->data[i] );
				sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int, T> );
				prodmat->size[i] = retk;
			}
		}
		cout << endl;
		timer.toc();


		return prodmat;
	}

	// Returns sparse product matrix by retaining only top k highest scoring rows of (*this) for every column in mat2
	SMat<T>* top_prod( SMat<T>* mat2, int k )
	{
		return prod(mat2, k);
	}

	SMat<T>* prod_for_gradient( SMat<T>* mat2, VecS& active_item_per_row, vector<T>& afreq1, vector<T>& afreq2, float alpha, float reg, float threshold, bool pp = false, int num_thread = 1)
	{
		int dim1 = nr;
		int dim2 = mat2->nc;

		string mesg = "ERROR: dimension mismatch in 'prod_for_gradient': " + to_string(nr) + "x" + to_string(nc) + " , " + to_string(mat2->nr) + "x" + to_string(mat2->nc);
		assertmesg(nc==mat2->nr, mesg );

		if(pp) 
		{
			LOGN("prod_for_gradient called with alpha : " << alpha << " reg : " << reg << " threshold: " << threshold << " num thread : "
				<< num_thread << " lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr 
				<< ", " << mat2->nc << ")");
		}

		SMat<T>* prodmat = new SMat<T>(dim1, dim2);
		TQDM tqdm(dim2, 10000); 

		#pragma omp parallel num_threads(num_thread)
		{
			vector<T> sum(dim1,0);

			#pragma omp for
			for(int i=0; i<dim2; i++)
			{
				if(pp) tqdm.step();

				vector<int> indices;
				prod_helper( mat2->size[i], mat2->data[i], indices, sum );

				int siz = indices.size();
				prodmat->size[i] = siz;
				prodmat->data[i] = new pair<int,T>[siz];

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					T val = sum[id];

					float total = afreq1[id]*afreq2[i];
					val = (val - alpha*total)/(total + reg);
					prodmat->data[i][j] = make_pair(id,val);
				}

				sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int,float> );

				int retk = 0;
				unordered_map<string,bool> map_active_item_per_row;
				for( int j=0; j<prodmat->size[i]; j++ )
				{
					int ind = prodmat->data[i][j].first;
					float val = prodmat->data[i][j].second;

					if( val < threshold )
						break;

					if( map_active_item_per_row.find( active_item_per_row[ind] ) == map_active_item_per_row.end() )
					{
						prodmat->data[i][retk++] = prodmat->data[i][j];
						map_active_item_per_row[ active_item_per_row[ind] ] = true;
					}
				}

				Realloc( prodmat->size[i], retk, prodmat->data[i] );
				sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int,float> );
				prodmat->size[i] = retk;

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					sum[id] = 0;
				}
			}
		}

		return prodmat;
	}

	SMat<T>* prod_for_fscore( SMat<T>* mat2, VecS& active_item_per_row, vector<T>& freq1, vector<T>& freq2, int tot_domain, vector<T>& afreq1, vector<T>& afreq2, float alpha = 0.5, int k = -1, float th = -0.1, bool pp = false, int num_thread = 1)
	{
		bool retain = ( k > -1 || th > -1e-5 );
		bool use_k=false, use_th=false;
		if( k>-1 )
			use_k = true;
		else if( th>-1e-5 )
			use_th = true;

		int dim1 = nr;
		int dim2 = mat2->nc;

		string mesg = "ERROR: dimension mismatch in 'prod': " + to_string(nr) + "x" + to_string(nc) + " , " + to_string(mat2->nr) + "x" + to_string(mat2->nc);
		assertmesg(nc==mat2->nr, mesg );

		if(pp) 
		{
			LOGN("fscore prod called with k : " << k << " th : " << th << " num thread : "
				<< num_thread << " lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr 
				<< ", " << mat2->nc << ")");
		}

		SMat<T>* prodmat = new SMat<T>(dim1, dim2);
		TQDM tqdm(dim2, 10000); 

		#pragma omp parallel num_threads(num_thread)
		{
			vector<T> sum(dim1,0);

			#pragma omp for
			for(int i=0; i<dim2; i++)
			{
				if(pp) tqdm.step();

				vector<int> indices;
				prod_helper( mat2->size[i], mat2->data[i], indices, sum );

				int siz = indices.size();
				prodmat->size[i] = siz;
				prodmat->data[i] = new pair<int,T>[siz];

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					T val = sum[id];

					
					// float A = 1.0;
					// val = A*log2( val/( freq2[i] + freq1[id] - val ) ) - 0.5*(1-A)*( log( (float)freq1[id] / (float)tot_domain ) - 0.5*log( (float)freq2[i] / (float)tot_domain ) );
					// val = pow( 2, val );
					

					val = val/( alpha*(freq2[i] + freq1[id] - val) + (1-alpha)*afreq1[id]*afreq2[i] ); // weighted F score combining precision and recall
					prodmat->data[i][j] = make_pair(id,val);
					//sum[id] = 0;
				}

				if( use_k && prodmat->size[i]>0 )
				{
					sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int,float> );

					int retk = 0;
					unordered_map<string,bool> map_active_item_per_row;
					for( int j=0; j<prodmat->size[i]; j++ )
					{
						int ind = prodmat->data[i][j].first;
						float val = prodmat->data[i][j].second;

						if( map_active_item_per_row.find( active_item_per_row[ind] ) == map_active_item_per_row.end() )
						{
							prodmat->data[i][retk++] = prodmat->data[i][j];
							map_active_item_per_row[ active_item_per_row[ind] ] = true;

							if( retk >= k )
								break;
						}
					}

					Realloc( prodmat->size[i], retk, prodmat->data[i] );
					sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int,float> );
					prodmat->size[i] = retk;
				}

				/*
				if(retain && prodmat->size[i]>0 )
				{
					sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int,float> );
					
					int retk=0;

					if( use_k )
					{
						retk = min( k, prodmat->size[i] );


						float good_th = 100.0;
						int c = 0;
						for( int l=0; l<prodmat->size[i]; l++ )
							if( prodmat->data[i][l].second > good_th )
								c++;
						retk = max( retk, c );

					}
					else if( use_th )
					{
						float norm = 0;
						for( int j=0; j<prodmat->size[i]; j++ )
							norm += SQ( prodmat->data[i][j].second );
						norm = sqrt( norm );

						for( int j=0; j<prodmat->size[i]; j++ )
						{
							if( prodmat->data[i][j].second < th*norm )
								break;
							retk++;
						}
					}
					Realloc( prodmat->size[i], retk, prodmat->data[i] );
					sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int,float> );

					for( int j=0; j<retk; j++ )
					{
						int id = prodmat->data[i][j].first;
						float val = sum[id];
						float alpha = 0.0;
						prodmat->data[i][j].second = val/( alpha*(freq2[i] + freq1[id] - val) + (1-alpha)*afreq1[id]*afreq2[i] );
					}
					prodmat->size[i] = retk;
				}
				*/

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					sum[id] = 0;
				}
			}
		}

		return prodmat;
	}

	SMat<T>* sparse_prod( SMat<T>* mat2, SMat<T>* pat_mat, bool pp = false)
	{
		int dim1 = pat_mat->nr;
		int dim2 = pat_mat->nc;
		int dim = nr;

		assert( nr == mat2->nr );
		assert( nc == dim1 );
		assert( mat2->nc == dim2 );

		SMat<T>* prod_mat = new SMat<T>( pat_mat );
		vector<T> mask(nr,0);

		if(pp) { LOGN("sparse prod called with lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr << ", " << mat2->nc << ")"); }

		TQDM tqdm(dim2, 1000);
		for( int i=0; i<dim2; i++ )
		{
			if(pp) tqdm.step();

			for( int j=0; j<mat2->size[i]; j++ )
				mask[ mat2->data[i][j].first ] = mat2->data[i][j].second;

			for( int j=0; j<pat_mat->size[i]; j++ )
			{
				int id = pat_mat->data[i][j].first;
				T prod = 0;
				for( int k=0; k<size[id]; k++ )
					prod += mask[ data[id][k].first ] * data[id][k].second;
				prod_mat->data[i][j].second = prod;
			}

			for( int j=0; j<mat2->size[i]; j++ )
				mask[ mat2->data[i][j].first ] = 0;
		}
		return prod_mat;
	}

	SMat<T>* get_rank_mat( string order )
	{
		// order=="desc" or order=="asc" is the sorting order to use over nonzero elements. Zeros are ignored. Replaces the value of each nonzero element in *this matrix with its rank in its column
		SMat<T>* rmat = new SMat<T>( this );

		if( order == "desc" )
			for( int i=0; i<rmat->nc; i++ )
				stable_sort( rmat->data[i], rmat->data[i]+rmat->size[i], comp_pair_by_second_desc<int,T> );
		else  if( order == "asc" )
			for( int i=0; i<rmat->nc; i++ )
				stable_sort( rmat->data[i], rmat->data[i]+rmat->size[i], comp_pair_by_second<int,T> );
			
		for( int i=0; i<rmat->nc; i++ )
			for( int j=0; j<rmat->size[i]; j++ )
				rmat->data[i][j].second = (j+1);

		for( int i=0; i<rmat->nc; i++ )
				sort( rmat->data[i], rmat->data[i]+rmat->size[i], comp_pair_by_first<int,T> );

		return rmat;
	}

	void eliminate_zeros()
	{
		assert( !contiguous );

		for( int i=0; i<nc; i++ )
		{
			int siz = size[i];
			int newsiz = 0;
			for( int j=0; j<siz; j++ )
			{
				if( abs(data[i][j].second) > 1e-10 )
				{
					data[i][newsiz] = data[i][j];
					newsiz++;
				}
			}
			Realloc(size[i], newsiz, data[i]);
			size[i] = newsiz;
		}
	}

	void append_bias_feat( T bias_feat )
	{
		if( contiguous )
		{
			pair<int,T>* new_cdata = new pair<int,T>[ get_nnz()+nc ];
			int ctr = 0;

			for( int i=0; i<nc; i++ )
			{
				for( int j=0; j<size[i]; j++ )
					new_cdata[ctr++] = data[i][j];

				new_cdata[ctr++] = make_pair( nr, bias_feat );
				size[i]++;
			}

			ctr = 0;
			for( int i=0; i<nc; i++ )
			{
				data[i] = new_cdata+ctr;
				ctr += size[i];
			}
			delete [] cdata;
			cdata = new_cdata;
		}
		else
		{
			for( int i=0; i<nc; i++ )
			{
				int siz = size[i];
				Realloc( siz, siz+1, data[i] );
				data[i][siz] = make_pair( nr, bias_feat );
				size[i]++;	
			}
		}
		nr++;
	}

	void append_bias_feat( vector<T> biases )
	{
		assertmesg(not contiguous, "append bias feat with vector input not implemented for contiguous smat");
		assertmesg(nc == biases.size(), "lenght of biases vector should be same as number of columns of smat");
		for( int i=0; i<nc; i++ )
		{
			int siz = size[i];
			Realloc( siz, siz+1, data[i] );
			data[i][siz] = make_pair( nr, biases[i] );
			size[i]++;	
		}
		nr++;
	}

	void remove_bias_feat()
	{
		assertmesg(not contiguous, "remove bias feat not implemented for contiguous smat");
		for( int i=0; i<nc; i++ )
		{
			int siz = size[i];
			assertmesg(data[i][siz-1].first == (nr-1), "assumption failed : data[i][siz-1].first == nr-1");
			Realloc( siz, siz-1, data[i] );
			size[i]--;
		}
		nr--;
	}

	void active_dims( VecI& cols, VecI& dims, VecI& counts, VecI& countmap )
	{
		dims.clear();
		counts.clear();

		for( int i=0; i<cols.size(); i++ )
		{
			int inst = cols[i];
			for( int j=0; j<size[inst]; j++ )
			{
				int dim = data[inst][j].first;
				if( countmap[ dim ]==0 )
					dims.push_back(dim);
				countmap[ dim ]++;
			}
		}

		sort(dims.begin(),dims.end());

		for( int i=0; i<dims.size(); i++ )
		{
			counts.push_back( countmap[ dims[i] ] );
			countmap[ dims[i] ] = 0;
		}
	}

	void in_place_shrink_mat(VecI& cols, SMat<T>*& s_mat, VecI& rows, VecI& countmap)
	{
		s_mat = new SMat<T>(this, false, true, cols);

		VecI counts;
        active_dims( cols, rows, counts, countmap );
	}
    
    void shrink_mat( VecI& cols, SMat<T>*& s_mat, VecI& rows, VecI& countmap, bool transpose )
    {
        int s_nc = cols.size();
        VecI counts;
        active_dims( cols, rows, counts, countmap );

        _ullint nnz = 0;
        for( int i=0; i<counts.size(); i++ )
            nnz += counts[i];

        int* maps = new int[ nr ];
        for( int i=0; i<rows.size(); i++ )
            maps[ rows[i] ] = i;

        int s_nr = rows.size();
        
        if( transpose )
        {
            s_mat = new SMat<T>( s_nc, s_nr, nnz, true );
        
            int sumsize = 0;
            for( int i=0; i<s_nr; i++ )
            {
                s_mat->size[i] = counts[i];
                s_mat->data[i] = s_mat->cdata + sumsize;
                sumsize += counts[i];
            }
            
            for( int i=0; i<s_nr; i++ )
                counts[i] = 0;
        }
        else
        {
            s_mat = new SMat<T>( s_nr, s_nc, nnz, true );

            int sumsize = 0;
            for( int i=0; i<s_nc; i++)
            {
                int col = cols[i];
                s_mat->size[i] = size[ col ];
                s_mat->data[i] = s_mat->cdata + sumsize;
                sumsize += size[ col ];
            }
        }
            
        for( int i=0; i<s_nc; i++ )
        {	
            int col = cols[ i ];
            for( int j=0; j<size[ col ]; j++ )
            {
                int row = maps[ data[ col ][ j ].first ];
                T val = data[ col ][ j ].second;
                
                if( transpose )
                {
                    s_mat->data[row][counts[row]] = make_pair( i, val );
                    counts[row]++;
                }
                else
                    s_mat->data[i][j] = make_pair( row, val );
            }
        }

        delete [] maps;
    }

	SMat<T>* filter_rows( VecI& retained_rows )
	{
		VecI active_rows( nr, -1 );
		for( int i=0; i<retained_rows.size(); i++ )
			active_rows[ retained_rows[i] ] = i;

		SMat<T>* mat = new SMat<T>( 0, nc );
		for( int i=0; i<nc; i++ )
		{
			VecIF entries;
			for( int j=0; j<size[i]; j++ )
				if( active_rows[ data[i][j].first ]>=0 )
					entries.push_back( make_pair( active_rows[ data[i][j].first ], data[i][j].second ) );
			mat->size[i] = entries.size();
			mat->data[i] = new pairIF[ entries.size() ];
			copy( entries.begin(), entries.end(), mat->data[i] );
		}

		mat->nr = retained_rows.size();
		return mat;
	}

    void split_mat( bool* split, SMat<T>*& mat1, SMat<T>*& mat2 )
    {
    	// split vector determines which columns are distributed to mat1 or mat2. If split[i]==false, ith column is given to mat1, else to mat2
    	int nc1 = 0, nc2 = 0;
    	for( int i=0; i<nc; i++ )
    	{
    		if( !split[i] )
    			nc1++;
    		else
    			nc2++;
    	}

    	mat1 = new SMat<T>( nr, nc1 );
    	mat2 = new SMat<T>( nr, nc2 );

    	int i1=0, i2=0;
    	for( int i=0; i<nc; i++ )
    	{
    		if( !split[i] )
    		{
    			mat1->size[ i1 ] = size[ i ];
    			mat1->data[ i1 ] = new pair<int,T>[ size[ i ] ];
    			copy( data[ i ], data[ i ] + size[ i ], mat1->data[ i1 ] );
    			i1++;
    		}
    		else
    		{
    			mat2->size[ i2 ] = size[ i ];
    			mat2->data[ i2 ] = new pair<int,T>[ size[ i ] ];
    			copy( data[ i ], data[ i ] + size[ i ], mat2->data[ i2 ] );
    			i2++;
    		}
    	}
    }

    vector<T> get_max( VecI& inds, int axis ) // Only inds columns/rows are used for calculating max
    {
    	assert( axis==0 || axis==1 ); // axis==0 => max along each column, axis==1 => max along each row

    	if( axis==0 )
    	{
    		cout << "Not yet implemented" << endl;
    		exit(1);
    	}
    	else if( axis==1 )
    	{
    		vector<T> maxval( nr, NEG_INF );
    		for( int i=0; i<inds.size(); i++ )
    		{
    			int ind = inds[i];
    			for( int j=0; j<size[ind]; j++ )
    			{
    				int colind = data[ind][j].first;
    				T colval = data[ind][j].second;
    				maxval[ colind ] = maxval[colind] > colval ? maxval[colind] : colval;
    			}
    		}
    		for( int i=0; i<nr; i++ )
    		{
    			if( maxval[i]==NEG_INF )
    				maxval[i] = 0;
    		}
    		return maxval;
    	}
    }

	vector<T> get_min( VecI& inds, int axis ) // Only inds columns/rows are used for calculating min
    {
    	assert( axis==0 || axis==1 ); // axis==0 => max along each column, axis==1 => max along each row

    	if( axis==0 )
    	{
    		cout << "Not yet implemented" << endl;
    		exit(1);
    	}
    	else if( axis==1 )
    	{
    		vector<T> minval( nr, INF );
    		for( int i=0; i<inds.size(); i++ )
    		{
    			int ind = inds[i];
    			for( int j=0; j<size[ind]; j++ )
    			{
    				int colind = data[ind][j].first;
    				T colval = data[ind][j].second;
    				minval[ colind ] = minval[colind] < colval ? minval[colind] : colval;
    			}
    		}
    		for( int i=0; i<nr; i++ )
    		{
    			if( minval[i]==INF )
    				minval[i] = 0;
    		}
    		return minval;
    	}
    }

	VecI get_freq( int axis )
    {
    	assert( axis==0 || axis==1 ); // axis==0 => frequency along each column, axis==1 => frequency along each row

    	if( axis==0 )
    	{
			VecI freq( nc, 0 );
			for( int i=0; i<nc; i++ )
				freq[i] = size[i];
			return freq;
    	}
    	else if( axis==1 )
    	{
			VecI freq( nr, 0 );
			for( int i=0; i<nc; i++ )
				for( int j=0; j<size[i]; j++ )
					freq[ data[i][j].first ]++;
    		return freq;
    	}
    }

	VecF get_sum( int axis )
    {
    	assert( axis==0 || axis==1 ); // axis==0 => sum along each column, axis==1 => sum along each row

    	if( axis==0 )
    	{
			VecF sum( nc, 0 );
			for( int i=0; i<nc; i++ )
				for( int j=0; j<size[i]; j++ )
					sum[i] += data[i][j].second;
			return sum;
    	}
    	else if( axis==1 )
    	{
			VecF sum( nr, 0 );
			for( int i=0; i<nc; i++ )
				for( int j=0; j<size[i]; j++ )
					sum[data[i][j].first] += data[i][j].second;
    		return sum;
    	}
    }

    SMat<T>* chunk_mat( int start, int num )
    {
    	int end = start+num-1;
    	assert( start>=0 && start<nc );
    	assert( end>=0 && end<nc );
    	assert( end>=start );

    	int chunk_nc = num;
    	int chunk_nr = nr;
    	_ullint chunk_nnz = 0;

    	for( int i=start; i<=end; i++ )
    		chunk_nnz += size[i];

    	SMat<T>* chunk = new SMat<T>( chunk_nr, chunk_nc, chunk_nnz, true );
    	int ctr = 0;

    	for( int i=0; i<num; i++ )
    	{    		
    		chunk->size[i] = size[i+start];
    		chunk->data[i] = chunk->cdata + ctr;

    		for( int j=0; j<size[i+start]; j++ )
    			chunk->data[i][j] = data[i+start][j];

    		ctr += size[i+start];
    	}
    	return chunk;
    }

    void append_mat( SMat<T>* chunk )
    {
    	assert( nr == chunk->nr );
    	int chunk_nc = chunk->nc;
    	int new_nc = nc + chunk_nc;
    	size.resize(new_nc, 0);
    	data.resize(new_nc, NULL);

    	for( int i=0; i<chunk_nc; i++ )
    	{
    		size[nc+i] = chunk->size[i];
    		data[nc+i] = new pair<int,T>[ chunk->size[i] ];
    		for( int j=0; j<chunk->size[i]; j++ )
    		{
    			data[nc+i][j] = chunk->data[i][j];
    		}
    	}

    	nc = new_nc;
    }

	void read_legacy_mat(ifstream& fin)
	{
		// TODO : remove
		vector<int> inds;
		vector<T> vals;

		string line;
		getline( fin, line );
		line += "\n";
		int pos = 0;
		int next_pos=line.find_first_of(" \n",pos);
		string s = line.substr(pos,next_pos-pos);
		nc = stoi( s );
		pos = next_pos+1;
		next_pos=line.find_first_of(" \n",pos);
		s = line.substr(pos,next_pos-pos);
		nr = stoi( s );

		size.resize(nc, 0);
		data.resize(nc, NULL);

		for(int i=0; i<nc; i++)
		{
			inds.clear();
			vals.clear();
			string line;
			getline(fin,line);
			line += "\n";
			int pos = 0;
			int next_pos;

			while(next_pos=line.find_first_of(": \n",pos))
			{
				if((size_t)next_pos==string::npos)
					break;
				inds.push_back(stoi(line.substr(pos,next_pos-pos)));
				pos = next_pos+1;

				next_pos = line.find_first_of(": \n",pos);
				if((size_t)next_pos==string::npos)
					break;

				vals.push_back(stof(line.substr(pos,next_pos-pos)));
				pos = next_pos+1;

			}

			assert(inds.size()==vals.size());
			assert(inds.size()==0 || inds[inds.size()-1]<nr);

			size[i] = inds.size();
			data[i] = new pair<int,T>[inds.size()];

			for(int j=0; j<size[i]; j++)
			{
				data[i][j].first = inds[j];
				data[i][j].second = (T)vals[j];
			}
		}	
	}

	void readBin(std::ifstream& fin)
	{
		read_bin(nc, fin);
		read_bin(nr, fin);
		read_vec_bin(size, fin);

		data.resize(nc, NULL);
		for(int col = 0; col < nc; ++col) 
			read_arr_bin(data[col], fin, size[col]);
	}

	void readPyBin(std::ifstream& fin)
	{
		read_bin(nc, fin);
		read_bin(nr, fin);
		read_vec_bin(size, fin);

		data.resize(nc, NULL);
		int nnz = accumulate(size.begin(), size.end(), 0);

		int* buf_inds = NULL;
		T* buf_data = NULL;
		read_arr_bin(buf_inds, fin, nnz);
		read_arr_bin(buf_data, fin, nnz);

		int ptr = 0;
		for(int col = 0; col < nc; ++col)
		{
			data[col] = new pairIF[size[col]];
			for(int i = 0; i < size[col]; ++i)
			{
				data[col][i].first = buf_inds[ptr];
				data[col][i].second = buf_data[ptr];
				ptr++;
			}
		}

		delete[] buf_inds;
		delete[] buf_data;
	}

	void writeBin(std::ofstream& fout)
	{
		write_bin(nc, fout);
		write_bin(nr, fout);
		write_vec_bin(size, fout);

		for(int col = 0; col < nc; ++col) 
			write_arr_bin(data[col], fout, size[col]);
	}

	void load(string fname)
	{
		check_valid_filename(fname, true);

		if(fname.substr(fname.size()-4, 4).compare(".bin") == 0)
		{
			LOG("reading binary format sparse matrix : " << fname);
			ifstream fin(fname, ios::in|ios::binary);
			this->readBin(fin);
		}
		else if (fname.substr(fname.size()-6, 6).compare(".pybin") == 0)
		{
			LOG("reading py binary format sparse matrix : " << fname);
			ifstream fin(fname, ios::in|ios::binary);
			this->readPyBin(fin);
		}
		else
		{
			LOG("reading text format sparse matrix : " << fname);
			ifstream fin;
			fin.open(fname);	
			fin >> (*this);
			fin.close();
		}
	}

	void dump(string fname)
	{
		check_valid_filename(fname, false);

		if(fname.substr(fname.size()-4, 4).compare(".bin") == 0)
		{
			LOG("writing binary format sparse matrix : " << fname);
			ofstream fout(fname, ios::binary | ios::out);
			writeBin(fout);
			fout.close();
		}
		else
		{
			LOG("writing text format sparse matrix : " << fname);
			write(fname);
		}		
	}

	void set_values( T value = T() )
	{
		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
				data[i][j].second = value;
	}

	void trunc_max_nnz_per_col( int max_nnz_per_col, string sort_type="desc" )
	{
		for( int i=0; i<nc; i++ )
		{
			if( sort_type=="desc" )
				sort( data[i], data[i] + size[i], comp_pair_by_second_desc<int, T> );
			else if( sort_type=="asc" )
				sort( data[i], data[i] + size[i], comp_pair_by_second<int, T> );
			int new_siz = min( size[i], max_nnz_per_col );
			Realloc( size[i], new_siz, data[i] );
			size[i] = new_siz;
			sort( data[i], data[i] + size[i], comp_pair_by_first<int, T> );
		}
	}

	void visualize( VecS& X, VecS& Y, string file_name, string sort_type="SECONDDESC" )
	{
		ofstream fout;
		fout.open( file_name );

		int num_X = nc;
		int num_Y = nr;

		for( int i=0; i<num_X; i++ )
		{
			fout << i << "\t" << X[i] << "\n";

			VecIF probs;

			for( int j=0; j<size[i]; j++ )
				probs.push_back( data[i][j] );

			if( sort_type=="SECONDDESC" )
				sort( probs.begin(), probs.end(), comp_pair_by_second_desc<int, T> );
			else if( sort_type=="SECOND" )
				sort( probs.begin(), probs.end(), comp_pair_by_second<int, T> );

			for( int j=0; j<probs.size(); j++ )
			{
				fout << "\t" << probs[j].first << "\t" << Y[probs[j].first] << "\t" << probs[j].second << "\n";
			}

			fout << "\n";
		}
		fout.close();
	}

	void apply_log()
	{
		eliminate_zeros();
		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
			{
				T val = data[i][j].second;
				if( val < 0 )
				{
					cerr << "ERROR: value cannot be -ve in apply_log" << endl;
					exit(1);
				}
				data[i][j].second = log( val );
			}
	}

	void apply_exp()
	{
		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
			{
				T val = data[i][j].second;
				data[i][j].second = exp( val );
			}
	}

	vector<T> get_min( int axis ) // Only inds columns/rows are used for calculating min
    {
    	assert( axis==0 || axis==1 ); // axis==0 => max along each column, axis==1 => max along each row

    	if( axis==0 )
    	{
			vector<T> minval( nc, INF );
    		for( int i=0; i<nc; i++ )
    		{
    			for( int j=0; j<size[i]; j++ )
    			{
    				int colind = data[i][j].first;
    				T colval = data[i][j].second;
    				minval[ i ] = minval[i] < colval ? minval[i] : colval;
    			}
    		}
    		for( int i=0; i<nc; i++ )
    		{
    			if( minval[i]==INF )
    				minval[i] = 0;
    		}
    		return minval;
    	}
    	else if( axis==1 )
    	{
    		vector<T> minval( nr, INF );
    		for( int i=0; i<nc; i++ )
    		{
    			for( int j=0; j<size[i]; j++ )
    			{
    				int colind = data[i][j].first;
    				T colval = data[i][j].second;
    				minval[ colind ] = minval[colind] < colval ? minval[colind] : colval;
    			}
    		}
    		for( int i=0; i<nr; i++ )
    		{
    			if( minval[i]==INF )
    				minval[i] = 0;
    		}
    		return minval;
    	}
    }

	// element-wise multiples the "this" matrix and "mat" matrix
	SMat<T>* emultiply( SMat<T>* mat )
	{
		assertmesg( nc == mat->nc && nr == mat->nr, "ERROR: dimensions of 2 matrices much match in emultiply" );

		vector<T> values( nr, 0 );
		SMat<T>* prod_mat = new SMat<T>( mat );

		for( int i=0; i<nc; i++ )
		{
			for( int j=0; j<size[i]; j++ )
				values[ data[i][j].first ] = data[i][j].second;

			for( int j=0; j<prod_mat->size[i]; j++ )
				prod_mat->data[i][j].second *= values[ prod_mat->data[i][j].first ];

			for( int j=0; j<size[i]; j++ )
				values[ data[i][j].first ] = 0;
		}

		prod_mat->eliminate_zeros();
		return prod_mat;
	}

	void scalar_multiply(T val)
	{
		for(int col = 0; col < nc; ++col)
			for(int i = 0; i < size[col]; ++i)
				data[col][i].second *= val;
	}

	SMat<T>* emax( SMat<T>* mat )
	{
		assertmesg( nc == mat->nc && nr == mat->nr, "ERROR: dimensions of 2 matrices much match in emultiply" );

		vector<T> values( nr, 0 );
		SMat<T>* max_mat = new SMat<T>( nr, nc );

		for( int i=0; i<nc; i++ )
		{
			vector<int> indices;

			for( int j=0; j<size[i]; j++ )
			{
				values[ data[i][j].first ] = data[i][j].second;
				indices.push_back( data[i][j].first );
			}

			for( int j=0; j<mat->size[i]; j++ )
			{
				values[ mat->data[i][j].first ] = max( values[ mat->data[i][j].first ], mat->data[i][j].second );
				indices.push_back( mat->data[i][j].first );
			}

			sort( indices.begin(), indices.end() );
			indices.resize( distance( indices.begin(), unique( indices.begin(), indices.end() ) ) );

			max_mat->size[i] = indices.size();
			max_mat->data[i] = new pair<int,T> [ indices.size() ];

			for( int j=0; j<indices.size(); j++ )
			{
				int ind = indices[j];
				max_mat->data[i][j].first = ind;
				max_mat->data[i][j].second = values[ ind ];
				values[ ind ] = 0;
			}
		}

		max_mat->eliminate_zeros();
		return max_mat;
	}

	void diag_multiply_before( vector<T>& w )
	{
		assertmesg( w.size()==nr, "ERROR: dimension mismatch in 'diag_multiply_before'" );

		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
				data[i][j].second *= w[ data[i][j].first ];
	}

	void diag_multiply_after( vector<T>& w )
	{
		assertmesg( w.size()==nc, "ERROR: dimension mismatch in 'diag_multiply_after'" );

		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
				data[i][j].second *= w[ i ];
	}

	void sparse_add_value( T val )
	{
		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
				data[i][j].second += val;
	}

	void print_shape()
	{
		cout << "shape : (" << nr << ", " << nc << ")" << endl;
	}

	void sort_indices()
	{
		for( int i=0; i<nc; i++ )
			sort( data[i], data[i]+size[i], comp_pair_by_first<int,T> );
	}

	void idf_transform()
	{
		VecI freq = get_freq( 1 );
		VecF idf( nr, 0 );

		for( int i=0; i<nr; i++ )
			idf[i] = log( (float)(nc+1) / (float)(freq[i]+1) );

		for( int i=0; i<nc; i++ )
			for( int j=0; j<size[i]; j++ )
				data[i][j].second *= idf[ data[i][j].first ];
	}

	float get_recall(SMat<T>* true_mat)
	{
		SMat<T>* intersection = this->emultiply(true_mat);
		float recall = intersection->get_nnz()*100.0/true_mat->get_nnz();
		delete intersection;
		return recall;
	}

	SMat<T>* prod_for_jaccard( SMat<T>* mat2, vector<T>& freq1, vector<T>& freq2, int tot_domain, vector<T>& afreq1, vector<T>& afreq2, float alpha = 0.5, int k = -1, float th = -0.1, bool pp = false, int num_thread = 1)
	{
		bool retain = ( k > -1 || th > -1e-5 );
		bool use_k=false, use_th=false;
		if( k > 0 )
			use_k = true;
		else if( th>-1e-5 )
			use_th = true;

		int dim1 = nr;
		int dim2 = mat2->nc;

		string mesg = "ERROR: dimension mismatch in 'prod': " + to_string(nr) + "x" + to_string(nc) + " , " + to_string(mat2->nr) + "x" + to_string(mat2->nc);
		assertmesg(nc==mat2->nr, mesg );

		if(pp) 
		{
			LOGN("jaccard prod called with k : " << k << " th : " << th << " num thread : "
				<< num_thread << " lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr 
				<< ", " << mat2->nc << ")");
		}

		SMat<T>* prodmat = new SMat<T>(dim1, dim2);
		TQDM tqdm(dim2, 1000); 

		#pragma omp parallel num_threads(num_thread)
		{
			vector<T> sum(dim1,0);

			#pragma omp for
			for(int i=0; i<dim2; i++)
			{
				if(pp) tqdm.step();

				vector<int> indices;
				prod_helper( mat2->size[i], mat2->data[i], indices, sum );

				int siz = indices.size();
				prodmat->size[i] = siz;
				prodmat->data[i] = new pair<int,T>[siz];

				for(int j=0; j<indices.size(); j++)
				{
					int id = indices[j];
					T val = sum[id];

					
					// float A = 1.0;
					// val = A*log2( val/( freq2[i] + freq1[id] - val ) ) - 0.5*(1-A)*( log( (float)freq1[id] / (float)tot_domain ) - 0.5*log( (float)freq2[i] / (float)tot_domain ) );
					// val = pow( 2, val );
					

					val = val/( alpha*(freq2[i] + freq1[id] - val) + (1-alpha)*afreq1[id]*afreq2[i] ); // weighted F score combining precision and recall
					prodmat->data[i][j] = make_pair(id,val);
					sum[id] = 0;
				}

				if(retain && prodmat->size[i]>0 )
				{
					sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int,float> );
					int retk=0;

					if( use_k )
					{
						retk = min( k, prodmat->size[i] );


						float good_th = 100.0;
						int c = 0;
						for( int l=0; l<prodmat->size[i]; l++ )
							if( prodmat->data[i][l].second > good_th )
								c++;
						retk = max( retk, c );

					}
					else if( use_th )
					{
						float norm = 0;
						for( int j=0; j<prodmat->size[i]; j++ )
							norm += SQ( prodmat->data[i][j].second );
						norm = sqrt( norm );

						for( int j=0; j<prodmat->size[i]; j++ )
						{
							if( prodmat->data[i][j].second < th*norm )
								break;
							retk++;
						}
					}
					Realloc( prodmat->size[i], retk, prodmat->data[i] );
					sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int,float> );
					prodmat->size[i] = retk;
				}
			}
		}

		return prodmat;
	}

	// SMat<T>* prod_for_jaccard( SMat<T>* mat2, vector<T>& freq1, vector<T>& freq2, int tot_domain, vector<T>& afreq1, vector<T>& afreq2, float alpha = 0.5, int k = -1, float th = -0.1, bool pp = false, int num_thread = 1)
	// {
	// 	bool retain = ( k > -1 || th > -1e-5 );
	// 	bool use_k=false, use_th=false;
	// 	if( k>-1 )
	// 		use_k = true;
	// 	else if( th>-1e-5 )
	// 		use_th = true;

	// 	int dim1 = nr;
	// 	int dim2 = mat2->nc;

	// 	string mesg = "ERROR: dimension mismatch in 'prod': " + to_string(nr) + "x" + to_string(nc) + " , " + to_string(mat2->nr) + "x" + to_string(mat2->nc);
	// 	assertmesg(nc==mat2->nr, mesg);

	// 	if(pp) 
	// 	{
	// 		LOGN("jaccard prod called with k : " << k << " th : " << th << " num thread : "
	// 			<< num_thread << " lmat : (" << nr << ", " << nc << ") rmat : (" << mat2->nr 
	// 			<< ", " << mat2->nc << ")");
	// 	}

	// 	SMat<T>* prodmat = new SMat<T>(dim1, dim2);
	// 	TQDM tqdm(dim2, 1000); 

	// 	#pragma omp parallel num_threads(num_thread)
	// 	{
	// 		vector<T> sum(dim1,0);

	// 		#pragma omp for
	// 		for(int i=0; i<dim2; i++)
	// 		{
	// 			if(pp) tqdm.step();			

	// 			vector<int> indices;
	// 			prod_helper( mat2->size[i], mat2->data[i], indices, sum );

	// 			int siz = indices.size();
	// 			prodmat->size[i] = siz;
	// 			prodmat->data[i] = new pair<int,T>[siz];

	// 			for(int j=0; j<indices.size(); j++)
	// 			{
	// 				int id = indices[j];
	// 				T val = sum[id];

	// 				//assert( freq2[i] + freq1[id] - val > 0 );
	// 				val = val/( freq2[i] + freq1[id] - val );
	// 				//val = log( (float)val / (float)tot_domain ) - log( (float)freq1[id] / (float)tot_domain ) - log( (float)freq2[i] / (float)tot_domain );
					
	// 				prodmat->data[i][j] = make_pair(id,val);
	// 				sum[id] = 0;
	// 			}

	// 			if(retain && prodmat->size[i]>0 )
	// 			{
	// 				sort( prodmat->data[i], prodmat->data[i]+prodmat->size[i], comp_pair_by_second_desc<int, T> );
	// 				int retk=0;

	// 				if( use_k )
	// 				{
	// 					retk = min( k, prodmat->size[i] );

	// 					float good_th = 1.0;
	// 					int c = 0;
	// 					for( int l=0; l<prodmat->size[i]; l++ )
	// 						if( prodmat->data[i][l].second > good_th )
	// 							c++;
	// 					retk = max( retk, c );

	// 				}
	// 				else if( use_th )
	// 				{
	// 					float norm = 0;
	// 					for( int j=0; j<prodmat->size[i]; j++ )
	// 						norm += SQ( prodmat->data[i][j].second );
	// 					norm = sqrt( norm );

	// 					for( int j=0; j<prodmat->size[i]; j++ )
	// 					{
	// 						if( prodmat->data[i][j].second < th*norm )
	// 							break;
	// 						retk++;
	// 					}
	// 				}
	// 				Realloc( prodmat->size[i], retk, prodmat->data[i] );
	// 				sort( prodmat->data[i], prodmat->data[i]+retk, comp_pair_by_first<int, T> );
	// 				prodmat->size[i] = retk;
	// 			}
	// 		}
	// 	}
	// 	return prodmat;
	// }

};
                  
template <typename T>
class DMat // a column-major dense matrix of type T
{
public:
	int nc;
	int nr;
	T** data;

	DMat()
	{
		nc = 0;
		nr = 0;
		data = NULL;
	}

	DMat(int nc, int nr)
	{
		this->nc = nc;
		this->nr = nr;
		data = new T*[nc];
		for(int i=0; i<nc; i++)
			data[i] = new T[nr]();
	}

	DMat(int nc, int nr, float val )
	{
		this->nc = nc;
		this->nr = nr;
		data = new T*[nc];
		for(int i=0; i<nc; i++)
		{
			data[i] = new T[nr]();
			fill( data[i], data[i]+nr, val );
		}
	}

	friend istream& operator>>( istream& fin, DMat<T>& mat )
	{
		fin >> mat.nc >> mat.nr;
		mat.data = new T*[mat.nc];
		for(int i=0; i<mat.nc; i++)
			mat.data[i] = new T[mat.nr];

		for( int i=0; i<mat.nc; i++ )
			for( int j=0; j<mat.nr; j++ )
				fin >> mat.data[i][j];

		return fin;
	}	

	DMat(string fname)
	{
		check_valid_filename(fname,true);

		ifstream fin;
		fin.open(fname);	

		fin >> (*this);

		fin.close();
	}


	void reset()
	{
		for( int i=0; i<nc; i++ )
			fill( data[i], data[i]+nr, 0.0 );
	}

	DMat(SMat<T>* mat)
	{
		nc = mat->nc;
		nr = mat->nr;
		data = new T*[nc];
		for(int i=0; i<nc; i++)
			data[i] = new T[nr]();

		for(int i=0; i<mat->nc; i++)
		{
			pair<int,T>* vec = mat->data[i];
			for(int j=0; j<mat->size[i]; j++)
			{
				data[i][vec[j].first] = vec[j].second;
			}
		}
	}

	~DMat()
	{
		for(int i=0; i<nc; i++)
			delete [] data[i];
		delete [] data;
	}

	friend ostream& operator<<( ostream& fout, const DMat<T>& mat )
	{
		int nc = mat.nc;
		int nr = mat.nr;

		fout << nc << " " << nr << "\n";

		for(int i=0; i<nc; i++)
		{
			for(int j=0; j<nr; j++)
			{
				if(j==0)
					fout << mat.data[i][j];
				else
					fout << " " << mat.data[i][j];
			}
			fout<<"\n";
		}

		return fout;
	}

	void write( string fname, int precision=6 )
	{
		check_valid_filename(fname,false);

		ofstream fout;
		fout.open(fname);
		fout << fixed << setprecision( precision );
		fout << (*this);

		fout.close();
	}

	void append_bias_feat( T bias_feat )
	{
		for( int i=0; i<nc; i++ )
		{
			Realloc( nr, nr+1, data[i] );
			data[i][nr] = bias_feat;
		}
		nr++;
	}

};

typedef SMat<float> SMatF;
typedef SMat<int> SMatI;
typedef SMat<int> SMatI;
typedef DMat<float> DMatF;
typedef DMat<int> DMatI;

void reindex_VecIF( VecIF& vec, VecI& index );

template <typename T>
inline T* read_vec( string fname )
{
	check_valid_filename( fname, true );
	ifstream fin;
	fin.open( fname );
	vector< T > vinp;
	T inp;
	while( fin >> inp )
	{
		vinp.push_back( inp );
	}
	fin.close();

	T* vptr = new T[ vinp.size() ];
	for( int i=0; i<vinp.size(); i++ )
		vptr[i] = vinp[i];

	return vptr;
}

inline pairII get_pos_neg_count( VecI& pos_or_neg )
{
	pairII counts = make_pair(0,0);
	for( int i=0; i<pos_or_neg.size(); i++ )
	{
		if(pos_or_neg[i]==+1)
			counts.first++;
		else
			counts.second++;
	}
	return counts;
}

inline void reset_d_with_s( pairIF* svec, int siz, float* dvec )
{
	for( int i=0; i<siz; i++ )
		dvec[ svec[i].first ] = 0;
}

inline void set_d_with_s( pairIF* svec, int siz, float* dvec )
{
	for( int i=0; i<siz; i++ )
		dvec[ svec[i].first ] = svec[i].second;
}

inline void init_2d_float( int dim1, int dim2, float**& mat )
{
	mat = new float*[ dim1 ];
	for( int i=0; i<dim1; i++ )
		mat[i] = new float[ dim2 ]; 
}

inline void delete_2d_float( int dim1, int dim2, float**& mat )
{
	for( int i=0; i<dim1; i++ )
		delete [] mat[i];
	delete [] mat;
	mat = NULL;
}

inline void reset_2d_float( int dim1, int dim2, float**& mat )
{
	for( int i=0; i<dim1; i++ )
		for( int j=0; j<dim2; j++ )
			mat[i][j] = 0;
}

inline float mult_d_s_vec( float* dvec, pairIF* svec, int siz )
{
	float prod = 0;
	for( int i=0; i<siz; i++ )
	{
		int id = svec[i].first;
		float val = svec[i].second;
		prod += dvec[ id ] * val;
	}
	return prod;
}

inline void add_s_to_d_vec( pairIF* svec, int siz, float* dvec )
{
	for( int i=0; i<siz; i++ )
	{
		int id = svec[i].first;
		float val = svec[i].second;
		dvec[ id ] += val;
	}
}

inline void add_sa_to_d_vec( float a, pairIF* svec, int siz, float* dvec )
{
	for( int i=0; i<siz; i++ )
	{
		int id = svec[i].first;
		float val = svec[i].second;
		dvec[ id ] += a*val;
	}
}

inline float get_norm_d_vec( float* dvec, int siz )
{
	float norm = 0;
	for( int i=0; i<siz; i++ )
		norm += SQ( dvec[i] );
	norm = sqrt( norm );
	return norm;
}

inline void div_d_vec_by_scalar( float* dvec, int siz, float s )
{
	for( int i=0; i<siz; i++)
		dvec[i] /= s;
}

inline void normalize_d_vec( float* dvec, int siz )
{
	float norm = get_norm_d_vec( dvec, siz );
	if( norm>0 )
		div_d_vec_by_scalar( dvec, siz, norm );
}


/* Replicating these SMat<T> template functions to enable compatibility with cython code */
SMatF* p_copy( SMatF* inmat );
void p_add( SMatF* mat1, SMatF* mat2 );
void p_shrink_mat( SMatF* refmat, vector<int>& cols, SMatF*& s_mat, vector<int>& rows, vector<int>& countmap, bool transpose );
SMatF* p_get_rank_mat( SMatF* refmat, string order );
SMatF* p_transpose( SMatF* refmat );
SMatF* p_prod( SMatF* refmat, SMatF* mat2 );
SMatF* p_sparse_prod( SMatF* refmat, SMatF* mat2, SMatF* pat_mat );
void p_visualize( SMatF* mat, VecS& X, VecS& Y, string file_name, string sort_type="SECONDDESC" );