#pragma once
#include "omp.h"
#include "graph.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>


using namespace std;
int th = 2;

//Результаты
void Print_res(long nE, int* EdgeSupport)
{
    int maxSup = 0;
    for (long i = 0; i < nE; i++) {
        if (maxSup < EdgeSupport[i]) {
            maxSup = EdgeSupport[i];
        }
    }

    long numMax = 0;
    for (long i = 0; i < nE; i++) {
        if (EdgeSupport[i] == maxSup) {
            numMax++;
        }
    }

    printf("\tMax-truss: %d\n#Edges in Max-truss: %ld\n\n", maxSup + 2, numMax);
}
//Инициализация доп.структур
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
}


//Последовательный алгоритм
//Подсчет поддержки
void SupAM(crsGraph* gr, int* eid, int* EdgeSupport)
{
    long nV = gr->V;
    long nE = gr->nz / 2;
    int* start = new int[nV];
    int* X = new int[gr->V];
    for (int i = 0; i < gr->V; i++) {
        X[i] = 0;
    }

    //Поиск начального индекса
    for (int i = 0; i < nV; i++)
    {
        int j = gr->Xadj[i];
        int nV = gr->Xadj[i + 1];
        while (j < nV && gr->Adjncy[j] < i){
           j++;
        }
        start[i] = j;
    }

    //Подсчет поддержки
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
                if (X[w]) { // Нашли треугольник
                    int e1 = eid[X[w] - 1], e2 = eid[j], e3 = eid[k];
                    EdgeSupport[e1] += 1;
                    EdgeSupport[e2] += 1;
                    EdgeSupport[e3] += 1;
                }
            }
        }
        //Зануление
        for (int j = start[u]; j < gr->Xadj[u + 1]; j++) {
            int w = gr->Adjncy[j];
            X[w] = 0;
        }
    }
}
void SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport)
{
    long nE = gr->nz / 2;
    int* X = new int[gr->V];
    for (long i = 0; i < gr->V; i++) {
        X[i] = (nE + 1);
    }

    for (long e = 0; e < nE; e++)
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
}
//Основной алгоритм
void Curr_init(int k_level, long nE, long& Tail, int* EdgeSupport, int* curr )
{
    for (long i = 0; i < nE; i++) {
        if (EdgeSupport[i] == k_level) {
            curr[Tail] = i;
            Tail = Tail + 1;
        }
    }
}
void SubLevel(crsGraph* gr, int* curr, long Tail, int* EdgeSupport, int k_level, int* next, long& nextTail, bool* flag, Edge* edTo, int* eid)
{
    for (long i = 0; i < Tail; i++) {
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
                if ((!flag[e2]) && (!flag[e3])) { //Если треугольник не обработан
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
    long nE = gr->nz / 2;
    long nV = gr->V;
    long Tail = 0;
    long nextTail = 0;
    int* curr = new int[nE];
    int* next = new int[nE];
    bool* flag = new bool[nE]; 
    for (int e = 0; e < nE; e++) {
        flag[e] = false;
    }

    //Подсчет k_truss
    int k_level = 0;
    long todo = nE;
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

    return k_level;
}


//Параллельный алгоритм
//Поддержка
void SupP(crsGraph* gr, int* eid, int* EdgeSupport, int t){
    th = t;
#pragma omp parallel num_threads(th) 
    {
        long nV = gr->V;
        long nE = gr->nz / 2;
        int* startEd = new int[nV]; 
        int* X = new int[gr->V];
        for (int i = 0; i < gr->V; i++) {
            X[i] = 0;
        }

#pragma omp for schedule( static) 
        for (int i = 0; i < nV; i++) {
            int j = gr->Xadj[i];
            int nV = gr->Xadj[i + 1];
            while (j < nV && gr->Adjncy[j] < i) {       
                j++;
            }
            startEd[i] = j;
        }

        //Подсчет поддержки
#pragma omp for schedule( static)     
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
#pragma omp atomic
                        EdgeSupport[e1]++;
#pragma omp atomic
                        EdgeSupport[e2]++;
#pragma omp atomic
                        EdgeSupport[e3]++;
                    }
                }
            }

            for (int j = startEd[u]; j < gr->Xadj[u + 1]; j++) {
                int w = gr->Adjncy[j];
                X[w] = 0;
            }
        }
#pragma omp barrier
    }
}

void PCurr_init(long nE, int* EdgeSupport, int k_level, int* curr, long& Tail, bool* InCurr)
{
    const long BUFFER_SIZE_BYTES = 2048;
    const long BUFFER_SIZE = BUFFER_SIZE_BYTES / sizeof(int);
    int buff[BUFFER_SIZE];
    long index = 0;

#pragma omp for schedule( static) 
    for (long i = 0; i < nE; i++) {
        if (EdgeSupport[i] == k_level) {
            buff[index] = i;
            InCurr[i] = true;
            index++;

            if (index >= BUFFER_SIZE) {
                long tmp;
#pragma omp atomic capture 
                { tmp = Tail;
                Tail += BUFFER_SIZE;
                }

                for (long j = 0; j < BUFFER_SIZE; j++) {
                    curr[tmp + j] = buff[j];
                }
                index = 0;
            }
        }
    }

    if (index > 0) {
        long tmp;
#pragma omp atomic capture 
        { tmp = Tail;
        Tail += index;
        }
        for (long j = 0; j < index; j++) {
            curr[tmp + j] = buff[j];
        }
    }

#pragma omp barrier


}
void PSubLevel(crsGraph* gr, int* curr, bool* InCurr, long Tail, int* EdgeSupport, int k_level, int* next, bool* InNext, long& nextTail, bool* flag, Edge* edTo, int* eid)
{//Size of cache line
const long BUFFER_SIZE_BYTES = 2048;
const long BUFFER_SIZE = BUFFER_SIZE_BYTES / sizeof(int);

int buff[BUFFER_SIZE];
long index = 0;

#pragma omp for schedule( static)
for (long i = 0; i < Tail; i++) {

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
                //Если в curr лежит только одно ребро из треугольника
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
                        if (sup <= k_level) {//если это ребро обрабатывалась еще оним потоком то возрващаем sup (?)
#pragma omp atomic 
                            EdgeSupport[E[q]] += 1;
                        }
                        if (index >= BUFFER_SIZE) {
                            long tmp;
#pragma omp atomic capture 
                            { tmp = nextTail;
                            nextTail += BUFFER_SIZE;
                            }
                            for (long y = 0; y < BUFFER_SIZE; y++) {
                                next[tmp + y] = buff[y];
                            }
                            index = 0;
                        }
                        q++;
                    }
                }
                //Если в curr лежит два ребра из треугольника
                else if (EdgeSupport[e2] > k_level) {
                    if ((e1 < e3 && InCurr[e3])||!InCurr[e3]) {
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
                            long tmp; 
#pragma omp atomic capture 
                            { tmp = nextTail;
                            nextTail += BUFFER_SIZE;
                            }

                            for (long y = 0; y < BUFFER_SIZE; y++)
                                next[tmp + y] = buff[y];
                            index = 0;
                        }
                    }
                }
                else if (EdgeSupport[e3] > k_level) {
                    if ((e1 < e2 && InCurr[e2])|| !InCurr[e2]){
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
                           EdgeSupport[e3]-= 1;
                        }

                        if (index >= BUFFER_SIZE) {
                            long tmp;
#pragma omp atomic capture 
                            { tmp = nextTail;
                            nextTail += BUFFER_SIZE;
                            }
                            for (long y = 0; y < BUFFER_SIZE; y++)
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
    long tmp;
#pragma omp atomic capture 
    { tmp = nextTail;
    nextTail += index;
    }
    for (long y = 0; y < index; y++)
        next[tmp + y] = buff[y];
}

#pragma omp barrier

#pragma omp for schedule( static)
for (long i = 0; i < Tail; i++) {
    int e = curr[i];

    flag[e] = true;
    InCurr[e] = false;
}

#pragma omp barrier

}
int PK_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid)
{
    int k_level = 0;
    long nE = gr->nz / 2;
    long nV = gr->V;
    long Tail = 0;
    long nextTail = 0;
    bool* flag = new bool[nE];
    int* curr = new int[nE];
    bool* InCurr = new bool[nE];
    int* next = new int[nE];
    bool* InNext = new bool[nE];
    int* startEd = new int[nV];

#pragma omp parallel num_threads(th) private(k_level)
    {
        int rank = omp_get_thread_num();

#pragma omp for schedule(static) 
        for (int e = 0; e < nE; e++) {
            flag[e] = false;
            InCurr[e] = false;
            InNext[e] = false;
        }

        k_level = 0;
        long todo = nE;
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
 
  return k_level;
  
}
