#ifndef PCB_E
#define PCB_E
#include <const.h>
#include <types10.h>
#include <listx.h>

/* PCB handling functions */

/* List view functions */

void freePcb(pcb_t *p);
pcb_t *allocPcb(void);
void initPcbs(void);

void mkEmptyProcQ(struct list_head *emptylist);
int emptyProcQ(struct list_head *head);
void insertProcQ(struct list_head *head, pcb_t *p);
pcb_t *removeProcQ(struct list_head *head);
pcb_t *outProcQ(struct list_head *head, pcb_t *p);
pcb_t *headProcQ(struct list_head *head);

/* Tree view functions */
int emptyChild(pcb_t *child);
void insertChild(pcb_t *parent, pcb_t *child);
pcb_t *removeChild(pcb_t *parent);
pcb_t *outChild(pcb_t *child);

#endif

