
;; ISR C routine ;;
extern ide_primary_irq
extern ide_secondary_irq
extern __isr_0
extern __isr_1
extern __isr_2
extern __isr_3
extern __isr_4
extern __isr_5
extern __isr_6
extern __isr_7
extern __isr_syscall
extern __isr_default

;; ISR wrap ;;
global isr_0
global isr_1
global isr_2
global isr_3
global isr_4
global isr_5
global isr_6
global isr_7
global isr_syscall
global isr_default
global isr_14
global isr_15

%macro EOI_MASTER 0
mov al,0x20
out 0x20,al
%endmacro

%macro EOI_SLAVE 0
mov al,0x20
out 0xa0,al
%endmacro

%macro SAVE_REGS 0
pushad
push ds
push es
push fs
push gs
push ebx
mov bx,0x10
mov ds,bx
pop ebx
%endmacro

%macro RESTORE_REGS 0
pop gs
pop fs
pop es
pop ds
popad
%endmacro

isr_0:
SAVE_REGS
call __isr_0
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_1:
SAVE_REGS
call __isr_1
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_2:
SAVE_REGS
call __isr_2
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_3:
SAVE_REGS
call __isr_3
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_4:
SAVE_REGS
call __isr_4
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_5:
SAVE_REGS
call __isr_5
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_6:
SAVE_REGS
call __isr_6
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_7:
SAVE_REGS
call __isr_7
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_14:
SAVE_REGS
call ide_primary_irq
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_15:
SAVE_REGS
call ide_secondary_irq
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret

isr_syscall:
SAVE_REGS
call __isr_syscall
EOI_MASTER
EOI_SLAVE
RESTORE_REGS
iret
