import sys
import os
import numpy as np
import scipy as sp
import math
import pdb
from scipy.sparse import *
from p_mat import *

def print_all_metrics( metrics ):
	mets = ["Pk", "PSPk", "Nk", "PSNk", "tPk", "tNk" ]
	for met in mets:
		if metrics.has_key( met ):
			metvec = metrics[met]
			print "%s:\t1: %.2f\t3: %.2f\t5: %.2f" %( met, metvec[0]*100, metvec[2]*100, metvec[4]*100 )

def get_all_metrics( score_mat, tst_X_Y, inv_prop, k=5 ):
	metrics = {}
	metrics[ "Pk" ] = precision_k( score_mat, tst_X_Y, k )
	metrics[ "PSPk" ] = ps_precision_k( score_mat, tst_X_Y, inv_prop, k )
	metrics[ "Nk" ] = nDCG_k( score_mat, tst_X_Y, k )
	metrics[ "PSNk" ] = ps_nDCG_k( score_mat, tst_X_Y, inv_prop, k )
	t_score_mat = transpose_mat( score_mat )
	t_tst_X_Y = transpose_mat( tst_X_Y )
	metrics[ "tPk" ] = precision_k( t_score_mat, t_tst_X_Y, k )
	metrics[ "tNk" ] = nDCG_k( t_score_mat, t_tst_X_Y, k )
	print_all_metrics( metrics )
	return metrics

def precision_k( score_mat, tst_X_Y, k ):
	P = np.empty( k )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		one_mat = one_mat.multiply( tst_X_Y )
		count_tp = one_mat.sum( 0 )
		P[ kitr-1 ] = np.mean( count_tp )/kitr
	return P
	
def ps_precision_helper( score_mat, tst_X_Y, inv_prop, k ):
	V = np.empty( k )
	diag_mat = diags( inv_prop )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		one_mat = diag_mat.dot( one_mat )
		one_mat = one_mat.multiply( tst_X_Y )
		count_tp = one_mat.sum( 0 )
		V[ kitr-1 ] = np.mean( count_tp )/kitr
	return V

def ps_precision_k( score_mat, tst_X_Y, inv_prop, k ):
	inv_prop = np.asarray( inv_prop ).flatten()
	Ptop = ps_precision_helper( score_mat, tst_X_Y, inv_prop, k )
	diag_mat = diags( inv_prop )
	wt_tst_X_Y = diag_mat.dot( tst_X_Y )
	wt_tst_X_Y = wt_tst_X_Y.tocsc()
	Pbottom = ps_precision_helper( wt_tst_X_Y, tst_X_Y, inv_prop, k )
	P = np.divide( Ptop, Pbottom )
	return P

def nDCG_k( score_mat, tst_X_Y, k ):
	num_inst = score_mat.shape[1]
	num_lbl = score_mat.shape[0]
	wts = np.log2( np.arange( num_lbl ) + 2 )
	wts = np.reciprocal( wts )
	cum_wts = np.cumsum(wts)

	N = np.empty( k )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		coeff_mat = rank_mat.copy()
		coeff_mat.data = np.reciprocal( np.log2( coeff_mat.data + 1 ) )
		coeff_mat = coeff_mat.multiply( tst_X_Y )
		num = coeff_mat.sum( 0 )
		counts = tst_X_Y.sum( 0 )
		counts = np.minimum( counts, kitr )
		counts[ counts==0 ] = 1
		counts = counts - 1
		den = cum_wts[ counts.astype( int ) ]
		N[ kitr-1 ] = np.mean( np.divide( num, den ) )
	return N

def ps_nDCG_helper( score_mat, tst_X_Y, inv_prop, k ):
	diag_mat = diags( inv_prop )
	num_inst = score_mat.shape[1]
	num_lbl = score_mat.shape[0]
	wts = np.log2( np.arange( num_lbl ) + 2 )
	wts = np.reciprocal( wts )
	cum_wts = np.cumsum(wts)

	V = np.empty( k )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		coeff_mat = rank_mat.copy()
		coeff_mat.data = np.reciprocal( np.log2( coeff_mat.data + 1 ) )
		coeff_mat = diag_mat.dot( coeff_mat )
		coeff_mat = coeff_mat.multiply( tst_X_Y )
		num = coeff_mat.sum( 0 )
		counts = kitr*np.ones( num_inst )
		counts = counts - 1
		den = cum_wts[ counts.astype( int ) ]
		V[ kitr-1 ] = np.mean( np.divide( num, den ) )
	return V

def ps_nDCG_k( score_mat, tst_X_Y, inv_prop, k ):
	inv_prop = np.asarray( inv_prop ).flatten()
	Ptop = ps_nDCG_helper( score_mat, tst_X_Y, inv_prop, k )
	diag_mat = diags( inv_prop )
	wt_tst_X_Y = diag_mat.dot( tst_X_Y )
	wt_tst_X_Y = wt_tst_X_Y.tocsc()
	Pbottom = ps_nDCG_helper( wt_tst_X_Y, tst_X_Y, inv_prop, k )
	PSN = np.divide( Ptop, Pbottom )
	return PSN

def inv_propensity( X_Y, A, B ):
	# Returns inverse propensity weights
	# A,B are parameters of the propensity model. Refer to paper for more details.
	# A,B values used for different datasets in paper:
    #	Wikipedia-LSHTC: A=0.5,  B=0.4
	#	Amazon:          A=0.6,  B=2.6
	#	Other:			 A=0.55, B=1.5
	num_inst = X_Y.shape[1]
	freqs = X_Y.sum( 1 );
	C = (math.log(num_inst)-1)* ((B+1)**A)
	wts = 1 + C* ( np.power( freqs+B, -A ) )
	return wts

def inv_propensity_wrap( X_Y, dset ):
	# Returns inverse propensity weights
	# A,B are parameters of the propensity model. Refer to paper for more details.
	# A,B values used for different datasets in paper:
    #	Wikipedia-LSHTC: A=0.5,  B=0.4
	#	Amazon:          A=0.6,  B=2.6
	#	Other:			 A=0.55, B=1.5
	if dset=='Wikipedia-LSHTC':
		A = 0.5
		B = 0.4
	elif dset=='Amazon':
		A = 0.6
		B = 2.6
	else:
		A = 0.55
		B = 1.5

	inv_prop = inv_propensity( X_Y, A, B )
	return inv_prop