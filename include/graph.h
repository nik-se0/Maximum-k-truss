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
//Инициализация доп.структур
void getEid(crsGraph* gr, int* eid, Edge* idEdge);
//Результаты
void Print_res(long nE, int* EdgeSupport);

//Последовательный алгоритм
//Подсчет поддержки
void SupAM(crsGraph* gr, int* eid, int* EdgeSupport);
void SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport);
//Основной алгоритм
void Curr_init(int k_level, long nE, long& Tail, int* EdgeSupport, int* curr);
void SubLevel(crsGraph* gr, int* curr, long Tail, int* EdgeSupport, int k_level, int* next, long& nextTail, bool* flag, Edge* edTo, int* eid);
int K_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid);


//Параллельный алгоритм
void SupP(crsGraph* gr, int* eid, int* EdgeSupport, int t);    //Подсчет поддержки
void PCurr_init(long nE, int* EdgeSupport, int k_level, int* curr, long& Tail, bool* InCurr);
void PSubLevel(crsGraph* gr, int* curr, bool* InCurr, long Tail, int* EdgeSupport, int k_level, int* next, bool* InNext, long& nextTail, bool* flag, Edge* edTo,int* eid);
int PK_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid);




