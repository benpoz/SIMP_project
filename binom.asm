# Calculate binomial coefficient

# Load n and k
lw $a0, $zero, 0x100  # n
lw $a1, $zero, 0x101  # k

# Base case: if k == 0 or n == k, return 1
beq $a1, $zero, base_case
beq $a0, $a1, base_case

# Recursive case: binom(n-1, k-1) + binom(n-1, k)
sub $t0, $a0, $imm1, $zero, 1, 0  # n-1
sub $t1, $a1, $imm1, $zero, 1, 0  # k-1

# Call binom(n-1, k-1)
add $a0, $t0, $zero, $zero, 0, 0
add $a1, $t1, $zero, $zero, 0, 0
jal $ra, $zero, $zero, $imm1, binom, 0
move $t2, $v0

# Call binom(n-1, k)
add $a0, $t0, $zero, $zero, 0, 0
add $a1, $a1, $zero, $zero, 0, 0
jal $ra, $zero, $zero, $imm1, binom, 0

# Add results
add $v0, $t2, $v0, $zero, 0, 0
sw $v0, $zero, 0x102
j end

base_case:
add $v0, $zero, $imm1, $zero, 1, 0  # Return 1
sw $v0, $zero, 0x102

end:
halt $zero, $zero, $zero, $zero, 0, 0
