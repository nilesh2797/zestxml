import sys
import os
import numpy as np
import scipy as sp
from scipy.sparse import *
import argparse
import time
from operator import *
from inspect import currentframe, getframeinfo
import csv
import random as rd
import time
import copy
import pdb
import math
from p_mat import *
from p_metrics import *
from inspect import currentframe, getframeinfo


EXP_DIR = os.environ.get( 'EXP_DIR' )

CF = currentframe

def LINE( cf ):
	frameinfo = getframeinfo( cf )
	filename = os.path.basename( frameinfo.filename )
	print( "###", filename, frameinfo.lineno, "###" )

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

def safe_open( filename, mode="r", encoding="utf-8", errors="strict" ):
	try:
		fil = open( filename, mode=mode, encoding=encoding, errors=errors )
	except IOError:
		sys.exit( "ERROR: cannot open file %s in mode %s" %(filename, mode ) )
	return fil

class Timer:
	def __init__( self ):
		1

	def tic( self ):
		self.start_time = time.time()

	def toc( self ):
		self.stop_time = time.time()
		elapsed_time = self.stop_time - self.start_time
		return elapsed_time

def myarr( vec ):
	if issparse( vec ):
		return np.asarray( vec.todense() ).flatten()
	else:
		return np.asarray( vec ).flatten()

def myfind( vec ):
	if issparse( vec ):
		vec = vec.todense()
	vec = myarr( vec )
	fd = np.nonzero( vec )[0]
	return fd

def exist_param( param, field ):
	if field in param and param[ field ] is not None:
		return True
	else:
		return False

def read_desc_file( fname ):
	descs = []
	
	try:
		fin = safe_open( fname, errors="replace" )
		count = 0
		for line in fin:
			count += 1
			descs.append( line )
		fin.close()		
	except:
		sys.exit( "ERROR: %s in function read_desc_file while reading line %d" %(sys.exc_info()[0], count+1) )

	descs = [x.strip() for x in descs]
	return descs

def write_desc_file( X, fname ):
	try:
		fout = safe_open( fname, mode='w' )
		count = 0
		for item in X:
			count += 1
			print(item, file=fout)
		fout.close()
	except:
		sys.exit( "ERROR: %s in function read_desc_file while writing line %d" %(sys.exc_info()[0], count+1) )

def read_csv_file( fname, delim=" " ):
	with safe_open( fname ) as csvfile:
		csvreader = csv.reader( csvfile, delimiter=delim )
		rows = [row for row in csvreader]
	rows = np.array( rows ).astype( float )
	return rows

def read_split_file( fname ):
	split = read_csv_file( fname ) # 0=train, 1=test, 2=validation
	split = myarr( split )
	#split = myarr( split==1 )
	return split

def get_uniform_split( N, fracs, seed=0 ): # can give multiple splits, e.g. 2 splits for train(0)/test(1) or 3 splits for train(0)/test(1)/val(2)
	rd.seed( seed )
	seq = list(range(N))
	rd.shuffle( seq )
	vec = np.zeros( N )
	c = 0
	for i in range(len(fracs)):
		d = min( c + math.ceil(N*fracs[i]), N )
		vec[ seq[c:d] ] = i
		c = d
	vec = myarr( list( map( int, vec) ) )
	return vec

def get_coverage_split( X_Y, frac, seed=0 ):
	N = X_Y.shape[1]
	rd.seed( seed )
	seq = list(range(N))
	test = rd.sample( seq, int(N*frac) )
	vec = np.zeros( N )
	vec[ test ] = 1
	vec = np.array( vec>0 )
	return vec

def split_vec( vec, split ):
	trainvec = [ vec[i] for i in myfind( split==0 ) ]
	testvec = [ vec[i] for i in myfind( split==1 ) ]
	valvec = [ vec[i] for i in myfind( split==2 ) ]
	return (trainvec, testvec, valvec)

def get_freq( mat, axis=0, allow_zeros=True ):
	freq = myarr( np.sum( mat>0, axis=axis ) )
	if not( allow_zeros ):
		freq[ freq==0 ] = 1
	return freq

def get_tfidf_features( trn_X_Xf, tst_X_Xf, p=1 ):
	num_trn = trn_X_Xf.shape[1]
	num_tst = tst_X_Xf.shape[1]
	num_ft = trn_X_Xf.shape[0]
	assert( num_ft==tst_X_Xf.shape[0] )
	assert( num_trn>0 and num_tst>0 and num_ft>0 )
	
	df = get_freq( trn_X_Xf, axis=1, allow_zeros=False )
	idf = np.log( num_trn/df )
	out_tst_X_Xf = diag_multiply_before( idf**p, tst_X_Xf )
	return out_tst_X_Xf	

def inv_propensity( X_Y, A, B ):
	# Returns inverse propensity weights
	# A,B are parameters of the propensity model. Refer to paper for more details.
	# A,B values used for different datasets in paper:
	#	Wikipedia-LSHTC: A=0.5,  B=0.4
	#	Amazon:		  A=0.6,  B=2.6
	#	Other:			 A=0.55, B=1.5
	num_inst = X_Y.shape[1]
	freqs = X_Y.sum( 1 );
	C = (math.log(num_inst)-1)* ((B+1)**A)
	wts = 1 + C* ( np.power( freqs+B, -A ) )
	wts = myarr( wts )
	return wts

def inv_propensity_wrap( X_Y, dataset_type ):
	# Returns inverse propensity weights
	# A,B are parameters of the propensity model. Refer to paper for more details.
	# A,B values used for different datasets in paper:
	#	Wikipedia-LSHTC: A=0.5,  B=0.4
	#	Amazon:		  A=0.6,  B=2.6
	#	Other:			 A=0.55, B=1.5
	if dataset_type=='wikipedia':
		A = 0.5
		B = 0.4
	elif dataset_type=='amazon':
		A = 0.6
		B = 2.6
	else:
		A = 0.55
		B = 1.5

	inv_prop = inv_propensity( X_Y, A, B )
	return inv_prop	

def get_inv_prop_labels( trn_X_Y, tst_X_Y, dataset_type="other" ):
	num_trn = trn_X_Y.shape[1]
	num_tst = tst_X_Y.shape[1]
	num_lbl = trn_X_Y.shape[0]
	assert( num_lbl==tst_X_Y.shape[0] )
	assert( num_trn>0 and num_tst>0 and num_lbl>0 )
	
	ip = inv_propensity_wrap( trn_X_Y, dataset_type )
	out_tst_X_Y = diag_multiply_before( ip, tst_X_Y )
	return out_tst_X_Y

def tautology_remover( X_Y, Xid, Yid ):
	for i in range( X_Y.shape[1] ):
		for j in range( X_Y.indptr[i], X_Y.indptr[i+1] ):
			col = i
			row = X_Y.indices[j]
			if Xid[col]==Yid[row]:
				X_Y[row,col] = 0
	X_Y.eliminate_zeros()

def create_recur_dir( dir_path ):
	os.system( "mkdir -p %s" %(dir_path) )

def visualize_freqs( X_Y, Y, outfile ):
	freq = get_freq( X_Y, axis=1 )
	inds = np.argsort( freq )

	fout = open( outfile, "w" )
	for i in range(len(freq)):
		print( "%d\t%s" %( freq[inds[i]], Y[inds[i]] ), file=fout )
	fout.close()