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

def threshold_mat( csc_mat, th ):
	c_csc_mat = csc_mat.copy()
	c_csc_mat.data[ c_csc_mat.data>th ] = 0
	c_csc_mat.eliminate_zeros()
	return c_csc_mat

def retain_topk( csc_mat, k ):
	rank_mat = get_rank_mat( csc_mat )
	rank_mat = threshold_mat( rank_mat, k )
	rank_mat = spones( rank_mat )
	csc_mat = csc_mat.multiply( rank_mat )
	return csc_mat

def soft_threshold_mat( csc_mat, th ):
	c_csc_mat = csc_mat.copy()
	c_csc_mat.data[ c_csc_mat.data>th ] = th
	c_csc_mat.eliminate_zeros()
	return c_csc_mat

def split_mat( csc_mat, split ): # 0=train, 1=test, 2=validation
	train_mat = csc_mat[ :, split==0 ]
	test_mat = csc_mat[ :, split==1 ]
	val_mat = csc_mat[ :, split==2 ]
	return (train_mat, test_mat, val_mat)

def sample_columns( inmat, k ):
	nc = inmat.shape[1]
	inds = random.sample( range( nc ), k )
	outmat = inmat[ :, inds ]
	return outmat

def spones( csc_mat ):
	c_csc_mat = csc_mat.copy()
	c_csc_mat.data[ c_csc_mat.data!=0 ] = 1
	return c_csc_mat

# elementwise multiply
def emultiply( csc_mat1, csc_mat2 ):
	prod_mat = csc_mat1.multiply( csc_mat2 )
	return prod_mat

# elementwise divide
def edivide( csc_mat1, csc_mat2 ):
	inv_mat = csc_mat2.copy()
	inv_mat.data = np.reciprocal( inv_mat.data.astype( np.double ) )
	div_mat = emultiply( csc_mat1, inv_mat )
	return div_mat

def diag_multiply_before( vec, mat ):
	if issparse( vec ):
		vec = np.asarray( vec.todense() ).flatten()
	else:
		vec =  np.asarray( vec ).flatten()
	diag_mat = spdiags( [vec], [0], len(vec), len(vec) )
	mat = diag_mat * mat
	mat = mat.tocsc()
	return mat

def diag_multiply_after( mat, vec ):
	if issparse( vec ):
		vec = np.asarray( vec.todense() ).flatten()
	else:
		vec =  np.asarray( vec ).flatten()
	diag_mat = spdiags( [vec], [0], len(vec), len(vec) )
	mat = mat * diag_mat
	mat = mat.tocsc()
	return mat

# elementwise divide. Checks that there are no Inf values during  division
def edivide_safe( csc_mat1, csc_mat2 ):
	assert sum( csc_mat1>0 ) == sum( emultiply( csc_mat1, csc_mat2 )>0 ), "Inf values occur during sparse matrix division"
	return edivide( csc_mat1, csc_mat2 )

def append_bias( csc_mat, bias ):
	nc = csc_mat.shape[1]
	vec = bias * np.ones( (1, nc) )
	csc_mat = vstack( [csc_mat, vec ] ).tocsc()
	return csc_mat

def normalize_matrix( mat ):
	norms = np.sqrt(mat.multiply(mat).sum(0))
	norms[ norms==0 ] = 1
	invnorms = np.reciprocal( norms )
	mat = mat.multiply( invnorms )
	mat = mat.tocsc()
	return mat

def normalize_features( trn_X_Xf, tst_X_Xf ):
	t_trn_X_Xf = trn_X_Xf.transpose()
	t_tst_X_Xf = tst_X_Xf.transpose()
	norms = np.sqrt( t_trn_X_Xf.multiply( t_trn_X_Xf ).sum(0))
	norms[ norms==0 ] = 1
	invnorms = np.reciprocal( norms )
	t_trn_X_Xf = t_trn_X_Xf.multiply( invnorms )
	t_tst_X_Xf = t_tst_X_Xf.multiply( invnorms )
	trn_X_Xf = t_trn_X_Xf.tocsc().transpose()
	tst_X_Xf = t_tst_X_Xf.tocsc().transpose()
	return (trn_X_Xf, tst_X_Xf)

def pdist2_func( mat1, mat2, B=1000, k=10 ):
	assert mat1.shape[0] == mat2.shape[0]
	tmat = mat1.transpose()
	D = mat1.shape[0]
	M = mat1.shape[1]
	N = mat2.shape[1]
	prods = []
	for i in range( 0, N, B ):
		a = i
		b = min( i+B, N )
		submat = mat2[ :, a:b ]
		print( "%d %d" %( a, b ) )
		pmat = tmat * submat
		pmat = pmat.tocsc()
		pmat = retain_topk( pmat, k )
		prods.append( pmat )
	prod_mat = hstack( prods )
	prod_mat = prod_mat.tocsc()
	return prod_mat

def pow_sparse( inmat, alpha ):
	outmat = inmat.copy()
	outmat.data = np.power( outmat.data, alpha )
	return outmat

