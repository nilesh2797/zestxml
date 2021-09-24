#include "zestxml.h"
using namespace std;

/************** shortlisting functions **************/

SMatF* get_approx_shortlist(SMatF* X_Y, SMatF* X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, Parameters& params)
{
	int num_Yf = Y_Yf->nr;
	int num_Y = Y_Yf->nc;
	int num_X = X_Xf->nc;
	int num_Xf = X_Xf->nr;

	int F = params.get<int>("F");
	double min_lim = 1e-6;
	int shortlist_size = params.get<int>("shortyK");

	LOGN("F       : " << F);
	LOGN("min_lim : " << min_lim);

	vector<VecIF> inverted_index(num_Xf);
	for(int i = 0; i < sparsity_pattern->nc; ++i)
		inverted_index[i] = VecIF(sparsity_pattern->data[i], sparsity_pattern->data[i]+sparsity_pattern->size[i]);

	SMatF* Yf_Y = Y_Yf->transpose();

	for( int i=0; i<num_Yf; i++ )
	    if( Yf_Y->size[i]>0 )
	        sort( Yf_Y->data[i], Yf_Y->data[i]+Yf_Y->size[i], comp_pair_by_second_desc<int,float> );

	SMatF* pred_X_Y = new SMatF( num_Y, num_X );

	TQDM tqdm(num_X, 1000);

	#pragma omp parallel num_threads(params.get<int>("num_thread"))
	{
		VecF point_proj( num_Yf, 0 );
		VecB seen_Y( num_Y, false );
		VecI Yf_itr( num_Yf, 0 );
		DenseSVec temp(num_Yf, 0);

		#pragma omp for
		for( int i=0; i<num_X; i++ )
		{
			tqdm.step();

			for(int j = 0; j < X_Xf->size[i]; ++j)
			{
				int xf = X_Xf->data[i][j].first;
				float xf_val = X_Xf->data[i][j].second;
				for(int k = 0; k < inverted_index[xf].size(); ++k)
					temp.add(inverted_index[xf][k].first, xf_val*inverted_index[xf][k].second);
			}
			
			VecIF x_Yf = temp.vecif();
			sort(x_Yf.begin(), x_Yf.end(), comp_pair_by_second_desc<int, float>);
			temp.reset();

		    VecI active_Y;
		    double lim = 1.0;

		    while( active_Y.size() < F*shortlist_size && lim>min_lim )
		    {
		        for( int j=0; j<x_Yf.size(); j++ )
		        {
		            int Yf_ind = x_Yf[j].first;
		            float Yf_point_val = x_Yf[j].second;

		            if( Yf_point_val < lim )
		                break;

		            double lim1 = lim/Yf_point_val;

		            int c = Yf_itr[ Yf_ind ];
		            int siz = Yf_Y->size[ Yf_ind ];
		            for( ; c<siz ; c++ )
		            {
		                if( Yf_Y->data[ Yf_ind ][ c ].second < lim1 )
		                    break;

		                int label = Yf_Y->data[ Yf_ind ][ c ].first;
		                if( ! seen_Y[ label ] )
		                {
		                    active_Y.push_back( label );
		                    seen_Y[ label ] = true;
		                }
		            }
		            Yf_itr[ Yf_ind ] = c;
		        }

		        lim /= 2;
		    }

		    pred_X_Y->size[i] = active_Y.size();
		    pred_X_Y->data[i] = new pairIF[ active_Y.size() ];

		    for( int j=0; j<x_Yf.size(); j++ )
		        point_proj[ x_Yf[j].first ] = x_Yf[j].second;

		    for( int j=0; j<active_Y.size(); j++ )
		    {
		        int label = active_Y[j];
		        float prod = 0;
		        for( int k=0; k<Y_Yf->size[label]; k++ )
		            prod += point_proj[ Y_Yf->data[label][k].first ] * Y_Yf->data[label][k].second;

		        pred_X_Y->data[i][j] = pairIF( active_Y[j], prod );
		        seen_Y[ active_Y[j] ] = false;
		    }

		    sort( pred_X_Y->data[i], pred_X_Y->data[i] + pred_X_Y->size[i], comp_pair_by_second_desc<int,float> );
		    int new_siz = min( (int)active_Y.size(), shortlist_size);
		    Realloc( pred_X_Y->size[i], new_siz, pred_X_Y->data[i] );
		    pred_X_Y->size[i] = new_siz;
		    sort( pred_X_Y->data[i], pred_X_Y->data[i] + pred_X_Y->size[i], comp_pair_by_first<int,float> );

		    for( int j=0; j<x_Yf.size(); j++ )
		    {
		        point_proj[ x_Yf[j].first ] = 0;
		        Yf_itr[ x_Yf[j].first ] = 0;
		    }
		}
	}
    delete Yf_Y; Yf_Y = NULL;

    return pred_X_Y;
}

SMatF* get_exact_shortlist(SMatF* X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, Parameters& params)
{
	SMatF* Yf_Y = Y_Yf->transpose();
	SMatF* X_Yf = sparsity_pattern->prod(X_Xf, -1, -0.1, "sum", params.get<int>("num_thread"), true);
	SMatF* xshortlist = Yf_Y->prod(X_Yf, params.get<int>("shortyK"), -0.1, "sum", params.get<int>("num_thread"), true);
	
	delete Yf_Y; Yf_Y = NULL;
	delete X_Yf; X_Yf = NULL;
	return xshortlist;
}

/****************** training function ******************/

VecF learn_bilinear_classifier(SMatF* assign_mat, SMatF* Xf_Yf, SMatF* Yf_Xf, SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* trn_X_Y, Parameters& params, mt19937& reng)
{
	LOGN("learning shared classifier...");
	LOGN("[STAT] : nnz in assign_mat : " << assign_mat->get_nnz());

	float classifier_eps 	= 0.01;
	float classifier_cost 	= params.get<float>("bilinear_classifier_cost");
	_Classifier_Kind classifier_kind = (_Classifier_Kind)params.get<int>("bilinear_classifier_kind");

	VecF w;

	LOGN("classifier_cost : " << classifier_cost);
	LOGN("classifier_kind : " << _Classifier_Kind_Names[classifier_kind]);
	TQDM tqdm(1, 1);

    SMatF* linear_form_mat = bilinear_to_linear_form(trn_X_Xf, Y_Yf, assign_mat, Xf_Yf, Yf_Xf, params.get<bool>("bilinear_add_bias"), true, params.get<int>("num_thread"));
    if(params.get<bool>("bilinear_normalize"))
        linear_form_mat->unit_normalize_columns();

    linear_form_mat->append_bias_feat(1.0);

    int num_inst 			= linear_form_mat->nc;
    int num_fts				= linear_form_mat->nr;
    int numy 				= trn_X_Y->nr;

    VecI y(num_inst, -1);
    VecF wts(num_inst, classifier_cost);
    VecI nnz = trn_X_Y->get_freq(1);
    if(w.size() == 0) w.resize(num_fts, 0.0);

    assign_mat->build_index();
    trn_X_Y->build_index();
    for(int ind = 0; ind < assign_mat->index.size(); ++ind)
    {
        pairII coo = assign_mat->get_coo(ind);
        if(trn_X_Y->exists(coo.first, coo.second))
        {
            y[ind] 	= +1;
            wts[ind] *= (params.get<float>("bilinear_classifier_pos_wt"));
        }
    }
    trn_X_Y->clear_index();
    assign_mat->clear_index();

    LOG("training bilinear svm...");
    if(classifier_kind == L2R_L2LOSS_SVC_DUAL)
        solve_l2r_l2loss_svc_dual(linear_form_mat, y.data(), w.data(), classifier_eps, wts.data(), params.get<int>("bilinear_classifier_maxitr"), reng);
    else if(classifier_kind == L2R_LR_DUAL)
        solve_l2r_lr_dual(linear_form_mat, y.data(), w.data(), classifier_eps, wts.data(), params.get<int>("bilinear_classifier_maxitr"), reng);

    VecIF scores = apply_classifier(w, NULL, linear_form_mat->nc, linear_form_mat, classifier_kind, false, true);
    delete linear_form_mat;
    float pos_acc = 0, neg_acc = 0, acc = 0;
    int pos_num = 0, neg_num = 0, num = scores.size();
    for(int i = 0; i < num; ++i)
    {
        if(y[i] == +1)
            pos_acc += scores[i].second, pos_num += 1;
        else 
            neg_acc += (1 - scores[i].second), neg_num += 1;
    }
    acc = (pos_acc + neg_acc);
    acc *= (100.0/num); pos_acc *= (100.0/pos_num); neg_acc *= (100.0/max(1, neg_num));
    LOGN(fixed << setprecision(2) << "acc : " << acc << "% pos acc : " << pos_acc << "% neg acc : " << neg_acc << "%");

    tqdm.step();
	
	return w;
}

void train(SMatF* trn_X_Y, VecI seen_labels, SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, SMatF* Xf_Yf, SMatF* Yf_Xf, Parameters& params)
{
	LOGN("\ngetting " << params.get<int>("shortyK") << " shortlist per point...");
	mt19937 reng; // random number generator used during training 
	int num_thread = params.get<int>("num_thread");

	// SMatF* shortlist = get_exact_shortlist(trn_X_Xf, Y_Yf, sparsity_pattern, params);
	SMatF* shortlist = get_approx_shortlist(trn_X_Y, trn_X_Xf, Y_Yf, sparsity_pattern, params);
    shortlist->dump(params.get<string>("model_dir") + OS_SEP + "shortlist.bin");
//     SMatF* shortlist = new SMatF(params.get<string>("model_dir") + OS_SEP + "shortlist.bin");
	LOGN("[STAT] nnz of shortlist    : " << shortlist->get_nnz());
	LOGN("[STAT] recall of shortlist : " << shortlist->get_recall(trn_X_Y) << "%");

    LOGN("\ntraining bilinear classifier");
//     shortlist->add(trn_X_Y);
    VecF bilinear_clf = learn_bilinear_classifier(shortlist, 
                                                  Xf_Yf,
                                                  Yf_Xf, 
                                                  trn_X_Xf, 
                                                  Y_Yf, 
                                                  trn_X_Y,
                                                  params,
                                                  reng);

    ofstream fout(params.get<string>("model_dir") + OS_SEP + "bilinear_clf.bin", ios::out | ios::binary);
    write_vec_bin(bilinear_clf, fout);

	delete shortlist;
}

/****************** prediction function ********************/

void predict(VecF& bilinear_clf, SMatF* tst_X_Y, SMatF* trn_X_Y, SMatF* tst_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, VecI& seen_labels, SMatF* Xf_Yf, SMatF* Yf_Xf, SMatF* direct_Xf_Yf, Parameters& params)
{
	LOGN("\ngetting " << params.get<int>("shortyK") << " shortlist per point...");
	
	int num_thread = params.get<int>("num_thread");
	int numx = tst_X_Xf->nc;
	int numy = Y_Yf->nc;

	SMatF* shortlist = get_approx_shortlist(trn_X_Y, tst_X_Xf, Y_Yf, sparsity_pattern, params);
	LOGN("[STAT] nnz of shortlist    : " << shortlist->get_nnz());
	LOGN("[STAT] recall of shortlist : " << shortlist->get_recall(tst_X_Y) << "%");
	shortlist->dump(params.get<string>("res_dir") + OS_SEP + "shortlist.bin");

    SMatF* bilinear_Xf_Yf = expand_linear_form_mat(bilinear_clf, sparsity_pattern, Xf_Yf, Yf_Xf);
    SMatF* bilinear_X_Yf = bilinear_Xf_Yf->prod(tst_X_Xf, -1, -0.1, "sum", params.get<int>("num_thread"), true);
    SMatF* bilinear_score_mat = Y_Yf->sparse_prod(bilinear_X_Yf, shortlist, true);
    delete bilinear_X_Yf; delete bilinear_Xf_Yf;

    float bias = bilinear_clf[bilinear_clf.size()-1];
    for(int i = 0; i < bilinear_score_mat->nc; ++i)
        for(int j = 0; j < bilinear_score_mat->size[i]; ++j)
            bilinear_score_mat->data[i][j].second = get_new_score(bilinear_score_mat->data[i][j].second + bias, 0, (_Classifier_Kind)params.get<int>("bilinear_classifier_kind"), false);

    bilinear_score_mat->dump(params.get<string>("res_dir") + OS_SEP + "bilinear_score_mat.bin");
    
    SMatF* knn_Xf_Yf = new SMatF(direct_Xf_Yf);
    knn_Xf_Yf->set_values(1.0);
    SMatF* knn_X_Yf = knn_Xf_Yf->prod(tst_X_Xf, -1, -0.1, "sum", params.get<int>("num_thread"), true);
    SMatF* knn_score_mat = Y_Yf->sparse_prod(knn_X_Yf, shortlist, true);
    delete knn_Xf_Yf; delete knn_X_Yf;
    knn_score_mat->dump(params.get<string>("res_dir") + OS_SEP + "knn_score_mat.bin");

    bilinear_score_mat->scalar_multiply(params.get<float>("score_alpha"));
    knn_score_mat->scalar_multiply(1.0 - params.get<float>("score_alpha"));
    bilinear_score_mat->add(knn_score_mat);
    bilinear_score_mat->dump(params.get<string>("res_dir") + OS_SEP + "score_mat.bin");                              
    
    delete knn_score_mat;
    delete bilinear_score_mat; bilinear_score_mat = NULL;
	delete shortlist;
}
