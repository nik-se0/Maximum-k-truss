#include <cstdlib>
#include "graphio.h"

// Формат CRS (по Matrix Market):
// m - число строк в матрице
// n - число столбцов в матрице
// nz - число ненулевых элементов в матрице
// matcode - массив char[4] с информацией о матрице (как в Matrix Market)
// rowstart - массив int[m+1] индексов начала строк матрицы,
//     (m+1)-й элемент ограничивает последнюю строку
// column - массив int[nz] столбцов у каждого элемента матрицы
// values - массив ValType[nz] значений элементов матрицы

// Формат BIN, порядок хранения данных:
// 1) char matcode[4]
// 2) size_t m
// 3) size_t n
// 4) size_t nz
// 5) int* rowstart   (число элементов - m+1)
// 6) int* column     (число элементов - nz)
// 7) ValType* values (число элементов - nz)

// !!!ATTENTION!!!
// 1) Нужно заранее знать тип значений матрицы, 
// чтобы передать '*values' правильного типа. Тип значений матрицы
// лежит в matcode[2], так что его надо считать заранее.
// 'I' - int, 'R' - double.
// 2) В функции выделяется память под массивы - это нужно учитывать,
// чтобы не было утечек памяти.

// Чтение графа CRS в бинарном формате из файла
int read_bin_to_crs(crsGraph* gr, const char* filename){

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return -1;
   
    size_t m_sizet, n_sizet, nz_sizet;
    
    fread(gr->matcode, 1, 1, fp);
    fread(gr->matcode + 1, 1, 1, fp);
    fread(gr->matcode + 2, 1, 1, fp);
    fread(gr->matcode + 3, 1, 1, fp);
    fread(&gr->V, sizeof(size_t), 1, fp);
    fread(&gr->V, sizeof(size_t), 1, fp);
    fread(&gr->nz, sizeof(size_t), 1, fp); 
    
    gr->Xadj = new int[gr->V+1];
    gr->Adjncy= new int[gr->nz];
    gr->Eweights = new double[gr->nz];
    
    fread(gr->Xadj, sizeof(int), gr->V + 1, fp);
    fread(gr->Adjncy, sizeof(int), gr->nz, fp);
    fread(gr->Eweights, sizeof(double), gr->nz, fp);
    
    fclose(fp);
    return 0;
}

// Запись графа CRS в память в бинарном формате
int write_crs_to_bin(crsGraph* gr, const char *filename ) {
      //  char* gr->matcode, int* rowstart, int* column,
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
        return -1;
    
    size_t m_sizet = (size_t)gr->V, 
           n_sizet = (size_t)gr->V,
           nz_sizet = (size_t)gr->nz;
    
    fwrite(gr->matcode, 1, 1, fp);
    fwrite(gr->matcode + 1, 1, 1, fp);
    fwrite(gr->matcode + 2, 1, 1, fp);
    fwrite(gr->matcode + 3, 1, 1, fp);
    fwrite(&m_sizet, sizeof(size_t), 1, fp);
    fwrite(&n_sizet, sizeof(size_t), 1, fp);
    fwrite(&nz_sizet, sizeof(size_t), 1, fp);
    fwrite(gr->Xadj, sizeof(int), gr->V+1, fp);
    fwrite(gr->Adjncy, sizeof(int), gr->nz, fp);
    fwrite(gr->Eweights, sizeof(double), gr->nz, fp);

    fclose(fp);
    return 0;
}