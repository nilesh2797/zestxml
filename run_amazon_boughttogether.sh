extra_args="${@:2}"
dataset=IRM-Amazon-BoughtTogether
DATA_DIR=/mnt/d/nilesh/irm/Datasets/$dataset

./run.sh $dataset $1  -bs_count 40 \
					  -bs_alpha 0.01 \
					  -bs_direct_wt 0.2 \
					  -bs_kind JACCARD \
					  -shortyK 100 \
					  -F 10 \
					  -bilinear_shortyK 100 \
					  -fresh 1 -ova 0 -bilinear 1 -knn 1 -shortlist 1 \
					  -bilinear_classifier_cost 2 \
					  -bilinear_classifier_pos_wt 4 \
					  -num_thread 48 \
					  -val_X_Xf ${DATA_DIR}/rand100k_tst_X_Xf.txt \
					  -val_X_Y ${DATA_DIR}/rand100k_tst_X_Y.txt \
					  -val_filter_mat ${DATA_DIR}/rand100k_pos_trn_tst.txt \
					  ${extra_args}