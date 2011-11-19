; MMMOC -- учебная операционная система
; (c) 2003, Kolia Morev <kolia39@mail.ru>
;
; ints.asm: Обработчики исключений, прерываний и сервисы.

.386p

TASK_STACK_SIZE = 200h

extrn C do_timer:proc
extrn C store_key:proc
extrn C puts:proc
extrn C exit:proc
extrn C dump:proc
extrn C dump_stack:proc
extrn C halt_cpu:proc

; Сигнал "конец прерывания" (End Of Interrupt)
; в ведущий контроллер прерываний.
EOI_to_master macro
	push	ax
	mov	al,20h
	out	20h,al
	pop	ax
              endm

; Сигнал "конец прерывания" (End Of Interrupt)
; в ведомый контроллер прерываний.
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

; Обработчик исключения при делении на нуль
public C exc_0
exc_0   proc
        mov     bx,1			; Коррекция делителя
	exception_common str_exc_0
        iretd
exc_0   endp

; Обработчик исключения недопустимого TSS
public C exc_0ah
exc_0ah proc
	exception_common str_exc_0a
	iretd
exc_0ah endp

; Обработчик исключения отсутствия сегмента
public C exc_0bh
exc_0bh proc
	exception_common str_exc_0b
	iretd
exc_0bh endp

; Обработчик исключения ошибки обращения к стеку
public C exc_0ch
exc_0ch proc
	exception_common str_exc_0c
	iretd
exc_0ch endp

; Обработчик исключения общей защиты
; Вешает компьютер
public C exc_0dh
exc_0dh proc
	exception_common str_exc_0d
	call	halt_cpu
	iretd
exc_0dh endp

; Обработчик исключения страничного нарушения
public C exc_0eh
exc_0eh proc
	exception_common str_exc_0e
	iretd
exc_0eh endp

; Обработчик остальных исключений
public C dummy_exc
dummy_exc proc
	exception_common str_exc_dummy
	call	halt_cpu
	iretd
dummy_exc endp

; Обработчик прерываний от таймера (1/18.2 sec)
public C new_08h
new_08h proc
	pushad
	EOI_to_master
	call	do_timer C
	popad
        iretd
new_08h endp

; Обработчик прерываний от клавиатуры
public C new_09h
new_09h proc
	push	ax
	xor	ax,ax
	in	al,60h          ; Вводим скан-код из порта 60h
	cmp	al,80h
	jae	@too_big
	call	store_key C,ax
@too_big:
	in	al,61h          ; Получим содержимое порта
	or	al,10000000b    ; установкой старшего бита
	out	61h,al          ; и последующим сбросом его
	and	al,01111111b    ; сообщим конроллеру клавиатуры
	out	61h,al          ; о приёме скан-кода символа
	pop	ax
	EOI_to_master
	iretd
new_09h endp

; Процедура для ведущего контроллера
public C master
master  proc
	EOI_to_master
        iretd
master  endp

; Процедура для ведомого контроллера
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
