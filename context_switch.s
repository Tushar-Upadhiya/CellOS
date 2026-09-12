.syntax unified
.thumb

.global PendSV_Handler
.global SVC_Handler
.extern current_task
.extern next_task

.thumb_func
PendSV_Handler:
    // ---- Save the outgoing task ----
    MRS r0, PSP                  // r0 = outgoing task's stack pointer
    STMDB r0!, {r4-r11}          // push R4-R11 onto its stack

    LDR r1, =current_task        // r1 = address of the current_task pointer variable
    LDR r2, [r1]                    // r2 = current_task itself (a TCB*)
    STR r0, [r2]                       // current_task->stack_pointer = r0

    // ---- Load the incoming task ----
    LDR r1, =next_task           // r1 = address of the next_task pointer variable
    LDR r2, [r1]                    // r2 = next_task itself (a TCB*)
    LDR r0, [r2]                       // r0 = next_task->stack_pointer

    LDMIA r0!, {r4-r11}          // pop R4-R11 from the incoming task's stack

    MSR PSP, r0                  // PSP now points at the remaining hardware frame

    LDR r1, =current_task        // current_task = next_task
    STR r2, [r1]

    LDR lr, =0xFFFFFFFD          // EXC_RETURN: return to Thread mode, using PSP
    BX lr

    .thumb_func
    SVC_Handler:
        LDR r0 ,=current_task
        LDR r1,[r0]
        LDR r0,[r1]
        LDMIA r0!,{r4-r11}

        MSR PSP, r0

        MOVS r0, #2
        MSR CONTROL,r0
        ISB

        LDR lr, =0xFFFFFFFD
        BX lr