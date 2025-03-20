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

; 设置光标位置为(12,12)
mov dh, 12
mov dl, 12
mov ah, 02h
int 10h

mov bl, 0x12; 颜色
mov si, 0
mov cx, 1

print_hello_world:
	; 写字符
    mov al, [si + string]
	mov ah, 09h
    int 10h

	; 移动光标
    inc dl
    mov ah, 02h
	int 10h

	; 当 si<8 时继续循环
	inc si
	cmp si, 8
	jb print_hello_world

string db '00000000'

times 510 - ($ - $$) db 0
db 0x55, 0xaa

