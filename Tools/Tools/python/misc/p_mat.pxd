from libcpp.string cimport string
from libcpp.pair cimport pair
from libcpp cimport bool
from libcpp.vector cimport vector
from scipy.sparse import *
import numpy as np
import time
import random

cdef extern from "mat.h":
	ctypedef pair[int,float] pairIF
	cdef cppclass SMat[float]:
		int nc
		int nr
		vector[int] size
		vector[ pairIF* ] data
		pair[int,float]* cdata
		SMat( int nr, int nc, bool contiguous ) except +
		SMat( int nr, int nc, unsigned long long int nnz, bool contiguous ) except +
		SMat( string fname ) except +
		unsigned long long int get_nnz()
		void write( string fname, int precision )
		void threshold( float th )
		void unit_normalize_columns()
		vector[float] column_norms()
		void active_dims( vector[int]& cols, vector[int]& dims, vector[int]& counts, vector[int]& countmap )
	ctypedef SMat[float] SMatF
	SMatF* p_copy( SMatF* inmat )
	void p_add( SMatF* mat1, SMatF* mat2 )
	void p_shrink_mat( SMatF* refmat, vector[int]& cols, SMatF*& s_mat, vector[int]& rows, vector[int]& countmap, bool transpose )
	SMatF* p_get_rank_mat( SMatF* refmat, string order )
	SMatF* p_transpose( SMatF* refmat )
	SMatF* p_prod( SMatF* refmat, SMatF* mat2 )
	SMatF* p_sparse_prod( SMatF* refmat, SMatF* mat2, SMatF* pat_mat )
	void p_visualize( SMatF* mat, vector[string]& X, vector[string] Y, string file_name, string sort_type )

cdef SMatF* SMatF_csc_to_cpp( csc_mat )
cdef SMatF_cpp_to_csc( SMatF* smat )