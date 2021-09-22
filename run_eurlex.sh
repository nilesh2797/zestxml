extra_args="${@:2}"
./run.sh GZ-Eurlex-4.3K $1 -bilinear_classifier_cost 5 -bs_count 120 -bs_direct_wt 0.8 -bs_alpha 0.02 -shortyK 150 ${extra_args}
