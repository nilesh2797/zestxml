#include "algos.h"
#define LOGBSC 1

using namespace std;

void balanced_kmeans( SMatF* mat, float acc, VecI& partition, mt19937& reng )
{
	int nc = mat->nc;
	int nr = mat->nr;

	int c[2] = {-1,-1};
	c[0] = get_rand_num( nc, reng );
	c[1] = c[0];
	while( c[1] == c[0] )
		c[1] = get_rand_num( nc, reng );

	float** centers;
	init_2d_float( 2, nr, centers );
	reset_2d_float( 2, nr, centers );
	for( int i=0; i<2; i++ )
		set_d_with_s( mat->data[c[i]], mat->size[c[i]], centers[i] );

	float** cosines;
	init_2d_float( 2, nc, cosines );
	
	pairIF* dcosines = new pairIF[ nc ];

	partition.resize( nc );

	float old_cos = -10000;
	float new_cos = -1;

	while( new_cos - old_cos >= acc )
	{

		for( int i=0; i<2; i++ )
		{
			for( int j=0; j<nc; j++ )
				cosines[i][j] = mult_d_s_vec( centers[i], mat->data[j], mat->size[j] );
		}

		for( int i=0; i<nc; i++ )
		{
			dcosines[i].first = i;
			dcosines[i].second = cosines[0][i] - cosines[1][i];
		}
		
		sort( dcosines, dcosines+nc, comp_pair_by_second_desc<int,float> );

		//cout << "\t" << old_cos << "\t" << new_cos << endl;

		old_cos = new_cos;
		new_cos = 0;
		for( int i=0; i<nc; i++ )
		{
			int id = dcosines[i].first;
			int part = (int)(i < nc/2);
			partition[ id ] = 1 - part;
			new_cos += cosines[ partition[id] ][ id ];
		}
		new_cos /= nc;

		reset_2d_float( 2, nr, centers );

		for( int i=0; i<nc; i++ )
		{
			int p = partition[ i ];
			add_s_to_d_vec( mat->data[i], mat->size[i], centers[ p ] );
		}

		for( int i=0; i<2; i++ )
			normalize_d_vec( centers[i], nr );
	}

	delete_2d_float( 2, nr, centers );
	delete_2d_float( 2, nc, cosines );
	delete [] dcosines;
}

void balanced_weighted_kmeans( SMatF* mat, VecF& weights, float acc, float balance, VecI& partition, mt19937& reng )
{
	// ratio of label weights in left and right children is ensured to lie between [0.5-balance,0.5+balance]

	int nc = mat->nc;
	int nr = mat->nr;

	int c[2] = {-1,-1};
	c[0] = get_rand_num( nc, reng );
	c[1] = c[0];
	while( c[1] == c[0] )
		c[1] = get_rand_num( nc, reng );

	float** centers;
	init_2d_float( 2, nr, centers );
	reset_2d_float( 2, nr, centers );
	for( int i=0; i<2; i++ )
		set_d_with_s( mat->data[c[i]], mat->size[c[i]], centers[i] );

	float** cosines;
	init_2d_float( 2, nc, cosines );
	
	pairIF* dcosines = new pairIF[ nc ];

	partition.resize( nc );

	float sum_weights = 0;
	for( int i=0; i<nc; i++ )
		sum_weights += weights[i];
	if( sum_weights==0 )
		sum_weights = 1;
	for( int i=0; i<nc; i++ )
		weights[i] /= sum_weights;

	float old_cos = -10000;
	float new_cos = -1;

	while( new_cos - old_cos >= acc )
	{
		for( int i=0; i<2; i++ )
		{
			for( int j=0; j<nc; j++ )
				cosines[i][j] = mult_d_s_vec( centers[i], mat->data[j], mat->size[j] );
		}

		for( int i=0; i<nc; i++ )
		{
			dcosines[i].first = i;
			dcosines[i].second = cosines[0][i] - cosines[1][i];
		}
		
		sort( dcosines, dcosines+nc, comp_pair_by_second_desc<int,float> );

		old_cos = new_cos;
		new_cos = 0;
		float frac = 0;

		for( int i=0; i<nc; i++ )
		{
			int id = dcosines[i].first;
			float val = dcosines[i].second;
			float wt = weights[id];

			if( i==0 || (i<nc-1 && (frac<0.5-balance || (frac<0.5+balance && val>0))) )
			{
				partition[ id ] = 0;
				frac += wt;
			}
			else
			{
				partition[ id ] = 1;
			}

			new_cos += wt*cosines[ partition[id] ][ id ];
		}

		reset_2d_float( 2, nr, centers );

		for( int i=0; i<nc; i++ )
		{
			int p = partition[i];
			float wt = weights[i];
			add_sa_to_d_vec( wt, mat->data[i], mat->size[i], centers[ p ] );
		}

		for( int i=0; i<2; i++ )
			normalize_d_vec( centers[i], nr );

		//cout << "\t\told_cos: " << old_cos << " new_cos: " << new_cos << " frac: " << frac << endl;

	}

	int num_pos = 0;
	float frac_pos = 0;
	for( int i=0; i<nc; i++ )
		if( partition[i]==0 )
		{
			num_pos++;
			frac_pos += weights[i];
		}
	//cout << "\t\t" << num_pos << ":" << nc-num_pos << "\t" << frac_pos << ":" << 1-frac_pos << endl;

	delete_2d_float( 2, nr, centers );
	delete_2d_float( 2, nc, cosines );
	delete [] dcosines;
}

void get_partition_X_count( SMatF* Y_X, VecI& partition, DMatI* partition_X_count )
{
	int num_X = Y_X->nr;
	int num_Y = Y_X->nc;

	partition_X_count->reset();
	for( int i=0; i<num_Y; i++ )
	{
		int part = partition[i];
		for( int j=0; j<Y_X->size[i]; j++ )
		{
			int id = Y_X->data[i][j].first;
			float val = Y_X->data[i][j].second;
			partition_X_count->data[part][id]++;
		}
	}
}

void get_bsc_sq_distances( SMatF* X_Xf, float** centers, float** sq_distances )
{
	int num_X = X_Xf->nc;
	int num_Xf = X_Xf->nr-1;

	for( int i=0; i<2; i++ )
	{
		float center_norm_sq = SQ( get_norm_d_vec( centers[i], num_Xf ) );

		for( int j=0; j<num_X; j++ )
		{
			float X_norm_sq = 1.0;
			float dot = mult_d_s_vec( centers[i], X_Xf->data[j], X_Xf->size[j]-1 );
			sq_distances[i][j] = center_norm_sq + X_norm_sq - 2*dot;
			//assertmesg( sq_distances[i][j]>= -1e-6, "sq_distance="+to_string(sq_distances[i][j]) );
		}
	}
}

float get_bsc_objective( float** sq_distances, DMatI* partition_X_count )
{
	int num_X = partition_X_count->nr;
	float obj = 0;

	for( int i=0; i<2; i++ )
		for( int j=0; j<num_X; j++ )
			obj += partition_X_count->data[i][j]>0 ? sq_distances[i][j]/(float)num_X : 0.0;

	return obj;
}

void get_bsc_grads( SMatF* Y_X, VecI& curr_partition, DMatI* curr_partition_X_count, DMatI* zero_partition_X_count, DMatI* one_partition_X_count, float** sq_distances, vector<VecF>& grads )
{
	int num_Y = Y_X->nc;
	int num_X = Y_X->nr;
	assert( grads[0].size() == num_Y );
	assert( grads[1].size() == num_Y );

	for( int i=0; i<num_Y; i++ )
	{
		float diff = 0;
		for( int j=0; j<Y_X->size[i]; j++ )
		{
			int id = Y_X->data[i][j].first;
			float val = Y_X->data[i][j].second;

			if( curr_partition[i]==1 )
			{	
				if( one_partition_X_count->data[1][id]==1 )
					diff += sq_distances[1][id]/(float)num_X;

				diff -= sq_distances[0][id]/(float)num_X;
			}
			else
			{
				if( curr_partition_X_count->data[1][id]==0 )
					diff += sq_distances[1][id]/(float)num_X;

				if( curr_partition_X_count->data[0][id]==1 )
					diff -= sq_distances[0][id]/(float)num_X;
			}
		}
		grads[0][i] = diff;

		//cout << i << ":" << diff << " ";
	}
	//cout << "\n";

	for( int i=0; i<num_Y; i++ )
	{
		float diff = 0;
		for( int j=0; j<Y_X->size[i]; j++ )
		{
			int id = Y_X->data[i][j].first;
			float val = Y_X->data[i][j].second;

			if( curr_partition[i]==1 )
			{
				if( curr_partition_X_count->data[1][id]==1 )
					diff += sq_distances[1][id]/(float)num_X;

				if( curr_partition_X_count->data[0][id]==0 )
					diff -= sq_distances[0][id]/(float)num_X;
			}
			else
			{
				diff += sq_distances[1][id]/(float)num_X;

				if( zero_partition_X_count->data[0][id]==1 )
					diff -= sq_distances[0][id]/(float)num_X;
			}
		}
		grads[1][i] = diff;

		//cout << i << ":" << diff << " ";
	}
	//cout << "\n";

	for( int i=0; i<num_Y; i++ )
	{
		float diff = 0;
		for( int j=0; j<Y_X->size[i]; j++ )
		{
			int id = Y_X->data[i][j].first;
			float val = Y_X->data[i][j].second;

			if( curr_partition[i]==1 )
			{
				if( curr_partition_X_count->data[0][id]==0 )
					diff -= sq_distances[0][id]/(float)num_X;
			}
			else
			{
				if( curr_partition_X_count->data[1][id]==0 )
					diff += sq_distances[1][id]/(float)num_X;
			}
		}
		grads[2][i] = diff;
		//cout << i << ":" << diff << " ";
	}
}

void get_bsc_centers( SMatF* X_Xf, DMatI* partition_X_count, float** centers )
{
	int num_X = X_Xf->nc;
	int num_Xf = X_Xf->nr-1;
	reset_2d_float( 2, num_Xf, centers );

	for( int i=0; i<2; i++ )
	{
		int count_X = 0;
		for( int j=0; j<num_X; j++ )
		{
			int count = partition_X_count->data[i][j];
			if( count>0 )
			{
				count_X++;
				for( int k=0; k<X_Xf->size[j]-1; k++ )
				{
					int id = X_Xf->data[j][k].first;
					float val = X_Xf->data[j][k].second;
					centers[i][id] += val;
				}
			}
		}

		if( count_X>0 )
			for( int j=0; j<num_Xf; j++ )
				centers[i][j] /= (float)count_X;
	}
}

void balanced_submodular_clustering( SMatF* X_Xf, SMatF* Y_X, float acc, VecI& partition, mt19937& reng )
{
	int max_inner_itrs = 3;
	int num_X = X_Xf->nc;
	int num_Xf = X_Xf->nr-1;
	int num_Y = Y_X->nc;

	if( num_Xf < 1 )
		return;

	float** centers;
	init_2d_float( 2, num_Xf, centers );
	reset_2d_float( 2, num_Xf, centers );

	float** sq_distances;
	init_2d_float( 2, num_X, sq_distances );

	VecI curr_partition( num_Y );
	for( int i=0; i<num_Y; i++ )
		curr_partition[i] = partition[i];
	DMatI* curr_partition_X_count = new DMatI( 2, num_X );
	get_partition_X_count( Y_X, curr_partition, curr_partition_X_count );
	get_bsc_centers( X_Xf, curr_partition_X_count, centers );
	
	VecI zero_partition( num_Y );
	fill( zero_partition.begin(), zero_partition.end(), 0 );
	DMatI* zero_partition_X_count = new DMatI( 2, num_X );
	get_partition_X_count( Y_X, zero_partition, zero_partition_X_count );

	VecI one_partition( num_Y );
	fill( one_partition.begin(), one_partition.end(), 1 );
	DMatI* one_partition_X_count = new DMatI( 2, num_X );
	get_partition_X_count( Y_X, one_partition, one_partition_X_count );

	vector<VecF> grads( 3 );
	grads[0].resize( num_Y );
	grads[1].resize( num_Y );
	grads[2].resize( num_Y );

	get_bsc_sq_distances( X_Xf, centers, sq_distances );	
	float old_obj = 10000;
	float curr_obj = get_bsc_objective( sq_distances, curr_partition_X_count );

	while( old_obj - curr_obj >= acc )
	{
		if( LOGBSC )
		cout << "curr obj: " << curr_obj << endl;
		old_obj = curr_obj;

		for( int inner_itr=0; inner_itr<max_inner_itrs; inner_itr++ )
		{
			get_bsc_grads( Y_X, curr_partition, curr_partition_X_count, zero_partition_X_count, one_partition_X_count, sq_distances, grads );

			for( int i=0; i<3; i++ )
			{
				VecF& grad = grads[i];
				pairIF* sort_grad = new pairIF[ num_Y ];

				for( int j=0; j<num_Y; j++ )
					sort_grad[j] = make_pair( j, grad[j] );

				sort( sort_grad, sort_grad+num_Y, comp_pair_by_second<int,float> );

				VecI local_partition( num_Y, 0 );
				for( int j=0; j<num_Y/2; j++ )
					local_partition[ sort_grad[j].first ] = 1;

				DMatI* partition_X_count = new DMatI( 2, num_X );
				get_partition_X_count( Y_X, local_partition, partition_X_count );

				float new_obj = get_bsc_objective( sq_distances, partition_X_count );
				
				if( new_obj < curr_obj )
				{
					if( LOGBSC )
					cout << "\tnew obj (label partition " << i << "): " << new_obj << endl;
					curr_partition = local_partition;
					delete curr_partition_X_count;
					curr_partition_X_count = partition_X_count;
					partition_X_count = NULL;
					curr_obj = new_obj;
				}
				delete [] sort_grad;
				delete partition_X_count;
			}
		}

		get_bsc_centers( X_Xf, curr_partition_X_count, centers );

		get_bsc_sq_distances( X_Xf, centers, sq_distances );

		float new_obj = get_bsc_objective( sq_distances, curr_partition_X_count );

		if( LOGBSC )
			cout << "\tnew obj (point assignment): " << new_obj << endl;
		curr_obj = new_obj;
	}
	
	delete_2d_float( 2, 100, centers );
	delete_2d_float( 2, num_X, sq_distances );
	delete curr_partition_X_count;
	delete zero_partition_X_count;
	delete one_partition_X_count;

	partition.clear();
	for( int i=0; i<num_Y; i++ )
		partition.push_back( curr_partition[i] );
}
