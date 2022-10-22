#ifndef CONSTS
#define CONSTS

/****************************************************************************
 *
 * This header file contains utility constants & macro definitions.
 *
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR 	0x10000000 /* RAM base physical address bus register located at 0x1000000 */
#define RAMBASESIZE		0x10000004
#define TODLOADDR		  0x1000001C
#define INTERVALTMR		0x10000020
#define TIMESCALEADDR	0x10000024


/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'
#define MAXPROC       20        /* Maximum concurrent processes */
#define MAXINT        0xFFFFFFFF

#define NULL 			    ((void *)0xFFFFFFFF)


/* Device constants */
#define PLT       1
#define INTERVAL  2
#define DISK      3
#define FLASH     4
#define NETWORK   5
#define PRINTER   6
#define TERMINAL  7

/* Device Interrupt constants */
#define PLTINT      0x00000200
#define INTERVALINT 0x00000400
#define DISKINT     0x00000800
#define FLASHINT    0x00001000
#define NETWINT     0x00002000
#define PRNTINT     0x00004000
#define TERMINT     0x00008000

#define DEVREG0     0x00000001
#define DEVREG1     0x00000002
#define DEVREG2     0x00000004
#define DEVREG3     0x00000008
#define DEVREG4     0x00000010
#define DEVREG5     0x00000020
#define DEVREG6     0x00000040
#define DEVREG7     0x00000080

#define NUMBEROFDEVICES 49 /* p. 28 of pops says there are 40 device registers in uMPS. On p. 41 of pops, it says there are 8 serial terminal device interfaces. Plus 1 for timer*/


#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */
#define DEVREGSIZE	  16 		/* device register size in bytes */

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			    2
#define DATA1			    3

/* device register field number for terminal devices */
#define RECVSTATUS  	0
#define RECVCOMMAND 	1
#define TRANSTATUS  	2
#define TRANCOMMAND 	3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			    1
#define BUSY			    3

/* device common COMMAND codes */
#define RESET			    0
#define ACK				    1

/* Memory related constants */
#define KSEG0           0x00000000
#define KSEG1           0x20000000
#define KSEG2           0x40000000
#define KUSEG           0x80000000
#define RAMSTART        0x20000000
#define BIOSDATAPAGE    0x0FFFF000
#define	PASSUPVECTOR	  0x0FFFF900

/* messing with bits */
#define	ALLOFF 0x00000000
#define	IECON 0x00000001 /* interrupt current on */
#define	IEON 0x00000004 /* interrupts on */
#define TEBITON 0x08000000  /* enable the processor Local Timer */
#define	IMON 0x0000FF00 /* turn on interrupt mask */
#define	KUON 0x00000008 /* Kernel mode on */
#define USERMODEOFF 0x00000002 /* User mode off */

/* Exceptions related constants */
#define	PGFAULTEXCEPT	  0
#define GENERALEXCEPT	  1


/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR)))

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
/* The time slice value is 5ms. A QUANTUM is the "time used by scheduling algorithms as a basis for determining when to preempt a thread from the CPU to allow another to run." - Operating Systems Concepts*/
#define QUANTUM 5000


#define NUCLEUSSTACKPAGE 0x20001000

/* "The Time Scale’s value indicates the number of clock ticks that will occur in a microsecond," p.21 pandos.
So 100 milliseconds is 100,000 microseconds. */
#define INTERVALTIMER 100000

#endif
