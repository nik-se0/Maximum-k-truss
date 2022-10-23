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









