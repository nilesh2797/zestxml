#!/usr/bin/env python3

from utils import *

trn_X_Y = read_text_mat( sys.argv[1] )
Y_T = read_text_mat( sys.argv[2] )
trn_X_per_Y_file = sys.argv[3]
Y_per_T_file = sys.argv[4]
trn_X_per_T_file = sys.argv[5]
T_precision_file = sys.argv[6]

trn_X_Y = spones( trn_X_Y )
Y_T = spones( Y_T )

trn_X_per_Y = myarr( np.sum( trn_X_Y, axis=1 ) )
write_desc_file( trn_X_per_Y, trn_X_per_Y_file )

Y_per_T = myarr( np.sum( Y_T, axis=1 ) )
write_desc_file( Y_per_T, Y_per_T_file )

trn_X_T = Y_T * trn_X_Y
trn_X_per_T = myarr( np.sum( spones( trn_X_T ), axis=1 ) )
write_desc_file( trn_X_per_T, trn_X_per_T_file )

num = myarr( np.sum( trn_X_T, axis=1 ) )
trn_X_per_T = myarr( np.sum( spones(trn_X_T), axis=1 ) )
den = trn_X_per_T*Y_per_T
den[ den==0 ] = 1
T_precision = num/den
write_desc_file( T_precision, T_precision_file )
