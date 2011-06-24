/**
 *  @file pcb.c
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @brief Modulo per la gestione delle code e degli alberi di pcb.
 *  @note Questo modulo contiene le funzioni per l'inizializzazione, l'allocazione,
 *  la deallocazione e la gestione delle code e degli alberi dei pcb.
 */

#include <const.h>
#include <types10.h>
#include <listx.h>
#include <pcb.e>

/*---------------------------------------------------------------------------------*/
/* Dichiarazione delle variabili globali del pcb.c */

/**
  * @brief Testa della lista dei pcb liberi.
 */
HIDDEN struct list_head pcbfree_h;
/**
  * @brief Vettore dei pcb disponibili.
 */
HIDDEN pcb_t pcbtable[MAXPROC];

/*---------------------------------------------------------------------------------*/

/* Funzioni sulle liste di pcb */

/**
  * @brief Inizializza i vari campi di un pcb.
  * @param p : puntatore al pcb da inizializzare.
  * @return void.
 */
HIDDEN void newPcb(pcb_t *p)
{
	int i;

	/* Inizializza le liste di 'p' */
	INIT_LIST_HEAD(&p->p_next);
	INIT_LIST_HEAD(&p->p_child);
	INIT_LIST_HEAD(&p->p_sib);

	/* Inizializza la struttura padre */
	p->p_prnt = NULL;

	/* Inizializza lo stato del pcb */
	p->p_state.entry_hi = 0;
	p->p_state.cause = 0;
	p->p_state.status = 0;
	p->p_state.pc_epc = 0;
	p->p_state.hi = 0;
	p->p_state.lo = 0;

	for(i=0;i<NREG;i++)
		p->p_state.gpr[i] = 0;
		
	/* Inizializza l'indirizzo del semaforo */
	p->p_semAdd = NULL;
	
	/* Altri campi */
	/* process id */
	p->p_pid = 0;
	
	/* CPU_TIME of process */
	p->p_cpu_time = 0;
	
	/* Exception State Vector */
	for(i=0;i<MAX_STATE_VECTOR;i++)
		p->ExStVec[i] = 0;
		
	/* Il processo non è bloccato su alcun semaforo inizialmente */
	p->p_isOnDev = FALSE;
}

/**
  * @brief Inserisce un pcb nella lista dei pcb liberi.
  * @param p : puntatore al pcb da inserire.
  * @return void.
 */
void freePcb(pcb_t *p)
{
	list_add(&p->p_next, &pcbfree_h);
}

/**
  * @brief Crea la lista dei pcb liberi.
  * @return void.
 */
void initPcbs(void)
{
	pcb_t *p;
	int i;

	INIT_LIST_HEAD(&pcbfree_h);

	for(i=0; i<MAXPROC; i++)
	{
		p=&pcbtable[i];
		freePcb(p);
	}
}

/**
  * @brief Alloca un pcb rimuovendolo dalla lista dei pcb liberi.
  * @return Restituisce un puntatore al pcb allocato oppure 'NULL' se la lista dei pcb liberi è vuota.
 */
pcb_t *allocPcb(void)
{
	pcb_t *p;

	/* Se la lista libera è vuota restituisce NULL */
	if (list_empty(&pcbfree_h))
		return NULL;
	else
	{
		/* Altrimenti preleva il pcb dalla lista, lo rimuove, lo inizializza e lo restituisce */
		p=container_of(pcbfree_h.next, pcb_t, p_next);
		list_del(pcbfree_h.next);
		newPcb(p);

		return(p);
	}
}

/* Funzioni sulle code di pcb */

/**
  * @brief Inizializza una coda di pcb vuota.
  * @param emptylist : puntatore alla coda di pcb vuota.
  * @return void.
 */
void mkEmptyProcQ(struct list_head *emptylist)
{
	INIT_LIST_HEAD(emptylist);
}

/**
  * @brief Controlla se una coda di pcb è vuota.
  * @param head : puntatore alla coda di pcb.
  * @return Restituisce vero (1) se la coda è vuota, altrimenti falso (0).
 */
int emptyProcQ(struct list_head *head)
{
	if (list_empty(head)) return 1;
	else return 0;
}

/**
  * @brief Inserisce un pcb in una coda di pcb.
  * @param head : puntatore alla coda di pcb.
  * @param p : puntatore al pcb da inserire.
  * @return void.
 */
void insertProcQ(struct list_head *head, pcb_t *p)
{
	/* list_add_tail perché devono essere ordinati dal più recente al più vecchio */
	list_add_tail(&p->p_next, head);
}

/**
  * @brief Rimuove il primo pcb da una coda di pcb.
  * @param head : puntatore alla coda di pcb.
  * @return Restituisce 'NULL' se la coda è vuota, altrimenti il puntatore al pcb rimosso.
 */
pcb_t *removeProcQ(struct list_head *head)
{
	pcb_t *p;

	/* Se la coda è vuota restituisce NULL */
	if (list_empty(head))
		return NULL;
	else
	{
		/* Altrimenti preleva il pcb dalla coda, lo rimuove e lo restituisce */
		p=container_of(head->next, pcb_t, p_next);
		list_del(head->next);

		return(p);
	}
}

/**
  * @brief Rimuove il pcb specificato da una coda di pcb.
  * @param head : puntatore alla coda di pcb.
  * @param p : puntatore al pcb da rimuovere.
  * @return Restituisce 'NULL' se il pcb non è presente nella coda, altrimenti il puntatore al pcb rimosso.
 */
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
	pcb_t *p_aux;

	if(list_empty(head))
		return NULL;
	else
	{
		/* Se la coda non è vuota, cerca 'p' nella coda dei processi */
		list_for_each_entry(p_aux, head, p_next)
		{
			/* Se lo trova, lo elimina dalla coda e lo restituisce */
			if(p==p_aux)
			{
				list_del(&p->p_next);
				return(p);
			}
		}

		/* Se invece non lo trova nemmeno nella coda di pcb, restituisce NULL */
		return NULL;
	}
}

/**
  * @brief Permette di avere la testa della coda di pcb.
  * @param head : puntatore alla coda di pcb.
  * @return Restituisce 'NULL' se la coda di pcb è vuota, altrimenti il puntatore al pcb testa della coda.
 */
pcb_t *headProcQ(struct list_head *head)
{
	pcb_t *p;

	/* Se la coda è vuota restituisce NULL */
	if (list_empty(head))
		return NULL;
	else
	{
		/* Altrimenti il pcb */
		p=container_of(head->next, pcb_t, p_next);
		return (p);
	}

}

/* Funzioni sugli alberi di pcb */

/**
  * @brief Aggiorna il puntatore del padre al pcb figlio.
  * @param parent : puntatore al pcb genitore
  * @param child : puntatore al pcb figlio.
  * @return void.
 */
HIDDEN void updateParent(pcb_t *parent, pcb_t *child)
{
	pcb_t *pNext;
	pcb_t *pPrev;
	
	pNext = container_of(child->p_sib.next, pcb_t, p_child);
	pPrev = container_of(child->p_sib.prev, pcb_t, p_child);
	/* Controlla se il figlio da rimuovere e' l'unico della lista 
	   se lo e', aggiorna il puntatore del padre reinizializzandolo
	   altrimenti lo aggiorna facendolo puntare al fratello del figlio rimosso */
	if((pNext == parent) && (pPrev == parent))
		INIT_LIST_HEAD(&parent->p_child);
	else
	{
		parent->p_child.next=child->p_sib.prev;
		parent->p_child.prev=child->p_sib.next;
	}
}

/**
  * @brief Controlla se un pcb non ha figli.
  * @param child : puntatore a un pcb.
  * @return Restituisce vero (1) se il pcb passato non ha figli, altrimenti falso (0).
 */
int emptyChild(pcb_t *child)
{
	if(list_empty(&child->p_child))	return 1;
	else return 0;
}

/**
  * @brief Assegna a un pcb genitore un pcb come figlio.
  * @param parent : puntatore al pcb genitore.
  * @param child : puntatore al pcb figlio.
  * @return void.
 */
void insertChild(pcb_t *parent, pcb_t *child)
{
	list_add_tail(&child->p_sib, &parent->p_child);
	child->p_prnt=parent;
}

/**
  * @brief Rimuove il primo figlio di un pcb genitore specificato.
  * @param parent : puntatore al pcb genitore.
  * @return Restituisce 'NULL' se il pcb genitore non ha figli, altrimenti il puntatore al pcb figlio.
 */
pcb_t *removeChild(pcb_t *parent)
{
	pcb_t *child;

	/* Se non ci sono figli, restituisce NULL */
	if (list_empty(&(parent->p_child)))
		return NULL;
	else
	{
		/* Prende il primo figlio del genitore */
		child=container_of(parent->p_child.next, pcb_t, p_sib);

		/* Aggiorna il puntatore del padre al figlio */
		updateParent(parent, child);

		/* E infine lo rimuove dalla lista */
		list_del(&(child->p_sib));
		child->p_prnt=NULL;

		return child;
	}
}

/**
  * @brief Rimuove dalla lista di figli, un pcb figlio specificato.
  * @param child : puntatore al pcb figlio.
  * @return Restituisce 'NULL' se il pcb non ha un pcb genitore, altrimenti il puntatore al pcb figlio rimosso.
 */
pcb_t *outChild(pcb_t *child)
{
	pcb_t *parent;

	/* Se è orfano restituisce NULL */
	if(child->p_prnt == NULL)
		return NULL;
	else
	{
		/* Preleva il genitore del figlio da rimuovere */
		parent=container_of(&child->p_prnt->p_sib, pcb_t, p_sib);

		/* Aggiorna il puntatore del padre al figlio */
		updateParent(parent, child);

		/* Altrimenti lo rimuove dalla lista e modifica il puntatore al genitore */
		list_del(&(child->p_sib));
		child->p_prnt=NULL;

		return child;
	}
}
