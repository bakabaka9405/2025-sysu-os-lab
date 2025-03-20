%include"boot.inc"
[bits 16]

mov  dword [GDT_START_ADDRESS+0x00], 0x00
mov  dword [GDT_START_ADDRESS+0x04], 0x00

; 创建描述符，这是一个数据段，对应0~4GB的线性地址空间
mov  dword [GDT_START_ADDRESS+0x08], 0x0000ffff   ; 基地址为0，段界限为0xFFFFF
mov  dword [GDT_START_ADDRESS+0x0c], 0x00cf9200   ; 粒度为4KB，存储器段描述符


                                                  ; 建立保护模式下的堆栈段描述符
mov  dword [GDT_START_ADDRESS+0x10], 0x00000000   ; 基地址为0x00000000，界限0x0
mov  dword [GDT_START_ADDRESS+0x14], 0x00409600   ; 粒度为1个字节

                                                  ; 建立保护模式下的显存描述符
mov  dword [GDT_START_ADDRESS+0x18], 0x80007fff   ; 基地址为0x000B8000，界限0x07FFF
mov  dword [GDT_START_ADDRESS+0x1c], 0x0040920b   ; 粒度为字节

                                                  ; 创建保护模式下平坦模式代码段描述符
mov  dword [GDT_START_ADDRESS+0x20], 0x0000ffff   ; 基地址为0，段界限为0xFFFFF
mov  dword [GDT_START_ADDRESS+0x24], 0x00cf9800   ; 粒度为4kb，代码段描述符



mov  word [pgdt], 39                              ; 描述符表的界限
lgdt [pgdt]

in   al,        0x92                              ; 南桥芯片内的端口
or   al,        0000_0010B
out  0x92,      al                                ; 打开A20

cli                                               ; 中断机制尚未工作
mov  eax,       cr0
or   eax,       1
mov  cr0,       eax                               ; 设置PE位

jmp  dword CODE_SELECTOR:protect_mode_begin

; 16位的描述符选择子：32位偏移
; 清流水线并串行化处理器
[bits 32]
protect_mode_begin:

mov  eax,       DATA_SELECTOR                     ; 加载数据段(0..4GB)选择子
mov  ds,        eax
mov  es,        eax
mov  eax,       STACK_SELECTOR
mov  ss,        eax
mov  esp,       0x7c00                            ; 设置栈指针
mov  eax,       VIDEO_SELECTOR
mov  gs,        eax

call clear_screen                                 ; 调用清屏函数

while:
    ; 输出第一个字符
    movzx eax, byte [pos_1]
    movzx ebx, byte [pos_1 + 1]
    movzx ecx, byte [string_index_1]
    movzx edx, byte [color_1]
    call display_char

    ; 更新颜色
    inc byte [color_1]

    ; 更新字符索引
    inc byte [string_index_1]
    movzx eax, byte [string_index_1]
    mov bl, byte [string + eax]
    cmp bl, 0
    jne reset_index_end_1
    mov byte [string_index_1], 0
    reset_index_end_1:

    ; 更新第一个字符的位置
    call update_position_1

    ; 输出第二个字符
    movzx eax, byte [pos_2]
    movzx ebx, byte [pos_2 + 1]
    movzx ecx, byte [string_index_2]
    movzx edx, byte [color_2]
    call display_char

    ; 更新颜色
    inc byte [color_2]

    ; 更新字符索引
    inc byte [string_index_2]
    movzx eax, byte [string_index_2]
    mov bl, byte [string + eax]
    cmp bl, 0
    jne reset_index_end_2
    mov byte [string_index_2], 0
    reset_index_end_2:

    ; 更新第二个字符的位置
    call update_position_2

    ; 显示标题
    call display_title

    ; 延时
    mov ecx, 0x500000
    delay_loop:
        loop delay_loop

    jmp while

clear_screen:
    push eax
    push ecx
    push edi

    mov eax, 0x0720
    mov ecx, 80*25
    mov edi, 0

    clear_loop:
        mov word [gs:edi], ax
        add edi, 2
        loop clear_loop

    pop edi
    pop ecx
    pop eax
    ret


display_char:
    ; eax: 行号
    ; ebx: 列号
    ; ecx: 字符索引
    ; edx: 颜色
    push eax
    push ebx
    push ecx
    push edx
    push edi

    ; 计算显存偏移
    imul eax, 80
    add eax, ebx
    shl eax, 1
    mov edi, eax

    ; 获取字符
    mov al, byte [string + ecx]
    mov ah, dl

    ; 写入显存
    mov word [gs:edi], ax

    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret


update_position_1:
    push eax
    push ebx

    ; 计算新位置
    mov al, byte [pos_1]
    mov bl, byte [pos_1 + 1]
    add al, byte [velocity_1]
    add bl, byte [velocity_1 + 1]

    ; 检查行边界
    cmp al, 0
    jl reverse_velocity_x_1
    cmp al, 24
    jg reverse_velocity_x_1
    jmp reverse_velocity_x_end_1

    reverse_velocity_x_1:
        neg byte [velocity_1]
        mov al, byte [pos_1]
        add al, byte [velocity_1]
    reverse_velocity_x_end_1:

    ; 检查列边界
    cmp bl, 0
    jl reverse_velocity_y_1
    cmp bl, 79
    jg reverse_velocity_y_1
    jmp reverse_velocity_y_end_1

    reverse_velocity_y_1:
        neg byte [velocity_1 + 1]
        mov bl, byte [pos_1 + 1]
        add bl, byte [velocity_1 + 1]
    reverse_velocity_y_end_1:

    ; 保存新位置
    mov byte [pos_1], al
    mov byte [pos_1 + 1], bl

    pop ebx
    pop eax
    ret


update_position_2:
    push eax
    push ebx

    ; 计算新位置
    mov al, byte [pos_2]
    mov bl, byte [pos_2 + 1]
    add al, byte [velocity_2]
    add bl, byte [velocity_2 + 1]

    ; 检查行边界
    cmp al, 0
    jl reverse_velocity_x_2
    cmp al, 24
    jg reverse_velocity_x_2
    jmp reverse_velocity_x_end_2

    reverse_velocity_x_2:
        neg byte [velocity_2]
        mov al, byte [pos_2]
        add al, byte [velocity_2]
    reverse_velocity_x_end_2:

    ; 检查列边界
    cmp bl, 0
    jl reverse_velocity_y_2
    cmp bl, 79
    jg reverse_velocity_y_2
    jmp reverse_velocity_y_end_2

    reverse_velocity_y_2:
        neg byte [velocity_2 + 1]
        mov bl, byte [pos_2 + 1]
        add bl, byte [velocity_2 + 1]
    reverse_velocity_y_end_2:

    ; 保存新位置
    mov byte [pos_2], al
    mov byte [pos_2 + 1], bl

    pop ebx
    pop eax
    ret

display_title:
    push eax
    push esi
    push edi

    mov eax, 0
    mov esi, 0
    mov edi, 64

    display_title_loop:
        mov cl, byte [title + esi]
        cmp cl, 0
        je display_title_end

        ; 写入字符到显存
        mov al, cl
        mov ah, 2
        mov word [gs:edi], ax

        inc esi
        add edi, 2
        jmp display_title_loop

    display_title_end:

    pop edi
    pop esi
    pop eax
    ret

jmp $

title db 'test 00000000',0
string db '1357924680',0

string_index_1 dw 0
color_1 db 0
pos_1 db 2,0
velocity_1 db 1,1

string_index_2 dw 0
color_2 db 0x21
pos_2 db 3,79
velocity_2 db -1,-1


pgdt dw 0
    dd GDT_START_ADDRESS