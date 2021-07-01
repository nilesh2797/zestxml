#!/bin/bash
# Parse command-line options
# Option strings
SHORT=vf:
LONG=verbose,file:
# read the options
OPTS=$(getopt --long $LONG --name "$0" -- "$@")
if [ $? != 0 ] ; then echo "Failed to parse options...exiting." >&2 ; exit 1 ; fi
eval set -- "$OPTS"
# set initial values
VERBOSE=""
# extract options and their arguments into variables.

echo $0
echo $1
echo $2

while true ; do
  case "$1" in
    -v | --verbose )
      VERBOSE="$2"
      shift 2
      ;;
    -f | --file )
      FILE="$2"
      shift 2
      ;;
    -- )
      shift
      break
      ;;
    *)
      echo "Internal error!"
      echo $1
      exit 1
      ;;
  esac
done
# Print the variables
echo "VERBOSE = $VERBOSE"
echo "FILE = $FILE"