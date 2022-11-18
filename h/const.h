#ifndef CONSTS
#define CONSTS

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define RAMBASESIZE		0x10000004
#define TODLOADDR		0x1000001C
#define INTERVALTMR		0x10000020
#define TIMESCALEADDR		0x10000024


/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'

#define NULL 			    ((void *)0xFFFFFFFF)

/* device interrupts */
#define DISKINT			  3
#define FLASHINT 		  4
#define NETWINT 		  5
#define PRNTINT 		  6
#define TERMINT			  7

#define DEVNUM            	  49
#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */
#define DEVREGSIZE	  	  16 		  /* device register size in bytes */
#define DEVCOUNT		  (DEVPERINT * DEVINTNUM)

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			  2
#define DATA1			  3

/* device register field number for terminal devices */
#define RECVSTATUS  	0
#define RECVCOMMAND 	1
#define TRANSTATUS  	2
#define TRANCOMMAND 	3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			1
#define BUSY			3

/* device common COMMAND codes */
#define RESET			0
#define ACK			1

#define STATEREGNUM		31

/* Memory related constants */
#define KSEG0           0x00000000
#define KSEG1           0x20000000
#define KSEG2           0x40000000
#define KUSEG           0x80000000
#define RAMSTART        0x20000000
#define BIOSDATAPAGE    0x0FFFF000
#define	PASSUPVECTOR	  0x0FFFF900 /* The pass up vector is where the BIOS finds the address of the Nucleus functions to pass control */

/* Exceptions related constants */
#define	PGFAULTEXCEPT	  0
#define GENERALEXCEPT	  1

#define CAUSEMASK	  0xFF

/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR)))

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))
#define MAXPROC 20
#define IOCLOCK 100000 /* aka 100 ms */
#define QUANTUM 5000
#define INTERVAL

/* syscalls */
#define CREATEPROCESS 1
#define TERMINATEPROCESS 2
#define PASSEREN 3
#define VERHOGEN 4
#define WAITIO 5
#define GETCPUTIME 6
#define WAITCLOCK 7
#define GETSUPPORTPTR 8

/* important places */
#define NUCLEUSSTACKPAGE 0x20001000
#define STATUSREG 0x10400000

/* bit operations */
#define ALLOFF 0x00000000
#define IEON 0x00000004
#define IECON	0x00000001
#define IMON 0x0000FF00
#define TEBITON 0x08000000
#define UMOFF 0x00000002
#define KUON 0x00000008

/* masks */
#define EXCODEMASK 0x0000007C
#define IPMASK 0x00FF00
#define LINE0INTON 1
#define LINE1INTON 2
#define LINE2INTON 4
#define LINE3INTON 8
#define LINE4INTON 16
#define LINE5INTON 32
#define LINE6INTON 64
#define LINE7INTON 128
#define TRANSBITS 15


#define PAGEMAX		32


/* Support for EntryHi */
#define TURNOFFVPNBITS 	0x3FFFF000 /* The virtual page number (VPN) bits in the EntryHi or EntryLo addresses are turned off since this hex is 111111111111111111000000000000 and the VPN bits are bits 12-31. */
#define VIRTSHIFT	12

#define PCINC 4
#define ZERO 0
#define ONE 1
#define ON 1
#define OFF 0
#define SHIFT 2


#define IOINTERRUPT 0
#define TLBEXCEPTION 3
#define SYSEXCEPTION 8

#define USERPROCMAX	8
#define POOLSIZE		(USERPROCMAX * 2)  

/* phase 3 constants */
#define MAXUPROC 1
#define PRINTCHR	2
#define RECVD 5
#define ASIDSHIFT 6
#define BYTELENGTH	8
#define TERMINATE 9
#define WRITETOPRINTER 11
#define WRITETOTERMINAL 12
#define READFROMTERMINAL 13
#define PGTABLESIZE 32
#define	TERM0ADDR 0x10000254
#define TERMSTATMASK 0xff
#define DIRTYON 0x00000400
#define VALIDON 0x00000200
#define GETPAGENO 0x00007000
#define GETASID 0x00000FC0
#define SWPSTARTADDR 0x20020000


#endif
