section .rodata
    BUFFER_SIZE         equ 4096  ; Must be greater or equal to 4, preferably something greater than 1000.
    NULL                equ 0
    ZERO_CHAR           equ 48
    NINE_CHAR           equ 57
    ; Constants used in calculating the polynomial.
    MODULO_SECOND       equ 8681818192107816089
    MODULO_G_SHIFT      equ 63
    MODULO_L_SHIFT      equ 19
    POLY_MODULO         equ 0x10FF80
    POLY_SUBTRACT       equ 0x80
    UTF_NEXT_LOWER_B    equ 10000000b
    UTF_NEXT_UPPER_B    equ 10111111b
    UTF_1BYTE_MAX       equ 01111111b
    UTF_2BYTES_MAX      equ 11011111b
    UTF_3BYTES_MAX      equ 11101111b
    UTF_4BYTES_MAX      equ 11110111b
    MAX_CHAR_UNI        equ 0x10FFFF
    ; Constants used to transform unicode to utf8.
    UNI_2BYTES_MIN      equ 0x80
    UNI_3BYTES_MIN      equ 0x0800
    UNI_4BYTES_MIN      equ 0x010000
    UNI_2BYTES_AND      equ 0x1F
    UNI_3BYTES_AND      equ 0x0F
    UNI_4BYTES_AND      equ 0x07
    UNI_OTHER_AND       equ 0x3F
    UNI_2BYTES_OR       equ 0xC0
    UNI_3BYTES_OR       equ 0xE0
    UNI_4BYTES_OR       equ 0xF0
    UNI_OTHER_OR        equ 0x80
    ; Constant used with syscalls.
    SYS_READ            equ 0
    SYS_WRITE           equ 1
    SYS_EXIT            equ 60
    EXIT_SUCCESS        equ 0
    EXIT_FAILURE        equ 1
    STDIN               equ 0
    STDOUT              equ 1

section .bss
    input_buffer resb BUFFER_SIZE
    input_buffer_curr_size resd 1
    input_buffer_curr_index resd 1
    output_buffer resb BUFFER_SIZE
    output_buffer_size resd 1

section .text
    global _start
    global parse_parameters
    global parse_number
    global calculate_polynomial
    global read_next_byte
    global print_bytes
    global flush_output_buffer
    global exit_failure

_start:
    ; Move program arguments info and rsp to preserved registers.
    mov r13, [rsp]
    lea r14, [rsp + 8]
    mov r15, rsp

    ; If there are no parameters, the program usage is incorrect.
    cmp r13, 1
    je exit_failure

.param_count_corr:
    ; Allocate memory for polynomial's coefficients.
    mov rax, r13
    imul rax, 8
    sub rsp, rax

    mov qword [rsp], r13    ; First number in the array is the number of arguments.
    dec qword [rsp]         ; qword [rsp] is the number of polynomial's coefficients.

    ; Parse parameters.
    mov rdi, rsp
    mov rsi, r14
    call parse_parameters
    mov r12, rsp

    ; Handle all bytes on standard input.
    mov dword [input_buffer_curr_index], BUFFER_SIZE
    mov dword [input_buffer_curr_size], BUFFER_SIZE
.read:
    mov rdi, r12
    call handle_next_utf
    cmp rax, 0
    je .read

    ; Fix some things before finishing.
    call flush_output_buffer
    mov rsp, r15                ; Restore stack.
    ; Exit with success.
    mov rax, SYS_EXIT
    mov rdi, EXIT_SUCCESS
    syscall

; Reads and parses next utf-8 character on the input,
; then performs calculations of it and prints it.
; Exits with code 1 if error was encountered.
; Arguments:    rdi - address of polynomial's coefficients
; Returns:      rax - 0 if character was read
;                     1 if there was nothing to read
; Modifies: rax, rdi, rsi, rdx, rcx, r8, r13, r14, r15
handle_next_utf:
    mov r15, rdi        ; Preserve the address.

    call read_next_utf
    ; Check if anything was read. If not, return 1.
    cmp rax, 0
    jne .check_if_one_byte
    mov rax, 1
    ret

.check_if_one_byte:
    ; Check if only one byte was read. If yes, print it.
    cmp rax, 1
    jne .calc
    mov rdi, rdx
    mov rsi, 1
    call print_bytes
    mov rax, 0
    ret

.calc:
    ; Translate utf-8 character to its unicode number.
    mov rdi, rdx    ; The utf-8 character.
    mov rsi, rax    ; Number of bytes.
    call trans_utf_to_unicode

    ; Check the correctness of the character.
    ; Numbers of bytes is already located in rsi.
    mov rdi, rax
    call is_char_correct

    ; Calculate the value of w(x - 0x80) + 0x80.
    mov rdi, rax
    sub rdi, POLY_SUBTRACT
    mov rsi, r15
    call calculate_polynomial
    add rax, POLY_SUBTRACT

    mov rdi, rax
    call trans_unicode_to_utf

    mov rdi, rax
    mov rsi, rdx
    call print_bytes

    mov rax, 0
    ret

; Checks if character with given unicode is below max value
; and if it's the shortest notation.
; Arguments:    rdi - unicode number
;               rsi - number of bytes the character is stored in
; Modifies: Nothing.
is_char_correct:
    ; Check if the number is not too big.
    cmp rdi, MAX_CHAR_UNI
    ja exit_failure

    ; Check if number of bytes is right.
.check_four:
    cmp rdi, UNI_4BYTES_MIN
    jb .check_three
    cmp rsi, 4
    jne exit_failure
    ret
.check_three:
    cmp rdi, UNI_3BYTES_MIN
    jb .check_two
    cmp rsi, 3
    jne exit_failure
    ret
.check_two:
    cmp rdi, UNI_2BYTES_MIN
    jb .check_one
    cmp rsi, 2
    jne exit_failure
    ret
.check_one:
    cmp rsi, 1
    jne exit_failure
    ret

; Translates a character in utf-8 to its unicode number.
; Works for 2-4 bytes.
; Arguments:    rdi - character in utf-8
;               rsi - number of bytes the character is stored in
; Returns:      rax - unicode number of the character
; Modifies rax, rdi, rcx
trans_utf_to_unicode:
    ; The loop will work for bytes with pattern 10xxxxxx.
    mov rcx, rsi
    dec rcx
    xor rax, rax

    ; Shift rdi two bytes to the left,
    ; so that the last byte has form xxxxxx00
    ; where x are last 6 digits of the unicode number
    shl rdi, 2
.loop:
    mov al, dil     ; Move current byte to al.
    shr al, 2       ; Get rid of the zeros in al.
    ror rax, 6      ; Rotate rax, so that it has a form x...x0...0.
    shr rdi, 8      ; Shift rdi to the right, so that the next byte is ready to be read.
    loop .loop

    ; Now dil contains last 6 bits of the first byte.
    ; Adjust dil so that it contains only bytes that are part of the unicode number.
    ; Simple switch branch is used, because loop would include way more jumps.

.adjust_two:
    cmp rsi, 2
    jne .adjust_three
    shl dil, 1      ; Get rid of redundant bytes.
    mov al, dil     ; Move the important bytes to al.
    shr al, 3       ; Get rid of zeros at the end.
    jmp .return
.adjust_three:
    cmp rsi, 3
    jne .adjust_four
    shl dil, 2
    mov al, dil
    shr al, 4
    jmp .return
.adjust_four:
    shl dil, 3
    mov al, dil
    shr al, 5
.return:
    ; Rotate rax to the left by 6 * (numbers of bytes - 1) and return it.
    ; This way we get our unicode number in rax.
    mov rcx, rsi
    dec rcx
    imul rcx, 6
    rol rax, cl
    ret

; Translates unicode number to corresponding utf-8 character.
; Works for 2-4 bytes.
; Arguments:    rdi - unicode number
; Returns:      rax - utf-8 characters
;               rdx - number of bytes occupied by that character
; Modifies rax, rdi, rsi, rcx, rdx, r8, r9.
trans_unicode_to_utf:
    mov rax, rdi    ; We will store the answer in rax.

    ; If it's less, the number has 2 bytes.
    cmp rdi, UNI_3BYTES_MIN
    jae .comp_three
    mov rdx, 2      ; Number of bytes in corresponding utf-8 character.
    ; Set the first byte.
    shr rax, 6
    and rax, UNI_2BYTES_AND
    or rax, UNI_2BYTES_OR
    jmp .fill_other
.comp_three:
    ; If it's less, the number has 3 bytes. Otherwise it has 4 bytes.
    cmp rdi, UNI_4BYTES_MIN
    jae .comp_four
    mov rdx, 3
    ; Set the first byte.
    shr rax, 12
    and rax, UNI_3BYTES_AND
    or rax, UNI_3BYTES_OR
    jmp .fill_other
.comp_four:
    mov rdx, 4
    ; Set the first byte.
    shr rax, 18
    and rax, UNI_4BYTES_AND
    or rax, UNI_4BYTES_OR
.fill_other:
    mov rcx, rdx
    dec rcx     ; Counter for loop is (number of bytes - 1).
    ; r8 will contain number of bytes to shift in current iteration.
    ; A shift must be performed by a multiplication of 6.
    mov r8, rdx
    sub r8, 2
    imul r8, 6
.loop:
    mov rsi, rdi    ; The original number.

    ; We move some registers here, because cl register has to be used.
    mov r9, rcx
    mov rcx, r8
    shr rsi, cl     ; Shift by multiplication of 6.
    mov rcx, r9

    shl rax, 8      ; Shift by whole byte, so that the last one is free for the next utf-8 byte.

    mov al, sil
    and al, UNI_OTHER_AND
    or al, UNI_OTHER_OR

    sub r8, 6
    loop .loop
    ret

; Calculates the polynomial's value for an utf-8 value.
; Arguments:    rdi - the argument of the polynomial (with trailing zeros)
;               rsi - address of polynomial's coefficients
;                   (starting with number of coefficients)
; Returns:      rax -  the calculated value
; Modifies rax, rdx, rcx, r8, r9.
calculate_polynomial:
    xor r9, r9
    mov rcx, qword [rsi]    ; Number of coefficients.
    mov r8, POLY_MODULO

.loop:
    ; Multiply current value by x and add a_k.
    imul r9, rdi
    add r9, qword[rsi + 8 * rcx]

    ; Calculate current result modulo POLY_MODULO.
    ; This algorithm is the one used by gcc.
    mov rdx, MODULO_SECOND
    mov rax, r9
    imul rdx
    mov rax, rdx
    mov rdx, r9
    sar rdx, MODULO_G_SHIFT
    sar rax, MODULO_L_SHIFT
    sub rax, rdx
    imul rdx, rax, POLY_MODULO
    mov rax, r9
    sub rax, rdx
    mov r9, rax

    loop .loop

    ret

; Reads and parses next utf-8 character on input.
; Exits with code 1 if error was encountered.
; Arguments: None.
; Returns:  rax - number of read bytes
;           rdx - read bytes
; Modifies: rax, rdi, rsi, rdx, r13, r14.
read_next_utf:
    call read_next_byte
    ; Check the result. Return 0 in rax if there was nothing to read.
    cmp ah, 0
    je .compare_byte
    mov rax, 0      ; We read 0 bytes.
    ret

.compare_byte:
    xor r14, r14
    mov r14, rax    ; Preserve rax in other register.

    ; If read byte is greater than first byte of four byte utf character, it is incorrect.
    cmp al, UTF_4BYTES_MAX
    ja exit_failure

    ; If read byte is greater than first byte of three byte utf character,
    ; the character must have four bytes.
    cmp al, UTF_3BYTES_MAX
    ja .read_four

    ; If read byte is greater than first byte of two byte utf character,
    ; the character must have three bytes.
    cmp al, UTF_2BYTES_MAX
    ja .read_three

    ; If read byte is greater than a byte that would be found in one of next bytes in utf (10xxxxxx),
    ; the character must have two bytes.
    cmp al, UTF_NEXT_UPPER_B
    ja .read_two

    ; If read byte is less or equal to 0x7F (01111111), it consists of one single byte.
    ; Otherwise, the character is wrong.
    cmp al, UTF_1BYTE_MAX
    jbe .read_one
    jmp exit_failure

.read_one:
    ; We have already read the byte, return it.
    xor rdx, rdx
    mov dl, al
    mov rax, 1  ; Number of read bytes.
    ret

    ; Move into rcx numbers of bytes that still have to be read.
    ; Move into r13 total number of read bytes.
.read_two:
    mov rcx, 1
    mov r13, 2
    jmp .read_loop
.read_three:
    mov rcx, 2
    mov r13, 3
    jmp .read_loop
.read_four:
    mov rcx, 3
    mov r13, 4

.read_loop:
    ; Preserve rcx in case the system does something irrational with it when syscalling.
    push rcx
    call read_next_byte
    pop rcx

    ; If reading failed, exit with error.
    cmp ah, 0
    jne exit_failure

    ; Check if the byte matches 10xxxxxx (must be in range from 10000000 to 10111111).
    cmp al, UTF_NEXT_LOWER_B
    jb exit_failure
    cmp al, UTF_NEXT_UPPER_B
    ja exit_failure

    shl r14, 8      ; Shift r14 by one byte, so that we can place the new byte in r14b.
    mov r14b, al    ; Move new byte to the register.

    loop .read_loop

    ; Move the results to proper registers.
    mov rax, r13
    mov rdx, r14
    ret

; Returns the next byte from input in the [al] register.
; Arguments:    None.
; Returns:      ah - 0 if reading was successful,
;                    1 otherwise.
; Modifies rax, rdi, rsi, rdx.
read_next_byte:
    mov eax, dword [input_buffer_curr_index]
    cmp eax, dword [input_buffer_curr_size]
    jne .return_next

    mov dword [input_buffer_curr_index], 0    ; Reset input_buffer current index.

    ; Read BUFFER_SIZE bytes to input_buffer.
    mov rax, SYS_READ
    mov rdi, STDIN
    mov rsi, input_buffer
    mov rdx, BUFFER_SIZE
    syscall

    mov dword [input_buffer_curr_size], eax

    ; If no bytes were read, set ah to 1 and return.
    cmp rax, 0
    jne .return_next
    mov ah, 1
    ret

.return_next:
    mov ebx, dword [input_buffer_curr_index]
    mov al, byte [input_buffer+ebx]
    inc dword [input_buffer_curr_index]
    mov ah, 0
    ret


; Parses program's parameters.
; Arguments:    rdi - pointer to memory for parsed parameters.
;               rsi - pointer to parameters.
; Returns: Nothing.
; Modifies rcx, r8, r9, r10, rsi, rdi, rax, rdx.
parse_parameters:
    mov rcx, qword [rdi]
    mov r10, rsi
    mov r9, 1
    mov r8, rdi
.parse:
    mov rdi, qword [r10 + 8 * r9]
    call parse_number
    mov qword [r8 + 8 * r9], rax
    inc r9
    loop .parse
    ret

; Parses string pointed by rdi. The string must end with 0.
; Exits with code 1 if string is not a valid number.
; Arguments:    rdi - string to be parsed into a number
; Returns:      rax - parsed number
; Modifies rsi, rax, rdx.
parse_number:
    ; If string is empty, exit(1).
    cmp byte [rdi], NULL
    je exit_failure

    ; Main loop for parsing.
    xor rsi, rsi
    xor rax, rax
    xor rdx, rdx
    jmp .condition

.parsing_loop:
    cmp dl, ZERO_CHAR
    jb exit_failure

    cmp dl, NINE_CHAR
    ja exit_failure

    ; Add current digit to result through formula: rax = 10 * rax + (dl - '0')
    imul rax, 10
    sub dl, ZERO_CHAR
    add rax, rdx

    inc rsi

.condition:
    mov dl, byte [rdi + rsi]
    cmp dl, NULL
    jne .parsing_loop

    ret

; Prints from 1 to 4 bytes on output.
; Bytes are stored in a buffer and released when full (or when buffer is flushed).
; Assumes correct data.
; Arguments: rdi - bytes to be stored (in last fours octets),
;            rsi - number of bytes stored
; Returns:  Nothing.
; Modifies: rax, rdi, rsi, rdx, rcx, r8, r9, r10.
print_bytes:
    ; Preserve the values.
    mov r9, rdi
    mov r10, rsi    ; Will be used as counter later.

    ; Update buffer size.
    xor r8, r8
    mov r8d, dword [output_buffer_size]
    add r8, r10

    ; If buffer would overflow after adding these bytes, flush it.
    cmp r8, BUFFER_SIZE
    jbe .print
    call flush_output_buffer

.print:
    ; Set r8 so that it points to the position of next place to put our byte.
    add dword [output_buffer_size], r10d
    mov rcx, r10     ; Use this value in counter.

    ; r8 will point to characters in buffer in descending order.
    xor r8, r8
    mov r8d, dword [output_buffer_size]
    dec r8
    mov r10, 1

.print_loop:
    mov byte [output_buffer + r8], r9b
    dec r8
    shr r9, 8       ; Shift r9 so that the next byte to be printed is in r9b.
    loop .print_loop

    ret

; Flushes output buffer. Prints output_buffer_size bytes to standard output.
; Arguments: None.
; Returns: Nothing.
; Modifies: rax, rdi, rsi, rdx.
flush_output_buffer:
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    mov rsi, output_buffer
    xor rdx, rdx
    mov edx, dword [output_buffer_size]
    syscall

    mov dword [output_buffer_size], 0
    ret

; Exits with return code 1 and flushes output buffer.
; Modifies rax and rdi, but ends the program after calling.
exit_failure:
    call flush_output_buffer
    mov rax, SYS_EXIT
    mov rdi, EXIT_FAILURE
    syscall