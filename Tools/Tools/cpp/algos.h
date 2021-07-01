#pragma once

#include <iostream>
#include <random>

#include "utils.h"
#include "mat.h"
#include "timer.h"
#include "linear_classifiers.h"
#include "tail_classifiers.h"

void balanced_kmeans( SMatF* mat, float acc, VecI& partition, mt19937& reng );
void balanced_weighted_kmeans( SMatF* mat, VecF& weights, float acc, float balance, VecI& partition, mt19937& reng );
void balanced_submodular_clustering( SMatF* X_Xf, SMatF* Y_X, float acc, VecI& partition, mt19937& reng );
void get_partition_X_count( SMatF* Y_X, VecI& partition, DMatI* partition_X_count );