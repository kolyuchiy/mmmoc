/* MMMOC -- �祡��� ����樮���� ��⥬�
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * util.h: ��������� �㭪権 �� util.c.
 */

#include "types.h"

#define FALSE (0)
#define TRUE (!FALSE)

/* �८�ࠧ������ ��ᮢ᪮�� ���� � 䨧��᪨� 32-���� */
#define physical_addr(seg,offs) (((int_32)seg << 4) + (int)(offs))
/* �८�ࠧ������ ���� "ᥣ���:�����" � ���� ��� far jmp ��� far call */
#define call_addr(seg,offs) (((int_32)seg << 16) + (int)(offs))

/* ���⨥ ᥣ���� � ᬥ饭�� � far-pointer'� */
#define FP_SEG(fp)     ((unsigned)(void __seg *)(void __far *)(fp))
#define FP_OFF(fp)     ((unsigned)(fp))

/* �⮬��� ����樨 � ᥬ��ࠬ� */
#define wait_for(s)	 				\
	while (s==0) sleep();				\
	__asm lock dec s;

#define release(s)					\
	__asm lock inc s;


char *htoa(int value, char *string);
int atoi(const char *string);
void dump(int_16 value);
void dump_stack(void);
void dump_mem(int_16 __far *p, int n);
void debug_dump(int_16 seg, void *offs, int n, const char *info);
void __far *memcpy(void __far *result, const void __far *src, int nelem);
void __far *memset(void __far *buffer, int character, int nelem);
void __far *wmemset(void __far *buffer, int character, int nelem);
int strcmp(const char *s1, const char *s2);
void parse_command(const char *strin, char *command, char *arg);

