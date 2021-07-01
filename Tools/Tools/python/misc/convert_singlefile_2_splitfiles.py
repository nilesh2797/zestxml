#!/usr/bin/env python3
from utils import *

parser = argparse.ArgumentParser( add_help=True )

parser.add_argument( 'in_dir', help="Folder to read the input data from" )
parser.add_argument( 'out_dir', help="Folder to write the finalized data to" )
parser.add_argument( '--point_miscs', help="Comma separated point miscs to load", dest="point_miscs", type=str, default="" )
args = vars( parser.parse_args() )
in_dir = args[ "in_dir" ]
out_dir = args[ "out_dir" ]
point_miscs = args[ "point_miscs" ]

split = read_split_file( os.path.join( in_dir, "split.0.txt" ) )
	
X_Xf = read_text_mat( os.path.join( in_dir, "X_Xf.txt" ) )
trn_X_Xf, tst_X_Xf = split_mat( X_Xf, split )
write_text_mat( trn_X_Xf, os.path.join( out_dir, "trn_X_Xf.txt" ), prec=3 )
write_text_mat( tst_X_Xf, os.path.join( out_dir, "tst_X_Xf.txt" ), prec=3 )
	
X_Y = read_text_mat( os.path.join( in_dir, "X_Y.txt" ) )
trn_X_Y, tst_X_Y = split_mat( X_Y, split )
write_text_mat( trn_X_Y, os.path.join( out_dir, "trn_X_Y.txt" ), prec=3 )
write_text_mat( tst_X_Y, os.path.join( out_dir, "tst_X_Y.txt" ), prec=3 )
 
X_misc_types = point_miscs.split( "," )
for i in range( len( X_misc_types ) ):
	X_misc_type = X_misc_types[i]
	if (i==0) and X_misc_type=="":
		break
	X_misc = read_desc_file( os.path.join( in_dir, "X.%s.txt" %( X_misc_type ) ) )
	trn_X_misc, tst_X_misc = split_vec( X_misc, split )
	write_desc_file( trn_X_misc, os.path.join( out_dir, "trn_X.%s.txt" %(X_misc_type) ))
	write_desc_file( tst_X_misc, os.path.join( out_dir, "tst_X.%s.txt" %(X_misc_type) ))
