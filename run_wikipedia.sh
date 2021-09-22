extra_args="${@:2}"
./run.sh GZ-Wikipedia-1M $1 -bs_count 20 -bs_direct_wt 0.1 -bs_alpha 0.01 ${extra_args}