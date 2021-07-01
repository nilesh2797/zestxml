#!/bin/bash

perl -C -MText::Unidecode -n -i -e'print unidecode( $_)' $1
