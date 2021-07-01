// Takes as input full wikipedia dataset (outfile_WithStem.txt)
// Counts the frequencies of features and labels.
// Outputs features and labels ordered lexicographically, paired with their frequencies.

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>

#include <cstdio>
#include <cstring>
#include <cstdlib>

// allocate memory and exit with error on failure
#define Malloc(ptr,type,n) ptr = (type*)malloc((n)*sizeof(type))
#define Realloc(ptr,type,n) ptr = (type*)realloc((ptr),(n)*sizeof(type))
#define Calloc(ptr,type,n) ptr = (type*)calloc((n),sizeof(type))


using namespace std;

map<string,int> fts;
map<string,int> lbls;

static int MAX_LEN;
static char * line = NULL;

static char* read_line(FILE *input)
{
  // reads next line from 'input'. Exits with message on failure

  int len;
  
  fgets(line,MAX_LEN,input);
  if(ferror(input))
  {
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

  char * tab = strrchr(line,'\t');
  *tab = '\0';

  tok = strtok(line,",");
  while(tok)
  {
    string lbl = string(tok);
    //printf(",%s",lbl.c_str());
    if(lbls.find(lbl)==lbls.end())
      lbls[lbl] = 1;
    else
      lbls[lbl]++;
    tok = strtok(NULL,",");
  }

  tok = strtok(tab+1,",");
  while(tok)
  {
    char * colon = strrchr(tok,':');
    *colon = '\0';
    string ft = string(tok);
    int freq = atoi(colon+1);
    //printf(",%s:%d",ft.c_str(),freq);
    if(fts.find(ft)==fts.end())
      fts[ft] = 1;
    else
      fts[ft]++;
    tok = strtok(NULL,",");
  }
  //printf("\n");
  //cin>>c;
    
  return;
}


int main()
{
  MAX_LEN = 1000;
  Malloc(line,char,MAX_LEN);

  FILE * data_file = fopen("outfile_WithStem.txt","r");

  int ctr = 0;
  
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

  FILE * features_file = fopen("features.txt","w");
  for(map<string,int>::iterator itr=fts.begin(); itr!=fts.end(); itr++)
  {
    fprintf(features_file,"%s,%d\n",(itr->first).c_str(),itr->second);
  }
  fclose(features_file);

  FILE * labels_file = fopen("labels.txt","w");
  for(map<string,int>::iterator itr=lbls.begin(); itr!=lbls.end(); itr++)
  {
    fprintf(labels_file,"%s,%d\n",(itr->first).c_str(),itr->second);
  }
  fclose(labels_file);


  return 0;

}
