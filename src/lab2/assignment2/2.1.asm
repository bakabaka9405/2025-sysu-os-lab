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

; 获取光标位置
mov ah, 03h 
int 10h

; 设置光标位置
add dh, 1
add dl, 2
mov ah, 02h 
int 10h

jmp $
