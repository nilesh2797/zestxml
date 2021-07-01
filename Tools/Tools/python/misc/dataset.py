from utils import *

class XMLDataset:
    
    def __init__( self, folder, split_format="single_file", split_no=0, load_ids=False, load_titles=False, load_miscs=False, point_miscs=None, label_miscs=None ):
		if split_format=="single_file":
   			split = read_split_file( os.path.join( folder, "split.%d.txt" %(split_no) ) )
	
 			X_Xf = read_text_mat( os.path.join( folder, "X_Xf.txt" ) )
			self.trn_X_Xf, self.tst_X_Xf = split_mat( X_Xf, split )
   
   			X_Y = read_text_mat( os.path.join( folder, "X_Y.txt" ) )
         	self.trn_X_Y, self.tst_X_Y = split_mat( X_Y, split )
            
		elif split_format=="split_files":
			X_Xf = read_text_mat( os.path.join( folder, "X_Xf.txt" ) )
		else:
			sys.exit( "ERROR: Invalid split_format: %s" %( split_format ) )