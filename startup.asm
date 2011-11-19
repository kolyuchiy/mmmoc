; MMMOC -- �祡��� ����樮���� ��⥬�
; (c) 2003, Kolia Morev <kolia39@mail.ru>
;
; startup.asm: ��砫쭠� ����㧪� ��.

.386p

TASK_STACK_SIZE = 200h	; ������ �⥪� �����
NOGD = 64		; ��᫮ ��������� ���ਯ�஢
NOINT = 256		; ��᫮ ���뢠���

extrn	C kernel:proc
extrn	C tasks

; ���ᠭ�� ��⥬��� ������� � �� ���祭�� �� 㬮�砭��:

; ���ਯ�� ᥣ����
descr   struc
 limit   dw      0       ; �࠭�� (���� 0..15)
 base_l  dw      0       ; ���� (���� 0..15)
 base_m  db      0       ; ���� (���� 16..23)
 attr_1  db      0       ; ���� ��ਡ�⮢ 1
 attr_2  db      0       ; �࠭�� (���� 16..19) � ��ਡ��� 2
 base_h  db      0       ; ���� (���� 24..31)
descr   ends

; ��� ���뢠���
intr    struc
	dw 4 dup (0)
intr    ends

; ������� ������:

; �८�ࠧ������ ���� ���� "ᥣ����:ᬥ饭��"
; � 䨧��᪨� ����. ������� -- � eax.
physical_addr	macro	segaddr,offsaddr
	xor	eax,eax
	mov	ax,segaddr
	shl	eax,4
	add	ax,offset offsaddr
		endm

; ��⠭���� ����� base ���ਯ��.
; descr -- ���� ���ਯ��
; eax -- 䨧��᪨� ����
set_descr_base	macro	descr
	mov	descr.base_l,ax
	shr	eax,16
	mov	descr.base_m,al
	mov	descr.base_h,ah
		endm

; ����প�
delay		macro
	jmp	$+2
		endm

DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public use16 'DATA'

public	C gdt
gdt		label	word
gdt_null	descr	<0,0,0,0,0,0>
sel_code	= 10q
gdt_code	descr	<0FFFFh,,,10011010b>
sel_data	= 20q
gdt_data	descr	<0FFFFh,,,10010010b>
sel_stack	= 30q
gdt_stack	descr	<TASK_STACK_SIZE,,,10010010b>
sel_tss_0	= 40q
gdt_tss_0	descr	<103,,,89h>
		descr	59 dup (<>)
gdt_size = NOGD * size descr

public	C idt
idt	label	word
	intr	NOINT dup (<>)
idt_size = NOINT * size intr

; ���� ������ �ணࠬ��
tmpdescr	descr	<>
message_exit	db	'* exit: PeaJIbHbIu Pe>|<uM.$'

; ���࠭���� ���祭�� ᥣ���� ����
; � ������ �� ॠ�쭮�� ०���
public	C real_data
real_data	dw	dgroup
public	C real_text
real_text	dw	_text

_DATA	ends

_BSS	segment word public use16 'BSS'
_BSS	ends

_TEXT	segment byte public use16 'CODE'
	assume	cs:_TEXT,ds:DGROUP

startup:
        mov     ax,dgroup
        mov     ds,ax

; ��⠭���� ���� ���� ���ਯ�஢
	physical_addr	dgroup,0
	set_descr_base	gdt_data

	physical_addr	_text,0
	set_descr_base	gdt_code

	physical_addr	dgroup,tasks	; �⥪ ����� 0
	set_descr_base	gdt_stack

	physical_addr	dgroup,tasks	; 㪠�뢠�� �� tss_0
	set_descr_base	gdt_tss_0

; �����⮢�� �ᥢ�����ਯ�� � ����㧪� ॣ���� GDTR
	physical_addr	dgroup,gdt
	set_descr_base	tmpdescr
        mov     tmpdescr.limit,gdt_size-1
        lgdt    tmpdescr

; �����⮢�� � ���室� � ������ ०��
        cli                     ; ����� �������� ���뢠���
        mov     al,8fh          ; ����� NMI
        out     70h,al          ; ���� CMOS
	delay
        mov     al,05h          ; ���� ���ﭨ� �⪫�祭�� 05:
        out     71h,al          ; ������ � �ணࠬ�� �
                                ; ��९ணࠬ���. ����஫��஢
        mov     ax,40h          ; ����㧪� � �������
        mov     es,ax           ; ������ BIOS ���� ������
				; � �ணࠬ��
        mov     word ptr es:[67h],offset return
        mov     word ptr es:[69h],cs

; ��९ணࠬ��஢���� ����饣� ����஫��� ���뢠���
; ������ ����� ⥯��� 20h
        mov     dx,20h      	; ���� ����஫���
        mov     al,11h      	; ���1: �㤥� ���3
        out     dx,al
	delay
        inc     dx          	; ��ன ���� ����஫���
        mov     al,20h      	; ���2: ������ �����
        out     dx,al
	delay
        mov     al,4        	; ���3: ������ ������祭 � �஢�� 2
        out     dx,al
	delay
        mov     al,1        	; ���4: 80x86, �ॡ���� EOI
        out     dx,al
        mov     al,0fch     	; ��᪠ ���뢠���
        out     dx,al

; ���樠������ �������� ����஫���
; ������ ����� ⥯��� 28h
        mov     dx,0a0h     	; ���� ����஫���
        mov     al,11h      	; ���1: �㤥� ���3
        out     dx,al
	delay
        inc     dx          	; ��ன ���� ����஫���
        mov     al,28h      	; ���2: ������ �����
        out     dx,al
	delay
        mov     al,2        	; ���3: ������ ������祭 � �஢�� 2
        out     dx,al
	delay
        mov     al,1        	; ���4: 80x86, �ॡ���� EOI
        out     dx,al
	delay

; ������� �� ���뢠��� � ������� ����஫���
        mov     dx,0a1h         ; ���� �������� ����஫���
        mov     al,0ffh         ; ��᪠ ���뢠��� -
        out     dx,al           ; ����᪨�㥬 �� ���뢠���

; ����㧪� IDTR
	physical_addr	dgroup,idt
	set_descr_base	tmpdescr
        mov     tmpdescr.limit,idt_size-1
        lidt    tmpdescr

; ��஥� ����� A20
        mov     al,0d1h     	; ������� �ࠢ�����
        out     64h,al      	;  ������ A20
        mov     al,0dfh     	; ��� ������
        out     60h,al      	;  ����� A20

; ���室 � ������ ०��
        mov     eax,cr0
        or      eax,1b
        mov     cr0,eax

; ������ ������ ࠡ�⠥� � ����񭭮� ०���
; ����㧪� � CS ᥫ���� ᥣ���� ������, ��१���㧪� ���� ������
        db      0eah        	; far jmp
        dw      offset continue
        dw      sel_code
continue:

; ���樠�����㥬 ᥣ����� ॣ�����
        mov     ax,sel_data
        mov     ds,ax
        mov     ax,sel_stack
        mov     ss,ax
        mov     ax,0
	mov	es,ax
        mov     gs,ax
        mov     fs,ax

        mov     ax,sel_tss_0    ; ����㧪� TR ᥫ���஬ TSS_0
        ltr     ax

; ����᪠�� �� ��
	call	kernel

; �࣠������ ���室� � ॠ��� ०��
; ���஥� ����� A20
        mov     al,0d1h
        out     64h,al
        mov     al,0ddh
        out     60h,al

; ��४��稬 ०�� ������
        mov     al,0feh         ; ���室�� � ॠ��� ०��
        out     64h,al          ; ���뫪�� ���� feh � ���� 64h
        hlt                     ; ��⠭�� � �������� ���

; ������ ������ ᭮�� ࠡ�⠥� � ॠ�쭮� ०���
; ����⠭���� ����樮���� �।� ॠ�쭮�� ०���
return: mov     ax,dgroup
        mov     ds,ax
        mov     ax,_stack
        mov     ss,ax
        mov     sp,size _stack

; �����᪨�㥬 ���뢠��� � ����஫���
        mov     al,0b8h
        out     21h,al
        mov     al,9dh
        out     0a1h,al

; ����訬 ������� � ����᪨�㥬� ���뢠���
        sti
        mov     al,0            ; ����訬 NMI
        out     70h,al

; �஢�ਬ �믮������ �㭪権 DOS � �����訬 �ணࠬ��
        mov     ah,09h
        mov     dx,offset message_exit
        int     21h
        mov     ax,4c00h
        int     21h

_TEXT	ends

_STACK	SEGMENT STACK 'STACK'
	org	200h
_STACK	ends

        end     startup
