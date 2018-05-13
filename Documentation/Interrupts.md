# Interrupts And Fuzix (Preliminary: Not Yet Fully Implemented)

## Introduction

Fuzix has an internal model of interrupt handling that is intentionally very
simplified. This document explains how that maps onto more complex interrupt
structures found in some hardware, and also how to use this model to run
Fuzix on top of your own thin hard real time layer if your platform requires
this for things like serial or audio drivers.

## How Fuzix Sees The World

As far as Fuzix is concerned there is a single interrupt entry point into C
code, and platform low level code to switch to the Kernel interrupt stack
and execute both system and platform interrupt code including timer events.
The code decides what needs to be done by inspecting interrupt controller
state or values set by the platform code, and invokes any needed functions.
It returns to the assembly code in lowlevel-[cpu].s which may then also
perform signal delivery or task switches, returning directly to the relevant
task.

This interrupt handler is prevented from executing by the di() function, and
re-enabled by the corresponding irqrestore or ei() functions.

A separate function '__hard_di()' works the same way as di() but must
actually block interrupts at the hardware level. It is called from the core 
when the platform is taking actions that might leave the interrupt vectors invalid or
the interrupt stack unmapped. In some cases it may also be used by drivers
with hard timing constraints - floppy disc drives being the usual culprit.

Currently some platform level assembly code is not correctly using the
separate soft/hard interrupt functions so the ability to use soft interrupt
models is not yet complete.

When you have multiple interrupt vectors but do not need the priority levels
or re-entrancy of interrupt service a simpler approach is just to store the
vector invoked somewhere and then call the Fuzix IRQ handler, with your
platform IRQ code checking to see what work needs doing.

## Serial Drivers

Fuzix specifically guarantees that you may call the tty character reception
function (tty_inproc from interrupt handlers other than the standard provided
one. If you do so you must ensure
- That you are on a valid private stack for your interrupt
- You do not call the function from two places at once for the same tty (eg interrupt and polled)
- You must deal with the fact it may in turn attempt to output data via tty_putc
- If you are using soft interrupts you honour di().

A second method which is often more efficient is to simply add bytes to a
queue in the interrupt handler, and optionally also maintain a small output
queue driven by interrupts as well. Then in the timer tick callback into
Fuzix you process the queue. This is particularly useful if you are trying
to run a soft interrupt model.

## The Soft Interrupt Model

The default model is that di() and irqrestore map to the __hard_di() and
__hard_irqrestore functions. If CONFIG_SOFT_IRQ is set then they will instead
be separate functions.

In this model instead of di() blocking interrupts di can manipulate a flag.
All interrupt handling, dealing with multiple interrupt levels and the like
are handled below Fuzix.

The rules for Fuzix then become
- You must block real interrupts in __hard_di()
- You must not call into the Fuzix kernel when the di flag is set
- You must not re-enter the Fuzix kernel
- You must allow for the fact that the invocation of the kernel may switch
or modify stacks before returning

In practice the final restriction means you either need to wire the Fuzix
interrupt to something like a low priority timer event, and field the other
interrupts without directly interacting with the Fuzix core, or you need to
run Fuzix as a task within an RTOS environment.

Using soft interrupts instead of hard ones does however allow you to field
very time sensitive interrupts fairly reliably depending upon the hardware
as the number of places Fuzix blocks 'real' interrupts can be quite low,
especially if the processor has separate supervisor and user mode stacks.

### CPU Specific Limitations

Currently the Z80 and 6502 platforms do not implement the correct soft
versus hard interrupt distinctions in the CPU specific low level code. This
would need changing in order to support soft interrupts.

Soft interrupts are not easily applicable to the 6502 platform due to the
fixed 256 byte stack. It is also more complex and less effective on a
platform without separate supervisor or interrupt stacks.

### 6809 Implementation

The 6809 core code blocks the IRQ line but does not touch the state of FIR,
so that the FIR interrupt can be used in this fashion if the platform
supports it. No special logic should be needed beyond setting up the FIR in
platform code and unmasking it when appropriate.

If you wish to use FIR in this fashion you need to define CONFIG_SOFT_IRQ
and ensure that your platform __hard_di()/__hard_irqrestore() block and restore
both IRQ and FIR (unless the FIR vector/code are always mapped), but your
di() and irqrestore() methods need not disable FIR.

Because the 6809 FIR mask lives in the condition codes, and the condition
codes get stacked and restored any FIR routine which tries to disable itself
is likely to get into trouble. Instead enable and disable FIR from the
kernel context.

