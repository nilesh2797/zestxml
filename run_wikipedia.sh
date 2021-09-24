extra_args="${@:2}"
./run.sh GZ-Wikipedia-1M $1 -bilinear_classifier_cost 2 -bs_count 40 -bs_direct_wt 0.1 -bs_alpha 0.01 -score_alpha 0.8 ${extra_args}
