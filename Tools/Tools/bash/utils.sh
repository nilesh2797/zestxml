#!/usr/bin/env bash

parse_arguments_from_console ()
{
# Simple parser for command line arguments
# Example arguments: dict --A a --B b
# Here 'dict' is the name of the global associative dictionary to store arguments into
# Stores the key-value pairs A-->a and B-->b into dict

    declare -n local_dict="$1"

    for i in $(seq 2 2 $#); do
        opt=${!i}
        pat="^--(.*)$"
        if [[ $opt =~ $pat ]]; then
            k=${BASH_REMATCH[1]}
            nexti=$((i+1))
            v=${!nexti}
            if [[ $v == "" ]]; then
                echo "ERROR: Empty parameter value for key '$k' in function 'parse_arguments_from_console'"
                exit 1
            fi
            local_dict[$k]=$v
        else
            echo "ERROR: Option name does not start with '--' in function 'parse_arguments_from_console'"
            exit 1
        fi
    done
}

parse_arguments_from_file ()
{
# Simple parser for arguments inputted from file
# Example arguments: dict fil sep --A a --B b
# Here 'dict' is the name of the global associative dictionary to store arguments into
# 'fil' is the input arguments file
# 'sep' is the separator between keys and values in 'fil'
# Stores the key-value pairs A-->a and B-->b into dict

    declare -n local_dict="$1"
    fil=$2
    sep=$3
    pat="^(.*)$sep(.*)$"
    while read line; do 
        if [[ $line =~ $pat ]]; then
            local_dict[${BASH_REMATCH[1]}]=${BASH_REMATCH[2]}
        else
            echo "ERROR: Pattern mismatch for a line '$line' in '$fil' in function 'parse_arguments_from_file'"
        fi
    done < $fil;    

    if [[ $line != "" ]]; then
        if [[ $line =~ $pat ]]; then
            local_dict[${BASH_REMATCH[1]}]=${BASH_REMATCH[2]}
        else
            echo "ERROR: Pattern mismatch for a line '$line' in '$fil' in function 'parse_arguments_from_file'"
        fi
    fi
}


parse_arguments () 
{
    # assumes that option_names and option_dict global variables are already declared and available from main script
    for opt in ${option_names[@]}; do
        for i in  $(seq 1 $#); do
            if [ "--$opt" == ${!i} ]; then
                nexti=$((i+1))
                option_dict[$opt]=${!nexti}
            fi
        done
    done

    for key in ${!option_dict[@]}; do
        val=${option_dict[$key]}
        echo "$key:$val"
    done
}

get_arg_string ()
{
    arg_string=""
    for opt in $@; do
        if [[ -n ${option_dict[$opt]} ]]; then
            val=${option_dict[$opt]}
            arg_string="$arg_string --$opt $val"
        fi    
    done
    echo $arg_string
}