#include <iostream>
#include <omp.h>

#include "mat.h"
#include "utils.h"
#include "timer.h"
#include "zestxml.h"
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
    params.set<string>("type", "-");
    
    params.set<string>("trn_X_Xf", "-");
    params.set<string>("tst_X_Xf", "-");
    params.set<string>("trn_X_Y", "-");
    params.set<string>("Y_Yf", "-");
    params.set<string>("Xf", "-");
    params.set<string>("Yf", "-");
    params.set<float>("propensity_A", 0.55);
    params.set<float>("propensity_B", 1.5);
    params.set<string>("seen_labels", "-");
    params.set<string>("sparsity_pattern_file", "-");

    params.set<int>("bs_count", 10);
    params.set<float>("bs_threshold", 0);
    params.set<float>("bs_alpha", 0.2);
    params.set<float>("bs_direct_wt", 0.2);
    params.set<bool>("binary_relevance", true); // If true, use non-binary values in X_Y. If false, set all +ves in X_Y to 1
    params.set<float>("score_alpha", 0.9);
    params.set<int>("F", 10);
    params.set<int>("shortyK", 100);
    
    params.set<bool>("bilinear_add_bias", false);
    params.set<bool>("bilinear_normalize", true);
    params.set<int>("bilinear_classifier_kind", 0);
    params.set<int>("bilinear_classifier_maxitr", 20);
    params.set<float>("bilinear_classifier_pos_wt", 1.0);
    params.set<float>("bilinear_classifier_cost", 1.0);

    params.set<string>("res_dir", "Results");
    params.set<string>("model_dir", "Results/model");
}

// if not weighted training then binarize X_Y and unit normalize all feature mats
void prepare(SMatF* X_Xf, SMatF* X_Y, SMatF* Y_Yf, SMatF* X_Y_feat, bool weighted = false)
{
    if(X_Y != NULL && weighted) 
        X_Y->set_values(1.0);
    if(X_Xf != NULL ) 
        X_Xf->unit_normalize_columns(); 
    if(Y_Yf != NULL) 
        Y_Yf->unit_normalize_columns();
    if(X_Y_feat != NULL) 
        X_Y_feat->unit_normalize_columns();
}

VecI remove_test_labels(SMatF* Y_Yf, SMatF* trn_X_Y, VecI seen_labels = VecI())
{
    if(seen_labels.size() == 0)
    {
        VecI nnz = trn_X_Y->get_freq(1);
        for(int y = 0; y < trn_X_Y->nr; ++y)
        {
            if(nnz[y] == 0)
            {
                Y_Yf->size[y] = 0;
                delete[] Y_Yf->data[y];
                Y_Yf->data[y] = NULL;
            }
            else
                seen_labels.push_back(y);
        }
    }
    else
    {
        set<int> unseen_labels;
        for(int y = 0; y < trn_X_Y->nr; ++y) unseen_labels.insert(y);
        for(auto y : seen_labels) unseen_labels.erase(y);
        for(auto y : unseen_labels)
        {
            Y_Yf->size[y] = 0;
            delete[] Y_Yf->data[y];
            Y_Yf->data[y] = NULL;
        }
    }
    return seen_labels;
}



void run_xhtp_approx(Parameters& params)
{
    LOG("loading input...");

    SMatF* trn_X_Xf                 = new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf                     = new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y                  = new SMatF(params.get<string>("trn_X_Y"));
    VecS Xf                         = read_desc_file(params.get<string>("Xf"));
    VecS Yf                         = read_desc_file(params.get<string>("Yf"));

    int   bs_count        = params.get<int>("bs_count");
    float bs_threshold    = params.get<float>("bs_threshold");
    float bs_direct_wt    = params.get<float>("bs_direct_wt");
    bool binary_relevance = params.get<bool>( "binary_relevance" );
    
    LOGN("loaded input.");
    LOGN("bs_count         : " << bs_count);
    LOGN("bs_threshold     : " << bs_threshold);
    LOGN("bs_alpha         : " << params.get<float>("bs_alpha"));
    LOGN("bs_direct_wt     : " << bs_direct_wt);
    LOGN("binary_relevance : " << binary_relevance);

    double time = 0;
    Timer timer; 
    timer.tic();
    
    remove_test_labels(Y_Yf, trn_X_Y);
    prepare(trn_X_Xf, trn_X_Y, Y_Yf, NULL, not binary_relevance);
    if(not binary_relevance)
        ips_weight(trn_X_Y, params);

    timer.tic();
    SMatF* Xf_Yf = NULL;
    SMatF* Yf_Xf = NULL;
    create_Xf_Yf(trn_X_Xf, Y_Yf, trn_X_Y, Xf, Yf, Xf_Yf, Yf_Xf, params);
    time += timer.toc();

    SMatF* Yf_Xf_t = Yf_Xf->transpose();
    SMatF* sparsity_pattern = new SMatF(Xf_Yf);
    sparsity_pattern->add( Yf_Xf_t );
    delete Yf_Xf_t;

    LOGN("[STAT] nnz of sparsity pattern mat : " << sparsity_pattern->get_nnz());
    LOGN("[STAT] avg nnz of sparsity pattern mat per row : " << (float) sparsity_pattern->get_nnz() / (float)sparsity_pattern->nr );
    LOGN("[STAT] avg nnz of sparsity pattern mat per col : " << (float) sparsity_pattern->get_nnz() / (float)sparsity_pattern->nc );
    LOGN("[STAT] nnz of Xf_Yf mat : " << Xf_Yf->get_nnz());
    LOGN("[STAT] nnz of Yf_Xf mat : " << Yf_Xf->get_nnz());

    Xf_Yf->dump(params.get<string>("model_dir") + OS_SEP + "Xf_Yf." + "bin");
    Yf_Xf->dump(params.get<string>("model_dir") + OS_SEP + "Yf_Xf." + "bin");
    sparsity_pattern->dump(params.get<string>("model_dir") + OS_SEP + "sparsity_pattern." + "bin");

    delete Xf_Yf;
    delete Yf_Xf;
    delete sparsity_pattern;
    delete trn_X_Xf;
    delete Y_Yf;
    delete trn_X_Y;

    LOGN(fixed << setprecision(2) << "\nfinished in " << time << " s");
}

void run_xhtp_fine_tune(Parameters& params)
{
    LOGN("loading input...");
   
    SMatF* trn_X_Xf             = new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf                 = new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y              = new SMatF(params.get<string>("trn_X_Y"));

    SMatF* sparsity_pattern     = NULL;
    SMatF* Xf_Yf                = NULL;
    SMatF* Yf_Xf                = NULL;

    LOGN("loading sparsity pattern mat from model dir");
    sparsity_pattern = new SMatF(params.get<string>("model_dir") + OS_SEP + "sparsity_pattern." + "bin");
    Xf_Yf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Xf_Yf." + "bin");
    Yf_Xf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Yf_Xf." + "bin");
   
    LOG("praparing train...");

    ifstream fin(params.get<string>("model_dir") + OS_SEP + "seen_labels.txt", ios::in);
    VecI seen_labels;

    if(fin.good())
    {
        LOGN("using seen labels file : " << params.get<string>("model_dir") + OS_SEP + "seen_labels.txt");
        int y;
        while(fin >> y) 
            seen_labels.push_back(y);
        remove_test_labels(Y_Yf, trn_X_Y, seen_labels);
    }
    else
    {
        LOGN("generating seen labels from trn_X_Y");
        seen_labels = remove_test_labels(Y_Yf, trn_X_Y);
        ofstream fout(params.get<string>("model_dir") + OS_SEP + "seen_labels.txt", ios::out);
        for(auto val : seen_labels) 
            fout << val << "\n";
        fout.close();
    }
    fin.close();

    double time = 0;
    Timer timer; timer.tic();
    prepare(trn_X_Xf, trn_X_Y, Y_Yf, NULL);

    LOGN("training...");
    train(trn_X_Y, seen_labels, trn_X_Xf, Y_Yf, sparsity_pattern, Xf_Yf, Yf_Xf, params);
    time += timer.toc();

    LOGN(fixed << setprecision(2) << "\nfinished in " << time << " s");
}

void run_predict(Parameters& params)
{
    LOGN("loading input...");

    SMatF* tst_X_Xf             = new SMatF(params.get<string>("tst_X_Xf"));
    SMatF* trn_X_Y              = new SMatF(params.get<string>("trn_X_Y"));
    SMatF* tst_X_Y              = new SMatF(params.get<string>("tst_X_Y"));
    SMatF* Y_Yf                 = new SMatF(params.get<string>("Y_Yf"));
    SMatF* direct_Xf_Yf         = new SMatF(params.get<string>("model_dir") + OS_SEP + "direct_Xf_Yf.bin");

    VecI seen_labels; int y;
    ifstream fin(params.get<string>("model_dir") + OS_SEP + "seen_labels.txt", ios::in);
    while(fin >> y) seen_labels.push_back(y);

    SMatF* sparsity_pattern     = NULL;
    SMatF* Xf_Yf                = NULL;
    SMatF* Yf_Xf                = NULL;
    if(params.get<string>("sparsity_pattern_file").compare("-") == 0)
    {
        LOGN("loading sparsity pattern mat from model dir");
        sparsity_pattern     = new SMatF(params.get<string>("model_dir") + OS_SEP + "sparsity_pattern.bin");
        Xf_Yf                = new SMatF(params.get<string>("model_dir") + OS_SEP + "Xf_Yf.bin");
        Yf_Xf                = new SMatF(params.get<string>("model_dir") + OS_SEP + "Yf_Xf.bin");
    }
    else
    {
        LOGN("loading sparsity pattern mat from file : " << params.get<string>("sparsity_pattern_file"));
        sparsity_pattern = new SMatF(params.get<string>("sparsity_pattern_file"));
        Xf_Yf            = new SMatF(sparsity_pattern);
        Yf_Xf            = new SMatF(Xf_Yf->nc, Xf_Yf->nr);
    }
    
    LOGN("loading model...");

    VecF bilinear_clf;
    fin.close(); fin.open(params.get<string>("model_dir") + OS_SEP + "bilinear_clf.bin", ios::in | ios::binary);
    read_vec_bin(bilinear_clf, fin);

    double time = 0;
    Timer timer; timer.tic();
    LOG("preparing predict...");
    prepare(tst_X_Xf, NULL, Y_Yf, NULL);

    predict(bilinear_clf, tst_X_Y, trn_X_Y, tst_X_Xf, Y_Yf, sparsity_pattern, seen_labels, Xf_Yf, Yf_Xf, direct_Xf_Yf, params);
    time += timer.toc();

    LOGN(fixed << setprecision(2) << "\nfinished in " << time << " s");
}

int main(int argc, char const *argv[])
{
    Parameters params;
    fill_default_params(params);
    fill_arg_params(argc, argv, params);
    string param_file = params.get<string>("res_dir") + string(OS_SEP) + string("params.txt");
    params.dump(param_file);

    string type = params.get<string>("type");
    LOGN("running : " << type << " using " << params.get<int>("num_thread") << " thread(s)");

    if(type == "xhtp_approx" or type == "train" or type == "all")
        run_xhtp_approx(params);
    if(type == "xhtp_fine_tune" or type == "train" or type == "all")
        run_xhtp_fine_tune(params);
    if(type == "predict" or type == "all")
        run_predict(params);

    return 0;
}
