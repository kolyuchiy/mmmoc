; MMMOC -- �祡��� ����樮���� ��⥬�
; (c) 2003, Kolia Morev <kolia39@mail.ru>
;
; ints.asm: ��ࠡ��稪� �᪫�祭��, ���뢠��� � �ࢨ��.

.386p

TASK_STACK_SIZE = 200h

extrn C do_timer:proc
extrn C store_key:proc
extrn C puts:proc
extrn C exit:proc
extrn C dump:proc
extrn C dump_stack:proc
extrn C halt_cpu:proc

; ������ "����� ���뢠���" (End Of Interrupt)
; � ����騩 ����஫��� ���뢠���.
EOI_to_master macro
	push	ax
	mov	al,20h
	out	20h,al
	pop	ax
              endm

; ������ "����� ���뢠���" (End Of Interrupt)
; � ������ ����஫��� ���뢠���.
EOI_to_slave macro
	push	ax
	mov	al,20h
	out	0a0h,al
	pop	ax
             endm

DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public use16 'DATA'

nl = 10
str_exc_0	db '* 0x00 #DE OTKA3: Divide Error',nl,0
str_exc_0a	db '* 0x0a #TS OTKA3: Invalid TSS',nl,0
str_exc_0b	db '* 0x0b #NP OTKA3: Segment Not Present',nl,0
str_exc_0c	db '* 0x0c #SS OTKA3: Stack-Segment Fault',nl,0
str_exc_0d	db '* 0x0d #GP OTKA3: General Protection',nl,0
str_exc_0e	db '* 0x0e #PF OTKA3: Page Fault',nl,0
str_exc_dummy	db '* 0x?? #?? _____: Heu3BecTHoe ucKJI|-O4eHue',nl,0

_DATA	ends

_BSS	segment word public use16 'BSS'
_BSS	ends

_TEXT	segment byte public use16 'CODE'
	assume	cs:_TEXT,ds:DGROUP

extrn C getsc:proc

exception_common macro str_error
	local	dmpstk
	pushad
;	cli
	call	puts C,offset str_error
	call	dump_stack C
;	sti
	popad
                 endm

; ��ࠡ��稪 �᪫�祭�� �� ������� �� ���
public C exc_0
exc_0   proc
        mov     bx,1			; ���४�� ����⥫�
	exception_common str_exc_0
        iretd
exc_0   endp

; ��ࠡ��稪 �᪫�祭�� �������⨬��� TSS
public C exc_0ah
exc_0ah proc
	exception_common str_exc_0a
	iretd
exc_0ah endp

; ��ࠡ��稪 �᪫�祭�� ������⢨� ᥣ����
public C exc_0bh
exc_0bh proc
	exception_common str_exc_0b
	iretd
exc_0bh endp

; ��ࠡ��稪 �᪫�祭�� �訡�� ���饭�� � �⥪�
public C exc_0ch
exc_0ch proc
	exception_common str_exc_0c
	iretd
exc_0ch endp

; ��ࠡ��稪 �᪫�祭�� ��饩 �����
; ��蠥� ��������
public C exc_0dh
exc_0dh proc
	exception_common str_exc_0d
	call	halt_cpu
	iretd
exc_0dh endp

; ��ࠡ��稪 �᪫�祭�� ��࠭�筮�� ����襭��
public C exc_0eh
exc_0eh proc
	exception_common str_exc_0e
	iretd
exc_0eh endp

; ��ࠡ��稪 ��⠫��� �᪫�祭��
public C dummy_exc
dummy_exc proc
	exception_common str_exc_dummy
	call	halt_cpu
	iretd
dummy_exc endp

; ��ࠡ��稪 ���뢠��� �� ⠩��� (1/18.2 sec)
public C new_08h
new_08h proc
	pushad
	EOI_to_master
	call	do_timer C
	popad
        iretd
new_08h endp

; ��ࠡ��稪 ���뢠��� �� ����������
public C new_09h
new_09h proc
	push	ax
	xor	ax,ax
	in	al,60h          ; ������ ᪠�-��� �� ���� 60h
	cmp	al,80h
	jae	@too_big
	call	store_key C,ax
@too_big:
	in	al,61h          ; ����稬 ᮤ�ন��� ����
	or	al,10000000b    ; ��⠭����� ���襣� ���
	out	61h,al          ; � ��᫥���騬 ��ᮬ ���
	and	al,01111111b    ; ᮮ�騬 ���஫���� ����������
	out	61h,al          ; � ��� ᪠�-���� ᨬ����
	pop	ax
	EOI_to_master
	iretd
new_09h endp

; ��楤�� ��� ����饣� ����஫���
public C master
master  proc
	EOI_to_master
        iretd
master  endp

; ��楤�� ��� �������� ����஫���
public C slave
slave   proc
	EOI_to_slave
	EOI_to_master
        iretd
slave   endp

public C puts_serv
puts_serv proc	far
	push	bp
	mov	bp,sp
	push	word ptr [bp+10]
	call	near ptr puts
	add	sp,2
	pop	bp
	db	66h
	ret
puts_serv endp

public C exit_serv
exit_serv proc	far
	push	bp
	mov	bp,sp
	push	word ptr [bp+10]
	call	near ptr exit
	add	sp,2
	pop	bp
	db	66h
	ret
exit_serv endp

_TEXT   ends

        end
