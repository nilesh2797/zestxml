from utils import *
from utils import _c
from sklearn.preprocessing import normalize

dataset = sys.argv[1]
RES_DIR = f'Results/{dataset}'

with CaptureIO() as capture:
	print('init') 

def _filter(score_mat, filter_mat, copy=True):
    if filter_mat is None:
        return score_mat
    if copy:
        score_mat = score_mat.copy()
    
    temp = filter_mat.tocoo()
    score_mat[temp.row, temp.col] = 0
    del temp
    score_mat = score_mat.tocsr()
    score_mat.eliminate_zeros()
    return score_mat

DATA_DIR = f'GZXML-Datasets/{dataset}'

print(_c("Loading files", attr="yellow"))
trn_X_Y = read_sparse_mat('%s/trn_X_Y.txt'%DATA_DIR, use_xclib=False)
tst_X_Y = read_sparse_mat('%s/tst_X_Y.txt'%DATA_DIR, use_xclib=False)

fname = f'{DATA_DIR}/pos_trn_tst.txt'
if os.path.exists(fname): 
	print(f'found filter mat : {fname}')
	filter_mat = read_sparse_mat(fname)
else : filter_mat = None

if "Amazon" in dataset: A = 0.6; B = 2.6
elif "Wiki" in dataset: A = 0.5; B = 0.4
else : A = 0.55; B = 1.5
inv_prop = xc_metrics.compute_inv_propesity(trn_X_Y, A, B)

score_mat = _filter(read_bin_spmat(f'{RES_DIR}/score_mat.bin').copy(), filter_mat)
print(_c("\nMetrics", attr="yellow"))
with CaptureIO(capture) as capture:
    printacc(score_mat, X_Y=tst_X_Y, inv_prop_=inv_prop)
print(capture[-1])

with open(f'{RES_DIR}/metrics.txt', 'a+') as f:
        f.write(';'.join([*capture, '']))
        f.write('\n-------------------------------------------------------\n')
