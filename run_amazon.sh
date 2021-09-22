extra_args="${@:2}"
dataset=GZ-Amazon-1M
./run.sh $dataset $1  -bs_count 40 \
					  -bs_alpha 0.01 \
					  -bs_direct_wt 0.2 \
					  -score_alpha 0.9 \
					  -num_thread 1 \
					  ${extra_args}