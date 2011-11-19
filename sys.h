/* MMMOC -- �祡��� ����樮���� ��⥬�
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * sys.h: ��������� �㭪権 �� sys.c, ���ᠭ�� ��⥬��� �������.
 */

#ifndef _SYS_H
#define _SYS_H

#include "types.h"

#define HND_SEL (0x08)		/* ������� ᥣ���� ��ࠡ��稪�� ���뢠��� */
#define KCODE_SEL (0x08)	/* ������� ���� �� */
#define KDATA_SEL (0x10)	/* ������� ������ �� */
#define KSTACK_SEL (0x18)	/* ������� �⥪� �� */
#define KTASK_SEL (0x20)	/* ������� TSS �� */
#define NOINT (256)		/* ������⢮ ���뢠��� � IDT */
#define NOGD (64)		/* ������⢮ ���ਯ�஢ � GDT */
#define NOLD (4)		/* ������⢮ ���ਯ�஢ � LDT */

/* ���ਡ��� � ���뢠��� */
#define IDT_INT  (0x8e)		/* ���뢠��� */
#define IDT_TRAP (0x8f)		/* ����誠 */

/* ���ਡ��� ���ਯ�� ᥣ���� */
#define SDA_GRAN_DEFAULT	(0x00)		/* �࠭��୮��� 1 ���� */
#define SDA_TYPE_DATARW		(0x02)		/* �����, r/w */
#define SDA_TYPE_LDT		(0x02)		/* LDT */
#define SDA_SYSTEM		(0x10)		/* ������� ������/���� */
#define SDA_DPL_0		(0x00)		/* �஢��� �ਢ������ */
#define SDA_PRESENT		(0x80)		/* ��������� � ����� */
#define SDA_DEFAULT		(SDA_DPL_0|SDA_PRESENT|SDA_SYSTEM)
#define SDA_TSS			(0x89)

/* ���ਡ��� ���ਯ�� Call Gate'� */
#define SDA_TYPE_CGATE		(0x0c)
#define CGATE_DEFAULT		(SDA_PRESENT|SDA_TYPE_CGATE)

typedef struct {
	int_16	lim_0_15;
	int_16	bas_0_15;
	int_8	bas_16_23;
	int_8	access;
	int_8	gran;
	int_8	bas_24_31;
} seg_desc_t;

typedef seg_desc_t ldt_t[NOLD];

typedef struct {
	int_16 prev;
	int_16 not_used_1;
	int_32 esp0;
	int_16 ss0;
	int_16 not_used_9;
	int_32 esp1;
	int_16 ss1;
	int_16 not_used_17;
	int_32 esp2;
	int_16 ss2;
	int_16 not_used_25;
	int_32 cr3;
	int_32 eip;
	int_32 eflags;
	int_32 eax;
	int_32 ecx;
	int_32 edx;
	int_32 ebx;
	int_32 esp;
	int_32 ebp;
	int_32 esi;
	int_32 edi;
	int_16 es;
	int_16 not_used_73;
	int_16 cs;
	int_16 not_used_77;
	int_16 ss;
	int_16 not_used_81;
	int_16 ds;
	int_16 not_used_85;
	int_16 fs;
	int_16 not_used_89;
	int_16 gs;
	int_16 not_used_93;
	int_16 ldt_ss;
	int_16 not_used_97;
	int_16 debug;
	int_16 io_map_base;

	/* Custom fields */
	int_16 selector;
	int state;
	ldt_t ldt;
} tss_t;

/* ����⠭�� ��� tss.state */
#define TS_NOTAVAIL	(0)
#define TS_RUNNING	(1)
#define TS_SUSPENDED	(2)

typedef struct {
	int_16	offset_0_15;
	int_16	seg_selector;
	int_8	reserved;
	int_8	attr;
	int_16	offset_16_31;
} gate_desc_t;

/* ���� ॣ���� EFLAGS */
#define FL_CF		(0x1)		/* Carry Flag */
#define FL_DEFAULT	(0x2)		/* �� 㬮�砭�� ��� 1 �ᥣ�� = 1 */
#define FL_PF		(0x4)		/* Parity Flag */
#define FL_AF		(0x10)		/* Adjust Flag */
#define FL_ZF		(0x40)		/* Zero Flag */
#define FL_SF		(0x80)		/* Sign Flag */
#define FL_TF		(0x100)		/* Trap Flag */
#define FL_IF		(0x200)		/* Interrupt Enable Flag */
#define FL_DF		(0x400)		/* Direction Flag */
#define FL_OF		(0x800)		/* Overflow Flag */
#define FL_IOPL_12	(0x1000)	/* IO Privilege Level */
#define FL_IOPL_13	(0x2000)	/* IO Privilege Level */
#define FL_NT		(0x4000)	/* Nested Task Flag */
#define FL_RF		(0x10000)	/* Resume Flag */        
#define FL_VM		(0x20000)	/* Virtual-8086 Mode */        
#define FL_AC		(0x40000)	/* Alignment Check */        
#define FL_VIF		(0x80000)	/* Virtual Interrupt Flag */        
#define FL_VIP		(0x100000)	/* Virtual Interrupt Pending */        
#define FL_ID		(0x200000)	/* Identification Flag */        

/* ��⠭�� ������ � �������� ���뢠��� */
#define sleep()	__asm	hlt

/* �맮� �ࢨ� */
#define call_service(addr,arg)					\
	__asm	push	arg;					\
	__asm	call	dword ptr addr;				\
	__asm	add	sp,2;

/* API */
extern int_32 puts_addr;
extern int_32 exit_addr;

void set_interrupt_handler(int n, void (*handler)(), int_16 iattr);
void setup_idt(void);
int gdt_add_descriptor(int_32 base, int_16 limit,
	int_8 rights, int_8 granularity);
int ldt_add_descriptor(seg_desc_t ldt[NOLD],
	int_32 base, int_16 limit,
	int_8 rights, int_8 granularity);
void free_selector(int n);
int gdt_add_gate(void (*func)(), int_16 sel, int_16 iattr, int narg);
void setup_api(void);

#endif /* _SYS_H */
