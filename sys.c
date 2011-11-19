/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * sys.c: Функции взаимодействия с системными структурами.
 */

#include "console.h"
#include "sys.h"
#include "util.h"

/* Обработчики прерываний */
extern void exc_0(void);
extern void exc_0ah(void);
extern void exc_0bh(void);
extern void exc_0ch(void);
extern void exc_0dh(void);
extern void exc_0eh(void);
extern void new_08h(void);
extern void new_09h(void);
extern void master(void);
extern void slave(void);
extern void dummy_exc(void);
                   
extern gate_desc_t idt[NOINT];
extern seg_desc_t gdt[NOGD];

/* API */
int_32 puts_addr;
int_32 exit_addr;

void set_interrupt_handler(int n, void (*handler)(), int_16 iattr) 
{
	idt[n].offset_0_15 = (int_16)handler;
	idt[n].seg_selector = HND_SEL;
	idt[n].reserved = 0;
	idt[n].attr = iattr;
	idt[n].offset_16_31 = 0;
}

int gdt_add_gate(void (*func)(), int_16 sel, int_16 iattr, int narg) 
{
	int n = 1;
	while(gdt[n].lim_0_15!=0 && n!=NOGD) n++;
	if(n==NOGD) return 0;

	((gate_desc_t*)gdt)[n].offset_0_15 = (int_16)func;
	((gate_desc_t*)gdt)[n].seg_selector = sel;
	((gate_desc_t*)gdt)[n].reserved = narg;
	((gate_desc_t*)gdt)[n].attr = iattr;
	((gate_desc_t*)gdt)[n].offset_16_31 = 0;

	return n*8;
}

int gdt_add_descriptor(int_32 base, int_16 limit,
	int_8 rights, int_8 granularity)
{
	int n = 1;
	while(gdt[n].lim_0_15!=0 && n!=NOGD) n++;
	if(n==NOGD) return 0;

	gdt[n].lim_0_15 = limit;
	gdt[n].bas_0_15 = base & 0x0000ffff;
	gdt[n].bas_16_23 = (base & 0x00ff0000) >> 16;
	gdt[n].access = rights;
	gdt[n].gran = granularity;
	gdt[n].bas_24_31 = (base & 0xff000000) >> 24;

	return n*8;
}

int ldt_add_descriptor(ldt_t ldt,
	int_32 base, int_16 limit,
	int_8 rights, int_8 granularity)
{
	int n = 0;
	while(ldt[n].lim_0_15!=0 && n!=NOLD) n++;
	if(n==NOLD) return 0;

	ldt[n].lim_0_15 = limit;
	ldt[n].bas_0_15 = base & 0x0000ffff;
	ldt[n].bas_16_23 = (base & 0x00ff0000) >> 16;
	ldt[n].access = rights;
	ldt[n].gran = granularity;
	ldt[n].bas_24_31 = (base & 0xff000000) >> 24;

	return n*8+4;
}

void free_selector(int n)
{
	n = n/8;
	if(n>=NOGD) return;
	gdt[n].lim_0_15 = 0;
}

void setup_idt(void) 
{
	int i;

	set_interrupt_handler(0x00,&exc_0,IDT_TRAP);
	for(i=0x01;i<=0x09;i++)
		set_interrupt_handler(i,&dummy_exc,IDT_TRAP);
	set_interrupt_handler(0x0a,&exc_0ah,IDT_TRAP);
	set_interrupt_handler(0x0b,&exc_0bh,IDT_TRAP);
	set_interrupt_handler(0x0c,&exc_0ch,IDT_TRAP);
	set_interrupt_handler(0x0d,&exc_0dh,IDT_TRAP);
	set_interrupt_handler(0x0e,&exc_0eh,IDT_TRAP);
	for(i=0x0f;i<=0x1f;i++)
		set_interrupt_handler(i,&dummy_exc,IDT_TRAP);
	set_interrupt_handler(0x20,&new_08h,IDT_INT);
	set_interrupt_handler(0x21,&new_09h,IDT_INT);
	for(i=0x22;i<=0x27;i++)
		set_interrupt_handler(i,&master,IDT_INT);
	for(i=0x28;i<=0x2f;i++)
		set_interrupt_handler(i,&slave,IDT_INT);

	/* Размаскируем NMI и аппаратные прерывания */
	__asm	mov	al,0;
	__asm	out	70h,al;
	__asm	sti;
}

void setup_api(void)
{
	extern void puts_serv(void);
	extern void exit_serv(void);
	int sel;

	sel = gdt_add_gate(puts_serv,KCODE_SEL,CGATE_DEFAULT,1);
	puts_addr = call_addr(sel,0);

	sel = gdt_add_gate(exit_serv,KCODE_SEL,CGATE_DEFAULT,1);
	exit_addr = call_addr(sel,0);
}

/* Остановить процессор. */
void halt_cpu(void)
{
	__asm	cli;
	__asm	mov     dx,0a1h;         /* Порт ведомого контроллера */
	__asm	mov     al,0ffh;         /* Маска прерываний */
	__asm	out     dx,al;           /* замаскируем все прерывания */
	puts("CPU halted. Shutdown.");
	sleep();
}

