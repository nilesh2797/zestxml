extra_args="${@:2}"
dataset=ZS-Amazon-1M
DATA_DIR=GZXML-Datasets/$dataset

./run.sh $dataset $1  -bs_count 40 \
					  -bs_alpha 0.01 \
					  -bs_direct_wt 0.2 \
					  -shortyK 100 \
					  -F 10 \
					  -bilinear_shortyK 100 \
					  -fresh 1 -bilinear 1 -knn 1 -shortlist 1 \
					  -bilinear_classifier_cost 2 \
					  -bilinear_classifier_pos_wt 4 \
					  -num_thread 1 \
					  ${extra_args}