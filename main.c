/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * main.c: Главный модуль.
 */

#include "util.h"
#include "keymap.h"
#include "sys.h"
#include "console.h"
#include "tasks.h"

#define VERSION "Bac npuBeTcTByeT MMMOC v0.10.5\n" 			\
                "build: "__TIME__", "__DATE__"\n"
#define GOODBYE "DocBuDaHu9I!!!\n"
#define NOSUCHCOMMAND "0LLIu6Ka: HeT TaKou KoMaHDbI. (\"help\" -- cnpaBKa)\n"
#define TTFULL "0LLIu6Ka: Hem MecTa B Ta6JIu,e 3aga4.\n"
#define ERR_THFUNCNAME "0LLIu6Ka: Hem maKou 3aga4u.\n"
#define HELP 								\
"-- MMMOC noDDep>|<uBaeT cJIeDy|-OLLLue KoMaHgbI:\n\n"			\
"help: BbIBecTu cnpaBky\n"						\
"ver: BbIBecTu Bepcu|-O OC\n"						\
"cls: o4ucTuTb -)KpaH\n"						\
"ls: cnucoK uMeH 3aga4\n"						\
"task name: 3anycK 3aga4u \"name\"\n"					\
"kill n: 3aBepLLIuTb 3aga4y n\n"					\
"susp n: npuocTaHoBuTb 3aga4y n\n"					\
"resm n: npogoJI>|<uTb 3aga4y n\n"					\
"ps: BbIBecTu cnucoK 3anyLLLeHHbI>< npou,eccoB\n"			\
"tss0: npocMoTp TSS0 (npou,ecc 9Igpa)\n"				\
"gdt: npocMoTp GDT\n"							\
"exit: 3aBepLLIuTb MMMOC\n"						\
"div0: npoBepKA ucKJI|-O4eHu9I \"DeJIeHue Ha HoJIb\"\n"			\
"gpf: npoBepKA ucKJI|-O4eHu9I \"HapyLLIeHue o6LLLeu 3aLLLuTbI\"\n"	\
"pf: npoBepKA ucKJI|-O4eHu9I \"cTpaHu4HoE HapyLLIeHue\"\n"

void thread1(void)
{
	char *str = ":: 3aga4a #1 ::  \n";
	while(TRUE) {
		call_service(puts_addr,str);
		str[16] = str[16] % 2 + 1;
		sleep();
	}
}

void thread2(void)
{
	char *str = ":: 3aga4a #2 ::  \n";
	while(TRUE) {
		call_service(puts_addr,str);
		str[16] = str[16] % 2 + 1;
		sleep();
	}
}

void thread3(void)
{
	char *str = ":: 3aga4a #3 ::  \n";
	while(TRUE) {
		call_service(puts_addr,str);
		str[16] = str[16] % 2 + 1;
		sleep();
	}
}

void fib(void)
{
	char *s1 = ":fib: BbI4ucJIeHue nepBbIx 10-mu 4uceJI (|)u6oHa44u\n";
	char *seol = "\n";
	int i, fib1 = 1, fib2 = 1;
	call_service(puts_addr,s1);
	for(i=0;i<5;i++) {
		dump(fib1); call_service(puts_addr,seol);
		dump(fib2); call_service(puts_addr,seol);
		fib1 += fib2; fib2 += fib1;
		sleep();
	}
	call_service(exit_addr,fib1);
}

typedef struct {
	char name[9];
	void (*func)();
} thread_func_t;

thread_func_t threads[] = {
	{"th1", thread1},
	{"th2", thread2},
	{"th3", thread3},
	{"fib", fib}
};

#define NRTHFUNC	(sizeof(threads)/sizeof(thread_func_t))

void kernel(void)
{
	init_kernel_task();
	init_console();
	setup_api();
	setup_idt();

	puts(VERSION);
	putstatus("-- MMMOC --",2);
	while(TRUE) {
		static char strin[SCR_X+1];
		static char str[SCR_X+1];
		static char arg[SCR_X+1];

		gets(strin);
		parse_command(strin,str,arg);		

		if (strcmp(str,"exit"))	break;
		else
		if (strcmp(str,"task")) {
			int n = 0;
			while(!strcmp(threads[n].name,arg) && n!=NRTHFUNC) n++;
			if(n==NRTHFUNC)	puts(ERR_THFUNCNAME);
			else if(!create_task(threads[n].func)) puts(TTFULL);
		} else
		if (strcmp(str,"ls")) {
			int i;
			puts("3aga4u:\n");
			for(i=0;i<NRTHFUNC;i++) {
				puts(threads[i].name);
				puts("\n");
			}
		} else
		if (strcmp(str,"kill")) kill_task(atoi(arg));
		else
		if (strcmp(str,"susp")) suspend_task(atoi(arg));
		else
		if (strcmp(str,"resm")) resume_task(atoi(arg));
		else
		if (strcmp(str,"ps"))	process_list();
		else
		if (strcmp(str,"tss0")) {
			extern task_t tasks[NRTASKS];
			debug_dump(_DS,&tasks,sizeof(tss_t),"TSS_0");
		} else
		if (strcmp(str,"gdt")) {
			extern seg_desc_t gdt[NOGD];
			debug_dump(_DS,&gdt,sizeof(seg_desc_t)*NOGD,"GDT");
		} else
		if (strcmp(str,"div0")) {
			int a = 1;
			a /= 0;
		} else
		if (strcmp(str,"pf"))	__asm int 14;
		else
		if (strcmp(str,"gpf"))	__asm mov es:0ffffh,ax;
		else
		if (strcmp(str,"cls"))	clrscr();
		else
		if (strcmp(str,"help"))	puts(HELP);
		else
		if (strcmp(str,"ver"))	puts(VERSION);
		else			puts(NOSUCHCOMMAND);
	}
	puts(GOODBYE);
}
