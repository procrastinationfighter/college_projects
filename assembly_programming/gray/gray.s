.arm
.text
.global R_WEIGHT
.global G_WEIGHT
.global B_WEIGHT
.global make_gray
.type make_gray, %function
make_gray:
    push {r5-r10}

    mul r4, r1, r2
    mov r7, #3
    mul r10, r4, r7
    eor r4, r4, r4
    eor r1, r1, r1

    ldr r3, =R_WEIGHT
    ldrb r7, [r3, #0]

    ldr r3, =G_WEIGHT
    ldrb r8, [r3, #0]

    ldr r3, =B_WEIGHT
    ldrb r9, [r3, #0]

loop:
    cmp r1, r10
    beq finish

    ldrb r3, [r0, r1]
    mul r5, r3, r7

    add r1, r1, #1
    ldrb r3, [r0, r1]
    mul r6, r3, r8
    add r5, r5, r6

    add r1, r1, #1
    ldrb r3, [r0, r1]
    mul r6, r3, r9
    add r5, r5, r6

    mov r5, r5, LSR #8
    strb r5, [r0, r4]

    add r1, r1, #1
    add r4, r4, #1

    b loop

finish:
    pop {r5-r10}
    bx lr
.end
