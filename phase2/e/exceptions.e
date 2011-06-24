/**
 *  @file exceptions.e
 *  @author Vincenzo Ferrari - Barbara Iadarola
 *  @brief File di definizione del modulo exceptions.c
 *  @note Contiene tutte le definizioni delle funzioni implementate nel modulo exceptions.c
 */
 
#ifndef EXCEPTIONS_E
#define EXCEPTIONS_E

#include <types10.h>
#include <listx.h>
#include <const.h>

void saveCurrentState (state_t *current, state_t *new);
void sysBpHandler();
int createProcess(state_t *statep);
int terminateProcess(int pid);
void verhogen(int *semaddr);
void passeren(int *semaddr);
int getPid();
cpu_t getCPUTime();
void waitClock();
unsigned int waitIO(int intlNo, int dnum, int waitForTermRead);
int getPpid();
void specTLBvect(state_t *oldp, state_t *newp);
void specPGMvect(state_t *oldp, state_t *newp);
void specSYSvect(state_t *oldp, state_t *newp);
void pgmTrapHandler();
void tlbHandler();
void intHandler();

#endif
