//
//  graphio.c
//  GraphIO
//
//  Created by Andrew Lebedew on 29.09.2020.
//
#include "graphio.h"

int init_graph(crsGraph* gr) {
    gr -> Adjncy = NULL;
    gr -> Xadj = NULL;
    gr -> Eweights = NULL;
    return 0;
}
int free_graph_pointers(crsGraph* gr) {
    if (!(gr -> Adjncy) || !(gr -> Xadj) || !(gr -> Eweights)) {
        printf("Graph is empty\n");
        return 1;
    }
    free(gr -> Adjncy);
    free(gr -> Xadj);
    free(gr -> Eweights);
    gr -> Adjncy = NULL;
    gr -> Xadj = NULL;
    gr -> Eweights = NULL;
    return 0;
}
int read_mtx_to_crs(crsGraph* gr, const char* filename) {
        
    // variables 
    int N, i, row, col, nz_size, j=0;
    int *edge_num, *last_el;
    double val;
    fpos_t position;
    FILE *file, *file1;

    char line[100000];
    char* s="";
     
    // mtx correctness check 
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Cannot open file\n");
        return 1;
    }
            file1 = fopen(filename, "r");
    if (mm_read_banner(file, &(gr -> matcode))) {
        return 1;
    }
            mm_read_banner(file1, &(gr -> matcode));
    if (mm_read_mtx_crd_size(file, &(gr -> V), &N, &(gr -> nz))) {
        return 1;
    }
            mm_read_mtx_crd_size(file1, &(gr -> V), &N, &(gr -> nz));
    if (mm_is_complex(gr -> matcode) || mm_is_array(gr -> matcode)) {
        printf("Thsis application doesn't support %s", mm_typecode_to_str(gr -> matcode));
        return 1;
    }
    if (N != (gr -> V)) {
        printf("Is not a square matrix\n");
        return 1;
    }
    // Allocating memmory to store adjacency list 
    last_el = (int*)malloc(sizeof(int) * gr -> V);
    edge_num = (int*)malloc(sizeof(int) * gr -> V);
    for (i = 0; i < (gr -> V); i++) {
        edge_num[i] = 0;
    }
    // Saving value of nz so we can change it 
    nz_size = gr -> nz;
    val = 0;  
    
    char str[80];
    fscanf(file1, "%[^\n]", line, sizeof(line));
    j = sscanf(line, "%d %d %lg", &row, &col, &val);
   // j = 2; ///!
    // Reading file to count degrees of each vertex 
    for(i = 0; i < nz_size; i++) {

       if (row == col) 
       { gr->nz--; }
       else
       { row--;
         col--;
         edge_num[row]++;
         if (mm_is_symmetric(gr->matcode)) {
               edge_num[col]++;
               gr->nz++;
           }
       }


       fscanf(file1, "%d %d", &row, &col);
       if (j == 3) { fscanf(file1, "%lg", &val); }
       //j=fscanf(file1, "%d %d %lg", &row, &col, &val);
    }


    // Checking if graph already has arrays 
    if ((gr -> Adjncy != NULL) || (gr -> Xadj != NULL) || (gr -> Eweights != NULL)) {
       free_graph_pointers(gr);
    }
    // Creating CRS arrays 
    gr -> Adjncy = (int*)malloc(sizeof(int) * (gr -> nz));
    gr -> Xadj = (int*)malloc(sizeof(int) * ((gr -> V) + 1));
    gr -> Eweights = (double*)malloc(sizeof(double) * (gr -> nz));

    // Writing data in Xadj and last_el 
    gr -> Xadj[0] = 0;
    for(i = 0; i < gr -> V; i++) {
       gr -> Xadj[i+1] = gr -> Xadj[i] + edge_num[i];
       last_el[i] = gr -> Xadj[i];
    }
    // Reading file to write it's content in crs 
    for(i = 0; i < nz_size; i++) {

       fscanf(file, "%d %d", &row, &col);
       if (j == 3) { fscanf(file, "%lg", &val); }
       //fscanf(file, "%d %d %lg", &row, &col, &val);
       row--;
       col--;
       if (row == col) {
           continue; //we don't need loops
       }
       gr -> Adjncy[last_el[row]] = col;
       gr -> Eweights[last_el[row]] = val;
       last_el[row]++;
       if (mm_is_symmetric(gr -> matcode)) {
           gr -> Adjncy[last_el[col]] = row;
           gr -> Eweights[last_el[col]] = val;
           last_el[col]++;
       }
    }

    free(edge_num);
    free(last_el);
    fclose(file);
    fclose(file1);
    return 0;
}
int read_gr_to_crs(crsGraph* gr, const char* filename) {
    int i, row, col;
    int *edge_num, *last_el;
    double val;
    char sym = 'c';
    char str[101];
    fpos_t position;
    FILE *file;
    
    /* checking if we can read file */
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Cannot open file\n");
        return 1;
    }

    while (sym == 'c') {
        sym = fgetc(file);
        if (sym == 'p') {
            fscanf(file, "%100s %d %d", str, &gr -> V, &gr -> nz);
            fgets(str, sizeof(str), file);
            fgetpos(file, &position);
        } else {
            fgets(str, sizeof(str), file);
        }
    }

    /* Allocating memmory to store adjacency list */
    last_el = (int*)malloc(sizeof(int) * gr -> V);
    edge_num = (int*)malloc(sizeof(int) * gr -> V);
    
    for (i = 0; i < (gr -> V); i++) {
        edge_num[i] = 0;
    }

    while ((sym = fgetc(file)) != EOF) {
        if (sym == 'a') {
            fscanf(file, "%d %d %lg", &row, &col, &val);
            row--;
            col--;
            if (row == col) {
                gr -> nz --; // We don't need loops
            } else {
                edge_num[row]++;
            }
        }
        fgets(str, sizeof(str), file); // Moving to a new line
    }

    /* Checking if graph already has arrays */
    if ((gr -> Adjncy != NULL) || (gr -> Xadj != NULL) || (gr -> Eweights != NULL)) {
       free_graph_pointers(gr);
    }

    /* Creating CRS arrays */
    gr -> Adjncy = (int*)malloc(sizeof(int) * (gr -> nz));
    gr -> Xadj = (int*)malloc(sizeof(int) * ((gr -> V) + 1));
    gr -> Eweights = (double*)malloc(sizeof(double) * (gr -> nz));

    /* Writing data in Xadj and last_el */
    gr -> Xadj[0] = 0;
    for(i = 0; i < gr -> V; i++) {
       gr -> Xadj[i+1] = gr -> Xadj[i] + edge_num[i];
       last_el[i] = gr -> Xadj[i];
    }

    /* Setting right position */
    fsetpos(file, &position);

    /* Reading file to write it's content in crs */
    while ((sym = fgetc(file)) != EOF) {
        if (sym == 'a'){
            fscanf(file, "%d %d %lg", &row, &col, &val);
            row--;
            col--;
            if (row == col) {
                fgets(str, sizeof(str), file);
                continue; //we don't need loops
            }
            gr -> Adjncy[last_el[row]] = col;
            gr -> Eweights[last_el[row]] = val;
            last_el[row]++;
            fgets(str, sizeof(str), file);
        } else {
            fgets(str, sizeof(str), file);
        }
    }

    free(edge_num);
    free(last_el);
    fclose(file);
    return 0;
}
int write_crs_to_mtx(crsGraph* gr, const char* filename) {
    int i,j;
    FILE* f;
    if ((f = fopen(filename, "w")) == NULL) {
        printf("Can't open file\n");
        return 1;
    }
    
    /* Writing banner and size in mtx */
    mm_write_banner(f, gr -> matcode);
    if(mm_is_symmetric(gr -> matcode)) {
        mm_write_mtx_crd_size(f, gr -> V, gr -> V, gr -> nz/2);
    } else {
        mm_write_mtx_crd_size(f, gr -> V, gr -> V, gr -> nz);
    }
    
    for(i = 0; i < gr -> V; i++) {
        for(j = gr -> Xadj[i]; j < gr -> Xadj[i+1]; j++) {
            if (i > gr -> Adjncy[j] || !mm_is_symmetric(gr -> matcode)) {
               //fprintf(f, "%d %d %lg\n", i + 1, gr -> Adjncy[j] + 1, gr -> Eweights[j]);
                fprintf(f, "%d %d\n", i + 1, gr->Adjncy[j] + 1);

            }
        }
    }
    fclose(f);
    return 0;
}
int write_crs_to_mtx_image(crsGraph* gr, const char* filename) {
    int i, j;
    FILE* f;
    if ((f = fopen(filename, "w")) == NULL) {
        printf("Can't open file\n");
        return 1;
    }

    int size = gr->V;
    int** M = (int**)malloc(size * sizeof(int*));
    for (i = 0; i < size; i++) {
        M[i] = (int*)malloc(size * sizeof(int));
    }
    
    
    for (i = 0; i < gr->V; i++) {
        for (j = gr->Xadj[i]; j < gr->Xadj[i + 1]; j++) {
            M[i][gr->Adjncy[j]]= 1;
        }
    }

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
           if(M[i][j]==1) fprintf(f, "1 ");
           else fprintf(f, "0 ");
        }
        fprintf(f,"\n");
    }

    fclose(f);
    return 0;
}
int read_arr_from_bin(double* arr, int size, const char* filename) {
    int result;
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Couldn't opem file\n");
        return 1;
    }
    result = fread(arr, sizeof(double), size, file);
    fclose(file);
    if (result == size) {
        return 0;
    } else {
        printf("Reading error\n");
        return 1;
    }
}
int write_arr_to_bin(double* arr, int size, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file){
        printf("Couldn't opem file\n");
        return 1;
    }
    fwrite(arr, sizeof(double), size, file);
    fclose(file);
    return 0;
}
int write_arr_to_txt(double* arr, int size, const char* filename) {
    int i;
    FILE* file = fopen(filename, "w");
    if (!file){
        printf("Couldn't opem file\n");
        return 1;
    }
    for(i = 0; i < size; i++) {
        fprintf(file, "%lg\n", arr[i]);
    }
    fclose(file);
    return 0;
}



void free_graph(crsGraph* gr)
{
    if (gr->Adjncy != NULL)
        free(gr->Adjncy);

    if (gr->Xadj != NULL)
        free(gr->Xadj);
}
int cmp(const void** i, const void** j)
    {
        return -(*(int*)i[0] - *(int*)j[0]);
    };
int sort_crs(crsGraph* gr)
{
    int i = 0 , x;
    int** dv, *Adj1,*Xadj1, *acc;
    double* Ewe1;
    dv = (int**)malloc(gr->V * sizeof(int*));
    acc= (int*)malloc(sizeof(int) * (gr->V));
    for (i = 0; i < gr->V; i++)
    { dv[i] = (int*)malloc(2 * sizeof(int)); }

    //Подсчет исходящих ребер
    for (i = 0; i < gr->V; i++) 
    { dv[i][0]=gr->Xadj[i+1]-gr->Xadj[i];
      dv[i][1]=i;
    }
    //Подсчет входящих ребер
    for (i = 0; i < gr->nz; i++)
    { dv[gr->Adjncy[i]][0]++; }

    qsort(dv, gr->V, sizeof(int*), cmp);
    for (i = 0; i < gr->V; i++)
    {
        acc[dv[i][1]] = i;
    }

    //Исключение входящих ребер
    for (i = 0; i < gr->nz; i++)
    {
        dv[acc[gr->Adjncy[i]]][0]--;
    }
        
    Adj1 = (int*)malloc(sizeof(int) * (gr->nz));
    Xadj1 = (int*)malloc(sizeof(int) * ((gr->V) + 1));
    Ewe1 = (double*)malloc(sizeof(double) * (gr->nz));

    Xadj1[0] = 0;
    for(i = 0; i < gr -> V; i++) 
    { Xadj1[i+1] = Xadj1[i] + dv[i][0];}
    for(i = 0; i < gr->V; i++) 
    { while(dv[i][0]>0)
      { x = gr->Xadj[dv[i][1]]+dv[i][0]-1;
        Adj1[Xadj1[i]+dv[i][0]-1]=acc[gr->Adjncy[x]];
        Ewe1[Xadj1[i]+dv[i][0]-1]=gr->Eweights[x];
        dv[i][0]--;
      }
    }

    gr->Adjncy = Adj1;
    gr->Eweights = Ewe1;
    gr->Xadj = Xadj1;
 
    return 0;
}
int vid_compare(const void* a, const void* b) { return (*(int*)a - *(int*)b);}
void sort_adj(crsGraph* gr)
{ int i=0;
  for (i = 0; i < gr->V; i++) 
  { qsort(gr->Adjncy + gr->Xadj[i], gr->Xadj[i + 1] - gr->Xadj[i], sizeof(int), vid_compare);}
}