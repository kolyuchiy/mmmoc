/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * console.h: Заголовки для console.c
 */

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "types.h"

/* Размер буфера клавиатуры */
#define KBBUFLEN (16)

/* Размеры экрана: 80x23
 * 24-я строка -- информационная;
 * 25-я строка -- устройство ввода.
 */
#define SCR_X	(80)
#define SCR_Y	(23)
#define SCR_NFO	(23)
#define SCR_IN	(24)

/* Цвета переднего плана */
#define COL_BLACK	(0x00)
#define COL_BLUE	(0x01)
#define COL_GREEN	(0x02)
#define COL_RED		(0x04)
#define COL_GRAY	(0x07)
/* Яркость */
#define COL_LIGHT	(0x08)

/* Цвета фона */
#define BG_BLACK	(0x00)
#define BG_BLUE		(0x10)
#define BG_GREEN	(0x20)
#define BG_RED		(0x40)
/* Мерцание */
#define BG_BLINK	(0x80)

typedef struct {
	int_8 character;
	int_8 color;
} charcell_t;

typedef charcell_t videobuf_t[SCR_Y][SCR_X];

typedef struct {
	int_8 buf[KBBUFLEN];
	int getp;
	int putp;
} keybbuf_t;

void clrscr(void);
void clrstatus(void);
void puts(const char *string);
void putstatus(const char *p, int pos);
void init_console(void);
int getsc(void);
char *gets(char *buffer);

#endif /* _CONSOLE_H */
