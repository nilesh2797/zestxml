dataset=$1
type=$2
extra_args="${@:3}"
DATA_DIR=/home/t-nilgup/Datasets/GZXML-Datasets/${dataset}
RES_DIR=Results/${dataset}
MODEL_DIR=Results/${dataset}/model
logfile=${RES_DIR}/log.txt

mkdir -p ${RES_DIR}
mkdir -p ${MODEL_DIR}

# set -x
/usr/bin/time -f "\nReal time : %E\nMax RAM : %M KB\nPercent CPU : %P" \
./run 	-trn_X_Xf ${DATA_DIR}/trn_X_Xf.txt \
		-tst_X_Xf ${DATA_DIR}/tst_X_Xf.txt \
		-val_X_Xf ${DATA_DIR}/val_X_Xf.txt \
		-Y_Yf ${DATA_DIR}/Y_Yf.txt \
		-inv_prop ${DATA_DIR}/inv_prop.txt \
		-trn_X_Y ${DATA_DIR}/trn_X_Y.txt \
		-tst_X_Y ${DATA_DIR}/tst_X_Y.txt \
		-val_X_Y ${DATA_DIR}/val_X_Y.txt \
		-val_filter_mat ${DATA_DIR}/pos_trn_val.txt \
		-tst_filter_mat ${DATA_DIR}/pos_trn_tst.txt \
		-Xf ${DATA_DIR}/Xf.txt \
		-Yf ${DATA_DIR}/Yf.txt \
		-Xf_Yf_map_kind JACCARD_DIRECT \
		-res_dir ${RES_DIR} \
		-model_dir ${MODEL_DIR} \
		-num_thread 12 \
		-type $type \
		-F 10 \
		-shortyK 1000 \
		-bs_threshold 0 \
		-bs_alpha 0.2 \
		-bs_direct_wt 0.2 \
		-binary_relevance 1 \
		-propensity_A 0.55 \
		-propensity_B 1.5 \
		-normalize_features 1 \
		-promote none \
		-shortlist_head_weight 0.5 \
		-shortlist_tail_weight 0.5 \
		-dump_linear_form_mat 0 \
		-bilinear_bsz 50000000 \
		-bilinear_shortyK 100 \
		-bilinear_add_bias 0 \
		-bilinear_normalize 0 \
		-bilinear_classifier_pos_wt 1 \
		-bilinear_classifier_cost 1 \
		-fresh 1 \
		-bilinear 1 \
		-knn 1 \
		-shortlist 1 \
		${extra_args} \
		| tee -a ${logfile}

 # -trn_X_Xf Datasets/IRM-EURLex/trn_X_Xf.txt -tst_X_Xf Datasets/IRM-EURLex/tst_X_Xf.txt -Y_Yf Datasets/IRM-EURLex/Y_Xf.txt -trn_X_Y Datasets/IRM-EURLex/trn_X_Y.txt -tst_X_Y Datasets/IRM-EURLex/tst_X_Y.txt -Xf Datasets/IRM-EURLex/Xf.txt -Yf Datasets/IRM-EURLex/Xf.txt -Xf_Yf_map_kind JACCARD_DIRECT -sparsity_pattern_file Results/IRM-EURLex/sparsity_pattern_file.txt -X_Y_feat_file Results/IRM-EURLex/X_Y_feat_file.txt.bin -param_file Results/IRM-EURLex/model_params.txt -jaccard_sparsity 10 -jaccard_threshold 0 -res_dir Results/IRM-EURLex -num_thread 1 -type run_predict_tree
