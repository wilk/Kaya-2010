/**
 *  @file asl.c
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @brief Modulo per la gestione dei semafori.
 *  @note Questo modulo contiene le funzioni per la creazione, l'inizializzazione
 *  e la gestione delle liste di descrittori di semafori attivi e non.
 */

#include <pcb.e>
#include <const.h>
#include <types10.h>
#include <listx.h>

/*---------------------------------------------------------------------------------*/
/* Dichiarazione delle variabili globali del asl.c */
/**
  * @brief Testa della lista di descrittori di semafori in uso.
 */
HIDDEN struct list_head semd_h;
/**
  * @brief Testa della lista di descrittori di semafori inutilizzati.
 */
HIDDEN struct list_head semdfree_h;
/**
  * @brief Vettore dei descrittori di semafori disponibili.
 */
HIDDEN semd_t semdtable[MAXPROC];
/*---------------------------------------------------------------------------------*/

/**
  * @brief Inizializza un descrittore di semafori.
  * @param s : puntatore a un descrittore di semaforo da inizializzare.
  * @param semAdd : indirizzo di un descrittore di semaforo.
  * @return void.
 */
void newSem(semd_t *s, S32 *semAdd)
{
	INIT_LIST_HEAD(&(s->s_next));
	INIT_LIST_HEAD(&(s->s_procQ));
	s->s_semAdd=semAdd;
}

/**
  * @brief Inserisce un descrittore di semaforo nella lista dei descrittori inutilizzati.
  * @param s : puntatore al descrittore di semaforo da inserire.
  * @return void.
 */
void freeSem(semd_t *s)
{
	list_add(&s->s_next, &semdfree_h);
}

/**
  * @brief Crea la lista di descrittori inutilizzati.
  * @return void.
 */
void initSemd(void)
{	
	int i;
	semd_t *s;

	INIT_LIST_HEAD(&semdfree_h);
	INIT_LIST_HEAD(&semd_h);

	for (i=0; i<MAXPROC; i++)
	{
		s=&semdtable[i];
		freeSem(s);
	}
}

/**
  * @brief Inserisce un pcb nella coda di pcb associata a un descrittore di semaforo.
  * Se il descrittore non è attivo (quindi non appartiene alla lista ASL), alloca un nuovo
  * descrittore dalla lista dei descrittore inutilizzati, lo inserisce nella ASL (nella posizione appropriata),
  * inizializza i vari campi e procede come sopra.
  * @param semAdd : puntatore al descrittore di semaforo contenente la coda di pcb.
  * @param p : puntatore al pcb da inserire nella coda di pcb di semAdd.
  * @return Restituisce vero (1) se un nuovo descrittore di semaforo deve essere allocato e la
  * lista di descrittori inutilizzati è vuota, altrimenti falso (0).
 */
int insertBlocked(S32 *semAdd, pcb_t *p)
{
	semd_t *s, *slist;

	/* Ciclo che controlla l'esistenza del semaforo con indirizzo semAdd all'interno della lista semd_h (quindi se è attivo) */
	list_for_each_entry(s, &semd_h, s_next)
	{
		/* Se ha trovato il semaforo */
		if(s->s_semAdd==semAdd)
		{
			/* Inserisce il pcb nella coda di processi del semaforo passato */
			insertProcQ(&s->s_procQ, p);
			/* Imposta il campo dell'indirizzo del semaforo del pcb */
			p->p_semAdd=semAdd;
			return(0);
		}
	}

	/* Se non ci sono semafori non attivi disponibili, ritorna vero */
	if(list_empty(&semdfree_h)) 
		return 1;
	else
	{
		/* Recupera l'indirizzo dalla lista semdfree_h */
		s=container_of(semdfree_h.next, semd_t, s_next);
		/* Preleva il primo semaforo inattivo dalla lista semdfree_h */
		list_del(semdfree_h.next);
		/* Poi lo inizializza */
		newSem(s, semAdd);
		/* Inserisce il pcb nella coda dei processi del semaforo */
		insertProcQ(&s->s_procQ, p);
		/* Infine colloca il semaforo nella posizione appropriata */
		list_for_each_entry(slist, &semd_h, s_next)
		{
			if(s->s_semAdd > slist->s_semAdd)
			{
				list_add(&s->s_next, &slist->s_next);
				p->p_semAdd=semAdd;
				return 0;
			}
		}

		/* Altrimenti è il primo che inserisce */
		list_add(&s->s_next, &semd_h);
		p->p_semAdd=semAdd;
		return 0;
	}
}

/**
  * @brief Rimuove il primo pcb dalla coda di pcb del descrittore di semaforo associato presente nella lista
  * ASL. Se la coda di pcb diventa vuota, rimuove il descrittore del semaforo dalla ASL e lo reinserisce nella
  * lista dei descrittori inutilizzati.
  * @param semAdd : puntatore al descrittore di semaforo contenente la coda di pcb.
  * @return Restituisce 'NULL' se il pcb non viene trovato nella coda di pcb, altrimenti un puntatore al pcb rimosso.
 */
pcb_t *removeBlocked(S32 *semAdd)
{
	semd_t *s;
	pcb_t *p;

	/* Ricerca all'interno della lista di semafori attivi ASL il semaforo passato (semAdd) */
	list_for_each_entry(s, &semd_h, s_next)
	{
		/* Se trova il semaforo nella lista (quindi è attivo) */
		if(s->s_semAdd==semAdd)
		{
			/* Preleva il pcb dalla coda dei processi del semaforo */
			p=container_of(s->s_procQ.next, pcb_t, p_next);
			/* Poi lo rimuove da tale coda */
			list_del(s->s_procQ.next);
			if(emptyProcQ(&s->s_procQ))
			{
				list_del(&s->s_next);
				freeSem(s);
			}
			p->p_semAdd = NULL;
			return p;
				
		}
	}
	return NULL;
}

/**
  * @brief Rimuove il pcb specificato dalla coda di pcb del descrittore di semaforo associato.
  * @param p : puntatore al pcb da rimuovere.
  * @return Restituisce 'NULL' se il pcb non viene trovato nella coda di pcb, altrimenti un puntatore al pcb rimosso.
 */
pcb_t *outBlocked(pcb_t *p)
{
	semd_t *s;
	pcb_t *p_aux;

	list_for_each_entry(s, &semd_h, s_next)
		/* Se trova il semaforo associato a 'p' cerca il pcb nella coda */
		if(p->p_semAdd==s->s_semAdd)
		{
			p_aux = outProcQ(&s->s_procQ, p);
			p_aux->p_semAdd = NULL;
			if(emptyProcQ(&s->s_procQ))
			{
				list_del(&s->s_next);
				freeSem(s);
			}
		/* Se lo trova lo rimuove altrimenti restituisce NULL */
			return p_aux;
		}
		
	return NULL;
}

/**
  * @brief Preleva la testa della coda di pcb del descrittore del semaforo associato.
  * @param semAdd : puntatore al descrittore di semaforo contenente la coda di pcb.
  * @return Restituisce 'NULL' se il descrittore non è presente nella lista ASL oppure se la coda di
  * pcb associata ad esso è vuota, altrimenti la testa della coda di pcb.
 */
pcb_t *headBlocked(S32 *semAdd)
{
	semd_t *s;
	pcb_t *p;
	
	/* Ricerca all'interno della lista di semafori attivi ASL il semaforo passato (semAdd) */
	list_for_each_entry(s, &semd_h, s_next)
	{
		/* Se trova il semaforo nella lista (quindi è attivo) */
		if(s->s_semAdd==semAdd)
		{
			/* Se la lista è vuota restituisce NULL */
			if(emptyProcQ(&s->s_procQ))
				return NULL;
			else
			{
				/* Altrimenti preleva il pcb dalla coda dei processi del semaforo */
				p=container_of(s->s_procQ.next, pcb_t, p_next);
				return (p);
			}
		}
	}

	return NULL;
}
