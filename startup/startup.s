.syntax unified
.thumb

.global Reset_Handler
.extern main

.thumb_func
Reset_Handler:
LDR r0, =_sidata //r0= source address (Flash)
LDR r1, =_sdata //r1=destination address (RAM)
LDR r2, =_edata //r2= end address (RAM)

copy_loop:
    CMP r1,r2
    BEQ copy_done

    LDR r3,[r0]
    STR r3,[r1]

    ADDS r0,r0,#4
    ADDS r1,r1,#4

    B copy_loop

copy_done:

LDR r1, =_sbss
LDR r2, =_ebss

MOVS r3,#0
zero_loop:
    CMP r1,r2   
    BEQ zero_done   
    STR r3,[r1]
    ADDS r1,r1,#4
    B zero_loop

zero_done:
    BL main

hang:
    B hang