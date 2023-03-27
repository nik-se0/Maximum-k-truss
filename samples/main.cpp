#include <iostream>
extern "C" {
#include "graphio.h" 
}
#include "graph.h"
#include <chrono>
#include <omp.h>
#include <sstream>

#define AM 1
#define AI (AM%2+1)%2

int main(int argc, char* argv[])
{
    //��������� �������� ������: (���-�� �������) (��� �����)
    std::stringstream convert{ argv[1] };
    int th; convert >> th;
    char* s1 = argv[2];

    //������������ �����
    crsGraph gr;
    init_graph(&gr);
    read_mtx_to_crs(&gr, s1);
    cout << "\tA graph with " << gr.V << " vertices and " << gr.nz / 2 << " edges is loaded\n\n";
    sort_adj(&gr);

    //������������� ���.�������� ��� k-truss
    int* EdgeSupport = new int[gr.nz / 2];      // ��������� ����� 
    Edge* edTo = new Edge[gr.nz / 2];           // ������ �����
    int* eid = new int[gr.nz];                  // id ����� ��� ������
    for (long i = 0; i < gr.nz / 2; i++) {
        EdgeSupport[i] = 0;
    }
    getEid(&gr, eid, edTo);

    if (th == 1) 
    { //��������������� ��������

        cout << "\tSERIAL:\n";
        //������� ���������
        auto begin = std::chrono::steady_clock::now();
#if AM
        SupAM(&gr, eid, EdgeSupport);         //���������� ���������  
#endif
#if AI
        SupAI(&gr, edTo, EdgeSupport);        //����������� ���������
#endif
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
        cout << "The time support:\t" << (double)elapsed_ms.count() / 1000000 << "\n";

        //������� k-truss
        auto begin1 = std::chrono::steady_clock::now();
        K_Truss(&gr, EdgeSupport, edTo, eid);
        auto end1 = std::chrono::steady_clock::now();
        auto elapsed_ms1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1);
        cout << "The time K-truss:\t" << (double)elapsed_ms1.count() / 1000000 << "\n\n";

        //���������
        Print_res(gr.nz / 2, EdgeSupport);
    }
    else
    { //������������ ��������

        for (long i = 0; i < gr.nz / 2; i++) {
            EdgeSupport[i] = 0;
        }

        cout << "\tPARALLEL " << th << " thread:\n";
        //������� ���������
#pragma omp barrier
        double itime = omp_get_wtime();
        SupP(&gr, eid, EdgeSupport, th);
        double ftime = omp_get_wtime();
        double exec_time = ftime - itime;
        cout << "The time support:\t" << exec_time << "\n";
        //������� k-truss
#pragma omp barrier
        double itime1 = omp_get_wtime();
        PK_Truss(&gr, EdgeSupport, edTo, eid);
        double ftime1 = omp_get_wtime();
        double exec_time1 = ftime1 - itime1;
        cout << "The time K-truss:\t" << exec_time1 << "\n\n";

        //���������
        Print_res(gr.nz / 2, EdgeSupport);

    }

    return 0;
}
