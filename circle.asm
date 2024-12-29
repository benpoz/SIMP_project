# Draw a circle on the screen
# Radius: 0x100

# Load radius
lw $a0, $zero, 0x100  # Load radius

# Calculate center of the screen
li $t0, 128  # Center x
li $t1, 128  # Center y

# Initialize drawing variables
add $t2, $zero, $zero, $zero, $a0, 0  # x = radius
add $t3, $zero, $zero, $zero, 0, 0    # y = 0
add $t4, $zero, $zero, $zero, 1, 0    # decision parameter

draw_circle:
    # Draw points for each octant
    # Example for one octant:
    add $a1, $t0, $t2, $zero, 0, 0
    add $a2, $t1, $t3, $zero, 0, 0
    out $a1, $a2, 0, $zero, 0, 0  # Write pixel

    # Update decision parameter and coordinates
    sub $t3, $t3, $imm1, $zero, 1, 0
    add $t4, $t4, $imm1, $zero, 2, 0
    beq $t3, $zero, end_draw

    beq $zero, $zero, $zero, $imm1, draw_circle, 0

end_draw:
halt $zero, $zero, $zero, $zero, 0, 0