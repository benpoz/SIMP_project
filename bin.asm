# Calculate binomial coefficient

##### Enable interrupts
#out $zero, $zero, $imm2, $imm1, 1, 0  # Enable irq0
#out $zero, $zero, $imm2, $imm1, 1, 1  # Enable irq1
#out $zero, $zero, $imm2, $imm1, 1, 2  # Enable irq2
#####

##### Set interrupt handler
#sll $sp, $imm1, $imm2, $zero, 1, 11  # Set $sp=1<<11=2048
#out $zero, $imm1, $zero, $imm2, 6, irq_handler
#####

.word 0x100 8 #initialize n, k in data memory
.word 0x101 3

lw $a0, $imm1 ,$zero, 0x100, 0  #  load n
lw $a1, $imm1 ,$zero, 0x101, 0  # load k
jal $ra, $zero, $zero, $imm2, 0, bin		# calc $v0 = fib(x)
sw $zero, $zero, $imm2, $v0, 0, 0x102

bin:

beq $a1, $zero, base_case # Base case: if k == 0 or n == k, return 1
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

#####
#irq_handler:
    # Handle interrupts
    #in $t1, $zero, $imm2, $zero, 0, 3  # Read irq0status
    #in $t2, $zero, $imm2, $zero, 0, 4  # Read irq1status
    #in $t3, $zero, $imm2, $zero, 0, 5  # Read irq2status

    # Clear interrupt statuses
    out $zero, $zero, $imm2, $zero, 0, 3
    out $zero, $zero, $imm2, $zero, 0, 4
    out $zero, $zero, $imm2, $zero, 0, 5

    reti $zero, $zero, $zero, $zero, 0, 0