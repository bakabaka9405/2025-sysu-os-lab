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

; 初始化 gs 到 0xb800
mov ax, 0xb800
mov gs, ax

mov ah, 0x12
mov cx, 8
mov si, 0
mov bx, 1944 ;初始偏移

print_hello_world:
    mov al, [si + string]
    mov [gs:bx], ax
    inc si
    add bx, 2
    loop print_hello_world

string db '00000000'

times 510 - ($ - $$) db 0
db 0x55, 0xaa

