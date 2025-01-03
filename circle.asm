# Draw a circle on the screen
# Radius: 0x100
######
# Enable interrupts

#out $zero, $zero, $imm2, $imm1, 1, 0  # Enable irq0
#out $zero, $zero, $imm2, $imm1, 1, 1  # Enable irq1
#out $zero, $zero, $imm2, $imm1, 1, 2  # Enable irq2

# Set interrupt handler
#sll $sp, $imm1, $imm2, $zero, 1, 11  # Set $sp=1<<11=2048
#out $zero, $imm1, $zero, $imm2, 6, irq_handler
######

# Load radius
lw $a0, $zero, 0x100,0,0  # Load radius

# Calculate center of the screen
add $a1, $zero, $imm1, $zero, 128, 0  # Center x
add $a2, $zero, $imm1, $zero, 128, 0  # Center y

# Initialize drawing variables
add $s0, $zero, $zero, $zero, $a0, 0  # x = radius
add $s1, $zero, $zero, $zero, 0, 0    # y = 0
add $s2, $zero, $zero, $zero, 1, 0    # decision parameter

# Draw circle
draw_circle:
    # Draw points for each octant
    # Calculate and draw a point in one octant, then mirror it to others
    add $t0, $a1, $s0, $zero, 0, 0  # Calculate x position
    add $t1, $a2, $s1, $zero, 0, 0  # Calculate y position
    out $t0, $t1, 0, $zero, 0, 22   # Write pixel to monitor

    sub $t0, $a1, $s0, $zero, 0, 0  # -x + center_x
    out $t0, $t1, 0, $zero, 0, 22   # (-x, y)

    add $t0, $a1, $s1, $zero, 0, 0  # y + center_x
    add $t1, $a2, $s0, $zero, 0, 0  # x + center_y
    out $t0, $t1, 0, $zero, 0, 22   # (y, x)

    sub $t0, $a1, $s1, $zero, 0, 0  # -y + center_x
    out $t0, $t1, 0, $zero, 0, 22   # (-y, x)

    sub $t1, $a2, $s1, $zero, 0, 0  # -y + center_y
    out $t0, $t1, 0, $zero, 0, 22   # (-y, -x)

    add $t0, $a1, $s0, $zero, 0, 0  # x + center_x
    sub $t1, $a2, $s1, $zero, 0, 0  # -y + center_y
    out $t0, $t1, 0, $zero, 0, 22   # (x, -y)

    sub $t0, $a1, $s0, $zero, 0, 0  # -x + center_x
    out $t0, $t1, 0, $zero, 0, 22   # (-x, -y)

# Update decision parameter and coordinates
    add $s1, $s1, $imm1, $zero, 1, 0  # Increment y
# Check if s2 < 0
    bge $s2,$s2, $zero, skip_update_x, 0,0  # If s2 >= 0, skip update_x


    sub $s0, $s0, $imm1, $zero, 1, 0  # Decrement x
    add $s2, $s2, $imm1, $zero, 2, 0  # Update decision parameter
skip_update_x:
    add $s2, $s2, $imm1, $zero, 2, 0  # Update decision parameter
    beq $s0,$s0, $s1, end_draw, 0,0       # If x == y, end drawing

    beq $zero, $zero, $zero, $imm1, draw_circle, 0  # Loop to draw_circle
end_draw:
halt $zero, $zero, $zero, $zero, 0, 0

#####
irq_handler:
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