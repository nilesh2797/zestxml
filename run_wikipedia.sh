extra_args="${@:2}"
./run.sh IRM-Wikipedia $1 -bs_count 20 -bs_direct_wt 0.1 -bs_alpha 0.01 -num_thread 20 ${extra_args}