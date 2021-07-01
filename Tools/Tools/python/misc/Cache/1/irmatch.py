from utils import *

def print_diff_visual_inspection( vis_file, tst_X_file, score_file1, score_file2, tst_X_Y_file, Y_file, k ):
    score_mat1 = read_text_mat( score_file1 )
    score_mat2 = read_text_mat( score_file2 )
    
    fout = open( vis_file, "w" )
    tst_X = read_desc_file( tst_X_file )
    score_mat = read_text_mat( score_file )
    all_Y = read_desc_file( all_Y_file )
    tst_X_Y = read_text_mat( tst_X_Y_file )
    Y = read_desc_file( Y_file )
    num_X = score_mat.shape[1]
    for i in range( num_X ):
        print( "%d\t%s" %( i, tst_X[i] ), file=fout )
        
        print( "true labels:", file=fout )
        labels = myfind( tst_X_Y[:, i] )
        for j in range( len( labels  ) ):
            print( "\t%s" %( Y[labels[j]] ), file=fout )

        print( "predicted labels/tokens:", file=fout )        
        scores = myarr( score_mat[:,i] )
        inds = np.argsort( -scores )
        for j in range( min(print_num_scores,len( inds ) ) ):
            print( "\t%s:%f" %( all_Y[inds[j]], scores[inds[j]] ), file=fout )
        print( "", file=fout )
    fout.close()


def print_visual_inspection( vis_file, tst_X_file, score_file, all_Y_file, tst_X_Y_file, Y_file, print_num_scores ):
    fout = open( vis_file, "w" )
    tst_X = read_desc_file( tst_X_file )
    score_mat = read_text_mat( score_file )
    all_Y = read_desc_file( all_Y_file )
    tst_X_Y = read_text_mat( tst_X_Y_file )
    Y = read_desc_file( Y_file )
    num_X = score_mat.shape[1]
    for i in range( num_X ):
        print( "%d\t%s" %( i, tst_X[i] ), file=fout )
        
        print( "true labels:", file=fout )
        labels = myfind( tst_X_Y[:, i] )
        for j in range( len( labels  ) ):
            print( "\t%s" %( Y[labels[j]] ), file=fout )

        print( "predicted labels/tokens:", file=fout )        
        scores = myarr( score_mat[:,i] )
        inds = np.argsort( -scores )
        for j in range( min(print_num_scores,len( inds ) ) ):
            print( "\t%s:%f" %( all_Y[inds[j]], scores[inds[j]] ), file=fout )
        print( "", file=fout )
    fout.close()

def print_label_visual_inspection( vis_file, X_file, Y_file, X_Y_file ):
    fout = open( vis_file, "w" )
    X = read_desc_file( X_file )
    Y = read_desc_file( Y_file )
    X_Y = read_text_mat( X_Y_file )
    num_X = X_Y.shape[1]
    for i in range( num_X ):
        print( "%d\t%s" %( i, X[i] ), file=fout )
        
        print( "true labels:", file=fout )
        labels = myfind( X_Y[:, i] )
        for j in range( len( labels  ) ):
            print( "\t%s:%f" %( Y[labels[j]], X_Y[ labels[j], i ] ), file=fout )

        print( "", file=fout )
    fout.close()

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    
def runBashCommand(command, args):
    bash_command = bash_command = "/usr/bin/time -f '\n" + bcolors.OKBLUE + "Total Time : %E (mm:ss)\nMax RAM : %M kB" + bcolors.ENDC + "\n' "
    display_command = command

    for arg in args:
        display_command += " " + arg
    bash_command += display_command

    print("Running command : " + display_command)

    if(os.system(bash_command) == 0):
        print(bcolors.OKGREEN + "Successfully" + bcolors.ENDC + " ran bash command : " + display_command)
    else:
        print(bcolors.FAIL + "Failure" + bcolors.ENDC + " while running bash command : " + display_command)
        exit(0)

#def print_head_tokens_labels( Y_token, token, Y ):
