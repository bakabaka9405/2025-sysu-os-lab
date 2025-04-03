[bits 32]

global asm_hello_world
global asm_lidt
global asm_unhandled_interrupt
global asm_halt

ASM_UNHANDLED_INTERRUPT_INFO db 'Unhandled interrupt happened, halt...'
                             db 0
ASM_IDTR dw 0
         dd 0

; void asm_unhandled_interrupt()
asm_unhandled_interrupt:
    cli
    mov esi, ASM_UNHANDLED_INTERRUPT_INFO
    xor ebx, ebx
    mov ah, 0x03
.output_information:
    cmp byte[esi], 0
    je .end
    mov al, byte[esi]
    mov word[gs:bx], ax
    inc esi
    add ebx, 2
    jmp .output_information
.end:
    jmp $

; void asm_lidt(uint32 start, uint16 limit)
asm_lidt:
    push ebp
    mov ebp, esp
    push eax

    mov eax, [ebp + 4 * 3]
    mov [ASM_IDTR], ax
    mov eax, [ebp + 4 * 2]
    mov [ASM_IDTR + 2], eax
    lidt [ASM_IDTR]

    pop eax
    pop ebp
    ret

asm_hello_world:
    pushad

    mov  ecx, helloworld_end - helloworld
    mov  ebx, 0
    mov  esi, helloworld
    mov  ah,  0x3
    output_helloworld:
        mov  al,           [esi]
        mov  word[gs:ebx], ax
        add  ebx,          2
        inc  esi
        loop output_helloworld
    popad
    ret

asm_halt:
    jmp $

helloworld db 'hello world'
helloworld_end: