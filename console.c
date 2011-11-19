/* MMMOC -- учебная операционная система
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * console.c: Вывод на консоль.
 */

#include "console.h"
#include "sys.h"
#include "util.h"
#include "keymap.h"

/* Буфер клавиатуры (FIFO) */
keybbuf_t keybbuf;

/* Позиция курсора */
int pos_x = 0;
int pos_y = 0;

/* Дальний указатель на видеобуфер */
videobuf_t __far *videobuf;

/* Таблица перевода скан-кодов в символы.
 * скан-код -- индекс
 */
char sc2char[] = {0,
	27,'1','2','3','4','5','6','7','8','9','0','-','=',0,9,
	'q','w','e','r','t','y','u','i','o','p','[',']',13,0,
	'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
	'z','x','c','v','b','n','m',',','.','/',0,
	'*',0,' ',0,0,0,0,0,0,0,0,0,0,0,0,0,
	'7','8','9','-','4','5','6','+','1','2','3','0','.',0,
	0,'<',0,0,0,0,0,0,0,0,0,
	13,0,'/',0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* Символ курсора */
#define SCR_CURSOR ('_')

/* Инициализация консоли */
void init_console(void)
{
	FP_SEG(videobuf) = gdt_add_descriptor(physical_addr(0xb800,0),
		4000-1,SDA_TYPE_DATARW|SDA_DEFAULT,SDA_GRAN_DEFAULT);
	FP_OFF(videobuf) = 0;
	clrscr();
	clrstatus();
}

/* Очистка экрана */
void clrscr(void) 
{
	memset(videobuf,0,SCR_X*SCR_Y*sizeof(charcell_t));
	pos_x = 0;
	pos_y = 0;
}

/* Очистка строки состояния */
void clrstatus(void)
{
	wmemset((*videobuf)[SCR_NFO],0x7020,SCR_X);
}

/* Очистка строки ввода */
void clrinput(void)
{
	memset((*videobuf)[SCR_IN],0,SCR_X*sizeof(charcell_t));
}

/* Сдвиг экрана на строку вверх */
void scr_up(void)
{
	memcpy(videobuf,(*videobuf)[1],SCR_X*(SCR_Y-1)*sizeof(charcell_t));
	memset((*videobuf)[SCR_Y-1],0,SCR_X*sizeof(charcell_t));
}

/* Запись строки в строку состояния */
void putstatus(const char *p, int pos)
{
	while(*p!=0) {
		if(pos>=SCR_X) return;
		(*videobuf)[SCR_NFO][pos].character = *p;
		pos++;
		p++;
	}
}

/* Вывести на экран символ в текущую позицию */
#define putc(col,c) 					\
	(*videobuf)[pos_y][pos_x].character = c;	\
	(*videobuf)[pos_y][pos_x].color = col;

int mutex = 1;

/* Выводит на экран строку в текущую позицию */
void puts(const char *p) 
{
	wait_for(mutex);
	putc(COL_GRAY,0);
	while(*p!=0) {
		if(*p=='\n') { pos_x = 0; pos_y++; }
		else { putc(COL_GRAY,*p); pos_x++; }
		p++;

		if(pos_x>=SCR_X) { pos_x = 0; pos_y++; }
		if(pos_y>=SCR_Y) { scr_up(); pos_y--; }
	}
	putc(COL_GRAY|COL_LIGHT,SCR_CURSOR);
	release(mutex);
}

/* Запомнить введенный скан-код в буфере */
void store_key(int sc)
{
	if(keybbuf.putp+1!=keybbuf.getp) {
		keybbuf.buf[keybbuf.putp] = sc;
		keybbuf.putp = (keybbuf.putp+1)%KBBUFLEN;
	}
}

/* Получение скан кода из буфера. */
int getsc(void)
{
	int ret;
	while(keybbuf.putp==keybbuf.getp) sleep();
	ret = keybbuf.buf[keybbuf.getp];
	keybbuf.getp = (keybbuf.getp+1)%KBBUFLEN;
	return ret;
}

/* Вывести в строке ввода символ в текущую позицию */
#define putcin(col,c) 					\
	(*videobuf)[SCR_IN][top].character = c;	\
	(*videobuf)[SCR_IN][top].color = col;

/* Ввод строки */
char *gets(char *buffer)
{
	int top = 0;

	clrinput();	
	while(TRUE) {
		int key = getsc();

		if(key==SC_Return||key==SC_KP_Enter) break;
		else if(key==SC_Delete) { putcin(COL_GRAY,0); top--; } 
		else if(sc2char[key]==0) continue;
		else {
			buffer[top] = sc2char[key];
			putcin(COL_GRAY,buffer[top]);
			top++;
		}

		if(top>=SCR_X) top--;
		if(top<0) top = 0;

		putcin(COL_GRAY|COL_LIGHT,SCR_CURSOR);
	}
	buffer[top] = 0;

	return buffer;
}
