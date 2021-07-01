#!/usr/bin/env python

from utils import *
from metrics import *
import argparse

def evaluate( score_file, trn_lbl_file, tst_lbl_file, A, B ):
	score_mat = read_text_mat( score_file )
	trn_X_Y = read_text_mat( trn_lbl_file )
	trn_X_Y = spones( trn_X_Y )
	tst_X_Y = read_text_mat( tst_lbl_file )
	tst_X_Y = spones( tst_X_Y )
	wts = inv_propensity( trn_X_Y, A, B )
	get_all_metrics( score_mat, tst_X_Y, wts, k=5 )

def evaluate_dataset( score_file, dataset ):
	_,tst_X_Y,wts = load_labels_dataset( dataset )
	score_mat = read_text_mat( score_file )
	get_all_metrics( score_mat, tst_X_Y, wts, k=5 )

def parse_args( arglist ):
	parser = argparse.ArgumentParser( add_help=True )

	# dataset arguments	
	parser.add_argument( '--score_file', help="input score file name. Expected in sparse matrix text format", required=True, dest='score_file' )
	parser.add_argument( '--dataset', help="dataset name", dest='dataset' )
	parser.add_argument( '--trn_X_Y', help="training label file name. Expected in sparse matrix text format", dest='trn_X_Y_file' )
	parser.add_argument( '--tst_X_Y', help="test label file name. Expected in sparse matrix text format", dest='tst_X_Y_file' )
	parser.add_argument( '-A', help="Parameter A of propensity scoring", dest='A', type=float )
	parser.add_argument( '-B', help="Parameter B of propensity scoring", dest='B', type=float )

	args = vars( parser.parse_args( arglist ) )
	return args

if __name__=="__main__":
	args = parse_args( sys.argv[1:] )
	if "dataset" in args and args["dataset"] is not None:
		evaluate_dataset( args["score_file"], args["dataset"] )
	else:
		evaluate( args["score_file"], args["trn_X_Y_file"], args["tst_X_Y_file"], args["A"], args["B"] )