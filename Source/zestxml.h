#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <queue>
#include <tuple>
#include <omp.h>

#include "mat.h"
#include "utils.h"
#include "algos.h"
#include "timer.h"
#include "linear_classifiers.h"
#include "parameters.h"

using namespace std;

SMatF* bilinear_to_linear_form( SMatF* X_Xf, SMatF* Y_Yf, SMatF* X_Y, SMatF* Xf_Yf, SMatF* Yf_Xf, bool add_biases=false, bool pp = true, int num_thread = 1);
SMatF* create_Xf_Yf_map_direct( VecS& Xf, VecS& Yf, float wt = 1.0 );
void create_Xf_Yf_map_jaccard( SMatF* _trn_X_Xf, SMatF* _Y_Yf, SMatF* _trn_X_Y, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params);
void create_Xf_Yf_map( SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* trn_X_Y, VecS& Xf, VecS& Yf, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params );
void ips_weight( SMatF* trn_X_Y, Parameters& params );

void train(SMatF* trn_X_Y, VecI seen_labels, SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, SMatF* Xf_Yf, SMatF* Yf_Xf, Parameters& params);
void predict(VecF& bilinear_clf, SMatF* tst_X_Y, SMatF* trn_X_Y, SMatF* tst_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, VecI& seen_labels, SMatF* Xf_Yf, SMatF* Yf_Xf, SMatF* direct_Xf_Yf, Parameters& params);