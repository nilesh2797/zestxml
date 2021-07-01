#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <sstream>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include "linear.h"
#include "object.h"
#include "utils.h"
#include "io.h"
#include "free.h"
#include "local_io.h"
#include "metrics.h"

using namespace std;

float bias;

void predict_leaf_node(prob_struct & prob_obj, param_struct & param_obj, node * nd, int * inds, map<int,float> * & tst_lbl_scores)
{
  int ctr;
  int lbl_per_leaf = param_obj.LblPerLeaf;
  pairIF * leaf_dist = nd->leaf_dist;

  ctr = 0;
  int ind = inds[ctr];
  // aggregate label scores for test instance 'ind' into tst_lbl_scores[ind] hash map
  while(ind!=-1)
  {
    for(int i=0; i<lbl_per_leaf; i++)
    {
      if(tst_lbl_scores[ind].find(leaf_dist[i].first) == tst_lbl_scores[ind].end())
	tst_lbl_scores[ind][leaf_dist[i].first] = leaf_dist[i].second;
      else
	tst_lbl_scores[ind][leaf_dist[i].first] += leaf_dist[i].second;
    }
    ctr++;
    ind = inds[ctr];
  }
}

void predict_internal_node(prob_struct & prob_obj, param_struct & param_obj, node * nd, int * inds, int * & left_inds, int * & right_inds)
{
  int ctr;
  int num_ft = prob_obj.num_ft;

  // ft_mask and ft_val: dense representation of sparse vector 'W'. Useful for fast prediction
  ///
  bool * ft_mask;                       // ft_mask[i] = true <=> W has index i
  Calloc(ft_mask,bool,num_ft);
  float * ft_val = new float[num_ft];   // if ft_mask[i]=true, then ft_val[i] = W[k].second (where W[k].first=i)
  ///

  pairIF* W = nd->W;
  float bias_coeff;
  ctr = 0;
  if(W)
    while(W[ctr].first != -1)
    {
      if(W[ctr].first == num_ft)
	bias_coeff = W[ctr].second;
      else
      {
	ft_mask[W[ctr].first] = true;
	ft_val[W[ctr].first] = W[ctr].second;
      }
      ctr++;
    }
  vector<int> left_inds_tmp;   // temporarily holds instances going to left child
  vector<int> right_inds_tmp;  // temporarily holds instances going to right child

  ctr = 0;
  while(inds[ctr]!=-1)
  {
    pairIF * ft_vec = prob_obj.ft_mat[inds[ctr]];
    int ctr1 = 0;
    float sum = 0;    // holds w'*x
    while(ft_vec[ctr1].first != -1)
    {
      if(ft_mask[ft_vec[ctr1].first])
      {
	sum += ft_val[ft_vec[ctr1].first]*ft_vec[ctr1].second;
      }
      ctr1++;
    }
    if(sum+bias_coeff*bias<0)
      left_inds_tmp.push_back(inds[ctr]);
    else
      right_inds_tmp.push_back(inds[ctr]);
    ctr++;
  }
  free(ft_mask);
  delete [] ft_val;

  int num_left = left_inds_tmp.size();
  int num_right = right_inds_tmp.size();

  // copying left and right instances from temprary vectors to output arrays
  ///
  left_inds = new int[num_left+1];
  for(int i=0; i<num_left; i++)
  {
    left_inds[i] = left_inds_tmp[i];
  }
  left_inds[num_left] = -1;

  right_inds = new int[num_right+1];
  for(int i=0; i<num_right; i++)
  {
    right_inds[i] = right_inds_tmp[i];
  }
  right_inds[num_right] = -1;
  ///
}

void diff_scores(int num_tst, pairIF** score_mat1, pairIF** score_mat2, pairIF** & score_mat)
{
  int ctr;
  Malloc(score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    map<int,float> diff_map;

    pairIF* score_vec = score_mat1[i];
    ctr = 0;
    while(score_vec[ctr].first!=-1)
    {
      if(diff_map.find(score_vec[ctr].first)!=diff_map.end())
	diff_map[score_vec[ctr].first] += score_vec[ctr].second;
      else
	diff_map[score_vec[ctr].first] = score_vec[ctr].second;
      ctr++;
    }

    score_vec = score_mat2[i];
    ctr = 0;
    while(score_vec[ctr].first!=-1)
    {
      if(diff_map.find(score_vec[ctr].first)!=diff_map.end())
	diff_map[score_vec[ctr].first] -= score_vec[ctr].second;
      else
	exit_with_message("error while diff, label does not exist in bigger score_mat\n");
      ctr++;
    }

    int num_entries = diff_map.size();
    score_mat[i] = new pairIF[num_entries+1];
    int j;
    map<int,float>::iterator itr;
    for(j=0,itr=diff_map.begin(); j<num_entries; j++,itr++)
    {
      score_mat[i][j].first = itr->first;
      score_mat[i][j].second = itr->second;
    }
    score_mat[i][num_entries].first = -1;
  }
}

void merge_scores(int num_tst,pairIF** & agg_score_mat,pairIF** score_mat)
{
  int ctr;
  for(int i=0; i<num_tst; i++)
  {
    map<int,float> merge_map;

    pairIF* score_vec = agg_score_mat[i];
    ctr = 0;
    while(score_vec[ctr].first!=-1)
    {
      if(merge_map.find(score_vec[ctr].first)!=merge_map.end())
	merge_map[score_vec[ctr].first] += score_vec[ctr].second;
      else
	merge_map[score_vec[ctr].first] = score_vec[ctr].second;
      ctr++;
    }

    score_vec = score_mat[i];
    ctr = 0;
    while(score_vec[ctr].first!=-1)
    {
      if(merge_map.find(score_vec[ctr].first)!=merge_map.end())
	merge_map[score_vec[ctr].first] += score_vec[ctr].second;
      else
	merge_map[score_vec[ctr].first] = score_vec[ctr].second;
      ctr++;
    }

    delete [] agg_score_mat[i];

    int num_entries = merge_map.size();
    agg_score_mat[i] = new pairIF[num_entries+1];
    int j;
    map<int,float>::iterator itr;
    for(j=0,itr=merge_map.begin(); j<num_entries; j++,itr++)
    {
      agg_score_mat[i][j].first = itr->first;
      agg_score_mat[i][j].second = itr->second;
    }
    agg_score_mat[i][num_entries].first = -1;
  }
}

void calc_backward_greedy_order(string output_folder_name,int num_trees,prob_struct & prob_obj,pairIF*** vec_score_mat)
{
  char c;
  int ctr;
  int num_tst = prob_obj.num_inst;
  int num_lbl = prob_obj.num_lbl;
  int** tst_lbl_mat = prob_obj.lbl_mat;

  vector<int> greedy_order;

  pairIF ** agg_score_mat;
  Malloc(agg_score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    agg_score_mat[i] = new pairIF[1];
    agg_score_mat[i][0].first = -1;
  }
  for(int i=0; i<num_trees; i++)
  {
    merge_scores(num_tst,agg_score_mat,vec_score_mat[i]);
  }

  vector<int> inds;
  for(int i=0; i<num_trees; i++)
    inds.push_back(i);

  for(int i=num_trees; i>=1; i--)
  {
    printf("i %d\n",i);

    if(i==1)
    {
      greedy_order.push_back(inds[0]);
      break;
    }

    vector<float> p1s;
    for(int j=0; j<inds.size(); j++)
    {
      int ind = inds[j];
      pairIF** score_mat;
      diff_scores(num_tst,agg_score_mat,vec_score_mat[ind],score_mat);
      pairIF** sorted_score_mat;
      sort_float_mat_by_value(num_tst,(const pairIF**)score_mat,sorted_score_mat);
      float P1 = get_precision_at_K(num_tst,num_lbl,(const int**)tst_lbl_mat,(const pairIF**)sorted_score_mat,1);
      free_float_mat(num_tst,sorted_score_mat);
      free_float_mat(num_tst,score_mat);
      p1s.push_back(P1);
    }
    int maxind = -1;
    float maxP1 = -1;
    for(int j=0; j<inds.size(); j++)
    {
      if(p1s[j]>maxP1)
      {
	maxP1 = p1s[j];
	maxind = j;
      }
    }
    greedy_order.push_back(inds[maxind]);
    inds.erase(inds.begin()+maxind);
  }
  free_float_mat(num_tst,agg_score_mat);

  Malloc(agg_score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    agg_score_mat[i] = new pairIF[1];
    agg_score_mat[i][0].first = -1;
  }

  string out_file_name = output_folder_name+"/p1_backward_greedy_order.txt";
  FILE * out_file;
  Fopen(out_file,out_file_name.c_str(),"w");

  for(int I=0; I<num_trees; I++)
  {
    int i = greedy_order[num_trees-I-1];
    merge_scores(num_tst,agg_score_mat,vec_score_mat[i]);
    pairIF** sorted_score_mat;
    sort_float_mat_by_value(num_tst,(const pairIF**)agg_score_mat,sorted_score_mat);
    float P1 = get_precision_at_K(num_tst,num_lbl,(const int**)tst_lbl_mat,(const pairIF**)sorted_score_mat,1);
    free_float_mat(num_tst,sorted_score_mat);
    fprintf(out_file,"%d\t%f\n",I+1,P1);
    printf("%d\t%f\n",I+1,P1);
  }

  fclose(out_file);

  free_float_mat(num_tst,agg_score_mat);

}

void calc_greedy_order(string output_folder_name,int num_trees,prob_struct & prob_obj,pairIF*** vec_score_mat)
{
  int ctr;
  int num_tst = prob_obj.num_inst;
  int num_lbl = prob_obj.num_lbl;
  int** tst_lbl_mat = prob_obj.lbl_mat;

  vector<pairIF> p1_vec;
  for(int i=0; i<num_trees; i++)
  {
    pairIF** sorted_score_mat;
    sort_float_mat_by_value(num_tst,(const pairIF**)vec_score_mat[i],sorted_score_mat);
    float P1 = get_precision_at_K(num_tst,num_lbl,(const int**)tst_lbl_mat,(const pairIF**)sorted_score_mat,1);
    free_float_mat(num_tst,sorted_score_mat);
    p1_vec.push_back(make_pair(i,P1));
  }
  sort(p1_vec.begin(),p1_vec.end(),comp_pairIF_by_second_desc);

  pairIF ** agg_score_mat;
  Malloc(agg_score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    agg_score_mat[i] = new pairIF[1];
    agg_score_mat[i][0].first = -1;
  }

  string out_file_name = output_folder_name+"/p1_greedy_order.txt";

  FILE * out_file;
  Fopen(out_file,out_file_name.c_str(),"w");

  for(int I=0; I<num_trees; I++)
  {
    int i = p1_vec[I].first;
    merge_scores(num_tst,agg_score_mat,vec_score_mat[i]);
    pairIF** sorted_score_mat;
    sort_float_mat_by_value(num_tst,(const pairIF**)agg_score_mat,sorted_score_mat);
    float P1 = get_precision_at_K(num_tst,num_lbl,(const int**)tst_lbl_mat,(const pairIF**)sorted_score_mat,1);
    free_float_mat(num_tst,sorted_score_mat);
    fprintf(out_file,"%d\t%f\n",I+1,P1);
    printf("%d\t%f\n",I+1,P1);
  }

  fclose(out_file);

  free_float_mat(num_tst,agg_score_mat);
}


void calc_normal_order(string output_folder_name,int num_trees,prob_struct & prob_obj,pairIF*** vec_score_mat)
{
  int ctr;
  int num_tst = prob_obj.num_inst;
  int num_lbl = prob_obj.num_lbl;
  int** tst_lbl_mat = prob_obj.lbl_mat;

  pairIF ** agg_score_mat;
  Malloc(agg_score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    agg_score_mat[i] = new pairIF[1];
    agg_score_mat[i][0].first = -1;
  }

  string out_file_name = output_folder_name+"/p1_normal_order.txt";

  FILE * out_file;
  Fopen(out_file,out_file_name.c_str(),"w");

  for(int i=0; i<num_trees; i++)
  {
    merge_scores(num_tst,agg_score_mat,vec_score_mat[i]);
    pairIF** sorted_score_mat;
    sort_float_mat_by_value(num_tst,(const pairIF**)agg_score_mat,sorted_score_mat);
    float P1 = get_precision_at_K(num_tst,num_lbl,(const int**)tst_lbl_mat,(const pairIF**)sorted_score_mat,1);
    free_float_mat(num_tst,sorted_score_mat);
    fprintf(out_file,"%d\t%f\n",i+1,P1);
    printf("%d\t%f\n",i+1,P1);
  }

  fclose(out_file);
}

void calc_score_mat(int split_num,int tree_num,string output_folder_name,pairIF** & score_mat,prob_struct & prob_obj,param_struct & param_obj)
{
  int num_tst = prob_obj.num_inst;

  stringstream ss;
  ss<<output_folder_name<<"/"<<split_num;
  string model_folder_name = ss.str();

  tree * T;
  float model_size;
  read_tree(T,model_folder_name,tree_num,model_size);

  int size = T->size;

  map<int,float> * tst_lbl_scores = new map<int,float>[num_tst];
  int ** tst_inds = new int* [size];
  int * inds = new int[num_tst+1];
  for(int j=0; j<num_tst; j++)
  {
    inds[j] = j;
  }
  inds[num_tst] = -1;
  tst_inds[0] = inds;

  // do prediction over node 'j' of tree 'i'
  for(int j=0; j<size; j++)
  {
    node * nd = T->node_list[j];
    if(nd->is_leaf)
    {
      // if node 'j' is leaf, aggregate label scores for the test instances in 'j' into 'tst_lbl_scores'
      predict_leaf_node(prob_obj, param_obj, nd, tst_inds[j], tst_lbl_scores);
    }
    else
    {
      // if node 'j' is internal, assign each test instance into either left child or right child of 'j'
      predict_internal_node(prob_obj, param_obj, nd, tst_inds[j], tst_inds[nd->left_child], tst_inds[nd->right_child]);
    }
  }

  Malloc(score_mat,pairIF*,num_tst);
  for(int i=0; i<num_tst; i++)
  {
    int num_tst_lbls = tst_lbl_scores[i].size();
    map<int,float>::iterator itr = tst_lbl_scores[i].begin();
    score_mat[i] = new pairIF[num_tst_lbls+1];
    for(int j=0; j<num_tst_lbls; j++)
    {
      score_mat[i][j] = make_pair(itr->first,itr->second);
      itr++;
    }
    score_mat[i][num_tst_lbls].first = -1;
  }

  for(int j=0; j<size; j++)
    delete [] tst_inds[j];
  delete [] tst_inds;

  for(int i=0; i<num_tst; i++)
  {
    tst_lbl_scores[i].clear();
  }
  delete [] tst_lbl_scores;

  free_tree(T);
}


int main(int argc, char * argv[])
{
  int ctr;

  string data_folder_name = string(argv[1]);
  string output_folder_name = string(argv[2]);
  int split_num = atoi(argv[3]);
  int normalize = atoi(argv[4]);

  pairIF** trn_ft_mat;
  pairIF** tst_ft_mat;
  int** trn_lbl_mat;
  int** tst_lbl_mat;
  int num_trn,num_tst,num_ft,num_lbl;

  read_train_test(data_folder_name,split_num,num_trn,num_tst,num_ft,num_lbl,trn_ft_mat,trn_lbl_mat,tst_ft_mat,tst_lbl_mat);

  if(normalize)
    normalize_features(num_trn,num_tst,num_ft,trn_ft_mat,tst_ft_mat);
  
  free_float_mat(num_trn,trn_ft_mat);
  free_bool_mat(num_trn,trn_lbl_mat);

  param_struct param_obj;
  read_param(output_folder_name,param_obj);

  prob_struct prob_obj;
  prob_obj.num_inst = num_tst;
  prob_obj.num_ft = num_ft;
  prob_obj.num_lbl = num_lbl;
  prob_obj.lbl_mat = tst_lbl_mat;
  prob_obj.ft_mat = tst_ft_mat;

  bias = param_obj.Bias;
  int num_trees = param_obj.NumTrees;
  pairIF *** vec_score_mat  = new pairIF** [num_trees];
  for(int i=0; i<num_trees; i++)
  {
    printf("%d\n",i);
    fflush(stdout);
    calc_score_mat(split_num,i,output_folder_name,vec_score_mat[i],prob_obj,param_obj);
  }

  calc_normal_order(output_folder_name,num_trees,prob_obj,vec_score_mat);
  calc_greedy_order(output_folder_name,num_trees,prob_obj,vec_score_mat);
  calc_backward_greedy_order(output_folder_name,num_trees,prob_obj,vec_score_mat);


  for(int i=0; i<num_trees; i++)
  {
    for(int j=0; j<num_tst; j++)
    {
      delete [] vec_score_mat[i][j];
    }
    free(vec_score_mat[i]);
  }
  delete [] vec_score_mat;

  free_float_mat(num_tst,tst_ft_mat);
  free_bool_mat(num_tst,tst_lbl_mat);

}
