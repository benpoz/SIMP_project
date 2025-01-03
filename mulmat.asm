# Matrix multiplication of two 4x4 matrices 
# Matrix A: 0x100 to 0x10F
# Matrix B: 0x110 to 0x11F
# Result Matrix C: 0x120 to 0x12F
# Base addresses

.word 0x100 1
.word 0x101 1
.word 0x102 1
.word 0x103 1
.word 0x104 1
.word 0x105 1
.word 0x106 1
.word 0x107 1
.word 0x108 1
.word 0x109 1
.word 0x10A 1
.word 0x10B 1
.word 0x10C 1
.word 0x10D 1
.word 0x10E 1
.word 0x10F 1
.word 0x110 1
.word 0x111 1
.word 0x112 1
.word 0x113 1
.word 0x114 1
.word 0x115 1
.word 0x116 1
.word 0x117 1
.word 0x118 1
.word 0x119 1
.word 0x11A 1
.word 0x11B 1
.word 0x11C 1
.word 0x11D 1
.word 0x11E 1
.word 0x11F 1



#####
# Enable interrupts
out $zero, $zero, $imm2, $imm1, 1, 0  # Enable irq0
out $zero, $zero, $imm2, $imm1, 1, 1  # Enable irq1
#out $zero, $zero, $imm2, $imm1, 1, 2  # Enable irq2

# Set interrupt handler
sll $sp, $imm1, $imm2, $zero, 1, 11  # Set $sp=1<<11=2048
out $zero, $imm1, $zero, $imm2, 6, irq_handler
#####

# Base addresses
add $s0, $zero, $imm1, $zero, 0x100, 0  # Matrix A
add $s1, $zero, $imm1, $zero, 0x110, 0  # Matrix B
add $s2, $zero, $imm1, $zero, 0x120, 0  # Result Matrix C

# Loop through rows of A
add $t0, $zero, $zero, $zero, 0, 0  # Row index i
loop_i:
    beq $zero, $imm2, $t0, $imm1, end, 4  # If i == 4, end

    # Loop through columns of B
    add $t1, $zero, $zero, $zero, 0, 0  # Column index j
    loop_j:
        beq $zero, $imm2, $t1, $imm1, next_i, 4  # If j == 4, next row

        # Calculate dot product for C[i][j]
        add $v0, $zero, $zero, $zero, 0, 0  # Sum
        add $t2, $zero, $zero, $zero, 0, 0  # Index k
        loop_k:
            beq $zero, $imm2, $t2, $imm1, store, 4  # If k == 4, store result

            # Load A[i][k] and B[k][j]
            mac $a0 , $t0, $imm1, $t2 ,4,0  # $t0 = offset for A[i][k]
            lw $a0,$s0,$a0,$zero, 0, 0        #$a0 = A[i][k]
            mac $a1, $t2, $imm1, $t1, 4, 0  #$t1 = offset for B[k][j]
            lw $a1, $s1, $a1, $zero, 0, 0     #$a1 = B[k][j]

            # Multiply and accumulate
            mac $v0, $a0, $a1, $v0, 0, 0

            # Increment k
            add $t2, $t2, $imm1, $zero, 1, 0
            beq $zero, $zero, $zero, $imm1, loop_k, 0

        store:
        # Store result in C[i][j]
        mac $a0, $t0, $imm1, $t1, 4, 0 #offset for C matrix 
        sw $zero, $a0, $s2, $v0, 0, 0

        # Increment j
        add $t1, $t1, $imm1, $zero, 1, 0
        beq $zero, $zero, $zero, $imm1, loop_j, 0

    next_i:
    # Increment i
    add $t0, $t0, $imm1, $zero, 1, 0
    beq $zero, $zero, $zero, $imm1, loop_i, 0

end:
halt $zero, $zero, $zero, $zero, 0, 0

#####
irq_handler:
    # Handle interrupts
    in $t1, $zero, $imm2, $zero, 0, 3  # Read irq0status
    in $t2, $zero, $imm2, $zero, 0, 4  # Read irq1status
    in $t3, $zero, $imm2, $zero, 0, 5  # Read irq2status

    # Clear interrupt statuses
    out $zero, $zero, $imm2, $zero, 0, 3
    out $zero, $zero, $imm2, $zero, 0, 4
    out $zero, $zero, $imm2, $zero, 0, 5

    reti $zero, $zero, $zero, $zero, 0, 0
#####