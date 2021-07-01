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
from p_mat import *
from metrics import *


EXP_DIR = os.environ.get( 'EXP_DIR' )

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

def get_uniform_split( X_Y, frac, seed=0 ):
	N = X_Y.shape[1]
	rd.seed( seed )
	seq = list(range(N))
	test = rd.sample( seq, int(N*frac) )
	vec = np.zeros( N )
	vec[ test ] = 1
	vec = np.array( vec>0 )
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

def get_tfidf_features( trn_X_Xf, tst_X_Xf ):
	num_trn = trn_X_Xf.shape[1]
	num_tst = tst_X_Xf.shape[1]
	num_ft = trn_X_Xf.shape[0]
	assert( num_ft==tst_X_Xf.shape[0] )
	assert( num_trn>0 and num_tst>0 and num_ft>0 )
	
	df = get_freq( trn_X_Xf, axis=1, allow_zeros=False )
	idf = np.log( num_trn/df )
	out_tst_X_Xf = diag_multiply_before( idf, tst_X_Xf )
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