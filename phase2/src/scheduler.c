/**
 *  @file scheduler.c
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @note Questo modulo implementa lo scheduler dei processi di Kaya e il rivelatore dei deadlock
 */

/* Inclusioni phase1 */ 
#include <pcb.e>

/* Inclusioni phase2 */
#include <exceptions.e>
#include <initial.e>
#include <interrupts.e>
#include <scheduler.e>

/* Inclusioni uMPS */
#include <libumps.e>

/**
  * @brief Gestione dello scheduler.
  * @return void.
 */ 
void scheduler()
{
	/* Se esiste attualmente un processo in esecuzione */
	if(currentProcess != NULL)
	{		
		/* Imposta il tempo di partenza del processo sulla CPU */
		currentProcess->p_cpu_time += (GET_TODLOW - processTOD);
		processTOD = GET_TODLOW;
		
		/* Aggiorna il tempo trascorso dello pseudo-clock tick*/
		timerTick += (GET_TODLOW - startTimerTick);
		startTimerTick = GET_TODLOW;
		
		/* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
		SET_IT(MIN((SCHED_TIME_SLICE - currentProcess->p_cpu_time), (SCHED_PSEUDO_CLOCK - timerTick)));

		/* Carica lo stato del processo corrente */
		LDST(&(currentProcess->p_state));
	}
	/* Se invece non è presente nessun processo sulla CPU */
	else if(currentProcess == NULL)
	{
		/* Se la Ready Queue è vuota */
		if(emptyProcQ(&readyQueue))
		{
			if(processCount == 0) HALT();
			if((processCount > 0) && (softBlockCount == 0)) PANIC();	/* Deadlock */
			if((processCount > 0) && (softBlockCount > 0))
			{
				/* Wait State */
				/* Interrupt attivati e non mascherati */
				setSTATUS((getSTATUS() | STATUS_IEc | STATUS_INT_UNMASKED));
				while(TRUE) ;
			}
			PANIC(); /* caso anomalo */
		}
		
		/* Prende il primo processo Ready */
		currentProcess = removeProcQ(&readyQueue);
		
		if(currentProcess == NULL) PANIC(); /* caso anomalo */
		
		/* Calcola i millisecondi trascorsi dall'avvio dello pseudo-clock tick */
		timerTick += GET_TODLOW - startTimerTick;
		startTimerTick = GET_TODLOW;
		
		/* Imposta il tempo di partenza del processo sulla CPU */
		currentProcess->p_cpu_time = 0;
		processTOD = GET_TODLOW;
		
		/* Imposta l'Interval Timer col tempo minore rimanente tra il timeslice e lo pseudo-clock tick */
		SET_IT(MIN(SCHED_TIME_SLICE, (SCHED_PSEUDO_CLOCK - timerTick)));
		
		/* Carica lo stato del processo sul processore */
		LDST(&(currentProcess->p_state));
	}
}
