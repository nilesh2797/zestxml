from libcpp.string cimport string
from libcpp.pair cimport pair
from libcpp cimport bool
from libcpp.vector cimport vector
from scipy.sparse import *
import numpy as np
import time
import random
import copy
from p_mat cimport *
import utils

cdef extern from "metrics.h":
    void p_print_all_metrics( SMatF* X_Y, SMatF* score_X_Y, int K )

def print_all_metrics( csc_X_Y, csc_score_X_Y, K ):
    cdef SMatF* X_Y = SMatF_csc_to_cpp( csc_X_Y )
    cdef SMatF* score_X_Y = SMatF_csc_to_cpp( csc_score_X_Y )
    p_print_all_metrics( X_Y, score_X_Y, K )

def print_all_metrics_restricted_labels( tst_X_Y, tst_X_Y_scores, K, restricted_labels ):

    restricted_tst_X_Y = utils.diag_multiply_before( restricted_labels, tst_X_Y )
    restricted_labels_points = utils.get_freq( restricted_tst_X_Y, axis=0 )>0
    restricted_tst_X_Y = restricted_tst_X_Y[ :, restricted_labels_points ]
    restricted_tst_X_Y.eliminate_zeros()

    restricted_tst_X_Y_scores = utils.diag_multiply_before( restricted_labels, tst_X_Y_scores )
    restricted_tst_X_Y_scores = restricted_tst_X_Y_scores[ :, restricted_labels_points ]
    restricted_tst_X_Y_scores.eliminate_zeros()

    cdef SMatF* X_Y = SMatF_csc_to_cpp( restricted_tst_X_Y )
    cdef SMatF* score_X_Y = SMatF_csc_to_cpp( restricted_tst_X_Y_scores )

    p_print_all_metrics( X_Y, score_X_Y, K )

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
