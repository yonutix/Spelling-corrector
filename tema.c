/**
 *@author Mihai Cosmin 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INT 0x7FFFFFFF
#define MIN(a, b) ((a>b)?b:a)
#define MAX(a, b) ((a<b)?b:a)
#define TMIN(a, b, c) ((a<b)?((a<c)?a:c):((b<c)?b:c))


/*m este matricea distantei levenshtein*/
int m[65][65], max_word_size;

/**
 * \brief Contine un cuvant din dictionar si frecventa asociata
 */
typedef struct dict{
  char word[128];
  int freq;
}dict;

/*elementele matricii cu pointeri catre elmente cu lungimi diferite ale cuvintelor din vectorul cu dictionarul
si lungimea partii din vector cu acelasi numar de litere
*/
typedef struct var_matrix{
  dict* fields;
  int len;
}var_matrix;

/**
 * \return Returneaza distanta dintre 2 cuvinte folosind algoritmul
 * Levenshtein http://en.wikipedia.org/wiki/Levenshtein_distance
 */
int get_dist(char* w1, char* w2, int k)
{
  int i, j, len_1, len_2;
  len_1 = strlen(w1);
  len_2 = strlen(w2);

  for(i = 0; i < len_1+1; i++)
    m[i][0] = i;

  for(i = 0; i < len_2+1; i++)
    m[0][i] = i;

  for(i = 1; i < len_1+1; i++){
    for(j = 1; j < len_2+1; j++){
      if(w1[i-1] == w2[j-1])
        m[i][j] = m[i-1][j-1];
      else{
        m[i][j] = TMIN(m[i-1][j], m[i-1][j-1], m[i][j-1]);
        m[i][j]++;
      }
      /*Pe diagonala distantele un crescatoare, daca a fost depasita distanta k
      algoritmul se intrerupe */
      
      if(i == j && m[i][j] > k+1)
          return MAX_INT;
          
    }
  }
  return m[len_1][len_2];
}

/**
 * \brief functie de comparare pentru quiqsort
 */
int compare(const void *a, const void *b)
{
  dict *x = (dict*)a, *y = (dict*)b;
  return strlen(x->word) - strlen(y->word);
}

/**
 * \return Returneaza un vector de dict si lungimea acestuia prin parametrul
 * @db_len
 */
dict* read_dict(int* db_len, var_matrix **inx)
{
  FILE *f = fopen("dict.txt", "r");
  if(f == NULL){
    perror("Fisierul nu exista");
    return NULL;
  }

  dict* db;
  *inx = (var_matrix*)malloc(sizeof(var_matrix) * 19);

  db = (dict*)malloc(sizeof(dict)*8001);
  *db_len = 0;

  while(fscanf(f, "%s %d", db[(*db_len)].word, &db[*db_len].freq) != -1)
    (*db_len)++;

  qsort(db, *db_len, sizeof(dict), compare);

  /*Asignez pinteri spre primele elemente cu lungime diferita*/
  int i, p = 0, last = 0;
  for(i = 0; i < *db_len; i++){
    if(strlen(db[i].word) != p){
      inx[0][p].len = last;
      last = 0;
      p++;
      inx[0][p].fields = db+i;
    }
    last++;
  }
  fclose(f);
  return db;
}


/**
 * \brief Cauta in dictionar cuvantul cu cea mai mica distanta
 * fata de cuvantul dat.Daca cele 2 sunt egale se ia cuvantul 
  *cu cea mai mare frecventa si in final dupa ordinea lexicografica
 */
int search_word(dict *found_elem, dict *db, char* word, var_matrix *inx, int k)
{
  int i, min_freq = -1, min_dist = k, dist, j, p;
  char* dict_word;
  int len = strlen(word);
  i = MIN(len, max_word_size);
  p = i+1;
  while(i > MAX(0, len - k -1) || p <= MIN(len*2+1, max_word_size)){
    /*De la mijloc spre cap*/
    if(i > 0)
      for(j = 0; j < inx[i].len; j++){
          dict_word = inx[i].fields[j].word;
          dist = get_dist(word, inx[i].fields[j].word, min_dist);

          if(dist <= min_dist){
            if(min_freq <= inx[i].fields[j].freq &&  dist == min_dist)
            {
              if(strcmp(inx[i].fields[j].word, word) < 0 && min_freq == inx[i].fields[j].freq){
                *found_elem = inx[i].fields[j];
                min_dist = dist;
                min_freq = inx[i].fields[j].freq;
              }
              if(min_freq < inx[i].fields[j].freq){
                *found_elem = inx[i].fields[j];
                min_dist = dist;
                min_freq = inx[i].fields[j].freq;
              }
            }
            if(dist < min_dist){

              *found_elem = inx[i].fields[j];
              min_dist = dist;
              min_freq = inx[i].fields[j].freq;
            }
        }
      }
    /*De la mijloc spre coada*/
    if(p <= MIN(len *2, max_word_size))
      for(j = 0; j < inx[p].len; j++){
          dict_word = inx[p].fields[j].word;
          dist = get_dist(word, inx[p].fields[j].word, min_dist);
          
            if(dist <= min_dist){
            if(min_freq <= inx[p].fields[j].freq &&  dist == min_dist)
            {
              if(strcmp(inx[p].fields[j].word, word) < 0 && min_freq == inx[p].fields[j].freq){
                *found_elem = inx[p].fields[j];
                min_dist = dist;
                min_freq = inx[p].fields[j].freq;
              }
              if(min_freq < inx[p].fields[j].freq){
                *found_elem = inx[p].fields[j];
                min_dist = dist;
                min_freq = inx[p].fields[j].freq;
              }
            }
            if(dist < min_dist){
              *found_elem = inx[p].fields[j];
              min_dist = dist;
              min_freq = inx[p].fields[j].freq;
            }
        }
      }

    if(min_dist <= len - i || min_dist < p - len)
      return min_dist;
    i--;
    p++;
  }
  return min_dist;
}

/**
 * \brief afla numarul de cuvinte dinstr-un string care e egal cu numarul de spatii plus 1
*/
int strapp(char* s)
{
  int i, nr = 0;
  for(i = 0; i < strlen(s); i++)
    if(s[i] == ' ')
      nr++;
  return nr+1;
}

/**
  *combina 2 cuvinte din dictionar si frecventele lor
  */
void combine(dict* merged, dict part1, dict part2)
{
  strcpy(merged->word, part1.word);
  strcat(merged->word, " ");
  strcat(merged->word, part2.word);
  merged->freq = part1.freq + part2.freq;
}

/**
 * \brief Genereaza o matrice care are deasupra diagonalei
 * principale distantele petru toate combinatiile dintr-un cuvant
 */
char* split_expr(char* word, dict *db, int db_len, var_matrix *inx)
{
  int **dist_m, **min, i, j, k, len = strlen(word);
  char* substr;

  /*variabila pentru substringuri*/
  substr = (char*)malloc(sizeof(char) * (len+1));

  /*matricea cu minimurile*/
  min = (int**)malloc(sizeof(int*) * len);

  /*matricea cu cuvintele din dictionar*/
  dict **word_min = (dict**)malloc(sizeof(dict*) * len);

  /*Alocarea memoriei si umplerea diagonalei*/
  for(i = 0; i < len; i++){
    min[i] = (int*)malloc(sizeof(int) * len);
    word_min[i] = (dict*)malloc(sizeof(dict) * len);
    strncpy(substr, word+i, 1);
    substr[1] = '\0';
    min[i][i] = search_word(&word_min[i][i], db, substr, inx, 2);
  }



  for(i = 1; i < len; i++){
    for(j = 0; j < len-i; j++){
      strncpy(substr, word+j, i+1);
      substr[i+1] = '\0';

      min[j][j+i] = MAX_INT;
      //aflu minimul dintre posibilitatile in care se poate pune un spatiu intre substringuri
      for(k = j; k < j+i; k++){
        /*Aici aleg minimul distanti apoi numarul minim de cuvinte, apoi frecventa totala maxima si in final ordineaexicografica*/
        if(min[j][k] + min[k+1][j+i] < min[j][j+i]){
          combine(&word_min[j][j+i], word_min[j][k], word_min[k+1][j+i]);
          min[j][j+i] = min[j][k] + min[k+1][j+i];
          continue;
        }
        if(min[j][k] + min[k+1][j+i] == min[j][j+i]){
          if(strapp(word_min[j][k].word) + strapp(word_min[k+1][j+i].word) < strapp(word_min[j][i+j].word)){
            combine(&word_min[j][j+i], word_min[j][k], word_min[k+1][j+i]);
            min[j][j+i] = min[j][k] + min[k+1][j+i];
            continue;
          }
          
          if(strapp(word_min[j][k].word) + strapp(word_min[k+1][j+i].word) == strapp(word_min[j][i+j].word)){
            if(word_min[j][k].freq + word_min[k+1][j+i].freq > word_min[j][i+j].freq){
              combine(&word_min[j][j+i], word_min[j][k], word_min[k+1][j+i]);
              min[j][j+i] = min[j][k] + min[k+1][j+i];
              continue;
            }
            
            if(word_min[j][k].freq + word_min[k+1][j+i].freq == word_min[j][i+j].freq){
              if(strcmp(word_min[j][k].word, word_min[j][j+i].word) < 0){
                combine(&word_min[j][j+i], word_min[j][k], word_min[k+1][j+i]);
                min[j][j+i] = min[j][k] + min[k+1][j+i];
                continue;
              }
            } 
          }
        }//end ifuri filtrare
      }//end for local
      /*daca cuvantul e mai lung decat dublul distantei ceilui mai mare cuvant atunci exista cel putin 2 cuvinte*/
      /*astfel incat distanta sa fie mai mica sau egala cu 18*/
      if(strlen(substr) > max_word_size * 2)
        continue;

      dict aux_word;
      aux_word.word[0] = 0;
      int aux_min;
      
      
      aux_min = search_word(&aux_word, db, substr, inx, min[j][j+i]);
      /*Daca nu a fost gasit un cuvant trece mai departe*/
      if(aux_word.word[0] == 0)
        continue;

        if(aux_min < min[j][j+i]){
          word_min[j][j+i] = aux_word;
          min[j][j+i] = aux_min;
          continue;
        }

        if(aux_min == min[j][j+i]){
          if(strapp(aux_word.word) < strapp(word_min[j][i+j].word)){
            word_min[j][j+i] = aux_word;
            min[j][j+i] = aux_min;
            continue;
          }
          
          if(strapp(aux_word.word) == strapp(word_min[j][i+j].word)){
            if(aux_word.freq > word_min[j][i+j].freq){
              word_min[j][j+i] = aux_word;
              min[j][j+i] = aux_min;
              continue;
            }
            
            if(aux_word.freq == word_min[j][i+j].freq){
              if(strcmp(aux_word.word, word_min[j][j+i].word) < 0){
                word_min[j][j+i] = aux_word;
                min[j][j+i] = aux_min;
                continue;
              }
            } 
          }
        }

    }
  }//end foruri parcurgere principala a matricii
 
  //cuvantul spatiat
  char* splited_word;
  splited_word = (char*)malloc(sizeof(char)* (strlen(word_min[0][len-1].word) +1));
  splited_word[strlen(word_min[0][len-1].word)] = '\0';
  strcpy(splited_word, word_min[0][len-1].word);

  free(substr);

  for(i = 0; i < len; i++){
    free(min[i]);
    free(word_min[i]);
  }
  free(min);
  free(word_min);
  
  return splited_word;
}

/**
 * \brief scoate spatiile dintr-o expresie
 */
char* shrink(char* word){
  int i, j = 0;
  char* shrinked_word;
  shrinked_word = (char*)malloc(sizeof(char) * (strlen(word) + 1));
  for(i = 0; i < (int)strlen(word); i++){
    if(!isspace(word[i])){
      shrinked_word[j] = word[i];
      j++;
    }
  }
  shrinked_word[j] = '\0';
  free(word);
  return shrinked_word;
}

int main(int argc, char** argv)
{
  int db_len;
  var_matrix *inx;
  /*lungimea maxima a unui cuvant din dictionar poate fi aflat si in timp ce citesc dictionarul
  deci nu cred ca se pune ca hard codare*/
  max_word_size = 18;
  dict* db = read_dict(&db_len, &inx);
  char *r;
  r = (char*)malloc(sizeof(char)*65);
  gets(r);
  r = shrink(r);
  char* splited_word = split_expr(r, db, db_len, inx);
  printf("%s\n", splited_word);

  free(db);
  return 0;
}