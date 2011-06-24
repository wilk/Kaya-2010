#ifndef ASL_E
#define ASL_E
#include <pcb.e>
#include <const.h>
#include <types10.h>
#include <listx.h>

/* Semaphore list handling functions */

int insertBlocked(S32 *semAdd, pcb_t *p);
pcb_t *removeBlocked(S32 *semAdd);
pcb_t *outBlocked(pcb_t *p);
pcb_t *headBlocked(S32 *semAdd);
void initSemd(void);

#endif

