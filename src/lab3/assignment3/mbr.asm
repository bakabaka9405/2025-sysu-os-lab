%include"boot.inc"
[bits 16]
xor   ax, ax                      ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov   ds, ax
mov   ss, ax
mov   es, ax
mov   fs, ax
mov   gs, ax

; 初始化栈指针
mov   sp, 0x7c00

; 隐藏光标
mov ah, 0x01
mov ch, 0x20
mov cl, 0x00
int 0x10

; 读取磁盘
mov   ah, 0x02                    ; 功能号
mov   al, LOADER_SECTOR_COUNT     ; 扇区数
mov   ch, 0                       ; 柱面
mov   cl, LOADER_START_SECTOR+1   ; 扇区
mov   dh, 0                       ; 磁头
mov   dl, 0x80                    ; 驱动器
mov   bx, LOADER_START_ADDRESS    ; 缓冲区地址
int   0x13

jc    load_error                  ; 检测 CF 是否不为 0
jmp   0x0000:LOADER_START_ADDRESS ; 跳转到bootloader

load_error:
jmp   $                           ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa
