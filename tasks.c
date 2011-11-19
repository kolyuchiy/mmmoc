/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * tasks.c: Реализация многозадачности.
 */

#include "tasks.h"
#include "util.h"
#include "console.h"

task_t tasks[NRTASKS];
extern int_16 real_data;

int create_task(void (*task)())
{
	int n = 1;
	while(tasks[n].task.state!=TS_NOTAVAIL && n!=NRTASKS) n++;
	if(n==NRTASKS) return 0;

	memset(&(tasks[n].task),0,sizeof(task_t));

	(int_16)tasks[n].task.eip = (int_16)task;
	tasks[n].task.eflags = FL_DEFAULT|FL_IF;
	tasks[n].task.ldt_ss = gdt_add_descriptor(
		physical_addr(real_data,&tasks[n].task.ldt),
		sizeof(ldt_t)-1,
		SDA_DPL_0|SDA_PRESENT|SDA_TYPE_LDT,SDA_GRAN_DEFAULT);
	tasks[n].task.cs = KCODE_SEL;
	tasks[n].task.ds = KDATA_SEL;
	tasks[n].task.ss = ldt_add_descriptor(
		tasks[n].task.ldt,
		physical_addr(real_data,&(tasks[n].stack)), TASK_STACK_SIZE-1,
		SDA_TYPE_DATARW|SDA_DEFAULT,SDA_GRAN_DEFAULT);
	tasks[n].task.esp = TASK_STACK_SIZE;
	tasks[n].task.ss0 = KSTACK_SEL;
	tasks[n].task.esp0 = TASK_STACK_SIZE;

	/* Нестандартные поля */
	tasks[n].task.selector = gdt_add_descriptor(
		physical_addr(real_data,&(tasks[n].task)),
		sizeof(tss_t)-1, SDA_TSS,SDA_GRAN_DEFAULT);
	tasks[n].task.state = TS_RUNNING;
	puts("- 3aga4a #"); dump(n); puts(" 3anyLLLeHa.\n");
	return n;
}

extern int mutex;

void kill_task(int n)
{
	if(n>=NRTASKS || tasks[n].task.state==TS_NOTAVAIL || n==0) return;
	wait_for(mutex);
	release(mutex);
	tasks[n].task.state = TS_NOTAVAIL;
	free_selector(tasks[n].task.ldt_ss);
	free_selector(tasks[n].task.selector);
	puts("- 3aga4a #"); dump(n); puts(" 3aBepLLIeHa.\n");
}

void suspend_task(int n)
{
	if(n>=NRTASKS || tasks[n].task.state!=TS_RUNNING || n==0) return;
	wait_for(mutex);
	release(mutex);
	tasks[n].task.state = TS_SUSPENDED;
	puts("- 3aga4a #"); dump(n); puts(" npuocTaHoBJIeHa.\n");
}

void resume_task(int n)
{
	if(n>=NRTASKS || tasks[n].task.state!=TS_SUSPENDED) return;
	tasks[n].task.state = TS_RUNNING;
	puts("- 3aga4a #"); dump(n); puts(" BoccTaHoBJIeHa.\n");
}

void exit(int n)
{
	static int_32 go_task;
	int curr, sel;

	/* Узнаем какая задача запущена по
	 * значению TR */
	__asm	str	ax;
	sel = _AX;
	for(curr=0;curr<NRTASKS;curr++)
		if(tasks[curr].task.selector==sel) break;
	puts("- 3aga4a #"); dump(curr); 
	puts(" 3aBepLLIuJIacb c KogoM "); dump(n); puts(".\n");

	tasks[curr].task.state = TS_NOTAVAIL;
	free_selector(tasks[curr].task.ldt_ss);
	free_selector(tasks[curr].task.selector);

	/* switch to 0 */
	go_task = call_addr(tasks[0].task.selector,0);
	__asm	jmp	dword ptr go_task;
}

void do_timer(void)
{
	int_16 sel;
	int curr_task = 0;
	int next_task = 0;
	static int clock_ticks = 0;
	clock_ticks++;

	/* Узнаем какая задача запущена по
	 * значению TR */
	__asm	str	ax;
	sel = _AX;
	for(curr_task=0;curr_task<NRTASKS;curr_task++)
		if(tasks[curr_task].task.selector==sel) break;
	/* Если ни одна задача не подходит, то по умолчанию
	 * curr_task = 0 -- ядро
	 */

	/* Находим следующую запущенную задачу и
	 * переключаемся к ней, если она не та же, что
	 * и текущая */
	next_task = (curr_task+1)%NRTASKS;
	while(tasks[next_task].task.state!=TS_RUNNING) {
		next_task = (next_task+1)%NRTASKS;
		if(next_task==curr_task) break;
	}
	if(next_task!=curr_task) switch_to(next_task);
}

/* Переключиться к задаче n */
int switch_to(int n)
{
	/* Проверим, запущена ли задача */
	if(tasks[n].task.state!=TS_RUNNING) return FALSE;
	else {
		static int_32 go_task;
		go_task = call_addr(tasks[n].task.selector,0);
		__asm	jmp	dword ptr go_task;
	}

	return TRUE;
}

/* Вывести на экран список активных процессов */
void process_list(void)
{
	int i;
	for(i=0;i<NRTASKS;i++)
	if(tasks[i].task.state!=TS_NOTAVAIL) {
		dump(i); puts(": selector ");
		dump(tasks[i].task.selector);
		puts("   state ");
		if(tasks[i].task.state==TS_RUNNING)
			puts("TS_RUNNING");
		else if(tasks[i].task.state==TS_SUSPENDED)
			puts("TS_SUSPENDED");
		puts("\n");
	}
}

void init_kernel_task(void)
{
	tasks[0].task.selector = KTASK_SEL;
	tasks[0].task.state = TS_RUNNING;
}
