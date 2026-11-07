# CellOS

A preemptive RTOS written from scratch for the **STM32F401 (ARM Cortex-M4)** in C and ARM Thumb assembly. No HAL, no CMSIS, no vendor startup files: the linker script, vector table, boot code, context switcher, scheduler and drivers are all written by hand.

> **Status: work in progress.** The core (boot, SysTick, tasks, context switching, Round Robin) is verified on real hardware. Synchronization, drivers and the memory allocator are at various stages, tagged honestly throughout this README.

**Status tags used in this document**

| Tag | Meaning |
|---|---|
| `[VERIFIED ON HARDWARE]` | Built, flashed and checked on the board, with evidence |
| `[WRITTEN, NOT YET VERIFIED]` | Code exists and was reasoned through, but not tested on the board yet |
| `[IN PROGRESS]` | Being worked on right now |
| `[NOT STARTED]` | Planned, no code yet |
| `[PLANNED]` | Depends on lab access or another milestone |

---

## Table of contents

1. [Why this project](#why-this-project)
2. [Status at a glance](#status-at-a-glance)
3. [Hardware and toolchain](#hardware-and-toolchain)
4. [Repository layout](#repository-layout)
5. [Build, flash and debug](#build-flash-and-debug)
6. [Stage 1: Linker script](#stage-1-linker-script)
7. [Stage 2: Vector table](#stage-2-vector-table)
8. [Stage 3: Reset_Handler](#stage-3-reset_handler)
9. [Stage 4: First bring-up](#stage-4-first-bring-up)
10. [Stage 5: SysTick](#stage-5-systick)
11. [Stage 6: Tasks and the TCB](#stage-6-tasks-and-the-tcb)
12. [Stage 7: Context switching (SVC and PendSV)](#stage-7-context-switching-svc-and-pendsv)
13. [Stage 8: Scheduler](#stage-8-scheduler)
14. [Stage 9: Mutex](#stage-9-mutex)
15. [Stage 10: Drivers](#stage-10-drivers)
16. [Stage 11: Memory allocator](#stage-11-memory-allocator)
17. [Bugs I hit and what they taught me](#bugs-i-hit-and-what-they-taught-me)
18. [Measurements](#measurements)
19. [Known limitations](#known-limitations)
20. [Roadmap](#roadmap)
21. [References](#references)

---

## Why this project

I wanted to understand what actually happens between power-on and a multitasking system running, instead of treating an RTOS as a black box. Every layer here is something I wrote, traced by hand, and debugged on hardware with GDB and the CPU's fault registers.

This is a learning and portfolio project. It is **not** a production RTOS and does not compete with FreeRTOS or Zephyr on features. The long-term goal is narrower: finish a small, correct kernel, then run a few measured comparisons (allocation time, context-switch time, footprint) on real hardware.

---

## Status at a glance

| Component | Status | Evidence |
|---|---|---|
| Linker script, vector table, `Reset_Handler` | `[VERIFIED ON HARDWARE]` | Board boots; vector table bytes inspected |
| SysTick (1 ms tick) | `[VERIFIED ON HARDWARE]` | `tick_count` read over GDB after ~5 s |
| TCB and `task_create()` | `[VERIFIED ON HARDWARE]` | Tasks start from fake exception frames |
| SVC bootstrap and PendSV context switch | `[VERIFIED ON HARDWARE]` | GDB shows two tasks alternating |
| Round Robin scheduler | `[VERIFIED ON HARDWARE]` | Six consecutive alternating stops in GDB |
| Task state tracking (READY / RUNNING / BLOCKED) | `[WRITTEN, NOT YET VERIFIED]` | Fix for a known PendSV bug is `[IN PROGRESS]` |
| Blocking mutex (single waiter) | `[WRITTEN, NOT YET VERIFIED]` | Tests planned (see Stage 9) |
| Multi-waiter mutex | `[NOT STARTED]` | Design done |
| UART driver | `[FILL IN: DONE / VERIFIED]` | `[FILL IN: baud, pins]` |
| I2C driver | `[IN PROGRESS]` | Known bugs, MPU6050 target |
| SPI driver | `[NOT STARTED]` | |
| Allocator (hosted prototype, first-fit free list) | `[IN PROGRESS]` | Coalescing still to finish |
| Allocator inside CellOS (bare-metal heap) | `[IN PROGRESS]` | Linker heap region being added |
| `calloc` | `[NOT STARTED]` | |
| TLSF allocator and benchmark | `[NOT STARTED]` | |
| Lab measurements (scope / logic analyzer) | `[PLANNED]` | |

---

## Hardware and toolchain

**Hardware**

- WeAct Black Pill, STM32F401CEU6 (512 KB flash, 96 KB SRAM), clocked from the internal 16 MHz HSI
- ST-Link V2 programmer / debugger over SWD
- External LEDs on PA0 and PA1 for task demos
- `[FILL IN: USB-to-serial adapter model]` for UART
- MPU6050 breakout for I2C bring-up

> **[SCREENSHOT 1: `docs/images/01-hardware-setup.jpg`]**
> A photo of the full bench: Black Pill, ST-Link, the two LEDs on PA0/PA1, the USB-serial adapter and the MPU6050 if wired. Label the connections if you can (a caption is fine).

**Toolchain (as used)**

| Tool | Version |
|---|---|
| `arm-none-eabi-gcc` | 16.1.0 |
| `arm-none-eabi-gdb` | 17.2 |
| `st-flash` / `st-util` (stlink) | 1.8.0 |
| Host OS | Arch Linux |

Compile flags: `-mcpu=cortex-m4 -mthumb -mfloat-abi=soft -g -nostdlib -T my_linker.ld`

---

## Repository layout

`[FILL IN: adjust names and folders to match your actual repo]`

```
CellOS/
├── README.md
├── makefile
├── my_linker.ld        # memory map and section placement
├── startup.s           # Reset_Handler
├── cos_vector.c        # vector table and default handlers
├── context_switch.s    # SVC_Handler and PendSV_Handler
├── systick.c / .h      # 1 ms tick, scheduler hook
├── task.c / .h         # TCB, task_create, os_start, schedule
├── mutex.c / .h        # blocking mutex
├── uart.c / .h         # UART driver
├── i2c.c / .h          # I2C driver (WIP)
├── main.c              # demo tasks
└── docs/
    └── images/         # screenshots used in this README
```

---

## Build, flash and debug

```bash
make              # builds cellos.elf and cellos.bin, prints section sizes
make flash        # st-flash write cellos.bin 0x08000000

st-util &         # GDB server on port 4242
arm-none-eabi-gdb cellos.elf
(gdb) target extended-remote localhost:4242
(gdb) monitor reset halt
(gdb) continue
```

Notes learned the hard way:

- Kill any leftover GDB server before flashing, otherwise `st-flash` cannot claim the USB device: `pkill st-util`.
- After editing a Makefile source list, check the link line: a missing object file shows up as `undefined reference` errors.
- `arm-none-eabi-nm cellos.elf` is the quickest way to confirm which symbols were actually linked.

> **[SCREENSHOT 2: `docs/images/02-build-output.png`]**
> Terminal after a clean `make clean && make`, showing every file compiled, the link line, and the `arm-none-eabi-size` output (text / data / bss). Capture the whole output without errors.

---

## Stage 1: Linker script

`[VERIFIED ON HARDWARE]` (heap section: `[IN PROGRESS]`, see Stage 11)

The linker script (`my_linker.ld`) tells the linker where everything goes on the chip. Without an OS loader, the layout is my responsibility, and the CPU expects specific things at specific addresses.

```
FLASH  0x08000000  512 KB   (vector table first, then code and constants)
RAM    0x20000000   96 KB   (.data, .bss, heap, then the stack growing down from the top)
```

What it defines:

- `.isr_vector` first in flash (wrapped in `KEEP` so the linker cannot drop it)
- `.text` and `.rodata` in flash
- `.data` with `> RAM AT> FLASH`: runs in RAM, but its initial values are stored in flash and copied at boot
- `.bss` in RAM, zeroed at boot
- Symbols the startup code uses: `_sidata`, `_sdata`, `_edata`, `_sbss`, `_ebss`, `_estack`
- `.heap` region with `_sheap` / `_eheap` and an `ASSERT` that fails the build if the heap and the main stack collide `[IN PROGRESS]`

```
RAM (0x20000000 - 0x20018000)

 high  +--------------------+  _estack = 0x20018000
       |  main stack (MSP)  |  grows downward
       |         ...        |
       +--------------------+  _eheap
       |        heap        |
       +--------------------+  _sheap
       |        .bss        |
       +--------------------+  _sbss / _ebss
       |        .data       |
 low   +--------------------+  _sdata
```

> **[SCREENSHOT 3: `docs/images/03-linker-symbols.png`]**
> Output of `arm-none-eabi-nm cellos.elf | grep -E "_sdata|_edata|_sbss|_ebss|_sheap|_eheap|_estack"`. Take this after the heap section is in the linker script, so `_sheap` and `_eheap` appear (they should differ by the heap size, and `_eheap` should sit well below `0x20018000`).

---

## Stage 2: Vector table

`[VERIFIED ON HARDWARE]`

`cos_vector.c` defines the exception vector table (entries 0 to 15) and places it in `.isr_vector` at `0x08000000`.

- Entry 0 is the initial stack pointer (`_estack`), entry 1 is `Reset_Handler`
- Reserved slots (7 to 10 and 13) hold zero, so every later handler keeps its fixed index
- Every handler is a weak alias of `Default_Handler` (an infinite loop), so any unhandled exception traps at a known address instead of jumping into garbage
- A strong definition elsewhere (for example `SysTick_Handler` in `systick.c`) automatically overrides the weak alias at link time

Verified against the real binary:

```
$ arm-none-eabi-objdump -s -j .isr_vector cellos.elf
 8000000 00800120 b5000008 41000008 41000008  ... ....A...A...
 8000010 41000008 41000008 41000008 00000000  A...A...A.......
```

Read little-endian: word 0 is `0x20018000` (top of RAM, the initial SP), word 1 is `0x080000b5` (`Reset_Handler` with the Thumb bit set), then `0x08000041` repeated (`Default_Handler`) and zeros for the reserved slots.

> **[SCREENSHOT 4: `docs/images/04-vector-table-dump.png`]**
> The `objdump -s -j .isr_vector` output above, plus `arm-none-eabi-readelf -h cellos.elf | grep Entry` and `arm-none-eabi-nm cellos.elf | grep Reset_Handler` in the same terminal. The entry point should be odd (Thumb bit set) and match the vector table's second word.

---

## Stage 3: Reset_Handler

`[VERIFIED ON HARDWARE]`

By the time `Reset_Handler` runs, the hardware has already loaded SP and PC from the vector table. RAM is still in an unknown state, so `startup.s` does the C runtime setup in ARM assembly:

1. Copy the initial values of `.data` from flash (`_sidata`) to RAM (`_sdata` to `_edata`), word by word
2. Zero `.bss` (`_sbss` to `_ebss`), word by word
3. Call `main`
4. If `main` ever returns, trap in an infinite loop

Both loops check the exit condition **before** doing the work, so an empty section runs zero iterations.

Details that mattered:

- `.syntax unified` and `.thumb` at the top, otherwise three-operand instructions such as `ADDS r0, r0, #4` are rejected
- `.global Reset_Handler` and `.extern main` so the linker can connect the files
- `.thumb_func` before the label so the symbol's address gets its Thumb bit; Cortex-M only runs Thumb code and a missing bit means a fault on the very first jump

---

## Stage 4: First bring-up

`[VERIFIED ON HARDWARE]`

The first milestone was a blinking LED from a minimal `main.c` using raw register writes (RCC clock enable, GPIO MODER and ODR). That single test exercises the linker script, both startup loops and the toolchain together.

Debugging path that worked:

1. Inspected the vector table bytes and the ELF entry point (Stage 2)
2. Attached GDB with `st-util`, reset-halted, and confirmed the PC was inside `delay()` inside `main()` (so boot was fine)
3. Read `GPIOC_MODER` and `GPIOC_ODR` directly to confirm the pin was configured
4. Switched to an external LED on PA0, which blinked

The onboard PC13 LED stayed dark even though the registers were correct. I suspect hardware damage to that LED from soldering, so all demos use external LEDs.

> **[SCREENSHOT 5: `docs/images/05-first-blink.gif` (or a photo)]**
> A short GIF or photo of the external LED blinking on PA0, ideally next to the board.
>
> **[SCREENSHOT 6: `docs/images/06-gdb-first-boot.png`]**
> GDB after `monitor reset halt` and `continue` / Ctrl+C, showing the PC inside `delay()` or `main()`, plus `x/1xw 0x40020800` showing the MODER value.

---

## Stage 5: SysTick

`[VERIFIED ON HARDWARE]`

`systick.c` configures the Cortex-M SysTick timer straight from its registers (`SYST_CSR`, `SYST_RVR`, `SYST_CVR`).

```
reload = (16,000,000 Hz / 1000 ticks per second) - 1 = 15999
CSR    = ENABLE | TICKINT | CLKSOURCE (core clock)
```

`SysTick_Handler` increments `tick_count` (and, later, runs the scheduler).

**Verification:** after about five seconds of free running, a GDB halt and `print tick_count` read **5108**, consistent with a 1 ms tick given manual Ctrl+C timing.

> **[SCREENSHOT 7: `docs/images/07-tick-count.png`]**
> GDB session showing `continue`, a pause of roughly five seconds, Ctrl+C, and `print tick_count`. If you repeat it, show a second read a known time later to prove the rate.

---

## Stage 6: Tasks and the TCB

`[VERIFIED ON HARDWARE]`

```c
typedef struct {
    uint32_t     *stack_pointer;   // saved SP while the task is not running (offset 0)
    task_state_t  state;           // READY / RUNNING / BLOCKED (offset 4)
    uint32_t      task_id;         // offset 8
} TCB;
```

Memory is statically allocated: `TCB tasks[MAX_TASKS]` and `uint32_t task_stacks[MAX_TASKS][STACK_SIZE]` (256 words per task). Static allocation keeps task creation deterministic and avoids heap fragmentation in the kernel itself.

`task_create(fn)` builds a **fake saved context** on the new task's stack, so the first switch into the task uses exactly the same restore path as every later one. Layout, low to high addresses:

```
 saved SP -> R4 R5 R6 R7 R8 R9 R10 R11   (software-saved by PendSV)
             R0 R1 R2 R3 R12 LR PC xPSR  (hardware-stacked on exception entry)
                              |   |
                              |   +-- 0x01000000 (Thumb bit in xPSR)
                              +------ task function address
             LR = task_return_handler  (traps if a task function ever returns)
```

Getting the order wrong sends execution to the wrong place without any warning, because the restore code trusts positions, not contents.

---

## Stage 7: Context switching (SVC and PendSV)

`[VERIFIED ON HARDWARE]` (state-tracking additions: `[WRITTEN, NOT YET VERIFIED]`)

Both handlers live in `context_switch.s`.

**`SVC_Handler`** bootstraps the first task. It loads `current_task->stack_pointer`, pops R4 to R11, sets PSP, writes CONTROL (`SPSEL = 1`, so Thread mode uses PSP), executes `ISB`, and returns with `EXC_RETURN = 0xFFFFFFFD`. The hardware then pops the remaining frame and lands at the task's first instruction.

**`PendSV_Handler`** does every later switch:

1. Read PSP, push R4 to R11 (`STMDB`), store the new PSP into the outgoing TCB
2. Load the incoming TCB's saved stack pointer, pop R4 to R11 (`LDMIA`)
3. Write PSP and return with `EXC_RETURN = 0xFFFFFFFD`

PendSV is triggered by setting `PENDSVSET` (bit 28) in `ICSR`, by SysTick for time slicing and by a blocking call for yielding. Keeping the switch in PendSV (lowest priority) means it never interrupts a more urgent interrupt handler.

Linking check: both handlers must be strong symbols, not the weak aliases.

```
$ arm-none-eabi-nm cellos.elf | grep -E "PendSV_Handler|SVC_Handler"
0800033c T PendSV_Handler
08000362 T SVC_Handler
```

**Hardware verification:** two test tasks toggle PA0 and PA1. With breakpoints on both task functions, six consecutive `continue` commands alternated `task1_func`, `task2_func`, `task1_func`, and so on, and `current_task->task_id` matched the running task.

> **[SCREENSHOT 8: `docs/images/08-nm-handlers.png`]**
> Terminal with the `nm | grep` output above. If you still have the earlier output where both handlers showed `W` at `08000040` (before `context_switch.s` was added to the Makefile), include that as a "before" shot, it makes a good story.
>
> **[SCREENSHOT 9: `docs/images/09-task-alternation.png`]**
> The GDB session with breakpoints on `task1_func` and `task2_func` and several `continue` commands showing the stops alternating, plus `print current_task->task_id` while stopped in one of them.
>
> **[SCREENSHOT 10: `docs/images/10-two-leds.gif`]**
> A GIF of both external LEDs (PA0 and PA1) blinking.

---

## Stage 8: Scheduler

`[VERIFIED ON HARDWARE]` for Round Robin; `[WRITTEN, NOT YET VERIFIED]` for blocked-task skipping

Round Robin is driven from `SysTick_Handler`: pick the next task, and if it differs from the current one, request PendSV. The scheduler is deliberately only *policy* (who runs next). PendSV is only *mechanism* (how to switch).

The search skips tasks that are not `TASK_READY`, but must still be allowed to pick the current task when everyone else is blocked:

```c
candidate = (candidate + 1) % task_count;
if (tasks[candidate].state == TASK_READY || candidate == current_task->task_id)
    next_task = &tasks[candidate];
```

Task states are updated in the switch code (`RUNNING` for the incoming task, `READY` for the outgoing one).

**Known issues being fixed** `[IN PROGRESS]`:

- PendSV currently sets the outgoing task to `READY` unconditionally, which overwrites `BLOCKED`. The fix: only demote it if it is still `RUNNING`.
- `mutex_lock` yields without choosing a new `next_task`. The fix: move the search into one `schedule()` function and call it from both `SysTick_Handler` and `mutex_lock`.

> **[SCREENSHOT 11: `docs/images/11-task-states.png`]**
> GDB stopped inside `task1_func`, then `print tasks[0].state` and `print tasks[1].state` (expect `1` for RUNNING and `0` for READY), then the same while stopped inside `task2_func` with the values swapped. Take this **after** the two fixes above are in.

---

## Stage 9: Mutex

`[WRITTEN, NOT YET VERIFIED]`

A first version spun on a flag inside a critical section. The current version blocks instead:

- `CPSID i` / `CPSIE i` make the check-and-set of `locked` atomic with respect to the scheduler
- If the lock is held, the task marks itself `TASK_BLOCKED`, records itself as the waiter, and yields via PendSV (no busy-waiting)
- `mutex_unlock` clears the lock and wakes the waiting task (`TASK_READY`)

Everything risky (the shared-state writes) happens inside the interrupt-disabled window. Only a harmless loop-back remains after PendSV is requested.

**Planned verification** `[PLANNED]`:

1. **Race without a mutex:** two tasks each increment `shared_counter` 1000 times (using a read, short delay, write sequence to widen the race window). Expect a final value below 2000.
2. **Same test with the mutex:** expect exactly 2000.
3. **Blocking test:** task 1 holds the lock through a long delay while task 2 tries to lock. In GDB, expect `tasks[1].state == 2` (BLOCKED), `counter_lock.locked == 1`, and `counter_lock.waiting_task == &tasks[1]`. After unlock, expect `tasks[1].state == 0` (READY).

> **[SCREENSHOT 12: `docs/images/12-race-without-mutex.png`]**
> GDB `print shared_counter` after both tasks finish their loops **without** the mutex (value less than 2000).
>
> **[SCREENSHOT 13: `docs/images/13-race-with-mutex.png`]**
> Same test **with** the mutex (value exactly 2000).
>
> **[SCREENSHOT 14: `docs/images/14-blocking-test.png`]**
> GDB output for the blocking test above, showing the three `print` results.

**Known limitation:** the mutex stores one waiter. A third task blocking on the same mutex would overwrite the first waiter's record. A queue of waiters (FIFO, sized to `MAX_TASKS`) is designed but `[NOT STARTED]`.

---

## Stage 10: Drivers

All drivers are register-level, written from the reference manual. No HAL.

### UART

`[FILL IN: DONE / VERIFIED]`

`[FILL IN: USART instance, TX/RX pins, baud rate, BRR value and how it was computed (APB clock / baud)]`

> **[SCREENSHOT 15: `docs/images/15-uart-terminal.png`]**
> A serial terminal (`picocom` or `minicom`) showing text printed by the board. Include the command line you used to open the terminal so the baud rate is visible.

### I2C

`[IN PROGRESS]`: basic transfers work, known bugs remain.

Target device: MPU6050 at address `0x68` (WHO_AM_I register `0x75`; the chip starts asleep, so `PWR_MGMT_1` at `0x6B` must be cleared before reading data). Goal: burst-read 14 bytes from `0x3B` (accelerometer, temperature, gyroscope) and print scaled values over UART.

`[FILL IN: current symptom and what you suspect]`

> **[SCREENSHOT 16: `docs/images/16-i2c-whoami.png`]**
> The UART terminal showing the WHO_AM_I value read from the MPU6050. Take this once the bug is fixed.
>
> **[SCREENSHOT 17: `docs/images/17-imu-readings.png`]**
> UART output of the accelerometer and gyro values at rest (Z axis near 1 g, gyro near zero).

### SPI

`[NOT STARTED]` (a loopback test, MOSI to MISO, is the first step)

---

## Stage 11: Memory allocator

`[IN PROGRESS]`

### Hosted prototype

A first-fit free-list allocator, built and tested on the PC with `mmap` as the memory source:

- Each block has a header (size, free flag, next pointer)
- Requests are rounded up to a multiple of 8 bytes
- A free block that is large enough gets split into an allocated block and a free remainder
- `free` marks the block free

Bugs found while building it:

- **Pointer arithmetic precedence:** `(BlockHeader*)(char*)curr + n` scales `n` by `sizeof(BlockHeader)`. The offset must be added while the pointer is still a `char*`: `(BlockHeader*)((char*)curr + n)`.
- **No coalescing yet** `[IN PROGRESS]`: freeing adjacent blocks does not merge them, which fragments the heap.
- **Alignment:** on a 32-bit ARM target the 12-byte header returns pointers that are only 4-byte aligned. The plan is a 16-byte header so user data stays 8-byte aligned.

### Bare-metal version inside CellOS

`[IN PROGRESS]`

- The memory source changes from `mmap` to a linker-defined heap region (`_sheap` to `_eheap`) passed to `heap_init()`
- Allocation and free run inside a critical section (`CPSID` / `CPSIE`) so a task switch cannot corrupt the list, and must never be called from an interrupt handler
- `heap_dump()` over UART to show the block list live
- `my_calloc` with an overflow check on `num * size` `[NOT STARTED]`

> **[SCREENSHOT 18: `docs/images/18-heap-dump.png`]**
> UART output of `heap_dump()` after a sequence of allocs and frees: once showing fragmented free blocks, and once after coalescing merged them.

### TLSF (Two-Level Segregated Fit)

`[NOT STARTED]`

A first-fit allocator walks a list, so its allocation time grows with the number of free blocks and it fragments over time. TLSF keeps free blocks in size-class lists found through two bitmap levels, giving constant-time allocation and free. Plan: implement it after the first-fit version is verified, then measure both under the same workload.

---

## Bugs I hit and what they taught me

| Symptom | Cause | How I found it | Fix and lesson |
|---|---|---|---|
| `instruction not supported in Thumb16 mode` on `ADDS` | Assembler defaulting to the old divided syntax | The error message named the instruction | `.syntax unified` and `.thumb` at the top of every `.s` file |
| Linker `syntax error` at the last line | Missing `;` after the `_estack` assignment | Error line pointed one token after the real mistake | Linker script statements end with `;`; parsers often fail one token late |
| `undefined reference` to `Reset_Handler` / `main` | Loop code was not wrapped in a labelled, `.global` function | Read the symbol list | Add label, `.global`, `.extern` |
| Risk of a UsageFault on the first jump | Handler symbols need `.thumb_func` so the Thumb bit is set in their addresses (found on `Reset_Handler`, then caught again on `SVC_Handler`) | Inspected the ELF entry point and vector table bytes | `.thumb_func` on every handler written in assembly |
| GDB stopped in `UsageFault_Handler` | A breakpoint I placed in the middle of a 4-byte instruction caused an instruction-fetch bus error | CFSR = `0x00000100` (IBUSERR) | Read the fault register: GDB shows only one name for handlers that share a weak alias |
| `PendSV_Handler` and `SVC_Handler` showed as `W` at the `Default_Handler` address | `context_switch.s` was missing from the Makefile's source list | `nm` | Always verify with `nm`; a clean build does not mean the right symbols were linked |
| `undefined reference` to `tasks`, `current_task`, etc. | `task.c` was missing from the link line | Link command had no `task.o` | Update the source list |
| Compile errors in `task.h` | Enum used before it was defined, and a variable defined (not `extern`) in a header | Read top to bottom: does everything exist above it? | Declare types first; `extern` for variables in headers |
| `implicit declaration of function '__asm_volatile'` | Typo for `__asm volatile` | Compiler error | Character-level reading of the error |
| Scheduler would not pick the running task | Loop only accepted `TASK_READY`, but the running task is `TASK_RUNNING` | Traced the loop by hand with real state values | Also accept `candidate == current_task->task_id` |
| `mutex_lock` could leave interrupts disabled forever | `CPSID` used where `CPSIE` was needed on the success path | Traced the instruction sequence | Every exit path must restore interrupt state; a frozen scheduler looks like a hang |
| Blocked tasks never stay blocked | PendSV overwrites `BLOCKED` with `READY` `[IN PROGRESS]` | Code review of the state writes | Only demote the outgoing task if it is still `RUNNING` |
| Allocator pointed outside the pool | Cast precedence in pointer arithmetic | Traced the expression piece by piece | Do byte arithmetic on `char*`, cast afterward |

---

## Measurements

`[PLANNED]`: to be taken with an oscilloscope or logic analyzer during a lab session.

Checklist:

- SysTick period (expect 1 ms)
- Context-switch duration: toggle a GPIO at the start and end of `PendSV_Handler` and measure the pulse width
- Allocator timing: toggle a pin around `malloc` and `free` for first-fit versus TLSF under the same allocation pattern, cross-checked with the DWT cycle counter (`CYCCNT`)
- UART and I2C waveforms (bit timing, ACK)

> **[SCREENSHOT 19: `docs/images/19-scope-systick.png`]**: scope capture of a pin toggled in `SysTick_Handler`, with the period measurement visible.
>
> **[SCREENSHOT 20: `docs/images/20-scope-context-switch.png`]**: pulse width of the pin toggled across `PendSV_Handler`, with the measurement shown.
>
> **[SCREENSHOT 21: `docs/images/21-scope-malloc-compare.png`]**: first-fit versus TLSF allocation pulse widths, one capture each, or a combined screenshot.
>
> **[SCREENSHOT 22: `docs/images/22-benchmark-table.png`]**: a table or plot of cycle counts for both allocators under identical workloads (take this from your own benchmark output or a small plot).

Results table (fill in after measuring):

| Measurement | Value | Method |
|---|---|---|
| SysTick period | `[FILL IN]` | Scope |
| Context switch time | `[FILL IN]` | Scope, GPIO pulse |
| Allocation time, first-fit | `[FILL IN]` | CYCCNT / scope |
| Allocation time, TLSF | `[FILL IN]` | CYCCNT / scope |
| Flash / RAM footprint | `[FILL IN]` | `arm-none-eabi-size` |

---

## Known limitations

- Round Robin only: no priorities, so no priority inversion handling yet
- Mutex supports a single waiter (fix designed, `[NOT STARTED]`)
- No semaphores, message queues, or software timers
- No MPU-based stack overflow protection; tasks have fixed 1 KB stacks
- Allocator is not yet interrupt-safe in the final form (critical section is a stopgap)
- Magic numbers for task states in the assembly (`0`, `1`) must stay in sync with the C enum
- Tested only on one board; the onboard LED was not usable, so demos use external LEDs
- Timing is tuned for the 16 MHz HSI clock; no PLL configuration yet

---

## Roadmap

**CellOS v1 (in order)**

1. Fix the two scheduler/mutex bugs and run the verification tests `[IN PROGRESS]`
2. Multi-waiter mutex `[NOT STARTED]`
3. I2C bug fixes, MPU6050 readings over UART `[IN PROGRESS]`
4. First-fit allocator inside CellOS, with coalescing, `heap_dump()` and `calloc` `[IN PROGRESS]`
5. TLSF allocator and benchmark `[NOT STARTED]`
6. Lab measurements, then update the results table above `[PLANNED]`

**After v1 (not committed)**

- SPI, ADC (timer-triggered, then DMA) and semaphores
- Message queues
- Priority scheduling with priority inheritance
- A measured comparison against FreeRTOS on the same board (footprint or context-switch time)
- A bootloader as a separate small project

---

## References

- STMicroelectronics, *RM0368: STM32F401 reference manual* (peripheral registers, vector table)
- STMicroelectronics, *PM0214: STM32 Cortex-M4 programming manual* (core registers, exceptions, SysTick)
- Arm, *ARMv7-M Architecture Reference Manual* (exception model, EXC_RETURN, stacking)
- Arpaci-Dusseau and Arpaci-Dusseau, *Operating Systems: Three Easy Pieces* (scheduling and concurrency concepts)
- Masmano et al., *TLSF: a new dynamic memory allocator for real-time systems* (2004)

---

## Author

Tushar Upadhiya, B.Tech ECE, Thapar Institute of Engineering and Technology.
`[FILL IN: LinkedIn / email]`

---

## Screenshot checklist (delete this section before publishing)

| # | File | What to capture | Needs |
|---|---|---|---|
| 1 | `01-hardware-setup.jpg` | Photo of the bench and connections | Camera |
| 2 | `02-build-output.png` | `make clean && make` with the size line | Now |
| 3 | `03-linker-symbols.png` | `nm` of the linker symbols | After heap section is in |
| 4 | `04-vector-table-dump.png` | `objdump -s -j .isr_vector`, `readelf` entry, `nm Reset_Handler` | Now |
| 5 | `05-first-blink.gif` | LED blinking | Camera |
| 6 | `06-gdb-first-boot.png` | GDB with PC in `main`/`delay` and MODER read | Now |
| 7 | `07-tick-count.png` | `print tick_count` after about 5 s | Now |
| 8 | `08-nm-handlers.png` | `nm` showing `T` for PendSV and SVC | Now |
| 9 | `09-task-alternation.png` | Breakpoints alternating in GDB | Now |
| 10 | `10-two-leds.gif` | Both LEDs blinking | Camera |
| 11 | `11-task-states.png` | `print tasks[n].state` in each task | After bug fixes |
| 12 | `12-race-without-mutex.png` | `shared_counter` below 2000 | After tests |
| 13 | `13-race-with-mutex.png` | `shared_counter` equals 2000 | After tests |
| 14 | `14-blocking-test.png` | Blocked state, lock, waiter in GDB | After tests |
| 15 | `15-uart-terminal.png` | Serial terminal output | Now |
| 16 | `16-i2c-whoami.png` | WHO_AM_I over UART | After I2C fix |
| 17 | `17-imu-readings.png` | Accelerometer and gyro values | After I2C fix |
| 18 | `18-heap-dump.png` | `heap_dump()` before and after coalescing | After allocator port |
| 19 to 22 | scope and benchmark images | Lab measurements and benchmark table | Lab day |
