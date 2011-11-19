/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * tasks.h: Заголовки tasks.c.
 */

#ifndef _TASKS_H
#define _TASKS_H

#include "sys.h"
#include "types.h"

/* Размер стека задачи */
#define TASK_STACK_SIZE (0x200)
/* Количество задач */
#define NRTASKS (5)

typedef union {
	int_32 stack[TASK_STACK_SIZE/sizeof(int_32)];
	tss_t task;
} task_t;

int create_task(void (*task)());
int switch_to(int n);
void process_list(void);
void init_kernel_task(void);
void kill_task(int n);
void suspend_task(int n);
void resume_task(int n);
void exit(int n);

#endif /* _TASKS_H */
