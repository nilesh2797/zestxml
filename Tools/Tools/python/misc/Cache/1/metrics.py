import sys
import os
import numpy as np
import scipy as sp
import math
import pdb
from scipy.sparse import *
from utils import *

def print_all_metrics( metrics ):
    mets = ["Pk", "WPk", "Nk", "WNk", "tPk", "tNk" ]
    for met in mets:
        if met in metrics:
            metvec = metrics[met]
            print( "%s:\t1: %.2f\t3: %.2f\t5: %.2f" %( met, metvec[0]*100, metvec[2]*100, metvec[4]*100 ) )
            sys.stdout.flush()

def get_all_metrics( score_mat, tst_X_Y, k=5 ):
    metrics = {}
    metrics[ "Pk" ] = precision_k( score_mat, tst_X_Y, k )
    metrics[ "WPk" ] = wt_precision_k( score_mat, tst_X_Y, k )
    metrics[ "Nk" ] = nDCG_k( score_mat, tst_X_Y, k )
    metrics[ "WNk" ] = wt_nDCG_k( score_mat, tst_X_Y, k )
    t_score_mat = transpose_mat( score_mat )
    t_tst_X_Y = transpose_mat( tst_X_Y )
    metrics[ "tPk" ] = precision_k( t_score_mat, t_tst_X_Y, k )
    metrics[ "tNk" ] = nDCG_k( t_score_mat, t_tst_X_Y, k )
    print_all_metrics( metrics )
    return metrics

def precision_k( score_mat, tst_X_Y, k ):
    bin_tst_X_Y = spones( tst_X_Y )
    P = np.empty( k )
    rank_mat = get_rank_mat( score_mat )
    for kitr in range( k, 0, -1 ):
        rank_mat = threshold_mat( rank_mat, kitr )
        one_mat = spones( rank_mat )
        one_mat = one_mat.multiply( bin_tst_X_Y )
        count_tp = one_mat.sum( 0 )
        P[ kitr-1 ] = np.mean( count_tp )/kitr
    return P

def wt_precision_helper( score_mat, tst_X_Y, k ):
    V = np.empty( k )
    rank_mat = get_rank_mat( score_mat )
    for kitr in range( k, 0, -1 ):
        rank_mat = threshold_mat( rank_mat, kitr )
        one_mat = spones( rank_mat )
        one_mat = one_mat.multiply( tst_X_Y )
        count_tp = one_mat.sum( 0 )
        V[ kitr-1 ] = np.mean( count_tp )/kitr
    return V

def wt_precision_k( score_mat, tst_X_Y, k ):
    Ptop = wt_precision_helper( score_mat, tst_X_Y, k )
    Pbottom = wt_precision_helper( tst_X_Y, tst_X_Y, k )
    P = np.divide( Ptop, Pbottom )
    return P

def nDCG_k( score_mat, tst_X_Y, k ):
    bin_tst_X_Y = spones( tst_X_Y )
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
        coeff_mat = coeff_mat.multiply( bin_tst_X_Y )
        num = coeff_mat.sum( 0 )
        counts = bin_tst_X_Y.sum( 0 )
        counts = np.minimum( counts, kitr )
        counts[ counts==0 ] = 1
        counts = counts - 1
        den = cum_wts[ counts.astype( int ) ]
        N[ kitr-1 ] = np.mean( np.divide( num, den ) )
    return N

def wt_nDCG_helper( score_mat, tst_X_Y, k ):
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
        coeff_mat = coeff_mat.multiply( tst_X_Y )
        num = coeff_mat.sum( 0 )
        counts = kitr*np.ones( num_inst )
        counts = counts - 1
        den = cum_wts[ counts.astype( int ) ]
        V[ kitr-1 ] = np.mean( np.divide( num, den ) )
    return V

def wt_nDCG_k( score_mat, tst_X_Y, k ):
    Ptop = wt_nDCG_helper( score_mat, tst_X_Y, k )
    Pbottom = wt_nDCG_helper( tst_X_Y, tst_X_Y, k )
    WN = np.divide( Ptop, Pbottom )
    return WN

def inv_propensity( X_Y, A, B ):
    # Returns inverse propensity weights
    # A,B are parameters of the propensity model. Refer to paper for more details.
    # A,B values used for different datasets in paper:
    #    Wikipedia-LSHTC: A=0.5,  B=0.4
    #    Amazon:          A=0.6,  B=2.6
    #    Other:             A=0.55, B=1.5
    num_inst = X_Y.shape[1]
    freqs = X_Y.sum( 1 );
    C = (math.log(num_inst)-1)* ((B+1)**A)
    wts = 1 + C* ( np.power( freqs+B, -A ) )
    return wts

def inv_propensity_wrap( X_Y, dset ):
    # Returns inverse propensity weights
    # A,B are parameters of the propensity model. Refer to paper for more details.
    # A,B values used for different datasets in paper:
    #    Wikipedia-LSHTC: A=0.5,  B=0.4
    #    Amazon:          A=0.6,  B=2.6
    #    Other:             A=0.55, B=1.5
    if dset=='WikiLSHTC-325K' or dset=='Wikipedia-IRM-1M' or dset=='Wikipedia-1M':
        A = 0.5
        B = 0.4
    elif dset=='Amazon-670K' or dset=='Amazon-IRM-79K' or dset=='Amazon-79K':
        A = 0.6
        B = 2.6
    else:
        A = 0.55
        B = 1.5

    inv_prop = inv_propensity( X_Y, A, B )
    return inv_prop

if __name__=="__main__":
    tst_X_Y_file = sys.argv[1]
    score_mat_file = sys.argv[2]
    k = int( sys.argv[3] )
    
    tst_X_Y = read_text_mat( tst_X_Y_file )
    score_mat = read_text_mat( score_mat_file )
    get_all_metrics( score_mat, tst_X_Y, k=k )