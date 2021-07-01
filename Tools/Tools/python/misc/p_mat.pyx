from libcpp.string cimport string
from libcpp.pair cimport pair
from libcpp cimport bool
from libcpp.vector cimport vector
from scipy.sparse import *
import numpy as np
import time
import random
import copy

cdef SMatF* SMatF_csc_to_cpp( csc_mat ):
	pyindptr = csc_mat.indptr
	pyindices = csc_mat.indices
	pydata = csc_mat.data
	cdef int nr = csc_mat.shape[0]
	cdef int nc = csc_mat.shape[1]
	cdef unsigned long long int nnz = csc_mat.getnnz()

	cdef SMatF* smat = new SMatF( nr, nc, nnz, True )
	cdef vector[ int ]* size = &(smat[0].size)
	cdef vector[ pairIF* ]* data = &(smat[0].data)
	cdef pairIF* cdata = smat[0].cdata

	cdef int ctr = 0
	cdef int i
	cdef int j

	for i in range( nc ):
		size[0][i] = pyindptr[i+1]-pyindptr[i]
		data[0][i] = cdata+ctr
		for j in range( size[0][i] ):
			data[0][i][j].first = pyindices[ ctr ]
			data[0][i][j].second = pydata[ ctr ]
			ctr = ctr+1

	return smat

cdef SMatF_cpp_to_csc( SMatF* smat ):
	cdef int nc = smat[0].nc
	cdef int nr = smat[0].nr
	cdef vector[ int ]* size = &(smat[0].size)
	cdef vector[ pairIF* ]* data = &(smat[0].data)
	cdef unsigned long long int nnz = smat[0].get_nnz()

	csc_mat = csc_matrix( (nr, nc) )
	csc_mat.indptr = np.empty( nc+1, dtype=int )
	csc_mat.indices = np.empty( nnz, dtype=int )
	csc_mat.data = np.empty( nnz, dtype=float )

	pyindptr = csc_mat.indptr
	pyindices = csc_mat.indices
	pydata = csc_mat.data

	cdef int ctr = 0
	cdef int i
	cdef int j

	pyindptr[ 0 ] = 0
	for i in range( nc ):
		for j in range( size[0][i] ):
			pyindices[ ctr ] = data[0][i][j].first
			pydata[ ctr ] = data[0][i][j].second
			ctr = ctr+1
		pyindptr[ i+1 ] = ctr

	return csc_mat

def read_text_mat( str fname ):
	fname_bytes = fname.encode()
	cdef SMatF* smat = new SMatF( fname_bytes )
	csc_mat = SMatF_cpp_to_csc( smat )
	return csc_mat

def write_text_mat( csc_mat, str fname, prec=6 ):
	csc_mat.sort_indices()
	cdef SMatF* smat = SMatF_csc_to_cpp( csc_mat )
	fname_bytes = fname.encode()
	smat[0].write( fname_bytes, prec )

def get_rank_mat( csc_mat, order="desc" ):
	cdef SMatF* smat = SMatF_csc_to_cpp( csc_mat )
	order_bytes = order.encode()
	cdef SMatF* rank_smat = p_get_rank_mat( smat, order_bytes )
	rank_csc_mat = SMatF_cpp_to_csc( rank_smat )
	return rank_csc_mat

def sparse_prod( csc_mat1, csc_mat2, csc_pat_mat ):
	cdef SMatF* mat1 = SMatF_csc_to_cpp( csc_mat1 )
	cdef SMatF* mat2 = SMatF_csc_to_cpp( csc_mat2 )
	cdef SMatF* pat_mat = SMatF_csc_to_cpp( csc_pat_mat )
	cdef SMatF* prod_mat = p_sparse_prod( mat1, mat2, pat_mat )
	csc_prod_mat = SMatF_cpp_to_csc( prod_mat )
	return csc_prod_mat

def transpose_mat( csc_mat ):
	cdef SMatF* smat = SMatF_csc_to_cpp( csc_mat )
	cdef SMatF* tmat = p_transpose( smat )
	t_csc_mat = SMatF_cpp_to_csc( tmat )
	return t_csc_mat

def visualize_mat( csc_mat, X, Y, file_name, sort_type="SECONDDESC" ):
	cdef SMatF* smat = SMatF_csc_to_cpp( csc_mat )
	Xenc = copy.deepcopy( X )
	for i in range( len( Xenc ) ):
		Xenc[i] = Xenc[i].encode()
	Yenc = copy.deepcopy( Y )
	for i in range( len( Yenc ) ):
		Yenc[i] = Yenc[i].encode()
	file_name_bytes = file_name.encode()
	sort_type_bytes = sort_type.encode()

	cdef vector[ int ]* size = &(smat[0].size)
	p_visualize( smat, Xenc, Yenc, file_name_bytes, sort_type_bytes )