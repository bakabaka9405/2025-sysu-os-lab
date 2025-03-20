; If you meet compile error, try 'sudo apt install gcc-multilib g++-multilib' first

%include "head.include"
; you code here

your_if:
	cmp dword [a1], 12
	jge else_if
	mov eax, dword [if_flag]
	mov ecx, 2
	xor edx, edx
	div ecx
	inc eax
	mov [if_flag], eax
	jmp your_if_end
else_if:
	cmp dword [a1], 24
	jge else
	mov eax, 24
	sub eax, dword [a1]
	mov ecx, dword [a1]
	xor edx, edx
	mul ecx
	mov [if_flag], eax
	jmp your_if_end
else:
	mov eax, dword [a1]
	shl eax, 4
	mov [if_flag], eax
your_if_end:


your_while:
	cmp dword [a2], 12
	jl your_while_end

	call my_random
	mov ecx, dword [a2]
	sub ecx, 12
	mov ebx, dword [while_flag]
	mov [ebx+ecx], al
	dec dword [a2]
	jmp your_while
your_while_end:

%include "end.include"

your_function:
	mov eax, dword [your_string]
	xor ebx,ebx
	for:
		cmp byte [eax], 0
		je for_end
		pushad
		mov bl, byte [eax]
		push ebx
		call print_a_char
		pop ebx
		popad
		inc eax
		jmp for
	for_end:
	ret