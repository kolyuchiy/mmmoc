/* MMMOC -- �祡��� ����樮���� ��⥬�
 * (c) 2003, Kolia Morev <kolia39@mail.ru>
 *
 * util.c: ������� �㭪樨.
 */

#include "types.h"
#include "util.h"
#include "console.h"
#include "tasks.h"

/* ��ॢ���� ��⭠����筮� ᫮�� � ��ப���� 
 * �।�⠢�����.
 */
char *htoa(int value, char *string) 
{
	int i;

	for(i=sizeof(int)*2-1;i>=0;i--) {
		int j = value & 0xf;
		string[i] = j<10 ? j+'0' : j-10+'a';
		value >>= 4;
	}
	string[sizeof(int)*2] = 0;

	return string;
}

/* ��ॢ���� ��ப���� �।�⠢����� ��⭠����筮�� �᫠ � �᫮ */
int atoi(const char *s)
{
	int ret = 0;
	while(TRUE) {
		if(*s==0) break;
		else if(*s>='0' && *s<='9') { ret <<= 4; ret += *s-'0'; }
		else if(*s>='a' && *s<='f') { ret <<= 4; ret += *s-'a'+10; }
		else return 0;
		s++;
	}
	return ret;
}

/* ���� ᫮�� � ��������� */
void dump(int_16 value)
{
	char str[5];
	htoa(value,str);
	puts(str);
}

/* ���� �⥪� */
void dump_stack(void)
{
	puts("stack:\n");
	__asm	add	bp,4;	/* �ய��⨬ ���� ������ � bp */
	while(_BP<TASK_STACK_SIZE) {
		__asm	mov	ax,[bp];
		dump(_AX); puts(" ");
		_BP += 2;
	}
	puts("\n");
}

/* ���� ������ �����. (n ����) */
void dump_mem(int_16 __far *p, int n)
{
	int i;

	puts("dump "); dump((n+1)/sizeof(int_16)); 
	puts(" words from "); dump(FP_SEG(p));
	puts(":"); dump(FP_OFF(p)); puts("\n");
	for(i=0;i<(n+1)/sizeof(int_16);i++) {
		dump(*p);
		puts(" ");
		p++;
	}
	puts("\n");

}

void debug_dump(int_16 seg, void *offs, int n, const char *info)
{
	void __far *p;

	puts(info); puts(": ");
	FP_SEG(p) = seg;
	FP_OFF(p) = (unsigned)offs;
	dump_mem(p,n);
}

/* �����஢��� nelem ���� ����� � ��㣮� ���� */
void __far *memcpy(void __far *result, const void __far *src, int nelem)
{
	int i;
	for(i=0;i<nelem;i++)
		((int_8 __far *)result)[i] = ((int_8 __far *)src)[i];
	return result;
}

/* ���樠����஢��� ���� ���. ᨬ����� */
void __far *memset(void __far *buffer, int character, int nelem)
{
	int i;
	for(i=0;i<nelem;i++) ((int_8 __far *)buffer)[i] = character;
	return buffer;
}

/* ���樠����஢��� ���� ���. ᫮��� */
void __far *wmemset(void __far *buffer, int character, int nelem)
{
	int i;
	for(i=0;i<nelem;i++) ((int_16 __far *)buffer)[i] = character;
	return buffer;
}

/* �ࠢ���� ��� ��ப� */
int strcmp(const char *s1, const char *s2)
{
	while(TRUE)
		if(*s1!=*s2) break;
		else if(*s1==0 && *s2==0) return TRUE;
		else { s1++; s2++; }
	return FALSE;
}

/* ��������� ��ப� �� ������� � ��㬥�� */
void parse_command(const char *strin, char *command, char *arg)
{
	int topc = 0;
	int topa = 0;
	int argf = 0;

	while(TRUE) {
		if(*strin==0) break;
		else if(*strin==' ') argf = 1;
		else {
			if(argf) { arg[topa] = *strin; topa++; }
			else { command[topc] = *strin; topc++; }
		}
		strin++;
	}
	command[topc] = 0;
	arg[topa] = 0;
}
