# Draw a circle on the screen
# Radius: 0x100
# Enable interrupts
out $zero, $zero, $imm2, $imm1, 1, 0  # Enable irq0
out $zero, $zero, $imm2, $imm1, 1, 1  # Enable irq1
out $zero, $zero, $imm2, $imm1, 1, 2  # Enable irq2

# Set interrupt handler
sll $sp, $imm1, $imm2, $zero, 1, 11  # Set $sp=1<<11=2048
out $zero, $imm1, $zero, $imm2, 6, irq_handler

# Load radius
lw $a0, $zero, 0x100  # Load radius

# Calculate center of the screen
add $t0, $zero, $imm1, $zero, 128, 0  # Center x
add $t1, $zero, $imm1, $zero, 128, 0  # Center y

# Initialize drawing variables
add $t2, $zero, $zero, $zero, $a0, 0  # x = radius
add $t3, $zero, $zero, $zero, 0, 0    # y = 0
add $t4, $zero, $zero, $zero, 1, 0    # decision parameter

draw_circle:
    # Draw points for each octant
    # Example for one octant:
    add $a1, $t0, $t2, $zero, 0, 0
    add $a2, $t1, $t3, $zero, 0, 0
    out $a1, $a2, 0, $zero, 0, 21  # Write pixel

    # Update decision parameter and coordinates
    sub $t3, $t3, $imm1, $zero, 1, 0
    add $t4, $t4, $imm1, $zero, 2, 0
    beq $t3, $zero, end_draw

    beq $zero, $zero, $zero, $imm1, draw_circle, 0

end_draw:
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