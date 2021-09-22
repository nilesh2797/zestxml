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