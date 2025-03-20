[bits 16]
[org 0x7c00]

; 清屏
mov ah, 0x06    
mov al, 0x00    
mov bh, 0x07    
mov cx, 0x0000  
mov dx, 0x184F  
int 10h

; 隐藏光标
mov ah, 0x01
mov ch, 0x20
mov cl, 0x00
int 0x10

while:
	; 移动光标
	mov dh, byte [pos_1]
	mov dl, byte [pos_1 + 1]
	mov bh, 0x00
	mov ah, 0x02
	int 10h

	; 获取当前位置的字符
	movzx si, byte [string_index_1]
	mov al, byte [string + si]
	mov bl, byte [color_1]
	mov ah, 0x09
	mov cx, 1
	int 10h

	inc bl
	mov byte [color_1], bl
	
	inc si
	mov al, byte [string + si]
	cmp al, 0
	jne reset_index_end_1
	mov si, 0
	reset_index_end_1:
	mov [string_index_1], si

	; 假设不反弹，计算新的位置
	mov ah, byte [velocity_1]
	mov al, byte [velocity_1 + 1]
	add ah, dh
	add al, dl

	; 检查垂直方向上是否要反弹
	cmp ah, 0
	jl reverse_velocity_x_1
	cmp ah, 25
	jge reverse_velocity_x_1
	jmp reverse_velocity_x_end_1

	reverse_velocity_x_1:
		neg byte [velocity_1]
		mov ah, byte [velocity_1]
		add ah, dh
	reverse_velocity_x_end_1:

	; 检查水平方向上是否要反弹
	cmp al, 0
	jl reverse_velocity_y_1
	cmp al, 80
	jge reverse_velocity_y_1
	jmp reverse_velocity_y_end_1

	reverse_velocity_y_1:
		neg byte [velocity_1 + 1]
		mov al, byte [velocity_1 + 1]
		add al, dl
	reverse_velocity_y_end_1:

	; 保存新的位置
	mov byte [pos_1], ah
	mov byte [pos_1 + 1], al

	; 移动光标
	mov dh, byte [pos_2]
	mov dl, byte [pos_2 + 1]
	mov bh, 0x00
	mov ah, 0x02
	int 10h

	; 获取当前位置的字符
	movzx si, byte [string_index_2]
	mov al, byte [string + si]
	mov bl, byte [color_2]
	mov ah, 0x09
	mov cx, 1
	int 10h

	inc bl
	mov byte [color_2], bl
	
	inc si
	mov al, byte [string + si]
	cmp al, 0
	jne reset_index_end_2
	mov si, 0
	reset_index_end_2:
	mov [string_index_2], si

	; 假设不反弹，计算新的位置
	mov ah, byte [velocity_2]
	mov al, byte [velocity_2 + 1]
	add ah, dh
	add al, dl

	; 检查垂直方向上是否要反弹
	cmp ah, 0
	jl reverse_velocity_x_2
	cmp ah, 25
	jge reverse_velocity_x_2
	jmp reverse_velocity_x_end_2

	reverse_velocity_x_2:
		neg byte [velocity_2]
		mov ah, byte [velocity_2]
		add ah, dh
	reverse_velocity_x_end_2:

	; 检查水平方向上是否要反弹
	cmp al, 0
	jl reverse_velocity_y_2
	cmp al, 80
	jge reverse_velocity_y_2
	jmp reverse_velocity_y_end_2

	reverse_velocity_y_2:
		neg byte [velocity_2 + 1]
		mov al, byte [velocity_2 + 1]
		add al, dl
	reverse_velocity_y_end_2:

	; 保存新的位置
	mov byte [pos_2], ah
	mov byte [pos_2 + 1], al

	; 移动到打印标题的开头
	mov ah, 0x02    
	mov bh, 0x00    
	mov dx, 0x0020
	int 10h

	mov si, 0
	mov bl, 0x02
	mov cx, 1
	print_title:
		mov al, [title + si]
		cmp al, 0
		je print_title_end
		mov ah, 0x09
		int 10h
		inc si
		inc dl
		mov ah, 0x02
		int 10h
		jmp print_title
	print_title_end:
	; 延时
	mov ah, 86h
	mov cx, 0x3
	mov dx, 0x0d00
	int 15h
	jmp while
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

times 510 - ($ - $$) db 0
db 0x55, 0xaa

