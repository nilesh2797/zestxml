#!/usr/bin/env python3

from libimport import *
import argparse

# ----- Argument Parser -----

parser = argparse.ArgumentParser( add_help=True )

# Compulsary arguments
parser.add_argument( 'in_dir', help="Folder to read the input data from", type=str )
parser.add_argument( 'final_dir', help="Folder to write the finalized data to", type=str )

# Optional arguments
# Dataset loading arguments
parser.add_argument( '--X_misc_types', help="Comma separated point miscs to load", dest="X_misc_types", type=str, default="" )
parser.add_argument( '--Y_misc_types', help="Comma separated label miscs to load", dest="Y_misc_types", type=str, default="" )
parser.add_argument( '--load_Xf', help="Load point-feature tokens or not", dest="load_Xf", type=int, choices=[0,1], default=1 )

# Dataset splitting arguments
parser.add_argument( '--create_split', help="1=>Splits the points into train and test, 0=>Assumes split already done", dest="create_split", type=int, choices=[0,1], default=1 )
parser.add_argument( '--split_type', help="uniform=>Random point split, coverage=>label coverage maximizing point split", dest="split_type", type=str, choices=["uniform","coverage"], default="uniform" )
parser.add_argument( '--split_frac', help="Fraction of points to use for test", dest="split_frac", type=float, default=0.2 )
parser.add_argument( '--create_v_split', help="1=>Splits the training points further into train and test for validation purposes, 0=>Assumes split already done", dest="create_v_split", type=int, choices=[0,1], default=1 )
parser.add_argument( '--v_split_type', help="uniform=>Random point split, coverage=>label coverage maximizing point split", dest="v_split_type", type=str, choices=["uniform","coverage"], default="uniform" )
parser.add_argument( '--v_split_frac', help="Fraction of training points to use for validation test", dest="v_split_frac", type=float, default=0.1 )
parser.add_argument( '--v_final_dir', help="Folder to write the finalized validation data to", dest="v_final_dir", type=str, default="" )

# Filtering and weighting arguments
parser.add_argument( '--remove_tautologies', help="1=>Remove positive labels having the same id as a given point. 0=>No removal. Throws error if point and label id are not loaded in miscs.", dest="remove_tautologies", type=int, choices=[0,1], default=0 )
parser.add_argument( '--min_ft_freq', help="Features which occur in less than this no of training points are deleted", dest="min_ft_freq", type=int, default=2 )
parser.add_argument( '--min_lbl_freq', help="Labels which occur in less than this no of training points are deleted", dest="min_lbl_freq", type=int, default=2 )
parser.add_argument( '--tfidf', help="1=>Weight features by tf-idf over training data, 0=>No tf-idf weighting", dest="tfidf", type=int, choices=[0,1], default=0 )
parser.add_argument( '--inv_prop', help="1=>Labels are weighted by their inverse propensity scores, 0=>No inverse propensity weighting", dest="inv_prop", type=int, choices=[0,1], default=0 )
parser.add_argument( '--inv_prop_type', help="amazon/wikipedia/other", dest="inv_prop_type", type=str, choices=["amazon","wikipedia","other"], default="other" )


args = parser.parse_args()

if args.in_dir==args.final_dir:
    parse.error( "'final_dir' cannot be same as 'in_dir'")

if args.create_v_split==1:
	if args.v_final_dir=="":
		parse.error( "'--v_final_dir' needs to be set when '--create_v_split' is set")
	elif args.v_final_dir==args.final_dir or args.v_final_dir==args.in_dir:
		parse.error( "'v_final_dir' cannot be same as 'final_dir' or 'in_dir'")

create_recur_dir( args.final_dir )
if args.v_final_dir!="":
	create_recur_dir( args.v_final_dir )

# ----- load dataset -----

dataset_dir = args.in_dir

X_misc_types = args.X_misc_types.split( "," )
if X_misc_types[0]=="":
	X_misc_types = []
	
Y_misc_types = args.Y_misc_types.split( "," )
if Y_misc_types[0]=="":
	Y_misc_types = []
 
load_Xf = args.load_Xf

if args.create_split:
	X_Xf = read_text_mat( os.path.join( dataset_dir, "X_Xf.txt" ) )
	X_Y = read_text_mat( os.path.join( dataset_dir, "X_Y.txt" ) )

	X_miscs = {}
	for i in range( len( X_misc_types ) ):
		X_misc_type = X_misc_types[i]
		X_miscs[ X_misc_type ] = read_desc_file( os.path.join( dataset_dir, "X.%s.txt" %( X_misc_type ) ) )

	Y_miscs = {}
	for i in range( len( Y_misc_types ) ):
		Y_misc_type = Y_misc_types[i]
		Y_miscs[ Y_misc_type ] = read_desc_file( os.path.join( dataset_dir, "Y.%s.txt" %( Y_misc_type ) ) )

	if args.load_Xf:
		Xf = read_desc_file( os.path.join( dataset_dir, "Xf.txt" ) )
	else:
		Xf = None

	tmp_dataset = XMLDataset()
	tmp_dataset.load_from_arguments( trn_X_Xf=X_Xf, trn_X_Y=X_Y, trn_X_miscs=X_miscs, Y_miscs=Y_miscs, Xf=Xf )
	dataset = tmp_dataset.split_training_data( args.split_type, args.split_frac )

else:
	dataset = XMLDataset()
	dataset.read( dataset_dir=dataset_dir, X_misc_types_str=args.X_misc_types, Y_misc_types_str=args.Y_misc_types, load_Xf=args.load_Xf )

# ----- finalize dataset -----

dataset.finalize( args )

# ----- write finalized dataset -----

dataset.check_dataset_dimension_consistency()
dataset.write( args.final_dir )
	

# ----- create validation split and save validation dataset -----

if args.create_v_split:
	v_dataset = dataset.split_training_data( args.v_split_type, args.v_split_frac )
	
	v_dataset.remove_empty_rows_cols()
 
	v_dataset.check_dataset_dimension_consistency()
 
	v_dataset.write( args.v_final_dir )
