#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "config.h"
#include "mat.h"
#include "utils.h"
#include "timer.h"
#include "irmatch.h"
#include "data.h"

using namespace std;

void print_diff_visual_inspection( string vis_file, VecS& X, SMatF* score_mat1, SMatF* score_mat2, SMatF* X_Y, VecS& Y, bool weighted, int k, int diff_type );
void visualize_true_predicted_labels( SMatF* X_Y, SMatF* score_mat, VecS& X, VecS& Y, string out_file, int max_X, int max_num_scores );
void visualize_predicted_labels( SMatF* score_mat, VecS& X, VecS& Y, string out_file, int max_X, int max_num_scores );