#pragma once

#include <iostream>

#include "math.h"

#include "config.h"
#include "utils.h"
#include "mat.h"

using namespace std;

VecF precision_k( SMatF* X_Y, SMatF* score_rank_mat, int K );
VecF nDCG_k( SMatF* X_Y, SMatF* score_rank_mat, int K );
VecF psprecision_k( SMatF* X_Y, SMatF* score_rank_mat, int K );
VecF psnDCG_k( SMatF* X_Y, SMatF* score_rank_mat, int K );
void print_all_metrics( SMatF* X_Y, SMatF* score_X_Y, int K );
void p_print_all_metrics( SMatF* X_Y, SMatF* score_X_Y, int K ); // duplicating print_all_metrics here for ease of function naming within cython