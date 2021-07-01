extra_args="${@:2}"
./run.sh ZS-Eurlex-4.3K $1 -bilinear_classifier_pos_wt 2 -bs_count 80 -bs_direct_wt 0.2 -shortyK 500 -binary_relevance 1 -bilinear_shortyK 100 -fresh 1 -bilinear 1 -knn 1 -shortlist 1 -num_thread 12 ${extra_args}
