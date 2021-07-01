// Takes as input wikipedia full data (outfile_WithStem.txt) and feature,label maps
// Calculates idf values of features
// Maps feature and label strings to their indices and outputs tf-idf features as well as labels into a libsvm format output

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

// allocate memory and exit with error on failure
#define Malloc(ptr,type,n) ptr = (type*)malloc((n)*sizeof(type))
#define Realloc(ptr,type,n) ptr = (type*)realloc((ptr),(n)*sizeof(type))
#define Calloc(ptr,type,n) ptr = (type*)calloc((n),sizeof(type))
#define DEBUG printf("1\n");fflush(stdout);
#define pairIF pair<int,float>

using namespace std;

map<string,int> ft_indices;
map<string,float> ft_idfs;
map<string,int> lbl_indices;

static int MAX_LEN;
static char * line = NULL;
FILE * output_file;

inline bool comp(pairIF a, pairIF b)
{
  if(a.first<b.first)
    return true;

  return false;
}

static char* read_line(FILE *input)
{
  // reads next line from 'input'. Exits with message on failure

  int len;
  
  fgets(line,MAX_LEN,input);
  if(ferror(input))
  {
    char mesg[100];
    printf("error while reading input file");
    exit(1);
  }
  if(feof(input))
    return NULL;

  while(strrchr(line,'\n') == NULL)
  {
    MAX_LEN *= 2;
    Realloc(line,char,MAX_LEN);
    len = (int) strlen(line);
    if(fgets(line+len,MAX_LEN-len,input) == NULL)
      break;
  }

  char * cptr = strrchr(line,'\n');
  *cptr = '\0';

  return line;
}

void parse_line()
{
  char c;
  int ctr;
  char * tok;
  string id;
  float val;

  vector<pairIF> fts;
  vector<int> lbls;

  char * tab = strrchr(line,'\t');
  *tab = '\0';

  tok = strtok(line,",");
  while(tok)
  {
    string lbl = string(tok);
    if(lbl_indices.find(lbl)!=lbl_indices.end())
      lbls.push_back(lbl_indices[lbl]);

    tok = strtok(NULL,",");
  }

  tok = strtok(tab+1,",");
  while(tok)
  {
    char * colon = strrchr(tok,':');
    *colon = '\0';
    string ft = string(tok);
    int freq = atoi(colon+1);
    if(ft_indices.find(ft) != ft_indices.end())
      fts.push_back(make_pair(ft_indices[ft],(float)freq*ft_idfs[ft]));

    tok = strtok(NULL,",");
  }

  sort(fts.begin(),fts.end(),comp);
  sort(lbls.begin(),lbls.end());

  for(int i=0; i<lbls.size(); i++)
    if(i==0)
      fprintf(output_file,"%d",lbls[0]);
    else
      fprintf(output_file,",%d",lbls[i]);

  for(int i=0; i<fts.size(); i++)
    fprintf(output_file," %d:%f",fts[i].first,fts[i].second);
  
  fprintf(output_file,"\n");
    
  return;
}

int main()
{
  int ctr;
  MAX_LEN = 10000;
  Malloc(line,char,MAX_LEN);

  int num_inst = 2512220;

  FILE * features_file = fopen("features.txt","r");
  ctr = 0;
  while(!feof(features_file))
  {
    fgets(line,MAX_LEN,features_file);
    if(strrchr(line,'\n')==NULL)
      continue;
    //cout<<line<<endl;
    //fflush(stdout);
    char * newline = strrchr(line,'\n');
    *newline = '\0';
    char * colon = strrchr(line,',');
    *colon = '\0';
    string ft = string(line);
    int freq = atoi(colon+1);
    ft_indices[ft] = ctr;
    ft_idfs[ft] = log((float)num_inst/freq);
    ctr++;
  }
  fclose(features_file);

DEBUG  

  FILE * labels_file = fopen("labels.txt","r");
  ctr= 0;
  while(!feof(labels_file))
  {
    fgets(line,MAX_LEN,labels_file);
    if(strrchr(line,'\n')==NULL)
      continue;
    char * newline = strrchr(line,'\n');
    *newline = '\0';
    char * colon = strrchr(line,',');
    *colon = '\0';
    string lbl = string(line);
    lbl_indices[lbl] = ctr;
    ctr++;
  }
  fclose(labels_file);

DEBUG

  output_file = fopen("data.txt","w");

  FILE * data_file = fopen("outfile_WithStem.txt","r");

  ctr = 0;
  while(true)
  {
    read_line(data_file);
    if(feof(data_file))
      break;

    parse_line();
    ctr++;
    if(ctr%100==0)
      printf("%d\n",ctr);
  }

  free(line);

  fclose(data_file);
  fclose(output_file);

  return 0;

}

