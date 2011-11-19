; MMMOC -- учебная операционная система
; (c) 2003, Kolia Morev <kolia39@mail.ru>
;
; startup.asm: Начальная загрузка ОС.

.386p

TASK_STACK_SIZE = 200h	; Размер стека задачи
NOGD = 64		; Число глобальных дескрипторов
NOINT = 256		; Число прерываний

extrn	C kernel:proc
extrn	C tasks

; Описания системных структур и их значения по умолчанию:

; Дескриптор сегмента
descr   struc
 limit   dw      0       ; Граница (биты 0..15)
 base_l  dw      0       ; База (биты 0..15)
 base_m  db      0       ; База (биты 16..23)
 attr_1  db      0       ; Байт атрибутов 1
 attr_2  db      0       ; Граница (биты 16..19) и атрибуты 2
 base_h  db      0       ; База (биты 24..31)
descr   ends

; Шлюз прерывания
intr    struc
	dw 4 dup (0)
intr    ends

; Различные макросы:

; Преобразование адреса вида "сегмент:смещение"
; в физический адрес. Результат -- в eax.
physical_addr	macro	segaddr,offsaddr
	xor	eax,eax
	mov	ax,segaddr
	shl	eax,4
	add	ax,offset offsaddr
		endm

; Установка полей base дескриптора.
; descr -- адрес дескриптора
; eax -- физический адрес
set_descr_base	macro	descr
	mov	descr.base_l,ax
	shr	eax,16
	mov	descr.base_m,al
	mov	descr.base_h,ah
		endm

; Задержка
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

; Поля данных программы
tmpdescr	descr	<>
message_exit	db	'* exit: PeaJIbHbIu Pe>|<uM.$'

; Сохраненные значения сегмента кода
; и данных из реального режима
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

; Установка поля базы дескрипторов
	physical_addr	dgroup,0
	set_descr_base	gdt_data

	physical_addr	_text,0
	set_descr_base	gdt_code

	physical_addr	dgroup,tasks	; стек задачи 0
	set_descr_base	gdt_stack

	physical_addr	dgroup,tasks	; указывает на tss_0
	set_descr_base	gdt_tss_0

; Подготовка псевдодескриптора и загрузка регистра GDTR
	physical_addr	dgroup,gdt
	set_descr_base	tmpdescr
        mov     tmpdescr.limit,gdt_size-1
        lgdt    tmpdescr

; Подготовка к переходу в защищённый режим
        cli                     ; Запрет аппаратных прерываний
        mov     al,8fh          ; Запрет NMI
        out     70h,al          ; Порт CMOS
	delay
        mov     al,05h          ; Байт состояния отключения 05:
        out     71h,al          ; возврат в программу и
                                ; перепрограммир. контроллеров
        mov     ax,40h          ; Загрузка в область
        mov     es,ax           ; данных BIOS адреса возврата
				; в программу
        mov     word ptr es:[67h],offset return
        mov     word ptr es:[69h],cs

; Перепрограммирование ведущего контроллера прерываний
; Базовый вектор теперь 20h
        mov     dx,20h      	; Порт контроллера
        mov     al,11h      	; СКИ1: будет СКИ3
        out     dx,al
	delay
        inc     dx          	; Второй порт контроллера
        mov     al,20h      	; СКИ2: базовый вектор
        out     dx,al
	delay
        mov     al,4        	; СКИ3: ведомый подключен к уровню 2
        out     dx,al
	delay
        mov     al,1        	; СКИ4: 80x86, требуется EOI
        out     dx,al
        mov     al,0fch     	; Маска прерываний
        out     dx,al

; Инициализация ведомого контроллера
; Базовый вектор теперь 28h
        mov     dx,0a0h     	; Порт контроллера
        mov     al,11h      	; СКИ1: будет СКИ3
        out     dx,al
	delay
        inc     dx          	; Второй порт контроллера
        mov     al,28h      	; СКИ2: базовый вектор
        out     dx,al
	delay
        mov     al,2        	; СКИ3: ведомый подключен к уровню 2
        out     dx,al
	delay
        mov     al,1        	; СКИ4: 80x86, требуется EOI
        out     dx,al
	delay

; Запретить все прерывания в ведомом контроллере
        mov     dx,0a1h         ; Порт ведомого контроллера
        mov     al,0ffh         ; Маска прерываний -
        out     dx,al           ; замаскируем все прерывания

; Загрузка IDTR
	physical_addr	dgroup,idt
	set_descr_base	tmpdescr
        mov     tmpdescr.limit,idt_size-1
        lidt    tmpdescr

; Откроем линию A20
        mov     al,0d1h     	; Команда управления
        out     64h,al      	;  линией A20
        mov     al,0dfh     	; Код открытия
        out     60h,al      	;  линии A20

; Переход в защищённый режим
        mov     eax,cr0
        or      eax,1b
        mov     cr0,eax

; Теперь процессор работает в защищённом режиме
; Загрузка в CS селектора сегмента команд, перезагрузка буфера команд
        db      0eah        	; far jmp
        dw      offset continue
        dw      sel_code
continue:

; Инициализируем сегментные регистры
        mov     ax,sel_data
        mov     ds,ax
        mov     ax,sel_stack
        mov     ss,ax
        mov     ax,0
	mov	es,ax
        mov     gs,ax
        mov     fs,ax

        mov     ax,sel_tss_0    ; Загрузка TR селектором TSS_0
        ltr     ax

; Запускаем ядро ОС
	call	kernel

; Организация перехода в реальный режим
; Закроем линию A20
        mov     al,0d1h
        out     64h,al
        mov     al,0ddh
        out     60h,al

; Переключим режим процессора
        mov     al,0feh         ; Переходим в реальный режим
        out     64h,al          ; засылкой кода feh в порт 64h
        hlt                     ; Останов в ожидании сброса

; Теперь процессор снова работает в реальном режиме
; Восстановим операционную среду реального режима
return: mov     ax,dgroup
        mov     ds,ax
        mov     ax,_stack
        mov     ss,ax
        mov     sp,size _stack

; Размаскируем прерывания в контроллере
        mov     al,0b8h
        out     21h,al
        mov     al,9dh
        out     0a1h,al

; Разрешим аппаратные и немаскируемые прерывания
        sti
        mov     al,0            ; Разрешим NMI
        out     70h,al

; Проверим выполнение функций DOS и завершим программу
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
