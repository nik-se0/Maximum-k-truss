#pragma once
#include"graphio.h"
using namespace std;

struct Edge 
{
    int u;
    int v;

    Edge() 
    {
        this->u = 0;
        this->v = 0;
    }
    Edge(int u, int v)
    {
        this->u = u;
        this->v = v;
    }
};

void getEid(crsGraph* gr, int* eid, Edge* idEdge);

//Подсчет поддержки
void SupAM(crsGraph* gr, int* eid, int* EdgeSupport);
int  SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport);

//Алгоритм
void Curr_init(long nE, int* EdgeSupport, int k_level, int* curr, long* Tail);
void SubLevel(crsGraph* gr, int* curr, long Tail, int* EdgeSupport, int k_level, int* next, long* nextTail, bool* flag, Edge* edTo, int* eid);
int K_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid);

//Статистика
void display_stats(int* EdgeSupport, long nE);






