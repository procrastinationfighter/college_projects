.arm
.data
.balign 4
matrix: .word 0

.balign 4
width:  .word 0

.balign 4
height: .word 0

.balign 4
weight: .word 0

.balign 4

.text
.global matrix
.global start
.type start, %function

.global step
.type step, %function

.type sum_neighbors, %function

/*
Saves the arguments in global variables.
Arguments:
r0 - width
r1 - height
r2 - address of the matrix
r3 - weight
*/
start:
    push { r4 }

    ldr r4, =width
    str r0, [r4, #0]

    ldr r4, =height
    str r1, [r4, #0]

    ldr r4, =matrix
    str r2, [r4, #0]

    ldr r4, =weight
    str r3, [r4, #0]

    pop { r4 }
    bx lr

/*
Arguments:
r0 - address of the new first column values
*/
step:
    push { r4-r11, lr }
    push { r0 }

    /* Update the fields. */
    ldr r1, =width
    ldr r2, [r1, #0]
    mov r3, #4
    mul r1, r2, r3

    ldr r10, =height
    ldr r7, [r10, #0]
    ldr r10, =weight
    ldr r8, [r10, #0]
    ldr r10, =matrix
    ldr r9, [r10, #0]

    /* Iterate through columns backwards (right to left). */
    b .update_col_cond

.update_column:
    /* Indices:
     r3 - left column, r4 - current column
     Rows are switched by adding 4 * value of width to the indices.
    */
    mov r4, #4
    mul r3, r2, r4
    mov r4, r3
    sub r3, r3, #4

    /* Calculate the sum for the first row,
    then in the loop calculate the rest of them.
    */
    mov r5, #1
    cmp r7, #1
    bne .sum_first
    mov r5, #3

.sum_first:

    bl sum_neighbors

    mov r6, r11

    mov r0, #0
    b .update_field_cond

.update_field:
    /* Every loop: calculate sum for current field, then update upper's field value. */
    mov r5, #0
    mov r10, r7
    sub r10, r10, #1
    cmp r0, r10
    bne .call_sum
    mov r5, #2
.call_sum:
    ldr r10, [r9, r4]

    /* Multiply r6 * weight */
    cmp r6, #0
    blt .mul_neg_in
    mul r11, r6, r8
    lsr r11, r11, #8
    b .add_delta_in
.mul_neg_in:
    push { r1 }

    mov r1, #0
    sub r1, r1, r6
    mul r11, r1, r8
    lsr r11, r11, #8
    mov r6, #0
    sub r11, r6, r11

    pop { r1 }
.add_delta_in:

    add r6, r10, r11

    bl sum_neighbors

    /* Update previous field and push current to r6. */
    sub r4, r4, r1
    str r6, [r9, r4]
    add r4, r4, r1

    mov r6, r11


.update_field_cond:
    add r0, r0, #1
    cmp r7, r0
    bgt .update_field

    /* Update last row. */
    cmp r6, #0
    blt .mul_neg_out
    mul r11, r6, r8
    lsr r11, r11, #8
    b .add_delta_out
.mul_neg_out:
    push { r1 }

    mov r1, #0
    sub r1, r1, r6
    mul r11, r1, r8
    lsr r11, r11, #8
    mov r6, #0
    sub r11, r6, r11

    pop { r1 }
.add_delta_out:
    ldr r10, [r9, r4]
    add r10, r10, r11
    str r10, [r9, r4]

.update_col_cond:
    /* Skip the first column. */
    sub r2, r2, #1
    cmp r2, #0
    bgt .update_column

    /* Replace the first column with new values. */
    /* 4 * Height is in r6, 4 * width in r1. */
    pop { r0 }
    mov r3, #0
    mov r4, #0
    mov r5, #4
    mul r6, r7, r5
.change_first:
    ldr r5, [r0, r4]
    str r5, [r9, r3]

.change_first_cond:
    add r3, r1
    add r4, #4
    cmp r6, r4
    bgt .change_first

    pop {r4-r11, lr}
    bx lr


/*
Sums neighbors' values.
DOES NOT FOLLOW THE ABI.
r1 - width * 4
r3 - left column
r4 - right column
r5 - row id (0 if inner row, 1 if first row, 2 if last row, 3 if its both first and last)
r9 - matrix pointer

The result is placed in r11.
*/
sum_neighbors:
    push { r8 }
    push { r10 }
    /* r8 counts number of summed neighbors. */
    mov r8, #0
    mov r11, #0
.upper:
    /* If the first row, skip. */
    cmp r5, #1
    beq .lower
    cmp r5, #3
    beq .left

    ldr r10, [r9, r3]
    add r11, r11, r10
    ldr r10, [r9, r4]
    add r11, r11, r10

    add r8, #2
    add r3, r3, r1
    add r4, r4, r1

.lower:
    /* If the last row, skip. */
    cmp r5, #2
    beq .left

    add r3, r3, r1
    add r4, r4, r1

    ldr r10, [r9, r3]
    add r11, r11, r10
    ldr r10, [r9, r4]
    add r11, r11, r10

    add r8, #2
    sub r3, r3, r1
    sub r4, r4, r1

.left:
    ldr r10, [r9, r3]
    add r11, r11, r10

    add r8, #1

.sum_ret:
    /* r4 contains index of current field */
    ldr r10, [r9, r4]
    mul r5, r10, r8
    sub r11, r11, r5

    pop { r10 }
    pop { r8 }
    bx lr

.end
