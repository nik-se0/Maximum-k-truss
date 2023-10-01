#include <iostream>
extern "C" {
#include "graphio.h" 
}
#include "graph.h"
#include "bin_reader.h"
//#include <sys/time.h>
#include <omp.h>
#include <sstream>

#define TIME 0

#define AM 1
#define AI (AM%2+1)%2

#if TIME
static double timer() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) (tp.tv_sec) + tp.tv_usec * 1e-6);
}
#endif

extern int th;

int main(int argc, char* argv[])
{
    //Обработка комадной строки: (Имя файла)
    //std::stringstream convert{ argv[1] };
    //int th; convert >> th;
    char* s1 = argv[1];

    s1 = "C:\\tmp\\m12.mtx";

    //Иницализация графа
    crsGraph gr;
    init_graph(&gr);
    read_mtx_to_crs(&gr, s1);
   
    cout << "\tA graph with " << gr.V << " vertices and " << gr.nz / 2 << " edges is loaded\n\n";
    sort_adj(&gr);

    //Инициализация доп.структур для k-truss
    int* EdgeSupport = new int[gr.nz / 2];      // поддержка ребра 
    Edge* edTo = new Edge[gr.nz / 2];           // массив ребер
    int* eid = new int[gr.nz];                  // id ребра для вершин
    for (long i = 0; i < gr.nz / 2; i++) {
        EdgeSupport[i] = 0;
    }
    getEid(&gr, eid, edTo);


    int q[5] = { 1,2,4,8,16 };
    int q0 = 0;
    while (q0 < 5)
    { th = q[q0];
	
    if (th == 1) 
    { //Послеовательный алгоритм

        cout << "\tSERIAL:\n";
        //Подсчет поддержки
#if TIME
        double triTime = 0;
	    double start =  timer();
#endif
#if AM
        SupAM(&gr, eid, EdgeSupport);         //Маркировка смежности  
#endif
#if AI
        SupAI(&gr, edTo, EdgeSupport);        //Пересечение смежности
#endif
#if TIME
        triTime = timer() - start ;
        cout << "The time support:\t" << (double)triTime << "\n";
        triTime = 0;
        start = timer();
#endif
        //Подсчет k-truss
        
        K_Truss(&gr, EdgeSupport, edTo, eid);
#if TIME
        triTime = timer() - start ;
        cout << "The time K-truss:\t" << (double)triTime << "\n\n";
#endif
        //Результат
        Print_res(gr.nz / 2, EdgeSupport);
    }
    else
    { //Паралленьный алгоритм

        for (long i = 0; i < gr.nz / 2; i++) {
            EdgeSupport[i] = 0;
        }

        cout << "\tPARALLEL " << th << " thread:\n";
        //Подсчет поддержки
#pragma omp barrier
        double itime = omp_get_wtime();
        SupP(&gr, eid, EdgeSupport);
        double ftime = omp_get_wtime();
        double exec_time = ftime - itime;
        cout << "The time support:\t" << exec_time << "\n";
        //Подсчет k-truss
#pragma omp barrier
        double itime1 = omp_get_wtime();
        PK_Truss(&gr, EdgeSupport, edTo, eid);
        double ftime1 = omp_get_wtime();
        double exec_time1 = ftime1 - itime1;
        cout << "The time K-truss:\t" << exec_time1 << "\n\n";

        //Результат
        Print_res(gr.nz / 2, EdgeSupport);

    }

        q0++;
    }
    return 0;
}
