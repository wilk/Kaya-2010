/**
 *  @file exceptions.c
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @note Questo modulo implementa il gestore dei TLB, delle Program Trap e delle SYS/Bp.
 */
 
/* Inclusioni phase1 */
#include <asl.e>
#include <pcb.e>

/* Inclusioni phase2 */
#include <exceptions.e>
#include <initial.e>
#include <interrupts.e>
#include <scheduler.e>

/* Inclusioni uMPS */
#include <libumps.e>

/* Old Area delle Syscall/BP - TLB - Program Trap */
HIDDEN state_t *sysBp_old = (state_t *) SYSBK_OLDAREA;
HIDDEN state_t *TLB_old = (state_t *) TLB_OLDAREA;
HIDDEN state_t *pgmTrap_old = (state_t *) PGMTRAP_OLDAREA;

/**
  * @brief Salva lo stato corrente (current) in un nuovo stato passato per parametro (new)
  * @param current : stato corrente.
  * @param new : nuovo stato.
  * @return void.
 */
void saveCurrentState (state_t *current, state_t *new)
{
	int i;

	new->entry_hi = current->entry_hi;
	new->cause = current->cause;
	new->status = current->status;
	new->pc_epc = current->pc_epc;
	new->hi = current->hi;
	new->lo = current->lo;

	for (i = 0; i < NREG; i++)
		new->gpr[i] = current->gpr[i];
}

/**
  * @brief Esegue una 'P' sul semaforo passato per parametro. E' diversa dalla SYS4 siccome viene incrementato il softBlockCount.
  * @param *semaddr : puntatore al semaforo su cui fare la 'P'
  * @return void.
 */
HIDDEN void passerenIO(int *semaddr)
{
	(*semaddr)--;

	if((*semaddr) < 0)
	{
		/* Inserisce il processo corrente in coda al semaforo specificato */
		if(insertBlocked((S32 *) semaddr, currentProcess)) PANIC(); /* PANIC se sono finiti i descrittori dei semafori */
		currentProcess->p_isOnDev = IS_ON_DEV;
		currentProcess = NULL;
		softBlockCount++;
		scheduler();
	}
}

/**
  * @brief Gestore delle SYSCALL/BP
  * @return void.
 */
void sysBpHandler()
{
	int cause_excCode;
	int kuMode;
	
	/* Salva lo stato della vecchia area SysBp */
	saveCurrentState(sysBp_old, &(currentProcess->p_state));
	
	/* Per evitare ciclo infinito di Syscall */
	currentProcess->p_state.pc_epc += 4;
	
	/* Recupera il bit della modalità della sysBp Old Area (per controllare se USER MODE o KERNEL MODE) */
	kuMode = ((sysBp_old->status) & STATUS_KUp) >> 0x3;
	
	/* Recupera il tipo di eccezione avvenuta */
	cause_excCode = CAUSE_EXCCODE_GET(sysBp_old->cause);
	
	/* Controlla se è una syscall */
	if (cause_excCode == EXC_SYSCALL)
	{
		/* Controlla se è in USER MODE */
		if(kuMode == TRUE)
		{
			/* Se è stata chiamata una delle 13 Syscall */
			if((sysBp_old->reg_a0 > 0) && (sysBp_old->reg_a0 < RANGE_SYSCALL))
			{
				/* Imposta Cause.ExcCode a RI */
				sysBp_old->cause = CAUSE_EXCCODE_SET(sysBp_old->cause, EXC_RESERVEDINSTR);
				/* Salva lo stato della SysBP Old Area nella pgmTrap Old Area */
				saveCurrentState(sysBp_old, pgmTrap_old);

				pgmTrapHandler();
			}
			else
			{
				/* Se non è già stata eseguita la SYS12, viene terminato il processo corrente */
				if(currentProcess->ExStVec[ESV_SYSBP] == 0)
				{
					int ris;
					
					ris = terminateProcess(-1);
					if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
					scheduler();
				}
				/* Altrimenti viene salvata la SysBP Old Area all'interno del processo corrente */
				else
				{
					saveCurrentState(sysBp_old, currentProcess->sysbpState_old);
					LDST(currentProcess->sysbpState_new);
				}
			}
		}
		/* Altrimenti la gestisce in KERNEL MODE */
		else
		{
			int ris;
			
			/* Salva i parametri delle SYSCALL */
			U32 arg1 = sysBp_old->reg_a1;
			U32 arg2 = sysBp_old->reg_a2;
			U32 arg3 = sysBp_old->reg_a3;
			
			/* Gestisce ogni singola SYSCALL */
			switch(sysBp_old->reg_a0)
			{
				case CREATEPROCESS:
					currentProcess->p_state.reg_v0 = createProcess((state_t *) arg1);
				break;
				
				case TERMINATEPROCESS:	
					ris = terminateProcess((int) arg1);
					if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
				break;
				
				case VERHOGEN:
					verhogen((int *) arg1);
				break;
				
				case PASSEREN:
					passeren((int *)  arg1);
				break;
				
				case GETPID:
					currentProcess->p_state.reg_v0 = getPid();
				break;
				
				case GETCPUTIME:
					currentProcess->p_state.reg_v0 = getCPUTime();
				break;
				
				case WAITCLOCK:
					waitClock();
				break;
				
				case WAITIO:
					currentProcess->p_state.reg_v0 = waitIO((int) arg1, (int) arg2, (int) arg3);
				break;
				
				case GETPPID:
					currentProcess->p_state.reg_v0 = getPpid();
				break;
				
				case SPECTLBVECT:
					specTLBvect((state_t *) arg1, (state_t *)arg2);
				break;
				
				case SPECPGMVECT:
					specPGMvect((state_t *) arg1, (state_t *)arg2);
				break;
				
				case SPECSYSVECT:
					specSYSvect((state_t *) arg1, (state_t *)arg2);
				break;
				
				default:
					/* Se non è già stata eseguita la SYS12, viene terminato il processo corrente */
					if(currentProcess->ExStVec[ESV_SYSBP] == 0) 
					{
						int ris;

						ris = terminateProcess(-1);
						if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
					}
					/* Altrimenti viene salvata la SysBP Old Area all'interno del processo corrente */
					else
					{
						saveCurrentState(sysBp_old, currentProcess->sysbpState_old);
						LDST(currentProcess->sysbpState_new);
					}
			}
			
			scheduler();
		}
	}
	/* Controlla se è una Breakpoint */
	else if(cause_excCode == EXC_BREAKPOINT)
	{
		/* Se non è già stata eseguita la SYS12, viene terminato il processo corrente */
		if(currentProcess->ExStVec[ESV_SYSBP] == 0)
		{
			int ris;

			ris = terminateProcess(-1);
			if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
			scheduler();
		}
		/* Altrimenti viene salvata la SysBP Old Area all'interno del processo corrente */
		else
		{
			saveCurrentState(sysBp_old, currentProcess->sysbpState_old);
			LDST(currentProcess->sysbpState_new);
		}
	}
	/* Chiamata di una Syscall/BP non esistente */
	else PANIC();
}

/**
  * @brief (SYS1) Crea un nuovo processo.
  * @param statep : stato del processore da cui creare il nuovo processo.
  * @return Restituisce -1 in caso di fallimento, mentre il PID (valore maggiore o uguale a 0) in caso di avvenuta creazione.
 */
int createProcess(state_t *statep)
{
	int i;
	pcb_t *p;
	
	/* In caso non ci fossero pcb liberi, restituisce -1 */
	if((p = allocPcb()) == NULL)
		return -1;
	else {
		/* Carica lo stato del processore in quello del processo */
		saveCurrentState(statep, &(p->p_state));

		/* Aggiorna il contatore dei processi e il pidCount (progressivo) */
		processCount++;
		pidCount++;
		p->p_pid = pidCount;
		
		/* Ricerca una cella vuota della tabella pcbused_table in cui inserire il processo appena creato */
		for(i=0; i<MAXPROC; i++)
			if(pcbused_table[i].pid == 0) break;
		
		/* Aggiorna la tabella dei pcb utilizzati, assegnando il giusto pid e il puntatore al pcb del processo creato */
		pcbused_table[i].pid = p->p_pid;
		pcbused_table[i].pcb = p;
		
		/* p diventa un nuovo figlio del processo chiamante */
		insertChild(currentProcess, p);

		insertProcQ(&readyQueue, p);
		
		return pidCount;
	}
}

/**
  * @brief (SYS2) Termina un processo passato per parametro (volendo anche se stesso) e tutta la sua progenie.
  * @param pid : identificativo del processo da terminare.
  * @return Restituisce 0 nel caso il processo e la sua progenie vengano terminati, altrimenti -1 in caso di errore.
 */
int terminateProcess(int pid)
{
	int i;
	pcb_t *pToKill;
	pcb_t *pChild;
	pcb_t *pSib;
	int isSuicide;
	
	pToKill = NULL;
	isSuicide = FALSE;
	
	/* Se è un caso di suicidio, reimposta il pid e aggiorna la flag */
	if((pid == -1) || (currentProcess->p_pid == pid))
	{
		pid = currentProcess->p_pid;
		isSuicide = TRUE;
	}
	
	/* Cerca nella tabella dei pcb usati quello da rimuovere */
	for(i=0; i<MAXPROC; i++)
	{
		if(pid == pcbused_table[i].pid)
		{
			pToKill = pcbused_table[i].pcb;
			break;
		}
	}
	
	/* Se si cerca di uccidere un processo che non esiste, restituisce -1 */
	if(pToKill == NULL) return -1;
	
	/* Se il processo è bloccato su un semaforo esterno, incrementa questo ultimo */
	if(pToKill->p_isOnDev == IS_ON_SEM)
	{
		/* Caso Anomalo */
		if(pToKill->p_semAdd == NULL) PANIC();
		
		/* Incrementa il semaforo e aggiorna questo ultimo se vuoto */
		(*pToKill->p_semAdd)++;
		pToKill = outBlocked(pToKill);
	}
	/* Se invece è bloccato sul semaforo dello pseudo-clock, si incrementa questo ultimo */
	else if(pToKill->p_isOnDev == IS_ON_PSEUDO) pseudo_clock++;
	
	/* Se il processo da uccidere ha dei figli, li uccide ricorsivamente */
	while(emptyChild(pToKill) == FALSE)
	{
		pChild = container_of(pToKill->p_child.prev, pcb_t, p_sib);
		
		if((terminateProcess(pChild->p_pid)) == -1) return -1;
	}
	
	/* Uccide il processo */
	if((pToKill = outChild(pToKill)) == NULL) return -1;
	else
	{
		/* Aggiorna la tabella dei pcb usati */
		pcbused_table[i].pid = 0;
		pcbused_table[i].pcb = NULL;
		
		freePcb(pToKill);
	}
	
	if(isSuicide == TRUE) currentProcess = NULL;

	processCount--;

	return 0;
}

/**
  * @brief (SYS3) Effettua una V su un semaforo.
  * @param semaddr : indirizzo del semaforo.
  * @return void.
 */
void verhogen(int *semaddr)
{
	pcb_t *p;
	
	(*semaddr)++;
	
	p = removeBlocked((S32 *) semaddr);
	/* Se è stato sbloccato un processo da un semaforo esterno */
	if (p != NULL)
	{
		/* Viene inserito nella readyQueue e viene aggiornata la flag isOnDev a FALSE */
		insertProcQ(&readyQueue, p);
		p->p_isOnDev = FALSE;
	}
}

/**
  * @brief (SYS4) Effettua una P su un semaforo.
  * @param semaddr : indirizzo del semaforo.
  * @return void.
 */
void passeren(int *semaddr)
{
	(*semaddr)--;
	
	/* Se un processo viene sospeso ... */
	if((*semaddr) < 0)
	{
		/* Inserisce il processo corrente in coda al semaforo specificato */
		if(insertBlocked((S32 *) semaddr, currentProcess)) PANIC();
		currentProcess->p_isOnDev = IS_ON_SEM;
		currentProcess = NULL;
	}
}

/**
  * @brief (SYS5) Restituisce l'identificativo del processo chiamante.
  * @return PID del processo chiamante.
 */
int getPid()
{
	return currentProcess->p_pid;
}

/**
  * @brief (SYS6) Restituisce il tempo d'uso della CPU da parte del processo chiamante.
  * @return Restituisce il tempo d'uso della CPU.
 */
cpu_t getCPUTime()
{
	/* Aggiorna il tempo di vita del processo sulla CPU */
	currentProcess->p_cpu_time += (GET_TODLOW - processTOD);
	processTOD = GET_TODLOW;
	
	return currentProcess->p_cpu_time;
}

/**
  * @brief (SYS7) Effettua una P sul semaforo dello pseudo-clock.
  * @return void.
 */
void waitClock()
{
	pseudo_clock--;

	if(pseudo_clock < 0)
	{
		/* Inserisce il processo corrente in coda al semaforo specificato */
		if(insertBlocked((S32 *) &pseudo_clock, currentProcess)) PANIC(); /* PANIC se sono finiti i descrittori dei semafori */
		currentProcess->p_isOnDev = IS_ON_PSEUDO;
		currentProcess = NULL;
		softBlockCount++;
		scheduler();
	}
	
}

/**
  * @brief (SYS8) Effettua una P sul semaforo del device specificato.
  * @param intlNo : ennesima linea di interrupt.
  * @param dnum : numero del device.
  * @param waitForTermRead : TRUE se aspetta una lettura da terminale. FALSE altrimenti.
  * @return Restituisce lo Status Word del device specificato.
 */
unsigned int waitIO(int intlNo, int dnum, int waitForTermRead)
{
	switch(intlNo)
	{
		case INT_DISK:
			passerenIO(&sem.disk[dnum]);
		break;
		case INT_TAPE:
			passerenIO(&sem.tape[dnum]);
		break;
		case INT_UNUSED:
			passerenIO(&sem.network[dnum]);
		break;
		case INT_PRINTER:
			passerenIO(&sem.printer[dnum]);
		break;
		case INT_TERMINAL:
			if(waitForTermRead)
				passerenIO(&sem.terminalR[dnum]);
			else
				passerenIO(&sem.terminalT[dnum]);
		break;
		default: PANIC();
	}
	
	return statusWordDev[intlNo - DEV_DIFF + waitForTermRead][dnum];
}

/**
  * @brief (SYS9) Restituisce l'identificativo del genitore del processo chiamante.
  * @return -1 se il processo chiamante è il processo radice (init), altrimenti il PID del genitore.
 */
int getPpid()
{
	if(currentProcess->p_pid == 1) return -1;
	else return currentProcess->p_prnt->p_pid;
}

/**
  * @brief (SYS10) Quando occorre un'eccezione di tipo TLB entrambi gli stati del processore (oldp e newp) vengono salvati nel processo.
  *		   Caso mai la SYS10 fosse già stata chiamata dallo stesso processo, viene trattata come una SYS2.
  * @param oldp : l'indirizzo del vecchio stato del processore.
  * @param newp : l'indirizzo del nuovo stato del processore.
  * @return void.
 */
void specTLBvect(state_t *oldp, state_t *newp)
{
	int ris;
	
	currentProcess->ExStVec[ESV_TLB]++;
	/* Se l'eccezione è già stata chiamata dal processo corrente precedentemente, viene terminato */
	if(currentProcess->ExStVec[ESV_TLB] > 1)
	{
		ris = terminateProcess(-1);
		if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
	}
	else
	{
		/* Altrimenti vengono salvati i due stati del processore nel processo corrente */
		currentProcess->tlbState_old = oldp;
		currentProcess->tlbState_new = newp;
	}
}

/**
  * @brief (SYS11) Quando occorre un'eccezione di tipo Program Trap entrambi gli stati del processore (oldp e newp) vengono salvati nel processo.
  *		   Caso mai la SYS11 fosse già stata chiamata dallo stesso processo, viene trattata come una SYS2.
  * @param oldp : l'indirizzo del vecchio stato del processore
  * @param newp : l'indirizzo del nuovo stato del processore
  * @return void.
 */
void specPGMvect(state_t *oldp, state_t *newp)
{
	int ris;
	
	currentProcess->ExStVec[1]++;
	/* Se l'eccezione è già stata chiamata dal processo corrente, viene terminato */
	if(currentProcess->ExStVec[ESV_PGMTRAP] > 1)
	{
		ris = terminateProcess(-1);
		if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
	}
	else
	{
		/* Altrimenti vengono salvati i due stati del processore nel processo corrente */
		currentProcess->pgmtrapState_old = oldp;
		currentProcess->pgmtrapState_new = newp;
	}
}

/**
  * @brief (SYS12) Quando occorre un'eccezione di tipo Syscall/Breakpoint entrambi gli stati del processore (oldp e newp) vengono salvati nel processo.
  *		   Caso mai la SYS12 fosse già stata chiamata dallo stesso processo, viene trattata come una SYS2.
  * @param oldp : l'indirizzo del vecchio stato del processore
  * @param newp : l'indirizzo del nuovo stato del processore
  * @return void.
 */
void specSYSvect(state_t *oldp, state_t *newp)
{
	int ris;
	
	currentProcess->ExStVec[ESV_SYSBP]++;
	/* Se l'eccezione è già stata chiamata dal processo corrente, viene terminato */
	if(currentProcess->ExStVec[ESV_SYSBP] > 1)
	{
		ris = terminateProcess(-1);
		if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
	}
	else
	{
		/* Altrimenti vengono salvati i due stati del processore nel processo corrente */
		currentProcess->sysbpState_old = oldp;
		currentProcess->sysbpState_new = newp;
	}
}

/**
  * @brief Gestione d'eccezione TLB.
  * @return void.
 */
void tlbHandler()
{
	int ris;
	
	/* Se un processo è attualmente eseguito dal processore, la TLB Old Area viene caricata sul processo corrente */
	if(currentProcess != NULL)
		saveCurrentState(TLB_old, &(currentProcess->p_state));
		
	/* Se non è già stata eseguita la SYS10, viene terminato il processo corrente */
	if(currentProcess->ExStVec[ESV_TLB] == 0) 
	{
		ris = terminateProcess(-1);
		if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
		scheduler();
	}
	/* Altrimenti viene salvata la TLB Old Area all'interno del processo corrente */
	else
	{
		saveCurrentState(TLB_old, currentProcess->tlbState_old);
		LDST(currentProcess->tlbState_new);
	}
}

/**
  * @brief Gestore d'eccezione Program Trap.
  * @return void.
 */
void pgmTrapHandler()
{
	int ris;
	
	/* Se un processo è attualmente eseguito dal processore, la pgmTrap Old Area viene caricata sul processo corrente */
	if(currentProcess != NULL)
	{
		saveCurrentState(pgmTrap_old, &(currentProcess->p_state));
	}
	
	/* Se non è già stata eseguita la SYS11, viene terminato il processo corrente */
	if(currentProcess->ExStVec[ESV_PGMTRAP] == 0)
	{
		ris = terminateProcess(-1);
		if(currentProcess != NULL) currentProcess->p_state.reg_v0 = ris;
		scheduler();
	}
	/* Altrimenti viene salvata la pgmTrap Old Area all'interno del processo corrente */
	else
	{
		saveCurrentState(pgmTrap_old, currentProcess->pgmtrapState_old);
		LDST(currentProcess->pgmtrapState_new);
	}
}
