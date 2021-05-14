        INPUT_MODE_ON               equ 1
        DEFAULT_EXCHANGE_PARTNER    equ 0
        ; Characters encountered in calc string.
        EXIT_INPUT_MODE_CHAR        equ '='
        ADDITION_CHAR               equ '+'
        MULTIPLICATION_CHAR         equ '*'
        ARITHM_NEGATION_CHAR        equ '-'
        AND_CHAR                    equ '&'
        OR_CHAR                     equ '|'
        XOR_CHAR                    equ '^'
        LOGIC_NEGATION_CHAR         equ '~'
        POP_CHAR                    equ 'Z'
        DUP_CHAR                    equ 'Y'
        SWAP_STACK_CHAR             equ 'X'
        PUSH_COUNT_CHAR             equ 'N'
        PUSH_ID_CHAR                equ 'n'
        CALL_DEBUG_CHAR             equ 'g'
        SWAP_WITH_OTHER_CHAR        equ 'W'
        END_CHAR                    equ 0
        DIGIT_15_SMALL              equ 'f'
        DIGIT_10_SMALL              equ 'a'
        DIGIT_15_CAPITAL            equ 'F'
        DIGIT_10_CAPITAL            equ 'A'
        DIGIT_0                     equ '0'
        DIGIT_9                     equ '9'

section .bss
        exchange_num resq N + 1
        exchange_partner resd N + 1         ; Will be initialized to 0 (default partner number).

section .text
        global notec
        extern debug

notec:
        ; Preserve registers that will be used.
        push rbx
        push r12
        push r13
        push r14
        push r15
        push rbp
        mov rbp, rsp

        xor r12, r12       ; Set 0 as value of the flag that tells if the input mode is on.
        xor r13, r13       ; Set iterator on calc string.
        mov r14, rdi       ; Preserve id of this Noteć (0..N-1).
        inc r14            ; Increase the id so its in 1..N.
        mov r15, rsi       ; Preserve the calc string.

        jmp .main_loop_cond
        ; At the start of each loop, al contains current character
        ; Detailed information about operations: https://moodle.mimuw.edu.pl/mod/assign/view.php?id=47506
.input_small:
        ; Check if char is between 'a' and 'f'.
        ; If yes, the resulting digit will be equal to [al] - 'a' + 10.
        cmp al, DIGIT_15_SMALL
        jg .add             ; If char is greater than 'f', it can't be a digit.
        cmp al, DIGIT_10_SMALL
        jl .input_capital

        sub al, DIGIT_10_SMALL
        add al, 10
        jmp .add_new_digit

.input_capital:
        ; Check if char is between 'A' and 'F'.
        ; If yes, the resulting digit will be equal to [al] - 'A' + 10.
        cmp al, DIGIT_15_CAPITAL
        jg .add             ; If char is between 'F' and 'a', it can't be a digit.
        cmp al, DIGIT_10_CAPITAL
        jl .input_arabic_num

        sub al, DIGIT_10_CAPITAL
        add al, 10
        jmp .add_new_digit

.input_arabic_num:
        ; Check if char is between '0' and '9'.
        ; If yes, the resulting digit will be equal to [al] - '0'.
        cmp al, DIGIT_9
        jg .add             ; If char is between '9' and 'A', it can't be a digit.
        cmp al, DIGIT_0
        jl .add             ; If char is less than '0', it can't be a digit.

        sub al, DIGIT_0
        jmp .add_new_digit

.add_new_digit:
        ; If we get here, al contains value of next digit.
        xor rdi, rdi            ; Reset rdi. If we're not in input mode, we will add new digit to 0.
        ; Check if we're in input mode.
        cmp r12b, INPUT_MODE_ON
        jne .shift_and_add
        pop rdi                 ; If input mode is on, pop value from stack.

.shift_and_add:
        mov r12b, 1             ; Set input_mode flag to true.
        shl rdi, 4
        add dil, al
        push rdi
        jmp .increase_iterator  ; Jump to the end of the loop, but DO NOT reset input_mode flag.

.add:
        cmp al, ADDITION_CHAR
        jne .multiplicate

        pop rdi
        add qword [rsp], rdi
        jmp .finish_input

.multiplicate:
        cmp al, MULTIPLICATION_CHAR
        jne .negate_arithm

        pop rax
        pop rdi
        mul rdi
        push rax
        jmp .finish_input

.negate_arithm:
        cmp al, ARITHM_NEGATION_CHAR
        jne .and

        neg qword [rsp]
        jmp .finish_input

.and:
        cmp al, AND_CHAR
        jne .or

        pop rdi
        and qword [rsp], rdi
        jmp .finish_input

.or:
        cmp al, OR_CHAR
        jne .xor

        pop rdi
        or qword [rsp], rdi
        jmp .finish_input

.xor:
        cmp al, XOR_CHAR
        jne .negate_bits

        pop rdi
        xor qword [rsp], rdi
        jmp .finish_input

.negate_bits:
        cmp al, LOGIC_NEGATION_CHAR
        jne .pop

        not qword [rsp]
        jmp .finish_input

.pop:
        cmp al, POP_CHAR
        jne .duplicate

        pop rdi
        jmp .finish_input

.duplicate:
        cmp al, DUP_CHAR
        jne .swap_stack

        push qword [rsp]
        jmp .finish_input

.swap_stack:
        cmp al, SWAP_STACK_CHAR
        jne .push_count

        pop rdi
        pop rsi
        push rdi
        push rsi
        jmp .finish_input

.push_count:
        cmp al, PUSH_COUNT_CHAR
        jne .push_id

        push N
        jmp .finish_input

.push_id:
        cmp al, PUSH_ID_CHAR
        jne .call_debug

        push r14
        dec qword [rsp]
        jmp .finish_input

.call_debug:
        cmp al, CALL_DEBUG_CHAR
        jne .swap_with_other

        mov rbx, rsp                ; Save stack pointer.
        and rsp, -16                ; 16-align the stack.
        mov rdi, r14                ; First argument (Noteć's id).
        dec rdi                     ; Decrease rdi, because r14 contained (id + 1) instead of id.
        mov rsi, rbx                ; Second argument (Noteć's top stack pointer).
        call debug
        mov rsp, rbx                ; Restore stack pointer.
        lea rsp, [rsp + 8 * rax]    ; Move stack according to debug's result.
        jmp .finish_input

.swap_with_other:
        cmp al, SWAP_WITH_OTHER_CHAR
        ; If all above chars were incorrect, we must have encountered character finishing input.
        jne .finish_input

        ; Synchronization works this way:

        ; Every Noteć has a cell in exchange_num and exchange_partner
        ; with number equal to id + 1 (Noteć with id = 0 owns cell with number 1 etc.).

        ; When ready to exchange, put your top number to exchange_num tab
        ; and the id of the partner to the exchange_partner tab.
        ; Then, wait for your partner to put your id in his exchange_partner cell.
        ; N is less than 2^32 (source: forum) so id in notation 1..N will fit into dword.

        ; r14 contains id + 1 (1..N) of this Noteć.
        pop rdi                             ; Get partner's id (0..N-1), should be enough to fit into dword.
        pop rsi                             ; Get top value.
        inc rdi                             ; Increase partner's id so it's in notation 1..N.

        ; Put information about yourself to static memory.
        lea rcx, [rel exchange_num]
        mov qword [rcx + 8 * r14], rsi      ; Put top value to exchange_num.
        lea rdx, [rel exchange_partner]
        mov dword [rdx + 4 * r14], edi      ; Put id of your partner to exchange_partner.

        ; Synchronize.
        ; rdx contains rel exchange_partner.
        ; rcx contains rel exchange_num.
        ; rdi contains partner's id (in 1..N notation).
.wait_for_partner:
        cmp r14d, dword [rdx + 4 * rdi]     ; Check if partner is ready to exchange with you.
        jne .wait_for_partner

        ; Perform the exchange.
        mov rsi, qword [rcx + 8 * rdi]                          ; Get the exchanged number.
        mov dword [rdx + 4 * rdi], DEFAULT_EXCHANGE_PARTNER     ; Reset partner's exchange partner.

.synchronize:
        cmp dword[rdx + 4 * r14], DEFAULT_EXCHANGE_PARTNER      ; Wait until your partner resets your exchange partner number.
        jne .synchronize
        push rsi        ; Push result on the stack.

.finish_input:
        xor r12, r12    ; Reset the INPUT_ON flag.
.increase_iterator:
        inc r13
.main_loop_cond:
        ; Check next byte in input and finish if it's equal to '\0'.
        mov al, byte [r15 + r13]
        cmp al, END_CHAR
        jne .input_small

        pop rax         ; Return value is located on the top of the stack.

        ; Repair stack and values in registers.
        mov rsp, rbp
        pop rbp
        pop r15
        pop r14
        pop r13
        pop r12
        pop rbx
        ret