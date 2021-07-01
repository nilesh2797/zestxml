from utils import *
import copy

class XMLDataset:
	
	def __init__( self ):
		self.trn_X_Xf = None
		self.tst_X_Xf = None
		self.trn_X_Y = None
		self.tst_X_Y = None
		self.trn_X_miscs = None
		self.tst_X_miscs = None
		self.Y_miscs = None
		self.Xf = None

	def load_from_arguments( self, trn_X_Xf=None, tst_X_Xf=None, trn_X_Y=None, tst_X_Y=None, trn_X_miscs=None, tst_X_miscs=None, Y_miscs=None, Xf=None ):
		self.trn_X_Xf = trn_X_Xf
		self.tst_X_Xf = tst_X_Xf
		self.trn_X_Y = trn_X_Y
		self.tst_X_Y = tst_X_Y
		self.trn_X_miscs = trn_X_miscs
		self.tst_X_miscs = tst_X_miscs
		self.Y_miscs = Y_miscs
		self.Xf = Xf

	def check_dataset_dimension_consistency( self ):
		num_trn_X = self.trn_X_Xf.shape[1]
		num_tst_X = self.tst_X_Xf.shape[1]
		num_Xf = self.trn_X_Xf.shape[0]
		num_Y = self.trn_X_Y.shape[0]

		assert self.trn_X_Xf.shape[0]==num_Xf and self.trn_X_Xf.shape[1]==num_trn_X, "ERROR: dimension consistency check fails for trn_X_Xf"
		assert self.tst_X_Xf.shape[0]==num_Xf and self.tst_X_Xf.shape[1]==num_tst_X, "ERROR: dimension consistency check fails for tst_X_Xf"
		assert self.trn_X_Y.shape[0]==num_Y and self.trn_X_Y.shape[1]==num_trn_X, "ERROR: dimension consistency check fails for trn_X_Y"
		assert self.tst_X_Y.shape[0]==num_Y and self.tst_X_Y.shape[1]==num_tst_X, "ERROR: dimension consistency check fails for tst_X_Y"
		
		if self.Xf is not None:
			assert len(self.Xf)==num_Xf, "ERROR: dimension consistency check fails for Xf"

		for X_misc_type in self.trn_X_miscs.keys():
			assert len(self.trn_X_miscs[ X_misc_type ])==num_trn_X, "ERROR: dimension consistency check fails for trn_X.%s" %( X_misc_type )
			assert len(self.tst_X_miscs[ X_misc_type ])==num_tst_X, "ERROR: dimension consistency check fails for tst_X.%s" %( X_misc_type )

		for Y_misc_type in self.Y_miscs.keys():
			assert len(self.Y_miscs[ Y_misc_type ])==num_Y, "ERROR: dimension consistency check fails for Y.%s" %( Y_misc_type )

	def print_dataset_dimensions( self ):
		num_trn_X = self.trn_X_Xf.shape[1]
		num_tst_X = self.tst_X_Xf.shape[1]
		num_Xf = self.trn_X_Xf.shape[0]
		num_Y = self.trn_X_Y.shape[0]

		print( "Dataset statistics:" )
		print( "\tnum_trn_X: %d" %(num_trn_X ) )
		print( "\tnum_tst_X: %d" %(num_tst_X ) )
		print( "\tnum_Xf: %d" %(num_Xf ) )
		print( "\tnum_Y: %d" %(num_Y ) )

	def remove_empty_rows_cols( self ):

		print( "Begin remove_empty_rows_cols" )	
	
		valid_trn_X = myfind( get_freq( self.trn_X_Y, axis=0 ) )
		print( "num_trn_X: %d\tnum_valid_trn_X: %d" %( self.trn_X_Y.shape[1], len( valid_trn_X ) ) )
		self.trn_X_Xf = self.trn_X_Xf[ :, valid_trn_X ]
		self.trn_X_Y = self.trn_X_Y[ :, valid_trn_X ]
		for X_misc_type in self.trn_X_miscs.keys():
			self.trn_X_miscs[ X_misc_type ] = [ self.trn_X_miscs[X_misc_type][j] for j in valid_trn_X ]

		valid_tst_X = myfind( get_freq( self.tst_X_Y, axis=0 ) )
		print( "num_tst_X: %d\tnum_valid_tst_X: %d" %( self.tst_X_Y.shape[1], len( valid_tst_X ) ) )
		self.tst_X_Xf = self.tst_X_Xf[ :, valid_tst_X ]
		self.tst_X_Y = self.tst_X_Y[ :, valid_tst_X ]
		for X_misc_type in self.tst_X_miscs.keys():
			self.tst_X_miscs[ X_misc_type ] = [ self.tst_X_miscs[X_misc_type][j] for j in valid_tst_X ]

		valid_Y = myfind( get_freq( self.trn_X_Y, axis=1 ) + get_freq( self.tst_X_Y, axis=1 ) )
		print( "num_Y: %d\tnum_valid_Y: %d" %( self.trn_X_Y.shape[0], len( valid_Y ) ) )
		self.trn_X_Y = self.trn_X_Y[ valid_Y, : ]
		self.tst_X_Y = self.tst_X_Y[ valid_Y, : ]
		for Y_misc_type in self.Y_miscs.keys():
			self.Y_miscs[ Y_misc_type ] = [ self.Y_miscs[Y_misc_type][j] for j in valid_Y ]
	
		valid_Xf = myfind( get_freq( self.trn_X_Xf, axis=1 ) + get_freq( self.tst_X_Xf, axis=1 ) )
		print( "num_Xf: %d\tnum_valid_Xf: %d" %( self.trn_X_Xf.shape[0], len( valid_Xf ) ) )
		self.trn_X_Xf = self.trn_X_Xf[ valid_Xf, : ]
		self.tst_X_Xf = self.tst_X_Xf[ valid_Xf, : ]
		if self.Xf is not None:
			self.Xf = [ self.Xf[i] for i in valid_Xf ]
		
		print( "End remove_empty_rows_cols" )

	def finalize( self, args ):

		self.check_dataset_dimension_consistency()
		self.print_dataset_dimensions()

		if args.remove_tautologies:
			assert ("id" in self.trn_X_miscs) and ("id" in self.Y_miscs), "ERROR: 'id' not in trn_X_miscs or Y_miscs but 'remove_tautologies' is True"
			print( "before tautology_remover nnz(trn_X_Y): %d" %( self.trn_X_Y.count_nonzero() ) )
			tautology_remover( self.trn_X_Y, self.trn_X_miscs["id"], self.Y_miscs["id"] )
			print( "after tautology_remover nnz(trn_X_Y): %d" %( self.trn_X_Y.count_nonzero() ) )
			print( "before tautology_remover nnz(tst_X_Y): %d" %( self.tst_X_Y.count_nonzero() ) )
			tautology_remover( self.tst_X_Y, self.tst_X_miscs["id"], self.Y_miscs["id"] )
			print( "after tautology_remover nnz(tst_X_Y): %d" %( self.tst_X_Y.count_nonzero() ) )	
	
		if args.min_ft_freq:
			freq = get_freq( self.trn_X_Xf, axis=1 )
			valid_Xf = myfind( freq>=args.min_ft_freq )
			print( "num_Xf: %d\tnum_valid_Xf: %d" %( self.trn_X_Xf.shape[0], len( valid_Xf ) ) )
			self.trn_X_Xf = self.trn_X_Xf[ valid_Xf, : ]
			self.tst_X_Xf = self.tst_X_Xf[ valid_Xf, : ]
			if self.Xf is not None:
				self.Xf = [ self.Xf[i] for i in valid_Xf ]
		
		if args.min_lbl_freq:
			freq = get_freq( self.trn_X_Y, axis=1 )
			valid_Y = myfind( freq>=args.min_lbl_freq )
			pdb.set_trace()
			print( "num_Y: %d\tnum_valid_Y: %d" %( self.trn_X_Y.shape[0], len( valid_Y ) ) )
			self.trn_X_Y = self.trn_X_Y[ valid_Y, : ]
			self.tst_X_Y = self.tst_X_Y[ valid_Y, : ]
			for Y_misc_type in self.Y_miscs.keys():
				self.Y_miscs[ Y_misc_type ] = [ self.Y_miscs[Y_misc_type][j] for j in valid_Y ]
		
		if args.tfidf:
			self.trn_X_Xf = get_tfidf_features( self.trn_X_Xf, self.trn_X_Xf )
			self.tst_X_Xf = get_tfidf_features( self.trn_X_Xf, self.tst_X_Xf )
			
		if args.inv_prop:
			self.trn_X_Y = get_inv_prop_labels( self.trn_X_Y, self.trn_X_Y, dataset_type=args.inv_prop_type )
			self.tst_X_Y = get_inv_prop_labels( self.trn_X_Y, self.tst_X_Y, dataset_type=args.inv_prop_type )
	
		self.check_dataset_dimension_consistency()
		self.print_dataset_dimensions()
	
		self.remove_empty_rows_cols( )

		self.check_dataset_dimension_consistency()
		self.print_dataset_dimensions()
	

	def read( self, dataset_dir, X_misc_types_str="", Y_misc_types_str="", load_Xf=False ):
		self.trn_X_Xf = read_text_mat( os.path.join( dataset_dir, "trn_X_Xf.txt" ) )
		self.tst_X_Xf = read_text_mat( os.path.join( dataset_dir, "tst_X_Xf.txt" ) )
		self.trn_X_Y = read_text_mat( os.path.join( dataset_dir, "trn_X_Y.txt" ) )
		self.tst_X_Y = read_text_mat( os.path.join( dataset_dir, "tst_X_Y.txt" ) )

		self.trn_X_miscs = {}
		self.tst_X_miscs = {}
		X_misc_types = X_misc_types_str.split( "," )
		if X_misc_types[0]=="":
			X_misc_types = []
		for i in range( len( X_misc_types ) ):
			X_misc_type = X_misc_types[i]
			self.trn_X_miscs[ X_misc_type ] = read_desc_file( os.path.join( dataset_dir, "trn_X.%s.txt" %( X_misc_type ) ) )
			self.tst_X_miscs[ X_misc_type ] = read_desc_file( os.path.join( dataset_dir, "tst_X.%s.txt" %( X_misc_type ) ) )
	
		self.Y_miscs = {}
		Y_misc_types = Y_misc_types_str.split( "," )
		if Y_misc_types[0]=="":
			Y_misc_types = []
		for i in range( len( Y_misc_types ) ):
			Y_misc_type = Y_misc_types[i]
			self.Y_miscs[ Y_misc_type ] = read_desc_file( os.path.join( dataset_dir, "Y.%s.txt" %( Y_misc_type ) ) )

		if load_Xf:
			self.Xf = read_desc_file( os.path.join( dataset_dir, "Xf.txt" ) )
		else:
			self.Xf = None


	def write( self, dataset_dir ):

		if self.trn_X_Xf is not None:
			write_text_mat( self.trn_X_Xf, os.path.join( dataset_dir, "trn_X_Xf.txt" ) )
		if self.tst_X_Xf is not None:
			write_text_mat( self.tst_X_Xf, os.path.join( dataset_dir, "tst_X_Xf.txt" ) )
		if self.trn_X_Y is not None:
			write_text_mat( self.trn_X_Y, os.path.join( dataset_dir, "trn_X_Y.txt" ) )
		if self.tst_X_Y is not None:
			write_text_mat( self.tst_X_Y, os.path.join( dataset_dir, "tst_X_Y.txt" ) )

		for X_misc_type in self.trn_X_miscs.keys():
			write_desc_file( self.trn_X_miscs[ X_misc_type ], os.path.join( dataset_dir, "trn_X.%s.txt" %(X_misc_type) ) )
			write_desc_file( self.tst_X_miscs[ X_misc_type ], os.path.join( dataset_dir, "tst_X.%s.txt" %(X_misc_type) ) )

		for Y_misc_type in self.Y_miscs.keys():
			write_desc_file( self.Y_miscs[ Y_misc_type ], os.path.join( dataset_dir, "Y.%s.txt" %(Y_misc_type) ) )

		if self.Xf is not None:
			write_desc_file( self.Xf, os.path.join( dataset_dir, "Xf.txt" ) )

	def split_training_data( self, split_type, split_frac ):
		if split_type=="uniform":
			split = get_uniform_split( self.trn_X_Y, split_frac, seed=0 )
		elif split_type=="coverage":
			split = get_coverage_split( self.trn_X_Y, split_frac, seed=0 )

		v_trn_X_Xf, v_tst_X_Xf = split_mat( self.trn_X_Xf, split )
		v_trn_X_Y, v_tst_X_Y = split_mat( self.trn_X_Y, split )
		v_trn_X_miscs = {}
		v_tst_X_miscs = {}
		for X_misc_type in self.trn_X_miscs.keys():
			v_trn_X_miscs[ X_misc_type ], v_tst_X_miscs[ X_misc_type ] = split_vec( self.trn_X_miscs[ X_misc_type ], split )

		v_Xf = copy.deepcopy( self.Xf )
		v_Y_miscs = copy.deepcopy( self.Y_miscs )

		v_dataset = XMLDataset()
		v_dataset.load_from_arguments( trn_X_Xf=v_trn_X_Xf, tst_X_Xf=v_tst_X_Xf, trn_X_Y=v_trn_X_Y, tst_X_Y=v_tst_X_Y, trn_X_miscs=v_trn_X_miscs, tst_X_miscs=v_tst_X_miscs, Y_miscs=v_Y_miscs, Xf=v_Xf )

		return v_dataset
