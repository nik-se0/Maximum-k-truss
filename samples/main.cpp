//  main.c
//  GraphIO
//
//  Created by Andrew Lebedev on 29.09.2020.
//
//#include <time.h>
extern "C" {
#include "graphio.h" 
}
#include "graph.h"

int main() 
{ 
    crsGraph gr;
    
    char* s1 = "mat.mtx";
    char* s2 = "write.mtx";
    init_graph(&gr);
    read_mtx_to_crs(&gr, s1);
    write_crs_to_mtx(&gr, s2);
    sort_adj(&gr);
     
    //Инициализация
    int* EdgeSupport = (int*)calloc(gr.nz / 2, sizeof(int)); // поддержка ребра (для подсчет треугольников)
    for (long i = 0; i < gr.nz / 2; i++) 
    { EdgeSupport[i] = 0;}
    Edge* edTo = (Edge*)malloc((gr.nz / 2) * sizeof(Edge)); // массив ребер
    int* eid = (int*)malloc(gr.nz * sizeof(int));           // id ребра для вершин
    getEid(&gr, eid, edTo);

    //Подсчет поддержки
    SupAM(&gr, eid, EdgeSupport);         //Маркировка смежности 
   //SupAI(gr, edTo, EdgeSupport);        //Пересечение смежности

    //Free memory
   free_graph(&gr);
   if (edTo != NULL)
   { free(edTo);}
   if (EdgeSupport != NULL)
   { free(EdgeSupport);}
         
   return 0;
}
