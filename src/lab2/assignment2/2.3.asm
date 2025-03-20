[bits 16]
xor ax, ax
; 初始化段寄存器
mov cx, 0x07c0
mov ds, cx
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00

; 获取键盘输入
mov ah, 0
int 16h

; 回显
mov bl, 07h
mov cx, 1
mov ah, 09h
int 10h

jmp $

times 510 - ($ - $$) db 0
db 0x55, 0xaa

