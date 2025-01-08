# set radius
.word 0x100 100
# Load radius
lw $a0, $zero, $imm1, $zero, 0x100, 0  # Load radius
mac $s0, $a0, $a0, $zero, 0, 0 # calculate r^2
add $s1, $zero, $imm1, $zero, 128, 0  # Center line and offset

add $t0, $zero, $zero, $zero, 0, 0 # set i = 0

# loop through all pixels
loop_i:
    beq $zero, $t0, $imm2, $imm1, fin, 256
    add $t1, $zero, $zero, $zero, 0, 0          # set j = 0
    loop_j:
        beq $zero, $t1, $imm1, $imm2, 256, incr_i # 6
        sub $a0, $t0, $s1, $zero, 0, 0          # calculate dist_x 
        mac $a0, $a0, $a0, $zero, 0, 0          # calculate dist_x^2
        sub $a1, $t1, $s1, $zero, 0, 0          # calculate dist_x 
        mac $a1, $a1, $a1, $zero, 0, 0          # calculate dist_x^2
        sub $v0, $s0, $a0, $a1, 0, 0            # calculate r^2 - x^2 - y^2
        mac $v0, $v0, $v0, $zero, 0, 0          # calculate (r^2 - x^2 - y^2)^2
        blt $zero, $v0, $imm1, $imm2, 5, draw   # if (r^2 - x^2 - y^2)^2 < margin (5?) - draw white pixel
    incr_j:   
        add $t1, $t1, $imm1, $zero, 1, 0          # j++ F
        beq $zero, $zero, $zero, $imm1, loop_j, 0   # loop back
incr_i:
    add $t0, $t0, $imm1, $zero, 1, 0          # i++
    beq $zero, $zero, $zero, $imm1, loop_i, 0   # loop back

draw:
    sll $t0, $t0, $imm1, $zero, 8, 0            # set line value
    add $t2, $t0, $t1, $zero, 0, 0              # calc pixel buffer 
    srl $t0, $t0, $imm1, $zero, 8, 0            # return i to normal
    out $zero, $imm1, $zero, $t2, 20, 0         # set monitor address
    out $zero, $imm1, $zero, $imm2, 21, 255     # set white color
    out $zero, $imm1, $zero, $imm2, 22, 1       # draw pixel
    beq $zero, $zero, $zero, $imm1, incr_j, 0   # back to loop
fin:    
    halt $zero, $zero, $zero, $zero, 0, 0       # halt