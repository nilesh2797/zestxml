#!/usr/bin/env bash
source utils.sh

option_names=( "do_lowercase" "do_stemming" "remove_stopwords" "max_token_ngram_len" "min_token_label_freq" "max_token_label_freq" "retain_full_label_token" )
declare -A option_dict
parse_arguments $@

trn_X_Y_file=$1
raw_Y_file=$2
proc_Y_file=$3
Y_T_file=$4
T_file=$5
Tf_file=$6
TfParam_file=$7

clean_text_args=$(get_arg_string do_lowercase remove_stopwords do_stemming)
gen_irmatch_tokens_args=$(get_arg_string max_token_ngram_len min_token_label_freq max_token_label_freq retain_full_label_token)
cat $raw_Y_file | CleanText.pm $clean_text_args | GenIRMatchTokens.pm $proc_Y_file $gen_irmatch_tokens_args --print_progress 1 | Text2SpMat.pm $T_file > $Y_T_file

<<COM
trn_X_per_Y_file=$(mktemp)
Y_per_T_file=$(mktemp)
trn_X_per_T_file=$(mktemp)
T_precision_file=$(mktemp)

echo "trn_X_per_Y_file: $trn_X_per_Y_file"
echo "Y_per_T_file: $Y_per_T_file"
echo "trn_X_per_T_file: $trn_X_per_T_file"
echo "T_precision_file: $T_precision_file"

get_irmatch_token_features.py $trn_X_Y_file $Y_T_file $trn_X_per_Y_file $Y_per_T_file $trn_X_per_T_file $T_precision_file
GenIRMatchTokenFeatures.pm $trn_X_per_Y_file $Y_per_T_file $trn_X_per_T_file $T_precision_file $proc_Y_file $Y_T_file $T_file $Tf_file $TfParam_file

#rm $trn_X_per_Y_file
#rm $Y_per_T_file
#rm $trn_X_per_T_file
#rm $T_precision_file
COM