[bits 32]

global asm_hello_world

asm_hello_world:
    pushad

    mov  ecx, student_id_end - student_id
    mov  ebx, 0
    mov  esi, student_id
    mov  ah,  0x3
    output_student_id:
        mov  al,           [esi]
        mov  word[gs:ebx], ax
        add  ebx,          2
        inc  esi
        loop output_student_id
    popad
    ret

student_id db '00000000'
student_id_end: