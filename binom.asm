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

sll $sp, $imm1, $imm2, $zero, 1, 11 # set $sp = 1 << 11 = 2048
lw $a0, $imm1 ,$zero, $zero 0x100, 0  #  load n
lw $a1, $imm1 ,$zero, $zero 0x101, 0  # load k
jal $ra, $zero, $zero, $imm2, 0, bin		# calc $v0
sw $zero, $zero, $imm2, $v0, 0, 0x102
halt $zero, $zero, $zero, $zero, 0, 0

bin:
add $sp, $sp, $imm2, $zero, 0, -4		# adjust stack for 4 items
sw $zero, $sp, $imm2, $s0, 0, 3			# save $s0
sw $zero, $sp, $imm2, $ra, 0, 2			# save return address
sw $zero, $sp, $imm2, $a0, 0, 1
sw $zero, $sp, $imm2, $a1, 0, 0
bne $zero, $a1, $zero, $imm1, L1, 0 #base conditions
bne $zero, $a1, $a0, $imm1, L1, 0
add $v0, $zero, $imm1, $zero, 1, 0  # Return 1
beq $zero, $zero, $zero, $imm2, 0, EXIT


L1:
sub $a0, $a0, $imm1, $zero, 1, 0  # n-1 # Call binom(n-1, k-1)
sub $a1, $a1, $imm1, $zero, 1, 0  # k-1
jal $ra, $zero, $zero, $imm1, bin, 0
add $s0, $v0, $imm2, $zero, 0, 0 # $s0 = binom(n-1, k-1)
lw $a0, $sp, $imm2, $zero, 0, 0 #restore $a0 = n
lw $a1, $sp, $imm2, $zero, 0, 1 #restore $a1 = k
sub $a0, $a0, $imm1, $zero, 1, 0
jal $ra, $zero, $zero, $imm1, bin, 0
add $v0, $v0, $s0, $zero, 0, 0
lw $a1, $sp, $imm2, $zero, 0, 0
lw $a0, $sp, $imm2, $zero, 0, 1
lw $ra, $sp, $imm2, $zero, 0, 2
lw $s0, $sp, $imm2, $zero, 0, 3

EXIT:
add $sp, $sp, $imm2, $zero, 0, 3		# pop 3 items from stack
beq $zero, $zero, $zero, $ra, 0, 0		# return

#####
#irq_handler:
    # Handle interrupts
    #in $t1, $zero, $imm2, $zero, 0, 3  # Read irq0status
    #in $t2, $zero, $imm2, $zero, 0, 4  # Read irq1status
    #in $t3, $zero, $imm2, $zero, 0, 5  # Read irq2status

    # Clear interrupt statuses
    #out $zero, $zero, $imm2, $zero, 0, 3
    #out $zero, $zero, $imm2, $zero, 0, 4
    #out $zero, $zero, $imm2, $zero, 0, 5

    #reti $zero, $zero, $zero, $zero, 0, 0
#####   