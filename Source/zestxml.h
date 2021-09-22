#pragma once

#include "helper.h"

using namespace std;

void train(SMatF* trn_X_Y, VecI seen_labels, SMatF* trn_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, SMatF* Xf_Yf, SMatF* Yf_Xf, Parameters& params);
void predict(VecF& bilinear_clf, SMatF* tst_X_Y, SMatF* trn_X_Y, SMatF* tst_X_Xf, SMatF* Y_Yf, SMatF* sparsity_pattern, VecI& seen_labels, SMatF* Xf_Yf, SMatF* Yf_Xf, SMatF* direct_Xf_Yf, Parameters& params);