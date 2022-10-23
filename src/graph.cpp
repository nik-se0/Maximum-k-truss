#include "graph.h"


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

