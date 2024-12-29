# Move contents of first 8 sectors one sector forward

# Initialize disk command registers
li $t0, 0  # Sector 0
li $t1, 1  # Sector 1
li $t2, 8  # Sector 8

move_sectors:
    beq $t0, $t2, end_disk

    # Read sector t0
    add $a0, $t0, $zero, $zero, 0, 0
    out $a0, $zero, 0, $zero, 1, 0  # Disk read command

    # Wait for disk to be ready
    in $a1, $zero, 0, $zero, 0, 0
    beq $a1, $zero, move_sectors

    # Write to sector t1
    add $a0, $t1, $zero, $zero, 0, 0
    out $a0, $zero, 0, $zero, 2, 0  # Disk write command

    # Increment sectors
    add $t0, $t0, $imm1, $zero, 1, 0
    add $t1, $t1, $imm1, $zero, 1, 0
    beq $zero, $zero, $zero, $imm1, move_sectors, 0

end_disk:
halt $zero, $zero, $zero, $zero, 0, 0