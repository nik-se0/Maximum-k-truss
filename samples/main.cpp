#include <iostream>
extern "C" {
#include "graphio.h" 
}
#include "graph.h"
#include "bin_reader.h"
#include <omp.h>
#include <sstream>

#define SUP 1      //Counting support by marking (1) or counting support by crossing (0)

static double timer() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double)(tp.tv_sec) + tp.tv_usec * 1e-6);
}

extern int th;

int main(int argc, char* argv[])
{
    //Command line processing: (File name)
    char* s1 = argv[1];

    cout << "\n\n\topen file: " << s1 << endl;

    //Graph Initialization
    crsGraph gr;
    init_graph(&gr);
    read_mtx_to_crs(&gr, s1);

    printf("\a");
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


    int TH[5] = { 1,2,4,8,16 };
    int t = 0;
    while (t < 5) //проход по потокам
    {
        th = TH[t];
        for (long i = 0; i < gr.nz / 2; i++) {
            EdgeSupport[i] = 0;
        }

        //Sequential algorithm
        if (th == 1)
        {
            printf("\a");
            cout << "\tSERIAL:\n";
            double triTime = 0;
            double start = timer();
            //Support counting
#if SUP
            SupAM(&gr, eid, EdgeSupport);         //Маркировка смежности
#else
            SupAI(&gr, edTo, EdgeSupport);        //Пересечение смежности
#endif
            triTime = timer() - start;
            cout << "The time support:\t" << (double)triTime << "\n";
            triTime = 0;
            start = timer();
            //K-truss counting
            K_Truss(&gr, EdgeSupport, edTo, eid);

            triTime = timer() - start;
            cout << "The time K-truss:\t" << (double)triTime << "\n\n";
        }
        //Parallel algorithm
        else
        {
            printf("\a");
            cout << "\tPARALLEL " << th << " thread:\n";
#pragma omp barrier
            double itime = omp_get_wtime();
            //Support counting
            SupP(&gr, eid, EdgeSupport);
            double ftime = omp_get_wtime();
            double exec_time = ftime - itime;
            printf("\a");
            cout << "The time support:\t" << exec_time << "\n";
#pragma omp barrier
            double itime1 = omp_get_wtime();
            //K-truss counting
            PK_Truss(&gr, EdgeSupport, edTo, eid);
            double ftime1 = omp_get_wtime();
            double exec_time1 = ftime1 - itime1;
            printf("\a");
            cout << "The time K-truss:\t" << exec_time1 << "\n\n";
        }
        //Output of the calculation result
        printf("\a");
        Print_res(gr.nz / 2, EdgeSupport);
        t++;
    }

return 0;
}
