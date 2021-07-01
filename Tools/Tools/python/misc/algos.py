from libimport import *
from p_mat import *
from utils import *

def balanced_2means( mat, eps, quiet=True ):
	nc = mat.shape[1]
	nr = mat.shape[0]
	c1 = random.randint( 0, nc-1 )
	c2 = c1
	while c2 == c1:
		c2 = random.randint( 0, nc-1 )
	centroids = mat[ :, [c1,c2] ].todense().transpose()

	old_cos = -10000
	new_cos = -1

	while new_cos - old_cos >= eps:
		cosines = centroids * mat
		cosine_diffs = myarr( cosines[0,:] - cosines[1,:] )
		sorted_inds = np.argsort( cosine_diffs )
		neg_inds = sorted_inds[ :int(nc)/2 ]
		pos_inds = sorted_inds[ int(nc)/2: ]

		old_cos = new_cos
		new_cos = ( np.sum( cosines[0, pos_inds] ) + np.sum( cosines[ 1, neg_inds ] ) )/nc

		centroids[0, :] = np.sum( mat[:, pos_inds], axis=1 ).transpose()
		centroids[1, :] = np.sum( mat[:, neg_inds], axis=1 ).transpose()
		norms = np.linalg.norm( centroids, axis=1 )
		norms[ norms==0 ] = 1
		centroids = centroids / norms[:, None ]

		if not( quiet ):
			print("\tnew_cos: %f\t old_cos: %f" %( new_cos, old_cos ))
	
	pos_inds.sort()
	neg_inds.sort()
	return [ pos_inds, neg_inds ]

def cosine_knn( mat1, mat2, k, block_size=1000, dense=False ):
	assert( mat1.shape[0]==mat2.shape[0] )

	mat1 = normalize_matrix( mat1 )
	mat2 = normalize_matrix( mat2 )
	tmat1 = transpose_mat( mat1 )
	nc = mat2.shape[1]

	if dense:
		tmat1 = tmat1.todense()
		mat2 = mat2.todense()

	results = []
	for a in range( 0, nc, block_size ):
		if a+block_size <= nc:
			b = a+block_size
		else:
			b = nc
		print("a:%d\tb:%d\tnc:%d" %(a,b,nc))

		mat = mat2[ :, a:b ]
		prod_mat = tmat1 * mat
		if dense:
			prod_mat = sp.sparse.csc_matrix( prod_mat )
		prod_mat = retain_topk( prod_mat, k )
		results.append( prod_mat )
		
	results = sp.sparse.hstack( results )
	return results

