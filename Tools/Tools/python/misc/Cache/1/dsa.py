from libimport import *
from metrics import *
from utils import *

EXP_DIR = os.environ.get( 'EXP_DIR' )

def load_dsa_dataset( dset ):
	global EXP_DIR
	data_dir = os.path.join( EXP_DIR, 'Datasets', 'DSA', dset )
	
	X_Xf_file = os.path.join( data_dir, 'X_Xf' )
	X_Xf = check_read_text_mat( X_Xf_file )	
	
	X_Y_cl_file = os.path.join( data_dir, 'X_Y_click' )
	X_Y_cl = check_read_text_mat( X_Y_cl_file )

	X_Y_im_file = os.path.join( data_dir, 'X_Y_impression' )
	X_Y_im = check_read_text_mat( X_Y_im_file )
	
	X_file = os.path.join( data_dir, 'X.txt' )
	X = check_read_desc_file( X_file )

	Xf_file = os.path.join( data_dir, 'Xf.txt' )
	Xf = check_read_desc_file( Xf_file )

	Y_file = os.path.join( data_dir, 'Y.txt' )
	Y = check_read_desc_file( Y_file )

	return ( X_Xf, X_Y_cl, X_Y_im, X, Xf, Y )

def create_labels( X_Y_cl, X_Y_im ):
	X_Y = edivide( X_Y_cl, X_Y_im )
	X_Y.data[ X_Y.data>1.0 ] = 0
	X_Y.data[ X_Y.data<0.0 ] = 0
	X_Y.eliminate_zeros()
	return X_Y

def split_dsa_dataset( X_Xf, X_Y, X_Y_cl, X_Y_im, X, frac ):
	num_inst = X_Xf.shape[1]
	split = get_random_split( num_inst, frac )
	( trn_X_Xf, tst_X_Xf ) = split_mat( X_Xf, split )
	( trn_X_Y, tst_X_Y ) = split_mat( X_Y, split )
	( trn_X_Y_cl, tst_X_Y_cl ) = split_mat( X_Y_cl, split )
	( trn_X_Y_im, tst_X_Y_im ) = split_mat( X_Y_im, split )
	( trn_X, tst_X ) = split_vec( X, split )
	return ( trn_X_Xf, tst_X_Xf, trn_X_Y, tst_X_Y, trn_X_Y_cl, tst_X_Y_cl, trn_X_Y_im, tst_X_Y_im, trn_X, tst_X ) 

def subsample_labels( X_Xf, X_Y_cl, X_Y_im, X, Xf, Y, sampleno, samplesize, num_min_ft ):
	np.random.seed( sampleno )
	num_Y = X_Y_cl.shape[0]
	samples = np.random.choice( range(num_Y), size=samplesize, replace=False )
	samples.sort()
	X_Y_cl = X_Y_cl[ samples, : ]
	X_Y_im = X_Y_im[ samples, : ]
	Y = [ Y[i] for i in samples ]
	act_X = myfind( np.sum( X_Y_cl>0, axis=0 )>0 )
	X_Y_cl = X_Y_cl[ :, act_X ]
	X_Y_im = X_Y_im[ :, act_X ]
	X_Xf = X_Xf[ :, act_X ]
	X = [ X[i] for i in act_X ]
	act_Xf = myfind( np.sum( X_Xf>0, axis=1 )>=num_min_ft )
	X_Xf = X_Xf[ act_Xf, : ]
	Xf = [ Xf[i] for i in act_Xf ]
	return ( X_Xf, X_Y_cl, X_Y_im, X, Xf, Y )

def load_std_dsa_dataset( dset ):
	( X_Xf, X_Y_cl, X_Y_im, X, Xf, Y ) = load_dsa_dataset( dset )
	X_Y = create_labels( X_Y_cl, X_Y_im )
	frac = 0.3
	( trn_X_Xf, tst_X_Xf, trn_X_Y, tst_X_Y, trn_X_Y_cl, tst_X_Y_cl, trn_X_Y_im, tst_X_Y_im, trn_X, tst_X ) = split_dsa_dataset( X_Xf, X_Y, X_Y_cl, X_Y_im, X, frac )
	return ( trn_X_Xf, tst_X_Xf, trn_X_Y, tst_X_Y, trn_X_Y_cl, tst_X_Y_cl, trn_X_Y_im, tst_X_Y_im )



############################

""" Metrics code """

def clicks_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	C = np.empty( k )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		num_mat = one_mat.multiply( tst_X_Y_cl )
		num = num_mat.sum()
		C[ kitr-1 ] = num
	return C

def impressions_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	C = np.empty( k )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		den_mat = one_mat.multiply( tst_X_Y_im )
		den = den_mat.sum()
		C[ kitr-1 ] = den
	return C

def micro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	C = np.empty( k )

	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		num_mat = one_mat.multiply( tst_X_Y_cl )
		num = num_mat.sum()
		den_mat = one_mat.multiply( tst_X_Y_im )
		den = den_mat.sum()
		C[ kitr-1 ] = num/den
	return C

def helper_macro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	num_X = score_mat.shape[1]
	C = np.zeros( (k, num_X) )
	rank_mat = get_rank_mat( score_mat )
	for kitr in range( k, 0, -1 ):
		rank_mat = threshold_mat( rank_mat, kitr )
		one_mat = spones( rank_mat )
		num_mat = one_mat.multiply( tst_X_Y_cl )
		den_mat = one_mat.multiply( tst_X_Y_im )
		div_mat = edivide( num_mat, den_mat )
		div_mat = soft_threshold_mat( div_mat, 1.0 )
		C[kitr-1, :] = div_mat.sum( axis=0 )
	return C

def macro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	num_X = score_mat.shape[1]
	tst_X_Y_ctr = edivide( tst_X_Y_cl, tst_X_Y_im )
	N = helper_macro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
	D = helper_macro_ctr_k( tst_X_Y_ctr, tst_X_Y_cl, tst_X_Y_im, k )
	D[ N==0 ] = 1
	C = np.divide( N, D )
	#pdb.set_trace()
	C = C.sum( axis=1 )/float(num_X)
	return C

def print_eval( vec, perc=False ):
	if perc:
		print "1: %.2f %%\t3: %.2f %%\t5: %.2f %%" %( vec[0]*100, vec[2]*100, vec[4]*100 )
	else:
		print "1: %.2f\t3: %.2f\t5: %.2f" %( vec[0], vec[2], vec[4] )

def evaluation( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	clicks = clicks_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
	print "clicks 1--%d" %(k)
	print_eval( clicks, False )
	impressions = impressions_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
	print "impressions 1--%d" %(k)
	print_eval( impressions, False )
	microctrs = micro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
	print "micro ctr 1--%d" %(k)
	print_eval( microctrs, True )
	macroctrs = macro_ctr_k( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
	print "macro ctr 1--%d" %(k)
	print_eval( macroctrs, True )

def per_point_evaluation( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	print "\nper point evaluations:\n"
	evaluation( score_mat, tst_X_Y_cl, tst_X_Y_im, k )

def per_label_evaluation( score_mat, tst_X_Y_cl, tst_X_Y_im, k ):
	print "\nper label evaluations:\n"
	score_mat = score_mat.transpose().tocsc()
	tst_X_Y_cl = tst_X_Y_cl.transpose().tocsc()
	tst_X_Y_im = tst_X_Y_im.transpose().tocsc()
	evaluation( score_mat, tst_X_Y_cl, tst_X_Y_im, k )
