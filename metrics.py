from utils import *
from sklearn.preprocessing import normalize

dataset = sys.argv[1]
RES_DIR = sys.argv[2]
alpha = -1; beta = -1; gamma = -1; delta = -1
if len(sys.argv) > 3: alpha = float(sys.argv[3])
if len(sys.argv) > 4: beta = float(sys.argv[4])
if len(sys.argv) > 5: gamma = float(sys.argv[5])
if len(sys.argv) > 6: delta = float(sys.argv[6])
validation = True

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

DATA_DIR = f'/home/t-nilgup/Datasets/GZXML-Datasets/{dataset}'

val_X_Y = read_sparse_mat('%s/val_X_Y.txt'%DATA_DIR, use_xclib=False)
trn_X_Y = read_sparse_mat('%s/trn_X_Y.txt'%DATA_DIR, use_xclib=False)
tst_X_Y = read_sparse_mat('%s/tst_X_Y.txt'%DATA_DIR, use_xclib=False)

fname = f'{DATA_DIR}/pos_trn_{["tst", "val"][int(validation)]}.txt'
if os.path.exists(fname): 
	print(f'found filter mat : {fname}')
	filter_mat = read_sparse_mat(fname)
else : filter_mat = None

nnz = trn_X_Y.getnnz(0)
valid_labels = np.where(nnz > 0)[0]
unseen_labels = np.where(nnz == 0)[0]
fewshot_labels = np.where((nnz >= 0) & (nnz <= 5))[0]
inv_valid_labels = np.vectorize({v : i for i, v in enumerate(valid_labels)}.get)

if "Amazon" in dataset: A = 0.6; B = 2.6
elif "Wiki" in dataset: A = 0.5; B = 0.4
else : A = 0.55; B = 1.5
inv_prop = xc_metrics.compute_inv_propesity(trn_X_Y, A, B)

all_score_mats = {}
for name in ['clf_score_mat', 'bilinear_score_mat', 'knn_score_mat', 'shortlist']:
	if os.path.exists(f'{RES_DIR}/{name}.bin'):
		if filter_mat is None:
			all_score_mats[name] = read_bin_spmat(f'{RES_DIR}/{name}.bin').copy()
		else:
			all_score_mats[name] = _filter(read_bin_spmat(f'{RES_DIR}/{name}.bin').copy(), filter_mat, copy=False)
	else:
		print(f'{RES_DIR}/{name}.bin file not found, skipping...')

if (alpha < 0) and (beta < 0) and (gamma < 0):
	print('not combining score mats since all weight parameters are none.')
else:
	name = f'combined_score_mat(alpha={alpha}, beta={beta}, gamma={gamma}, delta={delta})'
	if validation:
		all_score_mats[name] = csr_matrix((val_X_Y.shape))
	else:
		all_score_mats[name] = csr_matrix((tst_X_Y.shape))
	if alpha > 0: all_score_mats[name] += alpha*normalize(all_score_mats['clf_score_mat'], norm="max")
	if beta > 0: all_score_mats[name] += beta*normalize(all_score_mats['bilinear_score_mat'], norm="max")
	if gamma > 0: all_score_mats[name] += gamma*normalize(all_score_mats['knn_score_mat'], norm="max")
	if delta > 0: all_score_mats[name] += delta*normalize(all_score_mats['shortlist'], norm="max")

for name, score_mat in all_score_mats.items():
	with CaptureIO(capture) as capture:
		print(f'\n{name}: ')
		get_irm_metrics(score_mat, X_Y=[tst_X_Y, val_X_Y][int(validation)], unseen=unseen_labels, fewshot=fewshot_labels, inv_prop=inv_prop)
	print(capture[-1])

with open(f'{RES_DIR}/metrics.txt', 'a+') as f:
        f.write(';'.join([*capture, '']))
        f.write('\n-------------------------------------------------------\n')
