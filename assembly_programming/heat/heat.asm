    TRUE            equ  1
section .bss
    matrix          resq 1
    changes         resq 1
    heaters         resq 1
    width           resd 1
    height          resd 1
    freezer_temp    resd 1
    conduction      resd 1

section .text
    global start
    global place
    global step

; A macro to get the index in 1D array from 2D coordinates.
; The result is located in rax.
; Because of mul, it modifies rdx.
%macro get_index 2
    xor rax, rax
    mov eax, %1
    mul dword [rel width]
    add eax, %2
%endmacro

; Initialize data structures for the program.
; It assumes that memory contains (width * height * (4 + 4 + 1) bytes.)
; First (w * h) bytes contain initial board state.
; The rest of the memory contains zeroes.
; rdi  - width
; rsi  - height
; rdx  - pointer to memory given by caller
; xmm0 - temperature of freezers
; xmm1 - heat transfer coefficient
start:
    mov dword [rel width], edi
    mov dword [rel height], esi
    movss dword [rel freezer_temp], xmm0
    movss dword [rel conduction], xmm1
    mov qword [rel matrix], rdx

    mov r9, rdx

    ; Calculate offset for changes and heaters.
    mov rax, rdi
    mul esi
    ; Set changes as matrix + 4 * width + length
    lea r10, [r9 + 4 * rax]
    mov qword [rel changes], r10
    ; Set heaters as changes + 4 * width * length
    lea r11, [r10 + 4 * rax]
    mov qword[rel heaters], r11

    ret

; Place heaters.
; rdi - numbers of heaters
; rsi - x coordinates
; rdx - y coordinates
; rcx - heaters' temperatures
place:
    cmp rdi, 0
    je .skip
    mov r8, rdx
.set_heater:
    dec rdi
    get_index dword [rsi + 4 * rdi], dword [r8 + 4 * rdi]

    mov r11, qword [rel heaters]
    mov byte [r11 + rax], TRUE

    mov r11, qword [rel matrix]
    mov r10d, dword [rcx + 4 * rdi]
    mov dword [r11 + 4 * rax], r10d

    cmp rdi, 0
    jnz .set_heater
.skip:
    ret

; Make one step of the simulation.
; Takes no arguments.
step:
    push r12
    push r13
    push r14
    ; Move width and height to registers, they will be used often.
    mov r8d, dword [rel width]
    mov r9d, dword [rel height]

    ; Move pointers to registers
    mov r11, qword [rel matrix]
    mov r12, qword [rel changes]
    mov r13, qword [rel heaters]

    movss xmm2, dword [rel freezer_temp]
    movss xmm3, dword [rel conduction]

    xor rcx, rcx
    mov ecx, r8d
.calc_y_loop:
    dec ecx
    xor r14, r14
    mov r14d, r9d
.calc_x_loop:
    dec r14d
    ; Calculate the change of temperature for every cell.
    get_index r14d, ecx
    ; If it's a heater, don't bother with calculating.
    cmp byte[r13 + rax], TRUE
    je .update_calc_x_loop

    ; If not a heater, calculate the change and save it in the memory.
    xorps xmm0, xmm0
    movss xmm1, dword [r11 + 4 * rax]
    mov r10, rax
.up:
    ; If top row, a freezer is above this field.
    cmp r14d, 0
    je .up_wrong
    ; Get index of field (x - 1, y)
    mov esi, r14d
    dec esi
    get_index esi, ecx

    addss xmm0, dword [r11 + 4 * rax]
    subss xmm0, xmm1
    jmp .down
.up_wrong:
    addss xmm0, xmm2
    subss xmm0, xmm1
.down:
    ; If bottom row, a freezer is below this field.
    mov esi, r14d
    inc esi
    cmp esi, r9d
    je .down_wrong
    ; Get index of field (x + 1, y)
    get_index esi, ecx

    addss xmm0, dword [r11 + 4 * rax]
    subss xmm0, xmm1
    jmp .left
.down_wrong:
    addss xmm0, xmm2
    subss xmm0, xmm1
.left:
    ; If 0th column, a freezer is on the left.
    cmp ecx, 0
    je .left_wrong
    ; Get index of field (x, y - 1)
    mov esi, r14d
    mov edi, ecx
    dec edi
    get_index esi, edi

    addss xmm0, dword [r11 + 4 * rax]
    subss xmm0, xmm1
    jmp .right
.left_wrong:
    addss xmm0, xmm2
    subss xmm0, xmm1
.right:
    ; If the last column, a freezer is on the right.
    mov edi, ecx
    inc edi
    cmp edi, r8d
    je .right_wrong
    ; Get index of field (x, y + 1)
    mov esi, r14d
    get_index esi, edi

    addss xmm0, dword [r11 + 4 * rax]
    subss xmm0, xmm1
    jmp .update_change
.right_wrong:
    addss xmm0, xmm2
    subss xmm0, xmm1

.update_change:
    mulss xmm0, xmm3
    movss dword [r12 + 4 * r10], xmm0

.update_calc_x_loop:
    cmp r14d, 0
    jne .calc_x_loop

    cmp ecx,  0
    jne .calc_y_loop

    mov ecx, r8d
.set_y_loop:
    dec ecx
    mov r14d, r9d
.set_x_loop:
    dec r14d
    ; Set the new temperatures.
    get_index r14d, ecx
    ; Don't bother with heaters.
    cmp byte [r13 + rax], TRUE
    je .update_set_x_loop

    movss xmm0, dword [r11 + 4 * rax]
    addss xmm0, dword [r12 + 4 * rax]
    movss dword [r11 + 4 * rax], xmm0

.update_set_x_loop:
    cmp r14d, 0
    jne .set_x_loop

    cmp ecx, 0
    jne .set_y_loop

    pop r14
    pop r13
    pop r12

    ret