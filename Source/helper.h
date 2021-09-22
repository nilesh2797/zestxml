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
#include "timer.h"
#include "linear_classifiers.h"
#include "parameters.h"

using namespace std;

float get_classifier_cost( _Classifier_Kind classifier_kind, int num_all_instance, int num_node_instance, float raw_classifier_cost );
float get_new_score(float prod, float prev_score, _Classifier_Kind classifier_kind, bool logp = true);
VecIF apply_classifier(VecF& dense_clf, pairIF* shortlist, int shortlist_size, SMatF* X_Xf, _Classifier_Kind classifier_kind, bool logp = true, bool pp = false, bool raw = false);
VecIF apply_classifier(pairIF* clf, int clf_size, pairIF* shortlist, int shortlist_size, SMatF* X_Xf, _Classifier_Kind classifier_kind, VecF& temp, bool logp = true, bool raw = false);
void ips_weight( SMatF* trn_X_Y, Parameters& params );

SMatF* expand_linear_form_mat(const VecF& vec, SMatF* sparsity_pattern, SMatF* Xf_Yf, SMatF* Yf_Xf);
SMatF* bilinear_to_linear_form( SMatF* X_Xf, SMatF* Y_Yf, SMatF* X_Y, SMatF* Xf_Yf, SMatF* Yf_Xf, bool add_biases=false, bool pp = true, int num_thread = 1);

SMatF* create_Xf_Yf_map_direct( VecS& Xf, VecS& Yf, float wt = 1.0 );
void create_Xf_Yf_map( SMatF* _trn_X_Xf, SMatF* _Y_Yf, SMatF* _trn_X_Y, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params);
void create_Xf_Yf( SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* trn_X_Y, VecS& Xf, VecS& Yf, SMatF*& Xf_Yf, SMatF*& Yf_Xf, Parameters& params );

