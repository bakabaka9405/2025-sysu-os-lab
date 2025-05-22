[bits 32]

global asm_hello_world
global asm_lidt
global asm_unhandled_interrupt
global asm_halt
global asm_out_port
global asm_in_port
global asm_time_interrupt_handler
global asm_enable_interrupt
global asm_enable_interrupt
global asm_disable_interrupt
global asm_interrupt_status
global asm_switch_thread
global asm_atomic_exchange
global asm_acquire_lock
global asm_release_lock
global asm_init_page_reg

extern c_time_interrupt_handler
ASM_UNHANDLED_INTERRUPT_INFO db 'Unhandled interrupt happened, halt...'
                             db 0
ASM_IDTR dw 0
         dd 0

; void asm_switch_thread(PCB *cur, PCB *next);
asm_switch_thread:
    push ebp
    push ebx
    push edi
    push esi

    mov eax, [esp + 5 * 4] ; 取出第一个参数cur的地址到eax
    mov [eax], esp ; 保存当前栈指针到PCB中，以便日后恢复

    mov eax, [esp + 6 * 4] ; 取出第二个参数next的地址到eax
    mov esp, [eax] ; 此时栈已经从cur栈切换到next栈

    pop esi
    pop edi
    pop ebx
    pop ebp

    sti
    ret
; int asm_interrupt_status();
asm_interrupt_status:
    xor eax, eax
    pushfd
    pop eax
    and eax, 0x200
    ret

; void asm_disable_interrupt();
asm_disable_interrupt:
    cli
    ret
; void asm_init_page_reg(int *directory);

asm_enable_interrupt:
    sti
    ret
asm_time_interrupt_handler:
    pushad
    
    ; 发送EOI消息，否则下一次中断不发生
    mov al, 0x20
    out 0x20, al
    out 0xa0, al
    
    call c_time_interrupt_handler

    popad
    iret

; void asm_in_port(uint16 port, uint8 *value)
asm_in_port:
    push ebp
    mov ebp, esp

    push edx
    push eax
    push ebx

    xor eax, eax
    mov edx, [ebp + 4 * 2] ; port
    mov ebx, [ebp + 4 * 3] ; *value

    in al, dx
    mov [ebx], al

    pop ebx
    pop eax
    pop edx
    pop ebp
    ret

; void asm_out_port(uint16 port, uint8 value)
asm_out_port:
    push ebp
    mov ebp, esp

    push edx
    push eax

    mov edx, [ebp + 4 * 2] ; port
    mov eax, [ebp + 4 * 3] ; value
    out dx, al
    
    pop eax
    pop edx
    pop ebp
    ret

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

; void asm_atomic_exchange(uint32 *reg, uint32 *memory);
asm_atomic_exchange:
    push ebp
    mov ebp, esp
    pushad

    mov ebx, [ebp + 4 * 2] ; 取register地址
    mov eax, [ebx]      ; 取出register指向的变量的值
    mov ebx, [ebp + 4 * 3] ; 取memory的地址
    xchg [ebx], eax      ; 原子交换指令，交换eax中register的值与memory的值
    mov ebx, [ebp + 4 * 2] ; 取register的地址
    mov [ebx], eax      ; 将交换后存在eax中memory指向的值赋值给register指向的变量

    popad
    pop ebp
    ret

; void asm_acquire_lock(uint32 *lock);
asm_acquire_lock:
    push ebp
    mov ebp, esp
    pushad
    
    mov ebx, [ebp + 8]  ; 获取lock参数指针
.try_lock:
    lock bts dword [ebx], 0  ; 原子地测试并设置锁的第0位
    jc .try_lock        ; 如果进位标志被设置，表示锁已被占用，继续尝试
    
    popad
    pop ebp
    ret

; void asm_release_lock(uint32 *lock);
asm_release_lock:
    push ebp
    mov ebp, esp
    pushad
    
    mov ebx, [ebp + 8]  ; 获取lock参数指针
    btr dword [ebx], 0  ; 重置锁的第0位，释放锁
    
    popad
    pop ebp
    ret

asm_init_page_reg:
    push ebp
    mov ebp, esp

    push eax

    mov eax, [ebp + 4 * 2]
    mov cr3, eax ; 放入页目录表地址
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax           ; 置PG=1，开启分页机制

    pop eax
    pop ebp

    ret

asm_halt:
    jmp $