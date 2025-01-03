# Move contents of first 8 sectors one sector forward

# Enable interrupts
out $zero, $zero, $imm2, $imm1, 1, 0  # Enable irq0
out $zero, $zero, $imm2, $imm1, 1, 1  # Enable irq1
out $zero, $zero, $imm2, $imm1, 1, 2  # Enable irq2

# Set interrupt handler
sll $sp, $imm1, $imm2, $zero, 1, 11  # Set $sp=1<<11=2048
out $zero, $imm1, $zero, $imm2, 6, irq_handler

# Initialize disk command registers
li $t0, 0  # Sector 0
li $t1, 1  # Sector 1
li $t2, 8  # Sector 8

move_sectors:
    beq $t0, $t2, end_disk

    # Read sector t0
    add $a0, $t0, $zero, $zero, 0, 0
    out $a0, $zero, 0, $zero, 1, 14  # Disk read command

    # Wait for disk to be ready
    in $a1, $zero, 0, $zero, 0, 17
    beq $a1, $zero, move_sectors

    # Write to sector t1
    add $a0, $t1, $zero, $zero, 0, 0
    out $a0, $zero, 0, $zero, 2, 14  # Disk write command

    # Increment sectors
    add $t0, $t0, $imm1, $zero, 1, 0
    add $t1, $t1, $imm1, $zero, 1, 0
    beq $zero, $zero, $zero, $imm1, move_sectors, 0

end_disk:
halt $zero, $zero, $zero, $zero, 0, 0

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