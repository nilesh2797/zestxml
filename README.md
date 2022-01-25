<img src="Resources/KDD_Logo.jpg" height="100" align="right"/>

# Generalized Zero-Shot Extreme Multi-Label Learning
This is the official codebase for [KDD 2021](https://www.kdd.org/kdd2021/) paper [Generalized Zero-Shot Extreme Multi-Label Learning](http://manikvarma.org/pubs/gupta21.pdf)
> [Nilesh Gupta](https://nilesh2797.github.io/), [Sakina Bohra](https://www.linkedin.com/in/sakina-bohra-aa46b174/?originalSubdomain=in), [Yashoteja Prabhu](https://vervenumen.github.io/), Saurabh Purohit, [Manik Varma](http://manikvarma.org/)

## Overview
Extreme Multi-label Learning ([`XML`](http://manikvarma.org/downloads/XC/XMLRepository.html)) involves assigning the subset of most relevant labels to a data point from extremely large set of label choices. An unaddressed challenge in XML is that of predicting unseen labels with no training points. 

Generalized Zero-shot XML (`GZXML`) is a paradigm where the task is to **tag a data point with the most relevant labels from a large universe of both seen and unseen labels**.

## Running the Code
```shell
# Build
make

# Download GZ-Eurlex-4.3K dataset
mkdir GZXML-Datasets
cd GZXML-Datasets
pip install gdown
gdown "https://drive.google.com/uc?id=1j27bQZol6gOQ7AATawShcF4jXJr3Venb"
tar -xvzf GZ-Eurlex-4.3K.tar.gz
cd -

# Train and predict ZestXML on GZ-Eurlex-4.3K dataset
./run_eurlex.sh train
./run_eurlex.sh predict

# Install dependencies of metrics.py
pip install -r requirements.txt
# Install pyxclib for evaluation
git clone https://github.com/kunaldahiya/pyxclib.git
cd pyxclib
python3 setup.py install --user
cd -

# Prints evaluation metrics
python metrics.py GZ-Eurlex-4.3K
```
## Public Datasets
Following Datasets were used in the paper for benchmarking `GZXML` algorithms (all datasets can be downloaded from [here](https://drive.google.com/file/d/1Cyi40UP9b527DiPrfJuvqmwm8OUUii0o/view?usp=sharing))
* **GZ-EURLex-4.3K**, Document Tagging of EU law pages
* **GZ-Amazon-1M**, Item to Item Recommendation of Amazon products
* **GZ-Wikipedia-1M**, Document Tagging of Wikipedia pages

Following are some statistics of these datasets:
<table>
  <tr> <td><b>Dataset</b></td> <td colspan="2" align="center"><b>Num Points</b></td> <td colspan="2" align="center"><b>Num Labels</b></td> <td colspan="2" align="center"><b>Num Features</b></td> </tr>
  <tr> <td></td> <td><b>Train</b></td> <td><b>Test</b></td> <td><b>Seen</b></td> <td><b>Unseen</b></td> <td><b>Point</b></td> <td><b>Label</b></td> </tr>
  <tr> <td><b>GZ-Eurlex-4.3K</b></td> <td>45,000</td> <td>6,000</td> <td>4,108</td> <td>163</td> <td>100,000</td> <td>24,316</td> </tr>
  <tr> <td><b>GZ-Amazon-1M</b></td> <td>914,179</td> <td>1,465,767</td> <td>476,381</td> <td>483,725</td> <td>1,000,000</td> <td>1,476,381</td> </tr>
  <tr> <td><b>GZ-Wikipedia-1M</b></td> <td>2,271,533</td> <td>2,705,425</td> <td>495,107</td> <td>776,612</td> <td>1,000,000</td> <td>1,438,196</td> </tr>
</table>

## Data Format
All sparse matrices are stored in text sparse matrix format, please refer to the text sparse matrix format subsection for more details. Following are the details of required files:
* **`Xf.txt`**: all features used in `tf-idf` representation of documents (`(trn/tst/val)_X_Xf`), `ith` line denotes `ith` feature in the tf-idf representation. In particular, for datasets used in the paper, it's the stemmed bigram and unigram features of documents but you can choose to have any set of features depending on your application.
* **`Yf.txt`**: similar to `Xf.txt` it represents features of all labels. In addition to unigrams and bigrams, we also add a unique feature specific to each label (represented by `__label__<i>__<label-i-text>`, this feature will only be present in `ith` label's features), this allows the model to have label specific parameters and helps it to do well on many-shot labels. Features with `__parent__` in them are only specific to the `GZ-EURLex-4.3K` dataset because raw labels in this dataset have some additional information about parent concepts of each label, you can safely choose to ignore these features for any other/new dataset.
* **`(trn/tst/val)_X_Xf.txt`**: sparse matrix (*documents x document-features*) representing `tf-idf` feature matrix of *(trn/tst/val)* input documents.
* **`Y_Yf.txt`**: similar to `(trn/tst/val)_X_Xf.txt` but for labels, this is the sparse matrix (*labels x label-features*) representing `tf-idf` feature matrix of labels.
* **`trn_Y_Yf.txt`**: similar to `Y_Yf.txt` but contains features for only the seen labels (can be interpreted as `Y_Yf[seen-labels]`)
* **`(trn/tst/val)_X_Y.txt`**: sparse matrix (*documents x labels*) representing *(trn/tst/val)* document-label relevance matrix.

### Text sparse matrix format
This is a plain-text row-major representation of a sparse matrix. Following are the details of the format :
- The first line in this format is two space separated integers denoting the dimensions of the matrix (i.e. `num_row` `num_column`)
- `num_row` lines follow the first line and each line represents a sparse row vector
- a sparse row vector is represented as space separated non zero entries of the vector, an entry in the vector is represented as `<index>:<value>`. For example if the vector is `[0, 0, 0.5, 0.4, 0, 0.2]` then its sparse vector text representation is `2:0.5 3:0.4 5:0.2` (NOTE : the indexing starts from 0)
- You can check `GZ-Eurlex-4.3K/trn_X_Xf.txt` for sample example of a sparse matrix format

## Cite
```bib
@InProceedings{Gupta21,
  author    = "Gupta, N. and Bohra, S. and Prabhu, Y. and Purohit, S. and Varma, M.",
  title     = "Generalized Zero-Shot Extreme Multi-label Learning",
  booktitle = "Proceedings of the ACM SIGKDD Conference on Knowledge Discovery and Data Mining",
  month     = "August",
  year      = "2021"
}
```
