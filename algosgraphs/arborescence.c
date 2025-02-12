#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "graphes.h"
/* ci-dessous trois macros pour pouvoir utiliser les termes, booleens, VRAI et FAUX dans les programmes */
#define booleen unsigned char
#define VRAI 1
#define FAUX 0


/* ci-dessous, d�finition d'une structure de liste FIFO d'entiers (pour stocker des ensembles de sommets d'un graphe) */

typedef struct {
  int n; /* nombre d'elements */
  int prem; /* indice du premier element */
  int der; /* indice du dernier elemeent */
  int capacite; /* nombre d'elements que la liste peut contenir */
  int * elements; /* taleau contenant les elements de la liste*/
} ListeFIFO;

graphe * symEfficace(graphe * g)
/* ====================================================================== */
{
  graphe *g_1;
  int nsom, narc, k, x, y;
  pcell p;

  nsom = g->nsom;
  narc = g->narc;
  g_1 = initGraphe(nsom, narc);

  for (y = 0; y < nsom; y++) {/* pour tout y sommet de g */
    for( p = g->gamma[y]; p != NULL; p = p->suivant) {
      ajouteSuccesseur(g_1, p->som, y);}}
  return g_1;
} /* Sym() */

ListeFIFO* initListeFIFO(int capacite){
  ListeFIFO* l;
  l = (ListeFIFO*)malloc(sizeof(ListeFIFO));
  l->n = 0;
  l->capacite = capacite;
  l->prem = 0;
  l->der = 0;
  l->elements = (int*)calloc(capacite, sizeof(int));
  return l;
}

void termineListeFIFO(ListeFIFO *l){
  free(l->elements);
  free(l);  
}

int selectionSuppressionListeFIFO(ListeFIFO *l){
  int el;
  if (l->n > 0){
    el = l->elements[l->prem];
    l->n --;
    l->prem = (l->prem+1)%l->capacite;
    return el;
  }
  else{
    fprintf(stderr, "Erreur : liste vide\n");
    return -1;
  }      
}

booleen insertionListeFIFO(ListeFIFO *l, int x){
  if (l->n < l->capacite){
    l->elements[l->der] = x;
    l->der = (l->der+1)%l->capacite;
    l->n ++;
    return VRAI; /* pour indiquer que l'insertion ns'est bien deroulee */
  }
  else{
    fprintf(stderr, "Erreur : liste pleine \n");
    return FAUX; /* pour indiquer que l'insertion n'a pas etre effectuee */
  }      
}


booleen estNonVideListeFIFO(ListeFIFO *l){
  if(l->n == 0) return FAUX;
  else return VRAI;
}

graphe * symClosure(graphe * g, graphe * symG){
  graphe *Gs;
  pcell p;
  int m;
  int n;
  int i;

  n = g->nsom;
  m = g->narc;
  Gs = initGraphe(n, g->nmaxarc*2);

  // printf("n=%d, m=%d, nmaxarc=%d\n", n, m,  g->nmaxarc);

  for (i = 0; i < n; i++) {
    for (p = g->gamma[i]; p != NULL; p = p->suivant){
      ajouteSuccesseur(Gs, i, p->som);
    }

    for (p = symG->gamma[i]; p != NULL; p = p->suivant){
      ajouteSuccesseur(Gs, i, p->som);
    }
  }

  return Gs;
}

/***************************************************************************/
/* retourne l'exploration du graphe G depuis le sommet x                   */
/* en utilisant l'algorithme exploration largeur (version 1)               */
/***************************************************************************/
int * arborescence(graphe* G, int x){
  booleen *Z;       /* tableau booleens pour stocker l'exploration */
  ListeFIFO *E, *D; /* Liste pour les ensembles de sommets */
  ListeFIFO *tmp;   /* variable temporaire pour permettre l'echange des liste E et D */
  pcell p; /* pointeur-maillon pour parcourir des listes de successeurs */
  int y, z;         /* sommets du graphe */
  int k;            /* num�ro d'iteration */
  int *tree;
  graphe *Gs;

  Gs = symClosure(G, symEfficace(G));

  tree = (int*) malloc( sizeof(int) * Gs->nsom);
  tree[x] = -1;
  /* initialisation de l'ensemble E : E := {x}*/
  E = initListeFIFO(Gs->nsom);
  insertionListeFIFO(E, x);
  /* initialisation de l'ensemble Z : Z := {x}*/
  Z = (booleen*) calloc(Gs->nsom, sizeof(booleen));
  Z[x] = VRAI;
  /* initialisation de l'ensemble D */
  D = initListeFIFO(Gs->nsom);

  /* Completer ici avec le code l'algorithme exploration largeur */
  for (int k = 1; k < Gs->nsom-1; k++ ){
    while (estNonVideListeFIFO(E)){
      y = selectionSuppressionListeFIFO(E);
      for (p = Gs->gamma[y]; p != NULL; p = p->suivant){
        if (!Z[p->som]) {
          insertionListeFIFO(D, p->som);
          Z[p->som] = VRAI;
          tree[p->som] = y;
        }
      }
    }
    E = D;
    D = initListeFIFO(Gs->nsom);
  }
  
  termineListeFIFO(D);
  termineListeFIFO(E);
  
  return tree;
}



/* ====================================================================== */
int main(int argc, char **argv)
/* ====================================================================== */
{
  graphe *G;
  graphe * Gs;
  int* tree;
  int* Symtree;
  int x;               /* sommet de depart � explorer */
  int y;               /* indice muet pour un sommet*/
  booleen *Z;          /* tableau pour stocker l'exploration du graphe */
  int i;               /* indice muet*/
  int nZ;              /* pour compter le nombre de sommets explor�s */
  clock_t debut, fin;  /* pour mesurer les temps d'execution */
  
  if (argc != 3)
  {
    fprintf(stderr, "usage: %s <donnee.graphe> <sommet_initial_a_explorer> \n", argv[0]);
    fprintf(stderr, "            ou %s explore le graphe donnee.graphe deuis sommet_initial_a_explorer\n", argv[0]);
    exit(0);
  }

  /**********************************************************/
  /* Lecture des arguments passes au programmme             */
  /**********************************************************/
  
  G = litGraphe(argv[1]);       /* Lit un graphe stock�e dans un fichier */
  
  x = atoi(argv[2]);

  if(x >= G->nsom){
    fprintf(stderr, "Erreur: pas de sommet d'indice %d dans %s\n", x, argv[1]);
    exit(0);
  }

  fprintf(stderr, "%s lu\n", argv[1]);

  tree = arborescence(G,x);       /* traitement : calcule le symetrique de g */


  printf("Arborescence from vertex %d\n", x);
  for (int i = 0; i < G->nsom; i++){
    printf("Vertex %d <- %d\n", i, tree[i]);
  }

  Gs = symClosure(G, symEfficace(G));

  /* On afffiche l'exploration et on compte le nombre de sommets explor�s */
  nZ = 0;
  
  return 0;
} /* main() */

