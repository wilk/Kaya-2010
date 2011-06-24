#ifndef _CONST_H
#define _CONST_H

/**************************************************************************** 
 *
 * This header file contains the global constant & macro definitions.
 * 
 ****************************************************************************/

#include "base.h"

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
											 and system daemons */

#define DELAY_DAEMON_ASID UPROCMAX   /* to avoid overlapping ASIDs */
/* #define ANOTHER_DAEMON_ASID UPROCMAX + 1
   #define YET_ANOTHER_DAEMON_ASID UPROCMAX + 2
	 etc. */

/* Addresses for new and old areas (where new and old processor states are 
   stored on exceptions) */
#define INT_NEWAREA 0x2000008c
#define INT_OLDAREA 0x20000000
#define TLB_NEWAREA 0x200001a4
#define TLB_OLDAREA 0x20000118
#define PGMTRAP_NEWAREA 0x200002bc
#define PGMTRAP_OLDAREA 0x20000230
#define SYSBK_NEWAREA 0x200003d4
#define SYSBK_OLDAREA 0x20000348

/* nucleus (phase2)-handled SYSCALL values */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define VERHOGEN 3
#define PASSEREN 4
#define GETPID 5
#define GETCPUTIME 6
#define WAITCLOCK 7
#define WAITIO 8
#define GETPPID 9
#define SPECTLBVECT 10
#define SPECPGMVECT 11
#define SPECSYSVECT 12

#define SYSCALL_MAX 12

/* VM/IO support level (phase3)-handled SYSCALL values */
#define READTERMINAL 13
#define WRITETERMINAL 14
#define VSEMVIRT 15
#define PSEMVIRT 16
#define DELAY 17
#define DISK_PUT 18
#define DISK_GET 19
#define WRITEPRINTER 20
#define TERMINATE 21

#define SYSCALL_TOT 21

/* Bus register area. Among other informations, the start and amount of
   installed RAM are stored here */
#define BUS_RAMBASEADDR 0x10000000
#define BUS_INSTALLEDRAM 0x10000004

#define RAMTOP (*((U32 *)BUS_RAMBASEADDR)) + (*((U32 *)BUS_INSTALLEDRAM))

#define BUS_TIMESCALE 0x10000024  /* How many clock ticks per microsecond */
#define BUS_INTERVALTIMER 0x10000020

/* Elapsed clock ticks (CPU instructions executed) since system power on.
   Only the "low" part is actually used. */
#define BUS_TODLOW 0x1000001c
#define BUS_TODHIGH 0x10000018

#define DEV_USED_INTS 5 /* Number of ints reserved for devices: 3,4,5,6,7 */

#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

/* Total maximum number of devices: terminals are really two devices each, so
   re-add DEV_PER_INT devices, plus one for the pseudo-clock */
#define MAX_DEVICES (DEV_USED_INTS * DEV_PER_INT) + DEV_PER_INT + 1

/* The last semaphore is the pseudo-clock one */
#define CLOCK_SEM (MAX_DEVICES - 1)

/* Interrupt lines used by the devices */
#define INT_TIMER 2    /* timer interrupt */
#define INT_LOWEST 3   /* minimum interrupt number used by real devices */
#define INT_DISK 3
#define INT_TAPE 4
#define INT_UNUSED 5   /* network? */
#define INT_PRINTER 6
#define INT_TERMINAL 7

/* Device command and status codes
   Only those actually used in the code are defined here. If the need arises,
   other commands and states may be defined here. */

#define DEV_C_ACK   1 /* command common to all devices */

#define DEV_DISK_C_SEEKCYL  2        /* disk:       */
#define DEV_DISK_C_READBLK  3
#define DEV_DISK_C_WRITEBLK 4

#define DEV_TAPE_C_READBLK 3   /* tape:       */

#define DEV_PRNT_C_PRINTCHR 2   /* printer */

#define DEV_TRCV_C_RECVCHAR 2   /* terminal */
#define DEV_TTRS_C_TRSMCHAR 2   

#define DEV_S_READY   1   /* status common to all devices */

#define DEV_TRCV_S_RECVERR  4  /* terminal-specific */
#define DEV_TRCV_S_CHARRECV 5

#define DEV_TTRS_S_TRSMERR  4
#define DEV_TTRS_S_CHARTRSM 5

#define TAPE_EOF 1
#define TAPE_EOT 0

/*Device register words*/
#define DEV_REG_STATUS        0x0
#define DEV_REG_COMMAND       0x4
#define DEV_REG_DATA0         0x8
#define DEV_REG_DATA1         0xC

#define TERM_R_STATUS     0x0
#define TERM_R_COMMAND    0x4
#define TERM_T_STATUS     0x8
#define TERM_T_COMMAND    0xC


/* Physical memory frame size */
#define FRAME_SIZE 4096   /* or 0x1000 bytes, or 4K */
#define WORD_SIZE 4


/* Interrupting devices bitmaps starting address: the actual bitmap address is
   computed with INT_INTBITMAP_START + (WORD_SIZE * (int_no - 3)) */
#define PENDING_BITMAP_START 0x1000003c

/* Installed devices bitmap starting address: same as above */
/* #define INST_BITMAP_START 0x10000028 */

/* Address of the first real device register */
#define DEV_REGS_START 0x10000050

/* Size of a single device register */
#define DEV_REG_SIZE (4 * WORD_SIZE)

/* Size of a device register group */
#define DEV_REGBLOCK_SIZE (DEV_REG_SIZE * DEV_PER_INT)

/* Scheduling constants */
#define SCHED_TIME_SLICE 5000     /* in microseconds, aka 5 milliseconds */
#define SCHED_PSEUDO_CLOCK 100000 /* pseudo-clock tick "slice" length */
#define SCHED_BOGUS_SLICE 500000  /* just to make sure */

/* The next two are used a lot and should better be "inlined" for speed, so
   define them as macros */

/* "current" TOD value (elapsed CPU ticks), converted in microseconds */
#define GET_TODLOW (*((U32 *)BUS_TODLOW) / (*(U32 *)BUS_TIMESCALE))

/* Set the interval timer with the given value (in microseconds). Convert the
   value in CPU ticks, and load the interval timer register with it */
#define SET_IT(timer_val) ((*((U32 *)BUS_INTERVALTIMER)) = (timer_val * (*(U32 *)BUS_TIMESCALE)))

/* Utility definitions for the status register: OR the register with these
   to set a specific bit, AND with the opposite to unset a specific bit.
	 For example, to set STATUS_VMc, do status = status | STATUS_VMc;  to unset 
	 it do status = status & ~STATUS_VMc */
#define STATUS_IEc 0x00000001
#define STATUS_KUc 0x00000002
#define STATUS_IEp 0x00000004
#define STATUS_KUp 0x00000008
#define STATUS_IEo 0x00000010
#define STATUS_KUo 0x00000020
#define STATUS_VMc 0x01000000
#define STATUS_VMp 0x02000000
#define STATUS_VMo 0x04000000

/* All interrupts unmasked */
#define STATUS_INT_UNMASKED 0x0000ff00

/* Utility definitions for the entryHI register */
#define ENTRYHI_SEGNO_GET(entryHI) (((entryHI) & 0xc0000000) >> 30)
#define ENTRYHI_VPN_GET(entryHI) (((entryHI) & 0x3ffff000) >> 12)
#define ENTRYHI_ASID_GET(entryHI) (((entryHI) & 0x00000fc0) >> 6)

#define ENTRYHI_SEGNO_SET(entryHI, seg_no) (((entryHI) & 0x3fffffff) | ((seg_no) << 30) )
#define ENTRYHI_VPN_SET(entryHI, vpn) (((entryHI) & 0xc0000fff) | ((vpn) << 12))
#define ENTRYHI_ASID_SET(entryHI, asid) (((entryHI) & 0xfffff03f) | ((asid) << 6))

/* Utility definitions for the entryLO register */
#define ENTRYLO_PFN_GET(entryLO) (((entryLO) & 0xfffff000) >> 12)
#define ENTRYLO_PFN_SET(entryLO, pfn) (((entryLO) & 0x00000fff) | ((pfn) << 12))
#define ENTRYLO_NOCACHE 0x00000800  /* Not used in uMPS */
#define ENTRYLO_DIRTY 0x00000400
#define ENTRYLO_VALID 0x00000200
#define ENTRYLO_GLOBAL 0x00000100
#define ENTRYLO_ACCESSED 0x00000080  /* NEW flag, not in the specs */

/* Utility definitions for the Cause CP0 register */
#define CAUSE_EXCCODE_GET(cause) (((cause) & 0x0000007c) >> 2)
#define CAUSE_EXCCODE_SET(cause, exc_code) (((cause) & 0xffffff83) | ((exc_code) << 2))
#define CAUSE_CE_GET(cause) (cause & 0x30000000)

/* Returns 1 if the interrupt int_no is pending */
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8)))

/* Values for CP0 Cause.ExcCode */
#define EXC_INTERRUPT 0
#define EXC_TLBMOD 1
#define EXC_TLBINVLOAD 2
#define EXC_TLBINVSTORE 3
#define EXC_ADDRINVLOAD 4
#define EXC_ADDRINVSTORE 5
#define EXC_BUSINVFETCH 6
#define EXC_BUSINVLDSTORE 7
#define EXC_SYSCALL 8
#define EXC_BREAKPOINT 9
#define EXC_RESERVEDINSTR 10
#define EXC_COPROCUNUSABLE 11
#define EXC_ARITHOVERFLOW 12
#define EXC_BADPTE 13
#define EXC_PTEMISS 14

/* Used for disk_op() and in the pager to identify request types */
#define READ 0
#define WRITE 1

#define KSEGOS 0x0
#define KSEGOS_BASE_MAP 0x20000000 /* 00|10 0000 0000 0000 0000 */

#define KUSEG2 0x2
#define KUSEG2_BASE_MAP 0x80000000 /* 10|00 0000 0000 0000 0000 */

#define KUSEG3 0x3
#define KUSEG3_BASE_MAP 0xc0000000 /* 11|00 0000 0000 0000 0000 */

#define MAXPAGES 32
#define KUSEG_PAGES MAXPAGES  /* pages for segments 2 and 3 */

#define PAGE_SIZE 0x1000   /* 4 KB */

#define OS_PAGES 32   /* ARBITRARY!!!!! (but should be enough) */

/* The following definitions are useful to compute the overall space taken
   by the OS (see below) */

/* Tape DMA buffers */
#define OS_T_DMA_START  KSEGOS_BASE_MAP + (OS_PAGES * PAGE_SIZE)
#define OS_T_DMA_PAGES  DEV_PER_INT

/* Disk DMA buffers */
#define OS_D_DMA_START  OS_T_DMA_START + (OS_T_DMA_PAGES * PAGE_SIZE)
#define OS_D_DMA_PAGES  DEV_PER_INT

/* sys/bk stacks for the u-procs */
#define OS_USYSSTACK_START  OS_D_DMA_START + (OS_D_DMA_PAGES * PAGE_SIZE)
#define OS_USYSSTACK_PAGES  UPROCMAX

/* tlb stacks for the u-procs */
#define OS_UTLBSTACK_START  OS_USYSSTACK_START + (OS_USYSSTACK_PAGES * PAGE_SIZE)
#define OS_UTLBSTACK_PAGES  UPROCMAX

/* delay daemon stack */
#define OS_DSTACK_START  OS_UTLBSTACK_START + (OS_UTLBSTACK_PAGES * PAGE_SIZE)

/* Thus: */
#define KSEGOS_PAGES   OS_PAGES + OS_T_DMA_PAGES + OS_D_DMA_PAGES + OS_USYSSTACK_PAGES + OS_UTLBSTACK_PAGES + 1

#define SEGTABLE_START 0x20000500

#define PTE_MAGICNO 0x2a


/* Frame-pool definitions: play with the next to test VM */
#define FRAMEPOOL_FRAMES (2 * UPROCMAX)
/* #define FRAMEPOOL_FRAMES 14 */

#define FRAMEPOOL_SIZE (FRAMEPOOL_FRAMES * FRAME_SIZE)

#define FRAMEPOOL_END (RAMTOP - (2 * FRAME_SIZE))  /* the 2 stack frames */
#define FRAMEPOOL_START (FRAMEPOOL_END - FRAMEPOOL_SIZE)

/* Utility definitions */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define	HIDDEN static
#define	FALSE	0
#define	TRUE 1

#define NULL ((void *)0)

#define CR 0x0a   /* carriage return as returned by the terminal */

/* Number of pcb_t registers */
#define NREG 29

/* Number of syscall */
#define RANGE_SYSCALL 13

/* Exception State Vector */
#define MAX_STATE_VECTOR 3
#define ESV_TLB 0
#define ESV_PGMTRAP 1
#define ESV_SYSBP 2

/* isOnDev State */
#define IS_ON_DEV 1
#define IS_ON_PSEUDO 2
#define IS_ON_SEM 3

/* Status Word Table */
#define STATUS_WORD_ROWS 6
#define STATUS_WORD_COLS 8

/* Device Check Address */
#define DEV_CHECK_ADDRESS_0 0x1
#define DEV_CHECK_ADDRESS_1 0x2
#define DEV_CHECK_ADDRESS_2 0x4
#define DEV_CHECK_ADDRESS_3 0x8
#define DEV_CHECK_ADDRESS_4 0x10
#define DEV_CHECK_ADDRESS_5 0x20
#define DEV_CHECK_ADDRESS_6 0x40
#define DEV_CHECK_ADDRESS_7 0x80

/* Device Check Line */
#define DEV_CHECK_LINE_0 0
#define DEV_CHECK_LINE_1 1
#define DEV_CHECK_LINE_2 2
#define DEV_CHECK_LINE_3 3
#define DEV_CHECK_LINE_4 4
#define DEV_CHECK_LINE_5 5
#define DEV_CHECK_LINE_6 6
#define DEV_CHECK_LINE_7 7

/* Check Bit */
#define CHECK_FIFTH_BIT 	0x10
#define CHECK_EIGHTH_BIT 	0x80

/* Device Diff */
#define DEV_DIFF 3

/* Check Status Bit */
#define CHECK_STATUS_BIT 0xFF

#endif
