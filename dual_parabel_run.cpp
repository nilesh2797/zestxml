#include <iostream>
#include <omp.h>

#include "mat.h"
#include "utils.h"
#include "algos.h"
#include "timer.h"
#include "irmatch.h"
#include "dual_parabel.h"
#include "parameters.h"

void fill_arg_params(int argc, char const *argv[], Parameters& params)
{
	for(int i = 0; i < argc; ++i)
	{
		if(argv[i][0] == '-')
		{
			if((i < argc-1) && (argv[i+1][0] != '-')) 
				params.set<string>(argv[i]+1, argv[i+1]);
			else
				cerr << "Invalid argumet : no value provided for param " << argv[i]+1 << endl;
		}
	}
}

void fill_file_params(string file_name, Parameters& params)
{
	string pname, pval;
	ifstream f(file_name);

	if( !f.fail())
		while(f >> pname >> pval)
			params.set<string>(pname, pval);
	else
		cerr << "No file exists : " << file_name << endl;
}

void fill_default_params(Parameters& params)
{
	params.set<int>("num_thread", 1);
	params.set<string>("type", "run_bilinear_to_linear");
	params.set<string>("trn_X_Xf", "-");
	params.set<string>("Y_Yf", "-");
	params.set<string>("trn_X_Y", "-");
	params.set<string>("Xf", "-");
	params.set<string>("Yf", "-");
	params.set<string>("sparsity_pattern_file", "-");
	params.set<string>("Xf_Yf_map_kind", "-");
	params.set<string>("X_Y_feat_file", "-");
	params.set<int>("jaccard_sparsity", 10);
	params.set<float>("jaccard_threshold", 0);
	params.set<string>("res_dir", "Results");
}

// if not weighted training then binarize X_Y and unit normalize all feature mats
void prepare(SMatF* X_Xf, SMatF* X_Y, SMatF* Y_Yf, SMatF* X_Y_feat, Param& param )
{
	if(X_Y != NULL && !param.weighted) 
		X_Y->set_values(1.0);
	if(X_Xf != NULL ) 
		X_Xf->unit_normalize_columns();	
	if(Y_Yf != NULL) 
		Y_Yf->unit_normalize_columns();
	if(X_Y_feat != NULL) 
		X_Y_feat->unit_normalize_columns();
}

void run_bilinear_to_linear(Parameters& params)
{
	// initialize
	LOG("loading input...");

	SMatF* trn_X_Xf 				= new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf 					= new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y 					= new SMatF(params.get<string>("trn_X_Y"));
    VecS Xf 						= read_desc_file(params.get<string>("Xf"));
    VecS Yf 						= read_desc_file(params.get<string>("Yf"));
    string sparsity_pattern_file 	= string(params.get<string>("sparsity_pattern_file"));
    string Xf_Yf_map_kind 			= string(params.get<string>("Xf_Yf_map_kind")); // "DIRECT" => only Xf and Yf which exactly match are mapped, "JACCARD" => Xf and Yf with high jaccard similarity are mapped, "JACCARD_DIRECT" => both direct and jaccard maps are unioned
    string X_Y_feat_file 			= string(params.get<string>("X_Y_feat_file"));
    int   jaccard_sparsity  		= params.get<int>("jaccard_sparsity");
    float jaccard_threshold 		= params.get<float>("jaccard_threshold");
    
    LOGN("loaded input.");
	
	LOGN("jaccard_sparsity : " << jaccard_sparsity);
    LOGN("jaccard_threshold : " << jaccard_threshold);

    SMatF* Xf_Yf = NULL;
    SMatF* Yf_Xf = NULL; 
    int num_thread = omp_get_max_threads();
	if(params.get<int>("num_thread") > 0)
		num_thread = params.get<int>("num_thread");

    if( Xf_Yf_map_kind == "DIRECT" )
        Xf_Yf = create_Xf_Yf_map_direct( Xf, Yf );
    else if( Xf_Yf_map_kind == "JACCARD" )
        create_Xf_Yf_map_jaccard( trn_X_Xf, Y_Yf, trn_X_Y, jaccard_sparsity, jaccard_threshold, Xf_Yf, Yf_Xf );
    else if( Xf_Yf_map_kind == "JACCARD_DIRECT" )
        create_Xf_Yf_map_jaccard_direct( trn_X_Xf, Y_Yf, trn_X_Y, Xf, Yf, jaccard_sparsity, jaccard_threshold, Xf_Yf, Yf_Xf );

    SMatF* Yf_Xf_t = Yf_Xf->transpose();
    SMatF* sp_pattern_mat = Xf_Yf->emax(Yf_Xf_t);
    delete Yf_Xf_t, Xf_Yf, Yf_Xf;

    trn_X_Xf->set_values( 1.0 ); // TODO: comment if harmful to final accuracy
    Y_Yf->set_values( 1.0 ); // TODO: comment if harmful to final accuracy

    SMatF* linear_form_mat = bilinear_to_linear_form(trn_X_Xf, Y_Yf, trn_X_Y, sp_pattern_mat, num_thread);

    LOGN("\nStats : ");
    LOGN("avg. ft per X_Y pair: " << ( (float)linear_form_mat->get_nnz() / (float)linear_form_mat->nc ));

    int c = 0;
    for( int i=0; i<linear_form_mat->nc; i++ )
        if( linear_form_mat->size[i]>=1 )
            c++;

    LOGN("X_Y pair fraction with at least 1 feat: " << ( (float)c / (float)linear_form_mat->nc ));

    c = 0;
    for( int i=0; i<linear_form_mat->nc; i++ )
        if( linear_form_mat->size[i]>=2 )
            c++;

    LOGN("X_Y pair fraction with at least 2 feat: " << ( (float)c / (float)linear_form_mat->nc ));

    c = 0;
    for( int i=0; i<linear_form_mat->nc; i++ )
        if( linear_form_mat->size[i]>=5 )
            c++;

    LOGN("X_Y pair fraction with at least 5 feat: " << ( (float)c / (float)linear_form_mat->nc ));

    c = 0;
    for( int i=0; i<linear_form_mat->nc; i++ )
        if( linear_form_mat->size[i]>=10 )
            c++;

    LOGN("X_Y pair fraction with at least 10 feat: " << ( (float)c / (float)linear_form_mat->nc ));
    LOGN("X_Y non zeros: " << trn_X_Y->get_nnz());

    //linear_form_mat->trunc_max_nnz_per_col( 20 );

    linear_form_mat->dump(X_Y_feat_file);
    delete linear_form_mat;

    sp_pattern_mat->write( sparsity_pattern_file );
    delete sp_pattern_mat;
    delete trn_X_Xf;
    delete Y_Yf;
    delete trn_X_Y;

    LOGN("finished :)");
}

void run_rating_tree(Parameters& params)
{
	// initialize
	LOG("loading input...");

	SMatF* trn_X_Xf 				= new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf 					= new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y 					= new SMatF(params.get<string>("trn_X_Y"));
    string sparsity_pattern_file 	= string(params.get<string>("sparsity_pattern_file"));
    SMatF* trn_X_Y_feat				= new SMatF(params.get<string>("X_Y_feat_file"));
    
    LOGN("loaded input.");

    Param model_params;
    cout << model_params << endl;

    int tree_no = model_params.start_tree;
	prepare(trn_X_Xf, trn_X_Y, Y_Yf, trn_X_Y_feat, model_params);

    Tree* tree = learn_rating_tree_structure(trn_X_Xf, trn_X_Y, Y_Yf, trn_X_Y_feat, model_params);
    
    LOGN("writing tree...");
    tree->dump(params.get<string>("res_dir") + OS_SEP + "0.tree.bin");

    LOGN("reading tree...");
    delete tree; tree = new Tree;
    tree->load(params.get<string>("res_dir") + OS_SEP + "0.tree.bin");

    LOGN("finished :)");

    delete tree;
    delete trn_X_Xf;
    delete trn_X_Y;
    delete Y_Yf;
    delete trn_X_Y_feat;
}

void run_train_tree(Parameters& params)
{
	LOG("loading input...");

	SMatF* trn_X_Xf 			= new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf 				= new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y 				= new SMatF(params.get<string>("trn_X_Y"));
    SMatF* sparsity_pattern 	= new SMatF(params.get<string>("sparsity_pattern_file"));
    SMatF* trn_X_Y_feat			= new SMatF(params.get<string>("X_Y_feat_file"));
    string param_file			= params.get<string>("param_file");
    
    LOGN("loaded input.");

    check_valid_filename(param_file);
    Param model_params(param_file);
    cout << endl << model_params << endl;

    int tree_no = model_params.start_tree;
    LOGN("praparing train...");
	prepare(trn_X_Xf, trn_X_Y, Y_Yf, trn_X_Y_feat, model_params);
	LOGN("learning tree...");
	Tree* tree = learn_tree(trn_X_Xf, trn_X_Y, Y_Yf, trn_X_Y_feat, sparsity_pattern, model_params, tree_no);

    tree->dump(params.get<string>("res_dir") + OS_SEP + to_string(tree_no) + ".tree.bin");

    LOGN("\nfinished :)");
}

void run_predict_tree(Parameters& params)
{
	LOGN("loading input...");
	SMatF* tst_X_Xf 			= new SMatF(params.get<string>("tst_X_Xf"));
    SMatF* Y_Yf 				= new SMatF(params.get<string>("Y_Yf"));
    SMatF* tst_X_Y 				= new SMatF(params.get<string>("tst_X_Y"));
    SMatF* sparsity_pattern 	= new SMatF(params.get<string>("sparsity_pattern_file"));
    string param_file			= params.get<string>("param_file");
    string res_dir              = params.get<string>("res_dir");
    
    LOGN("loading model...");
    Param model_params(param_file);
    cout << endl << model_params << endl;

    int tree_no = model_params.start_tree;
    Tree* tree = new Tree(params.get<string>("res_dir") + OS_SEP + to_string(tree_no) + ".tree.bin");

    sparsity_pattern->build_index();
	prepare(tst_X_Xf, tst_X_Y, Y_Yf, NULL, model_params);

	LOGN("predicting tree " << tree_no << "...");
	SMatF* score_mat = predict_tree(tree, tst_X_Xf, Y_Yf, sparsity_pattern, model_params, res_dir, tree_no);
    score_mat->dump(res_dir + OS_SEP + "score_mat." + to_string(tree_no) + ".txt");

    LOGN("\nfinished :)");
}

int main(int argc, char const *argv[])
{
	Parameters params;
	fill_default_params(params);
	fill_arg_params(argc, argv, params);
	string param_file = params.get<string>("res_dir") + string(OS_SEP) + string("params.txt");
	params.dump(param_file);

	string type = params.get<string>("type");
	LOGN("running : " << type);

	if(type == "run_bilinear_to_linear")
		run_bilinear_to_linear(params);
	else if(type == "run_rating_tree")
		run_rating_tree(params);
	else if(type == "run_train_tree")
		run_train_tree(params);
	else if(type == "run_predict_tree")
		run_predict_tree(params);

    return 0;
}