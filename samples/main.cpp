#include <iostream>
extern "C" {
#include "graphio.h" 
}
#include "graph.h"
#include "bin_reader.h"
#include <sys/time.h>
#include <omp.h>
#include <sstream>

#define SUP 1      //Counting support by marking (1) or counting support by crossing (0)
#define TIME 0      //Mark the time
extern int th;

static double timer() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)(tp.tv_sec) + tp.tv_usec * 1e-6);
}

int main(int argc, char* argv[])
{
    //Command line processing: (File name, number of threads)
    std::stringstream convert{ argv[2] };
    convert >> th;
    char* s1 = argv[1];

    //Graph Initialization
    crsGraph gr;
    init_graph(&gr);
    read_mtx_to_crs(&gr, s1);

    cout << "\tA graph with " << gr.V << " vertices and " << gr.nz / 2 << " edges is loaded\n\n";
    sort_adj(&gr);

    //Initialization of additional structures for k-truss
    int* EdgeSupport = new int[gr.nz / 2];      //edge support 
    Edge* edTo = new Edge[gr.nz / 2];           //array of edges
    int* eid = new int[gr.nz];                  //edge id for vertices
    for (long i = 0; i < gr.nz / 2; i++) {
        EdgeSupport[i] = 0;
    }
    getEid(&gr, eid, edTo);

    //Sequential algorithm
    if (th == 1)
    {   cout << "\tSERIAL:\n";
    
#if TIME
        double triTime = 0;
        double start = timer();
#endif
    //Support counting
#if SUP
        SupAM(&gr, eid, EdgeSupport);         //Маркировка смежности  
#else
        SupAI(&gr, edTo, EdgeSupport);        //Пересечение смежности
#endif
#if TIME
        triTime = timer() - start;
        cout << "The time support:\t" << (double)triTime << "\n";
        triTime = 0;
        start = timer();
#endif
    //K-truss counting
      K_Truss(&gr, EdgeSupport, edTo, eid);
#if TIME
        triTime = timer() - start;
        cout << "The time K-truss:\t" << (double)triTime << "\n\n";
#endif
    }
    //Parallel algorithm
    else 
    {    cout << "\tPARALLEL " << th << " thread:\n";
#if TIME
#pragma omp barrier
        double itime = omp_get_wtime();
#endif
        //Support counting
        SupP(&gr, eid, EdgeSupport);
#if TIME
        double ftime = omp_get_wtime();
        double exec_time = ftime - itime;
        cout << "The time support:\t" << exec_time << "\n";
#pragma omp barrier
        double itime1 = omp_get_wtime();
#endif
        //K-truss counting
        PK_Truss(&gr, EdgeSupport, edTo, eid);
#if TIME
        double ftime1 = omp_get_wtime();
        double exec_time1 = ftime1 - itime1;
        cout << "The time K-truss:\t" << exec_time1 << "\n\n";
#endif
    }
    //Output of the calculation result
    Print_res(gr.nz / 2, EdgeSupport);
    return 0;
}
