/**
 *  @file initial.c
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @note Questo modulo implementa il main() ed esporta le variabili globali del nucleo.
 */

/* Inclusioni phase1 */
#include <asl.e>
#include <pcb.e>

/* Inclusioni phase2 */
#include <exceptions.e>
#include <scheduler.e>

/* Inclusioni uMPS */
#include <libumps.e>
 
/* Dichiarazione esterna della funzione test() */
extern void test(void);

/**
  * @brief Funzione che popola le New Areas
  * @param area : indirizzo dell'area da popolare
  * @param handler : inidirizzo del gestore dell'area
  * @return void.
 */
HIDDEN void populate(memaddr area, memaddr handler)
{
	/* Nuova area da popolare */
	state_t *newArea;
	
	/* La nuova area punta alla vecchia */
	newArea = (state_t *) area;
	
	/* Salva lo stato corrente del processore */
	STST(newArea);
	
	/* Imposta il PC all'indirizzo del gestore delle eccezioni */
	newArea->pc_epc = newArea->reg_t9 = handler;
	
	newArea->reg_sp = RAMTOP;
	
	/* Interrupt mascherati, Memoria Virtuale spenta, Kernel Mode attivo */
	newArea->status = (newArea->status | STATUS_KUc) & ~STATUS_INT_UNMASKED & ~STATUS_VMp;
}

/* Dichiarazione delle variabili globali */

/**
  * @brief Coda di processi in attesa di esecuzione
 */
struct list_head readyQueue;

/**
  * @brief Puntatore al pcb del processo in esecuzione
 */
pcb_t *currentProcess;

/**
  * @brief Contatore dei processi
 */
U32 processCount;

/**
  * @brief Contatore univoco degli identificativi dei processi
 */
U32 pidCount;

/**
  * @brief Contatore dei processi bloccati in attesa di I/O
 */
U32 softBlockCount;

/**
  * @brief Tabella degli STATUS WORD dei device
 */
int statusWordDev[STATUS_WORD_ROWS][STATUS_WORD_COLS];

/**
  * @brief Struttura dei Semafori
  * @note 8 linee di Interrupt per ogni dispositivo
  *	  Il terminale è visto come due dispositivi separati, uno per ricevere e uno per trasmettere
 */
struct {
	int disk[DEV_PER_INT];
	int tape[DEV_PER_INT];
	int network[DEV_PER_INT];
	int printer[DEV_PER_INT];
	int terminalR[DEV_PER_INT];
	int terminalT[DEV_PER_INT];
} sem;

/**
  * @brief Semaforo dello pseudo-clock
 */
int pseudo_clock;

/**
  * @brief Tempo d'avvio del processo corrente sul processore
 */
cpu_t processTOD;

/**
  * @brief Cronometro per riconoscere il tick (ogni 100 millisecondi)
 */
int timerTick;

/**
  * @brief Tempo d'avvio dello pseudo-clock tick
 */
cpu_t startTimerTick;

/**
  * @brief Tabella dei pcb utilizzati
 */
pcb_pid_t pcbused_table[MAXPROC];

/**
  * @brief Inizializzazione del nucleo.
  * @return void.
 */ 
int main(void)
{
	pcb_t *init;
	int i;

	/* Popolazione delle 4 nuove aree nella ROM Reserved Frame */
	
	/*	SYS/BP Exception Handling	*/
	populate(SYSBK_NEWAREA, (memaddr) sysBpHandler);
	
	/*	PgmTrap Exception Handling	*/
	populate(PGMTRAP_NEWAREA, (memaddr) pgmTrapHandler);
	
	/*	TLB Exception Handling		*/
	populate(TLB_NEWAREA, (memaddr) tlbHandler);
	
	/*	Interrupt Exception Handling	*/
	populate(INT_NEWAREA, (memaddr) intHandler);

	/* Inizializzazione delle strutture dati del livello 2 (phase1) */
	initPcbs();
	initSemd();
	
	/* Inizializzazione delle variabili globali */
	mkEmptyProcQ(&readyQueue);
	currentProcess = NULL;
	processCount = softBlockCount = pidCount = 0;
	timerTick = 0;
	
	/* Inizializzazione della tabella dei pcb utilizzati */
	for(i=0; i<MAXPROC; i++)
	{
		pcbused_table[i].pid = 0;
		pcbused_table[i].pcb = NULL;
	}
	
	/* Inizializzazione dei semafori dei device */
	for(i=0; i<DEV_PER_INT; i++)
	{
		sem.disk[i] = 0;
		sem.tape[i] = 0;
		sem.network[i] = 0;
		sem.printer[i] = 0;
		sem.terminalR[i] = 0;
		sem.terminalT[i] = 0;
	}
	
	/* Inizializzazione del semaforo dello pseudo-clock */
	pseudo_clock = 0;
	
	/* Inizializzazione del primo processo (init) */
	/* Se il primo processo (init) non viene creato, PANIC() */
	if((init = allocPcb()) == NULL)
		PANIC();
	
	/* Interrupt attivati e smascherati, Memoria Virtuale spenta, Kernel-Mode attivo */
	init->p_state.status = (init->p_state.status | STATUS_IEp | STATUS_INT_UNMASKED | STATUS_KUc) & ~STATUS_VMp;
	
	/* Il registro SP viene inizializzato a RAMTOP-FRAMESIZE */
	init->p_state.reg_sp = RAMTOP - FRAME_SIZE;
	
	/* PC inizializzato all'indirizzo di test() */
	init->p_state.pc_epc = init->p_state.reg_t9 = (memaddr)test;
	
	/* Il PID impostato per init è 1 */
	pidCount++;
	init->p_pid = pidCount;
	
	/* Aggiorna la tabella dei pcb utilizzati, assegnando il giusto pid e il puntatore al pcb di init */
	pcbused_table[0].pid = init->p_pid;
	pcbused_table[0].pcb = init;

	/* Inserisce init nella coda di processi Ready */
	insertProcQ(&readyQueue, init);
	
	processCount++;
	
	/* Avvio il tempo per il calcolo dello pseudo-clock tick */
	startTimerTick = GET_TODLOW;
	
	scheduler();
	
	return 0;
}
