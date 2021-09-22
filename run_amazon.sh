extra_args="${@:2}"
./run.sh GZ-Amazon-1M $1  -bs_count 40 -bs_alpha 0.01 -bs_direct_wt 0.2 ${extra_args}
