#include <iostream>
#include <omp.h>

#include "mat.h"
#include "utils.h"
#include "algos.h"
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
    params.set<string>("type", "run_bilinear_to_linear");
    params.set<string>("trn_X_Xf", "-");
    params.set<string>("tst_X_Xf", "-");
    params.set<string>("trn_X_Y", "-");
    params.set<string>("tst_X_Y", "-");
    params.set<string>("trn_Y_Yf", "-");
    params.set<string>("tst_Y_Yf", "-");
    params.set<string>("Y_Yf", "-");
    params.set<string>("seen_labels", "-");

    params.set<string>("Xf", "-");
    params.set<string>("Yf", "-");
    params.set<string>("sparsity_pattern_file", "-");
    params.set<string>("bs_kind", "-"); // bs=bilinear sparsity
    params.set<int>("bs_count", 10);
    params.set<float>("bs_threshold", -1); // only one of bs_count/bs_threshold is +ve at a time
    params.set<float>("bs_alpha", 0.5); // means different things for fscore based or gradient based bs_kind
    params.set<float>("bs_reg", 0); // only used when bs_kind=gradient
    params.set<float>("bs_direct_wt", 0);
    params.set<bool>("bs_rem_dup", true); // avoids bilinear weights with identical +ve relevance pattern
    params.set<string>("sparsity_pattern_file", "-"); 

    params.set<bool>("normalize_features", true); // unit normalize X_Xf, Y_Yf
    params.set<bool>("binary_relevance", true); // If true, use non-binary values in X_Y. If false, set all +ves in X_Y to 1
    params.set<float>("propensity_A", 0.55);
    params.set<float>("propensity_B", 1.5);

    params.set<bool>("ova", true);
    params.set<bool>("bilinear", true);
    params.set<bool>("knn", true);
    params.set<bool>("shortlist", true);
    params.set<float>("hp_step_size", 0.05);
    params.set<string>("metric", "gp");
    params.set<int>("hpK", 5);
    params.set<bool>("fresh", true);

    params.set<int>("F", 10);
    params.set<int>("shortyK", 1000);
    params.set<float>("shortlist_head_weight", 0.0);
    params.set<float>("shortlist_tail_weight", 0.0);

    params.set<int>("bilinear_shortyK", 100);
    params.set<bool>("bilinear_add_bias", true);
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
    // initialize
    LOG("loading input...");

    SMatF* trn_X_Xf                 = new SMatF(params.get<string>("trn_X_Xf"));
    SMatF* Y_Yf                     = new SMatF(params.get<string>("Y_Yf"));
    SMatF* trn_X_Y                  = new SMatF(params.get<string>("trn_X_Y"));
    SMatF* val_X_Xf                 = new SMatF(params.get<string>("val_X_Xf"));
    SMatF* val_X_Y                  = new SMatF(params.get<string>("val_X_Y"));
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

    if(params.get<string>("trn_X_Xf").substr(params.get<string>("trn_X_Xf").size()-4, 4).compare(".bin") != 0)
        trn_X_Xf->dump(params.get<string>("trn_X_Xf") + ".bin");
    if(params.get<string>("trn_X_Y").substr(params.get<string>("trn_X_Y").size()-4, 4).compare(".bin") != 0)
        trn_X_Y->dump(params.get<string>("trn_X_Y") + ".bin");
    if(params.get<string>("Y_Yf").substr(params.get<string>("Y_Yf").size()-4, 4).compare(".bin") != 0)
        Y_Yf->dump(params.get<string>("Y_Yf") + ".bin");

    double time = 0;
    Timer timer; timer.tic();
    remove_test_labels(Y_Yf, trn_X_Y);
    prepare(trn_X_Xf, trn_X_Y, Y_Yf, NULL, not binary_relevance);

    if( not binary_relevance )
    {
        ips_weight( trn_X_Y, params );
    }

    timer.tic();
    SMatF* Xf_Yf = NULL;
    SMatF* Yf_Xf = NULL;
    create_Xf_Yf_map(trn_X_Xf, Y_Yf, trn_X_Y, Xf, Yf, Xf_Yf, Yf_Xf, params);
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
    if(params.get<string>("sparsity_pattern_file").compare("-") == 0)
    {
        LOGN("loading sparsity pattern mat from model dir");
        sparsity_pattern = new SMatF(params.get<string>("model_dir") + OS_SEP + "sparsity_pattern." + "bin");
        Xf_Yf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Xf_Yf." + "bin");
        Yf_Xf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Yf_Xf." + "bin");
    }
    else
    {
        LOGN("loading sparsity pattern mat from file : " << params.get<string>("sparsity_pattern_file"));
        sparsity_pattern = new SMatF(params.get<string>("sparsity_pattern_file"));
        Xf_Yf            = new SMatF(sparsity_pattern);
        Yf_Xf            = new SMatF(Xf_Yf->nc, Xf_Yf->nr);
    }
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

typedef pair<string, string> pairSS;

void fill_hp(vector<VecF>& hp, VecF current, float step_size, int tot_params)
{
    float prev_sum = 0;
    for(auto val : current) prev_sum += val;

    if(current.size() < tot_params-1)
    {
        for(float val = 0.0; val <= 1.00001-prev_sum; val += step_size)
        {
            current.push_back(val);
            fill_hp(hp, current, step_size, tot_params);
            current.pop_back();
        }
        return;
    }

    current.push_back(1.0 - prev_sum);
    hp.push_back(current);
}

void restrict_mat(SMatF*& mat, VecI& active_rows, VecI& active_cols)
{
    SMatF* mat_t = mat->transpose();
    SMatF* temp = new SMatF(mat_t, false, true, active_rows, true);
    temp->in_place_transpose();

    if(active_cols.size() == 0)
    {
        for(int x = 0; x < temp->nc; ++x)
            if(temp->size[x] > 0)
                active_cols.push_back(x);
    }

    delete mat;
    mat = new SMatF(temp, false, true, active_cols, true);
}

void run_hyperparameter_tuning(Parameters& params)
{
    LOGN("loading input...");

    string metric               = params.get<string>("metric");
   
    SMatF* val_X_Xf             = new SMatF(params.get<string>("val_X_Xf"));
    SMatF* val_X_Y              = new SMatF(params.get<string>("val_X_Y"));
    SMatF* trn_X_Y              = new SMatF(params.get<string>("trn_X_Y"));
    SMatF* Y_Yf                 = new SMatF(params.get<string>("Y_Yf"));
    SMatF* direct_Xf_Yf         = new SMatF(params.get<string>("model_dir") + OS_SEP + "direct_Xf_Yf.bin");
    
    VecF inv_prop;
    if(metric.compare("gpsp") == 0)
    {
        float val;
        ifstream fin(params.get<string>("inv_prop"), ios::in);
        while(fin >> val)
            inv_prop.push_back(val);
    }

    VecI seen_labels, unseen_labels; int y; VecI seen_map(val_X_Y->nr, 0);
    ifstream fin(params.get<string>("model_dir") + OS_SEP + "seen_labels.txt", ios::in);
    while(fin >> y)
    {
        seen_labels.push_back(y);
        seen_map[y] = 1;
    }
    for(int i = 0; i < val_X_Y->nr; ++i)
        if(seen_map[i] == 0) unseen_labels.push_back(i);

    SMatF* sparsity_pattern     = NULL;
    SMatF* Xf_Yf                = NULL;
    SMatF* Yf_Xf                = NULL;
    if(params.get<string>("sparsity_pattern_file").compare("-") == 0)
    {
        LOGN("loading sparsity pattern mat from model dir");
        sparsity_pattern = new SMatF(params.get<string>("model_dir") + OS_SEP + "sparsity_pattern." + "bin");
        Xf_Yf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Xf_Yf." + "bin");
        Yf_Xf            = new SMatF(params.get<string>("model_dir") + OS_SEP + "Yf_Xf." + "bin");
    }
    else
    {
        LOGN("loading sparsity pattern mat from file : " << params.get<string>("sparsity_pattern_file"));
        sparsity_pattern = new SMatF(params.get<string>("sparsity_pattern_file"));
        Xf_Yf            = new SMatF(sparsity_pattern);
        Yf_Xf            = new SMatF(Xf_Yf->nc, Xf_Yf->nr);
    }

    SMatF* filter_mat = new SMatF(val_X_Y->nr, val_X_Y->nc);
    if(ifstream(params.get<string>("val_filter_mat")).good())
    {
        LOGN("found filter mat : " << params.get<string>("val_filter_mat"));
        filter_mat = new SMatF(params.get<string>("val_filter_mat"));
    }
    else
    {
        LOGN("filter mat file : " << params.get<string>("val_filter_mat") << " doesn't exist");
    }

    params.set<string>("res_dir", params.get<string>("res_dir") + OS_SEP + "validation");
    int sys_ret = system((string("mkdir -p ") + params.get<string>("res_dir")).c_str());

    if(params.get<bool>("fresh"))
    {
        LOGN("loading model...");
        VecF bilinear_clf;
        ifstream fin(params.get<string>("model_dir") + OS_SEP + "bilinear_clf.bin", ios::in | ios::binary);
        read_vec_bin(bilinear_clf, fin);

        LOGN("predicting on validation data...");
        prepare(val_X_Xf, NULL, Y_Yf, NULL);
        predict(bilinear_clf, val_X_Y, trn_X_Y, val_X_Xf, Y_Yf, sparsity_pattern, seen_labels, Xf_Yf, Yf_Xf, direct_Xf_Yf, params);
    }

    int K = params.get<int>("hpK");
    LOGN("\nloading score mats...");
    
    vector<pair<SMatF*, string>> all_score_mats;
    vector<pairSS> names = {pairSS("bilinear", "bilinear_score_mat.bin"), pairSS("knn", "knn_score_mat.bin"), pairSS("shortlist", "shortlist.bin")};
    for(auto it : names)
    {
        if(params.get<bool>(it.first))
        {
            LOGN("trying to load " << it.second << "...");
            SMatF* temp = new SMatF(params.get<string>("res_dir") + OS_SEP + it.second);
            temp->filter(filter_mat);
            temp->retain_topk(100);
            // temp->unit_normalize_columns("max");
            // temp->retain_topk(K);
            LOGN("recall of " << it.first << " : " << temp->get_recall(val_X_Y) << endl);
            all_score_mats.push_back(pair<SMatF*, string>(temp, it.first));
        }
        else
        {
            LOGN("skipping " << it.second << "...");
        }
    }

    if(metric.compare("sp") == 0)
    {
        VecI activex;
        restrict_mat(val_X_Y, unseen_labels, activex);
        for(auto& score_mat : all_score_mats)
            restrict_mat(score_mat.first, unseen_labels, activex);
    }

    LOGN("\nmetric is " << metric);
    LOGN("num active labels for evaluation : " << val_X_Y->nr);
    LOGN("num active points for evaluation : " << val_X_Y->nc);

    vector<VecF> hp;
    fill_hp(hp, VecF(), params.get<float>("hp_step_size"), all_score_mats.size());
    VecF numerator(hp.size(), 0);
    vector<double> precs(hp.size(), 0);
    float denom = (K*val_X_Y->nc);

    if(metric.compare("gpsp") == 0)
    {
        denom = 0;
        for(int x = 0; x < val_X_Y->nc; ++x)
        {
            VecF vals;
            for(int i = 0; i < val_X_Y->size[x]; ++i)
            {
                val_X_Y->data[x][i].second = inv_prop[val_X_Y->data[x][i].first];
                vals.push_back(val_X_Y->data[x][i].second);
            }
            retain_topk(vals, K);
            denom += accumulate(vals.begin(), vals.end(), 0.0);
        }
    }

    ofstream fout(params.get<string>("res_dir") + OS_SEP + "tuning_results.txt");
    for(auto it : all_score_mats)
        fout << it.second << "\t";
    fout << metric << "P@" << K << endl;

    TQDM tqdm(hp.size(), 1);
    LOGN("\nsearching best hyperparameters for " << metric << " P@" << K << "...");
    #pragma omp parallel num_threads(params.get<int>("num_thread"))
    {
        DenseSVec checky(val_X_Y->nr, 0);
        DenseSVec combine(val_X_Y->nr, 0);
        #pragma omp for
        for(int i = 0; i < hp.size(); ++i)
        {
            for(int x = 0; x < val_X_Y->nc; ++x)
            {
                for(int j = 0; j < val_X_Y->size[x]; ++j)
                {
                    if(metric.compare("gpsp") == 0)
                        checky.add(val_X_Y->data[x][j].first, val_X_Y->data[x][j].second);
                    else
                        checky.add(val_X_Y->data[x][j].first, 1.0);
                }

                for(int a = 0; a < all_score_mats.size(); ++a)
                {
                    SMatF* temp = all_score_mats[a].first;
                    for(int j = 0; j < temp->size[x]; ++j)
                        combine.add(temp->data[x][j].first, hp[i][a]*temp->data[x][j].second);
                }

                VecIF scores = combine.vecif();
                retain_topk(scores, K);
                for(auto it : scores)
                    numerator[i] += checky.get(it.first);

                checky.reset();
                combine.reset();
            }
            tqdm.step();
        }
    }

    int best_ind = 0;
    double best_prec = 0;
    for(int i = 0; i < hp.size(); ++i)
    {   
        precs[i] = numerator[i]*100.0/denom;
        if(precs[i] > best_prec)
            best_prec = precs[i], best_ind = i;
        for(auto val : hp[i])
            fout << fixed << setprecision(2) << val << "\t";
        fout << precs[i] << endl;
    }

    LOGN(fixed << setprecision(2) << "best " << metric << " P@" << K << " : " << best_prec << " at ");
    for(int j = 0; j < all_score_mats.size(); ++j)
    {
        LOGN(all_score_mats[j].second << " : " << hp[best_ind][j]);
    }

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
    LOGN("running : " << type << " using " << params.get<int>("num_thread") << " thread(s)");

    if(type == "xhtp_approx")
        run_xhtp_approx(params);
    else if(type == "xhtp_fine_tune")
        run_xhtp_fine_tune(params);
    else if(type == "predict")
        run_predict(params);
    else if(type == "tune")
        run_hyperparameter_tuning(params);

    return 0;
}
