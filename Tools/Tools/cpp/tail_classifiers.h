#pragma once

#include <iostream>
#include <string>
#include <cmath>

#include "config.h"
#include "utils.h"
#include "mat.h"
#include "timer.h"

SMatF* tail_classifier_train( SMatF* trn_X_Xf, SMatF* trn_X_Y, float& train_time );
SMatF* tail_classifier_predict( SMatF* tst_X_Xf, SMatF* score_mat, SMatF* model_mat, float alpha, float threshold, float& predict_time, float& model_size );
void tail_classifier_predict( SMatF* tst_X_Xf, int x, VecIF &node_score_mat, SMatF* model_mat, float alpha, float& predict_time, float& model_size );