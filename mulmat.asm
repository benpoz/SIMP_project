# Matrix multiplication of two 4x4 matrices
# Matrix A: 0x100 to 0x10F
# Matrix B: 0x110 to 0x11F
# Result Matrix C: 0x120 to 0x12F
# Base addresses
add $s0, $zero, $imm1, $zero, 0x100, 0  # Matrix A
add $s1, $zero, $imm1, $zero, 0x110, 0  # Matrix B
add $s2, $zero, $imm1, $zero, 0x120, 0  # Result Matrix C

# Loop through rows of A
add $t0, $zero, $zero, $zero, 0, 0  # Row index i
loop_i:
    beq $t0, $imm1, $zero, $imm1, end, 4  # If i == 4, end

    # Loop through columns of B
    add $t1, $zero, $zero, $zero, 0, 0  # Column index j
    loop_j:
        beq $t1, $imm1, $zero, $imm1, next_i, 4  # If j == 4, next row

        # Calculate dot product for C[i][j]
        add $t2, $zero, $zero, $zero, 0, 0  # Sum
        add $t3, $zero, $zero, $zero, 0, 0  # Index k
        loop_k:
            beq $t3, $imm1, $zero, $imm1, store, 4  # If k == 4, store result

            # Load A[i][k] and B[k][j]
            add $a0, $s0, $t0, $t3, 0, 0
            lw $a0, $a0, 0
            add $a1, $s1, $t3, $t1, 0, 0
            lw $a1, $a1, 0

            # Multiply and accumulate
            mac $t2, $a0, $a1, $t2

            # Increment k
            add $t3, $t3, $imm1, $zero, 1, 0
            beq $zero, $zero, $zero, $imm1, loop_k, 0

        store:
        # Store result in C[i][j]
        add $a0, $s2, $t0, $t1, 0, 0
        sw $a0, $t2, 0

        # Increment j
        add $t1, $t1, $imm1, $zero, 1, 0
        beq $zero, $zero, $zero, $imm1, loop_j, 0

    next_i:
    # Increment i
    add $t0, $t0, $imm1, $zero, 1, 0
    beq $zero, $zero, $zero, $imm1, loop_i, 0

end:
halt $zero, $zero, $zero, $zero, 0, 0
	
	
	