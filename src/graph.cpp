#pragma once
#include "omp.h"
#include "graph.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#define distribution_S static //dynamic,static, chunk
#define distribution_K static //dynamic,static, chunk
using namespace std;
int th;
int chunk;

//Results
void Print_res(int nE, int* EdgeSupport)
{
    int maxSup = 0;
    for (int i = 0; i < nE; i++) {
        if (maxSup < EdgeSupport[i]) {
            maxSup = EdgeSupport[i];
        }
    }

    int numMax = 0;
    for (int i = 0; i < nE; i++) {
        if (EdgeSupport[i] == maxSup) {
            numMax++;
        }
    }

    printf("\tMax-truss: %d\n#Edges in Max-truss: %ld\n\n", maxSup + 2, numMax);
}
//Initialization of additional structures
void getEid(crsGraph* gr, int* eid, Edge* edTo)
{
    int* X = new int[gr->V];
    for (int i = 0; i < gr->V; i++) {
        X[i] = gr->Xadj[i];
    }

    int i = 0;
    for (int u = 0; u < gr->V; u++) {
        for (int j = gr->Xadj[u]; j < gr->Xadj[u + 1]; j++) {
            int v = gr->Adjncy[j];
            if (u < v) {
                Edge e;
                e.u = u;
                e.v = v;
                eid[j] = i;
                X[u]++;
                if (gr->Adjncy[X[v]] == u) {
                    eid[X[v]] = i;
                    X[v]++;
                }
                edTo[i] = e;
                i++;
            }
        }
    }

    delete[] X;
}


//Sequential algorithm
//Counting support
void SupAM(crsGraph* gr, int* eid, int* EdgeSupport)
{
    int nV = gr->V;
    int nE = gr->nz / 2;
    int* start = new int[nV];
    int* X = new int[gr->V];
    for (int i = 0; i < gr->V; i++) {
        X[i] = 0;
    }

    for (int i = 0; i < nV; i++)
    {
        int j = gr->Xadj[i];
        int nV = gr->Xadj[i + 1];
        while (j < nV && gr->Adjncy[j] < i) {
            j++;
        }
        start[i] = j;
    }

    for (int u = 0; u < nV; u++) {
        for (int j = start[u]; j < gr->Xadj[u + 1]; j++) {
            int w = gr->Adjncy[j];
            X[w] = j + 1;
        }
        for (int j = gr->Xadj[u]; j < start[u]; j++) {
            int v = gr->Adjncy[j];
            for (int k = gr->Xadj[v + 1] - 1; k >= start[v]; k--) {
                int w = gr->Adjncy[k];
                if (w <= u) { break; }
                if (X[w]) {
                    int e1 = eid[X[w] - 1], e2 = eid[j], e3 = eid[k];
                    EdgeSupport[e1] += 1;
                    EdgeSupport[e2] += 1;
                    EdgeSupport[e3] += 1;
                }
            }
        }
        for (int j = start[u]; j < gr->Xadj[u + 1]; j++) {
            int w = gr->Adjncy[j];
            X[w] = 0;
        }
    }

    delete[] start;
    delete[] X;
}
void SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport)
{
    int nE = gr->nz / 2;
    int* X = new int[gr->V];
    for (int i = 0; i < gr->V; i++) {
        X[i] = (nE + 1);
    }

    for (int e = 0; e < nE; e++)
    {
        Edge edge = edTo[e];
        int sup = 0;
        int u = edge.u;
        int v = edge.v;
        for (int j = gr->Xadj[u]; j < gr->Xadj[u + 1]; j++) {
            int w = gr->Adjncy[j];
            if (w != v) {
                X[w] = e;
            }
        }
        for (int j = gr->Xadj[v]; j < gr->Xadj[v + 1]; j++) {
            int w = gr->Adjncy[j];
            if (w != u) {
                if (X[w] == e) {
                    sup++;
                }
            }
        }
        EdgeSupport[e] = sup;
    }

    delete[] X;
}
//Main algorithm
void Curr_init(int k_level, int nE, int& Tail, int* EdgeSupport, int* curr)
{
    for (int i = 0; i < nE; i++) {
        if (EdgeSupport[i] == k_level) {
            curr[Tail] = i;
            Tail = Tail + 1;
        }
    }
}
void SubLevel(crsGraph* gr, int* curr, int Tail, int* EdgeSupport, int k_level, int* next, int& nextTail, bool* flag, Edge* edTo, int* eid)
{
    for (int i = 0; i < Tail; i++) {
        int e1 = curr[i];
        Edge edge = edTo[e1];
        int u = edge.u;
        int v = edge.v;
        int uStart = gr->Xadj[u], uEnd = gr->Xadj[u + 1];
        int vStart = gr->Xadj[v], vEnd = gr->Xadj[v + 1];
        unsigned int nV = (uEnd - uStart) + (vEnd - vStart);
        int j = uStart, k = vStart;

        for (int count = 0; count < nV; count++) {
            if (j >= uEnd) { break; }
            else if (k >= vEnd) { break; }
            else if (gr->Adjncy[j] == gr->Adjncy[k]) {
                int e2 = eid[k];  //(v,w)
                int e3 = eid[j];  //(u,w)
                if ((!flag[e2]) && (!flag[e3])) { 
                    int* E = new int[4];
                    E[0] = EdgeSupport[e2] - k_level;
                    E[1] = EdgeSupport[e3] - k_level;
                    E[2] = e2; E[3] = e3;
                    int q = 0;
                    while (q < 2)
                    {
                        if (E[q] > 0) {
                            EdgeSupport[E[q + 2]] = EdgeSupport[E[q + 2]] - 1;
                            if (EdgeSupport[E[q + 2]] == k_level)
                            {
                                next[nextTail] = E[q + 2];
                                nextTail = nextTail + 1;
                            }
                        } q++;
                    }
                    delete[] E;
                }
                j++; k++;
            }
            else if (gr->Adjncy[j] < gr->Adjncy[k]) { j++; }
            else if (gr->Adjncy[k] < gr->Adjncy[j]) { k++; }
        }
        flag[e1] = true;
    }


}
int K_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid)
{
    int nE = gr->nz / 2;
    int nV = gr->V;
    int Tail = 0;
    int nextTail = 0;
    int* curr = new int[nE];
    int* next = new int[nE];
    bool* flag = new bool[nE];
    for (int e = 0; e < nE; e++) {
        flag[e] = false;
    }

    int k_level = 0;
    int todo = nE;
    while (todo > 0) {
        Curr_init(k_level, nE, Tail, EdgeSupport, curr);
        while (Tail > 0) {
            todo = todo - Tail;
            SubLevel(gr, curr, Tail, EdgeSupport, k_level, next, nextTail, flag, edTo, eid);
            int* tempCurr = curr;
            curr = next;
            next = tempCurr;
            Tail = nextTail;
            nextTail = 0;
        }
        k_level = k_level + 1;
    }

    delete[] curr;
    delete[] next;
    delete[] flag;

    return k_level;
}


//Parallel algorithm
void SupP(crsGraph* gr, int* eid, int* EdgeSupport) {
#pragma omp parallel num_threads(th)
    {
        long nV = gr->V;
        long nE = gr->nz / 2;
        int* startEd = new int[nV];
        int* X = new int[gr->V];
        int* BufSup = new int[nE];
        for (long i = 0; i < nE; i++) {
            BufSup[i] = 0;
        }
        for (long i = 0; i < nV; i++) {
            X[i] = 0;
        }

#pragma omp for schedule(distribution_S) 
        for (int i = 0; i < nV; i++) {
            X[i] = 0;
            int j = gr->Xadj[i];
            int nV = gr->Xadj[i + 1];
            while (j < nV && gr->Adjncy[j] < i) {
                j++;
            }
            startEd[i] = j;
        }

        //Подсчет поддержки
#pragma omp for schedule(distribution_S)     
        for (int u = 0; u < nV; u++) {
            for (int j = startEd[u]; j < gr->Xadj[u + 1]; j++) {
                int w = gr->Adjncy[j];
                X[w] = j + 1;
            }
            for (int j = gr->Xadj[u]; j < startEd[u]; j++) {
                int v = gr->Adjncy[j];
                for (int k = gr->Xadj[v + 1] - 1; k >= startEd[v]; k--) {
                    int w = gr->Adjncy[k];
                    if (w <= u) { break; }
                    if (X[w]) {
                        int e1 = eid[X[w] - 1], e2 = eid[j], e3 = eid[k];
                        BufSup[e1]++; BufSup[e2]++; BufSup[e3]++;
                    }
                }
            }

            for (int j = startEd[u]; j < gr->Xadj[u + 1]; j++) {
                int w = gr->Adjncy[j];
                X[w] = 0;
            }
        }
#pragma omp barrier
        for (int i = 0; i < nE; i++)
        {
            if (BufSup[i] != 0) {
#pragma omp atomic
                EdgeSupport[i] += BufSup[i];
            }
        }
#pragma omp barrier
        delete[] startEd;
        delete[] X;
        delete[] BufSup;
    }
}
void PCurr_init(int nE, int* EdgeSupport, int k_level, int* curr, int& Tail, bool* InCurr)
{
    const int BUFFER_SIZE_BYTES = 2048;
    const int BUFFER_SIZE = BUFFER_SIZE_BYTES / sizeof(int);
    int buff[BUFFER_SIZE];
    int index = 0;

#pragma omp for schedule(distribution_K) 
    for (int i = 0; i < nE; i++) {
        if (EdgeSupport[i] == k_level) {
            buff[index] = i;
            InCurr[i] = true;
            index++;

            if (index >= BUFFER_SIZE) {
                int tmp;
#pragma omp atomic capture 
                { tmp = Tail;
                Tail += BUFFER_SIZE;
                }

                for (int j = 0; j < BUFFER_SIZE; j++) {
                    curr[tmp + j] = buff[j];
                }
                index = 0;
            }
        }
    }

    if (index > 0) {
        int tmp;
#pragma omp atomic capture 
        { tmp = Tail;
        Tail += index;
        }
        for (int j = 0; j < index; j++) {
            curr[tmp + j] = buff[j];
        }
    }

#pragma omp barrier

}
void PSubLevel(crsGraph* gr, int* curr, bool* InCurr, int Tail, int* EdgeSupport, int k_level, int* next, bool* InNext, int& nextTail, bool* flag, Edge* edTo, int* eid)
{//Size of cache line
    const int BUFFER_SIZE_BYTES = 2048;
    const int BUFFER_SIZE = BUFFER_SIZE_BYTES / sizeof(int);

    int buff[BUFFER_SIZE];
    int index = 0;

#pragma omp for schedule(distribution_K) 
    for (int i = 0; i < Tail; i++) {

        int e1 = curr[i];
        Edge edge = edTo[e1];
        int u = edge.u;
        int v = edge.v;
        int uStart = gr->Xadj[u], uEnd = gr->Xadj[u + 1];
        int vStart = gr->Xadj[v], vEnd = gr->Xadj[v + 1];
        int nV = (uEnd - uStart) + (vEnd - vStart);
        int j = uStart, k = vStart;

        for (int count = 0; count < nV; count++) {
            if (j >= uEnd) {
                break;
            }
            else if (k >= vEnd) {
                break;
            }
            else if (gr->Adjncy[j] == gr->Adjncy[k]) {
                int e2 = eid[k];
                int e3 = eid[j];
                if ((!flag[e2]) && (!flag[e3])) {
                    if (EdgeSupport[e2] > k_level && EdgeSupport[e3] > k_level) {
                        int* E = new int[2];
                        E[0] = e2; E[1] = e3;
                        int q = 0;
                        while (q < 2) {
                            int sup;
#pragma omp atomic capture 
                            { sup = EdgeSupport[E[q]];
                            EdgeSupport[E[q]] -= 1;
                            }
                            if (sup - 1 == k_level) {
                                buff[index] = E[q];
                                InNext[E[q]] = true;
                                index++;
                            }
                            if (sup <= k_level) {
#pragma omp atomic 
                                EdgeSupport[E[q]] += 1;
                            }
                            if (index >= BUFFER_SIZE) {
                                int tmp;
#pragma omp atomic capture 
                                { tmp = nextTail;
                                nextTail += BUFFER_SIZE;
                                }
                                for (int y = 0; y < BUFFER_SIZE; y++) {
                                    next[tmp + y] = buff[y];
                                }
                                index = 0;
                            }
                            q++;
                        }
                        delete[] E;
                    }
                    else if (EdgeSupport[e2] > k_level) {
                        if ((e1 < e3 && InCurr[e3]) || !InCurr[e3]) {
                            int supE2;
#pragma omp atomic capture 
                            { supE2 = EdgeSupport[e2];
                            EdgeSupport[e2] -= 1;
                            }
                            if (supE2 == (k_level + 1)) {
                                buff[index] = e2;
                                InNext[e2] = true;
                                index++;
                            }

                            if (supE2 <= k_level) {
#pragma omp atomic 
                                EdgeSupport[e2] -= 1;

                            }

                            if (index >= BUFFER_SIZE) {
                                int tmp;
#pragma omp atomic capture 
                                { tmp = nextTail;
                                nextTail += BUFFER_SIZE;
                                }

                                for (int y = 0; y < BUFFER_SIZE; y++)
                                    next[tmp + y] = buff[y];
                                index = 0;
                            }
                        }
                    }
                    else if (EdgeSupport[e3] > k_level) {
                        if ((e1 < e2 && InCurr[e2]) || !InCurr[e2]) {
                            int supE3;
#pragma omp atomic capture 
                            { supE3 = EdgeSupport[e3];
                            EdgeSupport[e3] -= 1;
                            }
                            if (supE3 == (k_level + 1)) {
                                buff[index] = e3;
                                InNext[e3] = true;
                                index++;
                            }

                            if (supE3 <= k_level) {
#pragma omp atomic
                                EdgeSupport[e3] -= 1;
                            }

                            if (index >= BUFFER_SIZE) {
                                int tmp;
#pragma omp atomic capture 
                                { tmp = nextTail;
                                nextTail += BUFFER_SIZE;
                                }
                                for (int y = 0; y < BUFFER_SIZE; y++)
                                    next[tmp + y] = buff[y];
                                index = 0;
                            }
                        }
                    }

                }


                j++;
                k++;
            }
            else if (gr->Adjncy[j] < gr->Adjncy[k]) {
                j++;
            }
            else if (gr->Adjncy[k] < gr->Adjncy[j]) {
                k++;
            }
        }
    }


    if (index > 0) {
        int tmp;
#pragma omp atomic capture 
        { tmp = nextTail;
        nextTail += index;
        }
        for (int y = 0; y < index; y++)
            next[tmp + y] = buff[y];
    }

#pragma omp for schedule(distribution_K)
    for (int i = 0; i < Tail; i++) {
        int e = curr[i];

        flag[e] = true;
        InCurr[e] = false;
    }

#pragma omp barrier

}
int PK_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid)
{
    int k_level = 0;
    int nE = gr->nz / 2;
    int nV = gr->V;
    int Tail = 0;
    int nextTail = 0;
    bool* flag = new bool[nE];
    int* curr = new int[nE];
    bool* InCurr = new bool[nE];
    int* next = new int[nE];
    bool* InNext = new bool[nE];
    int* startEd = new int[nV];

#pragma omp parallel num_threads(th) private(k_level)
    {
        int rank = omp_get_thread_num();

#pragma omp for schedule(distribution_K) 
        for (int e = 0; e < nE; e++) {
            flag[e] = false;
            InCurr[e] = false;
            InNext[e] = false;
        }

        k_level = 0;
        int todo = nE;
        while (todo > 0) {
            PCurr_init(nE, EdgeSupport, k_level, curr, Tail, InCurr);
            while (Tail > 0) {
                todo = todo - Tail;
                PSubLevel(gr, curr, InCurr, Tail, EdgeSupport, k_level, next, InNext, nextTail, flag, edTo, eid);
                if (rank == 0) {
                    int* tmp = curr;
                    curr = next;
                    next = tmp;
                    bool* tm = InCurr;
                    InCurr = InNext;
                    InNext = tm;
                    Tail = nextTail;
                    nextTail = 0;
                }
#pragma omp barrier	
            }
            k_level++;
#pragma omp barrier	
        }
    }

    delete[] flag;
    delete[] curr;
    delete[] InCurr;
    delete[] next;
    delete[] InNext;
    delete[] startEd;

    return k_level;

}

