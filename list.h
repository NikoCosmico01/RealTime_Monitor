#ifndef _LIST_H
#define _LIST_H

#define BOOL int
#define FALSE 0
#define TRUE (!FALSE)


#define FORNITORE 0
#define LETTORE 1


/*** Elemento base contenuto nella lista ***/
typedef struct
{
	double value;
	char titolo[21];
	int quantita;
	int tipo;  // FORNITORE o LETTORE (0 o 1)
	int fd;  // descrittore del socket
}
itemType;


 /* Nodo della lista (per uso interno) */
struct LINKED_LIST_NODE
{
  itemType item;
  struct LINKED_LIST_NODE *next;
};


/* Per semplicità... */
typedef struct LINKED_LIST_NODE NODE;


/* La lista non è altro che il puntatore associato al primo elemento */
typedef NODE *LIST;





/*** Costruttore/Distruttore ***/

/* Inizializza una nuova lista */
LIST NewList();

/* Azzera la lista */
LIST DeleteList( LIST l );




/*** Predicati ***/

/* ritorna TRUE se la lista e' vuota */
bool isEmpty( LIST l );

/* ritorna il numero di elementi presenti nella lista */
int getLength( LIST l );



/*** Selettori ***/

/* Restituisce l'elemento in testa alla lista, senza modificarla */
itemType getHead( LIST l );

/* Restituisce l'elemento in coda alla lista, senza modificarla */
itemType getTail( LIST l );

/* Cerca l'elemento item nella lista e ne restituisce il puntatore - TODO
   ritorna NULL se non lo trova */
itemType* Find( LIST l, itemType item );



/*** Trasformatori ***/

/* Inserisce un elemento nella prima posizione della lista - TODO */
LIST EnqueueFirst(LIST l, itemType item );

/* Inserisce un elemento nell'ultima posizione della lista */
LIST EnqueueLast(LIST l, itemType item );

/* Inserisce un elemento mantenendo la lista ordinata - TODO */
LIST EnqueueOrdered(LIST l, itemType item );

 /* Toglie il primo elemento della lista (se non e' vuota) */
LIST DequeueFirst( LIST l );

/* Toglie l'ultimo elemento della lista (se non e' vuota) - TODO */
LIST DequeueLast( LIST l );

/* Toglie l'elemento item dalla lista (se e' presente) */
LIST Dequeue(LIST l, itemType item );



/*** Stampe ***/

/* Stampa a video un elemento della lista */
void PrintItem( itemType item );

/* Stampa a video la lista (esegue PrintItem() su tutti gli elementi) */
void PrintList( LIST l );

#endif
