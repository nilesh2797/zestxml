source ./utils.sh

declare -A dict

#parse_arguments_from_console dict --A a --B b

param_file=/mnt/f/Results/GeneXML/EURLex-4.3K-ZSL/Param.txt
sep="="
parse_arguments_from_file dict $param_file $sep

echo ${dict[A]}
echo ${dict[B]}