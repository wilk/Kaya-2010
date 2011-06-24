#ifndef _TYPES10_H
#define _TYPES10_H
#include <uMPStypes.h>
#include <listx.h>
#include <const.h>

typedef struct pcb_t {
	/*process queue fields */

	struct list_head	p_next;

	/*process tree fields */
	struct list_head	p_child,
				p_sib;

	struct pcb_t  *p_prnt;

	/* processor state, etc */
	state_t       p_state;

	S32           *p_semAdd;

	/* process id */
	int p_pid;
	
	/* CPU_TIME of process */
	cpu_t p_cpu_time;
	
	/* Exception State Vector */
	int ExStVec[MAX_STATE_VECTOR];
	
	/* TLB */
	state_t *tlbState_old;
	state_t *tlbState_new;
	
	/* PgmTrap */
	state_t *pgmtrapState_old;
	state_t *pgmtrapState_new;
	
	/* SysBP */
	state_t *sysbpState_old;
	state_t *sysbpState_new;
	
	/* Semaphore Flag */
	int p_isOnDev;
	
} pcb_t;

typedef struct semd_t {
	struct list_head	s_next;
	S32			*s_semAdd;
	struct list_head	s_procQ;
} semd_t;

/* Struttura per la tabella dei pcb utilizzati */
typedef struct pcb_pid_t {
	/* Pid del processo */
	int pid;
	/* Puntatore al pcb */
	pcb_t *pcb;
} pcb_pid_t;

#endif
