/* SAT-Based Deadlock Detection for Safety Petri Nets */
/* For more info, see our ATVA 2004 paper */
/* written by Tatsuhiro Tsuchiya (16 Jan 2007) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1000
#define MAX_P 1000
#define MAX_T 2000

char linebuf[BUFSIZE];
int matrix[MAX_T+1][MAX_P+1];
/*
  matrix[i][j] 
    = none  if there is no arc between transition i and place j
    = input if place j is an input place of transition j
    = output if place j is an output place of transition j
    = both if place j is an output and input place of transition j
 */
enum type {
  none, input, output, both
};

struct twovalues {
  int right_val, left_val;
}; 

int placeno; /* number of places */
int transno; /* number of transitions */
int stepno;  /* number of steps (k) */

FILE *fp; /* input file */
int flag_d = 0; /* disable preprocessing */
int flag_r = 0; /* remove unfirable transitions */

int c[MAX_P+1];

void initialize(void);
void get_line(void);
struct twovalues get_two_ids(void);
void read_file(void);
void print_status(void);
void generate_formula(void);
void generate_nondeadlock(void);


void preprocessing(void);
/* variables for preprocessing */
char initial_marking[MAX_P+1]; 
char visited[MAX_P+1];
int  done[MAX_T];
int  done_size = 0;


int main(int argc, char *argv[])
{
  int i;

  if (argc < 2) {
    fprintf(stderr, "usage: %s {-d -r -h} k [input_file]\n", argv[0]);
    exit(-1);
  }

  for(i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-h")) {
      printf("%s [-d] [-r] [-h] k [input file]\n", argv[0]); 
      printf("  -d: disable preprocessing\n"); 
      printf("  -r: further optimization (discard unfirable transitions)\n"); 
      printf("      valid only when dp is enabled\n"); 
      printf("  -h: help\n"); 
      exit(-1);
    }
    if (!strcmp(argv[i], "-d")) {
      flag_d = 1; 
      continue; 
    }
    if (!strcmp(argv[i], "-r")) {
      flag_r = 1; 
      continue; 
    }
    break;
  }

  if (i == argc) {
    fprintf(stderr, "usage: %s {-d -r -h} k [input_file]\n", argv[0]);
    exit(-1);
  }

  stepno = atoi(argv[i]); /* the value of k */
  if (stepno < 0) {
    fprintf(stderr, "k must be equal to or greater than 0.\n");
    exit(-1);
  }

  fp = stdin; 
  if (++i < argc) {
    if ((fp = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, "%s cannot be opened.\n", argv[i]);
      exit(-1);
    }
  }

  initialize();
  read_file();

  /* print_status(); */

  preprocessing();
  
  generate_formula(); 
  generate_nondeadlock();

  return 0;
}

void initialize()
{
  int i, j;
  for (i=0; i <= MAX_T; i++)
    for (j=0; j <= MAX_P; j++)
      matrix[i][j] = none;

  placeno = transno = 0;

  for (i = 0; i <= MAX_P; i++)
    initial_marking[i] = 0; 
  for (i = 0; i <= MAX_P; i++)
    visited[i] = 0; 
  for (i = 0; i < MAX_T; i++)
    done[i] = 0;
}

void get_line()
{
  int lf;
  
  if (fgets(linebuf, sizeof(linebuf), fp) == NULL) {
    fprintf(stderr, "error in input file\n");
    exit(1);
  }
  if (linebuf[(lf = strlen(linebuf) - 1)] != '\n') {
    fprintf(stderr, "too long line\n");
    exit(1);
  }
  linebuf[lf]  = '\0'; 
}

struct twovalues get_two_ids()
{
  int i = 0;
  int j = 0;

  char buffer[BUFSIZE];
  struct twovalues result;

  while((linebuf[i] != '<') && (linebuf[i] != '>')) {
      buffer[i] = linebuf[i];
      i++;
    }
  buffer[i] = '\0';
  result.left_val = atoi(buffer);
  
  i++; j = 0;
  while((linebuf[i] >= '0') && (linebuf[i] <= '9')) {
      buffer[j] = linebuf[i];
      i++; j++;
  }
  buffer[j] = '\0';
  result.right_val = atoi(buffer);

  return result;
}

void read_file() /* read input file and generate initial marking */
{
  int count = 0;
  struct twovalues two_ids;
  
  while (1) {
    get_line();
    if (strcmp(linebuf, "PL") == 0)
      break;
  }
 
  while (1) {
    int flag = 0;

    get_line();
    if (linebuf[0] != '"')
      break;

    if(++placeno > MAX_P) {
      fprintf(stderr, "too many places\n");
      exit(1);
    }
    /* go to the next quotation */
    {
      int i = 0;
      while (linebuf[++i] != '"');

      while (linebuf[++i] != 'M') 
	if(linebuf[i] == '\0')
	  break; 
      if ((linebuf[i] == 'M') && (linebuf[i+1] == '1'))
	/* initially marked */
	{
          initial_marking[placeno] = 1;
	  if (count == 0)
	    printf("(s%d_0", placeno);
	  else
	    printf(" & s%d_0", placeno);
	  count++;
	  if (++count > 10) { putchar('\n'); count = 1; }
	  flag = 1;
	}
    }
    if (flag != 1) /* not initially marked */
      {
	if (count == 0)
	  printf("(!s%d_0", placeno);
	else
	  printf(" & !s%d_0", placeno);
	if (++count > 10) { putchar('\n'); count = 1; }
      }
  }
  printf(")\n");

  while (1) {
    if (strcmp(linebuf, "TR") == 0)
      break;
    get_line();
  }

  while (1) {
    get_line();
    if (linebuf[0] != '"')
      break;
    transno++;
  }

  while (1) {
    if (strcmp(linebuf, "TP") == 0)
      break;
    get_line();
  }

  while (1) {
    get_line();
    if ((linebuf[0] < '0') || (linebuf[0] > '9'))
      break;

    two_ids = get_two_ids();
    matrix[two_ids.left_val][two_ids.right_val] = output;
  }

  while (1) {
    if (strcmp(linebuf, "PT") == 0)
      break;
    get_line();
  }

  while (1) {
    get_line();
    if ((linebuf[0] < '0') || (linebuf[0] > '9'))
      break;
    two_ids = get_two_ids();

    if (matrix[two_ids.right_val][two_ids.left_val] == none)
      matrix[two_ids.right_val][two_ids.left_val] = input;
    else
      matrix[two_ids.right_val][two_ids.left_val] = both;
  }

}

void print_status(void)
{
  int i,j;

  printf("no of places: %d\n", placeno);
  printf("no of transitions: %d\n", transno);
  for(i = 1; i <= transno; i++) {
    printf("transition %d:", i);
    printf("\ninput :");
    for(j = 1; j <= placeno; j++) 
      if(matrix[i][j] == input)
	printf("%d, ", j);
    printf("\noutput :");
    for(j = 1; j <= placeno; j++) 
      if(matrix[i][j] == output)
	printf("%d, ", j);
    printf("\nboth :");
    for(j = 1; j <= placeno; j++) 
      if(matrix[i][j] == both)
	printf("%d, ", j);
    printf("\n");
  }
}

void generate_formula()
{
  int a, b, d;

  for(a = 1; a <= placeno; a++)
    c[a] = 0;
  
  for(a = 1; a <= stepno; a++) {
    for(b = 0; b < transno; b++) {
      int first = 1;

      for(d = 1; d <= placeno; d++)
	if((matrix[done[b]][d] == input)||(matrix[done[b]][d] == both)) {
	  if (first == 1) {
	    printf("& ((");
	    printf("s%d_%d", d, c[d]);
	    first = 0;
	  }
	  else
	    printf(" & s%d_%d", d, c[d]);
	}

      for(d = 1; d <= placeno; d++)
	if(matrix[done[b]][d] == input) {
	  if (first == 1) {
	    printf("& (");
	    printf("!s%d_%d", d, (a-1) * transno + b + 1);
	    first = 0;
	  }
	  else
	    printf(" & !s%d_%d", d, (a-1) * transno + b + 1);
	}

      for(d = 1; d <= placeno; d++)
	if((matrix[done[b]][d] == output)||(matrix[done[b]][d] == both)) {
	  if (first == 1) {
	    printf("& (");
	    printf("s%d_%d", d, (a-1) * transno + b + 1);
	    first = 0;
	  }
	  else
	    printf(" & s%d_%d", d, (a-1) * transno + b + 1);
	}

      if (first != 1)
	printf(")");

      first =1 ;
      for(d = 1; d <= placeno; d++)
	if(matrix[done[b]][d] != none) {
	  if (first == 1) {
	    printf(" | (");
	    printf("(s%d_%d = s%d_%d)",
		   d, c[d], d, (a-1) * transno + b + 1);
	    first = 0;
	  }
	  else
	    printf(" & (s%d_%d = s%d_%d)",
		   d, c[d], d, (a-1) * transno + b + 1);
	}

      for(d = 1; d <= placeno; d++)
	if(matrix[done[b]][d] != none)
	  c[d] = (a-1) * transno + b + 1;

      printf("))\n");
    }
  }
}

/* Correction is called for to allow input transitions */
void generate_nondeadlock()
{
  int b, d;
  int first = 1;
  int first_tr;

  printf(" & (!(\n");
  for(b = 0; b < transno; b++) {
    if (first == 1) {
      printf("(");
      first = 0;
    }
    else
      printf("| (");

    first_tr = 1;
    for(d = 1; d <= placeno; d++)
      if((matrix[done[b]][d] == input)||(matrix[done[b]][d] == both)) {
	if (first_tr == 1) {
	  printf("s%d_%d", d, c[d]);
	  first_tr = 0;
	}
	else
	  printf(" & s%d_%d", d, c[d]);
      }

      printf(")\n");
  }
  printf("))\n");
}


/*  pre processing */

void preprocessing(void)
{
  void visit_place(int);
  void fill_unordered_transitions(void);
  int i;

  if (flag_d) {  /* disable preprocesisng */
    for (i = 0; i < MAX_T; i++)
      done[i] = i + 1;
    return;
  }

  for (i = 1; i <= placeno; i++)  {
    if (!initial_marking[i])
      continue;
    if (visited[i])
      continue;
    visit_place(i);
  }
  
  if (!flag_r)
    fill_unordered_transitions(); 
 
}

void visit_place(int place)
{
  int flag, i; 
  int t, p; 

//  fprintf(stderr, "p%d:\n", place); 
  visited[place] = 1;  // add p to Visited
  for (t = 1; t <= transno; t++) { // for all p's output transition t 
    if (!((matrix[t][place] == input) | (matrix[t][place] == both))) 
      continue;
    flag = 0; 
//    fprintf(stderr, "\tt%d:\n", t); 
    
    for (i = 0; i < done_size; i++) {    // if t is not in DONE
      if (done[i] == t) {
        flag = 1; break;
      }
    }
    if (flag) continue; 

    for (p = 1; p <= placeno; p++) { // if all t's input places are in VISITED 
      if (!((matrix[t][p] == input) | (matrix[t][p] == both))) 
        continue;

      if (!visited[p]) {
        flag = 1; break;
      }
    }
    if (flag) continue;

//    fprintf(stderr, "\t\tt%d:\n", t);     
    done[done_size++] = t; // enque t to DONE
    
    for(p = 1; p <= placeno; p++) { // for p: p is a t's output place and not in VISITED
      if (!((matrix[t][p] == output) | (matrix[t][p] == both)))
        continue;
      if(visited[p])
        continue;
      visit_place(p);
    }
  }
  return; 
}

// The transitions unreachable in the search performed 
// by the preprocessing cannot be fired.   
// Those transitions can be safely ignored. 
// At this moment however we do not have proof for this,  
// and thus we do not discard them. 
// If you want to do so, please use the -r option. 
void fill_unordered_transitions(void)
{
  int t, flag, i;
  for(t = 1; t <= transno; t++) {
    flag = 0;
    for (i = 0; i < done_size; i++) {
      if (t == done[i]) {
        flag = 1;
        break;
      }
    }
    if (!flag)
      done[done_size++] = t;
  }
}
