#pragma once
#include"graphio.h"
using namespace std;

struct Edge {
    int u;
    int v;
    Edge() {
        this->u = 0;
        this->v = 0;
    }
    Edge(int u, int v) {
        this->u = u;
        this->v = v;
    }
};
//Initialization of additional structures
void getEid(crsGraph* gr, int* eid, Edge* idEdge);
//Results
void Print_res(int nE, int* EdgeSupport);

//Sequential algorithm
void SupAM(crsGraph* gr, int* eid, int* EdgeSupport);
void SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport);
void Curr_init(int k_level, int nE, int& Tail, int* EdgeSupport, int* curr);
void SubLevel(crsGraph* gr, int* curr, int Tail, int* EdgeSupport, int k_level, int* next, int& nextTail, bool* flag, Edge* edTo, int* eid);
int K_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid);

//Parallel algorithm
void SupP(crsGraph* gr, int* eid, int* EdgeSupport);
void PCurr_init(int nE, int* EdgeSupport, int k_level, int* curr, int& Tail, bool* InCurr);
void PSubLevel(crsGraph* gr, int* curr, bool* InCurr, int Tail, int* EdgeSupport, int k_level, int* next, bool* InNext, int& nextTail, bool* flag, Edge* edTo,int* eid);
int PK_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid);




