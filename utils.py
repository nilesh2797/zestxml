import os
import sys
import numpy as np
import scipy.sparse as sp
from scipy.sparse import csr_matrix
import xclib
import xclib.evaluation.xc_metrics as xc_metrics
import xclib.data.data_utils as data_utils
import pandas as pd
from tabulate import tabulate
from io import StringIO
from tqdm import tqdm

class CaptureIO(list):
    def __enter__(self):
        self._stdout = sys.stdout
        sys.stdout = self._stringio = StringIO()
        return self
    def __exit__(self, *args):
        self.append(''.join(self._stringio.getvalue()))
        del self._stringio    # free up some memory
        sys.stdout = self._stdout

def write_sparse_mat(X, filename, header=True):
    if not isinstance(X, csr_matrix):
        X = X.tocsr()
    X.sort_indices()
    with open(filename, 'w') as f:
        if header:
            print("%d %d" % (X.shape[0], X.shape[1]), file=f)
        for y in X:
            idx = y.__dict__['indices']
            val = y.__dict__['data']
            sentence = ' '.join(['%d:%.5f'%(x, v)
                                 for x, v in zip(idx, val)])
            print(sentence, file=f)
            
def read_sparse_mat(filename, use_xclib=True):
    if use_xclib:
        return xclib.data.data_utils.read_sparse_file(filename)
    else:
        with open(filename) as f:
            nr, nc = map(int, f.readline().split(' '))
            data = []; indices = []; indptr = [0]
            for line in tqdm(f):
                if len(line) > 1:
                    row = [x.split(':') for x in line.split()]
                    tempindices, tempdata = list(zip(*row))
                    indices.extend(list(map(int, tempindices)))
                    data.extend(list(map(float, tempdata)))
                    indptr.append(indptr[-1]+len(tempdata))
                else:
                    indptr.append(indptr[-1])
            score_mat = csr_matrix((data, indices, indptr), (nr, nc))
            del data, indices, indptr
            return score_mat

size_dict = {bool : 1, np.int32 : 4, np.float32 : 4, np.int64 : 8}

def readbuf(buf, dtype, offset=0, count=1):
    val = np.frombuffer(buf, offset=offset, dtype=dtype, count=count)
    if count == 1: val = val[0]
    offset += size_dict[dtype]*count
    return val, offset

def read_buf_bin_vecif(buf, dtype, offset = 0, totlen = -1):
    if totlen < 0: totlen, offset = readbuf(buf, np.int64, offset, 1)
    totlen *= 2
    
    temp, _ = readbuf(buf, np.int32, offset, totlen)
    inds = temp.reshape(-1, 2)[:, 0];
    temp, offset = readbuf(buf, dtype, offset, totlen)
    data = temp.reshape(-1, 2)[:, 1];
    
    nr = 1
    if inds.shape[0] > 0: nr = inds.max()+1
    return csr_matrix((data, inds, [0, len(data)]), (1, nr)), offset

def read_buf_bin_vec(buf, dtype, offset = 0, totlen = -1):
    if totlen < 0: totlen, offset = readbuf(buf, np.int64, offset, 1)
    return readbuf(buf, dtype, offset, totlen)

def read_buf_bin_spmat(buf, dtype, offset = 0, old = False):
    (nr, nc), offset = readbuf(buf, np.int32, offset, 2)
    
    size = None
    if old: size, offset = read_buf_bin_vec(buf, np.int32, offset, nr)
    else: size, offset = read_buf_bin_vec(buf, np.int32, offset)

    data = []; inds = []; indptr = np.zeros(nr+1, int)
    indptr[1:] = size.cumsum()
    totlen = indptr[-1]
    temp, offset = read_buf_bin_vecif(buf, dtype, offset, totlen)

    return csr_matrix((temp.data, temp.indices, indptr), (nr, nc)), offset

def read_bin_spmat(fname, old = False):
    buf = open(fname, 'rb').read()
    print('loaded bin file in buffer')
    spmat, _ = read_buf_bin_spmat(buf, np.float32, 0, old)
    return spmat

class bcolors:
    purple = '\033[95m'
    blue = '\033[94m'
    green = '\033[92m'
    warn = '\033[93m' # dark yellow
    fail = '\033[91m' # dark red
    white = '\033[37m'
    yellow = '\033[33m'
    red = '\033[31m'
    
    ENDC = '\033[0m'
    bold = '\033[1m'
    underline = '\033[4m'
    reverse = '\033[7m'
    
    on_grey = '\033[40m'
    on_yellow = '\033[43m'
    on_red = '\033[41m'
    on_blue = '\033[44m'
    on_green = '\033[42m'
    on_magenta = '\033[45m'
    
def _c(*args, attr='bold'):
    string = ''.join([bcolors.__dict__[a] for a in attr.split()])
    string += ' '.join([str(arg) for arg in args])+bcolors.ENDC
    return string

def printacc(score_mat, K = 5, X_Y = None, disp = True, inv_prop_ = -1):
    if X_Y is None: X_Y = tst_X_Y
    if inv_prop_ is -1 : inv_prop_ = inv_prop
        
    acc = xc_metrics.Metrics(X_Y.tocsr().astype(np.bool), inv_prop_)
    metrics = np.array(acc.eval(score_mat, K))*100
    df = pd.DataFrame(metrics)
    
    if inv_prop_ is None : df.index = ['P', 'nDCG']
    else : df.index = ['P', 'nDCG', 'PSP', 'PSnDCG']
        
    df.columns = [str(i+1) for i in range(K)]
    if disp: print(tabulate(df.round(2), ["metric", *[str(i+1) for i in range(K)]], tablefmt="pretty"))
    return metrics

def restrict_mat(spmat, active_rows = None, active_cols = None, shrink=False):
    if active_cols is not None and active_rows is not None:
        temp = spmat[active_rows][:,active_cols]
    elif active_rows is not None: 
        temp = spmat[active_rows]
        if shrink: 
            active_cols = np.where(temp.getnnz(0)>0)[0]
            temp = temp[:, active_cols]
    elif active_cols is not None: 
        temp = spmat[:,active_cols]
        if shrink:
            active_rows = np.where(temp.getnnz(1)>0)[0]
            temp = temp[active_rows]
    else:
        if shrink:
            active_rows = np.where(spmat.getnnz(1)>0)[0]
            active_cols = np.where(spmat.getnnz(0)>0)[0]
            temp = spmat[active_rows][:,active_cols]
        else:
            return spmat, None, None
    
    return temp, active_rows, active_cols

def get_irm_metrics(score_mat, unseen = None, fewshot=None, X_Y=None, inv_prop=None):
    if X_Y is None : X_Y = tst_X_Y
    if unseen is None : unseen = unseen_labels
    if fewshot is None : fewshot = fewshot_labels
        
    mask = np.ones(X_Y.shape[1], bool); mask[unseen] = False; seen = np.where(mask)[0]
    metrics = {'GZSL' : None, 'SZSL' : None, 'XC' : None}
    
    print(_c('GZSL metrics (shape=(%d, %d)): '%(X_Y.shape[0], X_Y.shape[1]), attr='bold'))
    metrics['GZSL'] = printacc(score_mat, X_Y=X_Y, K=5, inv_prop_=inv_prop)
    
    temp_X_Y, active_rows, _ = restrict_mat(X_Y, active_rows=None, active_cols=fewshot, shrink=True)
    temp_score_mat, _, _     = restrict_mat(score_mat, active_rows=active_rows, active_cols=fewshot, shrink=True)
    print(_c('FewShot metrics (shape=(%d, %d)): '%(temp_X_Y.shape[0], temp_X_Y.shape[1]), attr='bold'))
    metrics['FewShot'] = printacc(temp_score_mat, X_Y=temp_X_Y, K=5, inv_prop_=None)
    
    temp_X_Y, active_rows, _ = restrict_mat(X_Y, active_rows=None, active_cols=unseen, shrink=True)
    temp_score_mat, _, _     = restrict_mat(score_mat, active_rows=active_rows, active_cols=unseen, shrink=True)
    print(_c('SZSL metrics (shape=(%d, %d)): '%(temp_X_Y.shape[0], temp_X_Y.shape[1]), attr='bold'))
    metrics['SZSL'] = printacc(temp_score_mat, X_Y=temp_X_Y, K=5, inv_prop_=None)
    
    temp_X_Y, active_rows, _ = restrict_mat(X_Y, active_rows=None, active_cols=seen, shrink=True)
    temp_score_mat, _, _     = restrict_mat(score_mat, active_rows=active_rows, active_cols=seen, shrink=True)
    print(_c('XC metrics (shape=(%d, %d)): '%(temp_X_Y.shape[0], temp_X_Y.shape[1]), attr='bold'))
    metrics['XC'] = printacc(temp_score_mat, X_Y=temp_X_Y, K=5, inv_prop_=inv_prop[seen])
    
    del temp_score_mat, temp_X_Y, active_rows
    return metrics