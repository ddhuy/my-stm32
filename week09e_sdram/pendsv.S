    .syntax unified
    .cpu cortex-m4
    .thumb

    .equ NTASKS, 3              /* keep in step with main.c */

/* --------------------------------------------------------------- */
    .global pendsv_handler
    .type   pendsv_handler, %function
    .thumb_func
pendsv_handler:
    /* freeze the ongoing task */
    mrs     r0, psp             /* side door: read PSP while sp = MSP     */
    stmdb   r0!, {r4-r11}       /* push software half onto ITS stack      */
                                /* (stmdb = store-multiple-decrement-     */
                                /*  before: exactly what 'push' means)    */
 
    /* ---- file the bookmark ---- */
    ldr     r1, =current
    ldr     r2, [r1]            /* r2 = current index                     */
    ldr     r3, =tcbs
    str     r0, [r3, r2, lsl #2]/* tcbs[current].sp = final sp.           */
                                /* sp is the tcb's first member: address  */
                                /* is tcbs + 4*index — why 'first' rule.  */
 
    /* ---- layer 8: policy — one line's worth ---- */
    adds    r2, r2, #1          /* current + 1                            */
    cmp     r2, #NTASKS
    blt     1f
    movs    r2, #0              /* ... % NTASKS                           */
1:  str     r2, [r1]
 
    /* ---- thaw the incoming task ---- */
    ldr     r0, [r3, r2, lsl #2]/* r0 = its saved sp (forged or genuine — */
                                /* we cannot tell, and that is the point) */
    ldmia   r0!, {r4-r11}       /* pop ITS software half                  */
    msr     psp, r0             /* re-aim PSP: the sleight of hand        */
 
    /* ---- let the hardware finish ---- */
    bx      lr                  /* lr holds EXC_RETURN (0xFFFFFFFD:       */
                                /* "thread mode, PSP"): hardware pops the */
                                /* 8-word frame — pc included — and the   */
                                /* incoming task runs. Resume and birth,  */
                                /* indistinguishable.                     */
 
/* --------------------------------------------------------------- */
    .global start_first_task
    .type   start_first_task, %function
    .thumb_func
start_first_task:
    /* r0 = top of task 0's stack, r1 = entry (Thumb bit set by C)   */
    msr     psp, r0             /* aim PSP at the empty private stack     */
    movs    r0, #2
    msr     control, r0         /* SPSEL=1: thread mode now uses PSP      */
    isb                         /* pipeline must notice before next fetch */
    bx      r1                  /* enter the task; never returns          */
 