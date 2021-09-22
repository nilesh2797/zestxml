/************ util functions ************/
#include "helper.h"

using namespace std;

/************ helper functions ************/

float get_classifier_cost( _Classifier_Kind classifier_kind, int num_all_instance, int num_node_instance, float raw_classifier_cost )
{
	float classifier_cost;
	if( classifier_kind == L2R_L2LOSS_SVC_DUAL )
		classifier_cost = raw_classifier_cost;
	else if( classifier_kind == L2R_LR_DUAL )
		classifier_cost = raw_classifier_cost * (float)num_node_instance / (float)num_all_instance;
	return classifier_cost;
}

float get_new_score(float prod, float prev_score, _Classifier_Kind classifier_kind, bool logp)
{
	_float newvalue = 0.0;

	if(classifier_kind == L2R_L2LOSS_SVC_DUAL)
		newvalue = - SQ(max((_float)0.0, 1-prod ));
	else if(classifier_kind == L2R_LR_DUAL)
		newvalue = - log(1 + exp( -prod ));
	else
		newvalue = log(abs(prod));

	float discount = 1.0;
	newvalue += discount * prev_score;

	if(logp) return newvalue;
	else return exp(newvalue);
}

VecIF apply_classifier(VecF& dense_clf, pairIF* shortlist, int shortlist_size, SMatF* X_Xf, _Classifier_Kind classifier_kind, bool logp, bool pp, bool raw)
{
	// WARNING : Assumption : temp vector is all zero and of size >= numxf
	assertmesg(dense_clf.size() >= X_Xf->nr, "Assumption Failed : dense clf vector is of size >= numx");
	if(pp) { LOGN("applying classifier..."); }
	
	TQDM tqdm(shortlist_size, 10000);
	VecIF result(shortlist_size);
	for(int i = 0; i < shortlist_size; ++i)
	{
		int x = i; float prev_val = 0;
		if(shortlist != NULL)
		{
			x = shortlist[i].first; 
			prev_val = shortlist[i].second;
		}

		float prod = mult_d_s_vec(dense_clf.data(), X_Xf->data[x], X_Xf->size[x]);
		if(raw) result[i] = pairIF(x, prod);
		else result[i] = pairIF(x, get_new_score(prod, prev_val, classifier_kind, logp));

		if(pp) tqdm.step();
	}

	return result;
}

VecIF apply_classifier(pairIF* clf, int clf_size, pairIF* shortlist, int shortlist_size, SMatF* X_Xf, _Classifier_Kind classifier_kind, VecF& temp, bool logp, bool raw)
{
	// WARNING : Assumption : temp vector is all zero and of size >= numxf
	assertmesg(temp.size() >= X_Xf->nr, "Assumption Failed : temp vector is of size >= numx");

	set_d_with_s(clf, clf_size, temp.data());
	VecIF result = apply_classifier(temp, shortlist, shortlist_size, X_Xf, classifier_kind, logp, false, raw);
	reset_d_with_s(clf, clf_size, temp.data());

	return result;
}
// -

SMatF* expand_linear_form_mat(const VecF& vec, SMatF* sparsity_pattern, SMatF* Xf_Yf, SMatF* Yf_Xf)
{
    SMatF* new_sparsity_pattern = new SMatF(sparsity_pattern->nr, sparsity_pattern->nc);
    vector<VecIF> temp(sparsity_pattern->nc);
    size_t c = 0;

    for(int i = 0; i < Xf_Yf->nc; ++i)
    	for(int j = 0; j < Xf_Yf->size[i]; ++j)
    		temp[i].push_back(pairIF(Xf_Yf->data[i][j].first, vec[c++]));

    for(int i = 0; i < Yf_Xf->nc; ++i)
    	for(int j = 0; j < Yf_Xf->size[i]; ++j)
    		temp[Yf_Xf->data[i][j].first].push_back(pairIF(i, vec[c++]));

    for(int i = 0; i < new_sparsity_pattern->nc; ++i)
    {
    	new_sparsity_pattern->size[i] = temp[i].size();
    	if(temp[i].size() > 0)
    		new_sparsity_pattern->data[i] = getDeepCopy(temp[i]);
    	temp[i].clear();
    }

    new_sparsity_pattern->sort_indices();
    return new_sparsity_pattern;
}

void ips_weight( SMatF* trn_X_Y, Parameters& params )
{
    float A = params.get<float>("propensity_A");
    float B = params.get<float>("propensity_B");
    int N = trn_X_Y->nc;
    float C = (log((float)N)-1)*pow(B+1,A);
    
    VecI freq = trn_X_Y->get_freq( 1 );
    VecF ips;
    for( int i=0; i<freq.size(); i++ )
        ips.push_back( 1 + C * exp( -A*log( freq[i]+B ) ) );

    
    float ips_max = 0;
    for( int i=0; i<ips.size(); i++ )
        ips_max = max( ips_max, ips[i] );

    for( int i=0; i<ips.size(); i++ )
        ips[i] /= ips_max;
    

    for( int i=0; i<trn_X_Y->nc; i++ )
        for( int j=0; j<trn_X_Y->size[i]; j++ )
            trn_X_Y->data[i][j].second = ips[ trn_X_Y->data[i][j].first ];
}

void remove_duplicates(SMatF*& Yf_Xf, SMatF* Xf_Yf)
{
    SMatF* Xf_Yf1 = Yf_Xf->transpose();
    delete Yf_Xf;

    VecB feats( Xf_Yf->nr, false );

    for( int i=0; i<Xf_Yf->nc; i++ )
    {
        for( int j=0; j<Xf_Yf->size[i]; j++ )
            feats[ Xf_Yf->data[i][j].first ] = true;

        int c = 0;
        for( int j=0; j<Xf_Yf1->size[i]; j++ )
        {
            if( ! feats[ Xf_Yf1->data[i][j].first ] )
            {
                Xf_Yf1->data[i][c] = Xf_Yf1->data[i][j];
                c++;
            }
        }
        Realloc( Xf_Yf1->size[i], c, Xf_Yf1->data[i] );
        Xf_Yf1->size[i] = c;

        for( int j=0; j<Xf_Yf->size[i]; j++ )
            feats[ Xf_Yf->data[i][j].first ] = false;
    }

    Yf_Xf = Xf_Yf1->transpose();
    delete Xf_Yf1;
}

/************ xhtp approx functions ************/

void create_Xf_Yf_map( SMatF* _trn_X_Xf, SMatF* _Y_Yf, SMatF* _trn_X_Y, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params)
{
    LOGN("creating Xf Yf map...");
    TQDM tqdm(1, 1);

    SMatF* trn_X_Xf = new SMatF( _trn_X_Xf );
    SMatF* Y_Yf = new SMatF( _Y_Yf );
    SMatF* trn_X_Y = new SMatF( _trn_X_Y );
    int sparsity = params.get<int>("bs_count");
    float threshold = params.get<float>("bs_threshold");

    int num_X = trn_X_Xf->nc;
    int num_Y = Y_Yf->nc;
    int num_Xf = trn_X_Xf->nr;
    int num_Yf = Y_Yf->nr;

    trn_X_Xf->set_values( 1.0 );
    Y_Yf->set_values( 1.0 );
    trn_X_Y->set_values( 1.0 );
    
    SMatF* trn_X_Yf = Y_Yf->prod( trn_X_Y );
    SMatF* trn_Y_X = trn_X_Y->transpose();
    SMatF* trn_Y_Xf = trn_X_Xf->prod( trn_Y_X );
    delete trn_Y_X;

    VecF Yf_freq = trn_X_Yf->get_sum( 1 );
    VecF Xf_freq = trn_Y_Xf->get_sum( 1 );
    VecF Xf_freq1 = trn_X_Xf->get_sum( 1 );
    VecF Yf_freq1 = Y_Yf->get_sum( 1 );
    int num_X_Y = trn_X_Y->get_nnz();

    LOG("creating Yf_Xf...");
    SMatF* Yf_Y = Y_Yf->transpose();
    Yf_Xf = trn_Y_Xf->prod_for_jaccard( Yf_Y, Xf_freq, Yf_freq, num_X_Y, Xf_freq1, Yf_freq1, params.get<float>("bs_alpha"), sparsity, -0.1, true, params.get<int>("num_thread"));
    Yf_Xf->threshold( threshold );

    for( int i=0; i<Yf_Xf->nc; i++ )
        for( int j=0; j<Yf_Xf->size[i]; j++ )
            Yf_Xf->data[i][j].second -= threshold;

    delete Yf_Y;

    LOG("creating Xf_Yf...");
    SMatF* trn_Xf_X = trn_X_Xf->transpose();
    Xf_Yf = trn_X_Yf->prod_for_jaccard( trn_Xf_X, Yf_freq, Xf_freq, num_X_Y, Yf_freq1, Xf_freq1, params.get<float>("bs_alpha"), sparsity, -0.1, true, params.get<int>("num_thread"));
    Xf_Yf->threshold( threshold );

    for( int i=0; i<Xf_Yf->nc; i++ )
        for( int j=0; j<Xf_Yf->size[i]; j++ )
            Xf_Yf->data[i][j].second -= threshold;

    delete trn_Xf_X;
    delete trn_Y_Xf;
    delete trn_X_Yf;
    delete trn_X_Xf;
    delete Y_Yf;
    delete trn_X_Y;

    tqdm.step();
}

SMatF* create_Xf_Yf_map_direct( VecS& Xf, VecS& Yf, float wt )
{
    int num_Xf = Xf.size();
    int num_Yf = Yf.size();

    map<string,int> Xf_indices;
    for( int i=0; i<num_Xf; i++ )
        Xf_indices[ Xf[i] ] = i;

    VecII Xf_Yf_pairs;
    for( int i=0; i<num_Yf; i++ )
    {
        size_t tag_pos = Yf[i].find_first_of( '_' );
        string s = Yf[i].substr( tag_pos+1 );
        if( Xf_indices.find( s ) != Xf_indices.end() )
            Xf_Yf_pairs.push_back( pairII( Xf_indices[s], i ) );
    }

    //sort( Xf_Yf_pairs.begin(), Xf_Yf_pairs.end(), comp_pair_by_first<int,int> );

    SMatF* Xf_Yf_map = new SMatF( num_Yf, num_Xf );
    for( int i=0; i<Xf_Yf_pairs.size(); i++ )
        Xf_Yf_map->size[ Xf_Yf_pairs[i].first ]++;
    for( int i=0; i<num_Xf; i++ )
        Xf_Yf_map->data[ i ] = new pairIF[ Xf_Yf_map->size[ i ] ];

    VecI counts( num_Xf, 0 );
    for( int i=0; i<Xf_Yf_pairs.size(); i++ )
    {
        int Xf_ind = Xf_Yf_pairs[i].first;
        int Yf_ind = Xf_Yf_pairs[i].second;
        Xf_Yf_map->data[ Xf_ind ][ counts[ Xf_ind ]++ ] = pairIF( Yf_ind, wt );
    }

    for( int i=0; i<num_Xf; i++ )
        sort( Xf_Yf_map->data[ i ], Xf_Yf_map->data[ i ] + Xf_Yf_map->size[ i ], comp_pair_by_first<int,float> );

    return Xf_Yf_map;
}

void create_Xf_Yf( SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* trn_X_Y, VecS& Xf, VecS& Yf, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params )
{
    float bs_direct_wt = params.get<float>( "bs_direct_wt" );
 
    create_Xf_Yf_map( trn_X_Xf, Y_Yf, trn_X_Y, Xf_Yf, Yf_Xf, params);
    Xf_Yf->print_shape();
    Yf_Xf->print_shape();

    if( bs_direct_wt > 0)
    {
        SMatF* Xf_Yf_direct = create_Xf_Yf_map_direct( Xf, Yf, bs_direct_wt );
        Xf_Yf->add(Xf_Yf_direct);

        Xf_Yf_direct->dump(params.get<string>("model_dir") + OS_SEP + "direct_Xf_Yf.bin");
        delete Xf_Yf_direct;
    }

    remove_duplicates(Yf_Xf, Xf_Yf);

    Xf_Yf->eliminate_zeros();
    Yf_Xf->eliminate_zeros();
}

/************ utilities for converting bilinear matrix to linear format ************/

SMatF* bilinear_to_linear_form_helper( SMatF* X_Xf, SMatF* Y_Yf, SMatF* X_Y, SMatF* Xf_Yf, bool pp, int num_thread)
{
    int num_X = X_Xf->nc;
    int num_Xf = X_Xf->nr;
    int num_Y = Y_Yf->nc;
    int num_Yf = Y_Yf->nr;
    int num_pairs = X_Y->get_nnz();

    VecI start_feat_index( num_Xf, 0 );
    for( int i=1; i<num_Xf; i++ )
        start_feat_index[i] = start_feat_index[i-1] + Xf_Yf->size[i-1];

    SMatF* linear_form_mat = new SMatF( Xf_Yf->get_nnz(), num_pairs );

    X_Y->build_index();
    TQDM tqdm(num_X, 1000);
    #pragma omp parallel num_threads(num_thread)
    {
        vector< VecIF > active_Xf_per_Yf( num_Yf, VecIF() );
        #pragma omp for
        for( int i=0; i<num_X; i++ )
        {
            if(pp) tqdm.step();
            unordered_map< int, bool > active_Yf;

            for( int j=0; j<X_Xf->size[i]; j++ )
            {
                int Xf_ind = X_Xf->data[i][j].first;
                float Xf_val = X_Xf->data[i][j].second;

                for( int k=0; k<Xf_Yf->size[ Xf_ind ]; k++ )
                {
                    int Yf_ind = Xf_Yf->data[ Xf_ind ][ k ].first;

                    // float Xf_Yf_weight = Xf_Yf->data[ Xf_ind ][ k ].second;
                    float Xf_Yf_weight = 1.0;
                    
                    active_Yf[ Yf_ind ] = true;
                    active_Xf_per_Yf[ Yf_ind ].push_back( pairIF( start_feat_index[ Xf_ind ] + k, Xf_val*Xf_Yf_weight ) );
                }
            }

            for( int j=0; j<X_Y->size[i]; j++ )
            {
                VecIF X_Y_feats;

                int Y_ind = X_Y->data[i][j].first;

                for( int k=0; k<Y_Yf->size[ Y_ind ]; k++ )
                {
                    int Yf_ind = Y_Yf->data[ Y_ind ][ k ].first;
                    float Yf_val = Y_Yf->data[ Y_ind ][ k ].second;

                    for( int l=0; l<active_Xf_per_Yf[ Yf_ind ].size(); l++ )
                        X_Y_feats.push_back( pairIF( active_Xf_per_Yf[ Yf_ind ][ l ].first, active_Xf_per_Yf[ Yf_ind ][ l ].second * Yf_val ) );
                }

                sort( X_Y_feats.begin(), X_Y_feats.end(), comp_pair_by_first<int,float> );

                int c = X_Y->get_index(i, Y_ind);
                assert(c >= 0);
                linear_form_mat->size[ c ] = X_Y_feats.size();
                linear_form_mat->data[ c ] = new pairIF[ X_Y_feats.size() ];
                copy( X_Y_feats.begin(), X_Y_feats.end(), linear_form_mat->data[c] );
            }

            for( auto kv : active_Yf )
                active_Xf_per_Yf[ kv.first ].clear();
        }
    }

    X_Y->clear_index();
    return linear_form_mat;
}

SMatF* bilinear_to_linear_form( SMatF* X_Xf, SMatF* Y_Yf, SMatF* X_Y, SMatF* Xf_Yf, SMatF* Yf_Xf, bool add_biases, bool pp, int num_thread)
{
    if(pp) { LOGN("Begin bilinear_to_linear_form"); }
    TQDM tqdm(1, 1);

    int num_X = X_Xf->nc;
    int num_Xf = X_Xf->nr;
    int num_Y = Y_Yf->nc;
    int num_Yf = Y_Yf->nr;
    int num_pairs = X_Y->get_nnz();

    SMatF* linear_form_mat = bilinear_to_linear_form_helper( X_Xf, Y_Yf, X_Y, Xf_Yf, pp, num_thread );
    SMatF* Y_X = X_Y->transpose();
    SMatF* linear_form_mat1 = bilinear_to_linear_form_helper( Y_Yf, X_Xf, Y_X, Yf_Xf, pp, num_thread );

    int c = 0;
    vector< tupleIII > indices;
    for( int i=0; i<num_X; i++ )
    {
        for( int j=0; j<X_Y->size[i]; j++ )
        {
            indices.push_back( make_tuple( i, X_Y->data[i][j].first, c ) );
            c++;
        }
    }

    sort( indices.begin(), indices.end(), TupleSorter_second_first() );

    c = 0;
    for( int i=0; i<Y_X->nc; i++ )
        for( int j=0; j<Y_X->size[i]; j++ )
        {
            assert( i == get<1>( indices[c] ) );
            assert( Y_X->data[i][j].first == get<0>( indices[c] ) );
            c++;
        }

    delete Y_X;

    int old_nr = linear_form_mat->nr;
    
    for( int i=0; i<num_pairs; i++ )
    {
        int index = get<2>( indices[i] );

        int old_size = linear_form_mat->size[ index ];
        int new_size = old_size +  linear_form_mat1->size[ i ];

        Realloc( old_size, new_size, linear_form_mat->data[ index ] );

        for( int j=0; j<linear_form_mat1->size[i]; j++ )
            linear_form_mat->data[ index ][ old_size + j ] = pairIF( old_nr + linear_form_mat1->data[i][j].first, linear_form_mat1->data[i][j].second );

        linear_form_mat->size[ index ] = new_size;
    }

    linear_form_mat->nr += linear_form_mat1->nr;

    delete linear_form_mat1;
    old_nr = linear_form_mat->nr;

    if( add_biases )
    {
        VecI Xf_maps( old_nr );
        VecI Yf_maps( old_nr );

        int c = 0;
        for( int i=0; i<Xf_Yf->nc; i++ )
        {
            for( int j=0; j<Xf_Yf->size[i]; j++ )
            {
                Xf_maps[ c ] = i;
                Yf_maps[ c ] = Xf_Yf->data[i][j].first;
                c++;
            }
        }

        for( int i=0; i<Yf_Xf->nc; i++ )
        {
            for( int j=0; j<Yf_Xf->size[i]; j++ )
            {
                Xf_maps[ c ] = Yf_Xf->data[i][j].first;
                Yf_maps[ c ] = i; 
                c++;
            }
        }

        VecB Xf_mask( num_Xf, false );
        VecB Yf_mask( num_Yf, false );

        sort( indices.begin(), indices.end(), TupleSorter_first_second() );

        for( int i=0; i<linear_form_mat->nc; i++ )
        {
            int old_siz = linear_form_mat->size[i];
            
            for( int j=0; j<old_siz; j++ )
            {
                int ind = linear_form_mat->data[i][j].first;
                Xf_mask[ Xf_maps[ ind ] ] = true;
                Yf_mask[ Yf_maps[ ind ] ] = true;
            }

            int point = get<0>( indices[i] );
            int label = get<1>( indices[i] );

            int num_point_ft = X_Xf->size[ point ];
            int num_label_ft = Y_Yf->size[ label ];

            int new_siz = old_siz + num_point_ft + num_label_ft;

            Realloc( old_siz, new_siz, linear_form_mat->data[i] );

            int c = old_siz;

            for( int j=0; j<num_label_ft; j++ )
            {
                {
                    //linear_form_mat->data[i][c++] = pairIF( old_nr + num_Xf + Y_Yf->data[label][j].first, 1.0 );
                    linear_form_mat->data[i][c++] = pairIF( old_nr + num_Xf + Y_Yf->data[label][j].first, Y_Yf->data[label][j].second );
                }
            }

            for( int j=0; j<old_siz; j++ )
            {
                int ind = linear_form_mat->data[i][j].first;
                Xf_mask[ Xf_maps[ ind ] ] = false;
                Yf_mask[ Yf_maps[ ind ] ] = false;
            }

            linear_form_mat->size[i] = c;
            Realloc( new_siz, c, linear_form_mat->data[i] );
        }
        linear_form_mat->nr = old_nr + num_Xf + num_Yf;
    }

    if(pp)
    {
        tqdm.step();
        LOGN("End bilinear_to_linear_form");
    }

    return linear_form_mat;
}