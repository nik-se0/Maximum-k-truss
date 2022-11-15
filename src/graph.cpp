#include "graph.h"

using namespace std;

void getEid(crsGraph* gr, int* eid, Edge* idEdge) 
{
    int* X = (int*)malloc(gr->V * sizeof(int));
    for (int i = 0; i < gr->V; i++) {
        X[i] = gr->Xadj[i];
    }

    int i = 0;
    for (int u = 0; u < gr->V; u++)
    { for (int j = gr->Xadj[u]; j < gr->Xadj[u + 1]; j++) 
      { int v = gr->Adjncy[j];
        if (u < v) 
        { Edge e;
          e.u = u;
          e.v = v;
          eid[j] = i;
          X[u]++;
          if (gr->Adjncy[X[v]] == u) 
         { eid[X[v]] = i;
            X[v]++;
         }
          idEdge[i] = e;
          i++;
         }
      }
    }
} 

//Подсчет поддержки
void SupAM(crsGraph* gr, int* eid, int * EdgeSupport)
{ long nV = gr->V;
  long nE = gr->nz / 2;
  int* startEd = (int*)malloc(nV * sizeof(int));
  int* X = (int*)malloc(gr->V * sizeof(int));
  for (int i = 0; i < gr->V; i++) 
  { X[i] = 0;}

  //Find the startEd for each vertex
  for (int i = 0; i < nV; i++) 
  { int j = gr->Xadj[i];
    int n = gr->Xadj[i + 1];
    while (j < n) 
    { if (gr->Adjncy[j] > i)
                break;
            j++;
    }
    startEd[i] = j;
  }

  //Подсчет поддержки
  for (int u = 0; u < nV; u++) 
  { for (int j = startEd[u]; j < gr->Xadj[u + 1]; j++)
    { int w = gr->Adjncy[j];
      X[w] = j + 1;
    }
    for (int j = gr->Xadj[u]; j < startEd[u]; j++)
    { int v = gr->Adjncy[j];
      for (int k = gr->Xadj[v + 1] - 1; k >= startEd[v]; k--) 
      { int w = gr->Adjncy[k];
        // check if: w > u
        if (w <= u) { break;}
        if (X[w]) 
        { //This is a triangle
          //edge id's are: <u,w> : eid[ X[w] -1] 
          //<u,w> : eid[ X[w] -1] 
          //<v,u> : eid[ j ]  
          //<v,w> : eid[ k ]   
          int e1 = eid[X[w] - 1], e2 = eid[j], e3 = eid[k];
          EdgeSupport[e1] += 1;
          EdgeSupport[e2] += 1;
          EdgeSupport[e3] += 1;
        }
            }
    }
    //зануление
    for (int j = startEd[u]; j < gr->Xadj[u + 1]; j++) 
    { int w = gr->Adjncy[j];
      X[w] = 0;
    }
  }

  free(X);
  free(startEd);
}
int  SupAI(crsGraph* gr, Edge* edTo, int* EdgeSupport)
{ long nE = gr->nz / 2;
  int maxSup = 0;
  int* X = (int*)malloc(gr->V * sizeof(int));
  for (long i = 0; i < gr->V; i++) 
  { X[i] = (nE + 1);  }

  //Compute the sup of each edge 
  for (long e = 0; e < nE; e++)
  { Edge edge = edTo[e];
    int sup = 0;
    int u = edge.u;
    int v = edge.v;
    //find the number of elements in the intersection of N(u) and N(v)
    //This can be done in time d(u) + d(v) 
    for (int j = gr->Xadj[u]; j < gr->Xadj[u + 1]; j++) 
    { int w = gr->Adjncy[j];
      if (w != v) //store edge id: e
      { X[w] = e; }
    }
    for (int j = gr->Xadj[v]; j < gr->Xadj[v + 1]; j++) 
    { int w = gr->Adjncy[j];
      if (w != u) 
      { //Check if it is marked
        if (X[w] == e) 
        { sup++; }
      }
    }
    EdgeSupport[e] = sup;
    if (maxSup < sup) 
    { maxSup = sup; }
  }

  free(X);

  return maxSup;
}

//Алгоритм
void Curr_init(long nE, int* EdgeSupport, int k_level, int* curr, long* Tail)
{ for (long i = 0; i < nE; i++) 
  { if (EdgeSupport[i] == k_level) 
    { curr[(*Tail)] = i;
      (*Tail) = (*Tail) + 1;
    }
  }
}
void SubLevel(crsGraph* gr, int* curr, long Tail, int* EdgeSupport, int k_level, int* next, long* nextTail, bool* flag, Edge* edTo, int * eid) 
{  
   for (long i = 0; i < Tail; i++)
   { int e1 = curr[i];
     Edge edge = edTo[e1];
     int u = edge.u;
     int v = edge.v;
     int uStart = gr->Xadj[u], uEnd = gr->Xadj[u + 1];
     int vStart = gr->Xadj[v], vEnd = gr->Xadj[v + 1];
     unsigned int n = (uEnd - uStart) + (vEnd - vStart);
     int j = uStart, k = vStart;

     for (unsigned int innerIdx = 0; innerIdx < n; innerIdx++) 
     { if (j >= uEnd) { break;}
       else if (k >= vEnd) { break; }
       else if (gr->Adjncy[j] == gr->Adjncy[k]) 
       { int e2 = eid[k];  //<v,w>
         int e3 = eid[j];  //<u,w> 
         if ((!flag[e2]) && (!flag[e3]))  //If e1, e2, e3 forms a triangle
         { if (EdgeSupport[e2] > k_level && EdgeSupport[e3] > k_level) //Decrease sup of both e2 and e3   
           { EdgeSupport[e2] = EdgeSupport[e2] - 1; //Process e2  
             if (EdgeSupport[e2] == k_level) 
             { next[(*nextTail)] = e2;
               (*nextTail) = (*nextTail) + 1;
             }
             EdgeSupport[e3] = EdgeSupport[e3] - 1; //Process e3     
             if (EdgeSupport[e3] == k_level) 
             { next[(*nextTail)] = e3;
               (*nextTail) = (*nextTail) + 1;
             }
           }
           else if (EdgeSupport[e2] > k_level) 
           { //process e2 
             EdgeSupport[e2] = EdgeSupport[e2] - 1;
             if (EdgeSupport[e2] == k_level)
             { next[(*nextTail)] = e2;
               (*nextTail) = (*nextTail) + 1;
             }
           }
           else if (EdgeSupport[e3] > k_level) 
           { //process e3 
             EdgeSupport[e3] = EdgeSupport[e3] - 1;
             if (EdgeSupport[e3] == k_level) 
             { next[(*nextTail)] = e3;
               (*nextTail) = (*nextTail) + 1;
             }
            }
         }
         j++; k++;
       }
       else if (gr->Adjncy[j] < gr->Adjncy[k]) { j++;}
       else if (gr->Adjncy[k] < gr->Adjncy[j]) { k++;}
      }
     flag[e1] = true;
    }
 

}
int K_Truss(crsGraph* gr, int* EdgeSupport, Edge* edTo, int* eid)
{  long nE = gr->nz / 2;
   long nV= gr->V;
   long Tail = 0;
   long nextTail = 0;
   int* curr = (int*)malloc(nE * sizeof(int));
   int* next = (int*)malloc(nE * sizeof(int));
   bool* flag = (bool*)malloc(nE * sizeof(bool)); //An array to mark flag array
   for (int e = 0; e < nE; e++) 
    { flag[e] = false; }

   //Подсчет k_truss
   int k_level = 0;
   long todo = nE;
   while (todo > 0)
    { Curr_init(nE, EdgeSupport, k_level, curr, &Tail);
      while (Tail > 0)
      { todo = todo - Tail;
        SubLevel(gr, curr, Tail, EdgeSupport, k_level, next, &nextTail, flag, edTo, eid);
        int* tempCurr = curr;
        curr = next;
        next = tempCurr;
        Tail = nextTail;
        nextTail = 0;
      }
      k_level = k_level + 1;
    }

    //Free memory
    free(next);
    free(curr);
    free(flag);
   
   return k_level;
}

//Статистика
void display_stats(int* EdgeSupport, long nE) 
{
    int minSup = INT_MAX;
    int maxSup = 0;

    for (long i = 0; i < nE; i++) {
        if (minSup > EdgeSupport[i]) {
            minSup = EdgeSupport[i];
        }

        if (maxSup < EdgeSupport[i]) {
            maxSup = EdgeSupport[i];
        }
    }

    long numEdgesWithMinSup = 0, numEdgesWithMaxSup = 0;

    for (long i = 0; i < nE; i++) {
        if (EdgeSupport[i] == minSup) {
            numEdgesWithMinSup++;
        }

        if (EdgeSupport[i] == maxSup) {
            numEdgesWithMaxSup++;
          // printf("%d\n",i);
        }
    }

    printf("\nMin-truss: %d\n#Edges in Min-truss: %ld\n\n", minSup + 2, numEdgesWithMinSup);
    printf("Max-truss: %d\n#Edges in Max-truss: %ld\n\n", maxSup + 2, numEdgesWithMaxSup);


}